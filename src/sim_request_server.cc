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
	return rNONE;
}

std::vector<std::string> sim_request_server::args(void) const
{
	std::vector<std::string> argss;

	for (const auto &i : _j.items()) {
		if (i.value().type() == nlohmann::json::value_t::object)
			continue;
		if (i.key() == "return")
			continue;
		if (i.key() == "type")
			continue;

		std::stringstream s;
		if (i.key() == "--ncount")
			s << i.key() << "=" << std::stoul(i.value().get<std::string>());
		else if (i.key() == "sample_size_y" and i.value() <= 0) continue;
		else
			s << i.key() << "=" << i.value();
		argss.push_back(s.str());
	}

	for (auto stage : stages) {
		auto istage    = stage.first;
		auto stagename = stage.second;
		if (istage == sNONE)
			continue;
		for (const auto &i : _j[stagename].items()) {
			std::stringstream s;
			if (istage == sSAMPLE and i.key() == "material") {
				switch (i.value().get<sample_material_t>()) {
				case H2O:
					s << "D22_sample=H2O_liq.qSq";
					break;
				case D2O:
					s << "D22_sample=D2O_liq.qSq";
					break;
				};
			} else {
				s << i.key() << "=" << i.value();
			}
			argss.push_back(s.str());
		}
	}

	return argss;
}


	std::vector<simHash_t> sim_request_server::stage_hashes(void) const
	{
		std::vector<simHash_t> hashes;
		for (auto stage : stages) {
			if (stage.first == sNONE)
				continue;
			hashes.push_back(hash(stage.first));
		}
		return hashes;
	}


	simHash_t sim_request_server::hash(stage_t s) const
	{
		nlohmann::json j(_j);
		j.erase("type");
		j.erase("--ncount");
		j.erase("return");

		switch (s) {
		case sFULL:
			break;
		case sDETECTOR:
			j.erase(stages.at(sDETECTOR));
			break;
		case sSAMPLE:
			j.erase(stages.at(sDETECTOR));
			j.erase(stages.at(sSAMPLE));
			break;
		}
#ifdef DEBUG
		std::cout << " + stage: " << s << "\thash: " << std::to_string(_hash(j)) << "\n\t" << j << std::endl;
#endif
		return std::to_string(_hash(j));
	}

} // namespace panosc
