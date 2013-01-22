#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include "params.h"
#include <ctype.h>
#include <iostream>
#include <fstream>

using namespace std;

int ifile_counter_k = 0;    // keyframe counter
int ifile_counter_c = 0;    // config counter
int ofile_counter_k = 0;      // output keyframe counter
const char *helpstr =
"This is 3D fractal raytracer, options are:\n"
"   -h          print this help\n"
"   -r <WxH>    set output resolution in pixels\n"
"   -o <fname>  save output image to <fname> instead of displaying it\n"
"   -c <fname>  load given config file on startup\n"
"   -p          preview mode (disable all effects), this option can only\n"
"               be specified alone\n";

bool Params::parseCmdline(int argc, char **argv){
    int params_ok = 0;

    //no params, defualt values
    if (argc == 1) {
        params_ok = 1;
    }

    //help
    if (argc == 2 && strcmp("-h", argv[1]) == 0) {
        params_ok = 1;
        printHelp();
        exit(0);
    }
    // preview mode
    if (argc == 2 && strcmp("-p", argv[1]) == 0){
        params_ok = 1;
        preview_mode = true;
    }else{
        preview_mode = false;
    }

    //normal params
    if (argc == 3 || argc == 5 || argc == 7) {
        for (int i = 0; i < argc; i++) {

            //resolution
            if (strcmp("-r", argv[i]) == 0) {
                params_ok = 1;
                if (parseGeometry(argv[i + 1], &width, &height)){
                    cerr << "Resolution is malformed!\n";
                    return 1;
                }
                i++;
            }


            //outfile
            if (strcmp("-o", argv[i]) == 0) {
                params_ok = 1;
                outfile = argv[i + 1];
                // we do not need interleave framebuffer now, so set grid
                // step to 1 for better data locality
                gs = 1;
                i++;
            }

            // configfile
            if (strcmp("-c", argv[i]) == 0){
                params_ok = 1;
                configfile = argv[i+1];
                i++;
            }
        }
    }

    if (!params_ok) {
        cerr << "Parsing command line parameters failed!\n";
        printHelp();
        return 1;
    }

    return 0;
}

bool parseGeometry(char *p, int *width, int *height){
    string temp = p;  


    //split by 'x'
    int pos = temp.find_first_of('x');
    if (!pos) {
        return 1;
    }

    string wid = temp.substr(0, pos);
    string hei = temp.substr(pos + 1, temp.length() - pos);

    if (!isInt(wid) || !isInt(hei)) {
        return 1;
    }

    istringstream is1(wid);
    is1 >> *width;
    istringstream is2(hei);
    is2 >> *height;

    return 0;
}

bool isInt(std::string s){
    bool is = true;
    for(unsigned int i = 0; i < s.length(); i++){
        if(!isdigit(s[i])) {
            is = false;
        }
    }

    return is;
}

void printHelp(void){
    std::cout << helpstr;
}

string bool2str(bool flag){
    return flag ? string("txt true\n") : string("txt false\n");
}

int str2val(string s){
    if (s == "txt true"){
        return 1;
    }else if (s== "txt false"){
        return 0;
    }else{
        return -1;
    }
}


void Params::saveParams() {
    ostringstream number;
    number << ofile_counter_k;

    std::string filename = "keyframe_";
    filename.append(number.str());
    filename.append(".txt");

    ofstream file;
    file.open (filename);
    file << max_iterations << endl;
    file << eps << endl;
    file << bulb_power << endl;
    file << max_steps << endl;
    file << min_distance << endl;
    file << bailout << endl;
    file << far << endl;
    file << zoom << endl;
    file << camera_pos << endl;
    file << camera_yaw << endl;
    file << camera_pitch << endl;
    file << camera_roll << endl;
    file << world_yaw << endl;
    file << world_pitch << endl;
    file << world_roll << endl;
    file << light[0] << endl;
    file << light[1] << endl;
    file << floor_col << endl;
    file << fog_dist << endl;
    file << alias_size << endl;

    // boolean/discrete values - do not interpolate
    file << bool2str(fractal_type);
    file << bool2str(light_en[0]);
    file << bool2str(light_en[1]);
    file << bool2str(ao_en);
    file << bool2str(shadow_en);
    file << bool2str(orbit_en);
    file << bool2str(phong_en);
    file << bool2str(reflect_en);
    file << bool2str(fog_en);
    file << bool2str(torch_en);
    file << bool2str(alias_en);
    file << bool2str(alias_random_en);
    file << bool2str(dynamic_DE_thd);


    file.close();
    cout << "Saved configuration to file " << filename << ".\n";
    ofile_counter_k++;
}


void Params::loadParams(bool next, bool interpolated) {

    if (next || configfile.empty()){
        ostringstream number;
        if (interpolated){
            // load configuration from interpolated files
            number << "config_" << setfill('0') <<setw(5)<< ifile_counter_c++;
            number << ".txt";
            configfile = number.str();
        } else {
            // load configuration from interpolation knots
            number << ifile_counter_k++;
            configfile = "keyframe_";
            configfile.append(number.str());
            configfile.append(".txt");
        }
    }

    ifstream file;
    file.open (configfile);
    if (!file.is_open()){
        std::cerr << "Cannot open configuration from file " << configfile << std::endl;
        return;
    }

    string temp = ""; 

    getline(file, temp); istringstream is5(temp); is5 >> max_iterations;
    getline(file, temp); istringstream is6(temp); is6 >> eps;
    getline(file, temp); istringstream is7(temp); is7 >> bulb_power;
    getline(file, temp); istringstream is8(temp); is8 >> max_steps;
    getline(file, temp); istringstream is9(temp); is9 >> min_distance;
    getline(file, temp); istringstream is10(temp); is10 >> bailout;
    getline(file, temp); istringstream is11(temp); is11 >> far;
    getline(file, temp); istringstream is12(temp); is12 >> zoom;
    getline(file, temp); istringstream is16(temp); is16 >> camera_pos[0];
    getline(file, temp); istringstream is17(temp); is17 >> camera_pos[1];
    getline(file, temp); istringstream is18(temp); is18 >> camera_pos[2];
    getline(file, temp); istringstream is19(temp); is19 >> camera_yaw;
    getline(file, temp); istringstream is20(temp); is20 >> camera_pitch;
    getline(file, temp); istringstream is21(temp); is21 >> camera_roll; 
    getline(file, temp); istringstream is22(temp); is22 >> world_yaw; 
    getline(file, temp); istringstream is23(temp); is23 >> world_pitch; 
    getline(file, temp); istringstream is24(temp); is24 >> world_roll; 
    getline(file, temp); istringstream is25(temp); is25 >> light[0][0]; 
    getline(file, temp); istringstream is26(temp); is26 >> light[0][1]; 
    getline(file, temp); istringstream is27(temp); is27 >> light[0][2];
    getline(file, temp); istringstream is28(temp); is28 >> light[1][0]; 
    getline(file, temp); istringstream is29(temp); is29 >> light[1][1]; 
    getline(file, temp); istringstream is30(temp); is30 >> light[1][2];
    getline(file, temp); istringstream is31(temp); is31 >> floor_col[0];
    getline(file, temp); istringstream is32(temp); is32 >> floor_col[1];
    getline(file, temp); istringstream is33(temp); is33 >> floor_col[2];
    getline(file, temp); istringstream is34(temp); is34 >> fog_dist;
    getline(file, temp); istringstream is35(temp); is35 >> alias_size;

    // booleans/discrete values - not interpolated
    getline(file, temp);
    fractal_type = str2val(temp) ? mengersponge: mandelbulb;
    getline(file, temp); light_en[0] = str2val(temp);
    getline(file, temp); light_en[1] = str2val(temp);
    getline(file, temp); ao_en = str2val(temp);
    getline(file, temp); shadow_en = str2val(temp);
    getline(file, temp); orbit_en = str2val(temp);
    getline(file, temp); phong_en = str2val(temp);
    getline(file, temp); reflect_en = str2val(temp);
    getline(file, temp); fog_en = str2val(temp);
    getline(file, temp); torch_en = str2val(temp);
    getline(file, temp); alias_en = str2val(temp);
    getline(file, temp); alias_random_en = str2val(temp);
    getline(file, temp); dynamic_DE_thd = str2val(temp);
    
    if (preview_mode){
        ao_en = false;
        shadow_en = false;
        orbit_en = false;
        reflect_en = false;
        alias_en = false;
        light_en[0] = false;
        light_en[1] = false;
        torch_en = true;
    }

    file.close();
    cout << "Loaded configuration from file " << configfile << ".\n";
}

void Params::setFileCounters(int keyframe_counter, int config_counter){
    if (keyframe_counter >= 0){
        ifile_counter_k = keyframe_counter;
    }
    if (config_counter >= 0){
        ifile_counter_c = config_counter;
    }
}
