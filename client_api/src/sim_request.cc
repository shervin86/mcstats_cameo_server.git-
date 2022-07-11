#include "sim_request.hh"
#include <cmath> // for nan
#include <iomanip>
#include <iostream>
#include <sstream>
namespace panosc
{

// this map is only of internal use
// define here the stage for each parameter, it should match the McStas instrument implementation
// define here the name for each parameter, it should match the McStas instrument implementation
const std::map<sim_request::param_t, sim_request::param_data> sim_request::param_names = {
    // clang-format off
	{sim_request::pWAVELENGTH     	, {sFULL  , "lambda"          , 3,  6                                                                             , "angs" }},
	{sim_request::pSOURCE_SIZE_X  	, {sNONE  , "source_size_x"   , std::numeric_limits<float>::quiet_NaN(),  std::numeric_limits<float>::quiet_NaN() , ""     }},
	{sim_request::pSOURCE_SIZE_Y  	, {sNONE  , "source_size_y"   , std::numeric_limits<float>::quiet_NaN(),  std::numeric_limits<float>::quiet_NaN() , ""     }},
	{sim_request::pSAMPLE_SIZE_R  	, {sSAMPLE  , "sample_size_r"   , 0,  std::numeric_limits<float>::quiet_NaN() , "[m]"     }},
	{sim_request::pSAMPLE_SIZE_X  	, {sNONE  , "sample_size_x"   , std::numeric_limits<float>::quiet_NaN(),  std::numeric_limits<float>::quiet_NaN() , ""     }},
	{sim_request::pSAMPLE_SIZE_Y  	, {sSAMPLE  , "sample_size_y"   , 0,  std::numeric_limits<float>::quiet_NaN() , ""     }},
	{sim_request::pSAMPLE_SIZE_Z  	, {sNONE  , "sample_size_z"   , std::numeric_limits<float>::quiet_NaN(),  std::numeric_limits<float>::quiet_NaN() , ""     }},
	{sim_request::pDETECTOR_DISTANCE, {sNONE  , "det"             , std::numeric_limits<float>::quiet_NaN(),  std::numeric_limits<float>::quiet_NaN() , ""     }},
	{sim_request::pBEAMSTOP_X       , {sNONE  , "bs_x"            , std::numeric_limits<float>::quiet_NaN(),  std::numeric_limits<float>::quiet_NaN() , ""     }},
	{sim_request::pBEAMSTOP_Y       , {sNONE  , "bs_y"            , std::numeric_limits<float>::quiet_NaN(),  std::numeric_limits<float>::quiet_NaN() , ""     }},
	{sim_request::pATTENUATOR       , {sNONE  , "attenuator"      , std::numeric_limits<float>::quiet_NaN(),  std::numeric_limits<float>::quiet_NaN() , ""     }},
	{sim_request::pTHICKNESS        , {sNONE  , "thickness"       , std::numeric_limits<float>::quiet_NaN(),  std::numeric_limits<float>::quiet_NaN() , ""     }},
	{sim_request::pCOLLIMATION      , {sSAMPLE, "D22_collimation" , 2                                      ,  18                                      , "m"    }},
	{sim_request::pA2               , {sFULL  , "a2"              , 0                                      , 360                                      , "degree"}},
	{sim_request::pA4               , {sFULL  , "a4"              , 0                                      , 360                                      , "degree"}},
	{sim_request::pA6               , {sFULL  , "a6"              , 0                                      , 360                                      , "degree"}},
	{sim_request::pA3               , {sNONE  , "a3"              , 0                                      , 360                                      , "degree"}},
	{sim_request::pNOTIMPLEMENTED   , {sNONE  , "not_implemented" , std::numeric_limits<float>::quiet_NaN(),  std::numeric_limits<float>::quiet_NaN() , ""     }},
    // clang-format on
};

// 1.2e8
const double sim_request::FLUX = 200000; // this is less than the 250 kHz to have some contingency

void sim_request::set_measurement_time(double time)
{
	set_num_neutrons((unsigned long long int)(time * FLUX));
} ///\todo add a check that the value does not exceed the unsigned long int
void sim_request::set_num_neutrons(unsigned long long int n) { _j["--ncount"] = std::to_string(n); }

std::string sim_request::to_cameo(void) const { return _j.dump(); }

void sim_request::set_instrument(instrument_t instr)
{
	switch (instr) {
	case D22:
		_instrument              = instr;
		_j["instrument"]["name"] = instr;
		for (auto istage : stages) {
			if (istage.first == sNONE)
				continue;
			_j[stages.at(istage.first)] = nlohmann::json::object();
		}
		_j["mcpl"] = nlohmann::json::object();
		break;
	case THALES:
		_instrument = instr;
		_j["instrument"]["name"] = instr;
		for( auto istage : stages){
			if(istage.first==sNONE)continue;
			_j[stages.at(istage.first)] = nlohmann::json::object();
		}
		_j["mcpl"] = nlohmann::json::object();
		break;
	}
}

void sim_request::add_parameter(param_t par, double value)
{
	const auto &par_data = param_names.at(par);
	if (par_data.stage == sNONE) {
		throw param_not_implemented((std::string("**** [WARNING] Parameter [") + par_data.name +
		                             std::string("] not implemented in McStas instrument ****"))
		                                .c_str());
		return;
	}
	const auto stage_name = stages.find(par_data.stage)->second;
	if (par == sim_request::pCOLLIMATION)
		value = 20 - value; /// \todo remove: best would be to change the instr file
	std::stringstream ss;
	ss << "Parameter [" << par_data.name << "] assigned a value that is out of admitted range: " << value
	   << " [" << par_data.min << ":" << par_data.max << "]";
	if (value < par_data.min or value > par_data.max) {
		throw std::out_of_range(ss.str().c_str());
	}
	_j[stage_name][par_data.name] = value;
}

void sim_request::add_parameter_array(param_t par, std::vector<double> &vec)
{
	const auto &par_data = param_names.at(par);
	if (par_data.stage == sNONE) {
		throw param_not_implemented((std::string("**** [WARNING] Parameter [") + par_data.name +
		                             std::string("] not implemented in McStas instrument ****"))
		                                .c_str());
		return;
	}
	const auto stage_name         = stages.at(par_data.stage);
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

void sim_request::set_sample_material(sample_material_t material) { _j["sSAMPLE"]["material"] = material; }

void sim_request::set_sample_size(double radius)
{
	add_parameter(sim_request::pSAMPLE_SIZE_R, radius);
	add_parameter(sim_request::pSAMPLE_SIZE_Y, 0);
}

void sim_request::set_sample_size(double radius, double height)
{
	add_parameter(sim_request::pSAMPLE_SIZE_R, radius);
	add_parameter(sim_request::pSAMPLE_SIZE_Y, height);
}

void sim_request::set_sample_size(double x, double y, double z)
{
	add_parameter(sim_request::pSAMPLE_SIZE_X, x);
	add_parameter(sim_request::pSAMPLE_SIZE_Y, y);
	add_parameter(sim_request::pSAMPLE_SIZE_Z, z);
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
