#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "cameo_mcstas_client.hh"
#include "sim_request_server.hh"
#include "sim_result_server.hh"

#include <cameo/api/cameo.h>
#include <filesystem>
namespace fs = std::filesystem;

// make sure that the message is translated correctly
TEST_CASE("Request server")
{
	std::ifstream       fin("client_api/request.json");
	panosc::sim_request request(fin);
	//	request.read_json(fin);

	panosc::sim_request_server server_req(request.to_cameo());
	CHECK(request.to_cameo() == server_req.to_cameo()); // make sure that the constructor works
}

// check that the sim_result_server class is correctly reading the McStas output file
// TEST_CASE("Server Result") {

// 	// find the file with the counts on the detector
// 	fs::path p;
// 	for (auto &p_itr : fs::directory_iterator("mcstas")) {
// 		std::string s   = p_itr.path().stem();
// 		auto        pos = s.rfind('_');
// 		if (pos != std::string::npos)
// 			s.erase(pos);
// 		if (s == "D22_Detector")
// 			p = p_itr.path();
// 	}
// 	std::cout << p << std::endl;
// 	std::ifstream fi(p);

// 	panosc::sim_result_server sim_result;
// 	sim_result.read_file(fi);

// 	//sim_result.set_status(0);
// 	CHECK(sim_result.dim_x() == 128);
// 	CHECK(sim_result.dim_y() == 256);
// 	CHECK(sim_result.data().size() == 32768);
// }

TEST_CASE("sim_result_server Test")
{
	panosc::sim_result_server sim_result;
	sim_result.set_test(panosc::sim_result::ansDONE);
	std::string cameo_message = sim_result.to_cameo();

	panosc::sim_result result(cameo_message);
	CHECK(result.answer_message() == "DEF");
	CHECK(result.dim_x() == 16);
	CHECK(result.dim_y() == 16);
	std::vector<float> c = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	CHECK(result.data() == c);
	CHECK(result.processed_neutrons() == 0);
	CHECK(result.requested_neutrons() == 0);
	CHECK(result.get_status() == panosc::sim_result::ansDONE);
	CAPTURE(cameo_message);
	//	std::cout <<"Cameo message: "<< cameo_message << std::endl;
}

TEST_CASE("sim_result_server error")
{
	panosc::sim_result_server sim_result(panosc::sim_result::ansERROR);
	std::string               cameo_message = sim_result.to_cameo();
	panosc::sim_result        result(cameo_message);
	std::cout << "Cameo message: " << cameo_message << std::endl;
}

TEST_CASE("Full server test from fakeNomad")
{
	std::string serverName = "mcstas_server";
	bool        useJSON    = false;
	cameo::application::This::init("testserver", "tcp://nourbakhshlnx:7323");
	cameo::application::State returnState = cameo::application::UNKNOWN;

	// New block to ensure cameo objects are terminated before the
	// application. needed because of zmq
	{

		cameo::application::This::setRunning();
		CHECK(cameo::application::This::isAvailable());
		// Get the local Cameo server.
		cameo::Server &server = cameo::application::This::getServer();
		CHECK(server.isAvailable());
		std::unique_ptr<cameo::application::Instance> responderServer = server.connect(serverName);
		if (responderServer->exists() == false) { // start it for me
			responderServer = server.start(serverName);
		}
		std::cout << "responder: " << *responderServer << "   --" << responderServer->exists()
		          << "--                 [" << cameo::application::toString(responderServer->now())
		          << "]" << std::endl;

		// Create a requester.
		std::unique_ptr<cameo::application::Requester> requester =
		    cameo::application::Requester::create(
		        *responderServer,
		        panosc::CAMEO_RESPONDER); // the name here has to be the
		                                  // same as on the server

		std::cout << "Requester: " << *requester << " ["
		          << "CREATED"
		          << "]" << std::endl;
		CHECK(requester.get() != nullptr);

		// Create a subscriber
		std::unique_ptr<cameo::application::Subscriber> subscriber =
		    cameo::application::Subscriber::create(*responderServer,
		                                           panosc::CAMEO_PUBLISHER); //
		std::cout << "Subscriber: " << *subscriber << " ["
		          << "CREATED"
		          << "]" << std::endl;
		CHECK(subscriber.get() != nullptr);

		SUBCASE("Sending standard request")
		{
			//! [request]
			panosc::sim_request request(panosc::sim_request::QUICK, panosc::D22);
			//! [request]
			std::ifstream jsonfile("request.json");
			if (useJSON)
				request.read_json(jsonfile);
			else {
				/// [request2]
				request.set_instrument(panosc::D22);
				//			request.set_num_neutrons(1000000);
				request.set_measurement_time(49);
				request.add_parameter(panosc::sim_request::pWAVELENGTH, 4.51);
				request.add_parameter(panosc::sim_request::pCOLLIMATION, 18.00);
				try {
					request.add_parameter(panosc::sim_request::pTHICKNESS, 2.00);
				} catch (const panosc::param_not_implemented &e) {
					std::cerr << e.what() << std::endl;
				}
				request.set_return_data(panosc::sim_request::rCOUNTS);

				//			request.set_sample_material(panosc::H2O);
				//			request.set_sample_size(0.005, 0.05);
				// request.set_sample_size(0.005);
				/// [request2]
			}
			jsonfile.close();
			std::cout << request << std::endl;

			// print the list of available parameters and values
			for (const auto &p : request.param_names) {
				std::cout << p.first << "\t" << std::boolalpha
				          << request.is_param_implemented(p.first) << "\t" << p.second.name
				          << "\t[" << p.second.min << ":" << p.second.max << "]\t"
				          << p.second.units << std::endl;
			}

			/// [send request]
			requester->sendBinary(request.to_cameo()); // request number 1
			/// [send request]
			// Wait for the response from the server.
			/// [receive result]
			std::optional<std::string> response = requester->receiveBinary();
			CHECK(response.has_value());
			panosc::sim_request_answer answer(response.value());
			std::cout << "Response to request: " << response.value() << std::endl;
			/// [receive result]
			/// [return state]
			if (answer.wait_pub()) {
				///[return state]
				panosc::sim_result::answer_t state   = panosc::sim_result::ansNOANS;
				bool                         waitPub = true;
				do {
					///[get data]
					std::optional<std::string> published_data =
					    subscriber->receiveBinary();
					CHECK(published_data.has_value());
					// std::cout << published_data.value() << std::endl;

					panosc::sim_result           result(published_data.value());
					panosc::sim_result::answer_t state = result.get_status();
					waitPub                            = result.wait_pub();
					std::cout << "[DEBUG] state = " << state << "\t" << std::endl;
					std::cout << result.dim_x() << "\t" << result.dim_y() << std::endl;
					const std::vector<float> &data = result.data();
					///[get data]
					for (auto d = data.begin();
					     d != data.end() && (d - data.begin()) < 10; d++) {
						std::cout << "Data: " << *d << std::endl;
					}

				} while (waitPub);
			} else {
				std::cerr << "[ERROR] The answer to the request is: " << answer.answer()

				          << std::endl;
			}
		}
		SUBCASE("request::CLEAR")
		{
			panosc::sim_request req(panosc::sim_request::CLEAR, panosc::D22);
			std::cout << req.to_cameo() << std::endl;
			requester->sendBinary(req.to_cameo());
			std::optional<std::string> response = requester->receiveBinary();
			CHECK(response.has_value());
			panosc::sim_request_answer answer(response.value());
			std::cout << "Response to request: " << response.value() << std::endl;
			CHECK(answer.wait_pub() == false);
		}
	}
}



TEST_CASE("Test image transformation from McStas to Nomad")
{
	std::ifstream fin("test/D22_Detector_0000000000.x_y");
	panosc::sim_result_server sim_result;
	sim_result.read_file(fin);
 	CHECK(sim_result.dim_x() == 128);
 	CHECK(sim_result.dim_y() == 256);
	CHECK(sim_result.data().size() == 32768);
	
	for(auto i : sim_result.data())
		CHECK(i != -999);
	CHECK(sim_result.data()[0] ==	doctest::Approx(256.001).epsilon(0.0001) );
	CHECK(sim_result.data()[32767] ==	doctest::Approx(1.128).epsilon(0.0001) );


}
// TEST_CASE("Server Result") {

// 	// find the file with the counts on the detector
// 	fs::path p;
// 	for (auto &p_itr : fs::directory_iterator("mcstas")) {
// 		std::string s   = p_itr.path().stem();
// 		auto        pos = s.rfind('_');
// 		if (pos != std::string::npos)
// 			s.erase(pos);
// 		if (s == "D22_Detector")
// 			p = p_itr.path();
// 	}
// 	std::cout << p << std::endl;
// 	std::ifstream fi(p);

// 	panosc::sim_result_server sim_result;
// 	sim_result.read_file(fi);

// 	//sim_result.set_status(0);
// 	CHECK(sim_result.dim_x() == 128);
// 	CHECK(sim_result.dim_y() == 256);
// 	CHECK(sim_result.data().size() == 32768);
// }
