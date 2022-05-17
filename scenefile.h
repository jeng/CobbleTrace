#ifndef __SCENEFILE_H__
#define __SCENEFILE_H__
#include <stdio.h>
#include <stdlib.h>
#include "mymath.h"
#include "SDL.h"

#define MAX_OBJECTS 100000
#define MAX_LIGHTS 100
#define MAX_SCENEFILE (1<<20)

enum lightType_t {LT_POINT, LT_DIRECTIONAL, LT_AMBIENT};
enum object_t {OT_SPHERE, OT_TRIANGLE};
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
};

struct scene_object_t {
    object_t type;
    union{
        sphere_t sphere;
        triangle_t triangle;
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

struct scene_t {
    scene_stack_t lightStack;
    scene_stack_t objectStack;
    camera_t camera;
    viewport_t viewport;
};

void ParseSceneFile(char *filename, scene_t *scene);
void InitSceneData(scene_t *scene);

#endif