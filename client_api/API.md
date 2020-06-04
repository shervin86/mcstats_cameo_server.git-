\defgroup SIM_PARAMETERS Simulation parameters

\defgroup clientAPI Client API
\brief C++ client API

\author Shervin Nourbakhsh nourbakhsh@ill.fr


## Headers
There are two classes to be included on the client side:
  - panosc::sim_request: \copybrief panosc::sim_request
  - panosc::sim_result: \copybrief panosc::sim_result

## Implemented instruments
Implemented instruments are listed in: panosc::instrument_t

## Parameters accepted
The list of accepted parameters is listed in: panosc::sim_request::param_t

If you try to add a parameter in the list that has been marked as NOT IMPLEMENTED, an exception of type panosc::param_not_implemented

## Workflow from the client side

 -# declare a sim_request object and set the simulation parameters \n
 Accepted simulation parameters are listed in panosc::sim_request::param_t
 \snippet src/fakeNomad.cpp request
 \snippet src/fakeNomad.cpp request2
 -# send the request via CAMEO to the server
 \snippet src/fakeNomad.cpp send request
 -# receive the answer (result + exit status)
   - answer received in a string
   - parse the string with the sim_result
 \snippet src/fakeNomad.cpp receive result
 -# check the job exit status
 \snippet src/fakeNomad.cpp return state
 -# request for the result data
	
    - Data are 2D array linearized in a vector (x1y1, ..., xNy1, x1y2, ..., xNy2, ..., x1yM, ..., xNyM)
	\snippet src/fakeNomad.cpp get data
    \todo -> data should be in openPMD format (JSON or HD5)






