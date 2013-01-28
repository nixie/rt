#include <iostream>
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include <SDL/SDL_mutex.h>
#include <SDL/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "params.h"
#include "raymarch.h"

using namespace std;

const double ROTATE_STEP=M_PI/16;

SDL_Surface* surf;
SDL_mutex *lock;
TTF_Font* font;
bool finished;
bool stop_rendering;
Params params;
int img_cnt=0;


void restartRendering(SDL_Thread *t){
    stop_rendering=1;
    SDL_WaitThread(t, NULL);
    stop_rendering=0;
    SDL_FillRect(SDL_GetVideoSurface(), NULL, 0 ); // clean screen
    t = SDL_CreateThread(rendering_thread, NULL);  // re-start
}

int main(int argc, char **argv){
    // Parse command line arguments from argv
    if (params.parseCmdline(argc, argv)){
        return 1;
    }
    if (!params.configfile.empty()){
        params.loadParams(false, false);
    }

    SDL_Event event;
    SDL_Thread* t1;
    lock = SDL_CreateMutex();
    atexit(SDL_Quit);
    stop_rendering=0;

    if (params.saveOutput()){
        // non-interactive mode, do not display any window
        char varsettings[]="SDL_VIDEODRIVER=dummy";
        putenv(varsettings);
    }

    // Initialize TTF font rendering subsystem
    if(TTF_Init()==-1) {
        cout << "TTF_Init: " <<  TTF_GetError() << endl;
        return 1;
    }
    atexit(TTF_Quit);
    
	if(!(font = TTF_OpenFont("TerminusMedium.ttf", params.font_size))) {
		cout << "Error loading font: " << TTF_GetError() << endl;
		return 1;
	}


    // Initialize SDL and the video subsystem
    SDL_Init(SDL_INIT_VIDEO);
    SDL_WM_SetCaption("3D Fractal Renderer", NULL);

    // Set the video mode
    surf = SDL_SetVideoMode(params.width, params.height, 24, SDL_SWSURFACE);
    if (surf == NULL){
        cerr << "Cannot get SDL surface: " << SDL_GetError() << endl;
    }

    // start rendering thread
    t1 = SDL_CreateThread(rendering_thread, NULL);
    if (t1 == NULL){
        cout << "SDL_CreateThread failed: " << SDL_GetError() << endl;
    }

    if (params.saveOutput()){
        // Just render a picture, save it and exit
        SDL_ShowCursor(SDL_DISABLE);
        SDL_WaitThread(t1, NULL);
        return 0;
    }


    // handle events
    bool finished = 0;
    int step_sign = 1;
    double rot_step = ROTATE_STEP;
    double move_step;
    Uint8 *keystate = SDL_GetKeyState(NULL);

    while ( !finished && SDL_WaitEvent(&event) >= 0 ) {
        switch (event.type) {
            case SDL_QUIT: 
                finished = 1;
                break;
            case SDL_MOUSEBUTTONUP:
                if (event.button.button == 2){
                    dumppx(event.button.x, event.button.y);
                    break;
                } else {
                    // chage direction using current zoom before zooming
                    double aspect = params.width/(double)params.height;
                    double dx = ((double)event.button.x/params.width-0.5);
                    double dy = ((double)event.button.y/params.height-0.5)/aspect;
                    params.camera_yaw -= dy * (FOVY/2)/params.zoom;
                    params.camera_pitch +=  dx * (FOVY/2)/params.zoom;

                    if (!keystate[SDLK_LCTRL]){
                        // set new zoom
                        params.zoom *= event.button.button == 1 ? 2 : 0.5;
                        if (params.zoom < 1.0){
                            params.zoom = 1.0;
                        }
                    }
                }

                cout << "log2(zoom): " << log2l(params.zoom) << endl;
                restartRendering(t1);
                break;
            case SDL_KEYDOWN: 
                step_sign = event.key.keysym.mod == KMOD_LSHIFT ? -1 : 1;
                rot_step = step_sign * ROTATE_STEP;
                move_step = params.last_distance * 0.5;
                if (event.key.keysym.mod == KMOD_LSHIFT){
                    move_step *= 0.2;
                }

                switch(event.key.keysym.sym) {
                    case SDLK_q:
                    case SDLK_ESCAPE:
                        finished = 1;
                        break;
                    case SDLK_s:
                        if (event.key.keysym.mod == KMOD_LSHIFT){
                            // 'S' - save current framebuffer to image
                            stringstream ss;
                            ss << "f" << img_cnt++ << ".bmp";
                            save_surface(ss.str());
                        }else{
                            // 's' - save current configuration to file
                            params.saveParams();
                        }
						break;
                    case SDLK_l:
                        params.loadParams(true,event.key.keysym.mod == KMOD_LSHIFT);
                        restartRendering(t1); break;
                    case SDLK_F12:  // load config from keyframe_0.txt
                        params.setFileCounters(0,0);
                        params.loadParams(false, false);
                        restartRendering(t1); break;

                    case SDLK_i:
                        params.display_params = true;
                        break;

                    case SDLK_c:
                        // reset input config file counters
                        params.setFileCounters(0,0);
                        break;
                    case SDLK_d:
                        params.dynamic_DE_thd = !params.dynamic_DE_thd;
                        params.min_distance = 0.001;
                        restartRendering(t1); break;

                    case SDLK_f:
                        params.fractal_type = (FType)!params.fractal_type;
                        restartRendering(t1); break;
                    case SDLK_a:
                        params.alias_en = !params.alias_en;
                        restartRendering(t1); break;


                    /* Navigation */
                    case SDLK_RIGHT:
                        params.camera_pos += params.m_cam*Vector3d(move_step,0,0);
                        restartRendering(t1); break;
                    case SDLK_LEFT:
                        params.camera_pos -= params.m_cam*Vector3d(move_step,0,0);
                        restartRendering(t1); break;
					case SDLK_UP:
                        params.camera_pos -= params.m_cam*Vector3d(0,move_step,0);
                        restartRendering(t1); break;
                    case SDLK_DOWN:
                        params.camera_pos += params.m_cam*Vector3d(0,move_step,0);
                        restartRendering(t1); break;
                    case SDLK_PAGEUP:
                        params.camera_pos += params.m_cam*Vector3d(0,0, move_step);
                        restartRendering(t1); break;
                    case SDLK_PAGEDOWN:
                        params.camera_pos -= params.m_cam*Vector3d(0,0,move_step);
                        restartRendering(t1); break;

                    /* Camera/World rotation */
                    case SDLK_y: params.camera_yaw += rot_step;
                                 restartRendering(t1); break;
                    case SDLK_p: params.camera_pitch += rot_step;
                                 restartRendering(t1); break;
                    case SDLK_r: params.camera_roll += rot_step;
                                 restartRendering(t1); break;
                    case SDLK_b: params.world_yaw += rot_step;
                                 restartRendering(t1); break;
                    case SDLK_n: params.world_pitch += rot_step;
                                 restartRendering(t1); break;
                    case SDLK_m: params.world_roll += rot_step;
                                 restartRendering(t1); break;
                    
					/* Light/Shadow effects */
					case SDLK_F1: params.ao_en = !params.ao_en; 
								  restartRendering(t1); break;
					case SDLK_F2: params.shadow_en = !params.shadow_en;
								  restartRendering(t1); break;
					case SDLK_F3: params.orbit_en = !params.orbit_en;
								  restartRendering(t1); break;
					case SDLK_F4: params.phong_en = !params.phong_en;
								  restartRendering(t1); break;
					case SDLK_F5: params.reflect_en = !params.reflect_en;
								  restartRendering(t1); break;
					case SDLK_F6: params.fog_en = !params.fog_en;
								  restartRendering(t1); break;
					case SDLK_F7: params.light_en[0] = !params.light_en[0];
								  restartRendering(t1); break;
					case SDLK_F8: params.light_en[1] = !params.light_en[1];
								  restartRendering(t1); break;
					case SDLK_F9: params.torch_en = !params.torch_en;
								  restartRendering(t1); break;

                    /* Fog distance */
					case SDLK_PLUS: params.fog_dist += 0.05;
                                  restartRendering(t1); break;
					case SDLK_MINUS: params.fog_dist -= 0.05;
                                  restartRendering(t1); break;

					default:
                                  break;
				}
				break;
			default:
				break;
		}

        // Update the screen
        SDL_mutexP(lock);
        int ret = SDL_Flip(surf);
        SDL_mutexV(lock);
        if (ret != 0) {
            cerr << "Failed to update screen: " << SDL_GetError() << endl;
        }
    }
    stop_rendering = 1;
    SDL_WaitThread(t1, NULL);
    SDL_DestroyMutex(lock);

    return 0;
}
