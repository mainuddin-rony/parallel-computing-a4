#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "Vector3.h"
#include "Solver.h"
#include "VisTIFF.h"
#include "rgbtiff.h"

#define abs(a) (((a) < 0) ? -(a) : (a))

typedef unsigned char bool;


float g = 9.81f;  // assume Planet Earth gravity

float LX; // length of domain along x
float LZ; // length of domain along z

int nbX; // number of elements/cells along x, including boundary cells
int nbZ; // number of elements/cells along z, including boundary cells

float radius; // water "drop" cylinder radius
float h1 = 5; // water "drop" cylinder height
float xC1; // water "drop" cylinder center x coord
float zC1; // water "drop" cylinder center z coord


float h0;   // initial height for water outside the water drop

int timeStep;   // current time step number
float t;        // current simulation time
float dt;       // change in simulation time between timesteps


int nbPoints; // total number of elements
int nbQuads;
float Lx;
float Lz;
float dx; // element x dimension
float dz; // element z dimension

// Flux along X
float* F1;
float* F2;
float* F3;

// Flux along Z
float* G1;
float* G2;
float* G3;

float* q1; // array of water heights       (y axis), including boundary elements
float* q2; // array of water velocities    (y axis), including boundary elements
float* q3; // array of water accelerations (y axis), including boundary elements


 
void initialize(float LX, float nX, float LZ, float nZ){
  
    timeStep = 0;
    t = 0.0;
    dt = 0.007;

    dx = LX / (nX-1);
    dz = LZ / (nZ-1);

    h0 = 0.5;
    h1 = 5;

    radius = LX/10.0;

    dt = 0.3 * dx/sqrt(g * h1); // PJR: made dt a function of resolution and max height.
    printf("dt = %f\n", dt);    // This avoids numerical instability at higher resolutions.


    /* Accounting for guard elements. Boundary elements are already included in nX, nY*/
    nbX = nX+2;
    nbZ = nZ+2;
    Lx = LX+dx; 
    Lz = LZ+dz; 
    nbPoints = nbX*nbZ;
    nbQuads = (nbX-1)*(nbZ-1);




    xC1 = LX/3;
    zC1 = Lz/3;
    //   xC2 = 2*LX/3;
    //   zC2 = 2*Lz/3;
    radius = LX/10;


    F1 = malloc(nbPoints *sizeof(float)); //new float[nbPoints];
    F2 = malloc(nbPoints *sizeof(float)); //new float[nbPoints];
    F3 = malloc(nbPoints *sizeof(float)); //new float[nbPoints];
    G1 = malloc(nbPoints *sizeof(float)); //new float[nbPoints];
    G2 = malloc(nbPoints *sizeof(float)); //new float[nbPoints];
    G3 = malloc(nbPoints *sizeof(float)); //new float[nbPoints];

    q1 = malloc(nbPoints *sizeof(float)); //new float[nbPoints];
    q2 = malloc(nbPoints *sizeof(float)); //new float[nbPoints];
    q3 = malloc(nbPoints *sizeof(float)); //new float[nbPoints];


    int i = 0;

    for (float z = -dz ; z <= Lz ; z += dz){
        for (float x = -dx; x <= Lx ; x += dx){

            float dist1 = (x-xC1)*(x-xC1) + (z-zC1)*(z-zC1);
            //dist1 = sqrt(dist1);

            if (dist1 <= radius * radius){
                q1[i] = h1;
            } else {
                q1[i] = h0;
            }

            q2[i] = 0;
            q3[i] = 0;

            i++;
        }
    }
}



void destroy(){

    free(F1);
    free(F2);
    free(F3);

    free(G1);
    free(G2);
    free(G3);
    
    free(q1);
    free(q2);
    free(q3); 
}





// Harten entropy fix
float harten_phi(float lambda){
  // empirical value
  float epsilon = 2;
  if (abs(lambda) >= epsilon)
    return abs(lambda);
  else
    return(lambda*lambda + epsilon*epsilon)/(2*epsilon);
}



/* Solve the riemann problem on X */
Vector3 riemannX(Vector3 *qL, Vector3 *qR){

  float hL = qL->x;
  float uL = qL->y/qL->x;
  float vL = qL->z/qL->x;
  float hR = qR->x;
  float uR = qR->y/qR->x;
  float vR = qR->z/qR->x;

  float hBar = 0.5*(hL+hR);
  float uTilde = (sqrt(hL)*uL+sqrt(hR)*uR)/(sqrt(hL)+sqrt(hR));
  float vTilde = (sqrt(hL)*vL+sqrt(hR)*vR)/(sqrt(hL)+sqrt(hR));
  float cTilde = sqrt(g*hBar);

  Vector3 r1={1, uTilde-cTilde, vTilde};
  Vector3 r2={0, 0, 1};
  Vector3 r3={1, uTilde+cTilde, vTilde};

  Vector3 alpha, lambda;
  Vector3 delta = *qR;
  //delta -= qL;
  minusEquals(&delta, *qL);
  Vector3 w;
  alpha.x = ((uTilde+cTilde)*delta.x-delta.y)/(2*cTilde);
  alpha.y = -vTilde*delta.x+delta.z;
  alpha.z = (-(uTilde-cTilde)*delta.x+delta.y)/(2*cTilde);
  lambda.x = uTilde-cTilde;
  lambda.y = uTilde;
  lambda.z = uTilde+cTilde;
  w =  multiply(r1, harten_phi(lambda.x)*alpha.x);
  plusEquals(&w, multiply(r2, harten_phi(lambda.y)*alpha.y));
  plusEquals(&w, multiply(r3, harten_phi(lambda.z)*alpha.z));
  w = multiply(w, 0.5);

  Vector3 F;
  F.x = 0.5*(qL->y+qR->y);
  F.y = 0.5*(qL->y*qL->y/qL->x+0.5*g*qL->x*qL->x + qR->y*qR->y/qR->x+0.5*g*qR->x*qR->x);
  F.z = 0.5*(qL->y*qL->z/qL->x+qR->y*qR->z/qR->x) ;
  //F = F - w;
  minusEquals(&F,w);
  //lambdaMax = max(lambda);
  return F;
}

/* Solve the riemann problem on Y */
Vector3 riemannY(Vector3 * qL,Vector3 * qR)
{
  float hL = qL->x;
  float uL = qL->y/qL->x;
  float vL = qL->z/qL->x;
  float hR = qR->x;
  float uR = qR->y/qR->x;
  float vR = qR->z/qR->x;
  float hBar = 0.5*(hL+hR);
  float uTilde = (sqrt(hL)*uL+sqrt(hR)*uR)/(sqrt(hL)+sqrt(hR));
  float vTilde = (sqrt(hL)*vL+sqrt(hR)*vR)/(sqrt(hL)+sqrt(hR));

  float cTilde = sqrt(g*hBar);

  Vector3 r1={1, uTilde, vTilde-cTilde};
  Vector3 r2={0, -1, 0};
  Vector3 r3={1, uTilde, vTilde+cTilde};

  Vector3 delta = *qR;
  //delta -= qL;
  minusEquals(&delta, *qL);
    
  Vector3 alpha;
  Vector3 lambda;
  Vector3 w;
  alpha.x = ((vTilde+cTilde)*delta.x-delta.z)/(2*cTilde);
  alpha.y = uTilde*delta.x-delta.y;
  alpha.z = (-(vTilde-cTilde)*delta.x+delta.z)/(2*cTilde);
  lambda.x = vTilde-cTilde;
  lambda.y = vTilde;
  lambda.z = vTilde+cTilde;
  w = multiply(r1, harten_phi(lambda.x)*alpha.x);
  plusEquals(&w, multiply(r2, harten_phi(lambda.y)*alpha.y));
  plusEquals(&w, multiply(r3, harten_phi(lambda.z)*alpha.z));
  w = multiply(w, 0.5);



  Vector3 G;
  G.x = 0.5*(qL->z+qR->z);
  G.y = 0.5*(qL->y*qL->z/qL->x+qR->y*qR->z/qR->x) ;
  G.z = 0.5*(qL->z*qL->z/qL->x+0.5*g*qL->x*qL->x + qR->z*qR->z/qR->x+0.5*g*qR->x*qR->x);

  //G = G - w;
  minusEquals(&G, w);
  
  return G;

  //lambdaMax = max(lambda);
}


// reset boundary conditions
void boundary(
        float * q1,
        float * q2,
        float * q3,
        
        int nbX,
        int nbZ
    )
{

  int j0;
  int i0;

  {
    /* Reflection on the boundary */
    for (int j = 1; j < nbZ-1; j++)
    {
      i0 = 1;
      q2[i0+j*nbX] = -q2[i0+1+j*nbX];
      i0 = nbX-2; //NOTE: not the last column
      q2[i0+j*nbX] = -q2[i0-1+j*nbX];
    }
    for (int i = 1; i < nbX-1; i++)
    {
      j0 = 1;
      q2[i+j0*nbX] = -q2[i+(j0+1)*nbX];
      j0 = nbZ-2;
      q2[i+j0*nbX] = -q2[i+(j0-1)*nbX];
    }
  }
}


void addDrops(
     float t,
     float  xC1,
     float zC1,
     float  dx,
     float  dz,
     
     int nbX,
     int nbZ,
     
     float  radius,
     float  height,
     
     float  *q1,
     float  *q2,
     float  *q3
     ) {
  
//  #pragma unused (nbZ)   

  float precision = 1e-2f;
  float x;
  float z;
  float dist;
  /* random */
  float tmp = t - (int) t;
  if (tmp < precision)
  {
    int iC = xC1/dx;
    int jC = zC1/dz;
    int iWidth = radius/dx;
    int jWidth = radius/dz;
    for (int i = iC-iWidth; i<= iC+iWidth; i++)
      for (int j = jC-jWidth; j<= jC+jWidth; j++)
      {
        x = i*dx;
        z = j*dz;
        dist = (x-xC1)*(x-xC1) + (z-zC1)*(z-zC1);
        //dist = sqrt(dist);

        if (dist <= radius * radius)
        {

          q1[i+j*nbX] = height;
          q2[i+j*nbX] = 0;
          q3[i+j*nbX] = 0;
        }
      }
  }
}




/* Compute flux and update Q */
void updateWater(){

    t += dt;
    timeStep++;
    
    
    Vector3 qL={0,0,0}, qRX={0,0,0}, qRY={0,0,0};
    for (int j =0; j < nbZ-1; j++){
        for (int i =0; i < nbX-1; i++){
        
          // The fields of the Vector3 struct are named x,y, and z,
          // but the qL, qRX, and qRY vectors hold height, velocity,
          // and acceleration.  
        
          qL.x = q1[i+j*nbX]; // center
          qL.y = q2[i+j*nbX];
          qL.z = q3[i+j*nbX];
  
          qRX.x = q1[i+1+j*nbX]; // east
          qRX.y = q2[i+1+j*nbX];
          qRX.z = q3[i+1+j*nbX];
  
          qRY.x = q1[i+j*nbX+nbX]; // south
          qRY.y = q2[i+j*nbX+nbX];
          qRY.z = q3[i+j*nbX+nbX];

          Vector3 FTmp = riemannX(&qL, &qRX);
          Vector3 GTmp = riemannY(&qL, &qRY); // should be named riemannZ?

          F1[i+j*nbX] = FTmp.x;
          F2[i+j*nbX] = FTmp.y;
          F3[i+j*nbX] = FTmp.z;

          G1[i+j*nbX] = GTmp.x;
          G2[i+j*nbX] = GTmp.y;
          G3[i+j*nbX] = GTmp.z;
        }
    }

    for (int j =0; j < nbZ-1; j++){
        for (int i =0; i < nbX-1; i++){
            if (i > 0){
              q1[i+j*nbX] = q1[i+j*nbX] - dt/dx*(F1[i+j*nbX]-F1[i-1+j*nbX]);
              q2[i+j*nbX] = q2[i+j*nbX] - dt/dx*(F2[i+j*nbX]-F2[i-1+j*nbX]);
              q3[i+j*nbX] = q3[i+j*nbX] - dt/dx*(F3[i+j*nbX]-F3[i-1+j*nbX]);
            }

            if (j > 0){
              q1[i+j*nbX] = q1[i+j*nbX] - dt/dz*(G1[i+j*nbX]-G1[i+(j-1)*nbX]);
              q2[i+j*nbX] = q2[i+j*nbX] - dt/dz*(G2[i+j*nbX]-G2[i+(j-1)*nbX]);
              q3[i+j*nbX] = q3[i+j*nbX] - dt/dz*(G3[i+j*nbX]-G3[i+(j-1)*nbX]);
            }
         }
      }

    boundary(q1, q2, q3, nbX, nbZ);
//    addDrops(t, xC1, zC1, dx, dz, nbX, nbZ, radius, h1, q1, q2, q3);
}


float arrayMax(float * arr, int length){

    float max=fabs(arr[0]);
    for(int i=0; i<length; i++){
    
    	float v = fabs(arr[i]);
    	
        max = (v > max) ? v : max;
    }
    
    return max;
}


void runUntilDone(float threshold, int saveInterval){

    float maxVelocity;
    int ts;
    
    //dt = _dt; // global
    
    if(saveInterval != 0)
        writeTIFF("water");

    do{
        updateWater();
        ts=getTimeStep();

         if( saveInterval != 0 && (ts % saveInterval) == 0)
             writeTIFF("water");
            
        maxVelocity=arrayMax(q2, nbPoints);
        //printf("%f\n", maxVelocity);
        
    } while( maxVelocity > threshold);
 
    if( saveInterval != 0 && (ts % saveInterval) != 0)
        writeTIFF("water");
}








float * getWaterHeights(){

    return q1;
}

void getWaterDimensions(int *nbXp, int *nbZp){

    *nbXp=nbX;
    *nbZp=nbZ;
}

void getElementDimensions(float *dxp, float  *dzp){

    *dxp = dx;
    *dzp = dz;
}


int getTimeStep(){

    return timeStep;
}

