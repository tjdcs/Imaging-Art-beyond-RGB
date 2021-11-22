#ifndef BTRGB_HPP
#define BTRGB_HPP

#include <string>

namespace btrgb {

    typedef unsigned short pixel;

    typedef struct {
        std::string filename;
        pixel* bitmap;
        int width;
        int height;
        int channels;
    } image;

}


#endif