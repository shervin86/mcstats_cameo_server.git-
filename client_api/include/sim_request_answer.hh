#ifndef SIM_REQUEST_ANSWER_CLASS_HH
#define SIM_REQUEST_ANSWER_CLASS_HH

#include "nlohmann/json.hpp"

namespace panosc
{

/**
 * \class sim_request_answer
 * \brief answer from the server to the panosc::sim_request received
 * \author Shervin Nourbakhsh nourbakhsh@ill.fr
 * \ingroup clientAPI
 */
class sim_request_answer
{
	public:
	/** \brief The responder fills the same request with the answer
	 */
	enum answer_t {
		ansRUNNING = 0, ///< a simulation for this request is already running
		ansDONE,        ///< a simulation for this request was already processed and finished
		ansSTARTING,    ///< this is a new request, start processing soon
		ansWAIT,  ///< another simulation is already running, cannot accept this request, try later
		ansOK,    ///< the request has been accepted and being processed, nothing will be published
		ansERROR, ///< an error occurred, nothing will be published
		ansNOANS  ///< there is no answer
	};

	sim_request_answer(std::string message);

	answer_t answer(void) const { return _answer; }
	bool     done(void) const { return _answer == ansDONE; }
	bool     wait_pub(void) const
	{
		return _answer == ansDONE or _answer == ansRUNNING or _answer == ansSTARTING;
	}
	bool ok(void) const { return _answer == ansOK; }
	bool error(void) const { return _answer == ansERROR or _answer == ansNOANS or _answer == ansWAIT; }

	std::string answer_message(void) const;

	private:
	answer_t    _answer;
	std::string _answer_message;
};
} // namespace panosc
#endif
