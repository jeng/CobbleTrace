// Cobble Trace - jeremy.english@gmail.com
//
// This is primarily port of the raytracer from "Computer Graphics from Scratch" to C.
// https://www.gabrielgambetta.com/computer-graphics-from-scratch/
//
// It is named Cobble Trace since I cobbled together information form a few 
// different locations:
//
// Threading code is basically Boss/Worker from:
// http://birrell.org/andrew/papers/035-Threads.pdf
//
// Triangle code was based on Glassner's Ray Tracing book, Scratch A Pixel and UC Davis Ray Tracing lectures
// https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/barycentric-coordinates
// https://www.youtube.com/watch?v=Ahp6LDQnK4Y
// https://archive.org/details/AnIntroductionToRayTracing_201902


// I still have a lot of things I would like to add:
//
// DONE - Make a true boss worker, provide a status bar, do not hang the UI
// TODO - Optimize by tracing every other pixel and averaging the one between
// TODO - Add memoization after doing some profiling
// DONE - allow for different size bitmaps and blit to screen
// DONE - Use previous image when a change has not been made to the scene
// TODO - Copy the object loading code from software renderer
// TODO - Allow for scene files to be opened
// TODO - Allow for object files to be specified in the scene
// TODO - Add refraction
// TODO - Add depth of field
    // https://drive.google.com/drive/folders/1--zhaSmXdVpPeV0ZhZJq-Qw5QCjUuStc
    // And other lenses effect fisheye, motion blur, etc.
// TODO - Recreate my 3D automaton povray scene
// TODO - Allow for saving scenes as PNG
// TODO - Allow subpixel rendering
// TODO - Allow for specifying images larger than the screen
// DONE - Create a standalone version
//  1) Just break out this code so you can upload something
//  2) Make SDL version that it can run on linux too
// TODO - test compiling on linux
// TODO - add a gui
// TODO - Create a distributed version I can run on the raspberry pi cluster
//  1) this is one of the articles I had on my todo list
// TODO - Fix camera rotation
// TODO - SDL is the platform layer here so I can remove eventQueue code

#define _USE_MATH_DEFINES // for C++
#include <cmath>
#include <assert.h>
#include <stdio.h>
#include "raythread.h"
#include "draw2d.h"
#include "color.h"
#include "scenefile.h"
#include "SDL.h"

const float FINF = 4294967296.0;
const uint32_t BACKGROUND_COLOR = RgbToColor(0x33, 0x33, 0x33);
const int MAX_THREADS = 16;


enum worker_status_t {
    WS_READY,
    WS_RUNNING,
    WS_FINISHED
};

struct display_partition_t{
    int yStart;
    int yEnd;
    environment_t *env;
    viewport_t viewport;
    camera_t camera;
    worker_status_t status;
    SDL_Thread *thread;
};

static display_partition_t *displayPart[MAX_THREADS];


// This is the ray intersect method from Glassner's Ray Tracing book(pg 50) and Scratch A Pixel
//
// It is using the method of solving for t to see when I point hits the plane
// based on reformulating with equation of a plane
//    
// It then uses barycentric coordinates to determine if we are inside or out
// side of the triangle

static bool
IntersectRayTriangle(v3_t origin, v3_t direction, triangle_t triangle, float *t, float *u, float *v){
    float epsilon = 0.000001;
    v3_t v0v1 = triangle.p2 - triangle.p1;
    v3_t v0v2 = triangle.p3 - triangle.p1;
    v3_t n = CrossProduct(v0v1, v0v2);  //Get the normal of the plane
    float denom = DotProduct(n,n); //The dot of a vector with itself is the magnitude of the vector squared.

    //If the plane and the ray are parallel then either the entire ray is in the plane or the ray
    //will never hit the plane.  Think of it like this:
    //
    // -------------->  (Ray)
    //
    // ===============> (plane)

    float NDotRayDir = DotProduct(n,direction);
    if (fabs(NDotRayDir) < epsilon)
        return false;

    //Using the equation of a plane.  See Glassner's
    float d = -1 * DotProduct(n, triangle.p1);

    //Compute T, this is the point where the ray hits the plane
    //This video from UC Davis explains this equation
    //https://www.youtube.com/watch?v=Ahp6LDQnK4Y  @ 16:14
    *t = -(d + DotProduct(n, origin)) / NDotRayDir;

    //If the triangle is behind
    if (t < 0) 
        return false;

    //This is just the standard find a point along the ray
    //Since we solved for t we can get the point
    v3_t p = origin + *t * direction;

    //Inside out test
    v3_t c; //vector perpendicular to the triangles plane  (Would this be the normal from above?)

    //Edge 0
    v3_t edge0 = triangle.p2 - triangle.p1;
    v3_t vp0 = p - triangle.p1;
    c = CrossProduct(edge0, vp0);
    if (DotProduct(n, c) < 0)
        return false; //P is on the right side?

    //Edge 1
    v3_t edge1 = triangle.p3 - triangle.p2;
    v3_t vp1 = p - triangle.p2;
    c = CrossProduct(edge1, vp1);
    *u = DotProduct(n, c);
    if (*u < 0)
        return false; //P is on the right side?

    //Edge 2
    v3_t edge2 = triangle.p1 - triangle.p3;
    v3_t vp2 = p - triangle.p3;
    c = CrossProduct(edge2, vp2);
    *v = DotProduct(n, c);
    if (*v < 0)
            return false; //P is on the right side?

    *u /= denom;
    *v /= denom;

    return true;
}
 
static void
IntersectRaySphere(v3_t origin, v3_t direction, sphere_t sphere, float *t1, float *t2){
    float r = sphere.radius;
    v3_t co = origin - sphere.center;
    float a = DotProduct(direction, direction);
    float b = 2 * DotProduct(co, direction);
    float c = DotProduct(co, co) - (r * r);
    float discriminant = b*b - 4*a*c;

    if (discriminant < 0){
        *t1 = FINF;
        *t2 = FINF;
    }

    *t1 = (-b + sqrt(discriminant)) / (2 * a);
    *t2 = (-b - sqrt(discriminant)) / (2 * a);
}

static void
CanvasPutPixel(bitmapSettings_t *bitmap, v2_t v2, uint32_t color){
    //We want to have -w/2 to w/2
    //and h/2 to -h/2 for the x and y axis
    v2_t v;
    v.x = v2.x + (bitmap->width/2);
    v.y = (bitmap->height/2) + -v2.y;
    PutPixel(bitmap, v, color);
}

static v3_t
CanvasToViewport(bitmapSettings_t *bitmap, viewport_t viewport, v2_t position){
    //Keep it square
    float width = bitmap->height;
    float height = bitmap->height;
    return {position.x * (viewport.width/width), 
            position.y * (viewport.height/height), 
            viewport.d};
}

//TODO too many parameters.  Clean this up
static bool
ClosestIntersection(v3_t origin, v3_t direction, 
    float tmin, float tmax, 
    float *tclosest, scene_object_t *closestScene){

    *tclosest = FINF;

    //TODO build environment from description language
    //Beginning of environment
    const int sceneLength = 8;
    scene_object_t scene[sceneLength];

    scene[0].type = OT_SPHERE;
    scene[0].sphere.center = {0, -1, 3};
    scene[0].sphere.radius = 1;
    scene[0].material.color = RgbToColor(0xff, 0x00, 0x00);
    scene[0].material.specular = 500;//250 + (250 * cos(theta+=0.1));//500; //shiny
    scene[0].material.reflection = 0.2;//0 to 1

    scene[1].type = OT_SPHERE;
    scene[1].sphere.center = {2, 0, 4};
    scene[1].sphere.radius = 1;
    scene[1].material.color = RgbToColor(0x00, 0xff, 0x00);
    scene[1].material.specular = 500; //shiny
    scene[1].material.reflection = 0.3;//0 to 1
    
    scene[2].type = OT_SPHERE;
    scene[2].sphere.center = {-2, 0, 4};
    scene[2].sphere.radius = 1;
    scene[2].material.color = RgbToColor(0xff, 0x00, 0xff);
    scene[2].material.specular = 10; //somewhat shiny
    scene[2].material.reflection = 0.4;//0 to 1

    scene[3].type = OT_SPHERE;
    scene[3].sphere.center = {0, -5001, 0};
    scene[3].sphere.radius = 5000;
    scene[3].material.color = RgbToColor(0xff, 0xff, 0x00);
    scene[3].material.specular = 1000; //very shiny
    scene[3].material.reflection = 0.5;//0 to 1

    scene[4].type = OT_SPHERE;
    scene[4].sphere.center = {-1, 2, 3};
    scene[4].sphere.radius = 0.10;
    scene[4].material.color = RgbToColor(0x55, 0x00, 0x99);
    scene[4].material.specular = 100; //somewhat shiny
    scene[4].material.reflection = 0;//0 to 1

    scene[5].type = OT_SPHERE;
    scene[5].sphere.center = {0, 2.25, -2};
    scene[5].sphere.radius = 0.10;
    scene[5].material.color = RgbToColor(0x55, 0x77, 0x00);
    scene[5].material.specular = 100; //somewhat shiny
    scene[5].material.reflection = 0;//0 to 1

    scene[6].type = OT_SPHERE;
    scene[6].sphere.center = {1, 2, 1};
    scene[6].sphere.radius = 0.10;
    scene[6].material.color = RgbToColor(0x55, 0x77, 0x99);
    scene[6].material.specular = 100; //somewhat shiny
    scene[6].material.reflection = 0;//0 to 1

    scene[7].type = OT_TRIANGLE;
    scene[7].triangle.p1 = scene[4].sphere.center;
    scene[7].triangle.p2 = scene[5].sphere.center;
    scene[7].triangle.p3 = scene[6].sphere.center;
    scene[7].material.color = RgbToColor(0xff, 0x00, 0x00);
    scene[7].material.specular = 100; //somewhat shiny
    scene[7].material.reflection = 0.25;//0 to 1
 
   //End of environment

    bool found = false;
    scene_object_t workingScene;
    for(int i = 0; i < sceneLength; i++){
        workingScene = scene[i];
        switch(scene[i].type){
            case OT_SPHERE: {
                float t1, t2;
                IntersectRaySphere(origin, direction, workingScene.sphere, &t1, &t2); 
                if(tmin <= t1 && t1 <= tmax && t1 <= *tclosest){
                    *tclosest = t1;
                    *closestScene = workingScene;
                    found = true;
                }

                if(tmin <= t2 && t2 <= tmax && t2 <= *tclosest){
                    *tclosest = t2;
                    *closestScene = workingScene;
                    found = true;
                }

                break; 
            } 
            case OT_TRIANGLE: 
            {
                static int set = 0;
                float t, u, v;
                bool hit = IntersectRayTriangle(origin, direction, workingScene.triangle, &t, &u, &v);
                 if(hit && tmin <= t && t <= tmax && t <= *tclosest){
                    *tclosest = t;
                    *closestScene = workingScene;
                    found = true;
                }
                break;
            }
        }
    }
    return found;
}

//Return vector R reflected as -R in relation to the normal
static v3_t
ReflectRay(v3_t ray, v3_t normal) {
    return 2.0 * normal * DotProduct(normal, ray) - ray;
}

static float
ComputeLighting(v3_t position, v3_t normal, v3_t viewVector, light_t *lights, int numLights, int specular){
    float intensity = 0.0;
    for(int i = 0; i < numLights; i++){
        light_t light = lights[i];
        bool hasLightRay = false;
        v3_t lightRay;
        float tMax;
        switch(light.type){
            case LT_AMBIENT:
                intensity += light.intensity;
                break;
            case LT_POINT:
                lightRay = light.position - position;
                hasLightRay = true;
                tMax = 1;
                break;
            case LT_DIRECTIONAL:
                lightRay = light.direction;
                hasLightRay = true;
                tMax = FINF;
                break;
        }

        if (hasLightRay){
            //Shadow check
            scene_object_t closestScene;
            float tclosest;
            bool shadowFound = 
                ClosestIntersection(position, lightRay, 0.001, tMax, &tclosest, &closestScene);

            if (shadowFound)
                continue;

            //Diffuse
            float nDotL = DotProduct(normal, lightRay);
            if (nDotL > 0){
                intensity += light.intensity * nDotL / (Magnitude(normal) * Magnitude(lightRay));
            }

            //Specular
            if (specular != -1){
                v3_t reflected = ReflectRay(lightRay, normal);
                float rDotV = DotProduct(reflected, viewVector);
                if (rDotV > 0){
                    intensity += light.intensity * 
                        pow(rDotV/(Magnitude(reflected) * Magnitude(viewVector)), specular);
                }
            }
        }
    } 
    return intensity;
}

static v3_t
NormalOfSceneObject(scene_object_t *sceneObject, v3_t position, v3_t direction){
    switch(sceneObject->type){
        case OT_SPHERE: {
            v3_t normal = position - sceneObject->sphere.center;
            return normal / Magnitude(normal);
        }
        case OT_TRIANGLE: {
            v3_t v0v1 = sceneObject->triangle.p2 - sceneObject->triangle.p1;
            v3_t v0v2 = sceneObject->triangle.p3 - sceneObject->triangle.p1;
            v3_t n = CrossProduct(v0v1, v0v2);
            float d = DotProduct(n, direction);
            if (d < 0) {
                return n;
            } else {
                return -n;
            }
        }
    }
    assert(false);
    return {0,0,0};
}


static uint32_t
TraceRay(v3_t origin, v3_t direction, float tmin, float tmax, int recursionDepth){

    light_t lights[3];
    lights[0].type = LT_AMBIENT;
    lights[0].intensity = 0.2;

    lights[1].type = LT_POINT;
    lights[1].intensity = 0.6;
    lights[1].position = {2, 1, 0};

    lights[2].type = LT_DIRECTIONAL;
    lights[2].intensity = 0.2;
    lights[2].direction = {1, 4, 4};

    float tclosest;
    scene_object_t closestScene;
    bool found = ClosestIntersection(origin, direction, tmin, tmax, &tclosest, &closestScene);
    if (found){
        v3_t position = origin + tclosest * direction;
        v3_t normal = NormalOfSceneObject(&closestScene, position, direction);
        hsv_t hsv = ColorToHsv(closestScene.material.color);
        //TODO environment that controls the actions for the current scene, (reflection, shadow, specular, ...)
        if (1){
            hsv.v = ComputeLighting(position, normal, -direction, &lights[0], 3, closestScene.material.specular);
            uint32_t localColor = HsvToColor(hsv);
        
            float reflection = closestScene.material.reflection;            
            if (recursionDepth <= 0 || reflection <= 0)
                return localColor;

            v3_t reflectedRay = ReflectRay(-direction, normal);
            uint32_t reflectedColor = TraceRay(position, reflectedRay, 0.001, FINF, recursionDepth - 1);

            v3_t localColorVec = ColorToRgbV3(localColor);
            v3_t reflectedColorVec = ColorToRgbV3(reflectedColor);

            v3_t computedColor = localColorVec * (1 - reflection) + reflectedColorVec * reflection;
            return RgbToColor(computedColor.x, computedColor.y, computedColor.z);
        }
        else {
            return closestScene.material.color;
        }
    } else
        return BACKGROUND_COLOR;
}

static bool
HandleKeyboard(environment_t *env, float *yaw, float *pitch, float *roll, v3_t *pos){
    bool result = false;
    event_t *event = ReadEvent(&env->events);

    while (event){
        if (event->type == ET_KEY_DOWN && event->value == 'w'){
            pos->y+= 0.1;
            result = true;
        } else if (event->type == ET_KEY_DOWN && event->value == 's'){
            pos->y-= 0.1;
            result = true;
        } else if (event->type == ET_KEY_DOWN && event->value == 'd'){
            pos->x+= 0.1;
            result = true;
        } else if (event->type == ET_KEY_DOWN && event->value == 'a'){
            pos->x-= 0.1;
            result = true;
        } else if (event->type == ET_KEY_DOWN && event->value == 'i'){
            pos->z+= 0.1;
            result = true;
        } else if (event->type == ET_KEY_DOWN && event->value == 'o'){
            pos->z-= 0.1;
            result = true;
        } else if (event->type == ET_KEY_DOWN && event->value == 'r'){
            (*roll)+=(M_PI_4/4);
            result = true;
        } else if (event->type == ET_KEY_DOWN && event->value == 'y'){
            (*yaw)+=(M_PI_4/4);
            result = true;
        } else if (event->type == ET_KEY_DOWN && event->value == 'p'){
            (*pitch)+=(M_PI_4/4);
            result = true;
        } else if (event->type == ET_KEY_DOWN && event->value == 'c'){
            *pos = {0, 0, 0};
            *yaw = 0; *pitch = 0; *roll = 0;
            result = true;
        }
        event = ReadEvent(&env->events);
     }
    return result;
}


int RayTracePatition(void *data) { 
    while(true){
        display_partition_t *partition  = (display_partition_t*)data;
        if (partition->status != WS_READY){
            SDL_Delay(1);
            continue;
        }

        bitmapSettings_t *bitmap = partition->env->bitmap;
        viewport_t vp = partition->viewport;
        camera_t camera = partition->camera;

        partition->status = WS_RUNNING;

        //Keep it square
        float width = bitmap->height/2;
        for (int x = -width; x < width; x++){
            for(int y = partition->yStart; y < partition->yEnd; y++){
                v3_t direction = CanvasToViewport(bitmap, vp, {(float)x, (float)y}) * camera.rotation;
                uint32_t color = TraceRay(camera.position, direction, 1, FINF, 10);
                CanvasPutPixel(bitmap, {(float)x, (float)y}, color);
            }
        }

        partition->status = WS_FINISHED;

    }
    return 0; 
} 


static void
RayTrace(environment_t *env){
    static bool changesMade = true;
    static float yaw = 0;
    static float pitch = 0;
    static float roll = 0;
    static v3_t cpos = {0, 0, -8};
    bitmapSettings_t *bitmap = env->bitmap;
    viewport_t vp = {1, 1, 1};
    v3_t origin = {0, 0, 0};

    changesMade = changesMade || HandleKeyboard(env, &yaw, &pitch, &roll, &cpos);

    if (!changesMade)
        return;

    changesMade = false;

    camera_t camera;
    camera.position = cpos;
    camera.rotation.data[0][0] = cos(yaw) * cos(pitch);
    camera.rotation.data[0][1] = cos(yaw) * sin(pitch) * sin(roll) - sin(yaw) * cos(roll);
    camera.rotation.data[0][2] = cos(yaw) * sin(pitch) * cos(roll) + sin(yaw) * sin(roll);
    camera.rotation.data[1][0] = sin(yaw) * cos(pitch);
    camera.rotation.data[1][1] = sin(yaw) * sin(pitch) * sin(roll) + cos(yaw) * cos(roll);
    camera.rotation.data[1][2] = sin(yaw) * sin(pitch) * cos(roll) - cos(yaw) * sin(roll);
    camera.rotation.data[2][0] = -sin(yaw);
    camera.rotation.data[2][1] = cos(pitch) * sin(roll);
    camera.rotation.data[2][2] = cos(pitch) * cos(roll);

    float height = bitmap->height/2; 
    float yStart = -height;
    float yStep  = bitmap->height/MAX_THREADS;

    for(int i=0; i < MAX_THREADS; i++) {
        // Generate unique data for each thread to work with.
        displayPart[i]->yStart   = yStart;
        displayPart[i]->yEnd     = yStart+yStep;
        displayPart[i]->env      = env;
        displayPart[i]->viewport = vp;
        displayPart[i]->camera   = camera;
        displayPart[i]->status   = WS_READY;

        yStart += yStep;

        if (displayPart[i]->thread == NULL) {
           exit(3);
        }
    }
}

void
AllocatePartitions(){
    for(int i = 0; i < MAX_THREADS; i++){
        displayPart[i] = (display_partition_t*) calloc(1, sizeof(display_partition_t));

        if( displayPart[i] == NULL ) {
            // If the array allocation fails, the system is out of memory
            // so there is no point in trying to print an error message.
            // Just terminate execution.
            exit(2);
        }

        displayPart[i]->status = WS_FINISHED;

        displayPart[i]->thread = SDL_CreateThread( 
            RayTracePatition,       // thread function name
            "",
            displayPart[i]);// returns the thread identifier 
    }
}

bool
RayThread(environment_t *env){
    static bool initialized = false;

    if (!initialized){
        AllocatePartitions();
        initialized = true;
    }

    // Wait until all workers have finished.
    bool complete = true;
    for(int i = 0; i < MAX_THREADS; i++){
        if (displayPart[i]->status != WS_FINISHED)
            complete = false;
    }

    if (complete)
        RayTrace(env);

    return true;
}

