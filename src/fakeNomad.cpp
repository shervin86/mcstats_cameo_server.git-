#include "sim_request.hh"
#include <cameo/cameo.h>
#include <fstream>
#include <iomanip>
#include <iostream>

//#define DEBUG
#define SERVERNAME "mcstas_server"
#define REQUESTER_RESPONDER_NAME "mcstas_responder"
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

int main(int argc, char *argv[])
{
	std::cout << "============================== Start of fakeNomad " << std::endl;

	// get the request from json file

	cameo::application::This::init(argc, argv);
	cameo::application::State returnState = cameo::application::UNKNOWN;

	// New block to ensure cameo objects are terminated before the
	// application. needed because of zmq
	{

		cameo::application::This::setRunning();

		// Get the local Cameo server.
		cameo::Server &server = cameo::application::This::getServer();
		if (cameo::application::This::isAvailable() && server.isAvailable()) {
			std::cerr << "Connected server " << server << std::endl;
		}

		// Connect to the mcstas_server: put the name of the cameo process as
		// indicated by the name in the config file
		std::unique_ptr<cameo::application::Instance> responderServer =
		    server.connect(SERVERNAME);

#ifdef DEBUG
		std::cout << "Application " << *responderServer << " has state "
		          << cameo::application::toString(responderServer->now()) << std::endl;
#endif

		// check that the server is running, otherwise wait till a given timeOut

		// Create a requester.
		std::unique_ptr<cameo::application::Requester> requester =
		    cameo::application::Requester::create(
		        *responderServer,
		        REQUESTER_RESPONDER_NAME); // the name here has to be the same as on the
		                                   // server
		std::cout << "Created requester " << *requester << std::endl;
		if (requester.get() == 0) {
			std::cout << "requester error" << std::endl;
			return -1;
		}

		/** creating the request
		 */
		std::ifstream jsonfile("request.json");
		// nlohmann::json j = {{"instrument", "D22"}, {"--ncount", 1000000},
		// {"lambda", 4.5}};
		sim_request request(jsonfile);

		requester->sendBinary(request.to_string());
		// Wait for the response from the server.
		std::string response;
		requester->receiveBinary(response);
		std::cout << "RESP=" << response << std::endl;
		returnState = std::stoul(response);
		//		std::cout << std::boolalpha <<
		//(cameo::application::SUCCESS==returnState) << std::endl;
		if (cameo::application::SUCCESS == returnState) {
			requester->receiveBinary(response);
			if (response.size() > 1000)
				writeFile(request.hash() + ".tgz", response);
		} else {
			std::cerr << "ERROR" << std::endl;
		}
		// #ifdef DEBUG
		// //			std::cout << response << std::endl;
		// #endif
		// 		} while (response != "OK" and response != "DIE" and response !=
		// "ERROR");

		std::cout << "Finished the application" << std::endl;

	} // end of block to make sure zmq objects are closed properly

	return 0;
}
