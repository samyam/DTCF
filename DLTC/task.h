#ifndef _TASK_H_
#define _TASK_H_

#include "define.h"
#include "nxtask.h"
#include "dltc_tensor.h"
#include "loops.h"
#include "expr.h"
#include "transp.h"
#include "compute.h"
#include "mpi.h"
namespace DLTC{
struct TaskLocal {
  int local;
};

struct TaskDesc {
  ExprType et;    // expression type
  int id;         // expression id
  int itlb[NIDX]; // iterator lowerbound
  int itub[NIDX]; // iterator upperbound
  int idvm[NIDX]; // original idvm description
};

#if TASCEL
void createTask(TaskPool* tp, 
    ExprType et, int id, const vector<int>& itlb, const vector<int>& itub);

void TaskFunc(TaskPool* tp,
    void *_bigd, int bigd_len,
    void *pldata, int pldata_len, vector<void *> data_bufs,
    int thd);
#else
void createTask(TaskPool tp,
    ExprType et, int id, const vector<int>& itlb, const vector<int>& itub);

void TaskFunc(TaskPool tp, task_t* task);
#endif
}
#endif
