#include "cameo_mcstas_client.hh"
#include <cameo/api/cameo.h>
#include <fstream>
#include <iomanip>
#include <iostream>

//#include "c++/7/experimental/filesystem"
// namespace fs = std::experimental::filesystem;
#include <filesystem>
namespace fs = std::filesystem;

enum exitCodes { exitOK = 0, exitNOCLIENT, exitNOSERVER, exitREQUESTER, exitFAILURE };

int main(int argc, char *argv[])
{
    exitCodes   ret        = exitOK;
    bool        useJSON    = false;
    std::string serverName = SERVERNAME;
    std::cout << "\n============================== Start of fakeNomad " << std::endl;
    for (int i = 0; i < argc; ++i) {
	if (strcmp(argv[i], "useJSON") == 0 or strcmp(argv[i], "-J") == 0)
	    useJSON = true;
	//		std::cout << "#" << argv[i] << "#\t" << useJSON << std::endl;
	if (strcmp(argv[i], "serverName") == 0 or strcmp(argv[i], "-s") == 0)
	    serverName = argv[++i];
    }
    cameo::application::This::init(argc, argv);
    cameo::application::State returnState = cameo::application::UNKNOWN;

    // New block to ensure cameo objects are terminated before the
    // application. needed because of zmq
    {

	cameo::application::This::setRunning();
	if (!cameo::application::This::isAvailable()) {
	    std::cout << "[ERROR] Application not available" << std::endl;
	    return exitNOCLIENT;
	}

	// Get the local Cameo server.
	cameo::Server &server = cameo::application::This::getServer();
	if (!server.isAvailable()) {
	    std::cout << "[ERROR] CAMEO server not connected, abort" << server << std::endl;
	    return exitNOSERVER;
	}

	// Connect to the mcstas_server: put the name of the cameo process as
	// indicated by the name in the config file
	std::unique_ptr<cameo::application::Instance> responderServer = server.connect(serverName);
	if (responderServer->exists() == false) { // start it for me
	    responderServer = server.start(serverName);
	}
	std::cout << "responder: " << *responderServer << "   --" << responderServer->exists()
	          << "--                 [" << cameo::application::toString(responderServer->now()) << "]"
	          << std::endl;

	// check that the server is running, otherwise wait till a given timeOut

	// Create a requester.
	std::unique_ptr<cameo::application::Requester> requester =
	    cameo::application::Requester::create(*responderServer,
	                                          panosc::CAMEO_RESPONDER); // the name here has to be the
	                                                                    // same as on the server
	std::cout << "Requester: " << *requester << " ["
	          << "CREATED"
	          << "]" << std::endl;
	if (requester.get() == 0) {
	    std::cerr << "[ERROR] requester error" << std::endl;
	    return exitREQUESTER;
	}

	// Create a subscriber
	std::unique_ptr<cameo::application::Subscriber> subscriber =
	    cameo::application::Subscriber::create(*responderServer,
	                                           panosc::CAMEO_PUBLISHER); //
	std::cout << "Subscriber: " << *subscriber << " ["
	          << "CREATED"
	          << "]" << std::endl;
	if (subscriber.get() == 0) {
	    std::cerr << "[ERROR] subscriber creation error" << std::endl;
	    return exitREQUESTER;
	}

	//! [request]
	panosc::sim_request request;
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
	for(const auto& p : request.param_names){
		std::cout << p.first << "\t" << std::boolalpha << request.is_param_implemented(p.first) << "\t" << p.second.name << "\t[" << p.second.min << ":" << p.second.max << "]\t" << p.second.units
			 << std::endl;
	}
	if (true) {
	    /// [send request]
	    requester->sendBinary(request.to_cameo()); // request number 1
	    /// [send request]
	    // Wait for the response from the server.
	    /// [receive result]
	    std::optional<std::string> response = requester->receiveBinary();
	    if (response.has_value() == false)
		throw std::runtime_error("no message received");
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
		    std::optional<std::string> published_data = subscriber->receiveBinary();
		    if (published_data.has_value() == false)
			throw std::runtime_error("no output from publisher");
		    // std::cout << published_data.value() << std::endl;

		    panosc::sim_result           result(published_data.value());
		    panosc::sim_result::answer_t state = result.get_status();
		    waitPub                            = result.wait_pub();
		    std::cout << "[DEBUG] state = " << state << "\t" << std::endl;
		    std::cout << result.dim_x() << "\t" << result.dim_y() << std::endl;
		    const std::vector<float> &data = result.data();
		    ///[get data]
		    for (auto d = data.begin(); d != data.end() && (d - data.begin()) < 10; d++) {
			std::cout << "Data: " << *d << std::endl;
		    }

		} while (waitPub);
	    } else {
		std::cerr << "[ERROR] The answer to the request is: " << answer.answer()

		          << std::endl;
		ret = exitFAILURE;
		return ret;
	    }
	} else {
	    // std::cout << "[INFO] Result already present in " << p << std::endl;
	    std::cout << "[INFO] Result already present in " << std::endl;
	}

	responderServer->stop();
	return 0;
	std::cout << "Sending two requests: testing the threads" << std::endl;
	// Create a requester.
	std::unique_ptr<std::thread> thread_A, thread_B, thread_C, thread_D;
	auto                         req_fun = [request,
                        &responderServer](std::string thread_name, double measurement_time,
                                          panosc::sim_request::req_t type = panosc::sim_request::SIMULATE) {
            std::cout << "[THREAD " << thread_name << "] "
                      << "START" << std::endl;
            std::unique_ptr<cameo::application::Requester> requester_thread =
                cameo::application::Requester::create(
                    *responderServer,
                    panosc::CAMEO_RESPONDER); // the name here has to be the same as on the
            panosc::sim_request req(type); // = request;
            req.set_instrument(panosc::D22);
            req.add_parameter(panosc::sim_request::pWAVELENGTH, 4.51);
            req.add_parameter(panosc::sim_request::pCOLLIMATION, 18.00);
            req.set_return_data(panosc::sim_request::rCOUNTS);

            req.set_measurement_time(measurement_time);

            requester_thread->sendBinary(req.to_cameo());
            std::cout << "[THREAD " << thread_name << "] "
                      << "sent" << std::endl;
            std::optional<std::string> response = requester_thread->receiveBinary();
            std::cout << "[THREAD " << thread_name << "] " << response.value() << std::endl;
	};

	thread_A.reset(new std::thread(req_fun, "A", 5)); // request number 2
	thread_B.reset(new std::thread(req_fun, "B", 5)); // request number 3

	// thread_A->join();
	// thread_B->join();

	thread_C.reset(new std::thread(req_fun, "C", 6)); // request number 4
	thread_D.reset(new std::thread(req_fun, "D", 7)); // request number 5

	if (thread_A.get() != nullptr)
	    thread_A->join();
	if (thread_B.get() != nullptr)
	    thread_B->join();

	thread_C->join();
	thread_D->join();

	// trying to stop an ongoing simulation

	thread_A.reset(new std::thread(req_fun, "E", 20)); // request number 2

	thread_B.reset(new std::thread(req_fun, "STOP", 20, panosc::sim_request::STOP)); // request number 2
	if (thread_A.get() != nullptr)
	    thread_A->join();
	if (thread_B.get() != nullptr)
	    thread_B->join();

	// std::unique_ptr<cameo::application::Requester> requester_bis =
	//     cameo::application::Requester::create(
	//         *responderServer,
	//         panosc::CAMEO_RESPONDER); // the name here has to be the same as on the

	// std::cout << "Old request: \n" << request << "\n";
	// request.set_measurement_time(5); // in seconds
	// std::cout << "New request: \n" << request << std::endl;
	// std::cout << "Requester: " << *requester << "\n"
	//           << "Requester_bis: " << *requester_bis << std::endl;

	// requester->sendBinary(request.to_cameo());
	// requester_bis->sendBinary(request.to_cameo());
	// // Wait for the response from the server.
	// std::string response;
	// requester->receiveBinary(response);
	// // std::cout << "[REQUESTER] " << response << std::endl;

	// requester_bis->receiveBinary(response);
	// std::cout << "[REQUESTER_BIS] " << response << std::endl;
	std::cout << "Finished the application" << std::endl;

    } // end of block to make sure zmq objects are closed properly

    return ret;
}
