#include "formulae.h"
#include "params.h"
#include <iostream>
extern Params params;
extern double dfloor;

using namespace std;

Vector4d orbit_trap;

// deltas for numerical differentiation
Vector3d dx(params.eps, 0.0, 0.0);
Vector3d dy(0.0, params.eps, 0.0);
Vector3d dz(0.0, 0.0, params.eps);

// Lower value -> better quality, but decreased rendering speed ...
// Range : (0, 1>
const float DE_step_factor=0.6;

double length(Vector3d v){
	return sqrt(v[0]*v[0] + v[1]*v[1]+ v[2]*v[2]);
}

/**
 * Computes surface normal using finite diferences method.
 * Code taken from mandelbulber.
 */
Vector3d normal(Vector3d p){
    dx(0) = params.min_distance;
    dy(1) = params.min_distance;
    dz(2) = params.min_distance;
    double s1 = distEstim(p);
    double s2 = distEstim(p+dx);
    double s3 = distEstim(p+dy);
    double s4 = distEstim(p+dz);

	return Vector3d(
			s2-s1, s3-s1, s4-s1).normalized();
}

Vector3d BulbPower(Vector3d z, int p){

	double zx=z(0), zy=z(1), zz=z(2);
	double r = length(z);
	// convert to polar coordinates
	double theta = acos(zz/r);
	double phi = atan2(zy,zx);

	// scale and rotate the point
	double zr = pow( r,p);
	theta = theta*p;
	phi = phi*p;

	// convert back to cartesian coordinates
	return zr*Vector3d(sin(theta)*cos(phi), sin(phi)*sin(theta), cos(theta));
}


/**
 * Distance estimator for mandelbul fractal. Code taken from
 * http://blog.hvidtfeldts.net/index.php/2011/06/distance-estimated-3d-fractals-part-i/
 */
double DE_mandelbulb(Vector3d p) {
	Vector3d z = p;
	double dr = 1.0;
	double r = length(z);
	int Power = params.bulb_power;
	for (int i = 0; i < params.max_iterations ; i++) {
		if (r>params.bailout) break;

		double zx=z(0), zy=z(1), zz=z(2);

		// convert to polar coordinates
		double theta = acos(zz/r);
		double phi = atan2(zy,zx);
		dr =  pow( r, Power-1.0)*Power*dr + 1.0;

		// scale and rotate the point
		double zr = pow( r,Power);
		theta = theta*Power;
		phi = phi*Power;

		// convert back to cartesian coordinates
		z = zr*Vector3d(sin(theta)*cos(phi), sin(phi)*sin(theta), cos(theta));
		z+=p;
		r = length(z);

		if(params.orbit_en && i<5){
			orbit_trap[0] = min((double)orbit_trap[0],(double)fabs(z(0)));
			orbit_trap[1] = min((double)orbit_trap[1],(double)fabs(z(1)));
			orbit_trap[2] = min((double)orbit_trap[2],(double)fabs(z(2)));
			orbit_trap[3] = min((double)orbit_trap[3],(double)fabs(r*r));
		}
	}
	return 0.5*log(r)*r/dr;
}

/**
 * Distance estimator for menger sponge fractal taken from mandelbuler code.
 */
double DE_sponge(Vector3d p){
	//this is our old friend menger

    double x=p(0), y=p(1), z=p(2);
    double r=sqrt(x*x+y*y+z*z), dist=0, t;
	int n;
    for (n=1; n < params.max_iterations; n++){
		x=fabs(x);y=fabs(y);z=fabs(z);
		if(x<y){t=x;x=y;y=t;}
		if(x<z){t=x;x=z;z=t;}
		if(y<z){t=y;y=z;z=t;}
		x=x*3.0-2.0;y=y*3.0-2.0;z=z*3.0;
        if (z > 1.0){
            z -=2.0;
        }

		if(params.orbit_en && n<5){
			orbit_trap[0] = min((double)orbit_trap[0],(double)fabs(x));
			orbit_trap[1] = min((double)orbit_trap[1],(double)fabs(y));
			orbit_trap[2] = min((double)orbit_trap[2],(double)fabs(z));
			orbit_trap[3] = min((double)orbit_trap[3],
					(double)Vector3d(x,y,z).dot(Vector3d(x,y,z)));	
		}

        r = sqrt(x*x+y*y+z*z);
        if (r > 1000){
            dist = (r-2.0)*pow(3.0,-(double)n);
            break;
        }
	}

    return dist;
}

// distance estimator selection for surface
double distEstim(Vector3d p){
	double dt;
	switch(params.fractal_type){
		case mengersponge:
			dt = DE_sponge(p);				// distance to fractal
			dfloor = p.dot(Vector3d(0.0,1.0,0.0)) + 1.0;// distance to floor
			if(dfloor < dt){
				dt = dfloor;
				dfloor *= DE_step_factor;	
			}
			break;
		case mandelbulb:
			dt = DE_mandelbulb(p);
			break;
		default: break;
	}

	return dt*DE_step_factor;
}

