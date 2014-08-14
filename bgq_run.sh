#!/bin/bash

declare -A n
./pars.out
mkdir RESULTS/`date +%d_%m_%y`
fold="RESULTS/`date +%d_%m_%y`"

#qsub --jobname RRR -A DAPPX -t 30 -n 4096 -O 128-cast --mode script bgq_run_script.sh 4096
for np in 4096 #32 #64 128 256 #512 1024 2048 4096 16384
do
    #outfile=./examples/RESULTS/$np-CAST
    #output=./$np-CAST
    output="${fold}/${np}-CAST"
    echo $output
    #command="qsub --jobname RRR -A PARTS -t 30 -n $np -O $output bgq_run_script.sh $np"
    #echo $outfile
    #echo $command
    qsub --jobname RRR -A DAPPX -t 30 -n ${np} -O ${output} --mode script bgq_run_script.sh ${np}
         
done



#for np in 16 64 256 1024
#do
#    for vp in 4 8 16 32 64 128
#    do
#	for N in 192 256 320 384
#	do
#	    for cntr in 2
#	    do
#	       until qsub -A PARTS -t 00:30:00 -n $np -O RESULTS/10_09_2013/cntr${cntr}/${np}_${N}_ctr${cntr}_${vp} ./run.x ${grid[$np]} $vp $N $cntr
#                do
#                    echo "Failed at: ${grid[$np]} $vp $N $cntr"
#                    sleep 60
#                done
#	    done            
#	done
#    done
#done

#grid[16]="4 4"
#grid[64]="8 8"
#grid[256]="16 16"
#grid[1024]="32 32"
#
#size[16]="8192"
#size[64]="12992"
#size[256]="20672"
#size[1024]="32768"
#
#
#for np in 16 64 256 1024
#do
#    for vp in 4 8 16 32 
#    do
##	for N in 192 256 512 1024 2048 4096 8192 16384 32768
##	do
#	    for cntr in 5
#	    do
#	       until qsub -A PARTS -t 00:30:00 -n $np -O RESULTS_BGQ_SCALABILITY/`date +%m_%d_%Y`/cntr${cntr}/${np}_${size[$np]}_ctr${cntr}_${vp} ./run.x ${grid[$np]} $vp ${size[$np]} $cntr
#                do
#                    echo "Failed at: ${grid[$np]} $vp $N $cntr"
#                    sleep 60
#                done
##	    done            
#	done
#    done
#done
