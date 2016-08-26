#ifndef _CC_H_
#define _CC_H_

#include "define.h"
#include "task.h"
#include "expr.h"
#include "exec.h"
#include "gapi.h"

extern vector<DLTC::Multiplication> m;
extern vector<DLTC::Summation> s;
extern vector<DLTC::Tensor> T;

extern int numTensors;
extern int numMuls;
extern int numSums;

extern vector<int> nget;
extern vector<int> ngaget;
extern vector<double> tget;

extern vector<double> Atget;
extern vector<double> Btget;

void ccd_t2();
void ccsd_t1();
void ccsd_t2();

#endif
