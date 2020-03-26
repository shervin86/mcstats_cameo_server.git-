#include "sim_request.hh"
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
	exitCodes ret = exitOK;
	std::cout << "\n============================== Start of fakeNomad " << std::endl;

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
			std::cout << "[ERROR] CAMEO server not connected, abort" << server
			          << std::endl;
			return exitNOSERVER;
		}

		// Connect to the mcstas_server: put the name of the cameo process as
		// indicated by the name in the config file
		std::unique_ptr<cameo::application::Instance> responderServer =
		    server.connect(SERVERNAME);

		std::cout << "responder: " << *responderServer << "                    ["
		          << cameo::application::toString(responderServer->now()) << "]"
		          << std::endl;

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

		// creating the request
		std::ifstream jsonfile("request.json");
		// nlohmann::json j = {{"instrument", "D22"}, {"--ncount", 1000000},
		// {"lambda", 4.5}};
		sim_request request(jsonfile);
		std::string dirName = baseDir + request.instrument_name() + "/";
		fs::path    p       = dirName;
		fs::create_directories(p);
		std::string hash_string = request.hash();
		p /= hash_string;
		p += ".tgz";
		if (!fs::exists(p)) {

			requester->sendBinary(request.to_string());
			// Wait for the response from the server.
			std::string response;
			requester->receiveBinary(response);
			std::cout << "RESP=" << response << std::endl;
			returnState = std::stoul(response);

			if (cameo::application::SUCCESS == returnState) {
				requester->receiveBinary(response);
				if (response.size() > 1000)
					writeFile(p, response);
			} else {
				std::cerr << "ERROR" << std::endl;
				ret = exitFAILURE;
			}
		} else {
			std::cout << "[INFO] Result already present in " << p << std::endl;
		}
		std::cout << "Finished the application" << std::endl;

	} // end of block to make sure zmq objects are closed properly

	return ret;
}
