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

// this enum has to be continous since there is a loop over the stages

static const std::vector<std::string> stages = {"sDETECTOR", "sSAMPLE", "sFULL"};
/**
 * \class sim_request
 * \brief helper to code and decode simulation requests between Nomad and the mcstas_server
 * \author Shervin Nourbakhsh nourbakhsh@ill.fr
 */

/* checking the the request is valid, should happen in this class
 * because it is used by both the client and the server and so it can make sure at compilation time
 * that both can understand the request
 *
 * JSON format:
 *
 *
 */

class sim_request
{
	public:
	static const size_t sFULL     = 2;
	static const size_t sSAMPLE   = 1;
	static const size_t sDETECTOR = 0;

	/** \brief implemented instruments
	 * \ingroup clientAPI
	 */
	enum instrument_t { D22 };

	/* \brief different stages of the simulation
	 *
	 * At each stage, a MCPL file is saved with the neutrons at that stage
	 * a previously produced MCPL file (or simulation result) is used if the hash for that stage
	 * matches but not the previous
	 */

	/** \brief constructor to be used for development
	 *  \param[in] j : json with the simulation parameters and instrument name
	 */
	sim_request(nlohmann::json j) : _j(j)
	{
		assert(check_json()); /// \todo use exception
		_instrument = _j["instrument"]["name"];
	};

	/** \brief constructor to be used for development
	 *  \param[in] jsonfile : input stream with the simulation parameters
	 *                        and instrument name in json format
	 *
	 *  json example
	 * \include request.json
	 */
	sim_request(std::ifstream &jsonfile)
	{
		_j = nlohmann::json::parse(jsonfile);
		assert(check_json()); ///\todo use exception
		_instrument = _j["instrument"]["name"];
	};

	/** \brief constructor that decodes the message from CAMEO (server side)
	 *  \param[in] message : request in string form received by the server
	 */
	sim_request(std::string message)
	{
		_j = nlohmann::json::parse(message);
		check_json();
		_instrument = _j["instrument"]["name"];
	}

	/** \brief empty constructor: parameters should be added one by one
	 *                            with dedicated methods
	 * \ingroup clientAPI
	 *   methods to be used:
	 *   - set_num_neutrons()
	 *   - set_instrument()
	 *   - add_parameter()
	 */
	sim_request(void){};

	/**
	 * \brief set the number of neutrons to simulate
	 * \ingroup clientAPI
	 */
	inline void set_num_neutrons(unsigned long int n) { _j["--ncount"] = n; }

	/** \brief sets the instrument
	 *  \ingroup clientAPI
	 * \details Implemented instruments:
	 * #instrument_t
	 */
	void set_instrument(instrument_t instr = D22)
	{
		switch (instr) {
		case D22:
			_instrument              = "D22";
			_j["instrument"]["name"] = _instrument;
			break;
		}
	}
	/** \brief add simulation parameter
	 * \ingroup clientAPI
	 * \param[in] stage : it defines to which stage of the simulation the parameter belongs. It
	 * can be:
	 *    - sim_request::#sFULL
	 *    - sim_request::#sSAMPLE
	 *    - sim_request::#sDETECTOR
	 * \param[in] name : name of the parameter, it should match the name in McStas
	 * \param[in] value : the value of the parameter, only float is implemented
	 */
	inline void add_parameter(size_t stage, std::string name, float value)
	{
		switch (stage) {
		case sFULL:
			_j["source"]["name"] = value;
			break;
		case sDETECTOR:
			_j["detector"][name] = value;
			break;
		case sSAMPLE:
			_j["sample"][name] = value;
			break;
		}
		return;
	}

	/// returing the string "SIM"+name of the instrument
	inline std::string instrument_name(void) const { return "SIM" + _instrument; };

	/// returns the arguments to be passed to the mcstas execution
	inline std::vector<std::string> args(void) const
	{
		std::vector<std::string> args;

		for (const auto &i : _j.items()) {
			if (i.value().type() == nlohmann::json::value_t::object)
				continue;

			std::stringstream s;
			s << i.key() << "=" << i.value();
			args.push_back(s.str());
		}

		for (const auto &i : _j["source"].items()) {
			std::stringstream s;
			s << i.key() << "=" << i.value();
			args.push_back(s.str());
		}

		for (const auto &i : _j["sample"].items()) {
			std::stringstream s;
			s << i.key() << "=" << i.value();
			args.push_back(s.str());
		}

		for (const auto &i : _j["detector"].items()) {
			std::stringstream s;
			s << i.key() << "=" << i.value();
			args.push_back(s.str());
		}

		return args;
	}

	/// \brief pretty print of the request in json format
	friend std::ostream &operator<<(std::ostream &os, const sim_request &s);

	inline std::string to_string(void) const { return _j.dump(); }
	inline std::string string(void) const { return _j.dump(); }

	/** \brief transform the request into a string to be sent through CAMEO
	 * \ingroup clientAPI
	 * \details
	 * this method is kept if one wants to decouple the printing and the encoding into string
	 *
	 * Example use with cameo:
	 * \code{.cpp}
	 * sim_request request();
	 * requester->sendBinary(request.to_cameo());
	 * \endcode
	 * \todo replace plain json with MessagePack or CBOR or something else
	 */
	inline std::string to_cameo(void) const { return _j.dump(); }

	/** \brief returns the hash of the entire request string */
	inline std::string hash(void) const { return std::to_string(_hash(to_string())); }

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
		return std::to_string(_hash(j.dump()));
	}

	private:
	nlohmann::json         _j;
	std::string            _instrument;
	std::hash<std::string> _hash; // this class calculates the hash from the string

	/** \brief checks if the request is valid
	 * \todo replace asserts with exceptions
	 * \return TRUE if everything is OK and FALSE otherwise
	 */
	bool check_json(void) const
	{
		bool ret = true; // true = OK
		// check mandatory parameters first
		ret = ret && check_json_common();
		return ret;
	}

	/** \brief checks for mandatory elements of the json request, common to all instruments
	 * \return TRUE if everything is OK and FALSE otherwise
	 */
	inline bool check_json_common(void) const
	{
		bool ret = true;
		// \todo use exceptions here
		assert(_j.contains("--ncount"));
		assert(_j.contains("instrument"));
		assert(_j.contains("source"));
		assert(_j.contains("sample"));
		ret = ret && check_json_source();
		ret = ret && check_json_sample();
		ret = ret && check_json_instrument();
		return ret;
	}

	inline bool check_json_source(void) const { return _j["source"].contains("lambda"); }
	inline bool check_json_sample(void) const { return true; }
	inline bool check_json_instrument(void) const { return _j["instrument"].contains("name"); }
};

std::ostream &operator<<(std::ostream &os, const sim_request &s)
{
	os << std::setw(4) << s._j;
	return os;
}
#endif
