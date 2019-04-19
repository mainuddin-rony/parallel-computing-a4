#ifndef PIXEL_H
#define PIXEL_H

// A simple pixel type. It's just a color represented
// as three channels. 'char' is 1 byte wide.
typedef struct {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} pixel;

#endif

