#!/bin/bash

for p1 in 4
do
    for p2 in 4 8
    do
        if [ "$((p1*p1*p2*p2))" -gt 1024 ]
        then
            continue
        fi
        for Vp in 4 8 16
        do
            for N in 256
            do
                #qsub -A PARTS -t 01:00:00 -n 256 ./run.x 4 4 16 128
                until qsub -A PARTS -t 01:00:00 -n $((p1*p1*p2*p2)) -O ${p1}_${p2}_${Vp}_${N} ./run.x $p1 $p2 $Vp $N
                do
                    echo "Failed at: $p1 $p2 $Vp $N"
                    sleep 60
                done
            done
        done
    done
done
