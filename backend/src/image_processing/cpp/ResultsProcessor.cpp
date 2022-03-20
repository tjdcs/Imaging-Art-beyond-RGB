#include "image_processing/header/ResultsProcessor.h"

void ResultsProcessor::execute(CommunicationObj* comms, btrgb::ArtObject* images){
    comms->send_info("","ResultsProcessor");
    comms->send_progress(0,"ResultsProcessor");
    
    this->output_results(images);  
    comms->send_progress(0.2,"ResultsProcessor");  
    this->output_images(images);
    
    comms->send_progress(1,"ResultsProcessor");
}

void ResultsProcessor::output_images(btrgb::ArtObject* images){
    for(const auto& [name, img]: *images) {
        images->outputImageAs(btrgb::TIFF, name, name);
    }
}

void ResultsProcessor::output_results(btrgb::ArtObject* images){
    CalibrationResults *calibration_res = images->get_results_obj(btrgb::ResultType::CALIBRATION);
    CalibrationResults *verification_res = images->get_results_obj(btrgb::ResultType::VERIFICATION);
    std::string output_dir = images->get_output_dir();

    std::ofstream calibration_stream;
    calibration_stream.open(output_dir + "CalibrationResults.csv");
    calibration_res->write_results(calibration_stream);
    calibration_stream.close();

    std::ofstream verification_stream;
    verification_stream.open(output_dir + "VerificationResults.csv");
    verification_res->write_results(verification_stream);
    verification_stream.close();
}