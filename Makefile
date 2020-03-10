INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/include
CXXFLAGS = -std=c++11
CC=g++
LIBRARIES=-L/usr/local/lib/
CAMEO_LDD=-lcameo -lzmq -lprotobuf

default: mcstas_server fakeNomad

fakeNomad: fakeNomad.cpp
	$(CC) $(CXXFLAGS) $(LIBRARIES) -o $@ $< $(CAMEO_LDD)

mcstas_server: mcstas_server.cpp
	$(CC) $(CXXFLAGS) $(LIBRARIES) -o $@ $< $(CAMEO_LDD)

