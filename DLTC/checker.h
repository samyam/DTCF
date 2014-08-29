#ifndef _CHECKER_H_
#define _CHECKER_H_

#include "define.h"
#include "nxtask.h"
#include "dltc_tensor.h"
#include "loops.h"
#include "expr.h"
#include "transp.h"
#include "compute.h"
#include "task.h"
#include "exec.h"
#include "gapi.h"

#define PRECISION 0.00000001
inline bool check_diff(int size, double *x, double *y) {
  for (int i = 0; i < size; i++) {
    if (abs(x[i] - y[i]) > PRECISION) {
      printf("%d: %.2f ?= %.2f\n", i, x[i], y[i]);
      return false;
    }
  }
  return true;
}

inline void print(double *m, char *str, int size, int w) {
#if DEBUG
  printf("%s",str);
  for(int i=0;i<size;i++) {
    printf("%.5lf\t",m[i]);
    if (i%w==w-1) printf("\n");
  }
  printf("\n");
#endif
}

void checkS(int id);
void checkM(int id);

#endif
