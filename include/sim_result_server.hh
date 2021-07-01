#ifndef SIM_RESULT_SERVER_CLASS_HH
#define SIM_RESULT_SERVER_CLASS_HH
#include "sim_result.hh"
#include <fstream>
#include <vector>
namespace panosc
{
/**
 * \class sim_result_server
 * \brief code and decode simulation results (only detector image) between client and server
 * \author Shervin Nourbakhsh nourbakhsh@ill.fr
 */
class sim_result_server : public sim_result
{
	public:
	/** \brief constructor reading the mcstas detector file (server-side)
	 */
	sim_result_server(answer_t stat) { set_status(stat); };
	sim_result_server(){};

	void read_file(std::ifstream &f);

	void set_test(answer_t status)
	{
		_status = status;
		_dim_x  = 16;
		_dim_y  = 16;
		_counts = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		_errors = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
		_n      = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
	};
	/** \brief transform the result into a string to be sent through CAMEO (server side)
	 *
	 * \details
	 *
	 * Example use with cameo:
	 * \code{.cpp}
	 * sim_result detector();
	 * request->replyBinary(sim_result.to_cameo());
	 * \endcode
	 * \todo replace plain json with MessagePack or CBOR or something else
	 */
	std::string to_cameo(void) const;

	inline void set_status(answer_t s) { _status = s; };
};
} // namespace panosc
#endif
