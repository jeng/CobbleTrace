#include <assert.h>
#include "scenefile.h"
#include "color.h"

struct FileBuffer{
    int size;
    int index;
    char *data;
};

struct String {
    int size;
    char *data;
};


String
CharPtrToString(const char *p){
    String result;
    result.size = 0;
    result.data = (char *)p;
    while(*p != '\0'){
        result.size++;
        p++;
    }
    return result;
}

bool
IsStringEqual(String *a, String *b){
    if (a->size != b->size)
        return false;
    for(int i = 0; i < a->size; i++) {
        if (a->data[i] != b->data[i])
            return false;
    }
    return true;
}

bool
IsStringEqual(String *a, const char *b){
    String sB = CharPtrToString(b);
    return IsStringEqual(a, &sB);
}

void
SkipSpace(FileBuffer *fb){
    while(fb->index < fb->size){
        if (fb->data[fb->index] != '\n' && 
            fb->data[fb->index] != '\r' && 
            fb->data[fb->index] != ' ' && 
            fb->data[fb->index] != '\t'){            
            return;
        }
        fb->index++;
    }
}

char
GetToken(FileBuffer *fb){
    if (fb->index >= fb->size){
        return 0;
    }
    SkipSpace(fb);
    return fb->data[fb->index++];
}

void
PushToken(FileBuffer *fb){
    if (fb->index > 0)
        fb->index--;
}



String
GetString(FileBuffer *fb){
    String result;
    result.size = 0;
    result.data = NULL;

    SkipSpace(fb);

    if (fb->index >= fb->size)
        return result;

    assert(fb->data[fb->index] == '"');

    fb->index++;
    
    if (fb->index >= fb->size)
        return result;

    result.data = &fb->data[fb->index];

    while(fb->index < fb->size && fb->data[fb->index] != '"'){
        result.size++;
        fb->index++;
    }

    fb->index++;

    return result;
}

bool
GetBoolean(FileBuffer *fb){
    bool result;
    String s;
    char d[10] = {0};
    s.size = 0;
    s.data = (char*)&d;
    SkipSpace(fb);

    while(fb->index < fb->size && s.size < 5 && 'a' <= fb->data[fb->index] && fb->data[fb->index] <= 'z'){
        s.data[s.size++] = fb->data[fb->index++];
    }

    if (IsStringEqual(&s, "true"))
        return true;
    else if (IsStringEqual(&s, "false"))
        return false;
    else
        assert(false);

    return false;
}

void 
InitSceneData(scene_t *scene){
    scene->lightStack.size = MAX_LIGHTS;
    scene->lightStack.index = 0;
    scene->lightStack.lights = (light_t*)calloc(MAX_LIGHTS, sizeof(light_t));
    scene->lightStack.type = ST_LIGHT;

    scene->objectStack.size = MAX_OBJECTS;
    scene->objectStack.index = 0;
    scene->objectStack.objects = (scene_object_t*)calloc(MAX_OBJECTS, sizeof(scene_object_t));
    scene->objectStack.type = ST_OBJECT;

    scene->settings.numberOfThreads = 8;
    scene->settings.subsampling = true;

    scene->camera.position = {0, 0, -8};
}

void
AssertNextToken(FileBuffer *fb, char c){
    char x = GetToken(fb);
    if (c != x){
        char s[100];
        sprintf(s, "expected %c got %c\n", c, x);
        SDL_Log(s);
    }
    assert(c == x);
}

float
GetNumber(FileBuffer *fb){
    char c = GetToken(fb);
    float n = 1;
    float result = 0;
    bool decimal = false;
    float d = 10;

    if (c == '-'){
        n = -1;
    } else {
        PushToken(fb);
    }
    
    while(fb->index < fb->size){
        c =  fb->data[fb->index];
        if ('0' <= c && c <= '9'){
            if (decimal){
                float x = c - '0';
                x = x/d;
                result += x;
                d *= 10;
                
            } else {
                result = result * 10;
                result += c - '0';
            }
        } else if (c == '.'){            
            assert(!decimal);
            decimal = true;
        } else {
            break;
        }
        fb->index++;
    }

    return n * result;    
}

v3_t
GetV3(FileBuffer *fb){
    float f;
    v3_t v;
    AssertNextToken(fb, '[');
    v.x = GetNumber(fb);
    AssertNextToken(fb, ',');
    v.y = GetNumber(fb);
    AssertNextToken(fb, ',');
    v.z = GetNumber(fb);
    AssertNextToken(fb, ']');
    return v;
}

void
LoadObjects(FileBuffer *fb, scene_t *scene){
    String s;

    AssertNextToken(fb, '[');
    while(fb->index < fb->size){

        AssertNextToken(fb, '{');
        scene_object_t obj = {};
        while(fb->index < fb->size){            
            AssertNextToken(fb, '"');
            PushToken(fb);
            s = GetString(fb);
            AssertNextToken(fb, ':');

            if (IsStringEqual(&s, "type")){
                s = GetString(fb);
                if (IsStringEqual(&s,"sphere")){
                    obj.type = OT_SPHERE;                    
                } else if (IsStringEqual(&s, "triangle")){
                    obj.type = OT_TRIANGLE;
                } else {
                    assert(false);
                }
            } else if (IsStringEqual(&s, "center")){                
                obj.sphere.center = GetV3(fb);
            } else if (IsStringEqual(&s, "radius")) {
                obj.sphere.radius = GetNumber(fb);
            } else if (IsStringEqual(&s, "color")) {
                v3_t v = GetV3(fb);
                obj.material.color = RgbToColor(v.x, v.y, v.z);
            } else if (IsStringEqual(&s, "specular")) {
                obj.material.specular = GetNumber(fb);
            } else if (IsStringEqual(&s, "reflection")) {
                obj.material.reflection = GetNumber(fb);
            } else if (IsStringEqual(&s, "p1")) {
                obj.triangle.p1 = GetV3(fb);
            } else if (IsStringEqual(&s, "p2")) {
                obj.triangle.p2 = GetV3(fb);
            } else if (IsStringEqual(&s, "p3")) {
                obj.triangle.p3 = GetV3(fb);
            }

            char c = GetToken(fb);
            if (c == '}')
                break;
            PushToken(fb);
            AssertNextToken(fb, ',');
        }
        
        //Push the object on the stack
        assert(scene->objectStack.index < scene->objectStack.size);
        SDL_memcpy(&scene->objectStack.objects[scene->objectStack.index++], &obj, sizeof(obj));
        
        //Are we at the end of the list?
        char c = GetToken(fb);
        if (c == ']')
            break;

        PushToken(fb);
        AssertNextToken(fb, ',');
    }
}

void
LoadLights(FileBuffer *fb, scene_t *scene){
    String s;

    AssertNextToken(fb, '[');
    while(fb->index < fb->size){

        AssertNextToken(fb, '{');
        light_t light = {};
        while(fb->index < fb->size){            
            AssertNextToken(fb, '"');
            PushToken(fb);
            s = GetString(fb);
            AssertNextToken(fb, ':');

            if (IsStringEqual(&s, "type")){
                s = GetString(fb);
                if (IsStringEqual(&s,"ambient")){
                    light.type = LT_AMBIENT;                    
                } else if (IsStringEqual(&s, "point")){
                    light.type = LT_POINT;
                } else if (IsStringEqual(&s, "directional")){
                    light.type = LT_DIRECTIONAL;                    
                } else {
                    assert(false);
                }
            } else if (IsStringEqual(&s, "intensity")){                
                light.intensity = GetNumber(fb);
            } else if (IsStringEqual(&s, "position")) {
                light.position = GetV3(fb);
            } else if (IsStringEqual(&s, "direction")) {
                light.direction = GetV3(fb);
            } else {
                assert(false);
            }

            char c = GetToken(fb);
            if (c == '}')
                break;
            PushToken(fb);
            AssertNextToken(fb, ',');
        }
        
        //Push the object on the stack
        assert(scene->lightStack.index < scene->lightStack.size);
        SDL_memcpy(&scene->lightStack.lights[scene->lightStack.index++], &light, sizeof(light));
        
        //Are we at the end of the list?
        char c = GetToken(fb);
        if (c == ']')
            break;

        PushToken(fb);
        AssertNextToken(fb, ',');
    }
}

void
LoadCamera(FileBuffer *fb, scene_t *scene){
    String s;
    AssertNextToken(fb, '{');
    AssertNextToken(fb, '"');
    PushToken(fb);
    s = GetString(fb);
    AssertNextToken(fb, ':');
    if (IsStringEqual(&s, "position")){
        scene->camera.position = GetV3(fb);
    } else {
        assert(false);
    }
    AssertNextToken(fb, '}');    
}

void
LoadSettings(FileBuffer *fb, scene_t *scene){
    String s;
    AssertNextToken(fb, '{');

    while(fb->index < fb->size){            
        AssertNextToken(fb, '"');
        PushToken(fb);
        s = GetString(fb);
        AssertNextToken(fb, ':');

        if (IsStringEqual(&s, "numberOfThreads")){
            scene->settings.numberOfThreads = GetNumber(fb);
        } else if (IsStringEqual(&s, "subsampling")){
            scene->settings.subsampling = GetBoolean(fb);
        } else if (IsStringEqual(&s, "wireFrame")){
            scene->settings.wireFrame = GetBoolean(fb);
        } else {
            assert(false);
        }

        char c = GetToken(fb);
        if (c == '}')
            break;
        PushToken(fb);
        AssertNextToken(fb, ',');
    }
}

void
ParseSceneFile(char *filename, scene_t *scene){    
    FileBuffer fb;
    FILE *f = fopen(filename, "r");
    fb.data = (char*)calloc(sizeof(char), MAX_SCENEFILE);
    fb.index = 0;
    fb.size = fread(fb.data, sizeof(char), MAX_SCENEFILE, f);
    if (!feof(f)){
        SDL_Log("Scene file is too big");
        exit(1);
    }
    fclose(f);

    AssertNextToken(&fb, '{');

    String s;

    while(fb.index < fb.size){
        AssertNextToken(&fb, '"');
        PushToken(&fb);
        s = GetString(&fb);        

        if (IsStringEqual(&s, "objects")){
            AssertNextToken(&fb, ':');
            LoadObjects(&fb, scene);
        } else if (IsStringEqual(&s, "lights")){
            AssertNextToken(&fb, ':');
            LoadLights(&fb, scene);
        } else if (IsStringEqual(&s, "camera")){
            AssertNextToken(&fb, ':');
            LoadCamera(&fb, scene);
        } else if (IsStringEqual(&s, "settings")){
            AssertNextToken(&fb, ':');
            LoadSettings(&fb, scene);
        }

        char c = GetToken(&fb);
        if (c == '}')
            break;
            
        PushToken(&fb);
        AssertNextToken(&fb, ',');
    }
            
    free(fb.data);    
}
