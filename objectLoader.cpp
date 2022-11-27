//TODO the file handling routines will need to move to the platform layer
//TODO also need to come up with a better way the handle object memory
//TODO for now I just want to get the object loaded and on the screen
//#include <windows.h>
#include <cstdio>
#include <cassert>
#include <cstdio>
//#include "platform.h"
#include "objectLoader.h"
//#include "trirender.h"

#define LOG_OBJECT_DATA (1) 

enum readState_t{
    NEW_LINE,
    READ_V,
    READ_F,
    SKIP
};

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


//If I could write this anyway that I wanted
// stringList_t sl = StringList(filename);
// for(int i = 0; i < sl.count; i++){
//     if (ObjectLineType(sl[i]) == VERTEX){
//         AppendList(vertices, ParseLine(sl[i]));
//     } else if (ObjectLineType(sl[i]) == FACE){
//         AppendList(faces, ParseLine(sl[i]))
//     }
// }

//TODO start using my own string class.
//I can use the one from the static site generator.  Just clean up the interface some
void ImportObject(scene_object_t importObject, scene_t *scene){
//triangleList_t LoadObject(const char *filename, scene_object_t obj, scene){
    //OFSTRUCT fileStruct;
    //HANDLE fileHandle;

    //fileHandle = CreateFile(importObject.filename,  // file to open
    //                   GENERIC_READ,                // open for reading
    //                   FILE_SHARE_READ,             // share for reading
    //                   NULL,                        // default security
    //                   OPEN_EXISTING,               // existing file only
    //                   FILE_ATTRIBUTE_NORMAL,       // normal file
    //                   NULL);                       // no attr. template
    FILE *fp;
    
    char *filename;
    int filenameSize = importObject.import.filename.size + 1;
    filename = (char *)malloc(filenameSize * sizeof(char));
    assert(filename != NULL);
    StringToCharPtr(importObject.import.filename, filename, filenameSize);

    fp = fopen(filename, "r");
    
    free(filename);

    assert(fp != NULL);
    fseek(fp, 0, SEEK_END);
    int pos = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    //TODO Change this code to dynamically allocate the buffer size based on the file.
    //     We will also need to dynamically grow the memory allocated for the NUM_FACES and NUM_VERT
    //
    //     Once we're loading ply files this code will have a lot of overlap with that code.  At that point it 
    //     will probably make sense to create a dynamic structure for v3_t.
    //
    //     I mention stacks in the comment below related to v3 vertices and faces but I'm not using a stack for allocations.
    //     I can probably change all of the scene parsing code to use a stack based allocation scheme.

    uint64_t bytesRead;
    const int BUFSIZE = 1024;
    char buffer[BUFSIZE];
    readState_t state = NEW_LINE;

    //TODO this is terrible and needs to be fixed.  I'm not loading anything huge though
    //Just start with 1024 faces and triangles 
    const int NUM_FACES = 1024;
    const int NUM_VERT = 1024;
    v3_t faces[NUM_FACES];
    v3_t vertices[NUM_VERT];
    int fidx = 0;
    int vidx = 0;
    int vtotal = 0;
    int ftotal = 0;

    int nidx = 0;
    int val_idx = 0;
    char val[1024];//TODO I hate this code

    for(;;){

        //TODO Get the file size from the file info
        //and allocate here

        //if (!ReadFile(fileHandle, 
        //            buffer,
        //            BUFSIZE - 1,
        //            &bytesRead, NULL)){
        //    ErrorDialog("Reading the file failed");
        //    CloseHandle(fileHandle);
        //    return result;
        //}

        bytesRead = fread(buffer, sizeof(char), BUFSIZE - 1, fp);
        if (bytesRead <= 0)
            break;

        //If the line starts with a v this is vertex data and we need to push a new v3 on the stack.
        //If the line starts with a f this is data for the face and we'll need
        //to refer back to the vertex data to get the correct vertices that
        //make up this triangle
        //
        //If we run out of bytes will reading then we need to get more data from the file
        for(int i = 0; i < bytesRead; i++){
            if (state == NEW_LINE && buffer[i] == 'v'){
                state = READ_V;
                nidx = -1;
            } else if (state == NEW_LINE && buffer[i] == 'f'){
                state = READ_F;
                nidx = -1;
            } else if (state == READ_V){
                if (buffer[i] == '\n'){
                    //TODO I hate this code
                    if (nidx == 2){
                        val[val_idx] = '\0';
                        sscanf(val, "%lf", &vertices[vidx].z);
                    }
                    vtotal++;
                    vidx++;
                    assert(vidx < NUM_VERT);
                    state = NEW_LINE;
                    //TODO this is all a huge hack.  I'm assuming single spaces between numbers.  I'm not sure why I didn't make this a skip space call to begin with?
                } else if (buffer[i] == ' '){
                    val[val_idx] = '\0';
                    val_idx = 0;

                    if (nidx == 0){
                        sscanf(val, "%lf", &vertices[vidx].x);
                    } else if (nidx == 1){
                        sscanf(val, "%lf", &vertices[vidx].y);
                    } else if (nidx == 2){
                        sscanf(val, "%lf", &vertices[vidx].z);
                    }

                    nidx++;
                } else {
                    val[val_idx++] = buffer[i];
                }
            } else if (state == READ_F){
                if (buffer[i] == '\n'){
                    if (nidx == 2){
                        val[val_idx] = '\0';
                        sscanf(val, "%lf", &faces[fidx].z);
                    }
                    ftotal++;
                    fidx++;
                    assert(fidx < NUM_FACES);
                    state = NEW_LINE;
                    //TODO this is all a huge hack.  I'm assuming single spaces between numbers
                } else if (buffer[i] == ' '){
                    val[val_idx] = '\0';
                    val_idx = 0;

                    if (nidx == 0){
                        sscanf(val, "%lf", &faces[fidx].x);
                    } else if (nidx == 1){
                        sscanf(val, "%lf", &faces[fidx].y);
                    } else if (nidx == 2){
                        sscanf(val, "%lf", &faces[fidx].z);
                    }

                    nidx++;
                } else {
                    val[val_idx++] = buffer[i];
                }
            } else if (state == SKIP){
                if (buffer[i] == '\n')
                    state = NEW_LINE;
            } else {
                state = SKIP;
            }
        }

        if (bytesRead < BUFSIZE - 1)
            break;
    }

    fclose(fp);

    //store the faces in an object that can be returned
    if (ftotal <= 0)
        return;

#if LOG_OBJECT_DATA    
    SDL_Log("Vector Output\n");
    for(int i = 0; i < vtotal; i++){
        char s[1024];
        sprintf(s, "v %f %f %f\n", vertices[i].x, vertices[i].y, vertices[i].z);
        SDL_Log(s);
    }
    SDL_Log("\n\nFace Output\n");
    for(int i = 0; i < ftotal; i++){
        char s[1024];
        sprintf(s, "f %.0f %.0f %.0f\n", faces[i].x, faces[i].y, faces[i].z);
        SDL_Log(s);
    }
#endif    

    for (int i = 0; i < ftotal; i++){
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
    //result.triangles = (triangle_t*)malloc(sizeof(triangle_t) * ftotal);
    //for(int i = 0; i < ftotal; i++){
    //    assert(faces[i].x - 1 >= 0);
    //    assert(faces[i].x - 1 < vtotal);
    //    assert(faces[i].y - 1 >= 0);
    //    assert(faces[i].y - 1 < vtotal);
    //    assert(faces[i].z - 1 >= 0);
    //    assert(faces[i].z - 1 < vtotal);
 
    //    result.triangles[i].p1 = vertices[(int)faces[i].x - 1];
    //    result.triangles[i].p2 = vertices[(int)faces[i].y - 1];
    //    result.triangles[i].p3 = vertices[(int)faces[i].z - 1];
    //}
    //result.size = ftotal;
}