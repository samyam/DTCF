#!/bin/bash

grid[32]="2 4"
grid[25]="5 1"
grid[36]="2 3"
grid[49]="7 1"
grid[64]="4 2"
grid[81]="3 3"
grid[100]="2 5"
grid[256]="4 4"

outfile=RESULTS_RI/10_13_2013/cntr${cntr}/${np}_${N}_ctr${cntr}_${vp}.out
outfile=$1.out
echo $outfile
command="sbatch -N $1 --output=$outfile ./ri_run.sbatch $1 $2 $3 $4 $5 $6 $7 $8"
echo $command
`$command`
			

#for np in 32 #64 256
#do
#	for vp in 4 #16
#	do
#		for N in 32 #128 192 256
#		do
#			for cntr in 1 #3 4 6 7
#			do
#				#outfile=RESULTS_RI/10_13_2013/cntr${cntr}/${np}_${N}_ctr${cntr}_${vp}.out
#				outfile=$1.out
#				echo $outfile
#				if [ $np -eq 256 ]
#				then
#					NP=$(echo "$np / 2" | bc) 
#					command="sbatch -N $NP --output=$outfile ./ri_run.sbatch $NP ${grid[$np]} $vp $N $cntr $np"
#				else
#					command="sbatch -N $np --output=$outfile ./ri_run.sbatch $np ${grid[$np]} $vp $N $cntr $np"
#				fi
#				echo $command
#				`$command`
#			done
#		done
#	done
#done
#

#for np in 64 256
#do
#	for vp in 4 8 16
#	do
#		for N in 128 192 #256
#		do
#			for cntr in 2
#			do
#				outfile=RESULTS_RI/10_13_2013/cntr${cntr}/${np}_${N}_ctr${cntr}_${vp}.out
#				echo $outfile
#				if [ $np -eq 256 ]
#				then
#					NP=$(echo "$np / 2" | bc) 
#					command="sbatch -N $NP --output=$outfile ./ri_run.sbatch $NP ${grid[$np]} $vp $N $cntr $np"
#				else
#					command="sbatch -N $np --output=$outfile ./ri_run.sbatch $np ${grid[$np]} $vp $N $cntr $np"
#				fi
#				echo $command
#				`$command`
#			done            
#		done
#	done
#done


#grid[16]="4 4"
#grid[64]="8 8"
#grid[256]="16 16"
#grid[1024]="32 32"
#
#for np in 64 256
#do
#	for vp in 4 8 16
#	do
#		for N in 192 256 512 1024 2048 4096 #8192 16384 32768
#		do
#			for cntr in 5
#			do
#				outfile=RESULTS_RI/10_13_2013/cntr${cntr}/${np}_${N}_ctr${cntr}_${vp}.out
#				echo $outfile
#				if [ $np -eq 256 ]
#				then
#					NP=$(echo "$np / 2" | bc) 
#					command="sbatch -N $NP --output=$outfile ./ri_run.sbatch $NP ${grid[$np]} $vp $N $cntr $np"
#				else
#					command="sbatch -N $np --output=$outfile ./ri_run.sbatch $np ${grid[$np]} $vp $N $cntr $np"
#				fi
#				echo $command
#				`$command`
#
#			done
#		done            
#	done
#done
