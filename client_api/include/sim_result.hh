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
	int                       get_status(void) const { return _status; };

	protected:
	sim_result(void){};
	static const size_t DIM_X = 128;
	static const size_t DIM_Y = 128;
	float               _dim_x;
	float               _dim_y;
	std::vector<float>  _counts, errors, n;
	int                 _status;
};
} // namespace panosc
#endif
