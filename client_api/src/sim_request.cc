#include "sim_request.hh"
#include <iostream>
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
	{sim_request::pSOURCE_SIZE_X			, {sNONE,	"source_size_x"	}},
	{sim_request::pSOURCE_SIZE_Y			, {sNONE,	"source_size_y"	}},
	{sim_request::pSAMPLE_SIZE_X			, {sNONE,	"sample_size_x"	}},
	{sim_request::pSAMPLE_SIZE_Y			, {sNONE,	"sample_size_y"	}},
	{sim_request::pDETECTOR_DISTANCE		, {sNONE,	"det"			}},
	{sim_request::pBEAMSTOP_X				, {sNONE,	"bs_x"			}},
	{sim_request::pBEAMSTOP_Y				, {sNONE,	"bs_y"			}},
	{sim_request::pATTENUATOR				, {sNONE,	"attenuator"	}},
	{sim_request::pTHICKNESS				, {sNONE,	"thickness"		}},
	{sim_request::pCOLLIMATION				, {sSAMPLE,		"D22_collimation"	}},
    // clang-format on
};

  //1.2e8
const double sim_request::FLUX = 1.2e7; ///< Flux of the source assumed for the measurement time to number of neutrons conversion
  

void sim_request::set_measurement_time(double time) { set_num_neutrons((unsigned long int)(time * FLUX)); }
void sim_request::set_num_neutrons(unsigned long int n) { _j["--ncount"] = n; }

std::string sim_request::to_cameo(void) const { return _j.dump(); }

void sim_request::set_instrument(instrument_t instr)
{
	switch (instr) {
	case D22:
		_instrument              = instr;
		_j["instrument"]["name"] = instr;
		for (stage_t istage = sSOURCE; istage <= sFULL; istage++) {
			_j[stages.at(istage)] = nlohmann::json::object();
		}
		_j["mcpl"] = nlohmann::json::object();
		break;
	}
}


void sim_request::add_parameter(param_t par	, double value) {
  const	auto& par_data = param_names.at(par);
  if(par_data.stage == sNONE){
	  throw param_not_implemented((std::string("**** [WARNING] Parameter [")+ par_data.name+std::string("] not implemented in McStas instrument ****")).c_str());
	  return;
  }
  const auto stage_name = stages.find(par_data.stage)->second;
	_j[stage_name][par_data.name] = value;
}

void sim_request::add_parameter_array(param_t par, std::vector<double> &vec)
{
	const	auto& par_data = param_names.at(par);
	if(par_data.stage == sNONE){
		throw param_not_implemented((std::string("**** [WARNING] Parameter [")+ par_data.name+std::string("] not implemented in McStas instrument ****")).c_str());
		return;
	}
	const auto stage_name = stages.at(par_data.stage);
	_j[stage_name][par_data.name] = vec;
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
