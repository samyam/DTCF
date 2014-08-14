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

np=$1
LOCARGS="--block $COBALT_PARTNAME ${COBALT_CORNER:+--corner} $COBALT_CORNER ${COBALT_SHAPE:+--shape} $COBALT_SHAPE"
echo "Cobalt location args: $LOCARGS" >&2


#for N in 384
#do
#    for cntr_num in 1 2 3 8 11 12 13 14 15 16
#    do
#	echo "Contraction $cntr_num"
#   	for vp in 16
#	do
#
#	    gdim=${grid_dim[$cntr_num]}
#	    grid=${n[$np,$gdim]}
#	    name=${cntr[$cntr_num]}
#	    EXE="examples/$name.x"
#
#	    ./change_make.py "examples/$name"
#	    make BGQ=1 1> dump 2>&1
#	    ./change_ri_run.py "examples/$name"
#	    
#	    command="runjob -t 00:10:00 -n $np $EXE $N $vp ${gdim} ${grid}"
#	    #echo $outfile
#	    #echo $command
#	    
#	    echo "Contraction : $name     Tensor Size : $N      Block Grid : $vp"
#	    echo "Num Node    : $np       Grid Dims : $gdim     Grid : $grid"
#	    runjob -n $np -p 1 $LOCARGS --verbose=INFO : $EXE $N $vp ${gdim} ${grid}
#	    echo "Next Contarction"
#	    echo ""
#	    
#	done            
#    done
#done


for N in 64
do
    for cntr_num in 4
    do
	echo "Contraction $cntr_num"
  	for vp in 16 32
	do

	    gdim=${grid_dim[$cntr_num]}
	    grid=${n[$np,$gdim]}
	    name=${cntr[$cntr_num]}
	    output="examples/${np}_${N}_${name}_${vp}"
	    EXE="examples/$name.x"

	    ./change_make.py "examples/$name"
	    make BGQ=1 1> dump 2>&1
	    ./change_ri_run.py "examples/$name"

	    outfile=${np}_${N}_${cntr}.out
	    command="runjob -t 00:10:00 -n $np $EXE $N $vp ${gdim} ${grid}"
	    #echo $outfile
	    #echo $command
	    
	    echo "Contraction : $name     Tensor Size : $N      Block Grid : $vp"
	    echo "Num Node    : $np       Grid Dims : $gdim     Grid : $grid"
	    runjob -n $np -p 1 $LOCARGS --verbose=INFO : $EXE $N $vp ${gdim} ${grid}
	    echo "Next Contarction"
	    echo ""
	    
	done            
    done
done
