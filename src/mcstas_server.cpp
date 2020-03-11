#include "sim_request.hh"
#include <cameo/cameo.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string>
#include <cassert>

/**
 * \brief name of the responder created by the server that can be
 * accessed by Nomad
 */
#define REQUESTER_RESPONDER_NAME "mcstas_responder"

#define MAX_BUFFER 1000000
/**********
 * \file mcstas_server.cpp
 * \brief server communicating with Nomad through CAMEO
 * \author Shervin Nourbakhsh nourbakhsh@ill.fr
 *
 * The server should be started by CAMEO as a deamon.
 * It answers to requests from NOMAD to run mcstas simulation for a given ILL instrument with
 * parameters provided by NOMAD
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

		// Loop on the requests.
		while (true) {
			// Receive the simple request.
			/* Get the request content that is
			 * ? json file ?
			 * ? other     ?
			 */
			std::unique_ptr<cameo::application::Request> request = responder->receive();
			sim_request sim_request_obj(request->getBinary());
#ifdef DEBUG
			if (sim_request_obj.good()) {
				std::cout << "========== [REQ] ==========\n"
				          << sim_request_obj
				          << "\n===========================" << std::endl;
			}
			request->replyBinary("==>\n" + sim_request_obj.to_string() +
			                     "\n <== RECEIVED");
#else
			request->replyBinary("RECEIVED");
#endif

			std::vector<std::string> args = sim_request_obj.args();

			// define a temp file in RAM
			std::string tmpFileName = tmpnam(nullptr);
			tmpFileName.replace(1, 3, "dev/shm/SIMD22/");
			// tmpFileName += "/";
			system("mkdir -p /dev/shm/SIMD22");
			args.push_back("--dir=" + tmpFileName);
			// args.push_back("--no-output-dir");

#ifdef DEBUG
			for (auto singlearg : args) {
				std::cout << singlearg << std::endl;
			}
#endif

			{
				auto start = clock();

				std::unique_ptr<cameo::application::Instance>
				    simulationApplication =
				        server.start(sim_request_obj.instrument(), args);
				std::cout << "Started simulation application "
				          << *simulationApplication << std::endl;
				cameo::application::State state = simulationApplication->waitFor();
				auto                      end   = clock();

				std::cout << "Finished the simulation application with state "
				          << cameo::application::toString(state) << std::endl;
				std::cout
				    << std::fixed << std::setprecision(3)
				    << "CPU time used: " << 1000.0 * (end - start) / CLOCKS_PER_SEC
				    << " ms" << std::endl;
			}

			{
				
				
				system((std::string("tar -czf ")+tmpFileName+".tgz "+tmpFileName+"/").c_str());
				std::ifstream f(tmpFileName+".tgz", std::ifstream::binary);
				f.seekg (0, f.end);
				int length = f.tellg();
				f.seekg (0, f.beg);
				char buffer[MAX_BUFFER];
				int toread = std::min(length, MAX_BUFFER);
				assert(length < MAX_BUFFER);
				f.read(buffer,toread);
				request->replyBinary(std::string(buffer, toread));
				// readFile(tmpFileName, outFileContent);
				// request->replyBinary("Simulation terminated");
				request->replyBinary("OK");
				//			remove(tmpFileName.c_str());
			}
		}
		std::cout << "Finished the application" << std::endl;
		
	} // end of block to make sure zmq objects are closed properly
	return 0;
}
