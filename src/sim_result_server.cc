#include "sim_result_server.hh"
#include <iostream>
#include <sstream>

namespace panosc
{

void sim_result_server::read_file(std::ifstream &f)
{
	std::string type;
	// std::shared_ptr< std::vector<float> > z(_counts);
	std::vector<float> *z = nullptr;
	while (f.good()) {
		std::string line;
		getline(f, line);
		if (line.front() == '#') { // comment block
			if (line.find("type") != std::string::npos) {
				type = line.substr(2);
				// need to parse the line to get the size
				_dim_x = 128;
				_dim_y = 128;
			} else if (line.find("Data") != std::string::npos) {
				if (line.find("I:"))
					z = &_counts;
				else if (line.find("N:"))
					z = &n;
			}
		} else { // data block
			std::stringstream ss(line);
			while (ss.good()) {
				float iz;
				ss >> iz;
				z->push_back(iz);
			}
		}
	}
	std::cout << type << std::endl;
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
