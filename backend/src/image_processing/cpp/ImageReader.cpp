#include <string>

#include "ImageUtil/Image.hpp"
#include "ImageUtil/BitDepthFinder.hpp"
#include "ImageUtil/ImageReader/LibRawReader.hpp"
#include "ImageUtil/ImageReader/TiffReaderOpenCV.hpp"
#include "image_processing/header/ImageReader.h"


ImageReader::ImageReader(ImageReader::reader_strategy strategy) {
    this->_set_strategy(strategy);
}

ImageReader::~ImageReader() {
    delete this->_reader;
}

void ImageReader::_set_strategy(reader_strategy strategy) {

    /* Don't change anything if strategy already selected.
     * Otherwise, delete old strategy if exists. */
    if(this->_reader != nullptr) {
        if(this->_current_strategy == strategy) {
            _reader->recycle();
            return;
        }
        delete this->_reader;
    }

    switch(strategy) {
        case TIFF_OpenCV:
            this->_reader = new btrgb::TiffReaderOpenCV;
            break;
        case RAW_LibRaw:
            this->_reader = new btrgb::LibRawReader;
            break;
        default: 
            throw std::logic_error("[ImageReader] Invalid strategy.");
    }

    this->_current_strategy = strategy;
}



void ImageReader::execute(CommunicationObj* comms, btrgb::ArtObject* images) {
    comms->send_info("Reading In Raw Image Data!", "ImageReader");

    btrgb::BitDepthFinder util;
    int bit_depth = -1;

    double total = images->imageCount();
    double count = 0;
    comms->send_progress(0, "RawImageReader");
    for(const auto& [key, im] : *images) {
        comms->send_info("Loading " + im->getName() + "...", "ImageReader");

        /* Initialize image reader. */
        if(btrgb::Image::is_tiff(im->getName()))
            this->_set_strategy(TIFF_OpenCV);
        else
            this->_set_strategy(RAW_LibRaw);

        try {
            cv::Mat raw_im;

            _reader->open(im->getName());
            _reader->copyBitmapTo(raw_im);
            _reader->recycle();


            if(raw_im.depth() != CV_16U)
                throw std::runtime_error(" Image must be 16 bit." );


            /* Find bit depth if image is white field #1. */
            if(key == "white1") {
                bit_depth = util.get_bit_depth(
                    (uint16_t*) raw_im.data,    
                    raw_im.cols, 
                    raw_im.rows,
                    raw_im.channels()
                );
                if(bit_depth < 0)
                    throw std::runtime_error(" Bit depth detection of 'white1' failed." );
            }

            /* Convert to floating point. */
            cv::Mat float_im;
            raw_im.convertTo(float_im, CV_32F, 1.0/0xFFFF);

            /* Init btrgb::Image object. */
            im->initImage(float_im);
            
            count++;
            comms->send_progress(count/total, "ImageReader");

        }
        catch(const btrgb::LibRawFileTypeUnsupported& e) {
            comms->send_error("File type unknown, or unsupported by LibRaw.", "ImageReader");
            images->deleteImage(key);
        }
        catch(const btrgb::ReaderFailedToOpenFile& e) {
            comms->send_error(std::string(e.what()) + " : " + im->getName(), "ImageReader");
            images->deleteImage(key);
        }
        catch(const std::runtime_error& e) {
            comms->send_error(std::string(e.what()) + " : " + im->getName(), "ImageReader");
            images->deleteImage(key);
        }


    }


    for(const auto& [key, im] : *images) {
        im->_raw_bit_depth = bit_depth;
        images->outputImageAs(btrgb::TIFF, key, key + "_raw");
    }
    
    comms->send_progress(1, "RawImageReader");

}
