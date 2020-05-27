### Clone
After cloning the repository, setup the githooks:
```
git config --local core.hooksPath .githooks/
```

### Install (client API only)

```
mkdir build/
cd build/
cmake ../

cmake --build .

make install
```

### Install (server)
In order to run the server, you need to have McStas installed.
Then compile the instrument file with McStas:

```
mkdir build/
cd build/
cmake -DSERVER=True -DCMAKE_INSTALL_PREFIX=/tmp/devel/ ..
cmake --build .

make install
```


### Documentation (Doxygen)
from your build directory
```
firefox  doc/html/index.html &
```

or you can find it at https://nourbakhsh.sites.code.ill.fr/mcstats_cameo_server/

## Cameo applications

 - fakeNomad: this program emulates what the Nomad server should/might do
 - mcstas_server: small server dealing with Nomad requests coming through CAMEO and launching the simulations


## Test
```
xterm -e cameo-server cameo_config.xml &
sleep 2s
xterm -e cmo -e tcp://localhost:7123 exec mcstas_server &
sleep 2s
xterm -e cmo -e tcp://localhost:7123 exec fakeNomad &

```



# Workflow

## Client

 #. build the request in json format using the API
 #. send the request via CAMEO to the server
 #. wait for the answer (exit status of the simulation)
 #. if SUCCESS, request for the result attributes
 #. request for the result data
    -> data should be in openPMD format (JSON or HD5)  \todo check openPMD API for C++


## Server

 #. wait for requests
   - calculation/simulation request:
	  #. receive the json request
	  #. check if simulation already done
	  #. if not check if partial simulation already done (any stage to be re-used)
	  #. start the job via CAMEO with the right parameters
   	  #. determine if asking for 
	     - no results -> return exit status
	     - counts     -> return exit status and counts (openPMD format (JSON or HDF5))
		 - errors     -> return exit status and errors
		 - MC neutrons
		 - entire folder -> return exit status and tgz of the simulation folder
	  
      
  calculation request:
    1. destroy previously stored images.   -> won't implement now, I don't think it is useful
    2. calculate and store the images.
    3. return related information (sizes, etc.).
  counts request:
    return the binary array of the counts image if it exists.
  errors request;
    return the binary array of the errors image if it exists.
  MC neutrons:
    return the binary array of the true neutrons image if it exists.

