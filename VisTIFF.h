#ifndef VISTIFF_H
#define VISTIFF_H

/**  Write a visualization of the current timeStep to disk as a TIFF image file. The
 *   file name will be formed from the given basefilename, the current timeStep number,
 *   and a tif extension. For example, if basefilename == "water", timestep 12 is
 *   written to a file named "water.0012.tif".
 */
int writeTIFF(char * basefilename);

#endif
