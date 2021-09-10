import sys, os

instrument_pyfile = sys.argv[1]


print(instrument_pyfile)
instrument_name = os.path.basename(instrument_pyfile)
print(instrument_name)
#from instrument_pyfile import instrument
exec(instrument_pyfile)
