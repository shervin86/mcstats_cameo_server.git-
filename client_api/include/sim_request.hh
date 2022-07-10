#ifndef SIM_REQUEST_CLASS_HH
#define SIM_REQUEST_CLASS_HH

#include "nlohmann/json.hpp"
#include <fstream>
#include <stdexcept>

// which instruments are defined
#include "instruments.hh"
// definition of stages and their string representation
#include "stages.hh"
// which samples are implemented
#include "samples.hh"

namespace panosc
{
///\brief Name of the responder in the CAMEO communication \ingroup clientAPI
static const std::string CAMEO_RESPONDER = "mcstas_responder";
static const std::string CAMEO_PUBLISHER = "mcstas_publisher";

/** \exception param_not_implemented
 * \brief Exception thrown if trying to add a parameter that is defined but not implemented
 * \ingroup clientAPI
 */
class param_not_implemented : public std::runtime_error
{
	public:
	param_not_implemented(const char *what) : std::runtime_error(what){};
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
	/** \brief Specify the request type
	 * \ingroup clientAPI */
	enum req_t {
		SIMULATE = 0, ///< SIMULATE
		STOP,         ///< stop the ongoing simulation
		QUICK,        ///< run a QUICK simulation using the SimpleSource component before the sample
		CLEAR,        ///< clear the cache (to be used only by experts)
		REQUNKNOWN    ///< for checking not implemented requests
	};

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

	/** \brief list of accepted parameters (floats) \ingroup clientAPI */
	enum param_t {
		pWAVELENGTH,        ///< neutron wavelength measured in 10^-10 m [ang]
		pSOURCE_SIZE_X,     ///< NOT IMPLEMENTED YET
		pSOURCE_SIZE_Y,     ///< NOT IMPLEMENTED YET
		pSAMPLE_SIZE_R,     ///< sample radius if sphere or cylinder [m]
		pSAMPLE_SIZE_X,     ///< NOT IMPLEMENTED YET
		pSAMPLE_SIZE_Y,     ///< height of the sample for cylinder or box [m]
		pSAMPLE_SIZE_Z,     ///< NOT IMPLEMENTED YET
		pDETECTOR_DISTANCE, ///< NOT IMPLEMENTED YET
		pBEAMSTOP_X,        ///< NOT IMPLEMENTED YET
		pBEAMSTOP_Y,        ///< NOT IMPLEMENTED YET
		pATTENUATOR,        ///< NOT IMPLEMENTED YET
		pTHICKNESS,         ///< NOT IMPLEMENTED YET
		pCOLLIMATION,       ///< collimation
		pA2,                ///< first axis
		pA4,                ///< second axis
		pA6,                ///< third axis
		pA3,                ///< sample rotation
		pNOTIMPLEMENTED,    ///< FOR UNIT TESTS
	};

	/** \brief structure defining some information about each parameter
	 * it is used internally by the library, but it is publicly exposed
	 * to let the client list and print the list of available parameters and their predefined values
	 */
	struct param_data {
		stage_t     stage;
		std::string name;
		float       min;
		float       max;
		std::string units;
	};

	/** \brief list of sample shapes */
	// enum sample_shape_t {
	// 	ssSPHERE, ///< sphere, radious defined bt the p_SAMPLE_SIZE_X parameter
	// }

	/** \brief empty constructor: parameters should be added one by one
	 *                            with dedicated methods
	 *
	 *   methods to be used:
	 *   - set_num_neutrons()
	 *   - set_instrument()
	 *   - add_parameter()
	 *   - set_return_data()
	 */
	sim_request(req_t request_type, instrument_t instr)
	{
		set_instrument(instr);
		set_type(request_type);
	};

	sim_request(std::ifstream &json_file) { read_json(json_file); }
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

	/** \brief sets the request as defined by req_t */
	void set_type(req_t request_type)
	{
		_type      = request_type;
		_j["type"] = request_type;
	};

	/** \brief type of sample among those implemented
	 * \param[in] material : choosing a material among those implemented
	 */
	void set_sample_material(sample_material_t material = H2O);

	/**{@ \brief the shape of the sample is inferred from the number of parameters
	 * this is not supposed to be like that, but the sample parameters should
	 * come from a dedicated class
	 */
	void set_sample_size(double radius);                ///< sphere
	void set_sample_size(double radius, double height); ///< cylinder
	void set_sample_size(double x, double y, double z); ///< box/plate
	/**@}*/

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

	/** \brief map containing the list of parameters defined in the simulation and some information
	 * \todo improve the documentation
	 */
	// define here the stage for each parameter, it should match the McStas instrument implementation
	// define here the name for each parameter, it should match the McStas instrument implementation
	static const std::map<param_t, param_data> param_names;

	bool is_param_implemented(param_t par) const
	{
		const auto &par_data = param_names.at(par);
		return par_data.stage != sNONE;
	}

	protected:
	sim_request(void){}; // empty contructor only for sim_request_server
	nlohmann::json _j;
	instrument_t   _instrument;
	req_t          _type;

	private:
	static const double FLUX; ///< \brief Flux of the source assumed for the measurement time to number of
	                          ///< neutrons conversion
};

} // namespace panosc
#endif
