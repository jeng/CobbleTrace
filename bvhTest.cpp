#define _USE_MATH_DEFINES // for C++
#include <cmath>
#include <assert.h>
#include <stdio.h>
#include "bvhTest.h"
#include "draw2d.h"
#include "color.h"
#include "scenefile.h"
#include "SDL.h"

#define NUM_TRIANGLES 256 

// This is the code from https://jacco.ompf2.com/2022/04/13/how-to-build-a-bvh-part-1-basics/
// with changes to work with cobbletrace

#define VERBOSE 0


struct bvh_node_t {
    v3_t aabbMin;
    v3_t aabbMax;
    uint32_t leftNode;
    uint32_t firstTriangleIndex;
    uint32_t triangleCount;
};

struct bvh_triangles_t{
    triangle_t data[NUM_TRIANGLES];
    uint32_t indexes[NUM_TRIANGLES];
};

bvh_node_t bvhNodes[NUM_TRIANGLES * 2 - 1];
uint32_t rootNodeIdx = 0;
uint32_t nodesUsed = 1;

void UpdateNodeBounds(uint32_t nodeIdx, bvh_triangles_t *triangles){
    bvh_node_t *node = &bvhNodes[nodeIdx];
    node->aabbMin = {1e30f, 1e30f, 1e30f};
    node->aabbMax = {-1e30f, -1e30f, -1e30f};//TODO Why not use float min and max for these?

    int first = node->firstTriangleIndex;
    for(int i = 0; i < node->triangleCount; i++){
        uint32_t leafIndex = triangles->indexes[first + i];
        triangle_t leafTriangle = triangles->data[leafIndex];

        node->aabbMin = MinV3(node->aabbMin, leafTriangle.p1);
        node->aabbMin = MinV3(node->aabbMin, leafTriangle.p2);
        node->aabbMin = MinV3(node->aabbMin, leafTriangle.p3);

        node->aabbMax = MaxV3(node->aabbMax, leafTriangle.p1);
        node->aabbMax = MaxV3(node->aabbMax, leafTriangle.p2);
        node->aabbMax = MaxV3(node->aabbMax, leafTriangle.p3);
    }
}

void Subdivide(uint32_t nodeIdx, bvh_triangles_t *triangles){
    bvh_node_t *node = &bvhNodes[nodeIdx];
    if (node->triangleCount <= 2){
        return;
    }

    //determine split axis and position
    v3_t extent = node->aabbMax - node->aabbMin;
    int axis = 0;
    if (extent.y > extent.x)
        axis = 1;

    if (extent.z > V3ByIndex(extent, axis))
        axis = 2;

    float splitPos = V3ByIndex(node->aabbMin, axis) + V3ByIndex(extent, axis) * 0.5f;

    //in-place partition
    int i = node->firstTriangleIndex;
    int j = i + node->triangleCount - 1;
    while(i <= j){
        if (V3ByIndex(triangles->data[triangles->indexes[i]].centroid, axis) < splitPos){
            i++;
        }
        else {
            uint32_t t = triangles->indexes[i];
            triangles->indexes[i] = triangles->indexes[j];
            triangles->indexes[j--] = t;
        }
    }

    //about split if one of the sides is empty
    int leftCount = i - node->firstTriangleIndex;
    if (leftCount == 0 || leftCount == node->triangleCount)
        return;

    //create child nodes
    int leftChildIdx = nodesUsed++;
    int rightChildIdx = nodesUsed++;
    node->leftNode = leftChildIdx;

    bvhNodes[leftChildIdx].firstTriangleIndex = node->firstTriangleIndex;
    bvhNodes[leftChildIdx].triangleCount = leftCount;

    bvhNodes[rightChildIdx].firstTriangleIndex = i;
    bvhNodes[rightChildIdx].triangleCount = node->triangleCount - leftCount;

    node->triangleCount = 0;
    UpdateNodeBounds(leftChildIdx, triangles);
    UpdateNodeBounds(rightChildIdx, triangles);

    //recurse
    Subdivide(leftChildIdx, triangles);
    Subdivide(rightChildIdx, triangles);
}

void BuildBVH(bvh_triangles_t *triangles){
    for(int i = 0; i < NUM_TRIANGLES; i++){
        triangles->data[i].centroid = (triangles->data[i].p1 + triangles->data[i].p2 + triangles->data[i].p3) * 0.3333f;        
    }

    bvh_node_t *root = &bvhNodes[rootNodeIdx];
    root->leftNode = 0;
    root->firstTriangleIndex = 0;
    root->triangleCount = NUM_TRIANGLES;
    UpdateNodeBounds(rootNodeIdx, triangles);
    Subdivide(rootNodeIdx, triangles);
}


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

bool IntersectAABB( ray_t *ray, v3_t bmin, v3_t bmax ) {
    float tx1 = (bmin.x - ray->origin.x) / ray->direction.x;
    float tx2 = (bmax.x - ray->origin.x) / ray->direction.x;
    float tmin = min( tx1, tx2 ); 
    float tmax = max( tx1, tx2 );
    float ty1 = (bmin.y - ray->origin.y) / ray->direction.y; 
    float ty2 = (bmax.y - ray->origin.y) / ray->direction.y;
    tmin = max( tmin, min( ty1, ty2 ) );
    tmax = min( tmax, max( ty1, ty2 ) );
    float tz1 = (bmin.z - ray->origin.z) / ray->direction.z; 
    float tz2 = (bmax.z - ray->origin.z) / ray->direction.z;
    tmin = max( tmin, min( tz1, tz2 ) );
    tmax = min( tmax, max( tz1, tz2 ) );
    return tmax >= tmin && tmin < ray->t && tmax > 0;
}

void IntersectBVH(ray_t *ray, uint32_t nodeIdx, bvh_triangles_t *triangles){
    bvh_node_t *node = &bvhNodes[nodeIdx];

    if (!IntersectAABB(ray, node->aabbMin, node->aabbMax)){
        return;
    }

    if (node->triangleCount > 0){
        for(int i = 0; i < node->triangleCount; i++)
            IntersectTriangle(ray, &triangles->data[triangles->indexes[node->firstTriangleIndex + i]]);
    } else {
        IntersectBVH(ray, node->leftNode, triangles);
        IntersectBVH(ray, node->leftNode + 1, triangles);
    }

}

bool BvhTest(environment_t *env, scene_t *scene){
    bvh_triangles_t triangles;
    static bool initialized = false;
    
    if (!initialized){
        for(int i = 0; i < NUM_TRIANGLES; i++){
            v3_t r0 = RandVertex();
            v3_t r1 = RandVertex();
            v3_t r2 = RandVertex();
            v3_t v5 = {5.0f, 5.0f, 5.0f};
            triangles.data[i].p1 = r0 * 9 - v5;
            triangles.data[i].p2 = triangles.data[i].p1 + r1;
            triangles.data[i].p3 = triangles.data[i].p1 + r2;
            triangles.indexes[i] = i;
#if VERBOSE
            SDL_Log("Triangle %.2d (p1): %f %f %f", i, triangles.data[i].p1.x, triangles.data[i].p1.y, triangles.data[i].p1.z);
            SDL_Log("Triangle %.2d (p2): %f %f %f", i, triangles.data[i].p2.x, triangles.data[i].p2.y, triangles.data[i].p2.z);
            SDL_Log("Triangle %.2d (p3): %f %f %f", i, triangles.data[i].p3.x, triangles.data[i].p3.y, triangles.data[i].p3.z);
#endif        
        }
        initialized = true;
    }

    uint32_t startTicks = SDL_GetTicks();
    SDL_Log("Build Stating at %d", startTicks);
    BuildBVH(&triangles);
    uint32_t endTicks = SDL_GetTicks();
    SDL_Log("Build Ending at %d", endTicks);

    uint32_t total = endTicks - startTicks;
    SDL_Log("Build Total time %dms %fs", total, total/1000.0);
    SDL_Log("Nodes used %d", nodesUsed);
    
#if VERBOSE    
    for(int i = 0; i < nodesUsed; i++){
        SDL_Log("Node size min %f %f %f max %f %f %f", 
            bvhNodes[i].aabbMin.x, bvhNodes[i].aabbMin.y, bvhNodes[i].aabbMin.z, 
            bvhNodes[i].aabbMax.x, bvhNodes[i].aabbMax.y, bvhNodes[i].aabbMax.z);
        SDL_Log("Triangle Count %d", bvhNodes[i].triangleCount);
        SDL_Log("First triangle Index %d", bvhNodes[i].firstTriangleIndex);
        SDL_Log("Left Node %d", bvhNodes[i].leftNode);
        SDL_Log("----------------------------------------");
    }
#endif    

    v3_t camPos = { 0, 0, -18 };
    v3_t p0 = { -1, 1, -15 }; 
    v3_t p1 = { 1, 1, -15 };
    v3_t p2 = { -1, -1, -15 };

    ray_t ray;

    startTicks = SDL_GetTicks();
    SDL_Log("Walk Stating at %d", startTicks);
    

    for (int y = 0; y < 640; y++){
        for (int x = 0; x < 640; x++){
            v3_t pixelPos = p0 + (p1 - p0) * (x / 640.0f) + (p2 - p0) * (y / 640.0f);
            ray.origin = camPos;
            ray.direction = Normalize( pixelPos - ray.origin );
            ray.t = 1e30f;   
#if 0            
            for( int i = 0; i < NUM_TRIANGLES; i++ ){
                IntersectTriangle( &ray, &triangles.data[i] );
                if (ray.t != 1e30f){
                    CanvasPutPixel(env->bitmap, {(float)x, (float)y}, RgbToColor(135, 105, 134));
                }
            }
#else
            IntersectBVH(&ray, rootNodeIdx, &triangles);
            if (ray.t != 1e30f){
                CanvasPutPixel(env->bitmap, {(float)x, (float)y}, RgbToColor(135, 105, 134));
            }
#endif
        }
    }

    endTicks = SDL_GetTicks();
    SDL_Log("Walk ending at %d", endTicks);

    total = endTicks - startTicks;
    SDL_Log("Walk total time %dms %fs", total, total/1000.0);

    return true;
}
