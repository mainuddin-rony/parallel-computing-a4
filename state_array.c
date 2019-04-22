#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "state_array.h"

/** This C code contains several functions that work with a "state array" that is declared
 *  statically in this file. That is, there is only one state array, and the functions work
 *  on that single instance. Of course, you'll want to call createStateArray() before doing
 *  much else.
 *
 *  Each element of the state array is of type "state", as found in state_arr.h.  Although
 *  it is useful to think of the array as two dimensional, C really only supports 1-D arrays.
 *  However, we can map (row, column) coordinates to a 1D "index" very easily when necessary.
 *  But if we want to "visit" every element (like for initialization), it's simpler to stick
 *  with the 1D index.
 *
 *  Elements in the first column or in the top row are considered boundary elements, and 
 *  are treated specially by functions like triggerWave(). The diagram below shows the position
 *  of the boundary elements for a 6 x 6 array, and the cardinal directions (N,S,E,W).
 *
 *          N
 *
 *      B B B B B B
 *      B * * * * *
 *      B * * * * *
 * W    B * * * * *     E
 *      B * * * * *
 *      B * * * * *            
 *
 *          S
 */



// These variables have "static" scope, meaning they are only visible
// inside this file. Currently, only one state array can be created.
state * state_arr=NULL;  // The state array.
int nrows, ncols;        // The number of rows and columns in the state array.
int arr_len = 0;         // The total number of elements in the state array.




/** Allocate a new state array with the specified number of rows and columns. For
 *  each element, the "condition variable" and "mutex" data members are initialized, and
 *  the timeStep is set to -1.
 *
 *  @param _nrows number of rows in the new array
 *  @param _ncols number of columns in the new array 
 */
void createStateArray(int _nrows, int _ncols){

    nrows = _nrows;
    ncols = _ncols;

    arr_len = nrows * ncols;
    
    state_arr = malloc(arr_len * sizeof(state));
    
    for(int i=0; i < arr_len; i++){
    
        pthread_cond_init(&(state_arr[i].cv), NULL);
        pthread_mutex_init(&(state_arr[i].lock), NULL);
        
        state_arr[i].timeStep = -1;
    }
}

/** Return a reference to the state array.*/
state * getStateArray(){

    return state_arr;
}

/** Return the number of rows in the state array.*/
int getNumRows(){

    return nrows;
}

/** Return the number of columns in the state array.*/
int getNumCols(){

    return ncols;
}



/** Destroy all mutex and condition variables in each element, and then free the memory used
 *  by the array.
 */
void destroyStateArray(){
    
    for(int i=0; i < arr_len; i++){
    
        pthread_cond_destroy(&(state_arr[i].cv));
        pthread_mutex_destroy(&(state_arr[i].lock));
    }

    free(state_arr);
    state_arr = NULL;
}



/** Increment the timeStep field for all boundary elements and broadcast on their CVs.
  */
void triggerWave(){

    //TODO: Write me.
    // This is similar to initBorders() from a3, but our boundary elements are 
    // now on the north and west edges, and we're incrementing, not setting to 1.
    // NOTE: Don't increment the corner twice!

    printf("Triggering the wave\n");

    for (int i = 0; i <  nrows; i ++){
        int b_idx = index(i, 0);
        // printf("Initializing border idx %d\n", b_idx);
        pthread_mutex_lock(&state_arr[b_idx].lock);
        state_arr[b_idx].timeStep += 1;
        pthread_cond_broadcast(&state_arr[b_idx].cv);
        pthread_mutex_unlock(&state_arr[b_idx].lock);
    }

    for (int i = 1; i <  ncols; i ++){
        int b_idx = index(0, i);
        // printf("Initializing border idx %d\n", b_idx);
        pthread_mutex_lock(&state_arr[b_idx].lock);
        state_arr[b_idx].timeStep += 1;
        pthread_cond_broadcast(&state_arr[b_idx].cv);
        pthread_mutex_unlock(&state_arr[b_idx].lock);
    }

}

/** Given a row and column, compute the index of the corresponding element of the state_array.
 *
 *  @param r the row coordinate
 *  @param c the column coordinate    
 *  @return the element index
*/
int index(int r, int c){

    return  r * ncols + c;
}

/** Given the index of an element, return the index of the north neighbor.
 *  @param index the element index
 *  @return the neighbor index
 */
int N(int index){

	return index - ncols;
}

/** Given the index of an element, return the index of the south neighbor.
 *  @param index the element index
 *  @return the neighbor index
 */
int S(int index){

	return index + ncols;
}

/** Given the index of an element, return the index of the east neighbor.
 *  @param index the element index
 *  @return the neighbor index
 */
int E(int index){

	return index + 1;
}

/** Given the index of an element, return the index of the west neighbor.
 *  @param index the element index
 *  @return the neighbor index
 */
int W(int index){

	return index - 1;
}

/**  
 *  Wait until the specified neighbor's timeStep field is equal to the 
 *  specified timeStep.
 *  @param index the neighbor element index
 *  @param timeStep the timeStep number we are waiting for.
 */
int waitOnNeighbor(int index, int timeStep){
	
    // TODO: Write me.
    //
    // Make sure you use the mutex!

    pthread_mutex_lock(&getStateArray()[index].lock);
    while (getStateArray()[index].timeStep == timeStep){
        pthread_cond_wait(&getStateArray()[index].cv, &getStateArray()[index].lock);
    }

//    getStateArray()[index].sum ;

    pthread_mutex_unlock(&getStateArray()[index].lock);
    

	return 0;
}
