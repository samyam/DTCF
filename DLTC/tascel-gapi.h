#ifndef _TASCEL_GAPI_H_
#define _TASCEL_GAPI_H_

#include <cstdio>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <map>
#include <cmath>

#include "mpi.h"
#include "tascel.h"
using namespace tascel;
using namespace std;

#include "gapi.h"

static ProcGroup* pgrp;
static int gid = 0;
static map<int, GArray<1>*> gmap;

void tascelProgress();
void tascelWait(GArrayRequest *req);
void tascelBarrier();

/*----- GReq ----- */
class GReq {
  public:
    static void initialize() {
      LockGuard<PthreadMutex> lock(mutex);
      reqid.write(0);
    }
    static void finalize() {
      LockGuard<PthreadMutex> lock(mutex);
      assert(reqmap.size() == 0);
    }
    static GArrayRequest* init(int* id) {
      LockGuard<PthreadMutex> lock(mutex);
      GArrayRequest* greq = GArrayRequest::construct();
      int r = reqid.add(1);
      reqmap[r] = greq;
      *id = r;
      return greq;
    }
    static void wait(int id) {
      LockGuard<PthreadMutex> lock(mutex);
      assert(reqmap.find(id) != reqmap.end());
      tascelWait(reqmap[id]);
      destroy(id);
    }

  private:
    static Atomic<int, PthreadMutex> reqid;
    static map<int, GArrayRequest*> reqmap;
    static PthreadMutex mutex;
    static void destroy(int id) {
      delete reqmap[id];
      reqmap.erase(id);    
    }
};

void Tascel_Init();
void Tascel_Finalize();

#endif
