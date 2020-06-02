#include "sim_request.hh"

#include <iomanip>
namespace panosc_sim_server
{

void sim_request::set_num_neutrons(unsigned long int n) { _j["--ncount"] = n; }

std::string sim_request::to_cameo(void) const { return _j.dump(); }

void sim_request::set_instrument(instrument_t instr)
{
	switch (instr) {
	case D22:
		_instrument              = instr;
		_j["instrument"]["name"] = instr;
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

void sim_request::add_parameter_array(stage_t stage, std::string name, std::vector<double> &vec)
{
	switch (stage) {
	case sFULL:
		_j["source"][name] = vec;
		break;
	case sDETECTOR:
		_j["detector"][name] = vec;
		break;
	case sSAMPLE:
		_j["sample"][name] = vec;
		break;
	}
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

void sim_request::read_json(std::ifstream &jsonfile)
{
	_j = nlohmann::json::parse(jsonfile);
	// assert(check_json()); ///\todo use exception
	_instrument = _j["instrument"]["name"].get<instrument_t>();
}

std::ostream &operator<<(std::ostream &os, const panosc_sim_server::sim_request &s)
{
	os << std::setw(4) << s._j;
	return os;
}

} // namespace panosc_sim_server
