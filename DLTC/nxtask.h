#ifndef _NXTASK_H_
#define _NXTASK_H_

#if CTX

#include "ga.h"
#include "macdecls.h"

inline int create_counter() {
  int dims[] = {1};
  int chunk[] = {-1};
  int ctx = NGA_Create(C_INT, 1, dims, "counter", chunk);
  GA_Zero(ctx);
  return ctx;
}

inline void destroy_counter(int ctx) {
  GA_Destroy(ctx);
}

inline int next_task(int ctx) {
  int zero = 0;
  int next = NGA_Read_inc(ctx, &zero, 1);
  return next;
}

#endif

#endif
