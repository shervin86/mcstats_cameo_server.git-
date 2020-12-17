#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "sim_request_server.hh"
#include "sim_result_server.hh"
//#include "c++/7/experimental/filesystem"
//namespace fs = std::experimental::filesystem;
#include <filesystem>
namespace fs = std::filesystem;

// make sure that the message is translated correctly
TEST_CASE("Request server")
{
	std::ifstream       fin("client_api/request.json");
	panosc::sim_request request;
	request.read_json(fin);

	panosc::sim_request_server server_req(request.to_cameo());
	CHECK(request.to_cameo() == server_req.to_cameo()); // make sure that the constructor works
}

// check that the sim_result_server class is correctly reading the McStas output file
TEST_CASE("Server Result") {


	// find the file with the counts on the detector
	fs::path p;
	for (auto &p_itr : fs::directory_iterator("mcstas")) {
		std::string s   = p_itr.path().stem();
		auto        pos = s.rfind('_');
		if (pos != std::string::npos)
			s.erase(pos);
		if (s == "D22_Detector")
			p = p_itr.path();
	}
	std::cout << p << std::endl;
	std::ifstream fi(p);

	panosc::sim_result_server sim_result;
	sim_result.read_file(fi);

	sim_result.set_status(0);
	CHECK(sim_result.dim_x() == 128);
	CHECK(sim_result.dim_y() == 256);
	CHECK(sim_result.data().size() == 32768);
}


	
