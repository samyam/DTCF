ifdef RI
FC = mpif90
CC = mpicxx
FLAGS += -DMKL=1 -mkl 
FLAGS += -DOMP=1 -openmp
FLAGS += -O3 -std=c++0x #-std=gnu99 #-std=gnu++0x #-std=gnu99 -O3 #-DUSE_MPI -DMPI
endif

ifdef TASCEL
TASCEL_DIR = /home/laip/tascel-ctce/build
INCS += -I$(TASCEL_DIR)/include/ -I$(TASCEL_DIR)/include/tascel 
LIBS += -L$(TASCEL_DIR)/lib -ltascel
FLAGS += -DTASCEL=1
else
# GA
GA_DIR = /home/rajbhand/Applications/ga-5-0/build
INCS = -I$(GA_DIR)/include/
LIBS = -L$(GA_DIR)/lib -lga -lifcore
# SCIOTO
SCIOTO_DIR = /home/rajbhand/Applications/scioto/trunk
INCS += -I$(SCIOTO_DIR)/include
LIBS += -L$(SCIOTO_DIR)/lib -ltc -lm
LIBS += -L$(SCIOTO_DIR)
endif

ifdef BGQ
# compiler
CC  = mpixlcxx_r
CXX = mpixlcxx_r
FC  = mpixlf77_r
# GA
INCS = -I/home/stock/laip/ga-5-0/build/include/
LIBS = -L/home/stock/laip/ga-5-0/build/lib -lga
# SCIOTO
INCS += -I/home/stock/laip/scioto/trunk/include
LIBS += -L/home/stock/laip/scioto/trunk/lib -ltc -lm
# ESSL
INCS += -I/bgsys/drivers/ppcfloor/ -I/soft/libraries/essl/current/include
LIBS += -L/soft/libraries/essl/current/lib64 -L/soft/compilers/ibmcmp-nov2012/xlf/bg/14.1/bglib64
LIBS += -lesslsmpbg -lxlsmp -lxlfmath -lxlf90_r
FLAGS += -O3 -qarch=qp -qtune=qp -qsmp=auto -DBGQ=1
# FLAGS += -DOMP=1 -openmp # will automatically take care of it
FLAGS += -std=gnu99 -O3 #-DUSE_MPI -DMPI
endif

LINK = $(CC)

EXE  = main.x
OBJS = main.o dltc_tensor.o compute.o transp-kernel-omp.o exec.o ccd-t2.o ccsd-t1.o ccsd-t2.o task.o expr.o loops.o profiler.o cache.o transp.o

ifdef TASCEL
OBJS += tascel-gapi.o
else
OBJS += scioto-gapi.o
endif

ifdef DEBUG
FLAGS+=-DDEBUG=$(DEBUG)
endif

ifdef CACHING
FLAGS+=-DCACHING=$(CACHING)
endif

ifeq ($(CHECK),1)
FLAGS+=-DCHECK=$(CHECK) -DCCD=0 -DCCSD=0
OBJS += checkerS.o checkerM.o
else
FLAGS+=-DCCD=1 -DCCSD=1
endif

ifeq ($(VERIFY),1)
FLAGS+=-DCHECK=$(VERIFY) -DVERIFY=$(VERIFY) -DCCD=0 -DCCSD=0
OBJS += checkerS.o checkerM.o
endif

ifdef TIMER
FLAGS+=-DTIMER=$(TIMER)
endif
ifdef CTX
FLAGS+=-DCTX=$(CTX)
endif

$(EXE): $(OBJS)
	$(LINK) $(FLAGS) -o $@ $^ $(LIBS)

%.o:%.cc
	$(CC) $(FLAGS) -c -o $@ $^ $(INCS)

.PHONY : clean
clean:
	rm -rf *.o *.x
