#include "sim_request.hh"
#include "sim_result.hh"
#include <cameo/cameo.h>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "c++/7/experimental/filesystem"
namespace fs = std::experimental::filesystem;

static const std::string baseDir = "/dev/shm/NOMAD/";

/**
 * Writes a string to a file as binary
 * @param fileName : the name of the output file
 * @param fileContent : the string to write to  */
void writeFile(const std::string &fileName, const std::string &fileContent)
{
	std::ofstream file(fileName, std::ios::binary | std::ios::ate);
	if (file.is_open()) {
		file << fileContent;
		file.flush();
		file.close();
	} else {
		std::cerr << "File " << fileName << " cannot be written." << std::endl;
	}
}

enum exitCodes { exitOK = 0, exitNOCLIENT, exitNOSERVER, exitREQUESTER, exitFAILURE };

int main(int argc, char *argv[])
{
	exitCodes ret     = exitOK;
	bool      useJSON = false;
	std::cout << "\n============================== Start of fakeNomad " << std::endl;
	for (int i = 0; i < argc; ++i) {
		if (strcmp(argv[i], "useJSON") == 0 or strcmp(argv[i], "-J") == 0)
			useJSON = true;
		//		std::cout << "#" << argv[i] << "#\t" << useJSON << std::endl;
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
		std::unique_ptr<cameo::application::Instance> responderServer = server.connect(SERVERNAME);

		std::cout << "responder: " << *responderServer << "                    ["
		          << cameo::application::toString(responderServer->now()) << "]" << std::endl;

		// check that the server is running, otherwise wait till a given timeOut

		// Create a requester.
		std::unique_ptr<cameo::application::Requester> requester =
		    cameo::application::Requester::create(
		        *responderServer,
		        panosc::CAMEO_RESPONDER); // the name here has to be the same as on the
		                                  // server
		std::cout << "Requester: " << *requester << " ["
		          << "CREATED"
		          << "]" << std::endl;
		if (requester.get() == 0) {
			std::cerr << "[ERROR] requester error" << std::endl;
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
			// request.set_num_neutrons(10000000);
			request.set_measurement_time(1);
			request.add_parameter(panosc::sim_request::pWAVELENGTH, 4.51);
			request.add_parameter(panosc::sim_request::pCOLLIMATION, 18.00);
			try {
				request.add_parameter(panosc::sim_request::pTHICKNESS, 2.00);
			} catch (const panosc::param_not_implemented &e) {
				std::cerr << e.what() << std::endl;
			}
			request.set_return_data(panosc::sim_request::rNONE);
			/// [request2]
		}
		jsonfile.close();
		std::cout << request << std::endl;

		// std::string dirName = baseDir + request.instrument_name() + "/";
		// fs::path    p       = dirName;
		// fs::create_directories(p);
		// std::string hash_string = request.hash();
		// p /= hash_string;
		// p += ".tgz";
		if (true) {
			/// [send request]
			requester->sendBinary(request.to_cameo()); // request number 1
			/// [send request]
			// Wait for the response from the server.
			/// [receive result]
			std::string response;
			requester->receiveBinary(response);
			panosc::sim_result result(response);
			/// [receive result]
			/// [return state]
			returnState = result.get_status();
			if (cameo::application::SUCCESS == returnState) {
				///[return state]
				// if (response.size() > 1000)
				//	writeFile(p, response);
				///[get data]
				std::cout << result.dim_x() << "\t" << result.dim_y() << std::endl;
				const std::vector<float> &data = result.data();
				///[get data]
				for (auto d = data.begin(); d != data.end() && (d - data.begin()) < 10; d++) {
					std::cout << "Data: " << *d << std::endl;
				}
			} else {
				std::cerr
				    << "[ERROR] exit status is: " << cameo::application::toString(returnState)
				    << std::endl;
				ret = exitFAILURE;
				return ret;
			}
		} else {
			// std::cout << "[INFO] Result already present in " << p << std::endl;
			std::cout << "[INFO] Result already present in " << std::endl;
		}

		std::cout << "Sending two requests: testing the threads" << std::endl;
		// Create a requester.
		std::unique_ptr<std::thread> thread_A, thread_B, thread_C, thread_D;
		auto req_fun = [request, &responderServer](std::string thread_name, double measurement_time,
		                                           panosc::sim_request::req_t type =
		                                               panosc::sim_request::SIMULATE) {
			std::cout << "[THREAD " << thread_name << "] "
			          << "START" << std::endl;
			std::unique_ptr<cameo::application::Requester> requester_thread =
			    cameo::application::Requester::create(
			        *responderServer,
			        panosc::CAMEO_RESPONDER); // the name here has to be the same as on the
			panosc::sim_request req(type);    // = request;
			req.set_instrument(panosc::D22);
			req.add_parameter(panosc::sim_request::pWAVELENGTH, 4.51);
			req.add_parameter(panosc::sim_request::pCOLLIMATION, 18.00);
			req.set_return_data(panosc::sim_request::rCOUNTS);

			req.set_measurement_time(measurement_time);

			std::string response;
			requester_thread->sendBinary(req.to_cameo());
			std::cout << "[THREAD " << thread_name << "] "
			          << "sent" << std::endl;
			requester_thread->receiveBinary(response);
			std::cout << "[THREAD " << thread_name << "] " << response << std::endl;
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

		thread_B.reset(
		    new std::thread(req_fun, "STOP", 20, panosc::sim_request::STOP)); // request number 2
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
