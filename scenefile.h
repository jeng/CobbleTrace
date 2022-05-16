#ifndef __SCENEFILE_H__
#define __SCENEFILE_H__
#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"

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

enum lightType_t {LT_POINT, LT_DIRECTIONAL, LT_AMBIENT};
enum object_t {OT_SPHERE, OT_TRIANGLE};

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

inline void
ParseSceneFile(char *filename){
    char s[1024];
    sprintf(s, "Parsing: %s", filename);
    SDL_Log(s);
}
#endif