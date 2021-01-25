# awk -f plot.awk file.x_y > file.gpl.dat
# this script splits the file into the three variables to be able to plot with gnuplot
# gnuplot -c plot.gpl file.gpl.dat outfile.pdf
BEGIN{



}
(/xlabel/){
	xlabel = sprintf("%s %s",$3, $4)
}

(/#/ && /I:/){
	variable="I"
	filename=sprintf("%s.dat",variable)
	printf("#%s",variable) > filename
	y=ymin
	printf("\n\n#%s\n",variable)
}

(/#/ && /I_err:/){
	variable="I_err"
	filename=sprintf("%s.dat",variable)
	printf("#%s",variable) > filename
	y=ymin
	printf("\n\n#%s\n",variable)
}

(/#/ && /N:/){
	variable="N"
	filename=sprintf("%s.dat",variable)
	printf("#%s",variable) > filename
	y=ymin
	printf("\n\n#%s\n",variable)

}

(/xylimits/){
	xmin=$3
	xmax=$4
	ymin=$5
	ymax=$6
}
	
(!/#/){
	print $0 >> filename
	xbin_width=(xmax-xmin)/(NF+1)
	ybin_width=(ymax-ymin)/(NF+1)
	x=xmin+xbin_width/2

	for(i =0; i <=NF; i++){
		z=$i
		printf("%f\t%f\t%f\n", x, y, z)
		x+=xbin_width
	}
	y+=ybin_width
	
}

END{

	printf("#set xlabel %s\n", xlabel)
	printf("#set ylabel %s\n", ylabel)




}
