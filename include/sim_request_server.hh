#ifndef SIM_REQUEST_SERVER_HH
#define SIM_REQUEST_SERVER_HH

#include "sim_request.hh"
namespace panosc_sim_server
{
class sim_request_server : public sim_request
{
	public:
	/** \brief constructor that decodes the message from CAMEO (server side)
	 *  \param[in] message : request in string form received by the server
	 */
	sim_request_server(std::string message)
	// sim_request::sim_request(std::string message)
	{
		_j = nlohmann::json::parse(message);
		check_json();
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

	private:
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

	/** \brief constructor to be used for development
	 *  \param[in] j : json with the simulation parameters and instrument name
	 */
	/*
	        sim_request_server(nlohmann::json j) : _j(j)
	        {
	                assert(check_json()); /// \todo use exception
	                _instrument = _j["instrument"]["name"];
	        };
	*/
	/** \brief constructor to be used for development
	 *  \param[in] jsonfile : input stream with the simulation parameters
	 *                        and instrument name in json format
	 *
	 *  json example
	 * \include request.json
	 */
	/*
	        sim_request_server(std::ifstream &jsonfile):
	        {
	                _j = nlohmann::json::parse(jsonfile);
	                assert(check_json()); ///\todo use exception
	                _instrument = _j["instrument"]["name"];
	        };
	*/
};
} // namespace panosc_sim_server

#endif
