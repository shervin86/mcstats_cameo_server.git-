#ifndef SIM_REQUEST_CLASS
#define SIM_REQUEST_CLASS
#include "nlohmann/json.hpp"
#include <cassert>
#include <fstream>
#include <functional>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <vector>

/**
 * \class sim_request
 * \brief helper to code and decode simulation requests between Nomad and the mcstas_server
 * \author Shervin Nourbakhsh nourbakhsh@ill.fr
 */

/* checking the the request is valid, should happen in this class
 * because it is used by both the client and the server and so it can make sure at compilation time
 * that both can understand the request
 */

class sim_request
{
	public:
	/** \brief constructor to be used on the client side
	 *  \param[in] j : json with the simulation parameters and instrument name
	 */
	sim_request(nlohmann::json j) : _j(j)
	{
		check_json();
		_instrument = _j["instrument"];
	};

	sim_request(std::ifstream &jsonfile)
	{
		_j = nlohmann::json::parse(jsonfile);
		check_json();
		_instrument = _j["instrument"];
	};

	/** \brief constructor that decodes the message from CAMEO (server side)
	 *  \param[in] message : request in string form received by the server
	 */
	sim_request(std::string message)
	{
		_j = nlohmann::json::parse(message);
		check_json();
		_instrument = _j["instrument"];
	}

	/// returing the string "SIM"+name of the instrument
	inline std::string instrument_name(void) const { return "SIM" + _instrument; };

	/// returns the arguments to be passed to the mcstas execution
	inline std::vector<std::string> args(void) const
	{
		std::vector<std::string> args;
		for (const auto &i : _j.items()) {
			if (i.key() == "instrument")
				continue;

			std::stringstream s;
			s << i.key() << "=" << i.value();
			args.push_back(s.str());
		}
		return args;
	}

	/// \brief pretty print of the request in json format
	friend std::ostream &operator<<(std::ostream &os, const sim_request &s);

	/** \brief transform the request into a string to be sent through CAMEO
	 * this method is kept if one wants to decouple the printing and the encoding into string
	 */
	inline std::string to_string(void) const { return _j.dump(); }
	inline std::string string(void) const { return _j.dump(); }

	/** \brief returns the hash of the entire request string */
	inline std::string hash(void) const { return std::to_string(_hash(to_string())); }

	private:
	nlohmann::json         _j;
	std::string            _instrument;
	std::hash<std::string> _hash; // this class calculates the hash from the string

	/** \brief checks if the request is valid
	 * \FIXME replace asserts with exceptions
	 * \return TRUE if everything is OK and FALSE otherwise
	 */
	bool check_json(void) const
	{
		bool ret = true; // true = OK
		// check mandatory parameters first
		check_json_common();
		return ret;
	}

	/** \brief checks for mandatory elements of the json request, common to all instruments
	 * \return TRUE if everything is OK and FALSE otherwise
	 */
	inline bool check_json_common(void) const
	{
		assert(_j.contains("instrument"));
		assert(_j.contains("--ncount"));
		return true;
	}

	inline bool check_json_source(void) const
	{
		assert(_j.contains("lambda"));
		return true;
	}
};

std::ostream &operator<<(std::ostream &os, const sim_request &s)
{
	os << std::setw(4) << s._j;
	return os;
}
#endif
