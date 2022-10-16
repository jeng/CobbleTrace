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

#define NUM_CELLS 26
#define BUF_SZ 11
#define HALF_SZ 5

//TODO what were these used for?
#define N_PBUF 1
#define N_BUF  0


bool prebuffer[BUF_SZ][BUF_SZ][BUF_SZ] = {0};
bool buffer[BUF_SZ][BUF_SZ][BUF_SZ] = {0};
v3_t twentysixcell[NUM_CELLS] = 
{
  {0, 0, 1},
  {0, 0, -1},
  {0, 1, 0},
  {0, 1, 1},
  {0, 1, -1},
  {0, -1, 0},
  {0, -1, 1},
  {0, -1, -1},
  {1, 0, 0},
  {1, 0, 1},
  {1, 0, -1},
  {1, 1, 0},
  {1, 1, 1},
  {1, 1, -1},
  {1, -1, 0},
  {1, -1, 1},
  {1, -1, -1},
  {-1, 0, 0},
  {-1, 0, 1},
  {-1, 0, -1},
  {-1, 1, 0},
  {-1, 1, 1},
  {-1, 1, -1},
  {-1, -1, 0},
  {-1, -1, 1},
  {-1, -1, -1}
};

void WriteSettings(){
    printf("  \"settings\":{\n");
    printf("    \"numberOfThreads\": 8,\n");
    printf("    \"subsampling\": true\n");
    printf("  }\n");
}

void WriteCamera(){
    printf("  \"camera\":{\n");
    printf("    \"position\": [6, 6, -20]\n");
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
    printf("            \"position\": [30, 30, -20]\n");
    printf("        }\n");
    // printf("        {\n");
    // printf("            \"type\": \"directional\",\n");
    // printf("            \"intensity\": 0.2,\n");
    // printf("            \"direction\": [1, 4, 4]\n");
    // printf("        }\n");
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


void WriteCube(v3_t translation, float xRotation, float yRotation, v3_t color, bool comma){
    triangle_t tlist[12];
    //define the cube faces
    //South face
    tlist[ 0].p1.x = -1; tlist[ 0].p1.y = -1; tlist[ 0].p1.z = -1; 
    tlist[ 0].p2.x = -1; tlist[ 0].p2.y = 1; tlist[ 0].p2.z = -1; 
    tlist[ 0].p3.x = 1; tlist[ 0].p3.y = 1; tlist[ 0].p3.z = -1; 

    tlist[ 1].p1.x = -1; tlist[ 1].p1.y = -1; tlist[ 1].p1.z = -1; 
    tlist[ 1].p2.x = 1; tlist[ 1].p2.y = 1; tlist[ 1].p2.z = -1; 
    tlist[ 1].p3.x = 1; tlist[ 1].p3.y = -1; tlist[ 1].p3.z = -1; 

    //East face
    tlist[ 2].p1.x = 1; tlist[ 2].p1.y = -1; tlist[ 2].p1.z = -1; 
    tlist[ 2].p2.x = 1; tlist[ 2].p2.y = 1; tlist[ 2].p2.z = -1; 
    tlist[ 2].p3.x = 1; tlist[ 2].p3.y = 1; tlist[ 2].p3.z = 1; 

    tlist[ 3].p1.x = 1; tlist[ 3].p1.y = -1; tlist[ 3].p1.z = -1; 
    tlist[ 3].p2.x = 1; tlist[ 3].p2.y = 1; tlist[ 3].p2.z = 1; 
    tlist[ 3].p3.x = 1; tlist[ 3].p3.y = -1; tlist[ 3].p3.z = 1; 

    //Top face
    tlist[ 4].p1.x = -1; tlist[ 4].p1.y = 1; tlist[ 4].p1.z = -1; 
    tlist[ 4].p2.x = -1; tlist[ 4].p2.y = 1; tlist[ 4].p2.z = 1; 
    tlist[ 4].p3.x = 1; tlist[ 4].p3.y = 1; tlist[ 4].p3.z = 1; 

    tlist[ 5].p1.x = -1; tlist[ 5].p1.y = 1; tlist[ 5].p1.z = -1; 
    tlist[ 5].p2.x = 1; tlist[ 5].p2.y = 1; tlist[ 5].p2.z = 1; 
    tlist[ 5].p3.x = 1; tlist[ 5].p3.y = 1; tlist[ 5].p3.z = -1; 

    //North face
    tlist[ 6].p1.x = 1; tlist[ 6].p1.y = -1; tlist[ 6].p1.z = 1; 
    tlist[ 6].p2.x = 1; tlist[ 6].p2.y = 1; tlist[ 6].p2.z = 1; 
    tlist[ 6].p3.x = -1; tlist[ 6].p3.y = 1; tlist[ 6].p3.z = 1; 

    tlist[ 7].p1.x = 1; tlist[ 7].p1.y = -1; tlist[ 7].p1.z = 1; 
    tlist[ 7].p2.x = -1; tlist[ 7].p2.y = 1; tlist[ 7].p2.z = 1; 
    tlist[ 7].p3.x = -1; tlist[ 7].p3.y = -1; tlist[ 7].p3.z = 1; 

    //Bottom face
    tlist[ 8].p1.x = 1; tlist[ 8].p1.y = -1; tlist[ 8].p1.z = 1; 
    tlist[ 8].p2.x = -1; tlist[ 8].p2.y = -1; tlist[ 8].p2.z = 1; 
    tlist[ 8].p3.x = -1; tlist[ 8].p3.y = -1; tlist[ 8].p3.z = -1; 

    tlist[ 9].p1.x = 1; tlist[ 9].p1.y = -1; tlist[ 9].p1.z = 1; 
    tlist[ 9].p2.x = -1; tlist[ 9].p2.y = -1; tlist[ 9].p2.z = -1; 
    tlist[ 9].p3.x = 1; tlist[ 9].p3.y = -1; tlist[ 9].p3.z = -1; 

    //West face
    tlist[10].p1.x = -1; tlist[10].p1.y = -1; tlist[10].p1.z = 1; 
    tlist[10].p2.x = -1; tlist[10].p2.y = 1; tlist[10].p2.z = 1; 
    tlist[10].p3.x = -1; tlist[10].p3.y = 1; tlist[10].p3.z = -1; 

    tlist[11].p1.x = -1; tlist[11].p1.y = -1; tlist[11].p1.z = 1; 
    tlist[11].p2.x = -1; tlist[11].p2.y = 1; tlist[11].p2.z = -1; 
    tlist[11].p3.x = -1; tlist[11].p3.y = -1; tlist[11].p3.z = -1; 

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

    m4x4_t rotate_x;
    rotate_x.data[0][0] = 1;
    rotate_x.data[0][1] = 0;
    rotate_x.data[0][2] = 0;
    rotate_x.data[0][3] = 0;
    rotate_x.data[1][0] = 0;
    rotate_x.data[1][1] = cos(xRotation);
    rotate_x.data[1][2] = -sin(xRotation);
    rotate_x.data[1][3] = 0;
    rotate_x.data[2][0] = 0;
    rotate_x.data[2][1] = sin(xRotation);
    rotate_x.data[2][2] = cos(xRotation);
    rotate_x.data[2][3] = 0;
    rotate_x.data[3][0] = 0;
    rotate_x.data[3][1] = 0;
    rotate_x.data[3][2] = 0;
    rotate_x.data[3][3] = 1;

    //Scale
    m4x4_t scale;
    scale.data[0][0] = 0.45;//10;
    scale.data[0][1] = 0;
    scale.data[0][2] = 0;
    scale.data[0][3] = 0;
    scale.data[1][0] = 0;
    scale.data[1][1] = 0.45;//10;
    scale.data[1][2] = 0;
    scale.data[1][3] = 0;
    scale.data[2][0] = 0;
    scale.data[2][1] = 0;
    scale.data[2][2] = 0.45;//10;
    scale.data[2][3] = 0;
    scale.data[3][0] = 0;
    scale.data[3][1] = 0;
    scale.data[3][2] = 0;
    scale.data[3][3] = 1;


    //Translate
    m4x4_t translate;
    translate.data[0][0] = 1;
    translate.data[0][1] = 0;
    translate.data[0][2] = 0;
    translate.data[0][3] = 0;
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


    v3_t colors[12];
    colors[0] = {100, 0, 0};
    colors[1] = {100, 100, 0};
    colors[2] = {100, 0, 100};
    colors[3] = {0, 100, 0};
    colors[4] = {0, 100, 100};
    colors[5] = {0, 0, 100};
    colors[6] = {255, 0, 0};
    colors[7] = {255, 255, 0};
    colors[8] = {0, 255, 255};
    colors[9] = {0, 255, 0};
    colors[10] = {0, 0, 255};
    colors[11] = {255, 0, 255};


    //m4x4_t operation;
    //operation = scale * rotate_x * rotate_y * translate;

    for(int i = 0; i < 12; i++){
        triangle_t t = tlist[i];        
        t.p1 = TranslatePoint(rotate_y, t.p1);
        t.p2 = TranslatePoint(rotate_y, t.p2);
        t.p3 = TranslatePoint(rotate_y, t.p3);

        t.p1 = TranslatePoint(rotate_x, t.p1);
        t.p2 = TranslatePoint(rotate_x, t.p2);
        t.p3 = TranslatePoint(rotate_x, t.p3);
        
        t.p1 = TranslatePoint(scale, t.p1);
        t.p2 = TranslatePoint(scale, t.p2);
        t.p3 = TranslatePoint(scale, t.p3);

        t.p1 = TranslatePoint(translate, t.p1);
        t.p2 = TranslatePoint(translate, t.p2);
        t.p3 = TranslatePoint(translate, t.p3);

        //WriteSphere(t.p1, 0.04, true);
        //WriteSphere(t.p2, 0.04, true);
        //WriteSphere(t.p3, 0.04, true);

        WriteTriangle(t, color/*colors[i]*/, 0, (comma) ? true : i != 11);
    }
}

void DrawGrid(){
    for (float i = -20; i < 20; i+=1){
        WriteSphere({i, 0, 0}, 0.1, true);
        WriteSphere({0, i, 0}, 0.1, true);
        //WriteSphere({0, 0, i}, 0.1, );
    }
}

void BuildDisplay(){
    bool first = true;
    for(int i = -HALF_SZ; i <= HALF_SZ; i++){
        for(int j = -HALF_SZ; j <= HALF_SZ; j++){
            for(int k = -HALF_SZ; k <= HALF_SZ; k++){
                v3_t v2 = {i + HALF_SZ, j + HALF_SZ, k + HALF_SZ};
                if (buffer[(int)v2.x][(int)v2.y][(int)v2.z]){
                    if (!first)
                        printf(",\n");

                    WriteCube(
                        v2, 
                        0,//-M_PI/8, 
                        0,//M_PI/4, 
                        {241, 156, 187},
                        false
                        );

                    first = false;                        
                }
            }
        }
    }
}

int CheckCells(v3_t v){
    int result = 0;
    for(int i = 0; i < NUM_CELLS; i++){
        v3_t v2 = {
            v.x + HALF_SZ + twentysixcell[i].x,
            v.y + HALF_SZ + twentysixcell[i].y,
            v.z + HALF_SZ + twentysixcell[i].z};
        if (-1 < v2.x && v2.x < BUF_SZ &&
            -1 < v2.y && v2.y < BUF_SZ &&
            -1 < v2.z && v2.z < BUF_SZ &&
            prebuffer[(int)v2.x][(int)v2.y][(int)v2.z])
            result++;
    }
    return result;
}

void CopyBuffer(){
    for(int i = 0; i < BUF_SZ; i++){
        for(int j = 0; j < BUF_SZ; j++){
            for(int k = 0; k < BUF_SZ; k++){
                prebuffer[i][j][k] = buffer[i][j][k];
            }
        }
    }
}

void SetCellValue(bool b[BUF_SZ][BUF_SZ][BUF_SZ], v3_t v, bool value){
    v3_t v2 = {v.x + HALF_SZ, v.y + HALF_SZ, v.z + HALF_SZ};
    b[(int)v2.x][(int)v2.y][(int)v2.z] = value;
}

void Rule1Init(){
    SetCellValue(prebuffer, {0,0,0}, true);
    SetCellValue(buffer, {0,0,0}, true);
}

void CellWalk(){
    for(int i = -HALF_SZ; i <= HALF_SZ; i++){
        for(int j = -HALF_SZ; j <= HALF_SZ; j++){
            for(int k = -HALF_SZ; k <= HALF_SZ; k++){
                v3_t v = {i, j, k};
                int cellSum = CheckCells(v);
                if (cellSum == 1){
                    SetCellValue(buffer, v, true);
                }
            }
        }
    }
}

int main(){
    triangle_t tri;
    tri.p1 = {-500, -500, 6.5};
    tri.p2 = { 500, -500, 4.5};
    tri.p3 = {   0,  500, 4.5};
    Rule1Init();
    for(int i = 0; i < 5; i++){
        CellWalk();
        CopyBuffer();
    }
    printf("{\n");    
    StartObjects();    
    //WriteTriangle(tri, {245, 201, 105}, 0, true);
    DrawGrid();
    BuildDisplay();
    //WriteCube({0, 0, 0}, 0.610865, -0.436332, {10,50,0}, false);
    //WriteCube({-1.25, 1.25, 2}, M_PI/8, {0,255,255}, true);
    //WriteCube({1, -1.25, 2}, M_PI/2 + M_PI/8, {255,0,255}, false);
    EndObjects();
    WriteLights();
    WriteCamera();
    printf(",\n");
    WriteSettings();
    printf("}\n");    
}