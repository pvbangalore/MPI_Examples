#!/bin/bash
module load openmpi/4.1.7-gcc12
mpiexec ./matmul_cannon 1000 2 2 3
mpiexec ./matmul_cannon 1000 2 2 3
mpiexec ./matmul_cannon 1000 2 2 3
mpiexec ./matmul_fox 1000 2 2 3
mpiexec ./matmul_fox 1000 2 2 3
mpiexec ./matmul_fox 1000 2 2 3
mpiexec ./matmul_bb 1000 2 2 3
mpiexec ./matmul_bb 1000 2 2 3
mpiexec ./matmul_bb 1000 2 2 3
