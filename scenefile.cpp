#include <assert.h>
#include "scenefile.h"
#include "color.h"
#include "objectLoader.h"
#include "ctstring.h"
#include "fileBuffer.h"


//TODO clean up the GetString code by using a string stack

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
    scene->settings.wireframe = false;
    scene->settings.supersampling = false;

    scene->camera.position = {0, 0, -8};
}

void
LoadObjects(filebuffer_t *fb, scene_t *scene){
    char data[FB_LINE_SZ];
    char fileData[FB_LINE_SZ];

    string_t s;
    s.data = &data[0];

    string_t filename;
    filename.data = &fileData[0];

    AssertNextToken(fb, '[');
    
    int triangleCount = 0;

    while(!IsEOF(fb)){

        AssertNextToken(fb, '{');
        scene_object_t obj = {};
        while(!IsEOF(fb)){            
            AssertNextToken(fb, '"');
            PushToken(fb);
            GetString(fb, &s, FB_LINE_SZ);
            AssertNextToken(fb, ':');

            if (IsStringEqual(&s, "type")){
                GetString(fb, &s, FB_LINE_SZ);
                if (IsStringEqual(&s,"sphere")){
                    obj.type = OT_SPHERE;                    
                } else if (IsStringEqual(&s, "triangle")){
                    obj.type = OT_TRIANGLE;
                    triangleCount++;
                } else if (IsStringEqual(&s, "import")){
                    obj.type = OT_IMPORT;
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
            } else if (IsStringEqual(&s, "filename")) {
                //TODO hack. I really want to be using a string stack here.
                //     Every GetString call would push a new string on the stack
                //     then when at the end of the scope we just pop everything
                GetString(fb, &filename, FB_LINE_SZ);
                obj.import.filename = filename;
            } else if (IsStringEqual(&s, "position")){
                obj.import.position = GetV3(fb);
            } else if (IsStringEqual(&s, "rotation")){
                obj.import.rotation = GetV3(fb);
            } else if (IsStringEqual(&s, "scale")){
                obj.import.scale = GetV3(fb);
            } else if (IsStringEqual(&s, "format")){
                GetString(fb, &s, FB_BUFFER_SZ);
                if (IsStringEqual(&s, "blender")){
                    obj.import.format = IT_BLENDER;
                } else if (IsStringEqual(&s, "ply")) {
                    obj.import.format = IT_PLY;
                }
            }

            char c = GetToken(fb);
            if (c == '}')
                break;
            PushToken(fb);
            AssertNextToken(fb, ',');
        }

        if (obj.type == OT_IMPORT){
            ImportObject(obj, scene);
        } else {
            //Push the object on the stack
            assert(scene->objectStack.index < scene->objectStack.size);
            SDL_memcpy(&scene->objectStack.objects[scene->objectStack.index++], &obj, sizeof(obj));
            scene->triangleLookup.triangleCount += triangleCount;
        }
        
       
        //Are we at the end of the list?
        char c = GetToken(fb);
        if (c == ']')
            break;

        PushToken(fb);
        AssertNextToken(fb, ',');
    }
}

void
LoadLights(filebuffer_t *fb, scene_t *scene){

    string_t s;
    char data[FB_LINE_SZ];
    s.data = &data[0];

    AssertNextToken(fb, '[');
    while(!IsEOF(fb)){

        AssertNextToken(fb, '{');
        light_t light = {};
        while(!IsEOF(fb)){            
            AssertNextToken(fb, '"');
            PushToken(fb);
            GetString(fb, &s, FB_LINE_SZ);
            AssertNextToken(fb, ':');

            if (IsStringEqual(&s, "type")){
                GetString(fb, &s, FB_LINE_SZ);
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
LoadCamera(filebuffer_t *fb, scene_t *scene){
    string_t s;
    char data[FB_LINE_SZ];
    s.data = &data[0];

    AssertNextToken(fb, '{');
    AssertNextToken(fb, '"');
    PushToken(fb);
    GetString(fb, &s, FB_LINE_SZ);
    AssertNextToken(fb, ':');
    if (IsStringEqual(&s, "position")){
        scene->camera.position = GetV3(fb);
    } else {
        assert(false);
    }
    AssertNextToken(fb, '}');    
}

void
LoadSettings(filebuffer_t *fb, scene_t *scene){
    string_t s;
    char data[FB_LINE_SZ];
    s.data = &data[0];

    AssertNextToken(fb, '{');

    while(!IsEOF(fb)){            
        AssertNextToken(fb, '"');
        PushToken(fb);
        GetString(fb, &s, FB_LINE_SZ);
        AssertNextToken(fb, ':');

        if (IsStringEqual(&s, "numberOfThreads")){
            scene->settings.numberOfThreads = GetNumber(fb);
        } else if (IsStringEqual(&s, "subsampling")){
            scene->settings.subsampling = GetBoolean(fb);
        } else if (IsStringEqual(&s, "wireframe")){
            scene->settings.wireframe = GetBoolean(fb);
        } else if (IsStringEqual(&s, "supersampling")){
            scene->settings.supersampling = GetBoolean(fb);
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

void MakeTriangleLookup(scene_t *scene){
    scene->triangleLookup.indexes = (uint32_t*)calloc(scene->triangleLookup.triangleCount, sizeof(uint32_t));
    int n = 0;
    for(int i = 0; i < scene->objectStack.index; i++){
        if(scene->objectStack.objects[i].type == OT_TRIANGLE){
            scene->triangleLookup.indexes[n++] = i;
        }
    }
}

void
ParseSceneFile(char *filename, scene_t *scene){    
    filebuffer_t fb;
    OpenFileBuffer(&fb, filename);

    AssertNextToken(&fb, '{');

    string_t s;
    char data[FB_LINE_SZ];
    s.data = &data[0];

    scene->triangleLookup.triangleCount = 0;

    while(!IsEOF(&fb)){
        AssertNextToken(&fb, '"');
        PushToken(&fb);
        GetString(&fb, &s, FB_LINE_SZ);        

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

    MakeTriangleLookup(scene);

    CloseFileBuffer(&fb);            
}
