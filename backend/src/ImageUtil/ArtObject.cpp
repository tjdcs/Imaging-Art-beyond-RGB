#include "ArtObject.hpp"

namespace btrgb {

    ArtObject::ArtObject(std::string ref_file, IlluminantType ilumination, ObserverType observer, std::string output_directory, bool batch) {
        this->ref_data = new RefData(ref_file, ilumination, observer, batch);

        bool is_windows = output_directory.front() != '/';
        if( is_windows && output_directory.back() != '\\' )
            this->output_directory = output_directory + "\\";
        else if( ! is_windows && output_directory.back() != '/' )
            this->output_directory = output_directory + "/";
        else
            this->output_directory = output_directory;
        this->batch = batch;
    }

    /*
    * Destructor deletes all btrgb::image structures that were made.
    */
    ArtObject::~ArtObject() {

        /* Delete every image in the map. */
        for( const auto& [name, img] : this->images )
            delete img;

        delete this->ref_data;

        if(nullptr != this->verification_ref){
            delete this->verification_ref;
        }

    }


    /*
    * Creates and maps a new image.
    */
    void ArtObject::newImage(std::string name, std::string filename) {

        if(this->images.contains(name))
            throw ArtObj_ImageAlreadyExists();

        Image* im = new Image(filename);

        this->images[name] = im;
    }

    //Collects the color target information from the request message and saves it in the ArtObject
    void ArtObject::setTargetInfo(TargetData target_data) {
        this->target_data = target_data;
    }

    // Builds and returns a color target
    // This asumes that the imageName specified actualy contains a color target
    ColorTarget ArtObject::get_target(std::string imageName, TargetType type){
        // If a target is requested and it does not exist it must then be in the art image
        if(imageName == TARGET(1) && !this->images.contains(imageName)){
            imageName = ART(1);
        }
        if(imageName == TARGET(2) && !this->images.contains(imageName)){
            imageName = ART(2);
        }

        try{
            Image* im = this->getImage(imageName);
            if(type == TargetType::VERIFICATION_TARGET){
                if(nullptr == this->verification_ref)
                    throw ArtObj_VerificationDataNull();
                return ColorTarget(im, this->verification_data, this->verification_ref);
            }
            else{
                return ColorTarget(im, this->target_data, this->ref_data);
            }
        }catch(ArtObj_ImageDoesNotExist){
            throw ArtObj_ImageDoesNotExist();
        }catch(ColorTarget_MissmatchingRefData){
            throw ColorTarget_MissmatchingRefData();
        }
    }

    std::unique_ptr<ColorTarget> ArtObject::get_target_pointer(std::string imageName, TargetType type) {
        try {
            Image* im = this->getImage(imageName);
            if (im == nullptr) { // Assuming getImage returns nullptr when the image doesn't exist
                return nullptr; // Image not found, return nullptr
            }
            if (type == TargetType::VERIFICATION_TARGET) {
                if (nullptr == this->verification_ref)
                    throw ArtObj_VerificationDataNull();
                return std::make_unique<ColorTarget>(im, this->verification_data, this->verification_ref);
            }
            else {
                return std::make_unique<ColorTarget>(im, this->target_data, this->ref_data);
            }
        }
        catch (ArtObj_ImageDoesNotExist) {
            return nullptr; // Image not found, return nullptr
        }
        catch (ColorTarget_MissmatchingRefData) {
            throw ColorTarget_MissmatchingRefData();
        }
    }

    //Returns a normalized value of the requested edge of the color target
    double ArtObject::getTargetInfo(std::string type) {
        if (type == "top") {
            return this->target_data.top_loc;
        }
        else if (type == "bot") {
            return this->target_data.bot_loc;
        }
        else if (type == "left") {
            return this->target_data.left_loc;
        }
        else if (type == "right") {
            return this->target_data.right_loc;
        }
        throw std::logic_error("[ArtObject::getTargetInfo] Parameter value \""
            + type + "\" is not a valid option.");
        return -1.0;
    }

    //Returns the requested dimension of the color target
    int ArtObject::getTargetSize(std::string edge){
        if(edge == "row"){
            return this->target_data.row_count;
        }
        else if(edge == "col"){
            return this->target_data.col_count;
        }
        throw std::logic_error("[ArtObject::getTargetSize] Parameter value \""
            + edge + "\" is not a valid option.");
        return -1;
    }

    /**
     * Return pointer to specified Result Object
     * This will return a null pointer if an unknown type is specified
     */
    CalibrationResults *ArtObject::get_results_obj(btrgb::ResultType type){
        switch (type){
            case btrgb::ResultType::CALIBRATION:
                return &this->calibration_res;
            case btrgb::ResultType::GENERAL:
                return &this->general_info;
            case btrgb::ResultType::VERIFICATION:
                return &this->verification_res;            
            default:
                return nullptr;
        }
    }

    /*
    * Map an image name to an existing image object.
    * If the name is used, throws ArtObj_ImageAlreadyExists.
    */
    void ArtObject::setImage(std::string name, Image* im) {

        if (this->images.contains(name))
            throw ArtObj_ImageAlreadyExists();

        this->images[name] = im;
    }


    /*
    * Returns a pointer to an image object if it is
    * is found in memory, otherwise throws ArtObj_ImageDoesNotExist.
    */
    Image* ArtObject::getImage(std::string name) {

        if(this->images.contains(name))
            return this->images[name];

        throw ArtObj_ImageDoesNotExist();
    }


    void ArtObject::deleteImage(std::string name) {
        if( ! this->images.contains(name) )
            throw ArtObj_ImageDoesNotExist();

        Image* im = this->images[name];
        delete im;
        this->images.erase(name);
    }

    /*
    * Returns a boolean to indicate if an image is stored in memory.
    */
    bool ArtObject::imageExists(std::string name) {
        return this->images.contains(name);
    }


    /*
     * The key of the image stored in memory to write to disk.
     */
    void ArtObject::outputImageAs(enum output_type filetype, std::string name, std::string filename) {

        if (! this->images.contains(name))
            throw ArtObj_ImageDoesNotExist();

        try {
            if(filename == "")
                filename = name;
            ImageWriter(filetype).write( this->images[name], this->output_directory + filename );

        }
        catch (ImageWritingError const& e) {
            throw ArtObj_FailedToWriteImage();
        }
        catch (ImageNotInitialized const& e) {
            throw;
        }
    }

    void ArtObject::setOutputFileName(std::string name) {
        this->output_file_name = name;
    }

    std::string ArtObject::getOutputFileName() {
        return this->output_file_name;
    }


    RefData* ArtObject::get_refrence_data(btrgb::TargetType target_type) {
        if(target_type == btrgb::TargetType::VERIFICATION_TARGET){
            return this->verification_ref;
        }
        return this->ref_data;
    }

    int ArtObject::imageCount(){
      return this->images.size();
    }

    std::string ArtObject::get_output_dir(){
        return this->output_directory;
    }

    bool ArtObject::get_batch() {
        return this->batch;
    }

    void ArtObject::init_verification_data(TargetData verification_data){
        std::string ref_file = verification_data.ref_base;
        IlluminantType illuminant = RefData::get_illuminant(verification_data.illum_base);
        ObserverType observer = RefData::get_observer(verification_data.obsv_base);
        this->verification_ref = new RefData(ref_file, illuminant, observer);
        this->verification_data = verification_data;
    }

}
