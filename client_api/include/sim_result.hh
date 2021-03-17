#ifndef SIM_RESULT_CLASS_HH
#define SIM_RESULT_CLASS_HH
#include "nlohmann/json.hpp"
#include <vector>

namespace panosc
{

/**
 * \class sim_result
 * \brief code and decode simulation results (only detector image) between client and server
 * \author Shervin Nourbakhsh nourbakhsh@ill.fr
 * \ingroup clientAPI
 */
class sim_result
{
	public:
	/** \brief constructor that decodes the message from CAMEO (client side)
	 *  \param[in] message : request in string form received by the server
	 * \todo use the empty constructor and make it a parse(message) method to delete the message string
	 */
	sim_result(std::string message);

	/// dimension x of the detector pixel array
	size_t dim_x(void) const { return _dim_x; };
	/// dimension y of the detector pixel array
	size_t dim_y(void) const { return _dim_y; };
	/// return the linearized vector of values
	const std::vector<float> &data() const { return _counts; };

	inline unsigned long long int processed_neutrons(void) const { return _processed_neutrons; }
	inline unsigned long long int requested_neutrons(void) const { return _requested_neutrons; }
	inline int                    get_status(void) const { return _status; }

	protected:
	sim_result(void){};
	size_t                 _dim_x = 0;
	size_t                 _dim_y = 0;
	std::vector<float>     _counts, _errors, _n;    // empty vectors
	int                    _status             = 0; // 0 = UNKNOWN in CAMEO
	unsigned long long int _processed_neutrons = 0, _requested_neutrons = 0;
};
} // namespace panosc
#endif
