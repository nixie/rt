#ifndef __LIGHT_H__
#define __LIGHT_H__

/**
 * Color computation includes:
 *  - orbit traps coloring
 *  - ambient occlusion
 *  - soft shadows
 *  - Blinn-Phong shading model
 *  - fog
 */

#include "Eigen/Dense"
using namespace Eigen;

Vector3d computeColor(Vector3d p, double dt, double distance, int depth);

// Distance estimation based on value of params.fractal_type
double distEstim(Vector3d p);

#endif

