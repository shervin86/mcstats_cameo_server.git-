#!/bin/fish
./build/DEVEL/mcstas/ILL_H512_D22.out -s 54321 -n 2.6e6 -d /tmp/ILL_2.6e6/ lambda=4.5
set file /tmp/ILL_2.6e6/D22_PreV_1608549728.x_y
sed '/#/ d; s| |\n|g;/^$/ d' $file | sed '/^$/d' | sort -n  >/tmp/ILL_2.6e6/weights.dat


### gnuplot
# p '/tmp/ILL_2.6e6/weights.dat' u 1 bins
