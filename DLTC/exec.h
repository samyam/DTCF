#ifndef _EXEC_H_
#define _EXEC_H_

#include "define.h"
#include "nxtask.h"
#include "dltc_tensor.h"
#include "loops.h"
#include "expr.h"
#include "transp.h"
#include "compute.h"
#include "task.h"

namespace DLTC{
#if TASCEL
void exec(ExprType et, int id, TaskPool* tp);
#else
void exec(ExprType et, int id, TaskPool tp);
#endif
}
#endif
