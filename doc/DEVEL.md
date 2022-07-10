# Add new instrument

   1. instruments.hh 
	  Add a new element to the enum instrument_t for the new instrument
   1. Add a new entry to the instrument_t json serialization with a string corresponing to the new enum element to represent in the json the name of the instrument
   1. check all the parameters required by the instrument are listed in the param_t enum
