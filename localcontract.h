#if RI
#include "mkl.h"
#endif

#if HOPPER || BGQ
extern "C" {
  void dgemm_(char* transa, char* transb,
      int* m, int* n, int* k, double* alpha, double* a, int* lda,
      double* b, int* ldb, double* beta, double* c, int* ldc);
}
#endif

extern void local_contraction(int n_a, int n_b, int n_k, 
    double* A,double* B,double* C,int at,int bt,double alpha);

