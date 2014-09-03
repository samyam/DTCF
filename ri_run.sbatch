#!/bin/bash
# Job name:
#SBATCH --job-name=CAST

# use --exclusive to get the whole nodes exclusively for this job
#SBATCH --exclusive

#SBATCH --time=00:10:00

#SBATCH -p batch

# Set up work environment
#. /opt/intel/mkl/bin/mklvars.sh intel64
#
# Load your modules
module load intel/latest
module load mvapich2/1.9-icc-slurm
export MV2_ENABLE_AFFINITY=0
export MV2_DEFAULT_TIME_OUT=16
export MV2_DEFAULT_RETRY_COUNT=10
export MV2_DEFAULT_RNR_RETRY=10
#SBATCH --ntasks-per-node=8

export OMP_NUM_THREADS=8

# Set up your environment 
#cd $HOME/CAST
#
# Start your MPI job
if [ $1 -eq 128 ]
then
	n=256
else
	n=$1
fi
EXE=CCSD.x #tests/ga_interface/gaintface.x 
srun -N $1 -n $n ./$EXE $2 $3 $4 $5 $6 $7 $8 $9 $10
#srun -N 16 -n 16 ./$EXE 16 4 4 2 2 2 2 #$2 $3 $4 $5 $6 $7 $8 $9 $10
#make
