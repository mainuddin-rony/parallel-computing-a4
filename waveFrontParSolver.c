#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

#include "Solver.h"
#include "Vector3.h"

#include "waveFrontParSolver.h"
#include "state_array.h"
#include "barrier.h"
#include "VisTIFF.h"


// gSaveInterval has static scope, making it visible to any code in this file. It's not a true global,
// since other files can't see it. 
int gSaveInterval=0;
float* tile_q2;
int loop_cond = 0;
int thread_count;
float gThreshold = 0.0;
// nbX ( # Cols) and nbZ (# Rows) are true globals defined in Solver.h, and determine the 
// resolution of the simulation.
//
// You'll also need q2, the velocities array, which is also found in Solver.h



typedef struct {
      
    int tid;
    
    int startWaterRow;
    int endWaterRow;
    
    int startWaterCol;
    int endWaterCol;
    
    barrier_t * barrier;
    int state_index; // index in state array
} jobSpec;

void * WFdoWork( void * a);
void WFupdateWater(int startRow, int endRow, int startCol, int endCol);
float WFarrayMax(float * arr, int startRow, int endRow, int startCol, int endCol, int totalCols);
void * barrier_function(void * a);



/** Compute new values for simulation timesteps until the maximum velocity found in
 *  the q2[] array is less than the given threshold. This implementation uses wavefront
 *  parallelism with an array of threads with the specified number of rows and columns.
 *  
 *  If saveInterval is non-zero, a TIF snapshot of the initial water state is written to
 *  disk before any calculation is performed. After that, a snapshot will be written out
 *  every "saveInterval" steps. A snapshot of the final water state is also written. For 
 *  example, if a simulation has 23 timeSteps and saveInterval==10, we expect 
 *  water.0000.tif, water.0010.tif, water.0020.tif, and water.0022.tif.
 *  
 *  If saveInterval is zero, no snapshots are written at all.
 *  
 *  @param nThreadRows number of rows in the thread array.
 *  @param nThreadCols number of columns in the thread array.
 *  @param threshold the velocity threshold
 *  @param saveInterval controls how frequently to save TIFF snapshots
 */
void WFRunUntilDone(int nThreadRows, int nThreadCols, float threshold, int saveInterval){

    gSaveInterval = saveInterval;
    //TODO: Write me.
    gThreshold = threshold;

    // After initializing local variables, the state_array, the barrier, etc.,
    // you'll a loop ( or nested loops) that iterate over the thread array
    // and create a jobSpec struct for each thread. 
    //
    // When filling out the jobSpec, you'll be assigning each thread its own
    // "tile" -- a rectangular subset of the simulation domain. When you do this,
    // make sure that no tile extends past row nbZ-2, or column nbX-2. nbZ and nbX
    // are true globals declared in Solver.c/h, and denote the number of array 
    // rows and columns used in the simulation.
    //
    // Also, since the wavefront runs from the north west to south east corner,
    // the threads map to the state_array a little differently. It's a minor change,
    // though.
    //
    // After you've launched your threads, call triggerWave() to kick off the
    // wavefront.
    // 
    // The main thread doesn't need to do anything at all, so it should not 
    // participate in the barrier. It should do the usual join() loop.
    //
    // After writing a final snapshot (if appropriate) this function just 
    // frees memory and ends.
//    printf("In WFRunUntilDone\n");

    int num_state_rows = nThreadRows + 1;
    int num_state_cols = nThreadCols + 1;

    createStateArray(num_state_rows, num_state_cols);

    int thread_arr_len = nThreadRows * nThreadCols;
//    printf("Length of thread array %d\n", thread_arr_len);

    pthread_t * thread_arr = malloc(thread_arr_len * sizeof(pthread_t));

    barrier_t barrier;

    barrier_init(&barrier, thread_arr_len + 1, barrier_function);

    int thrd_id = 0;
    int nRow = nbX - 1;
    int nCol = nbZ - 1;
    int rowAssigned, colAssigned = 0;
    int rowPerThread = round((float)nRow/(float)nThreadRows);
    int colPerThread = round((float)nCol/(float)nThreadCols);
    int rowStart = 0;
    int colStart = 0;
    tile_q2 = malloc(thread_arr_len * sizeof(float));

    printf("Number of Rows and columns per thread is %d, %d\n", rowPerThread, colPerThread);


    for(int i=0; i<num_state_rows; i++){
        for(int j=0; j<num_state_cols; j++){
            if((i!=0) && (j!=0)){
                int idx = index(i,j);
//                printf("Creating thread for %d\n", idx);

                jobSpec * args = malloc(sizeof(jobSpec));

                args->tid = thrd_id; //need to think it
                args->barrier = &barrier;
                args->state_index = idx;

                args->startWaterRow = rowStart;
                args->startWaterCol = colStart;
//                printf("Initial rowStar %d, colStart %d\n", rowStart,colStart);

//                printf("Row Start %d\n", rowStart);

                if (rowStart + rowPerThread <= nRow){
                    if (i == num_state_rows - 1){
                        args->endWaterRow = nRow - 1;
//                        printf("Here %d\n", nRow - 1);

                    }
                    else{
                        args->endWaterRow = rowStart + rowPerThread - 1;
                    }

//                  rowAssigned += rowPerThread;
                }
                else {
                    args->endWaterRow = nRow - 1;
                }

                if (colStart + colPerThread <= nCol) {
//                    printf("Inside if colStart %d\n", colStart);
                    if (j == num_state_cols -1){
                        args->endWaterCol = nCol - 1;
                        colAssigned = nCol;
                    }
                    else{
                        args->endWaterCol = colStart + colPerThread - 1;
                        colAssigned += colPerThread;
                        colStart += colPerThread;
                    }

                }
                else {
//                    printf("Else col start\n");
                    args->endWaterCol = nCol - 1;
                    colAssigned = nCol;
                }

                if (colAssigned == nCol){
                    colStart = 0;
                    rowStart += rowPerThread;
//                    rowAssigned += rowPerThread;
                }

                printf("Row Start %d, Row End %d, Col start %d, Col end %d\n", args->startWaterRow, args->endWaterRow, args->startWaterCol, args->endWaterCol);

                printf("Going to create threads for %d\n", thrd_id);
                pthread_create(&(thread_arr[thrd_id]), NULL, WFdoWork, args);
                thrd_id++;
            }
        }
    }

//    printf("Creating q2 array for tiles\n");



    triggerWave();

    printf("Joining Thread functions\n");

    for (int i=0; i<thread_arr_len; i++){

        pthread_join(thread_arr[i], NULL);
    }

    destroyStateArray();
    barrier_destroy(&barrier);


//    return EXIT_SUCCESS;

 }

/** This thread function computes new simulation values for a rectangular subset 
 *  ( or "tile") of the simulation domain. This function includes a loop that will 
 *  continue until computing timesteps until the maximum velocity found in the 
 *  the global q2 array is less than the threshold specified by the user. 
 *
 *  We're using wavefront synchronization, as with a3, but due to the nature of 
 *  the dependencies in the calculation, the wavefront proceeds from upper left 
 *  (row 0, column 0) to the lower right corner.
 *
 */
void * WFdoWork( void * a){

    //TODO: Write me.
    
    // After the usual unpacking of the arguments struct
    // the rest of this function is inside a loop that runs
    // until the barrier function decides it is time to stop.
    //
    // You'll want to call waitOnNeighbor() on the north and
    // west neighbors before doing anything else, since this
    // wavefront proceeds from the northwest corner down to the
    // southeast. 
    //
    // Next, call WFupdateWater() to compute new values for this
    // thread's "tile", or piece of the simulation domain. 
    //
    // q2 is an array declared in Solver.c/h, and is directly 
    // available to this function. Like all the other variables
    // described in Solver.h, it is a true global variable. 
    // In order to help the barrier function decide when to stop,
    // this thread should compute the maximum q2 value found 
    // in its tile, and make this max value available to the barrier 
    // function.
    //  
    // Lastly, use the state_array like in a3, to signal other threads
    // that this one has completed its tile. Then go to the barrier
    // and wait for the wave for this timeStep to be completed.
    //
    // repeat until done.


    jobSpec * args = (jobSpec * ) a;
    int idx = args->state_index;
    barrier_t *barr = args->barrier;
    int tid = args->tid;

    printf("Inside Thread Function for thread %d\n", tid);

    int startRow = args->startWaterRow;
    int endRow = args->endWaterRow;

    int startCol = args->startWaterCol;
    int endCol = args->endWaterCol;

    printf("Get sub domain from Row: %d to Row: %d to Col: %d Col: %d\n", startRow, endRow, startCol, endCol);

    free(args);
    args = NULL; a = NULL;
//    int init_ts = -1;

    printf("Loop condition value is %d\n", loop_cond);

    while(loop_cond == 0){
        int n_idx = N(idx);
        int w_idx = W(idx);
//        int ts = getStateArray()[idx].timeStep;
        waitOnNeighbor(n_idx, timeStep);
        waitOnNeighbor(w_idx, timeStep);

        printf("Updating water flow\n");

        WFupdateWater(startRow, endRow, startCol, endCol);

        float curr_max = fabs(0.0);

        for (int i = startRow; i <= endRow; i++ ){
            for (int j = startCol; j<= endCol; j++){
                int q_idx = i * nbZ + j;
                if (fabs(q2[q_idx]) > fabs(curr_max)) {
                    curr_max = q2[q_idx];
                }
            }
        }

        tile_q2[tid] = curr_max;

        pthread_mutex_lock(&getStateArray()[idx].lock);


        getStateArray()[idx].timeStep += 1;
        pthread_cond_broadcast(&getStateArray()[idx].cv);


        pthread_mutex_unlock(&getStateArray()[idx].lock);


        barrier_wait(barr, NULL);
    }



    return NULL;
}



 
/** Compute flux and update Q. The original code is found in Solver.c, but
  * this version operates over a "tile" -- a rectangular subset of the 
  * simulation domain. Several arrays defined in Solver.c are modified 
  * here. 
  *
  * @param startRow The first row of the tile
  * @param endRow The last row of the tile
  * @param startCol The first column of the tile
  * @param endCol The last column of the tile
  *
  */
void WFupdateWater(int startRow, int endRow, int startCol, int endCol){
    
    Vector3 qL={0,0,0}, qRX={0,0,0}, qRY={0,0,0};
    
    for (int j =startRow; j <= endRow; j++){
        for (int i =startCol; i <= endCol; i++){
        
            // The fields of the Vector3 struct are named x,y, and z,
            // but the qL, qRX, and qRY vectors hold height, velocity,
            // and acceleration in their x,y, and z fields, respectively.  

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

            //X Flux
            F1[i+j*nbX] = FTmp.x;
            F2[i+j*nbX] = FTmp.y;
            F3[i+j*nbX] = FTmp.z;

            //Y Flux
            G1[i+j*nbX] = GTmp.x;
            G2[i+j*nbX] = GTmp.y;
            G3[i+j*nbX] = GTmp.z;
        }
    }
    
 
    for (int j =startRow; j <= endRow; j++){
        for (int i =startCol; i <= endCol; i++){
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
}


/** This function is executed by the last thread to enter the barrier. It is executed under
 *  the protection of the barrier mutex, which guarantees that it runs before the other 
 *  threads have started running. While doWork() does the heavy lifting, this function 
 *  performs crucial housekeeping functions between timeSteps.
 *
 *  The function does several things:
 *
 *  It determines whether the simulation is finished by
 *  comparing the maximum velocity found in q2[] with the threshold velocity. It must then
 *  have some way of ensuring the other threads terminate cleanly.
 *
 *  It increments the global variables t and timeStep, both declared in Solver.c
 *
 *  If appropriate, it calls writeTIFF(), which saves a TIF image snapshot of the water.
 *
 *  It calls triggerWave(), defined in state_array.c, which kicks off the next wavefront.
 */
void * barrier_function(void * a){
		
    //TODO: Write me.
    // Tell the threads to stop if the max velocity in q2 is < gThreshold.
    //    The other threads should have already computed the max value for their tile.

    
    // update simulation time and timeStep (from solver.c)
    float best_max = arrayMax(tile_q2, thread_count);

    if (best_max < gThreshold) {
        loop_cond = 1;
    }
 	t += dt;
	timeStep++;
    
    boundary(q1, q2, q3, nbX, nbZ);
    
    if( gSaveInterval != 0 && (getTimeStep() % gSaveInterval) == 0){    
    	writeTIFF("water");
    }
    
    // Defined in state_array.c.  
    triggerWave();
     
    return NULL;
}







