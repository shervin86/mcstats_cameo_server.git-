#ifndef SIM_REQUEST_CLASS_HH
#define SIM_REQUEST_CLASS_HH

#include "nlohmann/json.hpp"
#include <fstream>

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

/**
 * \class sim_request
 * \brief code simulation requests from client to server
 * \author Shervin Nourbakhsh nourbakhsh@ill.fr
 * \ingroup clientAPI
 */

class sim_request
{
	public:
	/** \brief Specify what you want the server to return in the result */
	enum returnType {
		rNONE = 0, ///< return only the exit status and no data
		rCOUNTS,   ///< return the weighted counts on the detector
		rERRORS,   ///< return the per pixel errors
		rNEUTRONS, ///< return the number of MC neutrons on the detector
		rALL,      ///< return the three pixel maps counts, errors, true MC neutrons
		rFULL      ///< return the entire output directory in TGZ format
	};

	/// \brief neutron wavelength measured in 10^-10 m (ang)
	static const std::string pWAVELENGTH;
	/// \brief NOT IMPLEMENTED
	static const std::string pSOURCE_SIZE_X;
	/// \brief NOT IMPLEMENTED
	static const std::string pSOURCE_SIZE_Y;
	/// \brief NOT IMPLEMENTED
	static const std::string pSAMPLE_SIZE_X;
	/// \brief NOT IMPLEMENTED
	static const std::string pSAMPLE_SIZE_Y;
	/// \brief NOT IMPLEMENTED
	static const std::string pDETECTOR_DISTANCE;
	/// \brief NOT IMPLEMENTED
	static const std::string pBEAMSTOP_X;
	/// \brief NOT IMPLEMENTED
	static const std::string pBEAMSTOP_Y;
	/// \brief NOT IMPLEMENTED
	static const std::string pATTENUATOR;
	/// \brief NOT IMPLEMENTED
	static const std::string pTHINKNESS;
	/// \brief NOT IMPLEMENTED
	static const std::string pCOLLIMATION;

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
	void set_num_neutrons(unsigned long int n);

  	/** \brief set the number of neutrons starting from the acquisition time and assuming a FLUX for the
	 * source of 1.2e8
	 */
	void set_measurement_time(double time);

	/** \brief sets the instrument
	 * \details Implemented instruments:
	 * #instrument_t
	 */
	void set_instrument(instrument_t instr = D22);

	/** \brief add simulation parameter
	 * \param[in] stage : it defines to which stage of the simulation the parameter belongs. It
	 * can be:
	 *    - panosc::#sFULL
	 *    - panosc::#sSAMPLE
	 *    - panosc::#sDETECTOR
	 * \param[in] name : name of the parameter, it should match the name in McStas
	 * \param[in] value : the value of the parameter, only float is implemented
	 */
	void add_parameter(stage_t stage, std::string name, double value);

	/** \brief add simulation parameter
	 * \param[in] stage : it defines to which stage of the simulation the parameter belongs. It
	 * can be:
	 *    - panosc::#sFULL
	 *    - panosc::#sSAMPLE
	 *    - panosc::#sDETECTOR
	 * \param[in] name : name of the parameter, it should match the name in McStas
	 * \param[in] vec : parameter is a vector of floats
	 */
	void add_parameter_array(stage_t stage, std::string name, std::vector<double> &vec);

	/** \brief request results
	 *  \param[in] iret : what to return as defined by #returnType.
	 */
	void set_return_data(returnType iret = rNONE);

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

	/// \brief pretty print of the request in json format, for debug purposes
	friend std::ostream &operator<<(std::ostream &os, const panosc::sim_request &s);

	void read_json(std::ifstream &jsonfile);

	protected:
	nlohmann::json _j;
	instrument_t   _instrument;

	private:
	static const double FLUX; ///< \brief D22 source flux
};

} // namespace panosc
#endif
