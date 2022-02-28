//
// Defines a class to manage communication between a process and the front end
//

#include "comunication_obj.hpp"

unsigned char CommunicationObj::binID = 0;

CommunicationObj::CommunicationObj(server* s, websocketpp::connection_hdl hd1, message_ptr msg) {
	server_m = s;
	connectionHandle_m = hd1;
	opcode_m = msg->get_opcode();
}

CommunicationObj::CommunicationObj(const CommunicationObj &other) {
	server_m = other.server_m;
	connectionHandle_m = other.connectionHandle_m;
	opcode_m = other.opcode_m;
}

void CommunicationObj::send_msg(std::string msg) {
	server_m->send(connectionHandle_m, msg, opcode_m);
}

void CommunicationObj::send_bin(std::vector<uchar>& v){
	const void* binToSend = (void*)v.data();
	//Need to find out how to send bin without this send, since it needs a string for what it is sending
	server_m->send(connectionHandle_m, binToSend, v.size(), websocketpp::frame::opcode::binary);
}

void CommunicationObj::set_id(long newID){
	id = newID;
}

void CommunicationObj::send_info(std::string msg, std::string sender){
	jsoncons::json info_body;
	info_body.insert_or_assign("RequestID", id);
	info_body.insert_or_assign("ResponseType", "Info");
	jsoncons::json response_data;
	response_data.insert_or_assign("message", msg);
	response_data.insert_or_assign("sender", sender);
	info_body.insert_or_assign("ResponseData", response_data);
	std::string all_info;
	info_body.dump(all_info);
	std::cout<<all_info<<std::endl;
	send_msg(all_info);
}

void CommunicationObj::send_error(std::string msg, std::string sender){
	jsoncons::json info_body;
	info_body.insert_or_assign("RequestID", id);
	info_body.insert_or_assign("ResponseType", "Error");
	jsoncons::json response_data;
	response_data.insert_or_assign("message", msg);
	response_data.insert_or_assign("sender", sender);
	info_body.insert_or_assign("ResponseData", response_data);
	std::string all_info;
	info_body.dump(all_info);
	std::cout<<all_info<<std::endl;
	send_msg(all_info);
}

void CommunicationObj::send_progress(double val, std::string sender){
	jsoncons::json info_body;
	info_body.insert_or_assign("RequestID", id);
	info_body.insert_or_assign("ResponseType", "Progress");
	jsoncons::json response_data;
	response_data.insert_or_assign("value", val);
	response_data.insert_or_assign("sender", sender);
	info_body.insert_or_assign("ResponseData", response_data);
	std::string all_info;
	info_body.dump(all_info);
	std::cout<<all_info<<std::endl;
	send_msg(all_info);
}

void CommunicationObj::send_base64(btrgb::Image* image, enum btrgb::output_type type, enum btrgb::image_quality qual){
	jsoncons::json info_body;
	info_body.insert_or_assign("RequestID", id);
	info_body.insert_or_assign("ResponseType", "ImageBase64");
	jsoncons::json response_data;
	btrgb::base64_ptr_t b64 = image->toBase64OfType(type, qual);
	response_data.insert_or_assign("dataURL", *b64);
	response_data.insert_or_assign("name", image->getName());
	info_body.insert_or_assign("ResponseData", response_data);
	std::string all_info;
	info_body.dump(all_info);
	std::cout<<all_info<<std::endl;
	send_msg(all_info);
}

void CommunicationObj::send_binary(btrgb::Image* image, enum btrgb::output_type type, enum btrgb::image_quality qual){
	jsoncons::json info_body;
	info_body.insert_or_assign("RequestID", id);
	info_body.insert_or_assign("ResponseType", "ImageBinary");
	jsoncons::json response_data;
	response_data.insert_or_assign("id", this->binID);
	switch(type) {
			case btrgb::PNG: response_data.insert_or_assign("type", "png"); break;
			case btrgb::WEBP: response_data.insert_or_assign("type", "webp"); break;
	}
	response_data.insert_or_assign("name", image->getName());
	info_body.insert_or_assign("ResponseData", response_data);
	std::string all_info;
	info_body.dump(all_info);
	std::cout<<all_info<<std::endl;
	send_msg(all_info);
	btrgb::binary_ptr_t bin = image->toBinaryOfType(type, qual);
	bin->push_back(binID);
	send_bin(*bin);
	this->binID++;
}
