#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "../mymath.h"

struct triangle_t {
    v3_t p1;
    v3_t p2;
    v3_t p3;
};

void WriteSettings(){
    printf("  \"settings\":{\n");
    printf("    \"numberOfThreads\": 8,\n");
    printf("    \"subsampling\": true\n");
    printf("  }\n");
}

void WriteCamera(){
    printf("  \"camera\":{\n");
    printf("    \"position\": [0, 0, -3]\n");
    printf("  }\n");
}

void WriteLights(){
    printf("    \"lights\":[\n");
    printf("        {\n");
    printf("            \"type\": \"ambient\",\n");
    printf("            \"intensity\": 0.2\n");
    printf("        },\n");
    printf("        {\n");
    printf("            \"type\": \"point\",\n");
    printf("            \"intensity\": 0.6,\n");
    printf("            \"position\": [2, 1, 0]\n");
    printf("        },\n");
    printf("        {\n");
    printf("            \"type\": \"directional\",\n");
    printf("            \"intensity\": 0.2,\n");
    printf("            \"direction\": [1, 4, 4]\n");
    printf("        }\n");
    printf("    ],\n");
}

void WriteTriangle(triangle_t tri, v3_t color, float reflection, bool comma){
    printf("        {\n");
    printf("            \"type\": \"triangle\",\n");
    printf("            \"p1\": [%.4f, %.4f, %.4f],\n", tri.p1.x, tri.p1.y, tri.p1.z);
    printf("            \"p2\": [%.4f, %.4f, %.4f],\n", tri.p2.x, tri.p2.y, tri.p2.z);
    printf("            \"p3\": [%.4f, %.4f, %.4f],\n", tri.p3.x, tri.p3.y, tri.p3.z);
    printf("            \"color\": [%.0f, %.0f, %.0f],\n", color.x, color.y, color.z);
    printf("            \"specular\": 100,\n");
    printf("            \"reflection\": %.4f\n", reflection);
    printf("        }%c\n", (comma) ? ',': ' ');
}



void StartObjects(){
    printf("    \"objects\":[\n");
}

void EndObjects(){
    printf("    ],\n");
}

v3_t TranslatePoint(m4x4_t translation, v3_t point){
    v4_t v4;
    v4.x = point.x;
    v4.y = point.y;
    v4.z = point.z;
    v4.w = 1;

    v4 = v4 * translation;
    return {v4.x, v4.y, v4.z};
}

void WriteSphere(v3_t center, float radius, bool comma){
    printf("        {\n");
    printf("            \"type\": \"sphere\",\n");
    printf("            \"center\": [%.2f, %.2f, %.2f],\n", center.x, center.y, center.z);
    printf("            \"radius\": %f,\n", radius);
    printf("            \"color\": [100, 200, 0],\n");
    printf("            \"specular\": 100,\n");
    printf("            \"reflection\": 0.5\n");
    printf("        }%c\n", (comma) ? ',': ' ');
}


void WriteCube(v3_t translation, float yRotation, v3_t color, bool comma){
    triangle_t tlist[12];
    //define the cube faces
    //South face
    tlist[ 0].p1.x = 0; tlist[ 0].p1.y = 0; tlist[ 0].p1.z = 0; 
    tlist[ 0].p2.x = 0; tlist[ 0].p2.y = 1; tlist[ 0].p2.z = 0; 
    tlist[ 0].p3.x = 1; tlist[ 0].p3.y = 1; tlist[ 0].p3.z = 0; 

    tlist[ 1].p1.x = 0; tlist[ 1].p1.y = 0; tlist[ 1].p1.z = 0; 
    tlist[ 1].p2.x = 1; tlist[ 1].p2.y = 1; tlist[ 1].p2.z = 0; 
    tlist[ 1].p3.x = 1; tlist[ 1].p3.y = 0; tlist[ 1].p3.z = 0; 

    //East face
    tlist[ 2].p1.x = 1; tlist[ 2].p1.y = 0; tlist[ 2].p1.z = 0; 
    tlist[ 2].p2.x = 1; tlist[ 2].p2.y = 1; tlist[ 2].p2.z = 0; 
    tlist[ 2].p3.x = 1; tlist[ 2].p3.y = 1; tlist[ 2].p3.z = 1; 

    tlist[ 3].p1.x = 1; tlist[ 3].p1.y = 0; tlist[ 3].p1.z = 0; 
    tlist[ 3].p2.x = 1; tlist[ 3].p2.y = 1; tlist[ 3].p2.z = 1; 
    tlist[ 3].p3.x = 1; tlist[ 3].p3.y = 0; tlist[ 3].p3.z = 1; 

    //Top face
    tlist[ 4].p1.x = 0; tlist[ 4].p1.y = 1; tlist[ 4].p1.z = 0; 
    tlist[ 4].p2.x = 0; tlist[ 4].p2.y = 1; tlist[ 4].p2.z = 1; 
    tlist[ 4].p3.x = 1; tlist[ 4].p3.y = 1; tlist[ 4].p3.z = 1; 

    tlist[ 5].p1.x = 0; tlist[ 5].p1.y = 1; tlist[ 5].p1.z = 0; 
    tlist[ 5].p2.x = 1; tlist[ 5].p2.y = 1; tlist[ 5].p2.z = 1; 
    tlist[ 5].p3.x = 1; tlist[ 5].p3.y = 1; tlist[ 5].p3.z = 0; 

    //North face
    tlist[ 6].p1.x = 1; tlist[ 6].p1.y = 0; tlist[ 6].p1.z = 1; 
    tlist[ 6].p2.x = 1; tlist[ 6].p2.y = 1; tlist[ 6].p2.z = 1; 
    tlist[ 6].p3.x = 0; tlist[ 6].p3.y = 1; tlist[ 6].p3.z = 1; 

    tlist[ 7].p1.x = 1; tlist[ 7].p1.y = 0; tlist[ 7].p1.z = 1; 
    tlist[ 7].p2.x = 0; tlist[ 7].p2.y = 1; tlist[ 7].p2.z = 1; 
    tlist[ 7].p3.x = 0; tlist[ 7].p3.y = 0; tlist[ 7].p3.z = 1; 

    //Bottom face
    tlist[ 8].p1.x = 1; tlist[ 8].p1.y = 0; tlist[ 8].p1.z = 1; 
    tlist[ 8].p2.x = 0; tlist[ 8].p2.y = 0; tlist[ 8].p2.z = 1; 
    tlist[ 8].p3.x = 0; tlist[ 8].p3.y = 0; tlist[ 8].p3.z = 0; 

    tlist[ 9].p1.x = 1; tlist[ 9].p1.y = 0; tlist[ 9].p1.z = 1; 
    tlist[ 9].p2.x = 0; tlist[ 9].p2.y = 0; tlist[ 9].p2.z = 0; 
    tlist[ 9].p3.x = 1; tlist[ 9].p3.y = 0; tlist[ 9].p3.z = 0; 

    //West face
    tlist[10].p1.x = 0; tlist[10].p1.y = 0; tlist[10].p1.z = 1; 
    tlist[10].p2.x = 0; tlist[10].p2.y = 1; tlist[10].p2.z = 1; 
    tlist[10].p3.x = 0; tlist[10].p3.y = 1; tlist[10].p3.z = 0; 

    tlist[11].p1.x = 0; tlist[11].p1.y = 0; tlist[11].p1.z = 1; 
    tlist[11].p2.x = 0; tlist[11].p2.y = 1; tlist[11].p2.z = 0; 
    tlist[11].p3.x = 0; tlist[11].p3.y = 0; tlist[11].p3.z = 0; 

    //Rotate
    //float theta = M_PI/4;
    m4x4_t rotate_y;
    rotate_y.data[0][0] = cos(yRotation);
    rotate_y.data[0][1] = 0;
    rotate_y.data[0][2] = sin(yRotation);
    rotate_y.data[0][3] = 0;
    rotate_y.data[1][0] = 0;
    rotate_y.data[1][1] = 1;
    rotate_y.data[1][2] = 0;
    rotate_y.data[1][3] = 0;
    rotate_y.data[2][0] = -sin(yRotation);
    rotate_y.data[2][1] = 0;
    rotate_y.data[2][2] = cos(yRotation);
    rotate_y.data[2][3] = 0;
    rotate_y.data[3][0] = 0;
    rotate_y.data[3][1] = 0;
    rotate_y.data[3][2] = 0;
    rotate_y.data[3][3] = 1;


    //Translate
    m4x4_t translate;
    translate.data[0][0] = 1;
    translate.data[0][1] = 0;
    translate.data[0][2] = 0;
    translate.data[0][3] = 0;//-2;
    translate.data[1][0] = 0;
    translate.data[1][1] = 1;
    translate.data[1][2] = 0;
    translate.data[1][3] = 0;
    translate.data[2][0] = 0;
    translate.data[2][1] = 0;
    translate.data[2][2] = 1;
    translate.data[2][3] = 0;
    translate.data[3][0] = translation.x;//-1;
    translate.data[3][1] = translation.y;//3;
    translate.data[3][2] = translation.z;//5;
    translate.data[3][3] = 0;



    for(int i = 0; i < 12; i++){
        triangle_t t = tlist[i];
        t.p1 = TranslatePoint(rotate_y, t.p1);
        t.p2 = TranslatePoint(rotate_y, t.p2);
        t.p3 = TranslatePoint(rotate_y, t.p3);
        t.p1 = TranslatePoint(translate, t.p1);
        t.p2 = TranslatePoint(translate, t.p2);
        t.p3 = TranslatePoint(translate, t.p3);
        WriteSphere(t.p1, 0.04, true);
        WriteSphere(t.p2, 0.04, true);
        WriteSphere(t.p3, 0.04, true);
        WriteTriangle(t, color, 0, (comma) ? true : i != 11);
    }
}

int main(){
    triangle_t tri;
    tri.p1 = {-500, -500, 6.5};
    tri.p2 = { 500, -500, 4.5};
    tri.p3 = {   0,  500, 4.5};
    printf("{\n");    
    StartObjects();
    //WriteTriangle(tri, {245, 201, 105}, 0, true);
    WriteSphere({0, 0, 6}, 10, true);
    WriteCube({0, 0, 2}, M_PI/4, {0,255,0}, true);
    WriteCube({-1.25, 1.25, 2}, M_PI/8, {0,255,255}, true);
    WriteCube({1, -1.25, 2}, M_PI/2 + M_PI/8, {255,0,255}, false);
    EndObjects();
    WriteLights();
    WriteCamera();
    printf(",\n");
    WriteSettings();
    printf("}\n");    
}