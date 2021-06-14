#!/bin/sh

# this is the cameo config file
CONFIG_FILE=${1:-/usr/share/mcstas_server/mcstas_server.xml}
test -e ${CONFIG_FILE} || {
	echo "ERROR: config file \"${CONFIG_FILE}\" not found. Wrong filename or wong path"
	exit 1
}

which cameo-server || {
	echo "ERROR: cameo-server command not found. Cannot launch the simulation server"
	exit 1
}

cameo-server --log-console ${CONFIG_FILE}
