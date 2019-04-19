#ifndef SOLVER_H
#define SOLVER_H

#include "Vector3.h"

#define PI 3.141592654f

#ifdef __cplusplus
extern "C" {
#endif

void initialize(float LX, float nX, float LZ, float nZ);
void updateWater();
void runUntilDone(float maxChange, int saveInterval);


float * getWaterHeights();
void getWaterDimensions(int *nbX, int * nbZ);
void getElementDimensions(float *dxp, float  *dzp);
int getTimeStep();
void destroy();

// We need to expose some of the gnarlier inner workings of the solver so this code is
// available to pSolver.c
float arrayMax(float * arr, int length);
Vector3 riemannX(Vector3 *qL, Vector3 *qR);
Vector3 riemannY(Vector3 * qL,Vector3 * qR);
void boundary(float * q1, float * q2, float * q3, int nbX, int nbZ);


// These variables represent the state of the solver, including the water heights, fluxes,
// etc.  Many of these get set by the initialize() function in solver.c


extern float g;  // gravity

extern float LX; // length of domain along x
extern float LZ; // length of domain along z

extern int nbX; // number of elements/cells along x, including boundary cells
extern int nbZ; // number of elements/cells along z, including boundary cells

extern float radius; // water "drop" cylinder radius
extern float h1; // water "drop" cylinder height
extern float xC1; // water "drop" cylinder center x coord
extern float zC1; // water "drop" cylinder center z coord


extern float h0;   // initial height for water outside the water drop
extern float gMaxChange;

extern int timeStep;   // current time step number
extern float t;        // current simulation time
extern float dt;       // change in simulation time between timesteps


extern int nbPoints; // total number of elements
extern int nbQuads;
extern float Lx;
extern float Lz;
extern float dx; // element x dimension
extern float dz; // element z dimension

// Flux along X
extern float* F1;
extern float* F2;
extern float* F3;

// Flux along Z
extern float* G1;
extern float* G2;
extern float* G3;

// The Y axis is the water height axis
extern float* q1; // array of water heights       (y axis), including boundary elements
extern float* q2; // array of water velocities    (y axis), including boundary elements
extern float* q3; // array of water accelerations (y axis), including boundary elements




#ifdef __cplusplus
}
#endif

#endif
