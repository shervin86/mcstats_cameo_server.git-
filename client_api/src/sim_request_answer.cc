#include "sim_request_answer.hh"

namespace panosc
{
sim_request_answer::sim_request_answer(std::string message)
{
	nlohmann::json j = nlohmann::json::parse(message);
	_answer          = j["answer"];
	_answer_message  = j["answer_message"];
}

std::string sim_request_answer::answer_message(void) const { return _answer_message; }
} // namespace panosc
