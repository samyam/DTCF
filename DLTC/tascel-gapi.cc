#include "tascel-gapi.h"

Atomic<int, PthreadMutex> GReq::reqid;
map<int, GArrayRequest*> GReq::reqmap;
PthreadMutex GReq::mutex;

void Tascel_Init() {
  const unsigned numIntraRanks = 1;
  TascelConfig::initialize(numIntraRanks, MPI_COMM_WORLD);
  pgrp   = ProcGroup::construct();
  nprocs = theTwoSided().numProcRanks().toInt();
  me     = theTwoSided().getProcRank().toInt();
  if (!me) cout << "Tascel initialized.\n";
}

void Tascel_Finalize() {
  tascelBarrier();
  delete pgrp;
  pgrp = NULL;
  TascelConfig::finalize();
}

/*----- Support routines -----*/
void tascelProgress() {
  AmListenObjCodelet<NullMutex>* codelet;
  if ((codelet = theAm().amListeners[0]->progress()) != NULL) {
    codelet->execute();
  }
}

void tascelWait(GArrayRequest *req) {
  while (!req->test()) {
    tascelProgress();
  }
}

void tascelBarrier() {
  int epoch = pgrp->signalBarrier();
  while (!pgrp->testBarrier(epoch)) {
    tascelProgress();
  }
}

/*----- GA routines -----*/
int ga_create(DataType type, int dim) {
  tascelBarrier();
  Array<Offset, 1> dims, blocks, proc_grid;
  dims[0] = dim;
  proc_grid[0] = nprocs;
  blocks[0] = dim / nprocs + (dim % nprocs ? 1 : 0);
  size_t dsize = 0;
  if (type == CINT) dsize = sizeof(int);
  if (type == CDBL) dsize = sizeof(double);
  assert(dsize != 0);
  GArray<1>* ga = new GArray<1>(dims, blocks, proc_grid, kBlocked, kRowMajor, dsize);
  gmap[++gid] = ga;
  return gid;
}

void ga_zero(int gid){
  tascelBarrier();
  assert(gmap.find(gid) != gmap.end());
  GArray<1>* ga = gmap[gid];
  ga->zero();
  tascelBarrier();
}

void ga_nbwait(int handle) {
  GReq::wait(handle);
}

void ga_nbget(int id, int lo, int hi, void *buf, int& handle) {
  assert(gmap.find(id) != gmap.end());
  GArray<1>* ga = gmap[id];
  Box<1> gbox, localFullBox, lbox;
  int lh, rh;
  GArrayRequest *lreq = GReq::init(&lh);
  GArrayRequest *rreq = GReq::init(&rh);
  gbox.lo()[0] = lo;
  gbox.hi()[0] = hi + 1;
  lbox.lo()[0] = 0;
  lbox.hi()[0] = hi - lo + 1;
  localFullBox.lo()[0] = 0;
  localFullBox.hi()[0] = lbox.hi()[0] - lbox.lo()[0];
  ga->get(gbox, buf, localFullBox, lbox, lreq, rreq);
  GReq::wait(lh);
  //GReq::wait(rh);
  handle = rh;
}

void ga_get(int id, int lo, int hi, void *buf) {
  int rh;
  ga_nbget(id, lo, hi, buf, rh);
  ga_nbwait(rh);
}

void ga_nbput(int id, int lo, int hi, void *buf, int& handle) {
  assert(gmap.find(id) != gmap.end());
  GArray<1>* ga = gmap[id];
  Box<1> gbox, localFullBox, lbox;
  int lh, rh;
  GArrayRequest *lreq = GReq::init(&lh);
  GArrayRequest *rreq = GReq::init(&rh);
  gbox.lo()[0] = lo;
  gbox.hi()[0] = hi + 1;
  lbox.lo()[0] = 0;
  lbox.hi()[0] = hi - lo + 1;
  localFullBox.lo()[0] = 0;
  localFullBox.hi()[0] = lbox.hi()[0] - lbox.lo()[0];
  ga->put(gbox, buf, localFullBox, lbox, lreq, rreq);
  GReq::wait(lh);
  //GReq::wait(rh);
  handle = rh;
}

void ga_put(int id, int lo, int hi, void* buf) {
  int rh;
  ga_nbput(id, lo, hi, buf, rh);
  ga_nbwait(rh);
}

void ga_nbacc(int id, int lo, int hi, void* buf, int& handle) {
  assert(gmap.find(id) != gmap.end());
  GArray<1> *ga = gmap[id];
  Box<1> gbox, localFullBox, lbox;
  int lh, rh;
  GArrayRequest *lreq = GReq::init(&lh);
  GArrayRequest *rreq = GReq::init(&rh);
  gbox.lo()[0] = lo;
  gbox.hi()[0] = hi + 1;
  lbox.lo()[0] = 0;
  lbox.hi()[0] = hi - lo + 1;
  localFullBox.lo()[0] = 0;
  localFullBox.hi()[0] = lbox.hi()[0] - lbox.lo()[0];
  ga->acc<Rma::IntSum>(gbox, buf, localFullBox, lbox, lreq, rreq);
  GReq::wait(lh);
  handle = rh;
}

void ga_acc(int id, int lo, int hi, void *buf) {
  int rh;
  ga_nbacc(id, lo, hi, buf, rh);
  ga_nbwait(rh);
}

void Barrier() {
  tascelBarrier();
}
