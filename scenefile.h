#ifndef __SCENEFILE_H__
#define __SCENEFILE_H__
#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"

inline void
ParseSceneFile(char *filename){
    char s[1024];
    sprintf(s, "Parsing: %s", filename);
    SDL_Log(s);
}
#endif