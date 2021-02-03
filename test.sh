#!/usr/bin/fish

cd mcstas/build/
cmake .. -DCMAKE_INSTALL_PREFIX=/tmp/devel -DMCSTAS=$MCSTAS
cmake --build . --target install
rm /tmp/mcstas/ -Rf
cd -

./mcstas/build/ILL_H512_D22-sSAMPLE.out  -n 12345678 lambda=4.5 sample_size_r=0.005 D22_collimation=2.0 D22_sample=H2O_liq.qSq sample_size_y=0.05 Vin_filename=/dev/shm/panosc/prod/SIMD22/MCPL/sSAMPLE/17367311578647498031.mcpl.gz stage=-1 --dir=/tmp/mcstas/ | tee /tmp/mcstas/stdout.log


./tools/plot.sh /tmp/mcstas
