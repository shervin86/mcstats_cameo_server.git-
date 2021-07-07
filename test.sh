#!/usr/bin/fish
set -l build_dir /dev/shm/mcstats_cameo_server/server/
cmake -S . -B $build_dir -DSERVER=ON -DENABLE_TESTING=ON
cmake --build $build_dir || exit 1
launch_server.sh $build_dir/test/mcstas_server.xml &

ctest  -V --test-dir $build_dir -R testserver


exit 0
echo $MCSTAS
cd mcstas/build/
cmake .. -DCMAKE_INSTALL_PREFIX=/tmp/devel -DMCSTAS=$MCSTAS
cmake --build . --target install
rm /tmp/mcstas/ -Rf
cd -

#igprof ./mcstas/build/ILL_H512_D22-sSAMPLE.out  -n 1234567 lambda=4.5 sample_size_r=0.005 D22_collimation=2.0 D22_sample=H2O_liq.qSq sample_size_y=0.05 Vin_filename=/dev/shm/panosc/prod/SIMD22/MCPL/sSAMPLE/17367311578647498031.mcpl.gz stage=-1 --dir=/tmp/mcstas/ | tee /tmp/mcstas/stdout.log
exit 0
mkdir /tmp/mcstas/ -p
for in in (seq 1 5)
    set outdir /tmp/mcstas/$in
    rm $outdir -R
    ./mcstas/build/ILL_source_simple_test.out  -n 12345678 lambda=4.5 sample_size_r=0.005 D22_collimation=2.0 D22_sample=H2O_liq.qSq sample_size_y=0.05 Vin_filename=/dev/shm/panosc/prod/SIMD22/MCPL/sSAMPLE/17367311578647498031.mcpl.gz stage=-1 --dir=$outdir | tee /tmp/stdout-$in.log
end
#./tools/plot.sh /tmp/mcstas
