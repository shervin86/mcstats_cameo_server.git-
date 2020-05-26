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

namespace panosc_sim_server
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
		_instrument = _j["instrument"]["name"];
	}

	/// returing the string "SIM"+name of the instrument
	std::string instrument_name(void) const { return "SIM" + _instrument; };

	/// returns the arguments to be passed to the mcstas execution
	std::vector<std::string> args(void) const;

	/** \brief what is required to be returned */
	returnType get_return_data(void) const;

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
		for (size_t istage = 0; istage < stages.size(); ++istage) {
			hashes.push_back(hash(istage));
		}
		return hashes;
	}

	std::string to_string(void) const { return _j.dump(); }

	private:
	std::hash<nlohmann::json> _hash;
};
} // namespace panosc_sim_server

#endif
