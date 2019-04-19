/*
 *  rgbtiff.h
 *  
 *
 *  Copyright 2007 Philip J Rhodes. All rights reserved.
 *
 */

#ifndef RGBTIFF_H
#define RGBTIFF_H

#ifdef __cplusplus
extern "C" {
#endif



/* Given the name of an uncompressed RGB TIFF file as the first argument,
 this function attempts to open the file and read the RGB image data
 within. If successful, a pointer to the image data will be returned, and
 the width and height of the image are written to the corresponding pass
 by reference parameters.  If not successful, NULL is returned.
 */
unsigned char * readRGBtiff(const char * filename, int * width, int * height);



/* Write a tiff file with name given by the first argument to disk. The image
 data is given by the second argument, and the dimensions by the final two.
 Zero is returned upon success, and 1 if a failure occurred.
 */
int writeRGBtiff(char *outfilename, unsigned char pixels[], int width, int height);


#ifdef __cplusplus
}
#endif


#endif
