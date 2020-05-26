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

namespace panosc_sim_server
{
/**
 * \class sim_request
 * \brief code and decode simulation requests from client to server
 * \author Shervin Nourbakhsh nourbakhsh@ill.fr
 * \ingroup clientAPI
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

	/**
	 * \brief set the number of neutrons to simulate
	 * \ingroup clientAPI
	 */
	void set_num_neutrons(unsigned long int n);

	/** \brief sets the instrument
	 *  \ingroup clientAPI
	 * \details Implemented instruments:
	 * #instrument_t
	 */
	void set_instrument(instrument_t instr = D22);

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
	void add_parameter(size_t stage, std::string name, double value);

	/** \brief request results
	 *  \ingroup clientAPI
	 *  \param[in] iret : what to return as defined by #returnType.
	 */
	void set_return_data(returnType iret = rNONE);

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
	std::string to_cameo(void) const;

	/*------------------------------ server side */

	/// \brief pretty print of the request in json format
	friend std::ostream &operator<<(std::ostream &os, const panosc_sim_server::sim_request &s);

	protected:
	nlohmann::json _j;
	std::string    _instrument;

	public:
	inline std::string to_string(void) const { return _j.dump(); }
	void               read_json(std::ifstream &jsonfile)
	{
		_j = nlohmann::json::parse(jsonfile);
		// assert(check_json()); ///\todo use exception
		_instrument = _j["instrument"]["name"];
	};
};

} // namespace panosc_sim_server
#endif
