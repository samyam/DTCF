#ifndef _COMPUTE_H_
#define _COMPUTE_H_

#include "timer.h"
#include "define.h"
#include "expr.h"

#if MKL
#include "mkl.h"
#endif

#if LIBSCI
extern "C" {
  void dgemm_(char *transa, char *transb,
      int *m, int *n, int *k, double *alpha, double *a, int *lda,
      double *b, int *ldb, double *beta, double *c, int *ldc);
}
#endif

#if BGQ
extern "C" {
  void dgemm_(char *transa, char *transb,
      int *m, int *n, int *k, double *alpha, double *a, int *lda,
      double *b, int *ldb, double *beta, double *c, int *ldc);
}
#endif
namespace DLTC{
void compute(double *buf_c, double *buf_a, double *buf_b,
    Multiplication& M);
}
#endif
