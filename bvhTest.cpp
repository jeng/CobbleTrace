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

// This is the code from https://jacco.ompf2.com/2022/04/13/how-to-build-a-bvh-part-1-basics/
// with changes to work with cobbletrace


//TODO(JHE) why do I have multiple versions of this
// They're all doing things slightly different.  One has the origin at zero and
// the other is for the rasterizer

static void
CanvasPutPixel(bitmapSettings_t *bitmap, v2_t v2, uint32_t color){
    //We want to have -w/2 to w/2
    //and h/2 to -h/2 for the x and y axis
    v2_t v;
    v.x = v2.x;// + (bitmap->width/2);
    v.y = v2.y;//(bitmap->height/2) + -v2.y;
    PutPixel(bitmap, v, color);
}

v3_t
RandVertex(){
    v3_t point;
    point.x = (float)rand()/RAND_MAX;
    point.y = (float)rand()/RAND_MAX;
    point.z = (float)rand()/RAND_MAX;
    return point;
}

void IntersectTriangle(ray_t *ray, triangle_t *triangle) {
    v3_t edge1 = triangle->p2 - triangle->p1;
    v3_t edge2 = triangle->p3 - triangle->p1;
    v3_t h = CrossProduct( ray->direction, edge2 );
    float a = DotProduct( edge1, h );
    if (a > -0.0001f && a < 0.0001f) return; // ray parallel to triangle
    float f = 1 / a;
    v3_t s = ray->origin - triangle->p1;
    float u = f * DotProduct( s, h );
    if (u < 0 || u > 1) return;
    v3_t q = CrossProduct( s, edge1 );
    float v = f * DotProduct( ray->direction, q );
    if (v < 0 || u + v > 1) return;
    float t = f * DotProduct( edge2, q );
    if (t > 0.0001f) ray->t = min( ray->t, t );
}

bool BvhTest(environment_t *env, scene_t *scene){
    triangle_t triangles[NUM_TRIANGLES];
    static bool initialized = false;
    
    if (!initialized){
        for(int i = 0; i < NUM_TRIANGLES; i++){
            v3_t r0 = RandVertex();
            v3_t r1 = RandVertex();
            v3_t r2 = RandVertex();
            v3_t v5 = {5.0f, 5.0f, 5.0f};
            triangles[i].p1 = r0 * 9 - v5;
            triangles[i].p2 = triangles[i].p1 + r1;
            triangles[i].p3 = triangles[i].p1 + r2;
        }
        initialized = true;
    }

    v3_t camPos = { 0, 0, -18 };
    v3_t p0 = { -1, 1, -15 }; 
    v3_t p1 = { 1, 1, -15 };
    v3_t p2 = { -1, -1, -15 };

    ray_t ray;

    for (int y = 0; y < 640; y++){
        for (int x = 0; x < 640; x++){
            v3_t pixelPos = p0 + (p1 - p0) * (x / 640.0f) + (p2 - p0) * (y / 640.0f);
            ray.origin = camPos;
            ray.direction = Normalize( pixelPos - ray.origin );
            ray.t = 1e30f;   
            for( int i = 0; i < NUM_TRIANGLES; i++ ){
                IntersectTriangle( &ray, &triangles[i] );
                if (ray.t != 1e30f){
                    CanvasPutPixel(env->bitmap, {(float)x, (float)y}, RgbToColor(135, 105, 134));
                }
            }
        }
    }

    return true;
}
