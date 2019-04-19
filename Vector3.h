#ifndef VECTOR_3_H
#define VECTOR_3_H

typedef struct {

    float x;
    float y;
    float z;
} Vector3;


// This weird syntax lets me use alternative names for the fields of Vector3. But it
// causes complaints with initializers {1,2,3}
// typedef
// 	union {
// 		struct {
// 
// 			float x;
// 			float y;
// 			float z;
// 		};
// 	
// 		struct {
// 
// 			float h;  // height
// 			float v;  // velocity
// 			float a;  // acceleration
// 		};
// 	}	
// Vector3;




#ifdef __cplusplus
extern "C" {
#endif

Vector3 crossProduct(const Vector3 v1, const Vector3 v2);
float dotProduct(const Vector3 v1, const Vector3 v2);
void normalize(Vector3 *v);
Vector3 minusEquals(Vector3 *v1, Vector3 v2);
Vector3 plusEquals(Vector3 *v1, Vector3 v2);
Vector3 multiply(Vector3 v, float a);

#ifdef __cplusplus
}
#endif



// class Vector3
// {
//   public:
//     float x;
//     float y;
//     float z;
// 
//     Vector3();
//     Vector3(float x,float y,float z);
//     Vector3(const Vector3 & v);
// 
//     Vector3& operator= (const Vector3 & v);
// 
//     Vector3& operator+= (const Vector3 & v);
//     Vector3&  operator-= (const Vector3 & v);
//     Vector3&  operator*= (const float a);
//     Vector3&  operator/= (const float a);
// 
//     Vector3 operator+ (const Vector3 & v);
//     Vector3 operator- (const Vector3 & v);
//     Vector3 operator* (const float a);
//     Vector3 operator/ (const float a);
//     float& operator()(int i);
// 
// 
//     Vector3 crossProduct(const Vector3 & v);
//     void normalize();
//     float norm();
// };

#endif
