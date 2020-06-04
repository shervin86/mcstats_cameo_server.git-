#include "sim_request_server.hh"

namespace panosc
{

sim_request::return_t sim_request_server::get_return_data(void) const
{
	if ((!_j.contains("return")) or _j["return"] == "NONE")
		return rNONE;
	if (_j["return"] == "COUNTS")
		return rCOUNTS;
	if (_j["return"] == "ERRORS")
		return rERRORS;
	if (_j["return"] == "NEUTRONS")
		return rNEUTRONS;
	if (_j["return"] == "FULL")
		return rFULL;
	else
		assert(false);
}

std::vector<std::string> sim_request_server::args(void) const
{
	std::vector<std::string> argss;

	for (const auto &i : _j.items()) {
		if (i.value().type() == nlohmann::json::value_t::object)
			continue;
		if (i.key() == "return")
			continue;

		std::stringstream s;
		s << i.key() << "=" << i.value();
		argss.push_back(s.str());
	}

	for(stage_t istage = sSOURCE; istage <= sFULL; ++istage){
		for (const auto &i : _j[stages.at(istage)].items()) {
			std::stringstream s;
			s << i.key() << "=" << i.value();
			argss.push_back(s.str());
		}
	}

	return argss;
}
} // namespace panosc
