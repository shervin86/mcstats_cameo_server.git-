#include "sim_request.hh"

namespace panosc_sim_server
{
sim_request::sim_request(std::string message)
{
	_j = nlohmann::json::parse(message);
	check_json();
	_instrument = _j["instrument"]["name"];
}

void sim_request::set_num_neutrons(unsigned long int n) { _j["--ncount"] = n; }

std::string sim_request::to_cameo(void) const { return _j.dump(); }

void sim_request::set_instrument(instrument_t instr)
{
	switch (instr) {
	case D22:
		_instrument              = "D22";
		_j["instrument"]["name"] = _instrument;
		_j["source"]             = nlohmann::json::object();
		_j["detector"]           = nlohmann::json::object();
		_j["sample"]             = nlohmann::json::object();
		_j["mcpl"]               = nlohmann::json::object();
		break;
	}
}

void sim_request::add_parameter(size_t stage, std::string name, double value)
{
	switch (stage) {
	case sFULL:
		_j["source"][name] = value;
		break;
	case sDETECTOR:
		_j["detector"][name] = value;
		break;
	case sSAMPLE:
		_j["sample"][name] = value;
		break;
	}
	return;
}

void sim_request::set_return_data(returnType iret)
{
	switch (iret) {
	case rNONE:
		_j["return"] = "NONE";
		break;
	case rCOUNTS:
		_j["return"] = "COUNTS";
		break;
	case rERRORS:
		_j["return"] = "ERRORS";
		break;
	case rNEUTRONS:
		_j["return"] = "NEUTRONS";
		break;
	case rALL:
		_j["return"] = "ALL";
		break;
	case rFULL:
		_j["return"] = "FULL";
		break;
	}
}

sim_request::returnType sim_request::get_return_data(void) const
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

std::vector<std::string> sim_request::args(void) const
{
	std::vector<std::string> args;

	for (const auto &i : _j.items()) {
		if (i.value().type() == nlohmann::json::value_t::object)
			continue;
		if (i.key() == "return")
			continue;

		std::stringstream s;
		s << i.key() << "=" << i.value();
		args.push_back(s.str());
	}

	for (const auto &i : _j["source"].items()) {
		std::stringstream s;
		s << i.key() << "=" << i.value();
		args.push_back(s.str());
	}

	for (const auto &i : _j["sample"].items()) {
		std::stringstream s;
		s << i.key() << "=" << i.value();
		args.push_back(s.str());
	}

	for (const auto &i : _j["detector"].items()) {
		std::stringstream s;
		s << i.key() << "=" << i.value();
		args.push_back(s.str());
	}

	return args;
}

std::ostream &operator<<(std::ostream &os, const panosc_sim_server::sim_request &s)
{
	os << std::setw(4) << s._j;
	return os;
}

} // namespace panosc_sim_server
