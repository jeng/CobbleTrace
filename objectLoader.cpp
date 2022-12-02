//TODO the file handling routines will need to move to the platform layer
//TODO also need to come up with a better way the handle object memory
//TODO for now I just want to get the object loaded and on the screen
//TODO I also need to add asset caching so that I don't load all of the
//triangles again when we already have the object in memory

#include <cstdio>
#include <cassert>
#include <cstdio>
#include <cstring>
#include "objectLoader.h"
#include "fileBuffer.h"

#define LOG_OBJECT_DATA (0) 

v3_t TranslatePoint(m4x4_t translation, v3_t point){
    v4_t v4;
    v4.x = point.x;
    v4.y = point.y;
    v4.z = point.z;
    v4.w = 1;

    v4 = v4 * translation;
    return {v4.x, v4.y, v4.z};
}

void PlaceTriangle(scene_object_t *obj, v3_t translate, v3_t rotation, v3_t scale){
    //Rotate
    m4x4_t mRotateX;
    mRotateX.data[0][0] = 1;
    mRotateX.data[0][1] = 0;
    mRotateX.data[0][2] = 0;
    mRotateX.data[0][3] = 0;
    mRotateX.data[1][0] = 0;
    mRotateX.data[1][1] = cos(rotation.x);
    mRotateX.data[1][2] = -sin(rotation.x);
    mRotateX.data[1][3] = 0;
    mRotateX.data[2][0] = 0;
    mRotateX.data[2][1] = sin(rotation.x);
    mRotateX.data[2][2] = cos(rotation.x);
    mRotateX.data[2][3] = 0;
    mRotateX.data[3][0] = 0;
    mRotateX.data[3][1] = 0;
    mRotateX.data[3][2] = 0;
    mRotateX.data[3][3] = 1;

    m4x4_t mRotateY;
    mRotateY.data[0][0] = cos(rotation.y);
    mRotateY.data[0][1] = 0;
    mRotateY.data[0][2] = sin(rotation.y);
    mRotateY.data[0][3] = 0;
    mRotateY.data[1][0] = 0;
    mRotateY.data[1][1] = 1;
    mRotateY.data[1][2] = 0;
    mRotateY.data[1][3] = 0;
    mRotateY.data[2][0] = -sin(rotation.y);
    mRotateY.data[2][1] = 0;
    mRotateY.data[2][2] = cos(rotation.y);
    mRotateY.data[2][3] = 0;
    mRotateY.data[3][0] = 0;
    mRotateY.data[3][1] = 0;
    mRotateY.data[3][2] = 0;
    mRotateY.data[3][3] = 1;

    m4x4_t mRotateZ;
    mRotateZ.data[0][0] = cos(rotation.z);
    mRotateZ.data[0][1] = -sin(rotation.z);
    mRotateZ.data[0][2] = 0;
    mRotateZ.data[0][3] = 0;
    mRotateZ.data[1][0] = sin(rotation.z);
    mRotateZ.data[1][1] = cos(rotation.z);
    mRotateZ.data[1][2] = 0;
    mRotateZ.data[1][3] = 0;
    mRotateZ.data[2][0] = 0;
    mRotateZ.data[2][1] = 0;
    mRotateZ.data[2][2] = 1;
    mRotateZ.data[2][3] = 0;
    mRotateZ.data[3][0] = 0;
    mRotateZ.data[3][1] = 0;
    mRotateZ.data[3][2] = 0;
    mRotateZ.data[3][3] = 1;

    //Scale
    m4x4_t mScale;
    mScale.data[0][0] = scale.x;
    mScale.data[0][1] = 0;
    mScale.data[0][2] = 0;
    mScale.data[0][3] = 0;
    mScale.data[1][0] = 0;
    mScale.data[1][1] = scale.y;
    mScale.data[1][2] = 0;
    mScale.data[1][3] = 0;
    mScale.data[2][0] = 0;
    mScale.data[2][1] = 0;
    mScale.data[2][2] = scale.z;
    mScale.data[2][3] = 0;
    mScale.data[3][0] = 0;
    mScale.data[3][1] = 0;
    mScale.data[3][2] = 0;
    mScale.data[3][3] = 1;

    //Translate
    m4x4_t mTranslate;
    mTranslate.data[0][0] = 1;
    mTranslate.data[0][1] = 0;
    mTranslate.data[0][2] = 0;
    mTranslate.data[0][3] = 0;
    mTranslate.data[1][0] = 0;
    mTranslate.data[1][1] = 1;
    mTranslate.data[1][2] = 0;
    mTranslate.data[1][3] = 0;
    mTranslate.data[2][0] = 0;
    mTranslate.data[2][1] = 0;
    mTranslate.data[2][2] = 1;
    mTranslate.data[2][3] = 0;
    mTranslate.data[3][0] = translate.x;
    mTranslate.data[3][1] = translate.y;
    mTranslate.data[3][2] = translate.z;
    mTranslate.data[3][3] = 0;

    obj->triangle.p1 = TranslatePoint(mRotateY, obj->triangle.p1);
    obj->triangle.p2 = TranslatePoint(mRotateY, obj->triangle.p2);
    obj->triangle.p3 = TranslatePoint(mRotateY, obj->triangle.p3);

    obj->triangle.p1 = TranslatePoint(mRotateX, obj->triangle.p1);
    obj->triangle.p2 = TranslatePoint(mRotateX, obj->triangle.p2);
    obj->triangle.p3 = TranslatePoint(mRotateX, obj->triangle.p3);

    obj->triangle.p1 = TranslatePoint(mRotateZ, obj->triangle.p1);
    obj->triangle.p2 = TranslatePoint(mRotateZ, obj->triangle.p2);
    obj->triangle.p3 = TranslatePoint(mRotateZ, obj->triangle.p3);

    obj->triangle.p1 = TranslatePoint(mScale, obj->triangle.p1);
    obj->triangle.p2 = TranslatePoint(mScale, obj->triangle.p2);
    obj->triangle.p3 = TranslatePoint(mScale, obj->triangle.p3);

    obj->triangle.p1 = TranslatePoint(mTranslate, obj->triangle.p1);
    obj->triangle.p2 = TranslatePoint(mTranslate, obj->triangle.p2);
    obj->triangle.p3 = TranslatePoint(mTranslate, obj->triangle.p3);
}

void ImportPlyObject(scene_object_t importObject, scene_t *scene){
    filebuffer_t fb;
    OpenFileBuffer(&fb, importObject.import.filename);
    string_t token;
    char data[FB_LINE_SZ];
    token.data = &data[0];
    token.size = 0;
    GetStringRaw(&fb, &token, FB_LINE_SZ);
    assert(IsStringEqual(&token, "ply"));

    uint32_t totalVertices = 0;
    uint32_t totalFaces = 0;

    while(!IsEOF(&fb)){
        GetStringRaw(&fb, &token, FB_LINE_SZ);
        if (IsStringEqual(&token, "element")){
            GetStringRaw(&fb, &token, FB_LINE_SZ);
            if (IsStringEqual(&token, "vertex")){
                totalVertices = GetNumber(&fb);
            } else if (IsStringEqual(&token, "face")){
                totalFaces = GetNumber(&fb);
            }
        } else if (IsStringEqual(&token, "end_header")){
            SkipLine(&fb);
            break;
        }
    }

    v3_t *vertices = (v3_t*)malloc(totalVertices * sizeof(v3_t));
    for(int i = 0; i < totalVertices && !IsEOF(&fb); i++){
        vertices[i] = GetV3Raw(&fb);
        SkipLine(&fb);
    }

    //v3_t *faces = (v3_t*)malloc(totalFaces * sizeof(v3_t));
    for(int i = 0; i < totalFaces && !IsEOF(&fb); i++){
        int n = GetNumber(&fb);
        assert(n == 3);
        v3_t face = GetV3Raw(&fb);
        SkipLine(&fb);

        //TODO In the bvh code I'm converting from triangles back to a vertex list iirc.  
        //     I need to just decide on one and stick with it.  I could easily change the 
        //     triangle loading code in scenefile.cpp to use faces and vertices

        scene_object_t obj = importObject;
        obj.type = OT_TRIANGLE;
        obj.triangle.p1 = vertices[(int)face.x];
        obj.triangle.p2 = vertices[(int)face.y];
        obj.triangle.p3 = vertices[(int)face.z];

        PlaceTriangle(&obj, importObject.import.position, importObject.import.rotation, importObject.import.scale);

        assert(scene->objectStack.index < scene->objectStack.size);
        SDL_memcpy(&scene->objectStack.objects[scene->objectStack.index++], &obj, sizeof(obj));
        scene->triangleLookup.triangleCount++;
    }

    free(vertices);
    CloseFileBuffer(&fb);
}

//TODO start using my own string class.
//I can use the one from the static site generator.  Just clean up the interface some
void ImportBlenderObject(scene_object_t importObject, scene_t *scene){
    filebuffer_t fb;
    OpenFileBuffer(&fb, importObject.import.filename);
    
    //TODO Change this code to dynamically allocate the buffer size based on the file.
    //     We will also need to dynamically grow the memory allocated for the NUM_FACES and NUM_VERT
    //
    //     Once we're loading ply files this code will have a lot of overlap with that code.  At that point it 
    //     will probably make sense to create a dynamic structure for v3_t.
    //
    //     I mention stacks in the comment below related to v3 vertices and faces but I'm not using a stack for allocations.
    //     I can probably change all of the scene parsing code to use a stack based allocation scheme.

    //TODO this is terrible and needs to be fixed.  I'm not loading anything huge though
    //Just start with 1024 faces and triangles 
    const int NUM_FACES = 1024;
    const int NUM_VERT = 1024;
    v3_t faces[NUM_FACES];
    v3_t vertices[NUM_VERT];
    int vi = 0;
    int vf = 0;

    while(!IsEOF(&fb)){
        char c = GetToken(&fb);
        if (c == '#'){
            SkipLine(&fb);
        } else if (c == 'o'){
            SkipLine(&fb);
        } else if (c == 'v'){
            vertices[vi++] = GetV3Raw(&fb);
            assert(vi < NUM_VERT);
        } else if (c == 'f'){
            faces[vf++] = GetV3Raw(&fb);
            assert(vf < NUM_FACES);
        }
    }
    CloseFileBuffer(&fb);
    //store the faces in an object that can be returned
    if (vf <= 0)
        return;

#if LOG_OBJECT_DATA    
    SDL_Log("Vector Output\n");
    for(int i = 0; i < vi; i++){
        char s[1024];
        sprintf(s, "v %f %f %f\n", vertices[i].x, vertices[i].y, vertices[i].z);
        SDL_Log(s);
    }
    SDL_Log("\n\nFace Output\n");
    for(int i = 0; i < vf; i++){
        char s[1024];
        sprintf(s, "f %.0f %.0f %.0f\n", faces[i].x, faces[i].y, faces[i].z);
        SDL_Log(s);
    }
#endif    

    for (int i = 0; i < vf; i++){
        //TODO In the bvh code I'm converting from triangles back to a vertex list iirc.  
        //     I need to just decide on one and stick with it.  I could easily change the 
        //     triangle loading code in scenefile.cpp to use faces and vertices
        scene_object_t obj = importObject;
        obj.type = OT_TRIANGLE;
        obj.triangle.p1 = vertices[(int)faces[i].x - 1];
        obj.triangle.p2 = vertices[(int)faces[i].y - 1];
        obj.triangle.p3 = vertices[(int)faces[i].z - 1];

        PlaceTriangle(&obj, importObject.import.position, importObject.import.rotation, importObject.import.scale);

        assert(scene->objectStack.index < scene->objectStack.size);
        SDL_memcpy(&scene->objectStack.objects[scene->objectStack.index++], &obj, sizeof(obj));
        scene->triangleLookup.triangleCount++;
    }

}


void ImportObject(scene_object_t importObject, scene_t *scene){
    if (importObject.import.format == IT_BLENDER){
        ImportBlenderObject(importObject, scene);
    } else if (importObject.import.format == IT_PLY){
        ImportPlyObject(importObject, scene);
    }
}