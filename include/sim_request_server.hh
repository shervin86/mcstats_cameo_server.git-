#ifndef SIM_REQUEST_SERVER_HH
#define SIM_REQUEST_SERVER_HH

#include "sim_request.hh"
#include <cassert>
#include <fstream>
#include <functional>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <vector>

/*
  required double measurement_time        600 [seconds]
  repeated double source_size rectangular [30e-3, 30e-3] [m] or circular [30e-3] [m]
  required double col                     12.8, 10.3, 7.8, 5.3, or 2.8 [m]
  repeated double sample_size             rectangular [10e-3, 10e-3] [m] or circular [10e-3] [m]
  required double wav                     6 [angs]
  optional double det 10 [m] (detector distance for single detector instrument
  required double bx 0 [mm] (beamstop x position)
  required double by 0 [mm] (beamstop y position)
  required double attenuator 1, 100, 1000 (actual attenuation value in use)
  required double thickness 0.1 [cm] (sample thickness - applies only to the sample scatter model, not the
  background and blocked models
*/

namespace panosc
{
/** \brief decode simulation requests from client to server
 * \author Shervin Nourbakhsh nourbakhsh@ill.fr
 */
class sim_request_server : public sim_request
{
	public:
	/** \brief constructor that decodes the message from CAMEO (server side)
	 *  \param[in] message : request in string form received by the server
	 */
	sim_request_server(std::string message)
	{
		_j = nlohmann::json::parse(message);
		// check_json();
		_instrument = _j["instrument"]["name"].get<instrument_t>(); // take the enum here
	}

	/// returing the string "SIM"+name of the instrument
	std::string instrument_name(void) const
	{
		return "SIM" + _j["instrument"]["name"]
		                   .get<std::string>(); // take the string name here and not the enum
	};

	/// returns the arguments to be passed to the mcstas execution
	std::vector<std::string> args(void) const;

	/** \brief what is required to be returned */
	return_t get_return_data(void) const;

	// inline std::string string(void) const { return _j.dump(); }

	/** \brief returns the hash of the entire request string */
	// inline std::string hash(void) const { return std::to_string(_hash(to_string())); }
	inline std::string hash(void) const { return std::to_string(_hash(_j)); }

	inline std::string hash(size_t s) const
	{
		nlohmann::json j(_j);
		switch (s) {
		case sFULL:
			break;
		case sDETECTOR:
			j.erase("detector");
			break;
		case sSAMPLE:
			j.erase("detector");
			j.erase("sample");
			break;
		}
		return std::to_string(_hash(j));
	}

	inline std::vector<std::string> stage_hashes(void) const
	{
		std::vector<std::string> hashes;
		for (size_t istage = 0; istage < sFULL; ++istage) {
			hashes.push_back(hash(istage));
		}
		return hashes;
	}

	std::string to_string(void) const { return _j.dump(); }

	bool is_test(void) const { return _j["--ncount"] == "0"; };

	private:
	std::hash<nlohmann::json> _hash;
};
} // namespace panosc

#endif
