#define _USE_MATH_DEFINES // for C++
#include <cmath>
#include <assert.h>
#include <stdio.h>
#include "bvh.h"
#include "draw2d.h"
#include "color.h"
#include "scenefile.h"
#include "SDL.h"

// This is the code from https://jacco.ompf2.com/2022/04/13/how-to-build-a-bvh-part-1-basics/
// with changes to work with cobbletrace

#define VERBOSE 0

bvh_state_t InitializeBVHState(triangle_t *triangles, int size){
    bvh_state_t bvhState;
    bvhState.bvhNodes = (bvh_node_t*)calloc(size * 2 - 1, sizeof(bvh_node_t));
    bvhState.nodesUsed = 1;
    bvhState.rootNodeIdx = 0;
    bvhState.triangles.size = size;
    bvhState.triangles.data = triangles;
    bvhState.triangles.indexes = (uint32_t*)calloc(size, sizeof(uint32_t));
    for(int i = 0; i < size; i++){
        bvhState.triangles.indexes[i] = i;
    }
    return bvhState;
}

void UpdateNodeBounds(uint32_t nodeIdx, bvh_state_t *bvhState){
    bvh_triangles_t *triangles = &bvhState->triangles;
    bvh_node_t *node = &bvhState->bvhNodes[nodeIdx];
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

void Subdivide(uint32_t nodeIdx, bvh_state_t *bvhState){
    bvh_triangles_t *triangles = &bvhState->triangles;
    bvh_node_t *node = &bvhState->bvhNodes[nodeIdx];
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
    int leftChildIdx = bvhState->nodesUsed++;
    int rightChildIdx = bvhState->nodesUsed++;
    node->leftNode = leftChildIdx;

    bvhState->bvhNodes[leftChildIdx].firstTriangleIndex = node->firstTriangleIndex;
    bvhState->bvhNodes[leftChildIdx].triangleCount = leftCount;

    bvhState->bvhNodes[rightChildIdx].firstTriangleIndex = i;
    bvhState->bvhNodes[rightChildIdx].triangleCount = node->triangleCount - leftCount;

    node->triangleCount = 0;
    UpdateNodeBounds(leftChildIdx, bvhState);
    UpdateNodeBounds(rightChildIdx, bvhState);

    //recurse
    Subdivide(leftChildIdx, bvhState);
    Subdivide(rightChildIdx, bvhState);
}

void BuildBVH(bvh_state_t *bvhState){
    bvh_triangles_t *triangles = &bvhState->triangles;

    for(int i = 0; i < bvhState->triangles.size; i++){
        triangles->data[i].centroid = (triangles->data[i].p1 + triangles->data[i].p2 + triangles->data[i].p3) * 0.3333f;        
    }

    bvh_node_t *root = &bvhState->bvhNodes[bvhState->rootNodeIdx];
    root->leftNode = 0;
    root->firstTriangleIndex = 0;
    root->triangleCount = bvhState->triangles.size;
    UpdateNodeBounds(bvhState->rootNodeIdx, bvhState);
    Subdivide(bvhState->rootNodeIdx, bvhState);
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

bool IntersectTriangle(ray_t *ray, triangle_t *triangle) {
    v3_t edge1 = triangle->p2 - triangle->p1;
    v3_t edge2 = triangle->p3 - triangle->p1;
    v3_t h = CrossProduct( ray->direction, edge2 );
    float a = DotProduct( edge1, h );
    if (a > -0.0001f && a < 0.0001f) return false; // ray parallel to triangle
    float f = 1 / a;
    v3_t s = ray->origin - triangle->p1;
    float u = f * DotProduct( s, h );
    if (u < 0 || u > 1) return false;
    v3_t q = CrossProduct( s, edge1 );
    float v = f * DotProduct( ray->direction, q );
    if (v < 0 || u + v > 1) return false;
    float t = f * DotProduct( edge2, q );
    if (t > 0.0001f) ray->t = min( ray->t, t );
    return true;
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

void IntersectBVH(ray_t *ray, uint32_t nodeIdx, bvh_state_t *bvhState){
    bvh_triangles_t *triangles = &bvhState->triangles;
    bvh_node_t *node = &bvhState->bvhNodes[nodeIdx];

    if (!IntersectAABB(ray, node->aabbMin, node->aabbMax)){
        return;
    }

    if (node->triangleCount > 0){
        for(int i = 0; i < node->triangleCount; i++)
            IntersectTriangle(ray, &triangles->data[triangles->indexes[node->firstTriangleIndex + i]]);
    } else {
        IntersectBVH(ray, node->leftNode, bvhState);
        IntersectBVH(ray, node->leftNode + 1, bvhState);
    }
}

void IntersectBVHClosest(ray_t *ray, uint32_t nodeIdx, bvh_state_t *bvhState, float *tclosest, uint32_t *closestIndex){
    bvh_triangles_t *triangles = &bvhState->triangles;
    bvh_node_t *node = &bvhState->bvhNodes[nodeIdx];

    //*tclosest = 1e30f;
    //*closestIndex = 0;

    if (!IntersectAABB(ray, node->aabbMin, node->aabbMax)){
        return;
    }

    if (node->triangleCount > 0){
        for(int i = 0; i < node->triangleCount; i++){
            bool hit = IntersectTriangle(ray, &triangles->data[triangles->indexes[node->firstTriangleIndex + i]]);
            if (hit && ray->t != 1e30f && ray->t < *tclosest){
                *closestIndex = triangles->indexes[node->firstTriangleIndex + i];
                *tclosest = ray->t;
            }
        }
    } else {
        IntersectBVHClosest(ray, node->leftNode, bvhState, tclosest, closestIndex);
        IntersectBVHClosest(ray, node->leftNode + 1, bvhState, tclosest, closestIndex);
    }

}
bool BvhTest(environment_t *env, scene_t *scene){
    const int NUM_TRIANGLES = 256;
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
#if VERBOSE
            SDL_Log("Triangle %.2d (p1): %f %f %f", i, triangles[i].p1.x, triangles[i].p1.y, triangles[i].p1.z);
            SDL_Log("Triangle %.2d (p2): %f %f %f", i, triangles[i].p2.x, triangles[i].p2.y, triangles[i].p2.z);
            SDL_Log("Triangle %.2d (p3): %f %f %f", i, triangles[i].p3.x, triangles[i].p3.y, triangles[i].p3.z);
#endif        
        }
        initialized = true;
    }

    bvh_state_t state = InitializeBVHState(&triangles[0], NUM_TRIANGLES);

    uint32_t startTicks = SDL_GetTicks();
    SDL_Log("Build Stating at %d", startTicks);
    BuildBVH(&state);
    uint32_t endTicks = SDL_GetTicks();
    SDL_Log("Build Ending at %d", endTicks);

    uint32_t total = endTicks - startTicks;
    SDL_Log("Build Total time %dms %fs", total, total/1000.0);
    SDL_Log("Nodes used %d", state.nodesUsed);
    
#if VERBOSE    
    for(int i = 0; i < state.nodesUsed; i++){
        SDL_Log("Node size min %f %f %f max %f %f %f", 
            state.bvhNodes[i].aabbMin.x, state.bvhNodes[i].aabbMin.y, state.bvhNodes[i].aabbMin.z, 
            state.bvhNodes[i].aabbMax.x, state.bvhNodes[i].aabbMax.y, state.bvhNodes[i].aabbMax.z);
        SDL_Log("Triangle Count %d", state.bvhNodes[i].triangleCount);
        SDL_Log("First triangle Index %d", state.bvhNodes[i].firstTriangleIndex);
        SDL_Log("Left Node %d", state.bvhNodes[i].leftNode);
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
            IntersectBVH(&ray, state.rootNodeIdx, &state);
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
