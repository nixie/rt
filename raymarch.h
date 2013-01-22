#ifndef __RAYMARCH_H__
#define __RAYMARCH_H__

#include "Eigen/Dense"
using namespace Eigen;

int rendering_thread(void *data);
Vector3d getRayDirection(int x, int y);

#endif

