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
// definition of stages and their string representation
#include "stages.hh"
/**
 * \class sim_request
 * \brief code and decode simulation requests from client to server
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
	/** \brief empty constructor: parameters should be added one by one
	 *                            with dedicated methods
	 *
	 * \ingroup clientAPI
	 *   methods to be used:
	 *   - set_num_neutrons()
	 *   - set_instrument()
	 *   - add_parameter()
	 *   - set_return_data()
	 */
	sim_request(void){};

	/** \brief Specify what you want the server to return in the result 
	 * \ingroup clientAPI 
	 */
	enum returnType {
		rNONE = 0, ///< return only the exit status and no data
		rCOUNTS,   ///< return the weighted counts on the detector
		rERRORS,   ///< return the per pixel errors
		rNEUTRONS, ///< return the number of MC neutrons on the detector
		rALL,      ///< return the three pixel maps counts, errors, true MC neutrons
		rFULL      ///< return the entire output directory in TGZ format
	};
	/** \brief implemented instruments
	 * \ingroup clientAPI
	 */
	enum instrument_t { D22 /** D22 Detector */ };

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

	void read_json(std::ifstream &jsonfile)
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
	inline void set_instrument(instrument_t instr = D22)
	{
		switch (instr) {
		case D22:
			_instrument              = "D22";
			_j["instrument"]["name"] = _instrument;
			_j["source"]             = nlohmann::json::object();
			_j["detector"]           = nlohmann::json::object();
			_j["sample"]             = nlohmann::json::object();
			_j["mcpl"]               = nlohmann::json::object();
			break;
		}
	};
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
	inline void add_parameter(size_t stage, std::string name, double value)
	{
		switch (stage) {
		case sFULL:
			_j["source"][name] = value;
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

	/** \brief request results
	 *  \ingroup clientAPI
	 *  \param[in] iret : what to return as defined by #returnType. 
	 */
	inline void set_return_data(returnType iret = rNONE)
	{
		switch (iret) {
		case rNONE:
			_j["return"] = "NONE";
			break;
		case rCOUNTS:
			_j["return"] = "COUNTS";
			break;
		case rERRORS:
			_j["return"] = "ERRORS";
			break;
		case rNEUTRONS:
			_j["return"] = "NEUTRONS";
			break;
		case rFULL:
			_j["return"] = "FULL";
			break;
		}
	}

	/** \brief what is required to be returned */
	inline returnType get_return_data(void)
	{
		if ((!_j.contains("return")) or _j["return"] == "NONE")
			return rNONE;
		if (_j["return"] == "COUNTS")
			return rCOUNTS;
		if (_j["return"] == "ERRORS")
			return rERRORS;
		if (_j["return"] == "NEUTRONS")
			return rNEUTRONS;
		if (_j["return"] == "FULL")
			return rFULL;
		else
			assert(false);
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
			if (i.key() == "return")
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

	private:
	nlohmann::json _j;
	std::string    _instrument;
	// std::hash<std::string> _hash; // this class calculates the hash from the string
	std::hash<nlohmann::json> _hash;

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
