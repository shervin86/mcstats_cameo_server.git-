#include "sim_result.hh"
namespace panosc_sim_server
{
	sim_result::sim_result(std::string message)
	{
		nlohmann::json j = nlohmann::json::parse(message);
		_dim_x           = j["dim_x"];
		_dim_y           = j["dim_y"];
		_status          = j["status"];
		//_counts          =
		(j["data"]).get_to(_counts);
	}
}
