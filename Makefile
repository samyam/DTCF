#LIBS = -Wl,--start-group  $(MKLROOT)/lib/intel64/libmkl_intel_lp64.a $(MKLROOT)/lib/intel64/libmkl_sequential.a $(MKLROOT)/lib/intel64/libmkl_core.a -Wl,--end-group -lpthread -lm


ifdef RI
INCS = -I/home/rajbhand/DTCF/DLTC
INCS += -I/home/rajbhand/DTCF
GA_DIR = /home/rajbhand/Applications/ga-5-0/build
INCS += -I$(GA_DIR)/include/

# SCIOTO
SCIOTO_DIR = /home/rajbhand/Applications/scioto/trunk
INCS += -I$(SCIOTO_DIR)/include

CC = mpicxx
LIBS =  -I$(MKLROOT)/include -mkl=parallel -openmp -lpthread -lm
LIBS += -L$(SCIOTO_DIR)/lib -ltc -lm
LIBS += -L$(SCIOTO_DIR)
LIBS += -L$(GA_DIR)/lib -lga -lifcore
FLAGS = -DRI=1 -std=c++0x -O3
endif

ifdef HOPPER
CC = CC
LIBS = -fopenmp
FLAGS = -DHOPPER=1 -std=c++0x -O3
endif

ifdef BGQ
CC = mpixlcxx_r
LIBS = -I/bgsys/drivers/ppcfloor/ -I/soft/libraries/essl/current/include -L/soft/libraries/essl/current/lib64 -lesslsmpbg -L${IBM_MAIN_DIR}/xlf/bg/14.1/bglib64 -lxlf90_r
FLAGS = -std=c++-98x -qlanglvl=extended0x -DBGQ=1 -O3 -qarch=qp -qtune=qp -qsmp=auto
IBMCMP_ROOT=${IBM_MAIN_DIR}
BLAS_LIB=/soft/libraries/alcf/current/xl/BLAS/lib
LAPACK_LIB=/soft/libraries/alcf/current/xl/LAPACK/lib
ESSL_LIB=/soft/libraries/essl/current/essl/5.1/lib64
XLF_LIB=${IBMCMP_ROOT}/xlf/bg/14.1/bglib64
XLSMP_LIB=${IBMCMP_ROOT}/xlsmp/bg/3.1/bglib64
XLMASS_LIB=${IBMCMP_ROOT}/xlmass/bg/7.3/bglib64
MATH_LIBS="-L${XLMASS_LIB} -lmass -L${LAPACK_LIB} -llapack \
	-L${ESSL_LIB} -lesslsmpbg -L${XLF_LIB} -lxlf90_r \
	-L${XLSMP_LIB} -lxlsmp -lxlopt -lxlfmath -lxl -ldl \
	-Wl,--allow-multiple-definition"
#CC = /soft/compilers/wrappers/gcc/mpicxx 
#LIBS = -fopenmp -I/bgsys/drivers/ppcfloor/ -I/soft/libraries/essl/current/include ${MATH_LIBS} 
#LIBS = -I/bgsys/drivers/ppcfloor/ -I/soft/libraries/essl/current/include ${MATH_LIBS} 
#FLAGS = -qlanglvl=extended0x -DBGQ=1 -O3 -qarch=qp -qtune=qp -qsmp=auto
#FLAGS = -DBGQ=1 -std=c++0x -O3 -mcpu=a2 -ffast-math -funroll-loops -ftree-vectorize -fopenmp
endif

ifdef TIMER
	FLAGS+=-DTIMER=$(TIMER)
endif

MAIN = tests/ga_interface/gaintface
LINK = $(CC)

EXE = tests/ga_interface/gaintface.x
OBJS_DLTC = DLTC/dltc_tensor.o DLTC/compute.o DLTC/transp-kernel-omp.o DLTC/exec.o DLTC/ccd-t2.o DLTC/ccsd-t1.o DLTC/ccsd-t2.o DLTC/task.o DLTC/expr.o DLTC/loops.o DLTC/profiler.o DLTC/cache.o DLTC/transp.o DLTC/main.o

OBJS_RRR = tensor.o tensor_ga_api.o redistribute.o grid_redib.o contraction.o helper.o transpose_helper.o localcontract.o block_grid.o grid.o correct.o 



OBJS = $(OBJS_RRR)
OBJS += $(OBJS_DLTC) 
OBJS += $(MAIN).o
ifdef TIMER
FLAGS+=-DTIMER=$(TIMER)
endif


all: $(EXE)

$(EXE): $(OBJS)
	$(LINK) $(FLAGS) -o $@ $^ $(LIBS) 

%.o:%.cc
	$(CC) $(INCS) $(FLAGS) -c -o $@ $^ $(LIBS)

.PHONY : clean
clean:
	rm -rf *.o *.x *.out
	rm -rf DLTC/*.o *.x *.out
