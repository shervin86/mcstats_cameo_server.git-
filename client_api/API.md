\defgroup clientAPI Client API
\brief Set of classes to be used on the client side

There are two classes to be included on the client side:
  - sim_request: \copybrief panosc::sim_request
  - sim_result: \copybrief panosc::sim_result


# Workflow from the client side

 -# declare a sim_request object and set the simulation parameters
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






