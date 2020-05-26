#ifndef SIM_RESULT_SERVER_CLASS_HH
#define SIM_RESULT_SERVER_CLASS_HH
#include "sim_result.hh"
#include <fstream>
#include <vector>
namespace panosc_sim_server
{
/**
 * \class sim_result_detector
 * \brief code and decode simulation results (only detector image) between client and server
 * \author Shervin Nourbakhsh nourbakhsh@ill.fr
 * \ingroup clientAPI
 */
class sim_result_server : public sim_result
{
	public:
	/** \brief constructor reading the mcstas detector file (server-side)
	 */
	sim_result_server(void){};

	void read_file(std::ifstream &f);

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

	inline int get_status(void) { return _status; };

	inline void set_status(int s) { _status = s; };
};
} // namespace panosc_sim_server
#endif
