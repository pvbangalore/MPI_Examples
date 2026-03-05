#  Simple Makefile for compiling C/C++ and Fortran Programs

CC       = mpicc
# use the lines below for Intel compiler on ASAX
MKL_DIR  = /apps/x86-64/apps/intel_2023.1.0/mkl/latest/
CFLAGS   = -O3 -Wall -I${MKL_DIR}/include #-DDEBUG_PRINT # uncomment for debugging
# also make use to manually set a softlink to cblas.h in the source directory
# using the following command:
# ln -s /apps/x86-64/apps/intel_2023.1.0/mkl/latest/include/mkl_cblas.h cblas.h 
CXX      = mpic++
CXXFLAGS = ${CFLAGS} #-DDEBUG_PRINT #uncomment for debugging

OBJS     = mpi_matmul.o
# use the line below for Intel compiler on ASAX
LIBS     = ${MKL_DIR}/lib/intel64/libmkl_intel_lp64.a ${MKL_DIR}/lib/intel64/libmkl_sequential.a ${MKL_DIR}/lib/intel64/libmkl_core.a -lm
EXECS    = matmul_cannon matmul_bb matmul_fox 

.SUFFIXES: .o .c .cc

all:    $(EXECS)

matmul_cannon:   mpi_matmul.o matmul_cannon.o
	$(CC) $(CFLAGS) -o $@ $@.o ${OBJS} ${LIBS}

matmul_bb:   mpi_matmul.o matmul_bb.o
	$(CC) $(CFLAGS) -o $@ $@.o ${OBJS} ${LIBS}

matmul_fox:   mpi_matmul.o matmul_fox.o
	$(CC) $(CFLAGS) -o $@ $@.o ${OBJS} ${LIBS}

%.o:   %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o:  %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	/bin/rm -f *.o *~ $(EXECS)


# DO NOT DELETE
