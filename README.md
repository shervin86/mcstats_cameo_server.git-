### Clone
After cloning the repository, setup the githooks:
```
git config --local core.hooksPath .githooks/
```

### Compilation

```
mkdir build/
cd build/
cmake ../

cmake --build .
```

### McStas instrument
In order to run the server, you need to have McStas installed.
Then compile the instrument file with McStas:
```
#!/usr/bin/fish
set INSTRUMENT ILL_H512_D22
cd build/
mcstas -t  -o $INSTRUMENT.c  ../mcstas/$INSTRUMENT.instr 
gcc -O3 -o $INSTRUMENT.out $INSTRUMENT.c -lm
```


### Documentation (Doxygen)
from your build directory
```
firefox  doc/html/index.html &
```


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

