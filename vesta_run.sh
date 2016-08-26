#!/bin/bash

declare -A n


n[32,2]="8 4"
n[32,3]="4 4 2"
n[32,4]="4 2 2 2"
n[32,5]="2 2 2 2 2"

n[64,2]="8 8"
n[64,3]="4 4 4"
n[64,4]="4 4 2 2"
n[64,5]="4 2 2 2 2"

n[128,2]="16 8"
n[128,3]="8 4 4"
n[128,4]="4 4 4 2"
n[128,5]="4 4 2 2 2"

n[256,2]="16 16"
n[256,3]="8 8 4"
n[256,4]="4 4 4 4"
n[256,5]="4 4 4 2 2 "

n[512,2]="32 16"
n[512,3]="8 8 8"
n[512,4]="8 4 4 4"
n[512,5]="4 4 4 4 2"

n[1024,2]="32 32"
n[1024,3]="16 8 8"
n[1024,4]="8 8 4 4"
n[1024,5]="8 4 4 4 2"

n[2048,2]="64 32"
n[2048,3]="16 16 8"
n[2048,4]="8 8 8 4"
n[2048,5]="8 8 4 4 2"

n[4096,2]="64 64"
n[4096,3]="16 16 16"
n[4096,4]="8 8 8 8"
n[4096,5]="8 8 8 4 2"

n[8192,2]="128 64"
n[8192,3]="32 16 16"
n[8192,4]="16 8 8 8"
n[8192,5]="8 8 8 8 2"

n[16384,2]="128 128"
n[16384,3]="32 32 16"
n[16384,4]="16 16 8 8"
n[16384,5]="16 8 8 8 2"



cntr[1]="cntr1a_4D"
cntr[2]="cntr1b_5D"
cntr[3]="cntr1c_4D"
cntr[4]="cntr2a_4D"
cntr[5]="cntr2b_5D"
cntr[6]="cntr2c_5D"
cntr[7]="cntr2d_5D"
cntr[8]="cntr3a_4D"
cntr[9]="cntr3b_2D"
cntr[10]="cntr3c_5D"
cntr[11]="cntr4a_4D"
cntr[12]="cntr4b_4D"
cntr[13]="cntr5a_4D"
cntr[14]="cntr5b_5D"
cntr[15]="cntr5c_3D"
cntr[16]="cntr5d_3D"
cntr[17]="cntr3d_4D"
cntr[18]="redistribute_test"

grid_dim[1]=4
grid_dim[2]=5
grid_dim[3]=4
grid_dim[4]=4
grid_dim[5]=5
grid_dim[6]=5
grid_dim[7]=5
grid_dim[8]=4
grid_dim[9]=2
grid_dim[10]=5
grid_dim[11]=4
grid_dim[12]=4
grid_dim[13]=4
grid_dim[14]=5
grid_dim[15]=3
grid_dim[16]=3
grid_dim[17]=4
grid_dim[18]=4

mkdir RESULTS/`date +%d_%m_%y`
fold="RESULTS/`date +%d_%m_%y`"
./pars.out

for np in 256 #64 128 256 512 1024
do
    for cntr_num in 18 #1 2 3 8 9 10 11 12 13 14 15 16 #{1..16}
    do
	echo "Contraction $cntr_num"
	for N in 384 #128 256 384 #32 64 128 256
	do
	    for vp in 8 #8 16 32 #4 8 16 32 
	    do

		gdim=${grid_dim[$cntr_num]}
		grid=${n[$np,$gdim]}
		name=${cntr[$cntr_num]}
		output="${fold}/${np}_${name}_${N}_${vp}"
		#output="512_main_reduction"
		EXE="./examples/$name.x"
		#EXE="./main.x"
		./change_make.py "examples/$name"
		#./change_make.py "main"
		make BGQ=1 
		./change_ri_run.py "examples/$name"

		#outfile=${name}_${np}_${N}.out
		#command="runjob -t 00:10:00 -n $np $EXE $N $vp ${gdim} ${grid}"
		#echo $outfile
		#echo $command
		
		echo "Contraction : $name     Tensor Size : $N      Block Grid : $vp"
		echo "Num Node    : $np       Grid Dims : $gdim     Grid : $grid"
		#runjob -n $np -p 1 $LOCARGS --verbose=INFO : $EXE $N $vp ${gdim} ${grid}
		
		command="qsub --jobname RRR -A PARTS -t 30 -n $np -O $output $EXE $N $vp ${gdim} ${grid}"
		#echo $outfile
		echo $command
		until qsub --jobname RRR -A PARTS -O ${output} -t 10 -n $np ${EXE} $N $vp ${gdim} ${grid}
		do 
		    sleep 60
		done
		
	    done
	    
	    echo "Next Contarction"
	    echo ""
	    
	done            
    done
done

#for np in 1024 #64 128 256 512 1024
#    for cntr_num in 4 5 6 7
#    do
#	echo "Contraction $cntr_num"
#	for N in 32 48 64 #32 64 128 256
#	do
#	    for vp in  16 32 64 #4 8 16 32 
#	    do
#
#		gdim=${grid_dim[$cntr_num]}
#		grid=${n[$np,$gdim]}
#		name=${cntr[$cntr_num]}
#		output="${fold}/${np}_${name}_${N}_${vp}"
#		EXE="./examples/$name.x"
#
#		./change_make.py "examples/$name"
#		make BGQ=1 
#		./change_ri_run.py "examples/$name"
#
#		#outfile=${name}_${np}_${N}.out
#		#command="runjob -t 00:10:00 -n $np $EXE $N $vp ${gdim} ${grid}"
#		#echo $outfile
#		#echo $command
#		
#		echo "Contraction : $name     Tensor Size : $N      Block Grid : $vp"
#		echo "Num Node    : $np       Grid Dims : $gdim     Grid : $grid"
#		#runjob -n $np -p 1 $LOCARGS --verbose=INFO : $EXE $N $vp ${gdim} ${grid}
#		
#		command="qsub --jobname RRR -A PARTS -t 30 -n $np -O $output $EXE $N $vp ${gdim} ${grid}"
#		#echo $outfile
#		echo $command
#		until qsub --jobname RRR -A PARTS -O ${output} -t 10 -n $np ${EXE} $N $vp ${gdim} ${grid}
#		do 
#		    sleep 60
#		done
#		
#	    done
#	    
#	    echo "Next Contarction"
#	    echo ""
#	    
#	done            
#    done
#done
