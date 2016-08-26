#include "localcontract.h" 

void local_contraction(int n_a, int n_b, int n_k, double* A,double* B,double* C,int at,int bt,double alpha)
{

#if RI
    cblas_dgemm(CblasRowMajor, (at==0) ? CblasNoTrans : CblasTrans, 
		(bt==0) ? CblasNoTrans : CblasTrans,
		n_a, n_b, n_k, alpha, A, n_k, B, n_b, 1.0, C, n_b);
#endif

#if HOPPER || BGQ
  char nt = 'N';
  char  t = 'T';
  double done = 1.0;
  dgemm_((bt==0)? &nt : &t, (at==0)? &nt : &t, 
    &n_b, &n_a, &n_k, &alpha, B, &n_b, A, &n_k, &done, C, &n_b);
#endif

}


