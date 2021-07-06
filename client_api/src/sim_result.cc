#include "sim_result.hh"

namespace panosc
{
/** throw std::runtime_error if the *message* string is empty */
sim_result::sim_result(std::string message)
{
	if (message.empty()) {
		throw std::runtime_error("[SIM_RESULT] Received empty result message");
		_dim_x          = 0;
		_dim_y          = 0;
		_status         = ansERROR;
		_answer_message = "[ERROR] Received empty result message";
		return;
	}
	nlohmann::json j = nlohmann::json::parse(message);
	_dim_x           = j["dim_x"];
	_dim_y           = j["dim_y"];
	_status          = j["status"];
	_answer_message  = j["answer_message"];
	//_counts          =
	(j["data"]).get_to(_counts);
}
} // namespace panosc
