/*
 *  timing.h
 *  
 *
 *  Created by Philip Rhodes on 1/25/09.
 *  Copyright 2009 Philip J Rhodes. All rights reserved.
 *
 */

#ifndef TIMING_H
#define TIMING_H

#include <sys/time.h>


/* Subtract the `struct timeval' value 'then' from 'now',
   returning the difference as a float representing seconds
   elapsed.
*/
float elapsedTime(struct timeval now, struct timeval then);

double currentTime(void);

#endif

