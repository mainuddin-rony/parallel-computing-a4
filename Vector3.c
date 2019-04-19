#include <stdlib.h>
#include <math.h>

#include "Vector3.h"


Vector3 plusEquals(Vector3 *v1, Vector3 v2){

    v1->x += v2.x;
    v1->y += v2.y;
    v1->z += v2.z;
    
    return *v1;
}



Vector3 minusEquals(Vector3 *v1, Vector3 v2){

    v1->x -= v2.x;
    v1->y -= v2.y;
    v1->z -= v2.z;
    
    return *v1;
}


Vector3 multiply(Vector3 v, float a){

    v.x *= a;
    v.y *= a;
    v.z *= a;

    return v;
}


Vector3 crossProduct(const Vector3 v1, const Vector3 v2){
    
    Vector3 crp;
    
    crp.x = v1.y*v2.z   -   v1.z*v2.y;
    crp.y = -v1.x*v2.z  +   v1.z*v2.x;
    crp.z = v1.x*v2.y   -   v1.y*v2.x;
   
    return crp;
}

float dotProduct(const Vector3 v1, const Vector3 v2){
  
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}



float magnitude(Vector3 v){

    return sqrt( v.x*v.x + v.y*v.y + v.z*v.z);
}


void normalize(Vector3 *v){
    
    float mag = magnitude(*v);
    
    v->x /= mag;
    v->y /= mag;
    v->z /= mag;
}  
