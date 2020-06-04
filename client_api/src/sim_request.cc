#include "sim_request.hh"

#include <iomanip>
namespace panosc
{


// this struct is needed only in this library
struct param_data{
	stage_t stage;
	std::string name;
};

	//this map is only of internal use
	// define here the stage for each parameter, it should match the McStas instrument implementation
	// define here the name for each parameter, it should match the McStas instrument implementation
static const std::map<sim_request::param_t, param_data> param_names = {
	// clang-format off
	{sim_request::pWAVELENGTH				, {sFULL,		"lambda"		}},
	{sim_request::pSOURCE_SIZE_X			, {sFULL,		"source_size_x"	}},
	{sim_request::pSOURCE_SIZE_Y			, {sFULL,		"source_size_y"	}},
	{sim_request::pSAMPLE_SIZE_X			, {sSAMPLE,		"sample_size_x"	}},
	{sim_request::pSAMPLE_SIZE_Y			, {sSAMPLE,		"sample_size_y"	}},
	{sim_request::pDETECTOR_DISTANCE		, {sDETECTOR,	"det"			}},
	{sim_request::pBEAMSTOP_X				, {sDETECTOR,	"bs_x"			}},
	{sim_request::pBEAMSTOP_Y				, {sDETECTOR,	"bs_y"			}},
	{sim_request::pATTENUATOR				, {sDETECTOR,	"attenuator"	}},
	{sim_request::pTHINKNESS				, {sSAMPLE,		"thinkness"		}},
	{sim_request::pCOLLIMATION				, {sSAMPLE,		"D22_collimation"	}},
	//clang-format on
};

const double sim_request::FLUX = 1.2e7; ///< \brief D22 source flux 1.2e8

void sim_request::set_measurement_time(double time) { set_num_neutrons((unsigned long int)(time * FLUX)); }
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

void sim_request::add_parameter(param_t par	, double value) {
  const	auto& par_data = param_names.find(par)->second;
  const auto stage_name = stages.find(par_data.stage)->second;
	_j[stage_name][par_data.name] = value;
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

void sim_request::set_return_data(return_t iret)
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

std::ostream &operator<<(std::ostream &os, const panosc ::sim_request &s)
{
	os << std::setw(4) << s._j;
	return os;
}

} // namespace panosc
