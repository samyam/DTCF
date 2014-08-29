#make main.x TIMER=1 RI=1 CTX=0 CHECK=1 VERIFY=1 DEBUG=1 TASCEL=1 CACHING=1
#make clean
#make -j8 main.x TIMER=1 RI=1 CACHING=0
#cp main.x main-nocache

make clean
make -j8 main.x TIMER=1 RI=1 #CACHING=1

#cp main.x main-cache
# salloc -N 16
# srun -n 16 ./main.x -n 20 -w 10 -t 8
