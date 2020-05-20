#include "sim_request.hh"
#include "sim_result_detector.hh"
#include <cameo/cameo.h>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "c++/7/experimental/filesystem"
namespace fs = std::experimental::filesystem;

static const std::string baseDir = "/dev/shm/NOMAD/";

//#define SERVERNAME "mcstas_server"
//#define REQUESTER_RESPONDER_NAME "mcstas_responder"

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
		        REQUESTER_RESPONDER_NAME); // the name here has to be the same as on the
		                                   // server
		std::cout << "Requester: " << *requester << " ["
		          << "CREATED"
		          << "]" << std::endl;
		if (requester.get() == 0) {
			std::cerr << "[ERROR] requester error" << std::endl;
			return exitREQUESTER;
		}

		//! [request]
		panosc_sim_server::sim_request request;
		//! [request]
		std::ifstream jsonfile("request.json");
		if (useJSON)
			request.read_json(jsonfile);
		else {
			/// [request2]
			request.set_instrument(panosc_sim_server::sim_request::D22);
			request.set_num_neutrons(10000000);
			request.add_parameter(panosc_sim_server::sFULL, "lambda", 4.51);
			request.add_parameter(panosc_sim_server::sDETECTOR, "D22_collimation", 2.00);
			request.set_return_data(panosc_sim_server::sim_request::rNONE);
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
			requester->sendBinary(request.to_cameo());
			/// [send request]
			// Wait for the response from the server.
			/// [receive result]
			std::string response;
			requester->receiveBinary(response);
			panosc_sim_server::sim_result_detector result(response);
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
				std::cerr << "ERROR" << std::endl;
				ret = exitFAILURE;
			}
		} else {
			// std::cout << "[INFO] Result already present in " << p << std::endl;
			std::cout << "[INFO] Result already present in " << std::endl;
		}
		std::cout << "Finished the application" << std::endl;

	} // end of block to make sure zmq objects are closed properly

	return ret;
}
