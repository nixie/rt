#include <iostream>
#include <SDL/SDL.h>
#include <SDL/SDL_mutex.h>
#include <math.h>
#include "Eigen/Dense"
#include "raymarch.h"
#include "formulae.h"
#include "common.h"
#include "params.h"
#include "light.h"
#include "math.h"

using namespace Eigen;
using namespace std;

extern Params params;
extern SDL_mutex *lock;
extern bool stop_rendering;
extern Vector4d orbit_trap;

void render(void){
    // calculate camera rotation matrix and save it to params for future
    // read-only access from other modules of this raytracer
    Matrix3d m_cam = calcRotationMatrix(
            params.camera_yaw, params.camera_pitch, params.camera_roll);
    Matrix3d m_wld = calcRotationMatrix(
            params.world_yaw, params.world_pitch, params.world_roll);
    params.m_cam = m_cam;
    params.m_wld = m_wld;

    // print how far the camera is from fractal surface
    params.last_distance = distEstim(m_wld*params.camera_pos);
    cout << "Nearest surface: " << params.last_distance << endl;

    int niters=0;
    int steps_sum=0;
    int alias_size = params.alias_size;
    double aspect = (double)params.width/params.height;
    double k = tanl((FOVY/2)/params.zoom);

    //disable antialiasing = aliasing_size = 1
    if (params.alias_en == false) {
        alias_size = 1;
    }
    int aa_ncells = alias_size*alias_size;
    Vector3d *c = new Vector3d[aa_ncells];

    // loop through all screen pixels. Always skip gs pixels verticaly
    // and horizontaly, so we can see early results.
    for (int offset=0; offset < params.gs*params.gs; offset++){
        for (int y=offset/params.gs; y < params.height; y+=params.gs){
            for (int x=offset%params.gs; x < params.width; x+=params.gs){

                if (stop_rendering){
                    return;
                }


                //cycle for aa
                for (int aa = 0; aa < aa_ncells; aa++) {              
                    c[aa] = (255.0 * params.bg_col);
                }
                    
                for (int aa_x = 0; aa_x < alias_size; aa_x++) {
                    for (int aa_y = 0; aa_y < alias_size; aa_y++) {
                        double x_pos = ((x - 0.5) + (1.0 / (2.0 * alias_size)) + (aa_x * (1.0 / alias_size)));
                        double y_pos = ((y - 0.5) + (1.0 / (2.0 * alias_size)) + (aa_y * (1.0 / alias_size)));

                        //random position
                        if (params.alias_random_en == true) {
                            double rand_x = randomFloat((-1.0 / (2.0 * alias_size)), (1.0 / (2.0 * alias_size)));
                            double rand_y = randomFloat((-1.0 / (2.0 * alias_size)), (1.0 / (2.0 * alias_size)));
                            x_pos += (rand_x / 2);
                            y_pos += (rand_y / 2);
                        }

                        Vector3d ray = m_cam*Vector3d(
                                ((double)x_pos/params.width-0.5),
                                (((double)y_pos/params.height-0.5)/aspect),
                                1.0/k).normalized();

                        double dt, distance = params.last_distance/2.0;
                        int steps;
                        Vector3d p;

                        // now march along the ray
                        for (steps=0; distance < params.far; steps++){
                            p = Vector3d(m_wld*(distance*ray+params.camera_pos));
                            if (steps > params.max_steps){
                                cout << ".";
                                break;
                            }

                            dt = distEstim(p);
                            distance += dt;
                            
                            // dynamic DE threshold calculation. It depends
                            // on FOVY, distance from camera and resolution.
                            // The idea is: do not consider distances smaller
                            // than 1 pixel.
                            if (params.dynamic_DE_thd){
                                params.min_distance = distance*(k/params.width);
                            }

                            if (dt < params.min_distance){
                                distance -= (params.min_distance-dt);

                                // compute surface color
                                c[(aa_x * alias_size) + aa_y]
                                    = 255.0*computeColor(p, dt, distance, 1);

                                steps_sum += steps;
                                break;
                            }
                        }
                    }
                }

                //avg in [0]
                for (int aa = 1; aa < aa_ncells; aa++) {
                    c[0] += c[aa];
                }               
                c[0] = c[0] / aa_ncells;

                setpx(x, y, c[0][0], c[0][1], c[0][2]);
                niters++;
            }

            if (params.preview_mode){
                SDL_mutexP(lock);
                SDL_Flip(surf);
                SDL_mutexV(lock);
            }
        }

        SDL_mutexP(lock);
        SDL_Flip(surf);
        SDL_mutexV(lock);
    }
    cout << "avg steps: " << (double)steps_sum/niters << endl;
}


int rendering_thread(void *data){
    cout << SDL_ThreadID() << ": started" << endl;
    unsigned int begin = SDL_GetTicks();

    render();

    // Output time consumed for rendering
    unsigned int end = SDL_GetTicks();
    cout << SDL_ThreadID() << ": done, " << 1e-3*(end-begin) << "seconds\n";

    // If requested by the --outfile command line parameter, save rendering
    // to file
    if(params.saveOutput()){
        save_surface(params.outfile);
    }

    return 0;
}

