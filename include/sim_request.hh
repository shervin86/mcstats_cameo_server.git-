#ifndef SIM_REQUEST_CLASS
#define SIM_REQUEST_CLASS

/**
 * \class sim_request
 * \brief helper to code and decode simulation requests between Nomad and the mcstas_server
 * \author Shervin Nourbakhsh nourbakhsh@ill.fr
 *
 *
 */

class sim_request
{
      public:

	/** 
	 * 
	 * \par instrument_   name of the ILL instrument
	 * \par n_            number of neutrons to simulate
	 * \par params_       list of named parameters of the simulation 
	 */
	sim_request(std::string instrument_, unsigned long long int n_,
	            std::vector<std::string> params_)
	    :  _instrument(instrument_), _n(n_),  _params(params_)
	{
	}

	/** constructor that decodes the message from CAMEO
	 */
	sim_request(std::string message){
		std::stringstream s(message);
		std::string arg;
		s  >> _instrument 
		   >> arg /*-n*/
		   >> _n; 
		while(s.good()){

			s >> arg; // << "\n";
			_params.push_back(arg);
		}
		return;
	}
	
	/// transform the request into a string to be sent through CAMEO
	inline std::string encode(void){ 	
		return get_string();
	}
	

      private:
	std::string              _instrument;
	unsigned long long int   _n;
	std::vector<std::string> _params;


	/// transform the request into a string to be sent through CAMEO
	std::string get_string(void) const 	
	{
		std::stringstream s;
		s  << _instrument << "\n"
		   << "-n " << _n << "\n";
		for (auto &arg : _params) {
			s << arg << "\n";
		}
		return s.str();
	}

};

#endif
