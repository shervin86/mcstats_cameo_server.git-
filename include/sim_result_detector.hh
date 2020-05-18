#ifndef SIM_RESULT_DETECTOR_CLASS
#define SIM_RESULT_DETECTOR_CLASS
#include "nlohmann/json.hpp"
#include <cassert>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <sstream>
#include <vector>
/**
 * \class sim_result_detector
 * \brief helper to code and decode simulation results (only detector image) between Nomad and the
 * mcstas_server
 * \author Shervin Nourbakhsh nourbakhsh@ill.fr
 */

class sim_result_detector
{
	public:
	/** \brief constructor reading the mcstas detector file (server-side)
	 *
	 */
	sim_result_detector(std::ifstream &f)
	{
		std::string type;
		// std::shared_ptr< std::vector<float> > z(counts);
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
						z = &counts;
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
	};

	/** \brief transform the result into a string to be sent through CAMEO (server side)
	 *
	 * \details
	 *
	 * Example use with cameo:
	 * \code{.cpp}
	 * sim_result detector();
	 * request->replyBinary(sim_result.to_cameo());
	 * \endcode
	 * \todo replace plain json with MessagePack or CBOR or something else
	 */
	inline std::string to_cameo(void) const
	{
		nlohmann::json j;
		j["data"]  = counts;
		j["dim_x"] = _dim_x;
		j["dim_y"] = _dim_y;

		// std::cout << j << std::endl;
		return j.dump();
	}

	/** \brief constructor that decodes the message from CAMEO (client side)
	 * \ingroup clientAPI
	 *  \param[in] message : request in string form received by the server
	 */
	sim_result_detector(std::string message)
	{
		nlohmann::json j = nlohmann::json::parse(message);
		_dim_x           = j["dim_x"];
		_dim_y           = j["dim_y"];
	}

	inline size_t dim_x(void) { return _dim_x; };
	inline size_t dim_y(void) { return _dim_y; };

	private:
	static const size_t DIM_X = 128;
	static const size_t DIM_Y = 128;
	float               _dim_x;
	float               _dim_y;
	std::vector<float>  counts, errors, n;
};

#endif
