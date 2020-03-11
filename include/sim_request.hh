#ifndef SIM_REQUEST_CLASS
#define SIM_REQUEST_CLASS
#include <ostream>
#include <sstream>
#include <vector>
/**
 * \class sim_request
 * \brief helper to code and decode simulation requests between Nomad and the mcstas_server
 * \author Shervin Nourbakhsh nourbakhsh@ill.fr
 */

class sim_request
{
      public:
	/** \brief constructor to be used on the client side
	 *  \par instrument_   name of the ILL instrument
	 *  \par n_            number of neutrons to simulate
	 *  \par params_       list of named parameters of the simulation
	 */
	sim_request(std::string instrument_, unsigned long long int n_,
	            std::vector<std::string> params_)
	    : _instrument(instrument_), _n(n_), _params(params_)
	{
	}

	/** \brief constructor that decodes the message from CAMEO (server side)
	 */
	sim_request(std::string message)
	{
		std::stringstream s(message);
		std::string       arg;
		s >> arg; // expecting "SIM", to be discarded
		if (arg == "SIM")
			_good = true;
		s >> _instrument >> arg /*-n, to be discarded*/
		    >> _n;
		do {
			s >> arg; // << "\n";)
			_params.push_back(arg);
		} while (s.good());
		return;
	}

	/// returing the string "SIM"+name of the instrument
	inline std::string instrument(void) const { return "SIM" + _instrument; };

	/// returns the arguments to be passed to the mcstas execution
	inline std::vector<std::string> args(void) const
	{
		std::vector<std::string> args(_params.begin(), _params.end());
		args.push_back(std::string("-n ") + std::to_string(_n));
		return args;
	}

	/// \brief method to check if the request is a VALID request
	bool good(void) const { return _good; };

	friend std::ostream &operator<<(std::ostream &os, const sim_request &s);

	/** \brief transform the request into a string to be sent through CAMEO
	 *
	 * this method is kept if one wants to decouple the printing and the encoding into string
	 */
	inline std::string to_string(void) const
	{
		std::stringstream s;
		s << *this;
		return s.str();
	}


      private:
	std::string              _instrument;
	unsigned long long int   _n;
	std::vector<std::string> _params;
	bool                     _good;
};

std::ostream &operator<<(std::ostream &os, const sim_request &s)
{
	os << "SIM"
	   << " " << s._instrument << "\n"
	   << "-n " << s._n;
	for (const auto &arg : s._params) {
		os << "\n" << arg;
	}
	return os;
}
#endif
