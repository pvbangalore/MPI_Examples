/* Implementation of Fox's algorithm */
#include <mpi.h>
#include "cblas.h"

double **allocarray(double **, int, int);
void freearray(double **);

double **matmul(double **a, double **b, double **c, int n, 
                MPI_Comm rowcomm, MPI_Comm colcomm) {
    int p, q, P, Q, k, root, up, down;
    double **t1=NULL;
    MPI_Status status;

    t1 = allocarray(t1, n, n);
    MPI_Comm_size(colcomm, &P);
    MPI_Comm_size(rowcomm, &Q);
    MPI_Comm_rank(colcomm, &p);
    MPI_Comm_rank(rowcomm, &q);

    for (k = 0; k < P; k++) {
      /* Broadcast A along rowcomm starting with the diagonal */
      root = (p + k)%Q;
      if (q == root)
	MPI_Bcast(&a[0][0], n*n, MPI_DOUBLE, root, rowcomm);
      else
	MPI_Bcast(&t1[0][0], n*n, MPI_DOUBLE, root, rowcomm);

      /* Multiply local blocks */
      if (q == root)
	cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, n, n, n,
		    1.0, &a[0][0], n, &b[0][0], n, 1.0, &c[0][0], n);
      else 
	cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, n, n, n,
		    1.0, &t1[0][0], n, &b[0][0], n, 1.0, &c[0][0], n);

      /* Shift/roll B along colcomm */
      MPI_Cart_shift(colcomm, 0, -1, &down, &up);
      // printf("[%d,%d]: k=%d root=%d up=%d down=%d\n", 
      //     p, q, k, root, up, down);
      MPI_Sendrecv_replace(&b[0][0], n*n, MPI_DOUBLE, up, 2000, 
			   down, 2000, colcomm, &status);
    }

    freearray(t1);
    return c;
}

