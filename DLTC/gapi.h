#ifndef _GAPI_H_
#define _GAPI_H_

enum DataType { CINT, CDBL };
extern int me, nprocs;

int  ga_create(DataType type, int dim);
void ga_zero(int gid);
void ga_nbwait(int nbhandle);
void ga_nbget(int id, int lo, int hi, void* buf, int& handle);
void ga_get(int id, int lo, int hi, void* buf);
void ga_nbput(int id, int lo, int hi, void* buf, int& handle);
void ga_put(int id, int lo, int hi, void* buf);
void ga_nbacc(int id, int lo, int hi, void* buf, int& handle);
void ga_acc(int id, int lo, int hi, void* buf);

void Barrier();

#endif
