#include <iostream>
#include <string>
#include <sstream>
#include <SDL/SDL_ttf.h>
#include <assert.h>
#include "common.h"
#include "params.h"

extern Params params;
extern TTF_Font *font;

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
        case 1:
            *p = pixel;
            break;

        case 2:
            *(Uint16 *)p = pixel;
            break;

        case 3:
            if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                p[0] = (pixel >> 16) & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = pixel & 0xff;
            } else {
                p[0] = pixel & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = (pixel >> 16) & 0xff;
            }
            break;

        case 4:
            *(Uint32 *)p = pixel;
            break;
    }
}

void setpx(int x, int y, int r, int g, int b){
    assert(x < params.width && y < params.height);
    putpixel(surf, x, y, SDL_MapRGB(surf->format, r, g, b));
}

void dumppx(int x, int y){
    int bpp = 3;
    Uint8 *p = (Uint8*)surf->pixels + y * surf->pitch + bpp*x;
    std::cout << "[" << x << "," << y << "]=(" <<
        (int)p[0] << "," << (int)p[1] << "," <<(int)p[2] << ")\n";
}

int save_surface(std::string fname){
    int ret = SDL_SaveBMP(surf, fname.c_str());
    if (ret != 0){
        std::cout << "Cannot save image to file " << fname << ": "
            << SDL_GetError() << std::endl;
    }else{
        std::cout << SDL_ThreadID() << ": image saved to file "
            << fname << std::endl;
    }
    return ret;
}

float randomFloat(float min, float max) {
    float random = ((float) rand()) / (float) RAND_MAX;

    float range = max - min;
    return (random*range) + min;
}

void render_line(char *text, int x, int y){
    SDL_Color color={255,0,0,30};
    SDL_Surface *text_surface;
    SDL_Rect dest_rect;
    dest_rect.x = x;
    dest_rect.y = y;
    if(!(text_surface=TTF_RenderUTF8_Blended(font,text,color))) {
        //handle error here, perhaps print TTF_GetError at least
        std::cout << "Error :" << TTF_GetError() << std::endl;
    } else {
        SDL_BlitSurface(text_surface,NULL,surf,&dest_rect);
        //perhaps we can reuse it, but I assume not for simplicity.
        SDL_FreeSurface(text_surface);
    }
}

void render_text(std::string text){
    std::istringstream iss(text);
    char buf[256];
    int y_displ = 10;

    while(iss.good()){
        iss.getline(buf, 256);
        if (*buf != '\0'){
            render_line(buf, 10, y_displ);
        }
        y_displ += params.font_size+2;
    }
}

