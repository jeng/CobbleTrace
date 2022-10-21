// Cobble Trace - jeremy.english@gmail.com
//
// SDL wrapper for raythread.cpp

#include <stdio.h>
#include <stdbool.h>
#include "SDL.h"
#include "draw2d.h"
#include "raythread.h"
#include "scenefile.h"
#include "rasterizeScene.h"
#include "bvh.h"

#define WIDTH  (640)
#define HEIGHT (640)
#define EVENT_QUEUE_SIZE 1000

#define TEST_BVH (0)

void Help(){
    SDL_Log("cobbletrace scenefile\n\n");
    SDL_Log("To run cobbletrace provide a scene file.  See scene_file_1.json for an example.");
    exit(0);
}

int main(int argc, char *argv[]){

    scene_t scene;
    
    //TODO load the default scene if we don't get a file
    if (argc > 1){
        InitSceneData(&scene);
        ParseSceneFile(argv[1], &scene);
    } else {
        Help();
    }


    //https://www.libsdl.org/release/SDL-1.2.15/docs/html/sdlinit.html
    int result = SDL_Init(SDL_INIT_EVERYTHING);
    
    SDL_Window *window = SDL_CreateWindow("CobbleTrace", 
        SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED, 
        WIDTH, 
        HEIGHT, 
        SDL_WINDOW_RESIZABLE);

    if (window == NULL){
        //TODO This would need to be in the platform level and generate an error popup
        SDL_Log("Could not create window\n");
        exit(1);
    }
    SDL_SetWindowTitle(window, "Cobble Trace");
    
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    uint32_t *data = (uint32_t*)calloc(WIDTH * HEIGHT, sizeof(uint32_t));    
    if (data == NULL){
        SDL_Log("Failed to get memory for the buffer");
        exit(1);
    }

    environment_t env = {};
    bitmapSettings_t bitmap = {};
    bitmap.memory = data;
    bitmap.width = WIDTH;
    bitmap.height = HEIGHT;

    env.bitmap = &bitmap;
    env.clientHeight = HEIGHT * 2;
    env.clientWidth = WIDTH * 2;
    env.renderer = renderer;

    env.events.capacity = EVENT_QUEUE_SIZE;
    env.events.queue = (event_t*)calloc(env.events.capacity, sizeof(event_t));

#if TEST_BVH

    uint32_t startTicks = SDL_GetTicks();
    SDL_Log("Stating at %d", startTicks);
    BvhTest(&env, &scene);
    uint32_t endTicks = SDL_GetTicks();
    SDL_Log("Ending at %d", endTicks);

    uint32_t total = endTicks - startTicks;
    SDL_Log("Total time %dms %fs", total, total/1000.0);

#endif

    SDL_Event e;
    bool running = true;
    while(running){
        while(SDL_PollEvent(&e)){
            switch(e.type){
                case SDL_QUIT:
                    running = false;
                    break;

                //TODO I'm using my event queue since the engine I originally implemented the ray
                //     tracer in used that.  Since I'm using SDL here it is kinda of just duplicate
                //     code.  I'll probably remove it.

                case SDL_KEYDOWN:
                    AddEvent(&env.events, {ET_KEY_DOWN, EM_NONE, {0, 0}, (uint32_t)e.key.keysym.sym});
                    break;
            }
        }

#if TEST_BVH == 0

        if (scene.settings.wireframe)
            RasterizeScene(&env, &scene);
        else
            RayThread(&env, &scene);
#endif        

        Blit(&env);
        SDL_Delay(33);

    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
