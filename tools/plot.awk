# awk -f plot.awk file.x_y > file.gpl.dat
# this script splits the file into the three variables to be able to plot with gnuplot
# gnuplot -c plot.gpl file.gpl.dat outfile.pdf
BEGIN{



}
(/xlabel/){
	xlabel = sprintf("%s %s",$3, $4)
}

(/xylimits/){
	xmin=$3
	xmax=$4
	ymin=$5
	ymax=$6
	xbin_width=(xmax-xmin)/xbins
	ybin_width=(ymax-ymin)/ybins
}

(/# type:/){
	xbins=$3
	ybins=$4
	sub("array_2d.", "", xbins)
	sub("\\(", "", xbins)
	sub(",", "", xbins)
	sub("\\)", "", ybins)
}

(/#/ && (/I:/ || /I_err:/ || /N:/) ){
	variable=$4
	sub(":","",variable)
	filename=sprintf("%s.dat",variable)
#	printf("#%s",variable) > filename
	y=ymin-ybin_width/2
	printf("\n\n#%s\n",variable)
}

	
(!/#/){
	x=xmin-xbin_width/2
	for(i =0; i <=NF; i++){
		z=$i
		printf("%f\t%f\t%.4g\n", x, y, z)
		x+=xbin_width
	}
	y+=ybin_width
	
}

END{

	printf("#set xlabel %s\n", xlabel)
	printf("#set ylabel %s\n", ylabel)




}
