#include <stdio.h>
#include <stdlib.h>


#include "VisTIFF.h"
#include "Solver.h"
#include "rgbtiff.h"
#include "pixel.h"
#include "Vector3.h"

#define MIN(A, B) (((A) < (B))? (A) : (B))

pixel computeColor(float * heights, int row, int col, int nrows, int ncols, float dx, float dz){

    pixel thePixel;

    pixel ambientColor = { 200, 200, 200};
    float ambientCoeff = 0.2;
    
    
    pixel diffuseColor = { 200, 200, 255};
    float diffuseCoeff = 0.8;
    
    Vector3 L = {0,0,1};
//     Vector3 normal = {  heights[row * ncols + col] - heights[row * ncols + (col+1)],
//                         heights[row * ncols + col] - heights[(row +1) * ncols + col],
//                         1
//                      };
 
     Vector3 normal = {  dx,
                         dz,
                         heights[row * ncols + col] - heights[(row + 1) * ncols + (col+1)]
                     };

 
                 
    normalize(&normal);            

    
    float intensity = 1 - dotProduct(L, normal);

    thePixel.red =   MIN(255, ambientCoeff * ambientColor.red    + diffuseCoeff * diffuseColor.red   * intensity);
    thePixel.green = MIN(255, ambientCoeff * ambientColor.green  + diffuseCoeff * diffuseColor.green * intensity);
    thePixel.blue =  MIN(255, ambientCoeff * ambientColor.blue   + diffuseCoeff * diffuseColor.blue  * intensity);
    
    return thePixel;
}


/**  Write a visualization of the current timeStep to disk as a TIFF image file. The
 *   file name will be formed from the given basefilename, the current timeStep number,
 *   and a tif extension. For example, if basefilename == "water", timestep 12 is
 *   written to a file named "water.0012.tif".
 */
int writeTIFF(char * basefilename){
    //int nbX, nbZ;
    //float dx, dz;
    int timeStep = getTimeStep();
    float * waterHeights = getWaterHeights();
    //getWaterDimensions(&nbX, &nbZ);
    //getElementDimensions(&dx, &dz);
    int numPixels = nbX*nbZ;
    char filename[256];
    pixel * pixels = calloc(numPixels, sizeof(pixel)); // initializes memory to 0
    
    snprintf(filename, 255, "%s.%04d.tif", basefilename, timeStep);

    for( int r = 0; r < nbX-1; r++){
        for(int c = 0; c < nbZ-1; c++){
        
            pixels[r*nbZ + c] = computeColor(waterHeights, r, c, nbX, nbZ, dx, dz);
        }
    }

    int status = writeRGBtiff(filename, (unsigned char *) pixels, nbX, nbZ);        
    
    return status;
}
