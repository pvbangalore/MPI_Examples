# MPI_Examples
## Different versions to distribute (scatter data) using blocking, nonblocking, and collective functions

`sample1.c` - uses blocking send and recv (has a bug)

`sample2.c` - uses nonblocking send and blocking recv (has a bug)

`sample3.c` - nonblocking send and recv (posts nonblocking recv first)

`sample4.c` - uses nonblocking send and blocking recv; avoids sending and receiving data to/from process with rank 0 by using memcpy to copy data locally

`sample5.c` - uses MPI_Scatter collective function

## Different versions of mpi_vec_sum

`mpi_vec_sum1.c` - allocates and initializes vector locally

`mpi_vec_sum2.c` - allocates and initializes vector on process with rank 0 and uses MPI_Scatter to distribute the vector

`mpi_vec_sum3.c` - allocates and initializes vector on process with rank 0 and uses MPI_Scatterv to distribute the vector

## Blocking and nonblocking versions to implement a simple broadcast function

`blocking.c` - uses blocking send and receive functions to implement a simple broadcast

`nonblocking.c` - uses nonblocking send and receive functions to implement a simple broadcast

## Instructions for compiling and running the programs
Here are few important steps that you MUST follow in order to compile and run MPI programs on the ASA Cluster.

1. Make sure to add `module load openmpi/4.1.7-gcc12` in the file `~/.bashrc.local.asax` (make sure that this line added at the end of this file) and remove or comment out the line `module load intel`.
2. Remember to logout and login again after adding the line to the `~/.bashrc.local.asax` file (otherwise, you have to type `module load openmpi/4.1.7-gcc12` at the command prompt in your shell)
3. If you have completed the above steps and login to the ASA cluster, if you type `which mpicc` you should get the output that looks like this:
```
/apps/x86-64/apps/openmpi_4.1.7_gcc12/bin/mpicc
```
4. Compile the MPI programs from the textbook or the sample program I have provided using mpicc, for example:
```
mpicc -O -Wall -o mpi_vec_sum mpi_vec_sum.c
```
5. Create a job submission script, say, `myscript.sh`, and make sure to enter the following:
```
#!/bin/bash
module load module load openmpi/4.1.7-gcc12
mpiexec ./mat_vec_sum 500000
mpiexec ./mat_vec_sum 500000
mpiexec ./mat_vec_sum 500000
```
6. The only change you should make in the above file would be to change the name of the executable and other argument you provide to your program. Otherwise, the first two lines should not be changed.
7. Change the file permissions to have execute permission using the command:
```
chmod +x myscript.sh
```
9. Use `run_script_mpi` to submit the job and make sure to choose the `class` queue and request `number of cores = number of processes.`
