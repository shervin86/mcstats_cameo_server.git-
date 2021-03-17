#include "local_cache.hh"
#ifdef MONGO
#include "mongo_cache.hh"
#endif
#include "sim_request_answer_server.hh"
#include "sim_request_server.hh"
#include "sim_result_server.hh"

#include <cameo/api/cameo.h>
#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include <filesystem>
namespace fs = std::filesystem;
typedef  cameo::application::State state_t;

/********************************/
/**
 * \brief name of the responder created by the server that can be
 * accessed by Nomad
 */

//#define VERBOSE
//#define DEBUG

unsigned long long int NEUTRONS_PER_JOB  = 5000000; // 1e7 neutrons -> 280MB MCPL file
size_t                 MAX_PARALLEL_JOBS = 2;

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
panosc::sim_result_server read_result(const std::string output_dir, const state_t state)
{
    panosc::sim_result_server sim_result(state);
    
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
    // use fixed seeds for the simulations
    const std::vector<unsigned long long int> seeds = {
0xfe22, 0x59c7, 0x4925, 0x5b51, 0x37fe, 0x1521, 0x4938, 0xe26c,
0x03aa, 0x97f8, 0x31ea, 0xa7c4, 0x2f84, 0x4009, 0xf15c, 0xac02,
0x4259, 0x53cb, 0x109f, 0xe393, 0x7261, 0xe7c7, 0x33bf, 0x5583,
0x7397, 0xffad, 0x8768, 0xd3e8, 0x0b13, 0x112c, 0xd81a, 0xaecb,
0x59fe, 0x579d, 0x2813, 0x4e02, 0x6ba3, 0x3d07, 0x6e1f, 0x04d4,
0x3bfc, 0xb7eb, 0xc2e9, 0xa149, 0xc788, 0x2e57, 0x1851, 0xcb92,
0x7fee, 0x4095, 0x00c2, 0x08fc, 0x2ed9, 0xae32, 0x70cc, 0x753a,
0x7433, 0x9946, 0xdca3, 0xb399, 0x7874, 0xc64f, 0x9006, 0x373f,
0x7f05, 0xe922, 0x0980, 0x670c, 0x0016, 0xeba0, 0x4a05, 0x600d,
0x63bf, 0xfc26, 0xaadc, 0x7d3c, 0x52be, 0xd2b1, 0xe4ea, 0xb482,
0x8dba, 0x261a, 0x9571, 0xf7f6, 0x715b, 0x8456, 0x07e2, 0x7572,
0x8e23, 0xbb14, 0xac5b, 0xd23a, 0xee55, 0x5f1f, 0x6ee8, 0x0df3,
0x4f7d, 0x0ccd, 0xe6ba, 0xaab4, 0x2c63, 0xf9a5, 0x0fe0, 0xe050,
0x4179, 0xa106, 0xe1e6, 0x1342, 0xbd24, 0xbb1c, 0xf36d, 0xbf38,
0xfa0e, 0xa0a0, 0x8b71, 0xe00b, 0x0600, 0xd1a0, 0xf685, 0x3e99,
0xc834, 0x8277, 0x3bd8, 0xaf9e, 0xc2a2, 0x5b8c, 0x62ec, 0x1cf0,
0x97ee, 0xaf52, 0xcbfc, 0x6bd9, 0x8b17, 0xcc50, 0x4497, 0x386b,
0xdcf6, 0x0759, 0x0d2c, 0x6f7c, 0x4029, 0x5e89, 0xe2b7, 0x5b29,
0xbb3c, 0xa112, 0x1f0c, 0xc54d, 0xec40, 0x4829, 0xa896, 0x96e2
	};

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
#ifndef MONGO
    std::cout << isDevel << std::endl;
#endif
    cameo::application::This::init(argc, argv);
#ifdef MONGO
    panosc::mongo_cache mc(isDevel);
#endif
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

	// Define a publisher for the results
	std::unique_ptr<cameo::application::Publisher> publisher;

	try {
	    publisher = cameo::application::Publisher::create(panosc::CAMEO_PUBLISHER);
	    std::cout << "Created publisher " << *publisher << std::endl;
	} catch (const cameo::PublisherCreationException &e) {
	    std::cout << "Error in creation of publisher" << std::endl;
	    return -1;
	}

	std::unique_ptr<std::thread> thread;
	typedef struct {
	    state_t                                       state;
	    std::unique_ptr<cameo::application::Instance> instance;
	} sim_job_t;

	std::map<panosc::simHash_t, std::vector<sim_job_t>> running_simulations; // maps the hash to the list of running instances and their state

	std::unique_ptr<cameo::application::Request> request_th; // this is the request being treated in the thread

	// Loop on the requests
	// accept only one request for now
	while (true) {
	    std::cout << "\n------------------------------------------------------------\n"
	              << "[READY] waiting for new requests; "
	              << "currently running simulations: " << running_simulations.size() << std::endl;

	    // Receive the simple request.
	    std::unique_ptr<cameo::application::Request> request = responder->receive();

	    /////--------------- Process the request
	    panosc::sim_request_server sim_request_obj(request->getBinary());
	    std::cout << "========== [REQ] ==========" << *request
#ifdef VERBOSE
	              << "\n"
	              << sim_request_obj << "\n"
	              << "Request hash: " << sim_request_obj.hash() << "\n"
	              << "==========================="
#endif
	              << std::endl;

	    //--------------- Request not recognized
	    if (sim_request_obj.type() >= panosc::sim_request::REQUNKNOWN) {
		std::cout << "[ERROR] UNKNOWN REQUEST\n" << std::endl;
		request->replyBinary(panosc::sim_request_answer_server::to_cameo(panosc::sim_request_answer::ansERROR));

		///\todo add here the second part of the reply with a human readable message
		continue;
	    }
	    //--------------- Request for a communication test
	    if (sim_request_obj.is_test()) {
		panosc::sim_result_server sim_result;
		sim_result.set_test(cameo::application::SUCCESS);
		std::cout << "REQUEST FOR TEST!\n" << sim_result.to_cameo() << std::endl;

		request->replyBinary(panosc::sim_request_answer_server::to_cameo(panosc::sim_request_answer::ansDONE));
		publisher->sendBinary(sim_result.to_cameo());
		continue;
	    }

	    //--------------- Request for a clearing the local cache
	    if (sim_request_obj.type() == panosc::sim_request::CLEAR) {
		panosc::local_cache lc(sim_request_obj.instrument_name(), sim_request_obj.hash(), baseDir + "QUICK/");

		std::cout << "CLEARING " << lc.output_dir() << " and removing " << fs::remove_all(lc.output_dir()) << " files and directories" << std::endl;

		lc.clear_cache();

		panosc::local_cache lc_full(sim_request_obj.instrument_name(), sim_request_obj.hash(), baseDir + "FULL/");

		std::cout << "CLEARING " << lc_full.output_dir() << " and removing " << fs::remove_all(lc_full.output_dir()) << " files and directories" << std::endl;

		lc_full.clear_cache();
		std::cout << "CLEARING THE LOCAL CACHE!\n" << std::endl;

		request->replyBinary(panosc::sim_request_answer_server::to_cameo(panosc::sim_request_answer::ansOK));
		continue;
	    }

	    //------------------------------ Request for stopping the current simulation
	    if (sim_request_obj.type() == panosc::sim_request::STOP) {
		std::cout << "[REQUEST] Received stop request" << std::endl;
		for (size_t i = 0; i < running_simulations[sim_request_obj.hash()].size(); ++i) {

		    auto &simulationApplication = running_simulations[sim_request_obj.hash()][i].instance;
		    std::cout << "[STOPPING] " << *simulationApplication << "\t" << simulationApplication->getLastState() << "\t" << simulationApplication->getActualState() << std::endl;
		    simulationApplication->stop();
		}

		request->replyBinary(panosc::sim_request_answer_server::to_cameo(panosc::sim_request_answer::ansOK));
		///\todo add here the merge thread and the publishing of the result
		running_simulations.erase(sim_request_obj.hash());
		continue;
	    }

	    //------------------------------ Request is SIMULATE

	    //--------------- Simulation already running
	    //----- Same request
	    if (running_simulations.count(sim_request_obj.hash())) {
		std::cout << "[REQUEST] Received again same request as the one running: "
		          << "thread is active? " << std::boolalpha << (thread.get() != nullptr) << std::endl;
		request->replyBinary(panosc::sim_request_answer_server::to_cameo(panosc::sim_request_answer::ansRUNNING));
		continue;
	    }
	    //----- Another request but one already running
	    if (running_simulations.size() > 0) {
		std::cout << "There is already one simulation running, cannot accept more" << std::endl;

		request->replyBinary(panosc::sim_request_answer_server::to_cameo(panosc::sim_request_answer::ansWAIT));
		continue;
	    }

	    //--------------- New request and no other simulation running
#ifdef MONGO
	    mc.set_request(sim_request_obj);
#endif
	    // define a temp dir in RAM depending on the simulation type requested
	    panosc::local_cache lc(sim_request_obj.instrument_name(), sim_request_obj.hash(), baseDir + ((sim_request_obj.type() == panosc::sim_request::QUICK) ? "QUICK/" : "FULL/"));

	    if (!lc.isOK()) { // check if the simulation has already run and tgz
		// available
		std::cout << "[WARNING] Previous simulation run, but TGZ not created: either "
		             "it has been stopped or there was an error\n"
		          << "   removing the directory and restarting the simulation" << std::endl;
		fs::remove_all(lc.output_dir());
		// panosc::sim_result_server sim_result(cameo::application::FAILURE);
		// CHECK_REQUESTER_EXISTS
		// request->replyBinary(sim_result.to_cameo());
		// continue; // get ready to process new request
	    }

#ifdef MONGO
	    // save the result in the MongoDB if not yet there
	    bool is_done = mc.is_done();
	    if (is_done == false and lc.is_done() == true) {
		mc.save_request();
	    }
#endif
	    // now it is FORCED! FIXME
	    if (lc.is_done() && false) { // in this case re-use the previous results

		std::cout << "simulation exists, re-using the same results" << std::endl;
		auto sim_result = read_result(lc.output_dir(), cameo::application::SUCCESS
		                              //, sim_request_obj.get_return_data() == panosc::sim_request::rCOUNTS
		);

		request->replyBinary(panosc::sim_request_answer_server::to_cameo(panosc::sim_request_answer::ansDONE));
		publisher->sendBinary(sim_result.to_cameo());
	    } else {                                                  // in this case we need to re-run the simulation
		sim_request_obj.set_type(panosc::sim_request::QUICK); // forced to be QUICK FIXME

		// in the current implementation, this is not needed unless a stop request has
		// been received. In that case the number of running simulations is zero, but
		// the thread might be still running for a bit
		if (thread.get() != nullptr) { // wait that any previous simulation has ended
		    thread->join();
		}

		std::cout << "[STATUS] Running simulations: " << running_simulations.size() << std::endl;

		// set the number of jobs

		size_t nJobs = std::ceil(sim_request_obj.get_num_neutrons() / ((double)NEUTRONS_PER_JOB));

		std::cout << "[INFO] Splitting in " << nJobs << " jobs" << std::endl;
		// get the hash for all the stages
		std::vector<std::string> hashes = sim_request_obj.stage_hashes();

		// returns the stage corresponding to a previous simulation with same
		// parameters and the eventual mcpl filename
		// stages are ordered from the detector to the source
		auto  stage         = lc.get_stage(hashes);
		auto &istage        = stage.first;
		auto &mcpl_filename = stage.second;

#ifdef DEBUG
		std::cout << "------------------------------ Stages\n" << istage << "\t" << panosc::stages.at(istage) << "\tmcpl file:" << mcpl_filename << "\n" << std::endl;
#endif
		//--------------------- build list of arguments for the simulation program
		std::vector<std::string> args = sim_request_obj.args();

		args.push_back("--ncount=" + std::to_string(NEUTRONS_PER_JOB));
		args.push_back("stage=" + std::to_string(istage));
		if (!mcpl_filename.empty())
		    args.push_back("Vin_filename=" + mcpl_filename);

		// the app_name is the one defined in the cameo.xml file
		std::string app_name = (sim_request_obj.type() == panosc::sim_request::QUICK) ? sim_request_obj.instrument_name() + "-QUICK" : sim_request_obj.instrument_name() + "-" + panosc::stages.at(istage);

#ifdef DEBUG
		std::cout << "[DEBUG] APP: #" << app_name << "#" << std::endl;
		for (auto singlearg : args)
		    std::cout << "[DEBUG] arg: #" << singlearg << "#" << std::endl;
#endif

		if (nJobs > seeds.size()) {
		    std::cerr << "[ERROR] Jobs with ID > " << seeds.size() << " will have the same seed!" << std::endl;
		    throw std::runtime_error("Required number of jobs > the number of predefined seeds");
		    request->replyBinary(panosc::sim_request_answer_server::to_cameo(panosc::sim_request_answer::ansERROR));
		    continue;
		}
		request->replyBinary(panosc::sim_request_answer_server::to_cameo(panosc::sim_request_answer::ansSTARTING));

		//------------------------------ Start of the submission-retrieval thread
		request_th                       = std::move(request); // request_th has a higher scope
		std::vector<sim_job_t> &sim_jobs = running_simulations[sim_request_obj.hash()];
		panosc::simHash_t hash =sim_request_obj.hash();
			
		thread.reset(new std::thread([&server, &publisher, &running_simulations, &sim_jobs, app_name, lc, istage, &request_th, args, seeds, nJobs,hash]() {
		    //---------- function to merge the jobs
		    /* starts a merge job and waits for it to end and then publishes the result */
					auto mergeJobs = [&server, &publisher, &lc, &sim_jobs](bool finished) {
#ifdef DEBUG
			std::cout << "[STATUS] Start merging" << std::endl;
#endif
			std::string filenames;
			for (size_t iJob = 0; iJob < sim_jobs.size(); iJob++) {
			    const sim_job_t &sim_job = sim_jobs[iJob];
			    if (sim_job.state == cameo::application::SUCCESS) {
				filenames += (lc.output_dir(iJob) / "sDETECTOR.mcpl.gz");
					filenames += ";";
			    }
			}
			if(filenames.size()<1) return;
			std::vector<std::string> args;
			args.push_back("filenames=" + filenames);
			args.push_back("--dir=" + (lc.output_dir_merge()).string());

			fs::remove_all(lc.output_dir_merge());

			auto    mergingJob    = server.start("SIMD22-QUICKMERGE", args);
			state_t merging_state = mergingJob->waitFor();
			if (merging_state != cameo::application::SUCCESS) {
			    // put here an error message
			    std::cerr << "[ERROR MERGING] " << cameo::application::toString(merging_state) << std::endl;
			    panosc::sim_result_server sim_result(cameo::application::FAILURE);
			    publisher->sendBinary(sim_result.to_cameo());
			} else {
			    auto sim_result = read_result(lc.output_dir_merge(), finished ? cameo::application::SUCCESS : cameo::application::RUNNING);
			    publisher->sendBinary(sim_result.to_cameo());
			}
		    };

		    // submit immediately the maximum number of allowed jobs
		    // and then submit a new job when one finishes until nJobs
		    auto submitOneJob = [&server, &sim_jobs, app_name, &seeds, &lc, args]() {
			// let the simulation know the index in
			// order to set the output directories
			size_t i     = sim_jobs.size();
			auto   args_ = args;
			args_.push_back("--dir=" + lc.output_dir(i).string());
			args_.push_back("--seed=" + std::to_string(seeds[i]));

			sim_job_t sim_job;
			if (lc.is_done(i) == true) {
#ifdef DEBUG
			    std::cout << "[DEBUG] Job with index i=" << i << " already done" << std::endl;
#endif
			    sim_job.state = cameo::application::SUCCESS;
			} else {
#ifdef DEBUG
			    std::cout << "[DEBUG] Launching " << app_name << "\t";
			    for (const auto a : args_)
				std::cout << a << "\t";
			    std::cout << std::endl;
#endif
			    sim_job.instance = server.start(app_name, args_);
			    sim_job.instance->waitFor(511);
			    sim_job.state = (sim_job.instance->getLastState());
			}
			sim_jobs.push_back(std::move(sim_job));
		    }; // end of submitOneJob lambda function

		    size_t iJob = 0; // index of the last submitted job
		    for (; iJob < std::min(nJobs, MAX_PARALLEL_JOBS); ++iJob) {
			std::cout << "Submitting job ID: " << iJob << std::endl;
			submitOneJob();
		    }
		    std::cout << "END SUBMISSION" << std::endl;

		    for (size_t fJob = 0; fJob < nJobs; fJob++) {
			sim_job_t &sim_job = sim_jobs[fJob];
			// publish the partial result of the finished job
			if (fJob % MAX_PARALLEL_JOBS == 0 and sim_job.state!=cameo::application::SUCCESS)
			    mergeJobs(false);

			if (sim_job.state < cameo::application::PROCESSING_ERROR) { // it means that it is still running
			    std::cout << "Waiting from application " << *sim_job.instance << std::endl;
			    sim_job.state = sim_job.instance->waitFor();
			    std::cout << "[THREAD] Finished the simulation application "
			                 "with state "
			              << cameo::application::toString(sim_job.state) << std::endl;
			}
			if (sim_jobs.size() < nJobs) { // iJob is the index of the submitted
			    // jobs: i.e. not all submitted
			    // submit a new job to replace the old one
			    submitOneJob();
			}
		    }
		    mergeJobs(true);
		    running_simulations.erase(hash); // remove it from the list of
		    // running simulations

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
