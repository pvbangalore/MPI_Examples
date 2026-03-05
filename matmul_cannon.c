/* Implementation of Cannon's algorithm */
#include <mpi.h>
#include "cblas.h"

double **allocarray(double **, int, int);

double **matmul(double **a, double **b, double **c, int n, 
                MPI_Comm rowcomm, MPI_Comm colcomm) {
    int P, p, q, k, up, down, right, left;
    MPI_Status status[2];

    MPI_Comm_size(colcomm, &P);
    MPI_Comm_rank(colcomm, &p);
    MPI_Comm_rank(rowcomm, &q);

    /* Perform initial alignment, shift A along rows and B along columns */
    MPI_Cart_shift(rowcomm, 0, -p, &left, &right);
    MPI_Cart_shift(colcomm, 0, -q, &down, &up);
    //    printf("[%d,%d]: right=%d left=%d up=%d down=%d\n", 
    //	   p, q, right, left, up, down);
    MPI_Sendrecv_replace(&a[0][0], n*n, MPI_DOUBLE, right, 2000, 
			 left, 2000, rowcomm, &status[0]);
    MPI_Sendrecv_replace(&b[0][0], n*n, MPI_DOUBLE, up, 2000, 
			 down, 2000, colcomm, &status[1]);

    for (k = 0; k < P; k++) {
      /* Multiply local blocks */
      cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, n, n, n,
		  1.0, &a[0][0], n, &b[0][0], n, 1.0, &c[0][0], n);

      /* Shift A along rowcomm and B along colcomm */
      MPI_Cart_shift(rowcomm, 0, -1, &left, &right);
      MPI_Cart_shift(colcomm, 0, -1, &down, &up);
      //      printf("(%d)[%d,%d]: right=%d left=%d up=%d down=%d\n", 
      //	     k, p, q, right, left, up, down);

      MPI_Sendrecv_replace(&a[0][0], n*n, MPI_DOUBLE, right, 2000, 
			   left, 2000, rowcomm, &status[0]);
      MPI_Sendrecv_replace(&b[0][0], n*n, MPI_DOUBLE, up, 2000, 
			   down, 2000, colcomm, &status[1]);
    }

    /* Realign A and B */
    MPI_Cart_shift(rowcomm, 0, p, &left, &right);
    MPI_Cart_shift(colcomm, 0, q, &down, &up);
    MPI_Sendrecv_replace(&a[0][0], n*n, MPI_DOUBLE, right, 2000, 
			 left, 2000, rowcomm, &status[0]);
    MPI_Sendrecv_replace(&b[0][0], n*n, MPI_DOUBLE, up, 2000, 
			 down, 2000, colcomm, &status[1]);

    return c;
}

