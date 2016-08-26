#!/bin/bash
rm examples/*.o
./pars.out
./change_make.py $1
make RI=1
./change_ri_run.py $1
./schedule_ri.sh $1 $2
