#include "sim_request.hh"
#include "sim_result_detector.hh"
#include "local_cache.hh"

#include <cameo/cameo.h>
#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
//#include <stdio.h>
//#include <functional> // for std::hash
#include <string>
// class sim_result_detector{
// public:
// 	sim_result_detector(std::ifstream& fi){std::cout << fi<< std::endl;};
// 	std::string to_cameo(){std::string a; return a;};
// };

#include "c++/7/experimental/filesystem"
namespace fs = std::experimental::filesystem;

//static const std::string baseDir = "/dev/shm/";
/********************************/
/**
 * \brief name of the responder created by the server that can be
 * accessed by Nomad
 */

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
			cameo::application::State state  = cameo::application::UNKNOWN;
			size_t                    istage = 0;
			std::string               stage_hash;

			std::cout << "\n\n"
			          << "READY: waiting for new requests" << std::endl;
			// Receive the simple request.
			std::unique_ptr<cameo::application::Request> request = responder->receive();
			sim_request sim_request_obj(request->getBinary());

			std::cout << "========== [REQ] ==========\n"
			          << sim_request_obj << "\n"
			          << "Request hash: " << sim_request_obj.hash()
			          << "\n===========================" << std::endl;

			local_cache lc(sim_request_obj.instrument_name(), sim_request_obj.hash());
			// define a temp dir in RAM
			fs::path p = lc.path();

			if(! lc.isOK()){
				state = cameo::application::FAILURE;
				request->replyBinary(std::to_string(state));
				continue; // get ready to process new request
			}

			if(!lc.is_done()){ // in this case we need to re-run the simulation
				// if there is a failure, something should be reported somehow
				std::cout << "[TAR] does not exists" << std::endl;

				// here I could check what has changed and decide if/when/how to
				// re-use MCPL files
				/*
				  - sample
				  - detector
				  - source as well as instrument are bound.... so if any change,
				  re-run the entire simulation...
				 */

				std::string mcpl_filename;
				// check here if any MCPL exists for one of the stages
				// stages are ordered from the detector to the source
				for (istage = 0;
				     istage < stages.size() and mcpl_filename.empty();) {
					stage_hash = sim_request_obj.hash(istage);
					fs::path stage_path(p.parent_path() / "MCPL" /
					                    stages[istage] / stage_hash);
					stage_path += ".json";
					std::cout << "[INFO] check if MCPL file for stage "
					          << istage
					          << " exists\n       check existence of file"
					          << stage_path << std::endl;
					if (fs::exists(stage_path)) {
						mcpl_filename =
						    stage_path.parent_path() / stage_path.stem();
						mcpl_filename += ".mcpl.gz";
						std::cout << "    -> file found" << std::endl;
					} else
						++istage;
				}

				if (mcpl_filename.empty())
					istage = stages.size() - 1;
				// std::cout << istage << "\t" << stages[istage] << std::endl;
				//------------------------------ build list of arguments for the
				// simulation program
				std::vector<std::string> args = sim_request_obj.args();
				args.push_back("--dir=" + (p.parent_path() / p.stem()).string());

				if (!mcpl_filename.empty())
					args.push_back("Vin_filename=" + mcpl_filename);
				std::string app_name =
				    sim_request_obj.instrument_name() + "-" + stages[istage];
#ifdef DEBUG

				std::cout << "[DEBUG] APP: #" << app_name << "#" << std::endl;
				for (auto singlearg : args)
					std::cout << "[DEBUG] arg: #" << singlearg << "#"
					          << std::endl;
#endif

				// start the sim application
				auto start = clock();
				std::unique_ptr<cameo::application::Instance>
				    simulationApplication = server.start(app_name, args);

				std::cout << "Started simulation application "
				          << *simulationApplication << std::endl;

				state = simulationApplication->waitFor();

				auto end = clock();

				std::cout << "Finished the simulation application with state "
				          << cameo::application::toString(state) << std::endl;
				std::cout
				    << std::fixed << std::setprecision(3)
				    << "CPU time used: " << 1000.0 * (end - start) / CLOCKS_PER_SEC
				    << " ms" << std::endl;

			} else {
				// in this case re-use the previous results
				std::cout << "simulation exists, re-using the same results"
				          << std::endl;
				state = cameo::application::SUCCESS;
			}

			sim_result_detector result;
			result.set_status(std::to_string(state));
			
			// send back the exit status to the client
			request->replyBinary(std::to_string(state));

			if (state == cameo::application::SUCCESS) {

				lc.save_request(sim_request_obj.to_string());
				if (istage == sim_request::sFULL) {
					lc.save_stage(1, sim_request_obj.hash(1));
				}
				lc.save_tgz();
				for (auto &p_itr :
						 fs::directory_iterator(p.parent_path() / p.stem())) {
					std::cout << "--> " << p_itr.path().stem() << std::endl;
					std::string s   = p_itr.path().stem();
					auto        pos = s.rfind('_');
					if (pos != std::string::npos)
						s.erase(pos);
					if (s == "D22_Detector")
						p = p_itr.path();
					std::cout << "##> " << s << "\t" << p << std::endl;
				}
				std::ifstream fi(p);
				sim_result_detector sim_result;
				sim_result.read_file(fi);
				std::cout << sim_result.to_cameo() << std::endl;
				// p.parent_path()/
				std::string fileContent;
				readFile(p, fileContent);
				// request->replyBinary(fileContent);
				request->replyBinary(sim_result.to_cameo());
			}
		}
		std::cout << "Finished the application" << std::endl;

	} // end of block to make sure zmq objects are closed properly
	return 0;
}
