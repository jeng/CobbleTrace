#ifndef __MYMATH_H__
#define __MYMATH_H__

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

struct v2_t {
    double x;
    double y;
};

struct v3_t {
    double x;
    double y;
    double z;
};

struct v4_t {
    double x;
    double y;
    double z;
    double w;
};

struct m4x4_t {
    double data[4][4] = {0};
};

struct m3x3_t {
    double data[3][3] = {0};
};

extern double Inner(v2_t v, v2_t w);

inline v3_t
operator*(double A, v3_t B) {
    v3_t Result;

    Result.x = A*B.x;
    Result.y = A*B.y;
    Result.z = A*B.z;

    return(Result);
}

inline v4_t
operator*(v4_t v, m4x4_t m){
    v4_t result;
    result.x = v.x * m.data[0][0] + v.y * m.data[1][0] + v.z * m.data[2][0] + v.w * m.data[3][0];
    result.y = v.x * m.data[0][1] + v.y * m.data[1][1] + v.z * m.data[2][1] + v.w * m.data[3][1];
    result.z = v.x * m.data[0][2] + v.y * m.data[1][2] + v.z * m.data[2][2] + v.w * m.data[3][2];
    result.w = v.x * m.data[0][3] + v.y * m.data[1][3] + v.z * m.data[2][3] + v.w * m.data[3][3];
    return result;
}

inline v3_t
operator*(v3_t v, m3x3_t m){
    v3_t result;
    result.x = v.x * m.data[0][0] + v.y * m.data[1][0] + v.z * m.data[2][0];
    result.y = v.x * m.data[0][1] + v.y * m.data[1][1] + v.z * m.data[2][1];
    result.z = v.x * m.data[0][2] + v.y * m.data[1][2] + v.z * m.data[2][2];
    return result;
}


inline v3_t
operator*(v3_t B, double A) {
    v3_t Result = A*B;

    return(Result);
}

inline v3_t &
operator*=(v3_t &B, double A) {
    B = A * B;

    return(B);
}

inline v3_t
operator/(v3_t B, double A) {
    v3_t Result = (1.0f/A)*B;

    return(Result);
}

inline v3_t
operator/(double B, v3_t A) {
    v3_t Result =
    {
        B / A.x,
        B / A.y,
        B / A.z,
    };

    return(Result);
}

inline v3_t &
operator/=(v3_t &B, double A) {
    B = B / A;

    return(B);
}

inline v3_t
operator-(v3_t A) {
    v3_t Result;

    Result.x = -A.x;
    Result.y = -A.y;
    Result.z = -A.z;

    return(Result);
}

inline v3_t
operator+(v3_t A, v3_t B) {
    v3_t Result;

    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;

    return(Result);
}

inline v3_t &
operator+=(v3_t &A, v3_t B) {
    A = A + B;

    return(A);
}

inline v3_t
operator-(v3_t A, v3_t B) {
    v3_t Result;

    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z - B.z;

    return(Result);
}

inline v3_t &
operator-=(v3_t &A, v3_t B) {
    A = A - B;

    return(A);
}

inline v3_t
operator*(v3_t A, v3_t B) {
    v3_t Result = {A.x*B.x, A.y*B.y, A.z*B.z};

    return(Result);
}

inline v3_t
operator/(v3_t A, v3_t B) {
    v3_t Result = {A.x/B.x, A.y/B.y, A.z/B.z};

    return(Result);
}

inline v3_t
Lerp(v3_t A, float t, v3_t B) {
    v3_t Result = (1.0f - t)*A + t*B;

    return Result;
}

inline double
Slope(v2_t v, v2_t w){ 
    return (w.y - v.y)/(w.x - v.x);
}

inline v4_t
NormalizeByW(v4_t v){
    if (v.w != 0){
        v.x /= v.w;
        v.y /= v.w;
        v.z /= v.w;
    }
    return v;
}

inline float 
Magnitude(v3_t v){
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

inline v3_t
Normalize(v3_t v){
    double len = Magnitude(v);
    if (len != 0){
        v.x /= len;
        v.y /= len;
        v.z /= len;
    }
    return v;
}

inline float 
DotProduct(v3_t a, v3_t b){
    return a.x * b.x + a.y * b.y + a.z * b.z; 
}

inline v3_t
CrossProduct(v3_t a, v3_t b){
    //CX = AY * BZ - AZ * BY
    //CY = AZ * BX - AX * BZ
    //CZ = AX * BY - AY * BX
    return {a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x};
}

inline double Inner(v2_t v, v2_t w){
    return v.x * w.x + v.y * w.y;
}

#endif /*__MATH_H__*/
