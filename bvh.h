#ifndef __BVH_TEST_H__
#define __BVH_TEST_H__
#include "environment.h"
#include "scenefile.h"
struct bvh_node_t {
    v3_t aabbMin;
    v3_t aabbMax;
    uint32_t leftNode;
    uint32_t firstTriangleIndex;
    uint32_t triangleCount;
};

struct bvh_triangles_t{
    uint32_t size;
    triangle_t *data;
    uint32_t *indexes;
};

struct bvh_state_t{
    //bvh_node_t bvhNodes[NUM_TRIANGLES * 2 - 1];
    bvh_node_t *bvhNodes;
    uint32_t rootNodeIdx = 0;
    uint32_t nodesUsed = 1;
    bvh_triangles_t triangles;
};


bool BvhTest(environment_t *env, scene_t *scene);
bvh_state_t InitializeBVHState(triangle_t *triangles, int size);
void BuildBVH(bvh_state_t *bvhState);
void IntersectBVH(ray_t *ray, uint32_t nodeIdx, bvh_state_t *bvhState);
void IntersectBVHClosest(ray_t *ray, uint32_t nodeIdx, bvh_state_t *bvhState, float *tclosest, uint32_t *closestIndex);


#endif