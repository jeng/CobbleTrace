set SDL_LIB=%USERPROFILE%\src\CobbleTrace\SDL\SDL2-devel-2.0.22-VC\SDL2-2.0.22\lib\x64
set SDL_INC=%USERPROFILE%\src\CobbleTrace\SDL\SDL2-devel-2.0.22-VC\SDL2-2.0.22\include
set /A debug = 0

if %debug%==1 (
    echo "debug build"
cl -Zi cobbletrace.cpp raythread.cpp eventQueue.cpp scenefile.cpp rasterizeScene.cpp bvh.cpp objectLoader.cpp ctstring.cpp fileBuffer.cpp %SDL_LIB%\SDL2.lib %SDL_LIB%\SDL2main.lib -I %SDL_INC% shell32.lib /link %SDL_LIB%\SDL2.lib %SDL_LIB%\SDL2main.lib /SUBSYSTEM:WINDOWS
) else (
    echo "production build"
cl -O2 cobbletrace.cpp raythread.cpp eventQueue.cpp scenefile.cpp rasterizeScene.cpp bvh.cpp objectLoader.cpp ctstring.cpp fileBuffer.cpp %SDL_LIB%\SDL2.lib %SDL_LIB%\SDL2main.lib -I %SDL_INC% shell32.lib /link %SDL_LIB%\SDL2.lib %SDL_LIB%\SDL2main.lib /SUBSYSTEM:WINDOWS
)
