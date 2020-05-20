#!/bin/bash
#create a test config
sed 's|7123|7100|;s|logs|test/logs|' cameo_config.xml  > test/cameo_config_test.xml

# start another cameo server
/usr/local/bin/cameo-server test/cameo_config_test.xml &> test/cameo.log &
cameo_PID=$!

# start the simulation server
cmo -e tcp://localhost:7100 start mcstas_server &> test/server.log

# start the client
rm /dev/shm/SIMD22/* -Rf
time cmo -e tcp://localhost:7100 exec fakeNomad
time cmo -e tcp://localhost:7100 exec fakeNomad

#cmo -e tcp://localhost:7100 stop mcstas_server

#kill $cameo_PID





