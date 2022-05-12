#ifndef __ENVIRONMENT_H__
#define __ENVIRONMENT_H__

#include "eventQueue.h"
#include "SDL.h"

struct bitmapSettings_t {
    void *memory;
    int width;
    int height;
    float *depthBuffer;
};

struct environment_t {
    SDL_Renderer *renderer;
    bitmapSettings_t *bitmap;
    eventManager_t events;
    int clientWidth;
    int clientHeight;
};

#endif