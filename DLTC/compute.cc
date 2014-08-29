#include "compute.h"
namespace DLTC{
void compute(double *buf_c, double *buf_a, double *buf_b,
    Multiplication& M) {
#if TIMER
  MyTimer::start(DGM);
#endif

  int m = (int)getSize(M.m_ids());
  int n = (int)getSize(M.n_ids());
  int k = (int)getSize(M.k_ids());

#if MKL
  cblas_dgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,m,n,k,
      1.0, buf_a, k, buf_b, n, 1.0, buf_c, n);
  // ex: iajb += iakl * kljb  lda=kl ldb=jb ldc=jb
#endif

#if LIBSCI
  char nt = 'N';
  double done = 1.0;
  dgemm_(&nt,&nt,&n,&m,&k,
      &done,buf_b,&n,buf_a,&k,&done,buf_c,&n);
#endif

#if BGQ
  char nt = 'N';
  double done = 1.0;
  dgemm_(&nt,&nt,&n,&m,&k,
      &done,buf_b,&n,buf_a,&k,&done,buf_c,&n);
#endif

#if TIMER
  MyTimer::stop(DGM);
#endif
}
}
