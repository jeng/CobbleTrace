#define _USE_MATH_DEFINES // for C++
#include <cmath>
#include <assert.h>
#include <stdio.h>
#include "bvhTest.h"
#include "draw2d.h"
#include "color.h"
#include "scenefile.h"
#include "SDL.h"

#define NUM_TRIANGLES 64

v3_t
RandVertex(){
    v3_t point;
    point.x = (float)rand()/RAND_MAX;
    point.y = (float)rand()/RAND_MAX;
    point.z = (float)rand()/RAND_MAX;
    return point;
}

bool BvhTest(environment_t *env, scene_t *scene){
    triangle_t triangles[NUM_TRIANGLES];
    v3_t v = RandVertex();
    for(int i = 0; i < NUM_TRIANGLES; i++){
        v3_t r0 = RandVertex();
        v3_t r1 = RandVertex();
        v3_t r2 = RandVertex();
        v3_t v5 = {5.0f, 5.0f, 5.0f};
        triangles[i].p1 = r0 * 9 - v5;
        triangles[i].p2 = triangles[i].p1 + r1;
        triangles[i].p3 = triangles[i].p1 + r2;
        //char s[1024];
        //sprintf(s, "%f %f %f\n", v.x, v.y, v.z);
        //SDL_Log(s);
 
    }
    return true;
    //for(int i = 0; i < NUM_TRIANGLES; i++){
    //    v3_t v0 = {random}
    //}
}
