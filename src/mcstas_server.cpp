#include "sim_request.hh"
#include <cameo/cameo.h>
#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
//#include <stdio.h>
#include <string>
#include <functional>   // for std::hash

#include "c++/7/experimental/filesystem"
namespace fs = std::experimental::filesystem;

/********************************/
/**
 * \brief name of the responder created by the server that can be
 * accessed by Nomad
 */
#define REQUESTER_RESPONDER_NAME "mcstas_responder"

#define MAX_BUFFER 1000000

/**
 * Reads the file to a string as binary
 * @param fileName      : the name of the input file
 * @param fileContent   : the string to read the content in
 */
void readFile(const std::string &fileName, std::string &fileContent)
{
	std::streampos size;
	std::ifstream  file(fileName, std::ios::binary | std::ios::ate);
	if (file.is_open()) {
		size = file.tellg();       // Reserve the string.
		assert(size < MAX_BUFFER); // if this fails, smarter chuck reading and sending
		                           // should be implemented
		fileContent.resize(size);
		//
		// Get the allocated array.
		char *array = (char *)fileContent.data();
		file.seekg(0, std::ios::beg);
		file.read(array, size);
		file.close();
	} else {
		std::cerr << "File " << fileName << " cannot be read." << std::endl;
	}
}

/**********
 * \file mcstas_server.cpp
 * \brief server communicating with Nomad through CAMEO
 * \author Shervin Nourbakhsh nourbakhsh@ill.fr
 *
 * The server should be started by CAMEO as a deamon.
 * It answers to requests from NOMAD to run mcstas simulation for a given ILL
 * instrument with parameters provided by NOMAD
 */
int main(int argc, char *argv[])
{
	cameo::application::This::init(argc, argv);

	// New block to ensure cameo objects are terminated before the
	// application. needed because of zmq
	{

		cameo::application::This::setRunning();
		// Get the local Cameo server.
		cameo::Server &server = cameo::application::This::getServer();

		if (cameo::application::This::isAvailable() && server.isAvailable()) {
			std::cout << "Connected server " << server << std::endl;
		}

		// Define a responder.
		std::unique_ptr<cameo::application::Responder> responder;
		try {
			responder = cameo::application::Responder::create(REQUESTER_RESPONDER_NAME);
			std::cout << "Created responder " << *responder << std::endl;
		} catch (const cameo::ResponderCreationException &e) {
			std::cout << "Responder error" << std::endl;
			return -1;
		}

		std::hash<std::string> h_str;
		
		// Loop on the requests.
		while (true) {
			// Receive the simple request.
			std::unique_ptr<cameo::application::Request> request = responder->receive();
			sim_request sim_request_obj(request->getBinary());
			//hash h(sim_request_obj.to_string());
#ifdef DEBUG
			std::cout << "========== [REQ] ==========\n"
			          << sim_request_obj
			          << "\n===========================" << std::endl;
			std::cout << "#" << std::hex << h_str(sim_request_obj.to_string()) << std::endl;
			std::cout << "*" << h_str(sim_request_obj.to_string()) << std::endl;
#endif
			
			std::vector<std::string> args = sim_request_obj.args();

			// define a temp file in RAM
			std::string tmpFileName = std::to_string(h_str(sim_request_obj.to_string()));
			tmpFileName.replace(1, 3, "dev/shm/SIMD22/");
			// tmpFileName += "/";
			fs::create_directory("/dev/shm/SIMD22");
			//system("mkdir -p /dev/shm/SIMD22");
			args.push_back("--dir=" + tmpFileName);
			// args.push_back("--no-output-dir");

#ifdef DEBUG
			for (auto singlearg : args) {
				std::cout << "***" << singlearg << std::endl;
			}
			std::cout << "**#" << sim_request_obj.instrument_name() << std::endl;
#endif
			
			auto start = clock();

			std::unique_ptr<cameo::application::Instance> simulationApplication =
			    server.start(sim_request_obj.instrument_name(), args);
			std::cout << "Started simulation application " << *simulationApplication
			          << std::endl;
			cameo::application::State state = simulationApplication->waitFor();
			auto                      end   = clock();

			std::cout << "Finished the simulation application with state "
			          << cameo::application::toString(state) << std::endl;
			std::cout << std::fixed << std::setprecision(3)
			          << "CPU time used: " << 1000.0 * (end - start) / CLOCKS_PER_SEC
			          << " ms" << std::endl;

			if (state == cameo::application::SUCCESS) {

				std::string fileContent;
				system((std::string("tar -czf ") + tmpFileName + ".tgz " +
				        tmpFileName + "/")
				           .c_str());
				readFile(tmpFileName + ".tgz", fileContent);
				request->replyBinary(fileContent);
				// request->replyBinary("Simulation terminated");
				request->replyBinary("OK");
				//			remove(tmpFileName.c_str());
			}
		}
		std::cout << "Finished the application" << std::endl;

	} // end of block to make sure zmq objects are closed properly
	return 0;
}
