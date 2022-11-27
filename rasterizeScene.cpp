
#define _USE_MATH_DEFINES // for C++
#include <cmath>
#include <cstring>
#include "rasterizeScene.h"
#include "color.h"
#include "draw2d.h"


static void 
Swap(v3_t *a, v3_t *b){
    v3_t x = *a;
    *a = *b;
    *b = x;
}

static void
CanvasPutPixel(bitmapSettings_t *bitmap, v2_t v2, uint32_t color){
    //We want to have -w/2 to w/2
    //and h/2 to -h/2 for the x and y axis
    v2_t v;
    v.x = v2.x + (bitmap->width/2);
    v.y = (bitmap->height/2) + -v2.y;
    PutPixel(bitmap, v, color);
}

static void DrawLineHorizontalish(environment_t *env, v3_t p1, v3_t p2, uint32_t color) {
    if (p1.x > p2.x) {
        Swap(&p1, &p2);
    }
    float a = (p2.y - p1.y) / (p2.x - p1.x);
    //float b = p1.y - a * p1.x;
    float y = p1.y;
    for(int x = p1.x; x <= p2.x; x++){
        //float y = a * x + b;
        CanvasPutPixel(env->bitmap, {(float)x, y}, color);
        y += a;
    }
}

static void DrawLineVerticalish(environment_t *env, v3_t p1, v3_t p2, uint32_t color) {
    if (p1.y > p2.y) {
        Swap(&p1, &p2);
    }
    float a = (p2.x - p1.x) / (p2.y - p1.y);
    float x = p1.x;
    for(int y = p1.y; y <= p2.y; y++){
        //float y = a * x + b;
        CanvasPutPixel(env->bitmap, {x, (float)y}, color);
        x += a;
    }
}

static void DrawLine(environment_t *env, v3_t p1, v3_t p2, uint32_t color) {
    int dx = p2.x - p1.x;
    int dy = p2.y - p1.y;
    if (abs(dx) > abs(dy)) {
        DrawLineHorizontalish(env, p1, p2, color);
    } else {
        DrawLineVerticalish(env, p1, p2, color);
    }
}

static void
DrawWireframeTriangle(environment_t *env, triangle_t t, uint32_t color){
    DrawLine(env, t.p1, t.p2, color);
    DrawLine(env, t.p2, t.p3, color);
    DrawLine(env, t.p3, t.p1, color);
}


//static void RenderTriangle(environment_t *env, v3_t *triangle, v3_t *projectedVertexList){
//    DrawWireframeTriangle(env, {
//            projectedVertexList[(int)triangle->x],
//            projectedVertexList[(int)triangle->y],
//            projectedVertexList[(int)triangle->z]},
//        RgbToColor(0x10, 0x10, 0x10));
//}
//
static v3_t
ViewportToCanvas(environment_t *env, viewport_t *viewport, v3_t position){
    return {(position.x * env->bitmap->width) / viewport->width, (position.y * env->bitmap->height) / viewport->height, 0};
}

static v3_t
ProjectVertex(environment_t *env, viewport_t *viewport, camera_t *camera, v3_t position){
    return ViewportToCanvas(env, viewport, 
        {((camera->position.x - position.x) * viewport->d)/(camera->position.z - position.z), 
         ((camera->position.y - position.y) * viewport->d)/(camera->position.z - position.z), 
         0});
}

//static void RenderObject(environment_t *env, viewport_t *viewport, model_t *model){
//    //TODO I'm not crazy about allocating on the heap here
//    v3_t *projected = (v3_t*)calloc(model->verticesSize, sizeof(v3_t));
//    for(int i = 0; i < model->verticesSize; i++){
//        projected[i] = ProjectVertex(env, viewport, model->vertices[i]);
//    }
//    for(int i = 0; i < model->trianglesSize; i++){
//        RenderTriangle(env, &model->triangles[i], projected);
//    }
//    free(projected);
//}

//static triangle_t *GetSphereTriangles(sphere_t *sphere, int *size){
//    *size = 4;
//    triangle_t *result = (triangle_t*)calloc(*size, sizeof(triangle_t));
//    float a = 1.0f / 3.0f;
//    float b = sqrt(8.0f / 9.0f);
//    float c = sqrt(2.0f / 9.0f);
//    float d = sqrt(2.0f / 3.0f);
//    //TODO(JHE): using matrix translation for this
//    v3_t v0 = {0 + sphere->center.x, 0 + sphere->center.y, 1 + sphere->center.z};
//    v3_t v1 = {-c + sphere->center.x, d + sphere->center.y, -a + sphere->center.z};
//    v3_t v2 = {-c + sphere->center.x, -d + sphere->center.y, -a + sphere->center.z};
//    v3_t v3 = {b + sphere->center.x, 0 + sphere->center.y, -a + sphere->center.z};
//    result[0] = {v0, v1, v2};
//    result[1] = {v0, v2, v3};
//    result[2] = {v0, v3, v1};
//    result[3] = {v3, v2, v1};
//    return result;
//}

#define NUM_SPHERE_TRIANGLES (128)

triangle_t sphereTriangle[NUM_SPHERE_TRIANGLES] = {
    {{1,0,0}, {0.92388,0.382683,0}, {0.92388,0,0.382683}}, 
    {{0.92388,0.382683,0}, {0.707107,0.707107,0}, {0.816497,0.408248,0.408248}}, 
    {{0.92388,0,0.382683}, {0.92388,0.382683,0}, {0.816497,0.408248,0.408248}}, 
    {{0.92388,0,0.382683}, {0.816497,0.408248,0.408248}, {0.707107,0,0.707107}}, 
    {{0.707107,0.707107,0}, {0.382683,0.92388,0}, {0.408248,0.816497,0.408248}}, 
    {{0.382683,0.92388,0}, {0,1,0}, {0,0.92388,0.382683}}, 
    {{0.408248,0.816497,0.408248}, {0.382683,0.92388,0}, {0,0.92388,0.382683}}, 
    {{0.408248,0.816497,0.408248}, {0,0.92388,0.382683}, {0,0.707107,0.707107}}, 
    {{0.707107,0,0.707107}, {0.816497,0.408248,0.408248}, {0.408248,0.408248,0.816497}}, 
    {{0.816497,0.408248,0.408248}, {0.707107,0.707107,0}, {0.408248,0.816497,0.408248}}, 
    {{0.408248,0.408248,0.816497}, {0.816497,0.408248,0.408248}, {0.408248,0.816497,0.408248}}, 
    {{0.408248,0.408248,0.816497}, {0.408248,0.816497,0.408248}, {0,0.707107,0.707107}}, 
    {{0.707107,0,0.707107}, {0.408248,0.408248,0.816497}, {0.382683,0,0.92388}}, 
    {{0.408248,0.408248,0.816497}, {0,0.707107,0.707107}, {0,0.382683,0.92388}}, 
    {{0.382683,0,0.92388}, {0.408248,0.408248,0.816497}, {0,0.382683,0.92388}}, 
    {{0.382683,0,0.92388}, {0,0.382683,0.92388}, {0,0,1}}, 
    {{0,1,0}, {-0.382683,0.92388,0}, {0,0.92388,0.382683}}, 
    {{-0.382683,0.92388,0}, {-0.707107,0.707107,0}, {-0.408248,0.816497,0.408248}}, 
    {{0,0.92388,0.382683}, {-0.382683,0.92388,0}, {-0.408248,0.816497,0.408248}}, 
    {{0,0.92388,0.382683}, {-0.408248,0.816497,0.408248}, {0,0.707107,0.707107}}, 
    {{-0.707107,0.707107,0}, {-0.92388,0.382683,0}, {-0.816497,0.408248,0.408248}}, 
    {{-0.92388,0.382683,0}, {-1,0,0}, {-0.92388,0,0.382683}}, 
    {{-0.816497,0.408248,0.408248}, {-0.92388,0.382683,0}, {-0.92388,0,0.382683}}, 
    {{-0.816497,0.408248,0.408248}, {-0.92388,0,0.382683}, {-0.707107,0,0.707107}}, 
    {{0,0.707107,0.707107}, {-0.408248,0.816497,0.408248}, {-0.408248,0.408248,0.816497}}, 
    {{-0.408248,0.816497,0.408248}, {-0.707107,0.707107,0}, {-0.816497,0.408248,0.408248}}, 
    {{-0.408248,0.408248,0.816497}, {-0.408248,0.816497,0.408248}, {-0.816497,0.408248,0.408248}}, 
    {{-0.408248,0.408248,0.816497}, {-0.816497,0.408248,0.408248}, {-0.707107,0,0.707107}}, 
    {{0,0.707107,0.707107}, {-0.408248,0.408248,0.816497}, {0,0.382683,0.92388}}, 
    {{-0.408248,0.408248,0.816497}, {-0.707107,0,0.707107}, {-0.382683,0,0.92388}}, 
    {{0,0.382683,0.92388}, {-0.408248,0.408248,0.816497}, {-0.382683,0,0.92388}}, 
    {{0,0.382683,0.92388}, {-0.382683,0,0.92388}, {0,0,1}}, 
    {{-1,0,0}, {-0.92388,-0.382683,0}, {-0.92388,0,0.382683}}, 
    {{-0.92388,-0.382683,0}, {-0.707107,-0.707107,0}, {-0.816497,-0.408248,0.408248}}, 
    {{-0.92388,0,0.382683}, {-0.92388,-0.382683,0}, {-0.816497,-0.408248,0.408248}}, 
    {{-0.92388,0,0.382683}, {-0.816497,-0.408248,0.408248}, {-0.707107,0,0.707107}}, 
    {{-0.707107,-0.707107,0}, {-0.382683,-0.92388,0}, {-0.408248,-0.816497,0.408248}}, 
    {{-0.382683,-0.92388,0}, {0,-1,0}, {0,-0.92388,0.382683}}, 
    {{-0.408248,-0.816497,0.408248}, {-0.382683,-0.92388,0}, {0,-0.92388,0.382683}}, 
    {{-0.408248,-0.816497,0.408248}, {0,-0.92388,0.382683}, {0,-0.707107,0.707107}}, 
    {{-0.707107,0,0.707107}, {-0.816497,-0.408248,0.408248}, {-0.408248,-0.408248,0.816497}}, 
    {{-0.816497,-0.408248,0.408248}, {-0.707107,-0.707107,0}, {-0.408248,-0.816497,0.408248}}, 
    {{-0.408248,-0.408248,0.816497}, {-0.816497,-0.408248,0.408248}, {-0.408248,-0.816497,0.408248}}, 
    {{-0.408248,-0.408248,0.816497}, {-0.408248,-0.816497,0.408248}, {0,-0.707107,0.707107}}, 
    {{-0.707107,0,0.707107}, {-0.408248,-0.408248,0.816497}, {-0.382683,0,0.92388}}, 
    {{-0.408248,-0.408248,0.816497}, {0,-0.707107,0.707107}, {0,-0.382683,0.92388}}, 
    {{-0.382683,0,0.92388}, {-0.408248,-0.408248,0.816497}, {0,-0.382683,0.92388}}, 
    {{-0.382683,0,0.92388}, {0,-0.382683,0.92388}, {0,0,1}}, 
    {{0,-1,0}, {0.382683,-0.92388,0}, {0,-0.92388,0.382683}}, 
    {{0.382683,-0.92388,0}, {0.707107,-0.707107,0}, {0.408248,-0.816497,0.408248}}, 
    {{0,-0.92388,0.382683}, {0.382683,-0.92388,0}, {0.408248,-0.816497,0.408248}}, 
    {{0,-0.92388,0.382683}, {0.408248,-0.816497,0.408248}, {0,-0.707107,0.707107}}, 
    {{0.707107,-0.707107,0}, {0.92388,-0.382683,0}, {0.816497,-0.408248,0.408248}}, 
    {{0.92388,-0.382683,0}, {1,0,0}, {0.92388,0,0.382683}}, 
    {{0.816497,-0.408248,0.408248}, {0.92388,-0.382683,0}, {0.92388,0,0.382683}}, 
    {{0.816497,-0.408248,0.408248}, {0.92388,0,0.382683}, {0.707107,0,0.707107}}, 
    {{0,-0.707107,0.707107}, {0.408248,-0.816497,0.408248}, {0.408248,-0.408248,0.816497}}, 
    {{0.408248,-0.816497,0.408248}, {0.707107,-0.707107,0}, {0.816497,-0.408248,0.408248}}, 
    {{0.408248,-0.408248,0.816497}, {0.408248,-0.816497,0.408248}, {0.816497,-0.408248,0.408248}}, 
    {{0.408248,-0.408248,0.816497}, {0.816497,-0.408248,0.408248}, {0.707107,0,0.707107}}, 
    {{0,-0.707107,0.707107}, {0.408248,-0.408248,0.816497}, {0,-0.382683,0.92388}}, 
    {{0.408248,-0.408248,0.816497}, {0.707107,0,0.707107}, {0.382683,0,0.92388}}, 
    {{0,-0.382683,0.92388}, {0.408248,-0.408248,0.816497}, {0.382683,0,0.92388}}, 
    {{0,-0.382683,0.92388}, {0.382683,0,0.92388}, {0,0,1}}, 
    {{1,0,0}, {0.92388,0.382683,0}, {0.92388,0,-0.382683}}, 
    {{0.92388,0.382683,0}, {0.707107,0.707107,0}, {0.816497,0.408248,-0.408248}}, 
    {{0.92388,0,-0.382683}, {0.92388,0.382683,0}, {0.816497,0.408248,-0.408248}}, 
    {{0.92388,0,-0.382683}, {0.816497,0.408248,-0.408248}, {0.707107,0,-0.707107}}, 
    {{0.707107,0.707107,0}, {0.382683,0.92388,0}, {0.408248,0.816497,-0.408248}}, 
    {{0.382683,0.92388,0}, {0,1,0}, {0,0.92388,-0.382683}}, 
    {{0.408248,0.816497,-0.408248}, {0.382683,0.92388,0}, {0,0.92388,-0.382683}}, 
    {{0.408248,0.816497,-0.408248}, {0,0.92388,-0.382683}, {0,0.707107,-0.707107}}, 
    {{0.707107,0,-0.707107}, {0.816497,0.408248,-0.408248}, {0.408248,0.408248,-0.816497}}, 
    {{0.816497,0.408248,-0.408248}, {0.707107,0.707107,0}, {0.408248,0.816497,-0.408248}}, 
    {{0.408248,0.408248,-0.816497}, {0.816497,0.408248,-0.408248}, {0.408248,0.816497,-0.408248}}, 
    {{0.408248,0.408248,-0.816497}, {0.408248,0.816497,-0.408248}, {0,0.707107,-0.707107}}, 
    {{0.707107,0,-0.707107}, {0.408248,0.408248,-0.816497}, {0.382683,0,-0.92388}}, 
    {{0.408248,0.408248,-0.816497}, {0,0.707107,-0.707107}, {0,0.382683,-0.92388}}, 
    {{0.382683,0,-0.92388}, {0.408248,0.408248,-0.816497}, {0,0.382683,-0.92388}}, 
    {{0.382683,0,-0.92388}, {0,0.382683,-0.92388}, {0,0,-1}}, 
    {{0,1,0}, {-0.382683,0.92388,0}, {0,0.92388,-0.382683}}, 
    {{-0.382683,0.92388,0}, {-0.707107,0.707107,0}, {-0.408248,0.816497,-0.408248}}, 
    {{0,0.92388,-0.382683}, {-0.382683,0.92388,0}, {-0.408248,0.816497,-0.408248}}, 
    {{0,0.92388,-0.382683}, {-0.408248,0.816497,-0.408248}, {0,0.707107,-0.707107}}, 
    {{-0.707107,0.707107,0}, {-0.92388,0.382683,0}, {-0.816497,0.408248,-0.408248}}, 
    {{-0.92388,0.382683,0}, {-1,0,0}, {-0.92388,0,-0.382683}}, 
    {{-0.816497,0.408248,-0.408248}, {-0.92388,0.382683,0}, {-0.92388,0,-0.382683}}, 
    {{-0.816497,0.408248,-0.408248}, {-0.92388,0,-0.382683}, {-0.707107,0,-0.707107}}, 
    {{0,0.707107,-0.707107}, {-0.408248,0.816497,-0.408248}, {-0.408248,0.408248,-0.816497}}, 
    {{-0.408248,0.816497,-0.408248}, {-0.707107,0.707107,0}, {-0.816497,0.408248,-0.408248}}, 
    {{-0.408248,0.408248,-0.816497}, {-0.408248,0.816497,-0.408248}, {-0.816497,0.408248,-0.408248}}, 
    {{-0.408248,0.408248,-0.816497}, {-0.816497,0.408248,-0.408248}, {-0.707107,0,-0.707107}}, 
    {{0,0.707107,-0.707107}, {-0.408248,0.408248,-0.816497}, {0,0.382683,-0.92388}}, 
    {{-0.408248,0.408248,-0.816497}, {-0.707107,0,-0.707107}, {-0.382683,0,-0.92388}}, 
    {{0,0.382683,-0.92388}, {-0.408248,0.408248,-0.816497}, {-0.382683,0,-0.92388}}, 
    {{0,0.382683,-0.92388}, {-0.382683,0,-0.92388}, {0,0,-1}}, 
    {{-1,0,0}, {-0.92388,-0.382683,0}, {-0.92388,0,-0.382683}}, 
    {{-0.92388,-0.382683,0}, {-0.707107,-0.707107,0}, {-0.816497,-0.408248,-0.408248}}, 
    {{-0.92388,0,-0.382683}, {-0.92388,-0.382683,0}, {-0.816497,-0.408248,-0.408248}}, 
    {{-0.92388,0,-0.382683}, {-0.816497,-0.408248,-0.408248}, {-0.707107,0,-0.707107}}, 
    {{-0.707107,-0.707107,0}, {-0.382683,-0.92388,0}, {-0.408248,-0.816497,-0.408248}}, 
    {{-0.382683,-0.92388,0}, {0,-1,0}, {0,-0.92388,-0.382683}}, 
    {{-0.408248,-0.816497,-0.408248}, {-0.382683,-0.92388,0}, {0,-0.92388,-0.382683}}, 
    {{-0.408248,-0.816497,-0.408248}, {0,-0.92388,-0.382683}, {0,-0.707107,-0.707107}}, 
    {{-0.707107,0,-0.707107}, {-0.816497,-0.408248,-0.408248}, {-0.408248,-0.408248,-0.816497}}, 
    {{-0.816497,-0.408248,-0.408248}, {-0.707107,-0.707107,0}, {-0.408248,-0.816497,-0.408248}}, 
    {{-0.408248,-0.408248,-0.816497}, {-0.816497,-0.408248,-0.408248}, {-0.408248,-0.816497,-0.408248}}, 
    {{-0.408248,-0.408248,-0.816497}, {-0.408248,-0.816497,-0.408248}, {0,-0.707107,-0.707107}}, 
    {{-0.707107,0,-0.707107}, {-0.408248,-0.408248,-0.816497}, {-0.382683,0,-0.92388}}, 
    {{-0.408248,-0.408248,-0.816497}, {0,-0.707107,-0.707107}, {0,-0.382683,-0.92388}}, 
    {{-0.382683,0,-0.92388}, {-0.408248,-0.408248,-0.816497}, {0,-0.382683,-0.92388}}, 
    {{-0.382683,0,-0.92388}, {0,-0.382683,-0.92388}, {0,0,-1}}, 
    {{0,-1,0}, {0.382683,-0.92388,0}, {0,-0.92388,-0.382683}}, 
    {{0.382683,-0.92388,0}, {0.707107,-0.707107,0}, {0.408248,-0.816497,-0.408248}}, 
    {{0,-0.92388,-0.382683}, {0.382683,-0.92388,0}, {0.408248,-0.816497,-0.408248}}, 
    {{0,-0.92388,-0.382683}, {0.408248,-0.816497,-0.408248}, {0,-0.707107,-0.707107}}, 
    {{0.707107,-0.707107,0}, {0.92388,-0.382683,0}, {0.816497,-0.408248,-0.408248}}, 
    {{0.92388,-0.382683,0}, {1,0,0}, {0.92388,0,-0.382683}}, 
    {{0.816497,-0.408248,-0.408248}, {0.92388,-0.382683,0}, {0.92388,0,-0.382683}}, 
    {{0.816497,-0.408248,-0.408248}, {0.92388,0,-0.382683}, {0.707107,0,-0.707107}}, 
    {{0,-0.707107,-0.707107}, {0.408248,-0.816497,-0.408248}, {0.408248,-0.408248,-0.816497}}, 
    {{0.408248,-0.816497,-0.408248}, {0.707107,-0.707107,0}, {0.816497,-0.408248,-0.408248}}, 
    {{0.408248,-0.408248,-0.816497}, {0.408248,-0.816497,-0.408248}, {0.816497,-0.408248,-0.408248}}, 
    {{0.408248,-0.408248,-0.816497}, {0.816497,-0.408248,-0.408248}, {0.707107,0,-0.707107}}, 
    {{0,-0.707107,-0.707107}, {0.408248,-0.408248,-0.816497}, {0,-0.382683,-0.92388}}, 
    {{0.408248,-0.408248,-0.816497}, {0.707107,0,-0.707107}, {0.382683,0,-0.92388}}, 
    {{0,-0.382683,-0.92388}, {0.408248,-0.408248,-0.816497}, {0.382683,0,-0.92388}}, 
    {{0,-0.382683,-0.92388}, {0.382683,0,-0.92388}, {0,0,-1}}
};

static triangle_t *GetSphereTriangles(sphere_t *sphere, int *size){
    *size = NUM_SPHERE_TRIANGLES;
    triangle_t *result = (triangle_t*)calloc(*size, sizeof(triangle_t));
    for(int i = 0; i < *size; i++){
        result[i].p1 = (sphereTriangle[i].p1 * sphere->radius) + sphere->center;
        result[i].p2 = (sphereTriangle[i].p2 * sphere->radius) + sphere->center;
        result[i].p3 = (sphereTriangle[i].p3 * sphere->radius) + sphere->center;
    }

    return result;
}



static void RenderSceneObject(environment_t *env, viewport_t *viewport, scene_object_t *sceneObject, camera_t *camera) {
    switch(sceneObject->type){
        case OT_TRIANGLE: {
            v3_t projectedP1 = ProjectVertex(env, viewport, camera, sceneObject->triangle.p1);
            v3_t projectedP2 = ProjectVertex(env, viewport, camera, sceneObject->triangle.p2);
            v3_t projectedP3 = ProjectVertex(env, viewport, camera, sceneObject->triangle.p3);
            DrawWireframeTriangle(env, {projectedP1, projectedP2, projectedP3}, RgbToColor(0xff, 0x00, 0x00));
            break;
        }
        case OT_SPHERE: {
            int size = 0;
            triangle_t *sphereTriangles = GetSphereTriangles(&sceneObject->sphere, &size);
            for(int i = 0; i < size; i++){
                v3_t projectedP1 = ProjectVertex(env, viewport, camera, sphereTriangles[i].p1);
                v3_t projectedP2 = ProjectVertex(env, viewport, camera, sphereTriangles[i].p2);
                v3_t projectedP3 = ProjectVertex(env, viewport, camera, sphereTriangles[i].p3);
                DrawWireframeTriangle(env, {projectedP1, projectedP2, projectedP3}, RgbToColor(0xff, 0x00, 0x00));
            }
            break;
        }
    }
}

static bool
HandleKeyboard(environment_t *env, float *yaw, float *pitch, float *roll, v3_t *pos){
    bool result = false;
    event_t *event = ReadEvent(&env->events);

    while (event){
        if (event->type == ET_KEY_DOWN && event->value == 'w'){
            pos->y+= 0.1;
            result = true;
        } else if (event->type == ET_KEY_DOWN && event->value == 's'){
            pos->y-= 0.1;
            result = true;
        } else if (event->type == ET_KEY_DOWN && event->value == 'd'){
            pos->x+= 0.1;
            result = true;
        } else if (event->type == ET_KEY_DOWN && event->value == 'a'){
            pos->x-= 0.1;
            result = true;
        } else if (event->type == ET_KEY_DOWN && event->value == 'i'){
            pos->z+= 0.1;
            result = true;
        } else if (event->type == ET_KEY_DOWN && event->value == 'o'){
            pos->z-= 0.1;
            result = true;
        } else if (event->type == ET_KEY_DOWN && event->value == 'r'){
            (*roll)+=(M_PI_4/4);
            result = true;
        } else if (event->type == ET_KEY_DOWN && event->value == 'y'){
            (*yaw)+=(M_PI_4/4);
            result = true;
        } else if (event->type == ET_KEY_DOWN && event->value == 'p'){
            (*pitch)+=(M_PI_4/4);
            result = true;
        } else if (event->type == ET_KEY_DOWN && event->value == 'c'){
            *pos = {0, 0, 0};
            *yaw = 0; *pitch = 0; *roll = 0;
            result = true;
        } else if (event->type == ET_KEY_DOWN && event->value == 'm'){
            char s[1024];
            sprintf(s,"camera position: %f %f %f\n", pos->x, pos->y, pos->z);
            SDL_Log(s);
            result = true;
        }
        event = ReadEvent(&env->events);
     }
    return result;
}

static bool
HandleUpdates(environment_t *env, scene_t *scene){
    static bool changesMade = true;
    static float yaw = 0;
    static float pitch = 0;
    static float roll = 0;
    static v3_t cpos = {0, 0, -8};
    bitmapSettings_t *bitmap = env->bitmap;
    viewport_t vp = {1, 1, 1};
    v3_t origin = {0, 0, 0};

    changesMade = changesMade || HandleKeyboard(env, &yaw, &pitch, &roll, &scene->camera.position);

    if (!changesMade)
        return false;

    changesMade = false;
    
    scene->camera.rotation.data[0][0] = cos(yaw) * cos(pitch);
    scene->camera.rotation.data[0][1] = cos(yaw) * sin(pitch) * sin(roll) - sin(yaw) * cos(roll);
    scene->camera.rotation.data[0][2] = cos(yaw) * sin(pitch) * cos(roll) + sin(yaw) * sin(roll);
    scene->camera.rotation.data[1][0] = sin(yaw) * cos(pitch);
    scene->camera.rotation.data[1][1] = sin(yaw) * sin(pitch) * sin(roll) + cos(yaw) * cos(roll);
    scene->camera.rotation.data[1][2] = sin(yaw) * sin(pitch) * cos(roll) - cos(yaw) * sin(roll);
    scene->camera.rotation.data[2][0] = -sin(yaw);
    scene->camera.rotation.data[2][1] = cos(pitch) * sin(roll);
    scene->camera.rotation.data[2][2] = cos(pitch) * cos(roll);
    return true;

    // float height = bitmap->height/2; 
    // float yStart = -height;
    // float yStep  = bitmap->height/scene->settings.numberOfThreads;

//    for(int i=0; i < scene->settings.numberOfThreads; i++) {
//        // Generate unique data for each thread to work with.
//        displayPart[i]->yStart   = yStart;
//        displayPart[i]->yEnd     = yStart+yStep;
//        displayPart[i]->env      = env;
//        displayPart[i]->viewport = vp;
//        displayPart[i]->scene    = scene;
//        displayPart[i]->bvhState = bvhState;
//        displayPart[i]->status   = WS_READY;
//
//        yStart += yStep;
//
//        if (displayPart[i]->thread == NULL) {
//           exit(3);
//        }
//    }
}

void ClearBackground(environment_t *env){
    memset(env->bitmap->memory, 0, env->bitmap->width * env->bitmap->height * sizeof(uint32_t));
}

bool RasterizeScene(environment_t *env, scene_t *scene){
    viewport_t viewport = {1, 1, 1};

    //DrawLine(env, {0, 0, 0}, {100, 200, 0}, RgbToColor(0x11, 0xaa, 0xee));
    //DrawLine(env, {100, 200, 0}, {-100, -50, 0}, RgbToColor(0x11, 0xaa, 0x33));
    //DrawLine(env, {0, 200, 0}, {10, -50, 0}, RgbToColor(0xee, 0x11, 0x33));
    if (HandleUpdates(env, scene)){
        ClearBackground(env);
    }

    for(int i = 0; i < scene->objectStack.index; i++){
        scene_object_t workingObject = scene->objectStack.objects[i];
        switch(workingObject.type){
            case OT_SPHERE: {
                if (workingObject.sphere.radius < 2)
                    RenderSceneObject(env, &viewport, &workingObject, &scene->camera);
                //float t1, t2;
                //IntersectRaySphere(origin, direction, workingObject.sphere, &t1, &t2); 
                //if(tmin <= t1 && t1 <= tmax && t1 <= *tclosest){
                //    *tclosest = t1;
                //    *closestScene = workingObject;
                //    found = true;
                //}

                //if(tmin <= t2 && t2 <= tmax && t2 <= *tclosest){
                //    *tclosest = t2;
                //    *closestScene = workingObject;
                //    found = true;
                //}

                break; 
            } 
            case OT_TRIANGLE: {
                RenderSceneObject(env, &viewport, &workingObject, &scene->camera);
                //static int set = 0;
                //float t, u, v;
                //bool hit = IntersectRayTriangle(origin, direction, workingObject.triangle, &t, &u, &v);
                // if(hit && tmin <= t && t <= tmax && t <= *tclosest){
                //    *tclosest = t;
                //    *closestScene = workingObject;
                //    found = true;
                //}
                break;
            }
        }
    }

    return true;
}