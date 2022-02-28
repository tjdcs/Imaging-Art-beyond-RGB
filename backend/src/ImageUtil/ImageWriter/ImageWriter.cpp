#include "ImageWriter.hpp"

namespace btrgb {

    void ImageWriter::write(Image* im) {
        write(im, im->getName());
    }

    void ImageWriter::write(Image* im, std::string filename) {

        filename = "out/" + filename;

        std::string fname = filename;
        if (!fname.ends_with(this->file_extension))
            fname += this->file_extension;

        this->_write(im, fname);

    }

}
