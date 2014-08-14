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
cntr[17]="cntr2e_4D"
cntr[18]="cntr3d_4D"

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

for np in 128 #32 64 128
do
    for size in 256 #64 128 256 384
    do
	for cntr_num in 18 #18 12 16 14 #8 9 10 11 12 13 14 15 16 18 #2 3 8 9 10 11 12 13 14 15 16 18 #11 12 13 14 15 16
	do
	    for bg in 4 16 32 #8 16 32
	    do
		gdim=${grid_dim[$cntr_num]}
		grid=${n[$np,$gdim]}
		name=examples/${cntr[$cntr_num]}
		echo $name
		outfile="${fold}/${np}_${cntr[$cntr_num]}_${size}_${bg}_parallel.out"
		EXE="./examples/$name.x"
		./change_make.py $name
		make RI=1
		./change_ri_run.py $name
		command="sbatch -N $np --output=$outfile ./ri_run.sbatch $np $size $bg $gdim $grid"
		echo $command
		`$command`
		

	    done
	done
    done
done

#for np in 128 #32 64 128
#do
#    for size in 256 #64 128 256 384
#    do
#	for cntr_num in 18 12 14 #8 9 10 11 12 13 14 15 16 18 #2 3 8 9 10 11 12 13 14 15 16 18 #11 12 13 14 15 16
#	do
#	    for bg in 8 #8 16 32
#	    do
#		gdim=${grid_dim[$cntr_num]}
#		grid=${n[$np,$gdim]}
#		name=examples/${cntr[$cntr_num]}
#		echo $name
#		outfile="${fold}/${np}_${cntr[$cntr_num]}_${size}_${bg}_parallel.out"
#		EXE="./examples/$name.x"
#		./change_make.py $name
#		make RI=1
#		./change_ri_run.py $name
#		command="sbatch -N $np --output=$outfile ./ri_run.sbatch $np $size $bg $gdim $grid"
#		echo $command
#		`$command`
#		
#
#	    done
#	done
#    done
#done
#
#for np in 128 #32 64 128
#do
#    for size in 48 #64 128 256 384
#    do
#	for cntr_num in 7 #8 9 10 11 12 13 14 15 16 18 #2 3 8 9 10 11 12 13 14 15 16 18 #11 12 13 14 15 16
#	do
#	    for bg in 8 #8 16 32
#	    do
#		gdim=${grid_dim[$cntr_num]}
#		grid=${n[$np,$gdim]}
#		name=examples/${cntr[$cntr_num]}
#		echo $name
#		outfile="${fold}/${np}_${cntr[$cntr_num]}_${size}_${bg}_parallel.out"
#		EXE="./examples/$name.x"
#		./change_make.py $name
#		make RI=1
#		./change_ri_run.py $name
#		command="sbatch -N $np --output=$outfile ./ri_run.sbatch $np $size $bg $gdim $grid"
#		echo $command
#		`$command`
#		
#
#	    done
#	done
#    done
#done

#for np in 128
#do
#    for size in 32 48 
#    do
#	for cntr_num in 7 17
#	do
#	    for bg in 8 16
#	    do
#		gdim=${grid_dim[$cntr_num]}
#		grid=${n[$np,$gdim]}
#		name=examples/${cntr[$cntr_num]}
#		echo $name
#		outfile="examples/RESULTS/${np}_${cntr[$cntr_num]}_${size}_${bg}_parallel.out"
#		EXE="./examples/$name.x"
#		./change_make.py $name
#		make RI=1
#		./change_ri_run.py $name
#		command="sbatch -N $np --output=$outfile ./ri_run.sbatch $np $size $bg $gdim $grid"
#		echo $command
#		`$command`
#		
#
#	    done
#	done
#    done
#done


#for np in 32 64 128
#do
#    for size in 32 48 64
#    do
#	for cntr_num in 4 5 6 7 
#	do
#	    for bg in 8 16
#	    do
#		gdim=${grid_dim[$cntr_num]}
#		grid=${n[$np,$gdim]}
#		name=examples/${cntr[$cntr_num]}
#		echo $name
#		output="examples/RESULTS/${np}_${name}_${N}_${bg}"
#		EXE="./examples/$name.x"
#		./change_make.py $name
#		make RI=1
#		./change_ri_run.py $name
#		command="sbatch -N $np --output=$outfile ./ri_run.sbatch $np $size $bg $gdim $grid"
#		echo $command
#		`$command`
#		
#
#	    done
#	done
#    done
#done



#np=$2
#gd=$3
#size=$4
#bg=$5
#grid=${n[$np,$gd]}
#echo $grid
#
##rm examples/*.o
##./pars.out
##scancel -n CAST
#./change_make.py $1
#make RI=1
#./change_ri_run.py $1
#
#
#
#outfile=$1-np$2.out
#echo $outfile
#command="sbatch -N $2 --output=$outfile ./ri_run.sbatch $np $size $bg $gd $grid"
#echo $command
#`$command`
#

#./schedule_ri.sh $1 $2
