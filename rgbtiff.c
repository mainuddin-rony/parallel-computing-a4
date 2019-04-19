/*
 *  rgbtiff.c
 * 
 *
 *  Copyright 2007 Philip J Rhodes. All rights reserved.
 *
 */


#include "rgbtiff.h"
#include <stdio.h>
#include <stdlib.h>

#define RGB_HEADER_LENGTH 0xCC
#define RGB_BYTES_PER_PIXEL 3

#define RGB_WIDTH_OFFSET 0x34
#define RGB_HEIGHT_OFFSET 0x40


/* Given the name of an uncompressed RGB TIFF file as the first argument,
 this function attempts to open the file and read the RGB image data
 within. If successful, a pointer to the image data will be returned, and
 the width and height of the image are written to the corresponding pass
 by reference parameters.  If not successful, NULL is returned.
 
 The caller is responsible for freeing the returned array.
 */
unsigned char * readRGBtiff(const char * filename, int * width, int * height){
    
    FILE * infile;
    unsigned char * bytes;
    
    if(NULL == (infile=fopen(filename,"r"))){
        
        printf("Couldn't open input file: %s\n", filename);
        return NULL;
    }
    
    fseek(infile, RGB_WIDTH_OFFSET, SEEK_SET);
    fread(width, 1, sizeof(int), infile);
    
    
    fseek(infile, RGB_HEIGHT_OFFSET, SEEK_SET);
    fread(height, 1, sizeof(int), infile);
    
    
    bytes = (unsigned char *) malloc( (*width) * (*height) * RGB_BYTES_PER_PIXEL);
    
    fseek(infile, RGB_HEADER_LENGTH, SEEK_SET);
    fread(bytes, RGB_BYTES_PER_PIXEL, *width * *height, infile);
    
    fclose(infile);
    
    return bytes;
}



/** This array stores the header bytes for an uncompressed RGB TIFF file. */
unsigned char RGBtifheader[] =
{   0x49, 0x49, 0x2A, 0x00, 0x1E, 0x00, 0x00, 0x00, 0xD0, 0x02, 0x00,
    0x00, 0x0A, 0x00, 0x00, 0x00, 0xD0, 0x02, 0x00, 0x00, 0x0A, 0x00,
    0x00, 0x00, 0x08, 0x00, 0x08, 0x00, 0x08, 0x00, 0x0E, 0x00, 0xFE,
    0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0xF4, 0x01, 0x00,
    0x00, 0x01, 0x01, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x77, 0x01,
    0x00, 0x00, 0x02, 0x01, 0x03, 0x00, 0x03, 0x00, 0x00, 0x00, 0x18,
    0x00, 0x00, 0x00, 0x03, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x06, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x02, 0x00, 0x00, 0x00, 0x11, 0x01, 0x04, 0x00, 0x01, 0x00,
    0x00, 0x00, 0xCC, 0x00, 0x00, 0x00, 0x15, 0x01, 0x03, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x16, 0x01, 0x04, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x77, 0x01, 0x00, 0x00, 0x17, 0x01, 0x04,
    0x00, 0x01, 0x00, 0x00, 0x00, 0x44, 0x95, 0x08, 0x00, 0x1A, 0x01,
    0x05, 0x00, 0x01, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x1B,
    0x01, 0x05, 0x00, 0x01, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
    0x1C, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x28, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


/* Write a tiff file with name given by the first argument to disk. The image
 data is given by the second argument, and the dimensions by the final two.
 Zero is returned upon success, and 1 if a failure occurred.
 */
int writeRGBtiff(char *outfilename, unsigned char pixels[], int width, int height){
    
    FILE *outfile;
    
    if(NULL == (outfile=fopen(outfilename,"w"))){
        
        printf("Couldn't open output file: %s\n",outfilename);
        return 1;
    }
    
    // Here, we write the width and height of the image into the header array.
    // These values are actually little endian 32 bit ints, but I can just write
    // the least significant 16 bits into the array, assuming that images won't
    // be larger than 65536 rows or columns.
    
    RGBtifheader[RGB_WIDTH_OFFSET] = width & 0xFF;
    RGBtifheader[RGB_WIDTH_OFFSET+1] = (width & 0xFF00) >> 8;
    
    RGBtifheader[RGB_HEIGHT_OFFSET] = height & 0xFF;
    RGBtifheader[RGB_HEIGHT_OFFSET+1] = (height & 0xFF00) >> 8;
    
    
    // This value is a duplicate of the height value, but indicates the number
    // of rows per "strip". See the TIFF standard for more information. The
    // reader code doesn't need to worry about this.
    RGBtifheader[0x88] = height & 0xFF;
    RGBtifheader[0x89] = (height & 0xFF00) >> 8;
    
    
    fwrite(RGBtifheader, sizeof(char), RGB_HEADER_LENGTH, outfile);
    
    fwrite(pixels, sizeof(char), width * height * RGB_BYTES_PER_PIXEL, outfile);
    
    fclose(outfile);
    
    return 0;
}


