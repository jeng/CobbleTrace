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
// DONE - Optimize by tracing every other pixel and averaging the one between
// TODO - Add memoization after doing some profiling
// DONE - allow for different size bitmaps and blit to screen
// DONE - Use previous image when a change has not been made to the scene
// TODO - Copy the object loading code from software renderer
// DONE - Allow for scene files to be loaded
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
// DONE - test compiling on linux
// TODO - add a gui
// TODO - Create a distributed version I can run on the raspberry pi cluster
//  1) this is one of the articles I had on my todo list
// TODO - Fix camera rotation
// TODO - SDL is the platform layer here so I can remove eventQueue code
// TODO - Check the reflection on triangles it might be broken

#define _USE_MATH_DEFINES // for C++
#include <cmath>
#include <assert.h>
#include <stdio.h>
#include "raythread.h"
#include "draw2d.h"
#include "color.h"
#include "scenefile.h"
#include "SDL.h"
#include "bvh.h"

const float FINF = 4294967296.0;
const uint32_t BACKGROUND_COLOR = RgbToColor(0x33, 0x33, 0x33);
//const int MAX_THREADS = 16;
//const bool SUBSAMPLING = true;

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
    scene_t *scene;    
    worker_status_t status;
    SDL_Thread *thread;
    bvh_state_t *bvhState;
};

static display_partition_t **displayPart;


// This is the ray intersect method from Glassner's Ray Tracing book(pg 50) and Scratch A Pixel
//
// It is using the method of solving for t to see when I point hits the plane
// based on reformulating with equation of a plane
//    
// It then uses barycentric coordinates to determine if we are inside or out
// side of the triangle

static bool
IntersectRayTriangle(ray_t ray, triangle_t triangle, float *t, float *u, float *v){
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

    float NDotRayDir = DotProduct(n, ray.direction);
    if (fabs(NDotRayDir) < epsilon)
        return false;

    //Using the equation of a plane.  See Glassner's
    float d = -1 * DotProduct(n, triangle.p1);

    //Compute T, this is the point where the ray hits the plane
    //This video from UC Davis explains this equation
    //https://www.youtube.com/watch?v=Ahp6LDQnK4Y  @ 16:14
    *t = -(d + DotProduct(n, ray.origin)) / NDotRayDir;

    //If the triangle is behind
    if (t < 0) 
        return false;

    //This is just the standard find a point along the ray
    //Since we solved for t we can get the point
    v3_t p = ray.origin + *t * ray.direction;

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
IntersectRaySphere(ray_t ray, sphere_t sphere, float *t1, float *t2){
    float r = sphere.radius;
    v3_t co = ray.origin - sphere.center;
    float a = DotProduct(ray.direction, ray.direction);
    float b = 2 * DotProduct(co, ray.direction);
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
ClosestIntersection(ray_t ray,
    float tmin, float tmax, 
    float *tclosest, scene_object_t *closestScene,
    scene_t *scene,
    bvh_state_t *bvhState){

    *tclosest = FINF;
    uint32_t closestIndex = 0;

    IntersectBVHClosest(&ray, bvhState->rootNodeIdx, bvhState, tclosest, &closestIndex);
    //TODO add sphere's to the bvh
    //TODO clean up the object management.  I going to need these triangles
    //indexed by object type so I don't have to loop over them
    *closestScene = scene->objectStack.objects[scene->triangleLookup.indexes[closestIndex]];
    //scene_object_t workingObject;
    //int triangleCount = 0;
    //if(ray.t != 1e30f){
    //    for(int i = 0; i < scene->objectStack.index; i++){
    //        workingObject = scene->objectStack.objects[i];
    //        if(workingObject.type == OT_TRIANGLE){
    //            if (triangleCount++ == closestIndex){
    //                *closestScene = workingObject;
    //                break;
    //            }
    //        }
    //    }
    //}
    
    
    return (ray.t != 1e30f);


    //bool found = false;
    //scene_object_t workingObject;
    //for(int i = 0; i < scene->objectStack.index; i++){
    //    workingObject = scene->objectStack.objects[i];
    //    switch(workingObject.type){
    //        case OT_SPHERE: {
    //            float t1, t2;
    //            IntersectRaySphere(ray, workingObject.sphere, &t1, &t2); 
    //            if(tmin <= t1 && t1 <= tmax && t1 <= *tclosest){
    //                *tclosest = t1;
    //                *closestScene = workingObject;
    //                found = true;
    //            }

    //            if(tmin <= t2 && t2 <= tmax && t2 <= *tclosest){
    //                *tclosest = t2;
    //                *closestScene = workingObject;
    //                found = true;
    //            }

    //            break; 
    //        } 
    //        case OT_TRIANGLE: 
    //        {
    //            static int set = 0;
    //            float t, u, v;
    //            bool hit = IntersectRayTriangle(ray, workingObject.triangle, &t, &u, &v);
    //             if(hit && tmin <= t && t <= tmax && t <= *tclosest){
    //                *tclosest = t;
    //                *closestScene = workingObject;
    //                found = true;
    //            }
    //            break;
    //        }
    //    }
    //}
    //return found;
}

//Return vector R reflected as -R in relation to the normal
static v3_t
ReflectRay(v3_t ray, v3_t normal) {
    return 2.0 * normal * DotProduct(normal, ray) - ray;
}

static float
ComputeLighting(v3_t position, v3_t normal, v3_t viewVector, scene_t *scene, int specular, bvh_state_t *bvhState){
    float intensity = 0.0;
    for(int i = 0; i < scene->lightStack.index; i++){
        light_t light = scene->lightStack.lights[i];
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
                ClosestIntersection({position, lightRay, 1e30f}, 0.001, tMax, &tclosest, &closestScene, scene, bvhState);

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
TraceRay(ray_t ray, float tmin, float tmax, int recursionDepth, scene_t *scene, bvh_state_t *bvhState){

    float tclosest;
    scene_object_t closestScene;
    bool found = ClosestIntersection(ray, tmin, tmax, &tclosest, &closestScene, scene, bvhState);
    if (found){
        v3_t position = ray.origin + tclosest * ray.direction;
        v3_t normal = NormalOfSceneObject(&closestScene, position, ray.direction);
        hsv_t hsv = ColorToHsv(closestScene.material.color);
        //TODO environment that controls the actions for the current scene, (reflection, shadow, specular, ...)
        if (1){
            hsv.v = ComputeLighting(position, normal, -ray.direction, scene, closestScene.material.specular, bvhState);
            uint32_t localColor = HsvToColor(hsv);
        
            float reflection = closestScene.material.reflection;            
            if (recursionDepth <= 0 || reflection <= 0)
                return localColor;

            v3_t reflectedRay = ReflectRay(-ray.direction, normal);
            uint32_t reflectedColor = TraceRay({position, reflectedRay, 0}, 0.001, FINF, recursionDepth - 1, scene, bvhState);

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


int RayTracePartition(void *data) { 
    while(true){
        display_partition_t *partition  = (display_partition_t*)data;
        if (partition->status != WS_READY){
            SDL_Delay(1);
            continue;
        }

        bitmapSettings_t *bitmap = partition->env->bitmap;
        viewport_t vp = partition->viewport;
        scene_t *scene = partition->scene;
        bvh_state_t *bvhState = partition->bvhState;

        partition->status = WS_RUNNING;

        //Keep it square
        //TODO why am I doing this?
        float width = bitmap->height/2;
        for (int x = -width; x < width; x++){
            uint32_t lastColor = 0; 
            for(int y = partition->yStart; y < partition->yEnd;){ 

                uint32_t color = 0;
                if (scene->settings.supersampling){
                    const int samples = 4;
                    float denom = samples * 2;
                    float stepsize = 1/(float)samples;
                    float jitter = stepsize/8;
                    float sampleX = (float)x;
                    bool first = true;
                    for (int xs = 0; xs < samples; xs++){
                        float sampleY = (float)y;
                        for(int ys = 0; ys < samples; ys++){
                            float rx = (float)rand() / (float)RAND_MAX;
                            float ry = (float)rand() / (float)RAND_MAX;

                            rx = (rx * (2 * jitter)) - jitter;
                            ry = (ry * (2 * jitter)) - jitter;

                            sampleX += rx;
                            sampleY += ry;

                            v3_t direction = CanvasToViewport(bitmap, vp, {(float)sampleX, (float)sampleY}) * scene->camera.rotation;
                            uint32_t tempColor = TraceRay({scene->camera.position, direction, 1e30f}, 1, FINF, 10, scene, bvhState);

                            if (first){
                                first = false;
                                color = tempColor;
                            } else {
                                rgb_t rgb = ColorToRgb(color);
                                rgb_t tempRgb = ColorToRgb(tempColor);
                                rgb.r -= (rgb.r/denom);
                                rgb.r += (tempRgb.r/denom);

                                rgb.g -= (rgb.g/denom);
                                rgb.g += (tempRgb.g/denom);

                                rgb.b -= (rgb.b/denom);
                                rgb.b += (tempRgb.b/denom);

                                color = RgbToColor(rgb);
                            }

                            sampleY -= ry;
                            sampleY += stepsize;
                            sampleX -= rx;
                        }
                        sampleX += stepsize;
                    }
                } else {
                    v3_t direction = CanvasToViewport(bitmap, vp, {(float)x, (float)y}) * scene->camera.rotation;
                    color = TraceRay({scene->camera.position, direction, 1e30f}, 1, FINF, 10, scene, bvhState);
                }
                CanvasPutPixel(bitmap, {(float)x, (float)y}, color);

                if (scene->settings.subsampling){
                    if (y == partition->yStart)
                        lastColor = color;

                    //TODO add an average function to color.h that averages a list of colors
                    rgb_t rgbLast = ColorToRgb(lastColor);
                    rgb_t rgbColor = ColorToRgb(color);
                    rgb_t rgbAvg;
                    rgbAvg.r = min((rgbLast.r + rgbColor.r)/2, 0xff);
                    rgbAvg.g = min((rgbLast.g + rgbColor.g)/2, 0xff);
                    rgbAvg.b = min((rgbLast.b + rgbColor.b)/2, 0xff);
                    uint32_t avgColor = RgbToColor(rgbAvg);
                    CanvasPutPixel(bitmap, {(float)x, (float)y-1}, avgColor);

                    if (y + 2 >= partition->yEnd)
                        y++;
                    else
                        y+=2;

                }
                else
                    y++;

                lastColor = color;
            }
        }

        partition->status = WS_FINISHED;

    }
    return 0; 
} 


static void
HandleUpdates(environment_t *env, scene_t *scene, bvh_state_t *bvhState){
    static bool changesMade = true;
    static float yaw = 0;
    static float pitch = 0;
    static float roll = 0;
    static v3_t cpos = {0, 0, -8};
    bitmapSettings_t *bitmap = env->bitmap;
    viewport_t vp = {1, 1, 1};
    v3_t origin = {0, 0, 0};

    changesMade = changesMade || HandleKeyboard(env, &yaw, &pitch, &roll, &scene->camera.position);

    if (!changesMade)
        return;

    changesMade = false;
    
    scene->camera.rotation.data[0][0] = cos(yaw) * cos(pitch);
    scene->camera.rotation.data[0][1] = cos(yaw) * sin(pitch) * sin(roll) - sin(yaw) * cos(roll);
    scene->camera.rotation.data[0][2] = cos(yaw) * sin(pitch) * cos(roll) + sin(yaw) * sin(roll);
    scene->camera.rotation.data[1][0] = sin(yaw) * cos(pitch);
    scene->camera.rotation.data[1][1] = sin(yaw) * sin(pitch) * sin(roll) + cos(yaw) * cos(roll);
    scene->camera.rotation.data[1][2] = sin(yaw) * sin(pitch) * cos(roll) - cos(yaw) * sin(roll);
    scene->camera.rotation.data[2][0] = -sin(yaw);
    scene->camera.rotation.data[2][1] = cos(pitch) * sin(roll);
    scene->camera.rotation.data[2][2] = cos(pitch) * cos(roll);

    float height = bitmap->height/2; 
    float yStart = -height;
    float yStep  = bitmap->height/scene->settings.numberOfThreads;

    for(int i=0; i < scene->settings.numberOfThreads; i++) {
        // Generate unique data for each thread to work with.
        displayPart[i]->yStart   = yStart;
        displayPart[i]->yEnd     = yStart+yStep;
        displayPart[i]->env      = env;
        displayPart[i]->viewport = vp;
        displayPart[i]->scene    = scene;
        displayPart[i]->bvhState = bvhState;
        displayPart[i]->status   = WS_READY;

        yStart += yStep;

        if (displayPart[i]->thread == NULL) {
           exit(3);
        }
    }
}

void
AllocatePartitions(scene_t *scene){

    displayPart = (display_partition_t**)calloc(scene->settings.numberOfThreads, sizeof(display_partition_t));

    for(int i = 0; i < scene->settings.numberOfThreads; i++){
        displayPart[i] = (display_partition_t*) calloc(1, sizeof(display_partition_t));

        if( displayPart[i] == NULL ) {
            // If the array allocation fails, the system is out of memory
            // so there is no point in trying to print an error message.
            // Just terminate execution.
            exit(2);
        }

        displayPart[i]->status = WS_FINISHED;

        displayPart[i]->thread = SDL_CreateThread( 
            RayTracePartition,       // thread function name
            "",
            displayPart[i]);// returns the thread identifier 
    }
}

//TODO(JHE) make an object stack I can work with instead of calling calloc here
triangle_t *
GetSceneTriangles(scene_t *scene, uint32_t *size){
    *size = 0;
    for(int i = 0; i < scene->objectStack.index; i++){
        scene_object_t workingObject = scene->objectStack.objects[i];
        if (workingObject.type == OT_TRIANGLE){
            (*size)++;
        }
    }
    triangle_t *triangles = (triangle_t*)calloc(*size, sizeof(triangle_t));
    int n = 0;
    for(int i = 0; i < scene->objectStack.index; i++){
        scene_object_t workingObject = scene->objectStack.objects[i];
        if (workingObject.type == OT_TRIANGLE){
            triangles[n++] = workingObject.triangle;
        }
    }
    return triangles;
 }

bool
RayThread(environment_t *env, scene_t *scene){
    static bool initialized = false;
    static bvh_state_t bvhState;
    static triangle_t *triangles;

    if (!initialized){
        AllocatePartitions(scene);
        uint32_t size;
        triangle_t *triangles = GetSceneTriangles(scene, &size);
        bvhState = InitializeBVHState(triangles, size);
        BuildBVH(&bvhState);
        initialized = true;
    }

    // Wait until all workers have finished.
    bool complete = true;
    for(int i = 0; i < scene->settings.numberOfThreads; i++){
        if (displayPart[i]->status != WS_FINISHED)
            complete = false;
    }

    if (complete)
        HandleUpdates(env, scene, &bvhState);

    return true;
}