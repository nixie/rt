#ifndef __FORMULAE_H__
#define __FORMULAE_H__

/* Formulae sources:
 *  http://blog.hvidtfeldts.net/index.php/2011/06/distance-estimated-3d-fractals-part-i/
 *  http://www.fractalforums.com/http://www.fractalforums.com/
 */

#include "Eigen/Dense"
using namespace Eigen;

Vector3d BulbPower(Vector3d z, int p);

double DE_sponge(Vector3d p);
double DE_mandelbulb(Vector3d p);
double distEstim(Vector3d p);

double length(Vector3d v);
Vector3d normal(Vector3d p);

#endif

