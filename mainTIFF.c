#include <stdio.h>
#include <stdlib.h>

#include "Vector3.h"
#include "Solver.h"

#include "waveFrontParSolver.h"
#include "VisTIFF.h"
#include "timing.h"


void usage();

// a4 <num grid rows> <num grid columns> <num thread rows> <num thread cols> <velocity threshold> [saveInterval]
int main(int argc, char* argv[])
{

    float LX = 7; // Domain dimensions
    float LZ = 7;
    
    if (argc != 6 && argc != 7){
    
        usage();
    }
    
    // grid dimensions
    int nX = atoi(argv[1]);
    int nZ = atoi(argv[2]);
    
    int nThreadRows = atoi(argv[3]);
    int nThreadCols = atoi(argv[4]);
    
    float threshold = atof(argv[5]);
    
    int saveInterval = 50;
    
    if (argc == 7){
    
        saveInterval = atoi(argv[6]);
    }

    initialize(LX, nX, LZ, nZ);

    double then, now;

    if(nThreadCols == 1 && nThreadRows == 1){
    
        then = currentTime();
        runUntilDone(threshold, saveInterval);
        now = currentTime();
        printf("%%%%%% serialTime %.3lf seconds\n", now - then);
    } else {
    
        then = currentTime();
        WFRunUntilDone(nThreadRows, nThreadCols, threshold, saveInterval);
        now = currentTime();
        printf("%%%%%% parallelTime %.3lf seconds\n", now - then);
    }
    
    printf("It took %.3f seconds of wall-clock time to calculate %.3f seconds of simulation time.\n", now-then, t);
        

    return 0;
}

void usage(){

    printf("Usage: a4 <num water rows> <num water columns> <num thread rows> <num thread cols> <velocity threshold> [saveInterval]\n");
    printf("        where <num water rows> <num water columns> determine the resolution of the simulation,\n");
    printf("        and <num thread rows> <num thread cols> determine the number of threads used in the wavefront,\n");
    printf("        and <velocity threshold> determines the water velocity at which the simulation terminates,\n");
    printf("        and <saveInterval> is the number of timesteps elapsed between .tif image snapshots of the simulation.\n");
    printf("\n");
    printf("        if both <num thread rows> and <num thread cols> are 1, then the serial code is run.\n");
    printf("        if <saveInterval> is not specified, the default value of 50 is used.\n");
    printf("        if <saveInterval> is specified as 0, no tif images are written.\n");
    
    exit(1);
}
