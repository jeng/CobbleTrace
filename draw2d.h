#ifndef __DRAW_2D_H__
#define __DRAW_2D_H__
#include <cstdint>
#include "environment.h"
#include "mymath.h"
#include "SDL.h"

inline void
PutPixel(bitmapSettings_t *bitmapSetting, v2_t v, uint32_t color) {

    if (v.y < 0 || v.y >= bitmapSetting->height)
        return;
    if (v.x < 0 || v.x >= bitmapSetting->width)
        return;

    uint32_t *pixel = (uint32_t*)bitmapSetting->memory;
    pixel += ((int)v.y * bitmapSetting->width);
    pixel += (int)v.x;
    *pixel = color;
}

inline void
Blit(environment_t *environment){
    SDL_Renderer *renderer = environment->renderer;
    uint32_t *buffer = (uint32_t*)environment->bitmap->memory;
    Uint32 rmask, gmask, bmask, amask;
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
    int shift = (req_format == STBI_rgb) ? 8 : 0;
    rmask = 0xff000000 >> shift;
    gmask = 0x00ff0000 >> shift;
    bmask = 0x0000ff00 >> shift;
    amask = 0x000000ff >> shift;
    #else // little endian, like x86
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
    #endif

    SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(buffer, 
        environment->bitmap->width, 
        environment->bitmap->height, 
        32,
        4*environment->bitmap->width, 
        rmask, gmask, bmask, 0);//amask);

    if (surface == NULL){
        SDL_Log("Creating surface failed: %s", SDL_GetError());
        exit(1);        
    }

    SDL_Rect dstrect;

    dstrect.x = 0;
    dstrect.y = 0;
    dstrect.w = environment->bitmap->width;
    dstrect.h = environment->bitmap->height;

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_RenderCopy(renderer, texture, NULL, &dstrect);
    SDL_RenderPresent(renderer);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}
#endif