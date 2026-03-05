/* Driver program to call the parallel matrix-matrix multiplication
 * functions. Uses MPI cartesian topology functions to create a process
 * grid and create row and column communicators for communication.
 *
 * This program uses the BLAS library for the DGEMM function to perform
 * local matrix-matrix multiplication. 
 *
 * See Makefile for instructions on specific include paths and library
 * paths to include for compilation and linking. 
 *
 * To Run: mpiexec -n <# of processes> <exec> <MatrixSize> <P> <Q> <flag>
 *         where P X Q = # of processes
 *         mpiexec -n 4 ./matmul_cannon 100 2 2 3
 * 
 */
#include <stdio.h>
#include <stdlib.h> 
#include <mpi.h>
#include "cblas.h"

#define N_DIMS 2

typedef enum{FALSE, TRUE} BOOLEAN;

double **matmul(double**, double**, double**, int, MPI_Comm, MPI_Comm);

double **allocarray(double **a, int P, int Q) {
  int i;
  double *p;
  
  p = (double *)malloc(P*Q*sizeof(double));
  a = (double **)malloc(P*sizeof(double*));

  if (p == NULL || a == NULL) 
    printf("Error allocating memory\n");

  /* for row major storage */
  for (i = 0; i < P; i++)
    a[i] = &p[i*Q];
  
  return a;
}

/* function to delete the 2-D array */
void freearray(double **a) {
  free(&a[0][0]);
  free(a);
}

double **initarray(double **a, int mrows, int ncols, double value) {
  int i,j;
  double drand48();

  for (i=0; i<mrows; i++)
    for (j=0; j<ncols; j++)
      a[i][j] = drand48()*value;
  
  return a;
}

void printarray(double **a, int mrows, int ncols, int p, int q, int k) {
  int i,j;
  
  for (i=0; i<mrows; i++) {
    printf("(%d)[%d,%d]: ", k, p, q);
    for (j=0; j<ncols; j++)
      printf("%f ", a[i][j]);
    printf("\n");
  }
}

/* Initialize local matrices A and B */
void init_data(double **a, double **b, int N, int n, int p, int q) {
    int  i, j;
    double astart;
    
    astart = (double)p*n*N + (double)q*n; /* assign A row-wise */
    for (i = 0; i < n; i++)
      for (j = 0; j < n; j++) 
        a[i][j] = (double)(astart + (double)i*N + (double)j + 1.0);

    b = initarray(b, n, n, (double)0.0); /* set B to I */
    if (p == q)
      for (i=0; i<n; i++)
	b[i][i] = (double)1.0;
}

/* Check if C = A*I */
void check_data(double **c, double **a, int n, int p, int q) {
  int i, j;

  for (i=0; i<n; i++)
    for (j=0; j<n; j++)
      if (c[i][j] != a[i][j])
	printf("[%d,%d]: c[%d][%d] = %lf a[%d][%d] = %lf\n", 
	       p, q, i, j, c[i][j], i, j, a[i][j]);
}

/* Create a 2-D cartesian topology */
void create_2dgrid(int P, int Q, MPI_Comm comm_in, MPI_Comm *comm_2d, 
		   MPI_Comm *row_comm, MPI_Comm *col_comm) {
    int dims[N_DIMS],         /* number of dimensions */
        period[N_DIMS],       /* aperiodic flags */
        remain_dims[N_DIMS];  /* sub-dimension computation flags */
    int reorder;

    /* Generate a new communicator with virtual topology */
    dims[0] = P;
    dims[1] = Q;
    reorder = TRUE;
    period[0] = period[1] = TRUE;
    MPI_Cart_create(comm_in, N_DIMS, dims, period, reorder, comm_2d);

    /* Get row and column communicators using cartesian sub-topology */
    remain_dims[0] = FALSE; 
    remain_dims[1] = TRUE;
    MPI_Cart_sub(*comm_2d, remain_dims, row_comm);

    remain_dims[0] = TRUE; 
    remain_dims[1] = FALSE;
    MPI_Cart_sub(*comm_2d, remain_dims, col_comm);
}

int main(int argc, char **argv) 
{
    int rank, size, N, n, P, Q, flag, p, q, local[N_DIMS];
    double **a=NULL, **b=NULL, **c=NULL;
    double starttime, endtime;
    MPI_Comm comm2d, rowcomm, colcomm;

    /* Initialize */
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 5) {
      printf("Usage: %s <N> <P> <Q> <flag>\n", argv[0]);
      MPI_Abort(MPI_COMM_WORLD,-1);
    }
    
    N = atoi(argv[1]);
    P = atoi(argv[2]);
    Q = atoi(argv[3]);
    flag = atoi(argv[4]);
    
    if (P*Q != size) {
      printf("Process grid layout %d X %d != %d\n", P, Q, size); 
      MPI_Abort(MPI_COMM_WORLD,-1);
    }

    /* Compute size of local matrices, assume equal distribution */
    n = N/P;
    if (rank == 0)
      printf("Input: P=%d Q=%d N=%d n=%d\n", P, Q, N, n);

    /* Create a 2-D cartesian topology */
    create_2dgrid(P, Q, MPI_COMM_WORLD, &comm2d, &rowcomm, &colcomm);

    /* Determine the position in the grid */
    MPI_Cart_coords(comm2d, rank, N_DIMS, local);
    p = local[0]; 
    q = local[1];

    /* Allocate memory for all three matrices and temporary arrays */
    a = allocarray(a, n, n);
    b = allocarray(b, n, n);
    c = allocarray(c, n, n);
    
    /* Initialize the matrices */
    if (flag == 1)  /* A is random, B = I */
      init_data(a, b, N, n, p, q);
    else if (flag == 2)  /* A = I, B is random */
      init_data(b, a, N, n, p, q);
    else {      /* both A and B are random */
      a = initarray(a, n, n, (double)(rank+1));
      b = initarray(b, n, n, (double)(rank+1));
    }
    c = initarray(c, n, n, (double)0.0);

    /* Perform matrix multiplication */
    MPI_Barrier(MPI_COMM_WORLD);
    starttime = MPI_Wtime();
    c = matmul(a, b, c, n, rowcomm, colcomm);
    endtime = MPI_Wtime() - starttime;
    MPI_Reduce(&endtime, &starttime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    /* Check if the values are correct */
    if (flag == 1)
      check_data(c, a, n, p, q);
    else if (flag == 2) 
      check_data(c, b, n, p, q);

    if (rank == 0) 
      printf("Time taken for N=%d P=%d Q=%d: %f\n", N, P, Q, starttime);

#ifdef DEBUG_PRINT
    printarray(a, n, n, p, q, rank);
    printf("\n");
    printarray(b, n, n, p, q, rank);
    printf("\n");
    printarray(c, n, n, p, q, rank);
#endif

    freearray(a);
    freearray(b);
    freearray(c);

    /* Quit */
    MPI_Finalize();

    return 0;
}
