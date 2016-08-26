#!/usr/bin/bash
filename[1]="cntr1a"
filename[2]="cntr1b"
filename[3]="cntr1c"
for i in {1,2,3}
do
   ./change_make.py examples/${filename[$i]} examples/${filename[$i]}.x
   make RI=1
done

