#ifndef SIM_REQUEST_CLASS_HH
#define SIM_REQUEST_CLASS_HH

#include "nlohmann/json.hpp"
#include <fstream>
#include <stdexcept>

// definition of stages and their string representation
#include "stages.hh"

namespace panosc
{

/** \brief implemented instruments */
enum instrument_t { D22 /** D22 Detector */ };

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// map instrument_t values to JSON as strings
NLOHMANN_JSON_SERIALIZE_ENUM(instrument_t, {
                                               {D22, "D22"},
                                           })
#endif

/** \exception param_not_implemented
 * \brief Exception thrown if trying to add a parameter that is defined but not implemented
 * \ingroup clientAPI
 */
class param_not_implemented :  public std::runtime_error
{
public:
	param_not_implemented(const char* what): std::runtime_error(what){};
};
	

/**
 * \class sim_request
 * \brief code simulation requests from client to server
 * \author Shervin Nourbakhsh nourbakhsh@ill.fr
 * \ingroup clientAPI
 */

class sim_request
{
	public:
	/** \brief Specify what you want the server to return in the result
	 * \ingroup clientAPI */
	enum return_t {
		rNONE = 0, ///< return only the exit status and no data
		rCOUNTS,   ///< return the weighted counts on the detector
		rERRORS,   ///< return the per pixel errors
		rNEUTRONS, ///< return the number of MC neutrons on the detector
		rALL,      ///< return the three pixel maps counts, errors, true MC neutrons
		rFULL      ///< return the entire output directory in TGZ format
	};

	/** \brief list of accepted parameters \ingroup clientAPI */
	enum param_t {
		pWAVELENGTH,        ///< neutron wavelength measured in 10^-10 m (ang)
		pSOURCE_SIZE_X,     ///< NOT IMPLEMENTED YET
		pSOURCE_SIZE_Y,     ///< NOT IMPLEMENTED YET
		pSAMPLE_SIZE_X,     ///< NOT IMPLEMENTED YET
		pSAMPLE_SIZE_Y,     ///< NOT IMPLEMENTED YET
		pDETECTOR_DISTANCE, ///< NOT IMPLEMENTED YET
		pBEAMSTOP_X,        ///< NOT IMPLEMENTED YET
		pBEAMSTOP_Y,        ///< NOT IMPLEMENTED YET
		pATTENUATOR,        ///< NOT IMPLEMENTED YET
		pTHICKNESS,         ///< NOT IMPLEMENTED YET
		pCOLLIMATION,       ///< NOT IMPLEMENTED YET
		pNOTIMPLEMENTED,    ///< FOR UNIT TESTS
	};
	/** \brief empty constructor: parameters should be added one by one
	 *                            with dedicated methods
	 *
	 *   methods to be used:
	 *   - set_num_neutrons()
	 *   - set_instrument()
	 *   - add_parameter()
	 *   - set_return_data()
	 */
	sim_request(void){};

	/** \brief set the number of neutrons to simulate */
	void set_num_neutrons(unsigned long long int n);

	/** \brief set the number of neutrons starting from the acquisition time and assuming a FLUX
	 * for the source of 1.2e8
	 */
	void set_measurement_time(double time);

	/** \brief sets the instrument
	 * \details Implemented instruments:
	 * #instrument_t
	 */
	void set_instrument(instrument_t instr = D22);

	/** \brief add simulation parameter
	 * \param[in] par : parameter as defined in param_t
	 * \param[in] value : the value of the parameter, only float is implemented
	 */
	void add_parameter(param_t par, double value);

	/** \brief add simulation parameter
	 * \param[in] par : parameter as defined in param_t
	 * \param[in] vec : parameter is a vector of floats
	 */
	void add_parameter_array(param_t par, std::vector<double> &vec);

	/** \brief request results
	 *  \param[in] iret : what to return as defined by #return_t.
	 */
	void set_return_data(return_t iret = rNONE);

	/** \brief transform the request into a string to be sent through CAMEO
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

	/*------------------------------ for fakeNomad */

	/// \internal \brief pretty print of the request in json format, for debug purposes  \endinternal
	friend std::ostream &operator<<(std::ostream &os, const panosc::sim_request &s);

	void read_json(std::ifstream &jsonfile);

	protected:
	nlohmann::json _j;
	instrument_t   _instrument;

	private:
	static const double FLUX; ///< \brief Flux of the source assumed for the measurement time to number of neutrons conversion
};

} // namespace panosc
#endif
