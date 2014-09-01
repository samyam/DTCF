#ifndef _DTENSOR_H_
#define _DTENSOR_H_

#include <cassert>
#include <mpi.h>
#include <map>

#include "define.h"
#include "timer.h"
#include "memory.h"
#include "loops.h"

#if TASCEL
#include "tascel-gapi.h"
#endif

extern int me, nprocs;

namespace DLTC
{

class Tensor {
  private:
    vector<IndexName> ids_;
    vector<IndexType> idt_; /* indices order = data layout */
    vector<int> ranges_;    /* range of indices  (ranges[x]=wbs[x]*sps[x]) */
    vector<int> wbs_;       /* tilewidth of indices */
    vector<int> sps_;       /* number of tiles of indices */
    int handle_;            /* ga handle */
    int tensorSize_;
    int tileSize_;
    int dim_;
    int id_;

    int nget_;
    int ngaget_;
    int nacc_;
    int ngaacc_;
    double tget_;

  public:
    Tensor() {};
    ~Tensor() {};
    Tensor(const vector<IndexType>& idt, int handle, int id);

    inline int id()         { return id_;  }
    inline int dim()        { return dim_; }
    inline int handle()     { return handle_; }
    inline int tensorSize() { return tensorSize_; }
    inline int tileSize()   { return tileSize_; }
    inline const vector<int>& wbs() { return wbs_; }
    inline const vector<int>& sps() { return sps_; }
    inline const vector<IndexName>& ids() { return ids_; }

    inline int nGET() { return nget_; } // number of get of this tensor
    inline int nACC() { return nacc_; } // number of get of this tensor
    inline int nGAGET() { return ngaget_; } // number of get of this tensor
    inline int nGAACC() { return ngaacc_; } // number of get of this tensor
    inline double tGET() { return tget_; } // time of get of this tensor

    void getData(double* buf, int bufSize); // get entire tensor
    void putData(double* buf, int bufSize); // put entire tensor
    void clrData();

    void getTile(double*& buf, int bufSize, const vector<int>& idv, bool& inCache);
    void getTile2(double*& buf, int bufSize, const vector<int>& idvm, bool& inCache);

    void accTile(double* buf, int bufSize, const vector<int>& idv, double coef);
    void accTile2(double* buf, int bufSize, const vector<int>& idvm, double coef);

    void setIndexName(const vector<IndexName>& ids);
};


Tensor Tensor2(IndexType t1, IndexType t2, int id);
Tensor Tensor4(IndexType t1, IndexType t2, IndexType t3, IndexType t4, int id);

void setIndexName2(Tensor& t, IndexName i1, IndexName i2);
void setIndexName4(Tensor& t, IndexName i1, IndexName i2, IndexName i3, IndexName i4);
}
#endif
