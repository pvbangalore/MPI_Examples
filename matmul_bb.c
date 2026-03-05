/* Implementation of broadcast-broadcast algorithm */
#include <mpi.h>
#include "cblas.h"

double **allocarray(double **, int, int);
void freearray(double **);

double **matmul(double **a, double **b, double **c, int n, 
                MPI_Comm rowcomm, MPI_Comm colcomm) {
    int p, q, P, Q, k;
    double **t1=NULL, **t2=NULL;

    t1 = allocarray(t1, n, n);
    t2 = allocarray(t2, n, n);
    MPI_Comm_size(colcomm, &P);
    MPI_Comm_size(rowcomm, &Q);
    MPI_Comm_rank(colcomm, &p);
    MPI_Comm_rank(rowcomm, &q);

    for (k = 0; k < P; k++) {
      /* Broadcast A along rowcomm */
      if (q == k) 
	MPI_Bcast(&a[0][0], n*n, MPI_DOUBLE, k, rowcomm);
      else
	MPI_Bcast(&t1[0][0], n*n, MPI_DOUBLE, k, rowcomm);

      /* Broadcast B along colcomm */
      if (p == k) 
	MPI_Bcast(&b[0][0], n*n, MPI_DOUBLE, k, colcomm);
      else
	MPI_Bcast(&t2[0][0], n*n, MPI_DOUBLE, k, colcomm);
	
      /* Multiply local blocks */
      if ((p == k) && (q == k))
	cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, n, n, n,
		    1.0, &a[0][0], n, &b[0][0], n, 1.0, &c[0][0], n);
      else if (q == k)
	cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, n, n, n,
		    1.0, &a[0][0], n, &t2[0][0], n, 1.0, &c[0][0], n);
      else if (p == k)
	cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, n, n, n,
		    1.0, &t1[0][0], n, &b[0][0], n, 1.0, &c[0][0], n);
      else
	cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, n, n, n,
		    1.0, &t1[0][0], n, &t2[0][0], n, 1.0, &c[0][0], n);
    }

    freearray(t1);
    freearray(t2);

    return c;
}

