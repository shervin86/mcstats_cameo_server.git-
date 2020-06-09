#include "sim_request.hh"
#include <cassert>


int main(void){

  panosc::sim_request request;
  request.set_instrument(panosc::D22);
			// request.set_num_neutrons(10000000);
  request.set_measurement_time(1);

  // block of implemented parameters
  try{
    request.add_parameter(panosc::sim_request::pWAVELENGTH, 4.51);
    request.add_parameter(panosc::sim_request::pCOLLIMATION, 2.00);
  } catch (const panosc::param_not_implemented &e) {
    return 1;
  }

  // test exception throwing for not implemented parameters
  try {
    request.add_parameter(panosc::sim_request::pNOTIMPLEMENTED, 2.00);
  } catch (const panosc::param_not_implemented &e) {
    //std::cerr << e.what() << std::endl;
  }
  request.set_return_data(panosc::sim_request::rNONE);

  // print out the request to check if anything changed 
  std::ofstream fout("testclient.json");
  fout << request << std::endl;

  std::ifstream fin("../../request.json");
  panosc::sim_request req_json;
  req_json.read_json(fin);

  // test the the json is unchanged
  assert(req_json.to_cameo() == request.to_cameo());
  return 0;
}
