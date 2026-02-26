/* Sample program to illusrate how to distribute an array using 
   nonblocking send routines and blocking receive.

   This program has a bug that will cause the program to deadlock. 
   What is the issue?
   
   Author: Purushotham V. Bangalore
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <mpi.h>

/* allocate row-major two-dimensional array */
int **allocarray(int P, int Q) {
  int i;
  int *p, **a;

  p = (int *)malloc(P*Q*sizeof(int));
  a = (int **)malloc(P*sizeof(int*));
  for (i = 0; i < P; i++)
    a[i] = &p[i*Q]; 

  return a;
}

/* initialize the array */
void initarray(int **a, int M, int N) {
  for (int i=0; i<M; i++)
      for (int j=0; j<N; j++)
          a[i][j] = i*N + j + 1;
}

/* check value received */
void checkarray(int **a, int M, int N, int myrank) {
  int offset = myrank*M;
  for (int i=0; i<M; i++)
      for (int j=0; j<N; j++)
          assert(a[i][j] == ((offset+i)*N + j + 1));
}

/* free allocated memory */
void freearray(int **a) {
  free(&a[0][0]);
  free(a);
}

int main(int argc, char **argv) {
  int rank, size;
  int M, N, myM, i, offset;
  int **image, **myimage;
  MPI_Request *req;

  if (argc != 3) {
    printf("Usage: %s <#rows> <#cols>\n", argv[0]);
    MPI_Abort(MPI_COMM_WORLD, -1);
  }
  M = atoi(argv[1]);
  N = atoi(argv[2]);

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  myM = M / size;
  image = allocarray(M, N);
  initarray(image, M, N);
  myimage = allocarray(myM, N);
  req = (MPI_Request *)malloc(size*sizeof(MPI_Request));

  if (rank == 0) {
    for (i = 0, offset = 0; i < size; i++, offset += myM)
      MPI_Isend(&image[offset][0], myM*N, MPI_INT, i, 0, MPI_COMM_WORLD, &req[i]);
    MPI_Waitall(size, req, MPI_STATUSES_IGNORE);
  } 
  MPI_Recv(&myimage[0][0], myM*N, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  checkarray(myimage, myM, N, rank);
  printf("[%d]: Program terminated normally\n", rank);

  freearray(image);
  freearray(myimage);
  MPI_Finalize();

  return 0;
}

