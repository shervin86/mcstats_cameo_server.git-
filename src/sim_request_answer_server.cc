#include "sim_request_answer_server.hh"

namespace panosc
{

	std::string sim_request_answer_server::to_cameo(sim_request_answer::answer_t answer) 
{
	nlohmann::json j;
	j["answer"]   = answer;
	j["answer_message"] = "";
	return j.dump();
}
} // namespace panosc
