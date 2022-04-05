#include "backend_process/ViewTiff.hpp"

unsigned int ViewTiff::id = 0;

ViewTiff::ViewTiff() {
    this->set_process_name("ViewTiff(" + std::to_string(ViewTiff::id) + ")");
    ViewTiff::id += 1;
}

ViewTiff::~ViewTiff() {}


void ViewTiff::run() {

    std::unique_ptr<btrgb::LibTiffReader> tiff_reader(new btrgb::LibTiffReader);
    
    try {
        std::string prj_filename = this->process_data_m->get_string("name");
        std::ifstream prj_file(prj_filename);
        std::string local_file = jsoncons::json::parse(prj_file)["OutPutFiles"]["CM"].as<std::string>();
        std::string filename = prj_filename.substr(0, prj_filename.find_last_of("/\\") + 1) + local_file;

        if( ! btrgb::Image::is_tiff(filename) )
            throw std::runtime_error("Invalid ViewTiff JSON");

        cv::Mat im;
        tiff_reader->open(filename);
        tiff_reader->copyBitmapTo(im);

        try {
            void* profile;
            uint32_t profile_size;
            tiff_reader->getColorProfile(&profile_size, &profile);
            btrgb::ColorProfiles::convert(im, 
                profile, profile_size, 
                (void*) btrgb::sRGB2014_icc_data, btrgb::sRGB2014_icc_size
            );
        } catch(const std::exception& e) {}

        tiff_reader->recycle();

        /* Wrap the Mat as an Image object. */
        btrgb::Image imObj(filename);
        imObj.initImage(im);
        imObj.setColorProfile(btrgb::ColorSpace::sRGB);

        /* Send image. */
        this->coms_obj_m->send_binary(&imObj, btrgb::FULL);

    }
    catch(const ParsingError& e) {
        this->coms_obj_m->send_error("Invalid ViewTiff JSON", "ViewTiff");
    }
    catch(const std::exception& e) {
        this->coms_obj_m->send_error(e.what(), "ViewTiff");
    }

}
