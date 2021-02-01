#include "local_cache.hh"
#include "mongo_cache.hh"
#include "sim_request_server.hh"
#include "sim_result_server.hh"

#include <cameo/cameo.h>
#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include <filesystem>
namespace fs = std::filesystem;

/********************************/
/**
 * \brief name of the responder created by the server that can be
 * accessed by Nomad
 */

#define CHECK_REQUESTER_EXISTS                                                                               \
	std::cout << "Requester " << *(request->connectToRequester()) << " exists? " << std::boolalpha       \
	          << request->connectToRequester()->exists() << std::endl;

#define MAX_BUFFER 1000000
#define VERBOSE
#define DEBUG
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

/**
 * \brief finds the output file of the McStas simulation and returns the result as a sim_result_server object
 * \param[in] directory where the McStas output files are for the current simulation (provided by
 * local_cache::output_dir() )
 * \param[in] the status of the job, it is returned to the client
 * \param[in] if the result has to
 * be effectively included or if to set just the state
 *
 * WARNING: if the application was stopped, the number of effective neutrons are not saved in the file!
 */
panosc::sim_result_server send_result(const std::string output_dir, const cameo::application::State state,
                                      bool send_results = true)
{
	panosc::sim_result_server sim_result;
	sim_result.set_status(state);
	if ((state == cameo::application::SUCCESS or state == cameo::application::STOPPED) && send_results) {
		fs::path p = output_dir; // path of the entire mcstas ouput directory
		// find the file with the counts on the detector
		for (auto &p_itr : fs::directory_iterator(p.parent_path() / p.stem())) {
			//			std::cout << "--> " << p_itr.path().stem() << std::endl;
			std::string s   = p_itr.path().stem();
			auto        pos = s.rfind('_');
			if (pos != std::string::npos)
				s.erase(pos);
			if (s == "D22_Detector")
				p = p_itr.path();
			//			std::cout << "##> " << s << "\t" << p << std::endl;
		}
#ifdef VERBOSE
		std::cout << "File with detector image: " << p << std::endl;
#endif
		std::ifstream fi(p);
		sim_result.read_file(fi);
		// std::cout << sim_result.to_cameo() << std::endl;
	}
	return sim_result;
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
	bool        isDevel = false;
	std::string baseDir = "/dev/shm/";
	for (int i = 0; i < argc; ++i) {
		if (strcmp(argv[i], "--baseDir") == 0 or strcmp(argv[i], "-d") == 0)
			baseDir = argv[++i];
		else if (strcmp(argv[i], "--devel") == 0)
			isDevel = true;
#ifdef DEBUG
		std::cout << "#" << argv[i] << "#\t" << std::endl;
#endif
	}

	cameo::application::This::init(argc, argv);

	panosc::mongo_cache mc(isDevel);

	// New block to ensure cameo objects are terminated before the
	// application. needed because of zmq
	{

		cameo::application::This::setRunning();
		// Get the local Cameo server.
		cameo::Server &server = cameo::application::This::getServer();

		if (cameo::application::This::isAvailable() && server.isAvailable()) {
			std::cout << "Connected server " << server << std::endl;
		}

		// Define a responder for the requests
		std::unique_ptr<cameo::application::Responder> responder;
		try {
			responder = cameo::application::Responder::create(panosc::CAMEO_RESPONDER);
			std::cout << "Created responder " << *responder << std::endl;
		} catch (const cameo::ResponderCreationException &e) {
			std::cout << "Responder error" << std::endl;
			return -1;
		}

		// // Define a publisher for the results
		// std::unique_ptr<cameo::application::Publisher> publisher;
		// try {
		// 	publisher = cameo::application::Publisher::create(panosc::CAMEO_PUBLISHER, 1);
		// 	std::cout << "Created publisher " << *publisher << std::endl;
		// } catch (const cameo::PublisherCreationException &e) {
		// 	std::cout << "[ERROR] Publisher creation error" << std::endl;
		// 	return -1;
		// }

		std::unique_ptr<std::thread>       thread;
		std::map<std::string, std::string> running_simulations; // maps the hash to the app_name
		std::unique_ptr<cameo::application::Request>
		    request_thread; // this is the request being treated in the thread

		// Loop on the requests
		// accept only one request for now
		unsigned int request_count = 0;
		while (true) {
			std::cout << "\n------------------------------------------------------------\n"
			          << "[READY] waiting for new requests; "
			          << "currently running simulations: " << running_simulations.size()
			          << std::endl;

			// Receive the simple request.
			std::unique_ptr<cameo::application::Request> request = responder->receive();

			/////--------------- Process the request
			panosc::sim_request_server sim_request_obj(request->getBinary());
			++request_count;
			std::cout << "========== [REQ] ==========" << *request
#ifdef VERBOSE
			          << "\n"
			          << sim_request_obj << "\n"
			          << "Request hash: " << sim_request_obj.hash() << "\n"
			          << "==========================="
#endif
			          << std::endl;

			//--------------- Request for a communication test
			if (sim_request_obj.is_test()) {
				panosc::sim_result_server sim_result;
				sim_result.set_test(cameo::application::SUCCESS);
				std::cout << "REQUEST FOR TEST!\n" << sim_result.to_cameo() << std::endl;

				CHECK_REQUESTER_EXISTS
				request->replyBinary(sim_result.to_cameo());
				continue;
			}

			//------------------------------ Request for stopping the current simulation
			if (sim_request_obj.type() == panosc::sim_request::STOP) {
				std::cout << "[REQUEST] Received stop request" << std::endl;
				std::unique_ptr<cameo::application::Instance> simulationApplication =
				    server.connect(running_simulations[sim_request_obj.hash()]);
				std::cout << "[STOPPING] " << *simulationApplication << "\t"
				          << simulationApplication->getLastState() << "\t"
				          << simulationApplication->getActualState() << std::endl;
				simulationApplication->stop();
				running_simulations.erase(sim_request_obj.hash());

				panosc::sim_result_server sim_result(cameo::application::STOPPING);
				CHECK_REQUESTER_EXISTS
				request->replyBinary(sim_result.to_cameo());
				continue;
			}

			//------------------------------ Request is SIMULATE

			//--------------- Simulation already running
			//----- Same request
			if (running_simulations.count(sim_request_obj.hash())) {
				std::cout << "[REQUEST] Received again same request as the one running: "
				          << "thread is active? " << std::boolalpha
				          << (thread.get() != nullptr) << std::endl;
				panosc::sim_result_server sim_result(cameo::application::RUNNING);
				CHECK_REQUESTER_EXISTS
				request->replyBinary(sim_result.to_cameo());
				continue;
			}
			//----- Another request but one already running
			if (running_simulations.size() > 0) {
				std::cout << "There is already one simulation running, cannot accept more"
				          << std::endl;

				panosc::sim_result_server sim_result(cameo::application::PROCESSING_ERROR);
				CHECK_REQUESTER_EXISTS
				request->replyBinary(sim_result.to_cameo());
				continue;
			}

			//--------------- New request and no other simulation running
			mc.set_request(sim_request_obj);

			// define a temp dir in RAM
			panosc::local_cache lc(sim_request_obj.instrument_name(), sim_request_obj.hash(),
			                       baseDir);

			if (!lc.isOK()) { // check if the simulation has already run and tgz
				          // available
				std::cout << "[WARNING] Previous simulation run, but TGZ not created: either "
				             "it has been stopped or there was an error\n"
				          << "   removing the directory and restarting the simulation"
				          << std::endl;
				fs::remove_all(lc.output_dir());
				// panosc::sim_result_server sim_result(cameo::application::FAILURE);
				// CHECK_REQUESTER_EXISTS
				// request->replyBinary(sim_result.to_cameo());
				// continue; // get ready to process new request
			}

			// save the result in the MongoDB if not yet there
			bool is_done = mc.is_done();
			if (is_done == false and lc.is_done() == true) {
				mc.save_request();
			}

			if (lc.is_done()) { // in this case re-use the previous results

				std::cout << "simulation exists, re-using the same results" << std::endl;
				auto sim_result = send_result(lc.output_dir(), cameo::application::SUCCESS,
				                              sim_request_obj.get_return_data() ==
				                                  panosc::sim_request::rCOUNTS);

				CHECK_REQUESTER_EXISTS
				request->replyBinary(sim_result.to_cameo());

			} else { // in this case we need to re-run the simulation

				// in the current implementation, this is not needed unless a stop request has
				// been received. In that case the number of running simulations is zero, but
				// the thread might be still running for a bit
				if (thread.get() != nullptr) { // wait that any previous simulation has ended
					thread->join();
				}
				std::cout << "[STATUS] Running simulations: " << running_simulations.size()
				          << std::endl;
				// get the hash for all the stages
				std::vector<std::string> hashes = sim_request_obj.stage_hashes();

				// returns the stage corresponding to a previous simulation with same
				// parameters and the eventual mcpl filename
				// stages are ordered from the detector to the source
				auto  stage         = lc.get_stage(hashes);
				auto &istage        = stage.first;
				auto &mcpl_filename = stage.second;

#ifdef DEBUG
				std::cout << "------------------------------ Stages\n"
				          << stage.first << "\t" << stage.second << "\n"
				          << std::endl;
				for (auto i : hashes) {
					std::cout << i << std::endl;
				}
#endif
				//--------------------- build list of arguments for the simulation program
				std::vector<std::string> args = sim_request_obj.args();
				args.push_back("--dir=" + (lc.output_dir()).string());

				args.push_back("stage=" + std::to_string(istage));
				if (!mcpl_filename.empty())
					args.push_back("Vin_filename=" + mcpl_filename);

				// the app_name is the one defined in the cameo.xml file
				std::string app_name =
				    sim_request_obj.instrument_name() + "-" + panosc::stages.at(istage);

#ifdef DEBUG
				std::cout << "[DEBUG] APP: #" << app_name << "#" << std::endl;
				for (auto singlearg : args)
					std::cout << "[DEBUG] arg: #" << singlearg << "#" << std::endl;
#endif

				std::unique_ptr<cameo::application::Instance> simulationApplication =
				    server.start(app_name, args);
				running_simulations[sim_request_obj.hash()] =
				    simulationApplication->getName(); // getNameId(); // app_name;

				std::cout << "------------------------------------------------------------\n"
				             "| Started simulation application "
				          << simulationApplication->getNameId() << "\t"
				          << *simulationApplication << "\n| with arguments:\n";
				for (auto singlearg : args)
					std::cout << "| >>>>#" << singlearg << "#\n";

				std::cout << "------------------------------------------------------------"
				          << std::endl;

				// by reference objects outside the while(true) loop
				// the rest by value
				// if the request is not put a in more global scope, it can be deleted at the
				// end of the while(true) loop but before the thread starts, so that the reply
				// in the thread cannot happen
				request_thread = std::move(request);
				std::cout << "[BEFORE THREAD]: " << request_count << std::endl;
				thread.reset(new std::thread([&server, app_name, lc, istage, sim_request_obj,
				                              &running_simulations, &request_thread]() {
					std::unique_ptr<cameo::application::Instance> running_sim =
					    server.connect(app_name);

					cameo::application::State state_thread = running_sim->waitFor();

					std::cout
					    << "[THREAD] Finished the simulation application with state "
					    << cameo::application::toString(state_thread) << std::endl;

					if (state_thread == cameo::application::SUCCESS) {

						lc.save_request(sim_request_obj.to_string());
						if (istage == panosc::sFULL) {
							lc.save_stage(
							    panosc::sSAMPLE,
							    sim_request_obj.hash(
							        panosc::sSAMPLE)); ///\todo to be fixed
						}
						lc.save_tgz();
						// mc.save_request();
					}
					// std::cout << "before send result" << std::endl;
					auto sim_result = send_result(lc.output_dir(), state_thread,
					                              sim_request_obj.get_return_data() ==
					                                  panosc::sim_request::rCOUNTS);
					std::cout << "[THREAD] after send result" << std::endl;
					// I want to make sure that the client has not died... otherwise the
					// server would crash
					// std::unique_ptr<cameo::application::Instance> requester =
					//    request_thread->connectToRequester();
					//					std::cout << "after connect to
					// requester" << std::endl;
					// if (requester.get() == nullptr) {
					// 	std::cout << "[THREAD ERROR] Instance does not exist"
					// 	          << std::endl;
					// } else {
					// 	std::cout << "[THREAD OK]" << std::endl;
					//}
					// std::cout << "before requester exists check" << std::endl;
					// if (requester->exists())
					std::cout << "[THREAD] Requester exists? " << std::boolalpha
					          << *(request_thread->connectToRequester()) << "\t"
					          << request_thread->connectToRequester()->exists()
					          << std::endl;

					request_thread->replyBinary(sim_result.to_cameo());

					running_simulations.erase(
					    sim_request_obj
					        .hash()); // remove it from the list of running simulations
					std::cout << "[THREAD] END" << std::endl;
				}));

				//				panosc::sim_result_server  sim_result;
				//				sim_result.set_status(cameo::application::RUNNING);
				//				request->replyBinary(sim_result.to_cameo());
			}
		}
		if (thread.get() != nullptr) { // wait that any previous simulation has ended
			thread->join();
		}

		std::cout << "Finished the application" << std::endl;

	} // end of block to make sure zmq objects are closed properly
	return 0;
}
