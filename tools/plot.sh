#!/usr/bin/fish
set dir $argv

for file in $dir/*.x_y
    set -l f $dir/(basename $file .x_y | sed -r 's|_[0-9]+||').dat
    awk -f tools/plot.awk $file > $f
    gnuplot -c tools/plot.gpl $f $f.pdf
end      
