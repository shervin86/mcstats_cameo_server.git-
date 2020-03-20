#include "sim_request.hh"
#include <cameo/cameo.h>
#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
//#include <stdio.h>
//#include <functional> // for std::hash
#include <string>

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

		// std::hash<std::string> h_str;  // this should return the hash of the request

		// Loop on the requests
		// accept only one request for now

		while (true) {
			// initialize
			cameo::application::State state = cameo::application::UNKNOWN;

			std::cout << "\n\n"
			          << "READY: waiting for new requests" << std::endl;
			// Receive the simple request.
			std::unique_ptr<cameo::application::Request> request = responder->receive();
			sim_request sim_request_obj(request->getBinary());
			//#ifdef DEBUG
			std::cout << "========== [REQ] ==========\n"
			          << sim_request_obj << "\n"
			          << "Request hash: " << sim_request_obj.hash()
			          << "\n===========================" << std::endl;
			//#endif

			// define a temp dir in RAM
			std::string dirName =
			    std::string("/dev/shm/") + sim_request_obj.instrument_name() + "/";
			fs::path p = dirName;
			fs::create_directories(p);
			std::string hash_string = sim_request_obj.hash();
			p /= hash_string;
			p += ".tgz";

			if (!fs::exists(p) && (fs::exists(p.parent_path() / p.stem()))) {
				std::cerr << "[ERROR] Sim dir already exists but not TGZ, "
				             "please clean up"
				          << std::endl;
				state = cameo::application::FAILURE;

				request->replyBinary(cameo::application::toString(state));
				continue; // get ready to process new request
			}

			if (!fs::exists(p)) { // in this case we need to re-run the simulation
				// if there is a failure, something should be reported somehow
				std::cout << "[TAR] does not exists" << std::endl;
				std::vector<std::string> args = sim_request_obj.args();
				args.push_back("--dir=" + (p.parent_path() / p.stem()).string());

#ifdef DEBUG
				for (auto singlearg : args) {
					std::cout << "***" << singlearg << std::endl;
				}
				std::cout << "**#" << sim_request_obj.instrument_name()
				          << std::endl;
#endif

				auto start = clock();

				std::unique_ptr<cameo::application::Instance>
				    simulationApplication =
				        server.start(sim_request_obj.instrument_name(), args);
#ifdef DEBUG
				std::cout << "Started simulation application "
				          << *simulationApplication << std::endl;
#endif
				state = simulationApplication->waitFor();

				auto end = clock();
#ifdef DEBUG
				std::cout << "Finished the simulation application with state "
				          << cameo::application::toString(state) << std::endl;
				std::cout
				    << std::fixed << std::setprecision(3)
				    << "CPU time used: " << 1000.0 * (end - start) / CLOCKS_PER_SEC
				    << " ms" << std::endl;
#endif
			} else {
				// in this case re-use the previous results
				std::cout << "simulation exists, re-using the same results"
				          << std::endl;
				state = cameo::application::SUCCESS;
			}

			request->replyBinary(
			    std::to_string(state)); // cameo::application::toString(state));
			if (state == cameo::application::SUCCESS) {

				std::ofstream request_dump_file(
				    (p.parent_path() / p.stem()).string() + "/request.json");
				request_dump_file << sim_request_obj << std::endl;

				system((std::string("tar -cz -C ") + p.parent_path().string() +
				        " " + p.stem().string() + " > " + p.string())
				           .c_str());

				std::string fileContent;
				readFile(p, fileContent);
				request->replyBinary(fileContent);
			}
		}
		std::cout << "Finished the application" << std::endl;

	} // end of block to make sure zmq objects are closed properly
	return 0;
}
