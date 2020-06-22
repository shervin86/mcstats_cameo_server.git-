#include "sim_result_server.hh"
#include <iostream>
#include <sstream>

namespace panosc
{

void sim_result_server::read_file(std::ifstream &f)
{
	
	std::string type;
	std::vector<float> *z = nullptr;
	while (f.good()) {
		std::string line;
		getline(f, line);
		if (line.empty())
			continue;
		if (line.front() == '#') { // comment block
			if (line.find("type") != std::string::npos) {
				type = line.substr(2);
				/// \todo throw exception
				assert(sscanf(type.c_str(), "type: array_2d(%lu,%lu)", &_dim_x, &_dim_y) ==
				       2);
			} else {
				if (line.find("I:") != std::string::npos) {
					z = &_counts;
					z->clear();
				} else if (line.find("I_err:") != std::string::npos) {
					z = &_errors;
					z->clear();
				} else if (line.find("N:") != std::string::npos) {
					z = &_n;
					z->clear();
				}
			}
		} else { // data block
			std::stringstream ss(line);
			while (ss.good()) {
				float iz;
				ss >> iz;
				if (ss.good())
					z->push_back(iz);
			}
		}
	}

	assert(z->size() == _dim_x * _dim_y);
}

std::string sim_result_server::to_cameo(void) const
{
	nlohmann::json j;
	j["data"]   = _counts;
	j["dim_x"]  = _dim_x;
	j["dim_y"]  = _dim_y;
	j["status"] = _status;
	// std::cout << j << std::endl;
	return j.dump();
}
} // namespace panosc
