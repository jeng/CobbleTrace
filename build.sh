#!/bin/bash
g++ cobbletrace.cpp raythread.cpp eventQueue.cpp scenefile.cpp rasterizeScene.cpp bvh.cpp -lm -lSDL2 -I /usr/include/SDL2 -o cobbletrace
