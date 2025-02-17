#include "ref_data.hpp"
#include <iostream>
#include <fstream>
#include "server/globals_siglton.hpp"


std::string LAST_USED_FILE_NAME = "";



RefData::RefData(const std::string& file, IlluminantType illum_type, ObserverType so_type, bool batch) {
	dataFilePath = REF_DATA_PATH;
	loadRefDataList(); // Load existing reference data files list
	this->observer = new StandardObserver(so_type);
	this->illuminants = new Illuminants(illum_type);
	this->white_pts = new WhitePoints(so_type, illum_type);
	this->white_pts = new WhitePoints(so_type, illum_type);
	this->batch = batch;
	std::string path = REF_DATA_PATH;
	this->f_name = file;

	if (LAST_USED_FILE_NAME != "") {
		this->read_in_data(LAST_USED_FILE_NAME);
	}
	else {
		if (this->is_custom(file)) {
			this->read_in_data(file);
		}
		else {
			this->read_in_data(path + file);
		}
	}
	std::cout << "before is_custom" << std::endl;
	if(this->is_custom(file)){
		std::cout << "is_custom file" << std::endl;
		std::cout << file << std::endl;
		this->read_in_data(file);
	}
	else{
		std::cout << "Standard RefData: " << file << std::endl;
		this->read_in_data(path + file);
	}
	this->init_color_patches();
	std::cout << "---SAVING...---" << std::endl;
	this->save_last_used_file(file);
}

void RefData::RefDataFolder(const std::string folder) {
	std::string path = REF_DATA_PATH;
	// Assuming path is defined as std::string path = REF_DATA_PATH; and is a correct path to the folder
	// Assuming REF_COUNT is the total number of files to consider
	// Assuming folder is an actual object that can list or contain files, but this part is unclear in the provided code.
	// You may need to adjust this part to fit how your application is supposed to access and list files in a directory.

	for (int i = 0; i < REF_COUNT; i++) {
		// This assumes you have a way to access each file's name within REF_DATA_PATH.folder
		// For demonstration, let's pretend folder[i] gives you each file name. You'll need to replace this logic with actual file listing code.
		std::string file_name = REF_DATA_PATH + folder[i]; // Placeholder: Adjust with actual method to access file names

		if (file_name.find("Reflectance_Data.csv") != std::string::npos) {
			// Assuming 'path' is a directory path and 'file_name' is the name of the file
			std::string file_path = file_name; // Correctly concatenate path and file name

			// Append the file name to vector ref_files
			ref_files.push_back(file_path);

			// Read data from the selected file
			this->read_in_data(file_path);

			// Assuming RefData constructor is expecting a file path
			RefData* ref = new RefData(file_path); // If you're dynamically allocating, make sure to manage the memory properly

			// Initialize color patches - note that this seems to be repeated for each file, which might be intended or might need adjustment
			this->init_color_patches();
		}
	}

}

RefData::~RefData() {
	if (nullptr != this->observer) {
		delete this->observer;
	}
	if (nullptr != this->illuminants) {
		delete this->illuminants;
	}
	if (nullptr != this->white_pts) {
		delete this->white_pts;
	}
	for (int row = 0; row < this->row_count; row++) {
		for (int col = 0; col < this->col_count; col++) {
			delete this->color_patches[row][col]; /* ColorPatch pointer */
		}
		delete[] this->color_patches[row]; /* array of pointers */
	}
	delete[] this->color_patches; /* array of double pointers */
	
	saveRefDataList(); // Save the current list of reference data files on destruction

	if (batch) {
		delete observer;
		delete illuminants;
		delete white_pts;
	}
}

IlluminantType RefData::get_illuminant(std::string illum_str){
	// Default to D50
    IlluminantType type = IlluminantType::D50;
    if (illum_str == "A") {
        type = IlluminantType::A;
    }
    if (illum_str == "D65") {
        type = IlluminantType::D65;
    }
    return type;
}

ObserverType RefData::get_observer(int observer_num){
	// Default to 1931
    ObserverType type = ObserverType::SO_1931;
    if (observer_num == 1964) {
        type = ObserverType::SO_1964;
    }
    return type;
}

int RefData::get_row_count() {
	return this->row_count;
}

int RefData::get_col_count() {
	return this->col_count;
}

double RefData::get_x(int row, int col) {
	ColorPatch* cp = this->get_color_patch(row, col);
	return cp->get_x();
}
double RefData::get_y(int row, int col) {
	ColorPatch* cp = this->get_color_patch(row, col);
	return cp->get_y();
}
double RefData::get_z(int row, int col) {
	ColorPatch* cp = this->get_color_patch(row, col);
	return cp->get_z();
}

double RefData::get_L(int row, int col) {
	ColorPatch* cp = this->get_color_patch(row, col);
	return cp->get_L();
}

double RefData::get_a(int row, int col) {
	ColorPatch* cp = this->get_color_patch(row, col);
	return cp->get_a();
}

double RefData::get_b(int row, int col) {
	ColorPatch* cp = this->get_color_patch(row, col);
	return cp->get_b();
}

ColorPatch* RefData::get_color_patch(int row, int col) {
	if(row < this->row_count && col < this->col_count)
		return this->color_patches[row][col];
	throw std::out_of_range("Index out of bounds");
}

ColorPatch* RefData::get_estimated_white_patch() {
	ColorPatch* current_best = this->get_color_patch(0, 0);
	for (int row = 0; row < this->row_count; row++) {
		for (int col = 0; col < this->col_count; col++) {
			ColorPatch* cp = this->get_color_patch(row, col);
			if (cp->get_y() > current_best->get_y()) {
				current_best = cp;
			}
		}
	}
	return current_best;
}

int RefData::get_estimated_white_patch_row() {
	return this->get_estimated_white_patch()->get_row();
}

int RefData::get_estimated_white_patch_col() {
	return this->get_estimated_white_patch()->get_col();
}

WhitePoints* RefData::get_white_pts(){
	return this->white_pts;
}

void RefData::read_in_data(std::string file_path) {
	if( !this->open_file(file_path) )
		throw RefData_FailedToRead(file_path);
	try{
		std::string header = this->get_next_line();
		this->identify_data_size(header);
		this->init_data_storage();
		this->pars_header(header);

		while (this->has_next_line()) {
			std::string line = this->get_next_line();
			this->pars_line(line);
		}
	}
	catch(const RefData_ParssingError& e){
		this->close_file();
		throw;
	}catch(std::exception e){
		this->close_file();
		throw RefData_ParssingError();
	}
	this->close_file();
}



void RefData::pars_line(std::string line) {
	// Ignore Wavlen col
	this->get_next<int>(line);
	for (int col = 0; col < col_count; col++) {
		for (int row = 0; row < row_count; row++) {
			try{
				double item = this->get_next<double>(line);
				this->color_patches[row][col]->append(item);
			} catch (const std::runtime_error& e) {
				throw RefData_ParssingError("@("
						+ std::to_string(row + 1) + ","
						+ std::to_string(col + 1) + ")" + e.what());
			} catch(std::exception e){
				std::string msg = "Row: " + std::to_string(row+1) + " Col: " + std::to_string(col+1);
				throw RefData_ParssingError(msg);
			}
		}
	}
}

void RefData::pars_header(std::string header) {
	// Ignore wavelength col
	this->get_next<std::string>(header);
	for (int col = 0; col < col_count; col++) {
		for (int row = 0; row < row_count; row++) {
			std::string token = this->get_next<std::string>(header);
			this->color_patches[row][col]->set_name(token);
		}
	}
}

void RefData::identify_data_size(std::string header) {
	std::string h_coppy = header;
	// Subtract 1 to ignore wavelength
	int item_count = this->count_line_items(h_coppy) - 1;
	std::string current_col_id = "";
	// Start at -1 to ignor wavelength col
	int col_count = -1;
	int i = 0;
	while (this->has_next(h_coppy)) {
		std::string item = this->get_next<std::string>(h_coppy);
		std::string col_id = this->get_col_id(item);
		if (current_col_id != col_id || current_col_id.length() == 0) {
			col_count++;
			current_col_id = col_id;
		}
	}
	int row_count = item_count / col_count;
	this->row_count = row_count;
	this->col_count = col_count;
	if( row_count <= 0 || col_count <= 0){
		throw RefData_ParssingError("Invalid File Format");
	}

}

void RefData::init_data_storage() {
	this->color_patches = new ColorPatch ** [this->row_count];
	for (int row = 0; row < this->row_count; row++) {
		this->color_patches[row] = new ColorPatch*[this->col_count];
		for (int col = 0; col < this->col_count; col++) {
			this->color_patches[row][col] = new ColorPatch(row, col, this->illuminants, this->observer, this->white_pts);
		}
	}
}

std::string RefData::get_col_id(std::string header_item) {
	size_t pos = header_item.find(":");
	return header_item.substr(0, pos);
}

void RefData::init_color_patches() {
	for (int row = 0; row < row_count; row++) {
		for (int col = 0; col < col_count; col++) {
			this->color_patches[row][col]->init();
		}
	}
}

void RefData::output_xyz() {
	std::string header = "ValueType";
	std::string y_values = "Y";
	std::string x_values = "X";
	std::string z_values = "Z";
	std::string L_values = "L*";
	std::string a_values = "a*";
	std::string b_values = "b*";
	std::string comma = ",";
	for (int col = 0; col < this->get_col_count(); col++) {
		for (int row = 0; row < this->get_row_count(); row++) {
			ColorPatch* cp = this->get_color_patch(row, col);
			header += comma + cp->get_name();
			y_values += comma + std::to_string(cp->get_y());
			x_values += comma + std::to_string(cp->get_x());
			z_values += comma + std::to_string(cp->get_z());
			L_values += comma + std::to_string(cp->get_L());
			a_values += comma + std::to_string(cp->get_a());
			b_values += comma + std::to_string(cp->get_b());
		}
	}
	std::cout << this->f_name << std::endl;
	std::cout << "Xn: " << this->white_pts->get_white_point(WhitePoints::ValueType::Xn);
	std::cout << comma << " Yn: " << this->white_pts->get_white_point(WhitePoints::ValueType::Yn);
	std::cout << comma << " Zn: " << this->white_pts->get_white_point(WhitePoints::ValueType::Zn) << std::endl;

	std::cout << header << std::endl;
	std::cout << x_values << std::endl;
	std::cout << y_values << std::endl;
	std::cout << z_values << std::endl << std::endl;
	std::cout << L_values << std::endl;
	std::cout << a_values << std::endl;
	std::cout << b_values << std::endl << std::endl;
	ColorPatch* cp = this->get_estimated_white_patch();
	std::cout << "White Patch," << cp->get_name() << std::endl;
	std::cout << "Y Value, " << cp->get_y() << ",Row," << cp->get_row() << ",Col," << cp->get_col() << std::endl << std::endl << std::endl;
}

#include "utils/calibration_util.hpp"
cv::Mat RefData::as_matrix(){
	int col_count = this->row_count * this->col_count;
	cv::Mat ref_data = cv::Mat_<double>(REFLECTANCE_SIZE, col_count, CV_32FC1);
	for(int mat_row = 0; mat_row < ref_data.rows; mat_row++){
		int wave_len = INDEX_TO_WAVELEN(mat_row);
		// The Color patches are stored in
			// A1, B1, C1, ..., K1
			// A2, B2, C2, ..., K2
			// ..., ..., ..., ..., ...
			// Ak, Bk, Ck, ..., Kk
		for(int row = 0; row < this->row_count; row++){
			for(int col = 0; col < this->col_count; col++){
				int mat_col = col + row * this->col_count;
				ColorPatch *cp = this->get_color_patch(row, col);
				double reflectance_value = cp->get_ref_by_wavelen(wave_len);
				// std::cout << cp->get_name() << ": " << cp->get_ref_by_wavelen(wave_len) << " mat_col:" << mat_col << " wavelen: " << wave_len  << std::endl;
				ref_data.at<double>(mat_row, mat_col) = reflectance_value;
				// btrgb::calibration::enter_to_continue();
			}
			// std::cout << std::endl << std::endl;
		}
	}
	return ref_data;
}

cv::Mat RefData::xyz_as_matrix(){
	int col_count = this->row_count * this->col_count;
	cv::Mat xyz = cv::Mat_<double>(3, col_count, CV_32FC1);
	for(int row = 0; row < this->row_count; row++){
		for(int col = 0; col < this->col_count; col++){
			int mat_col = col + row * this->col_count;
			ColorPatch *cp = this->get_color_patch(row, col);
			xyz.at<double>(0, mat_col) = cp->get_x();
			xyz.at<double>(1, mat_col) = cp->get_y();
			xyz.at<double>(2, mat_col) = cp->get_z();
		}
	}
	return xyz;
}

bool RefData::is_custom(std::string file){

	for (int i = 0; i < REF_COUNT; i++) {
		if (ref_files[i].find(file) != std::string::npos) {
			return false;
		}
	}
	std::string originalFilePath = file;

	std::filesystem::path p(file);
	std::string filename = p.filename().string();

	std::string customFileName = REF_DATA_PATH + filename;
	LAST_USED_FILE_NAME = customFileName;
	std::string newFilePath = customFileName;

	// Copy the file
	std::cout << "Copy file names:" << std::endl;
	std::cout << originalFilePath << std::endl;
	std::cout << newFilePath << std::endl;

	std::ifstream src(originalFilePath, std::ios::binary);
	std::ofstream dst(newFilePath, std::ios::binary);

	if (!src) {
		std::cerr << "Error: Unable to open source file for copying: " << originalFilePath << std::endl;
		return false; // or handle error appropriately
	}

	if (!dst) {
		std::cerr << "Error: Unable to open destination file for copying: " << newFilePath << std::endl;
			
		return false; // or handle error appropriately
	}

	dst << src.rdbuf(); // Copy contents

	src.close();
	dst.close();
	if (file.find("Reflectance_Data.csv") != std::string::npos) {
		std::string file_path = REF_DATA_PATH+ filename;
		this->read_in_data(file_path);
		this->init_color_patches();
	}
	save_last_used_file(LAST_USED_FILE_NAME);
	return true;
}


void RefData :: save_last_used_file(const std::string& path) {
	
	std::ofstream out_file(LAST_USED_FILE_NAME, std::ios::trunc);
	if (out_file) {
		out_file << LAST_USED_FILE_NAME;
		out_file.close();
	}
	else {
		std::cerr << "Error: Unable to open file to save last used ref data: " << LAST_USED_FILE_NAME << std::endl;
	}
}

void RefData :: load_last_used_file() {
	std::ifstream in_file(LAST_USED_FILE_NAME);
	if (in_file) {
		this -> read_in_data(LAST_USED_FILE_NAME);
	}
	else {
		std::cerr << "Error: Unable to open file to load last used ref data: " << LAST_USED_FILE_NAME << std::endl;
	}
}
