#include "LibRawReader.hpp"

static void exif_callback(void*, int, int, int, unsigned int, void*, INT64);

namespace btrgb {

LibRawReader::LibRawReader(LibRawReader::libraw_type method) {
    this->_method = method;
    this->_configLibRawParams();
}


LibRawReader::~LibRawReader() {
    this->_reader.recycle();
}


void LibRawReader::recycle() {
    this->_reader.recycle();
    this->_configLibRawParams();
    this->_width = -1;
    this->_height = -1;
    this->_depth = -1;
    this->_channels = -1;
}


void LibRawReader::_configLibRawParams() {
	libraw_output_params_t* opt = &( this->_reader.imgdata.params );
    switch(this->_method) {
        
    case UNPROCESSED:

        /* Do not subtract camera black level. */
        opt->user_black = 0;

        /* Don't use white balance. */
        opt->use_camera_wb = 0;
        opt->use_auto_wb = 0;

        /* Don't use camera matrix if present. */
        opt->use_camera_matrix = 0;

        break;

    case PREVIEW:

        /* Use each 2x2 bayer as one pixel, skip registration. */
        opt->half_size = 1;

        break;

    default: this->_error("[LibRawReader] Invalid method.");
    }
    
    /* Keep as 16 bit. */
    opt->output_bps = 16;

    /* Do not use histogram brightness/color scaling */
    opt->no_auto_bright = 1;
    opt->no_auto_scale = 1;

    /* Linear */
    opt->gamm[0] = 1.0;
    opt->gamm[1] = 1.0;

    /* Use raw color space. */
    opt->output_color = 0;

    /* Bi-linear interpolation. */
    opt->user_qual = 0;

    /* Use raw speed. */
    //opt->use_rawspeed = 1;

}


void LibRawReader::open(std::string filename) {
    int error_code;

    /* Open the file. */
    error_code = this->_reader.open_file(filename.c_str());
    if(error_code) {
        this->recycle();
        if( error_code == LIBRAW_FILE_UNSUPPORTED )
            throw LibRawFileTypeUnsupported();
        else
            throw ReaderFailedToOpenFile();
    }

    /* Set the exif callback which will run for each exif tag. */
    this->_reader.set_exifparser_handler(exif_callback, &(this->_tags));

    /* Unpack raw data into structures for processing. */
    error_code = this->_reader.unpack();
    if(error_code) 
        this->_error("[LibRaw] Failed to unpack image.");

    /* Post-process (configured by LibRawReader::configLibRawParams) */
    error_code = this->_reader.dcraw_process();
    if(error_code)
        this->_error("[LibRaw] Failed to process image.");

    /* Get the format of the bitmap result. */
    this->_reader.get_mem_image_format(&_width, &_height, &_channels, &_depth);


}



void LibRawReader::copyBitmapTo(void* buffer, uint32_t size) {
    if( size < _width * _height * _channels * (_depth / 8) )
        throw std::logic_error("[LibRawReader] Buffer size is too small.");
    
    int stride = this->_width * this->_channels * (_depth / 8);
    int error_code = this->_reader.copy_mem_image(buffer, stride, false);
    
    if(error_code)
        this->_error("[LibRawReader] Failed to copy image to buffer.");
}


void LibRawReader::copyBitmapTo(cv::Mat& im) {

    /* Get depth of image to create. */
    int cv_depth;
    switch(this->_depth) {
        case  8: cv_depth = CV_8U; break;
        case 16: cv_depth = CV_16U; break;
        default: this->_error("[LibRawReader] Bit depth not supported.");
    }

    
    /* Copy LibRaw image data to OpenCV Mat. */
    cv::Mat raw_im(_height, _width, CV_MAKETYPE(cv_depth, _channels));
    int stride = _width * _channels * (_depth / 8);
    int error_code = this->_reader.copy_mem_image(raw_im.data, stride, false);
    if(error_code)
        this->_error("[LibRawReader] Failed to copy image to buffer.");
    
            
    cv::Mat rgb_im(_height, _width, CV_MAKETYPE(cv_depth, 3));

    /* Verify expected number of channels. */
    if( !(raw_im.channels() == 3 || raw_im.channels() == 4) )
        this->_error("[LibRawReader] Unsupported number of channels." );

    /* Set refernce/output image to the resulting image. */
    im = raw_im;

}

void LibRawReader::_error(std::string msg) {
    this->recycle();
    throw std::runtime_error(msg);
}

}


static void exif_callback(void *context, int tag, int type, int len, unsigned int ord, void *ifp, INT64 base) {
    #define BTRGB_BUFFER_LENGTH 128

    LibRaw_abstract_datastream* data = static_cast<LibRaw_abstract_datastream*>(ifp);
    btrgb::exif *tags = static_cast<btrgb::exif*>(context);
    static char buffer[BTRGB_BUFFER_LENGTH] = {0};
    
    tag &= 0x0fffff; // Undo (ifdN + 1) << 20)
    switch (tag) {
        case btrgb::TAG_MAKE:
            data->read(buffer, sizeof(char), BTRGB_BUFFER_LENGTH);
            tags->make = buffer;
            break;
        case btrgb::TAG_MODEL:
            data->read(buffer, sizeof(char), BTRGB_BUFFER_LENGTH);
            tags->model = buffer;
            break;
        default: break;
    }

    #undef BTRGB_BUFFER_LENGTH
} 

