#ifndef REF_DATA_H
#define REF_DATA_H

#include <string>
#include <opencv2/opencv.hpp>
//#include <iostream>
#include "utils/csv_parser.hpp"
#include "color_patch.hpp"
#include "illuminants.hpp"
#include "standard_observer.hpp"
#include "white_points.hpp"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>


#define REF_COUNT 4

typedef Illuminants::IlluminantType IlluminantType;
typedef StandardObserver::ObserverType ObserverType;

/**
* Class representing a spicific set of reference data.
* This class will read in its data from given file_name.
* It can provide ColorPatches, find the white patch,
* and provide Tristimulus/CIELAB values
*/
class RefData: public CSVParser {
	std::vector<std::string> ref_files; // Use a vector instead of a fixed-size array
	std::string dataFilePath;           // Path to the file where the reference list is saved

public:
	RefData(const std::string& file, IlluminantType illum_type = IlluminantType::D50, ObserverType so_type = ObserverType::SO_1931, bool batch = false);



	~RefData();





bool addCustomRefData(const std::string& new_ref_data) {
	// Add the new reference data file to the list
	ref_files.push_back(new_ref_data);
	return saveRefDataList(); // Save the updated list to file
}

bool saveRefDataList() const {
	// Save the current list of reference data files to the specified data file
	std::ofstream file_out(dataFilePath);
	if (!file_out) {
		std::cerr << "Error: Unable to open file for writing: " << dataFilePath << std::endl;
		return false;
	}

	for (const auto& ref_file : ref_files) {
		file_out << ref_file << std::endl;
	}

	file_out.close();
	return true;
}


bool loadRefDataList() {
	namespace fs = std::filesystem;

	for (const auto& entry : fs::directory_iterator(dataFilePath)) {
		if (entry.is_regular_file()) { // Check if the entry is a file
			// Add the full path of the file to the list
			ref_files.push_back(entry.path().string());
		}
	}
	return true;
}


	static IlluminantType get_illuminant(std::string illuminant_string);
	static ObserverType get_observer(int observer_num);

	/**
	* Retrives a reference to a ColorPatch at a given row and col
	* @param row: the row to get the ColorPatch from
	* @param col: the col to get the ColorPatch from
	* @return: pointer to the ColorPatch
	*/
	ColorPatch* get_color_patch(int row, int col);

	/**
	* Finds the white ColorPatch as defined by the reference data. The whit patch is the one with
	* the hightest y value.
	* NOTE: this is not the same as the user defined white patch
	* @return: a pointer to the white ColorPatch
	*/
	ColorPatch* get_estimated_white_patch();

	/**
	* Get the row the white patch is in as defined by the reference data
	* @return: white patch row as an int
	*/
	int get_estimated_white_patch_row();

	/**
	* Get the colum the white patch is in as defined by the reference data
	* @return: white patch colum as an int
	*/
	int get_estimated_white_patch_col();

	/**
	* Gets the x value from the ColorPatch at the given row and col
	* @param row: the row of the ColorPatch to get x value from
	* @param col: the colum of the ColorPatch to get x value from
	*/
	double get_x(int row, int col);

	/**
	* Gets the y value from the ColorPatch at the given row and col
	* @param row: the row of the ColorPatch to get y value from
	* @param col: the colum of the ColorPatch to get y value from
	*/
	double get_y(int row, int col);

	/**
	* Gets the z value from the ColorPatch at the given row and col
	* @param row: the row of the ColorPatch to get z value from
	* @param col: the colum of the ColorPatch to get z value from
	*/
	double get_z(int row, int col);

	/**
	* Gets the L value from the ColorPatch at the given row and col
	* @param row: the row of the ColorPatch to get L value from
	* @param col: the colum of the ColorPatch to get L value from
	*/
	double get_L(int row, int col);

	/**
	* Gets the a value from the ColorPatch at the given row and col
	* @param row: the row of the ColorPatch to get a value from
	* @param col: the colum of the ColorPatch to get a value from
	*/
	double get_a(int row, int col);

	/**
	* Gets the b value from the ColorPatch at the given row and col
	* @param row: the row of the ColorPatch to get b value from
	* @param col: the colum of the ColorPatch to get b value from
	*/
	double get_b(int row, int col);

	/**
	* Gets the number of rows the RefData holds
	* @return: number of rows
	*/
	int get_row_count();

	/**
	* Gets the number of colums the RefData holds
	* @return: number of colums
	*/
	int get_col_count();

	/**
	 * @brief Get the white pts ptr
	 * 
	 * @return WhitePoints* 
	 */
	WhitePoints* get_white_pts();

	/**
	* Ouputs RefData to standard out.
	* This is used to capture Reference data to verify 
	* that values are computed correctly
	* Format:
	* RefData file_name
	* Valuetype,A:1,A:2,A:3,...,B:1,B:2,B:3,...,...,...
	* X,value1,value2,value3,...
	* Y,value1,value2,value3,...
	* Z,value1,value2,value3,...
	* WhitPatch,<WhiteColorPatch_name>
	* Y Value,<White ColorPatch yvalue>,Row,<White ColorPatch row>,Col,<White ColorPatch col> 
	*/
	void output_xyz();

	/**
	 * @brief Convert refdata to a MxN matrix
	 * 	M is the number of wavelenghts
	 * 	N is the number of ColorPatches
	 * The ColorPatches are added in order of
	 * 	A1_wavlen380, B1_wavlen380, C1_wavlen380, ..., K1_wavlen380, A2_wavlen380, ... , Kk_wavlen380
	 *  A1_wavlen390, B1_wavlen390, C1_wavlen390, ..., K1_wavlen390, A2_wavlen390, ... , Kk_wavlen390
	 *  ...			, ...		  , ...			, ..., ...		   , ...		 , ... , ...								
	 *  A1_wavlen730, B1_wavlen730, C1_wavlen730, ..., K1_wavlen730, A2_wavlen730, ... , Kk_wavlen730
	 * 
	 * 
	 * @return cv::Mat 
	 */
	cv::Mat as_matrix();

	/**
	 * @brief Out put the xyz_ref as a 3XN matrix
	 * where 
	 * 	N is the number of ColorPatches
	 * 	n is the number of rows
	 * 	k is the number of colums
	 * The ColorPatches are added in order of
	 * 	x_A:0, x_B:0, ..., x_k:0, x_A:1, x_B:1, ..., x_k:1, ..., x_k:n
	 * 	y_A:0, y_B:0, ..., y_k:0, y_A:1, y_B:1, ..., y_k:1, ..., y_k:n
	 * 	z_A:0, z_B:0, ..., z_k:0, z_A:1, z_B:1, ..., z_k:1, ..., z_k:n
	 * 
	 * @return cv::Mat 
	 */
	cv::Mat xyz_as_matrix();

	void RefDataFolder(const std::string folder);


private:
	/**
	* Read in reference data from given file
	*/
	void read_in_data(std::string file_path);

	/**
	* Pars given csv line and append data to related ColorPatch
	*/
	void pars_line(std::string line);

	/**
	* Pars header csv line and append data to related ColorPatch
	*/
	void pars_header(std::string header);

	/**
	* Pars given header and identify number of rows and colums 
	* needed to house ColorPatches
	*/
	void identify_data_size(std::string header);

	/**
	* Initialize ColorPatch storage
	*/
	void init_data_storage();

	/**
	* Parse given item from header to extrac the letter
	* Header item format: "A:1"
	* Given "A:1" this would retur "A"
	*/
	std::string get_col_id(std::string header_item);

	/**
	* Initialize ColorPatch Tristimulus and CIELAB values
	*/
	void init_color_patches();

	bool is_custom(std::string file);

	void save_last_used_file(const std::string& path);

	void load_last_used_file();

	ColorPatch*** color_patches;
	StandardObserver* observer = nullptr;
	Illuminants* illuminants = nullptr;
	WhitePoints* white_pts = nullptr;
	std::string f_name;
	int row_count;
	int col_count;
	bool batch;



	
};

class RefDataError : public std::exception {};

class RefData_FailedToRead : public RefDataError{
	public:
	RefData_FailedToRead(){};
	RefData_FailedToRead(std::string file){this->error = "RefData Error: failed to read " + file;}
    virtual char const * what() const noexcept { return error.c_str(); }

	private:
	std::string error = "RefData Read Error";
};

class RefData_ParssingError : public RefDataError{
	public:
	RefData_ParssingError(){};
	RefData_ParssingError(std::string msg){this->error = "RefData Parsing Error: invalid file format -> " + msg;}
    virtual char const * what() const noexcept { return error.c_str(); }

	private:
	std::string error = "RefData Parsing Error";
};
#endif //REF_DATA_H