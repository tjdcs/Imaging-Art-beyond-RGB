//
// Created by ThinkPad41 on 10/10/2021.
//

#ifndef BEYOND_RGB_BACKEND_IMAGECALIBRATOR_H
#define BEYOND_RGB_BACKEND_IMAGECALIBRATOR_H


#include "ImgProcessingComponent.h"

class ImageCalibrator : public ImgProcessingComponent{
private:
    std::vector<std::shared_ptr<ImgProcessingComponent>> components;
public:
    ImageCalibrator(const std::vector<std::shared_ptr<ImgProcessingComponent>>& components);
    void execute(CommunicationObj* comms, btrgb::ArtObject* images) override;
    std::string get_component_list() override;

};


#endif //BEYOND_RGB_BACKEND_IMAGECALIBRATOR_H
