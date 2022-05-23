#!/bin/bash
g++ cobbletrace.cpp raythread.cpp eventQueue.cpp scenefile.cpp -lm -lSDL2 -I /usr/include/SDL2 -o cobbletrace
