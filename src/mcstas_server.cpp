#include <cameo/cameo.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <fstream>

#define REQUESTER_RESPONDER_NAME "mcstas_responder"
#define DEBUG

/**********
 * \file mcstas_server.cpp
 * \brief server communicating with Nomad through CAMEO
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
			/** Get the request content that is
			 * ? json file ?
			 * ? other     ?
			 */
			std::unique_ptr<cameo::application::Request> request = responder->receive();
			const std::string &requestText = request->getBinary();
#ifdef DEBUG
			std::cout << "========== [REQ] ==========\n"
			          << requestText << "\n===========================" << std::endl;
#endif

			request->replyBinary("==>\n" + requestText + "\n <== RECEIVED");

			// just check if the message is requiring a simulation
			std::cout << "Compare: [" << requestText.compare(0, 3, "SIM") << "]" << std::endl;
			if (requestText.compare(0, 3, "SIM") != 0) {
				std::cerr << "[ERROR] request not understood" << std::endl;
				return 1; // there is something wrong
			}

			/** parse from the request the list of arguments for the simulation executable
			 * the command line parameters should be understood by the simulation program
			 */
			std::vector<std::string> args; 			
			std::stringstream s(requestText);
			s.ignore(1000, '\n'); // skip the line SIM D22
			while (s.good()) {    // read all the SIM parameters
				std::string singlearg;
				getline(s,singlearg);
				if(!singlearg.empty())
					args.push_back(singlearg);
				request->replyBinary(singlearg);
			}

			// define a temp file in RAM
			std::string tmpFileName = tmpnam(nullptr);
			tmpFileName.replace(1,3,"dev/shm/SIMD22/");
			tmpFileName+="/";
			args.push_back("--dir="+tmpFileName);

#ifdef DEBUG
			for (auto singlearg : args) {
				std::cout << singlearg << std::endl;
			}
#endif
			auto start = clock();
			std::unique_ptr<cameo::application::Instance> simulationApplication =
			    server.start("SIMD22", args);
			std::cout << "Started simulation application " << *simulationApplication << std::endl;
			cameo::application::State state = simulationApplication->waitFor();
			auto end = clock();
			
			std::cout << "Finished the simulation application with state "
			          << cameo::application::toString(state) << std::endl;
			std::cout << std::fixed << std::setprecision(3)
			          << "CPU time used: " << 1000.0 * (end - start) / CLOCKS_PER_SEC << " ms" << std::endl;
			
			std::string outFileContent;
			// readFile(tmpFileName, outFileContent);
			// request->replyBinary("Simulation terminated");
			request->replyBinary("OK");
			//			remove(tmpFileName.c_str());

		}
		std::cout << "Finished the application" << std::endl;

	} // end of block to make sure zmq objects are closed properly
	return 0;
}
