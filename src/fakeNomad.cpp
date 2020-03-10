#include <cameo/cameo.h>
#include <iostream>
#include <iomanip>

#define DEBUG
#define SERVERNAME "mcstas_server"
#define REQUESTER_RESPONDER_NAME "mcstas_responder"

int main(int argc, char *argv[])
{
	std::cout << "============================== Start of fakeNomad " << std::endl;
	cameo::application::This::init(argc, argv);

	// New block to ensure cameo objects are terminated before the
	// application. needed because of zmq
	{
#ifdef DEBUG
		for(size_t i=0; i < argc; ++i){
			std::cerr << "==" << argv[i] << std::endl;
		}
#endif
		
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
		          << cameo::application::toString(
		                 responderServer->now())
		          << std::endl;
#endif
		
		
		// Create a requester.
		std::unique_ptr<cameo::application::Requester> requester =
		    cameo::application::Requester::create(*responderServer,
		                                          REQUESTER_RESPONDER_NAME); // the name here has to be the same as on the server
		std::cout << "Created requester " << *requester << std::endl;
		if (requester.get() == 0) {
			std::cout << "requester error" << std::endl;
			return -1;
		}


/**
 *
 *
 *
 *
 *
 */
		std::string request="SIMD22\nlambda=4.5\n-n 1e6\n--dir=test_output/"; // \nnewparam=1";
		requester->sendBinary(request);
		// Wait for the response from the server.
		std::string response;
		requester->receiveBinary(response);
		#ifdef DEBUG
		std::cout << response << std::endl;
		#endif

		while(response!="OK"){
			requester->receiveBinary(response);
		}

		std::cout << "Finished the application" << std::endl;



		
	} // end of block to make sure zmq objects are closed properly
	return 0;
}
