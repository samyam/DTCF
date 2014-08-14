#!/bin/bash
for p1 in 4 #1 2 3 4 5 6 7 8
do
  for p2 in 4 #1 2 3 4 5 6 7 8
  do
    np=$((p1*p1*p2*p2)) 
    nc=$((np*6))
    for Vb in 8 16 32 64
    do
      for N in 128 256
      do
        (echo '#PBS -q regular'
        echo '#PBS -l mppwidth='$nc
        echo '#PBS -l walltime=00:30:00'
        echo '#PBS -N cast'$p1$p2$Vb$N
        echo '#PBS -e cast'$p1$p2$Vb$N'.$PBS_JOBID.err'
        echo '#PBS -o cast'$p1$p2$Vb$N'.$PBS_JOBID.out'
        echo '#PBS -V'
        echo 'cd $PBS_O_WORKDIR'
        echo 'setenv OMP_NUM_THREADS 6'
        echo 'aprun -n '$np' -N 4 -S 1 -d 6 ./run.x '$p1 $p2 $Vb $N) > j$p1$p2$Vb$N.scp
        echo 'qsub j'$p1$p2$Vb$N'.scp'
        qsub j$p1$p2$Vb$N.scp
      done
    done
  done
done
