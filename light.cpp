#include <iostream>
#include <string>
#include "Eigen/Dense"
#include "light.h"
#include "params.h"
#include "formulae.h"

using namespace Eigen;
using namespace std;

extern Params params;
extern Vector4d orbit_trap;

double dfloor = 0.0;

// surface parameters
Vector3d shad_col(0.0, 0.0, 0.0);  // shadow color
double Camb = 0.15;		   // ambient coefficient
Vector3d amb_col(1.0, 1.0, 1.0);   // ambient color
double Cdiff = 0.4;		   // diffuse coefficient
Vector3d diff_col(1.0, 1.0, 1.0);  // diffuse color
double Cspec = 0.35;	 	   // specular coefficient
Vector3d spec_col(1.0, 1.0, 1.0);  // specular color
double shininess = 16.0;	   // specular shininess

// orbit traps colors
Vector3d YZ_col(1.0, 1.0, 1.0);
Vector3d XZ_col(0.34, 0.67, 0.0);
Vector3d XY_col(1.0, 0.67, 0.0);     
Vector3d R_col(0.1, 1.0, 0.94);

// value normalization
double constr(double minV, double maxV, double value){
	return min(maxV,max(value,minV));
}

// vector normalization
Vector3d constr(double minV, double maxV, Vector3d vec){
	vec[0] = constr(minV,maxV,vec[0]);
	vec[1] = constr(minV,maxV,vec[1]);
	vec[2] = constr(minV,maxV,vec[2]);
	return vec;
}

// vector(color) interpolation
Vector3d inter(Vector3d vecA, Vector3d vecB, double value){
	return constr(0.0,1.0,(vecA+value*(vecB-vecA)));
}

// ambient occlusion approximation
double ambOcclusion(Vector3d p, Vector3d n){
	double Cao = 0.7;	// ambient occlusion coef.
	double ao = 0.0;
	double sum = 0.0;
	double w = 1.0;	// weight
	double dt = distEstim(p);
	double d = 1.0 - (0.5*(double)rand()/RAND_MAX);
	for(double i = 1.0; i < 6.0; i++){
		double D = (distEstim(p+n*0.1*d*i*i)-dt)/(0.1*d*i*i);
		w *= 0.6;
		ao += w*constr(0.0, 1.0, 1.0-D);
		sum += w;
	}
	return constr(0.0, 1.0, Cao*ao/sum);
}

// single shadow calculation
double shadow(Vector3d p, Vector3d light){
	Vector3d l = (p-light).normalized(); // light source vector
	double shad = 1.0;  // {1.0 = no shadow, 0.0 = hard shadow}
	double distance = 0.0;
	double dt;
	double dlight = length(p-(light+l*distance));	// distance to light
	for(int steps = 0; steps < params.max_steps; steps++){
		Vector3d pos(light+l*distance);
		dt = distEstim(pos); // trace from light to surface
		shad = min(shad, 2.0*pow((dt/(dlight-distance)), 0.5));
		distance += dt; 
		if(dt < params.min_distance && 
				distance > dlight-params.min_distance){  // we hit something
			return shad;
		}
	}
	return 0.0;
}

// all shadows in the scene calculation
double shadow_all(Vector3d p){
	double Cshad = 0.0;
	int n = sizeof(params.light) / sizeof(params.light[0]); // number of lights
	int lights = 0;  // number of active lights
	for(int i = 0; i < n; i++){
		if(!params.light_en[i]){continue;} // inactive light
		Cshad += shadow(p, params.light[i]);
		lights++;
	}
	if(params.torch_en){
		Cshad += shadow(p, params.m_wld*params.camera_pos);
		lights++;
	}
	return Cshad/max((int)1, lights);
}

// reflected color calculation
Vector3d reflect(Vector3d p, Vector3d col, int depth){

	Vector3d n = normal(p);	// surface normal

	Vector3d v = (params.m_wld*params.camera_pos-p) // camera vector
		.normalized();
	Vector3d r = (2*n*(n.dot(v))-v) // reflection vector
		.normalized();
	Vector3d rc(params.bg_col);	// reflected color

	double dt = 0.0;
	double distance = 10e-3 + params.min_distance;//3.0*params.eps;

	for(int steps = 0; distance < params.far &&
			steps < params.max_steps; steps++){

		Vector3d pos(p+r*distance);
		dt = distEstim(pos);		// trace to surface
		distance += dt;

		if(dt < params.min_distance){
			distance -= (params.min_distance-dt);
			rc = computeColor(pos, dt, distance, depth-1);
			break;
		}
	}

	return inter(col, rc, 0.8*(1.0-pow(max(0.0, r.dot(n)), 0.1)));
}

// Blinn-Phong shading model
Vector3d blinnPhong(Vector3d p, Vector3d n, Vector3d col){
	double Tdiff = 0.0;	// total diffuse
	double Tspec = 0.0;	// total specular

	Vector3d v = 
		(params.m_wld*params.camera_pos-p) // camera vector
		.normalized();
	double Tamb = max(0.0, v.dot(n));

	Vector3d light;
	int lights = sizeof(params.light)/		 // number of lights
		sizeof(params.light[0]);

	if(params.torch_en){lights++;}			// camera light is enabled

	for(int i = 0; i < lights; i++){
		// select light
		if(i == lights-1 && params.torch_en){
			light = params.m_wld*params.camera_pos;
		}else{
			if(!params.light_en[i]){continue;} // light inactive
			light = params.light[i];
		}

		Vector3d l = (light-p).normalized(); // light source vector
		double diff = l.dot(n);
		double spec = 0.0;
		double shad = 1.0;

		if(diff > 0.0){
			Vector3d h = (l+v).normalized(); // halfway vector
			spec = h.dot(n);
			//Vector3d r = (2*n*(n.dot(l))-l)// light reflection vector
			//	       .normalized();
			//spec = r.dot(v);
			if(spec > 0.0){
				spec = pow(spec, shininess);
			}else{
				spec = 0.0;
			}
		}else{
			diff = 0.0;
		}

		if(params.shadow_en){shad = shadow(p,light);}
		Tdiff += diff*shad;
		Tspec += spec*shad;
	}

	Vector3d bp = Vector3d(
			amb_col*Camb*Tamb
			+diff_col*constr(0.0,1.0,Tdiff)*Cdiff
			+spec_col*constr(0.0,1.0,Tspec)*Cspec
			);

	bp[0] *= col[0];
	bp[1] *= col[1];
	bp[2] *= col[2];

	return bp;
}

// orbit trap coloring
Vector3d orbitTrap(){
	orbit_trap[3] = sqrt(orbit_trap[3]);
	Vector3d orbit_col;
	if(params.fractal_type==mengersponge){
		orbit_col = Vector3d(
				YZ_col*0.2*orbit_trap[0] +
				XZ_col*0.1*orbit_trap[1] +
				XY_col*1.00*orbit_trap[2] +
				R_col*-0.04*orbit_trap[3]);
	}else{

		orbit_col = Vector3d(
				YZ_col*1.0*orbit_trap[0] +
				XZ_col*0.1*orbit_trap[1] +
				XY_col*1.00*orbit_trap[2] +
				R_col*-0.07*orbit_trap[3]);			
	}
	return constr(0.0, 1.0, 3.0*orbit_col);
}


// get the color of a surface
Vector3d computeColor(Vector3d p, double dt, double distance, int depth){

	orbit_trap = Vector4d(10.0, 10.0, 10.0, 10.0);

	Vector3d col;
	Vector3d n = normal(p);

	// color of the fractal
	if(params.orbit_en){
		col = orbitTrap();
	}else{
		col = diff_col;
	}
	if(dt == dfloor){ // floor hit
		col = params.floor_col;
	}

	// ambient occlusion (normal sampling AO)
	if(params.ao_en){
		col = inter(col, shad_col, ambOcclusion(p, n));
	}

	// shadows
	if(params.shadow_en && !params.phong_en){
		col = inter(col, shad_col, 1.0 - shadow_all(p));
	}

	// Blinn-phong shading
	if(params.phong_en){
		col = blinnPhong(p, n, col);
	}

	// reflection
	if(params.reflect_en){
		if(depth){
			col = reflect(p, col, depth);
		}
	}

	// fog
	if(params.fog_en){
		col = inter(col, params.bg_col,
				1.0-exp(-pow(params.fog_dist,4.0)*distance*distance));
	}

	return col;
}

