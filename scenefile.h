#ifndef __SCENEFILE_H__
#define __SCENEFILE_H__
#include <stdio.h>
#include <stdlib.h>
#include "mymath.h"
#include "SDL.h"
#include "ctstring.h"

#define MAX_OBJECTS 100000
#define MAX_LIGHTS 100
#define MAX_SCENEFILE (1<<22)

enum lightType_t {LT_POINT, LT_DIRECTIONAL, LT_AMBIENT};
enum object_t {OT_SPHERE, OT_TRIANGLE, OT_IMPORT};
enum import_type_t {IT_BLENDER, IT_PLY};
enum stack_type_t {ST_LIGHT, ST_OBJECT};


struct viewport_t {
    float width;
    float height;
    float d; //Distance from the camera to the viewport
};

struct material_t {
    uint32_t color;
    int specular;
    float reflection;
};

struct sphere_t {
    v3_t center;
    float radius;
};

struct triangle_t {
    v3_t p1;
    v3_t p2;
    v3_t p3;
    v3_t centroid;
};

struct import_t {
    String filename;
    v3_t position;
    v3_t rotation;
    v3_t scale;
};

struct scene_object_t {
    object_t type;
    union{
        sphere_t sphere;
        triangle_t triangle;
        import_t import;
    };
    material_t material;
};

struct light_t {
    lightType_t type;
    float intensity;
    v3_t position;
    v3_t direction;
};

struct camera_t {
    v3_t position;
    m3x3_t rotation;
};

struct scene_stack_t {
    int size;
    int index;
    stack_type_t type;
    union{
        light_t *lights;
        scene_object_t *objects;
    };
};

struct settings_t {
    int numberOfThreads;
    bool subsampling;
    bool wireframe;
    bool supersampling;
};

struct scene_triangle_lookup_t {
    uint32_t triangleCount;
    uint32_t *indexes;
};

struct scene_t {
    scene_stack_t lightStack;
    scene_stack_t objectStack;
    scene_triangle_lookup_t triangleLookup;
    camera_t camera;
    viewport_t viewport;
    settings_t settings;
};

struct ray_t {
    v3_t origin;
    v3_t direction;
    float t;
};

void ParseSceneFile(char *filename, scene_t *scene);
void InitSceneData(scene_t *scene);

#endif