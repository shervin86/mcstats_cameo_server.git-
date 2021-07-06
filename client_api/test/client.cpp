#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "sim_request.hh"
#include "sim_result.hh"


TEST_CASE("Testing test request (zero neutrons)")
{
	panosc::sim_request request;
	request.set_instrument(panosc::D22);
	request.set_measurement_time(0);
	request.set_return_data(panosc::sim_request::rNONE);
	//	std::cout << request.to_cameo() << std::endl;

	//	CHECK(request.is_test() == true);

	CHECK(request.to_cameo() ==
	      "{\"--ncount\":\"0\",\"instrument\":{\"name\":\"D22\"},\"mcpl\":{},\"return\":\"NONE\","
	      "\"sDETECTOR\":{},\"sFULL\":{},\"sSAMPLE\":{},\"sSOURCE\":{},\"type\":0}");

	// panosc::sim_result result;
	// result.set_test(0);
	// CHECK(result.to_cameo() == "{\"data\":[0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0."
	//                            "0],\"dim_x\":16,\"dim_y\":16,\"status\":64}");
}

TEST_CASE("Testing json equivalence")
{
	panosc::sim_request request;
	request.set_instrument(panosc::D22);
	// request.set_num_neutrons(10000000);
	request.set_measurement_time(1);

	// block of implemented parameters
	try {
		request.add_parameter(panosc::sim_request::pWAVELENGTH, 4.51);
		request.add_parameter(panosc::sim_request::pCOLLIMATION, 18.00);
	} catch (const panosc::param_not_implemented &e) {
		CHECK(false);
	}

	// test exception throwing for not implemented parameters
	try {
		request.add_parameter(panosc::sim_request::pNOTIMPLEMENTED, 18.00);
	} catch (const panosc::param_not_implemented &e) {
		std::cerr << e.what() << std::endl;
		CHECK(true);
	} catch (...) {
		CHECK(false);
	}
	request.set_return_data(panosc::sim_request::rNONE);

	// print out the request to check if anything changed
	std::ofstream fout("testclient.json");
	fout << request << std::endl;

	std::ifstream       fin("./request.json");
	panosc::sim_request req_json;
	req_json.read_json(fin);

	// test the the json is unchanged

	WARN(req_json.to_cameo() == request.to_cameo());
}


TEST_CASE("testing sim_result"){

    // empty message
	
//	CHECK_THROWS_AS(panosc::sim_result(""), "[SIM_RESULT] Received empty result message", std::runtime_error);
	CHECK_THROWS_AS(panosc::sim_result(""),  std::runtime_error);
	CHECK_THROWS(panosc::sim_result("{}"));
}

