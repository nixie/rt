#ifndef __PARAMS_H__
#define __PARAMS_H__

#include <string>
#include "Eigen/Dense"

using namespace Eigen;


bool parseGeometry(char *p, int *w, int *h);
bool isInt(std::string s);
void printHelp();

const double FOVY=M_PI/2.0;

enum FType {mandelbulb, mengersponge};

class Params {
	public:
		Params() :
			width(240), height(180), gs(6), fractal_type(mengersponge),
			max_iterations(100), eps(1e-8), bulb_power(8), max_steps(250),
			min_distance(0.001), bailout(2.0), far(6.0), zoom(1.0),
			camera_pos(0.0, 0.0, 3.0),
			camera_yaw(M_PI), camera_pitch(0.0), camera_roll(0.0),
			world_yaw(0.0), world_pitch(M_PI/4), world_roll(0.0),
            alias_size(2), 
			light{Vector3d(0.2,0.2,1.2), Vector3d(4.0, 0.0, 1.05)},
			light_en{true, false}, floor_col(0.1843, 0.1333, 0.0706),
			bg_col(0.5, 0.5, 0.6), fog_dist(0.4), ao_en(false), 
			shadow_en(false), orbit_en(false), phong_en(true), 
			reflect_en(false), fog_en(false), torch_en(false),
			alias_en(false), alias_random_en(false),
            dynamic_DE_thd(true),
            display_params(false), font_size(12) {};

		bool parseCmdline(int argc, char **argv);
		bool saveOutput() { return !outfile.empty(); }
		void saveParams();
		void loadParams(bool next, bool interpolated);
        std::string toString();
        void setFileCounters(int keyframe_counter, int config_counter);

		int width, height;      // image resolution
		int gs;                 // interleave grid step
		std::string outfile;
		std::string configfile;
        bool preview_mode;
		enum FType fractal_type;

		int max_iterations;
		double eps;
		int bulb_power;
		int max_steps;         	// maximum ray-marching steps
		double min_distance;   	// minimum distance to surface
		double last_distance;
		double bailout;
		double far;            	// far clipping plane
		double zoom;
		Vector3d camera_pos;
		double camera_yaw;
		double camera_pitch;
		double camera_roll;
		double world_yaw;
		double world_pitch;
		double world_roll;
		int alias_size;

		Matrix3d m_wld;
		Matrix3d m_cam;

		Vector3d light[2];	// array with lights
		bool light_en[2];	// enable light
		Vector3d floor_col;	// floor color
		Vector3d bg_col;	// background color
		double fog_dist;	// fog distance

		bool ao_en;			// enable ambient occlusion
		bool shadow_en;		// enable soft shadow
		bool orbit_en;		// enable orbit trap coloring
		bool phong_en;		// enable blinn-phong shading
		bool reflect_en;	// enable reflection (by secondary rays)
		bool fog_en;		// enable fog
		bool torch_en;		// enable light from camera position
		bool alias_en;		// enable aa
		bool alias_random_en;		// enable random aa
        bool dynamic_DE_thd;
        bool display_params;

        int font_size;
        double avg_stepcount;

};

#endif

