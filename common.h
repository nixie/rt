#ifndef __COMMON_H__
#define __COMMON_H__

#include <SDL/SDL.h>
#include <iostream>
#include <string>

extern SDL_Surface *surf;

// Set pixel in our framebuffer to some value
void setpx(int x, int y, int r, int g, int b);

// Get the pixel value and print it to stdout
void dumppx(int x, int y);

// Save current framebuffer to file $fname
int save_surface(std::string fname);

//Random float in interval
float randomFloat(float min, float max);


#endif

