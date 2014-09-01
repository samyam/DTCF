#include "dltc_tensor.h"
#include "profiler.h"
#include "cache.h"

extern DLTC::LRUCache Cache;
extern int nget2, nhit2;
extern int nget4, nhit4;
extern vector<int> nget;
extern vector<int> ngaget;
extern vector<double> tget;
extern double traffic;

namespace DLTC{
int allocGA(const vector<IndexType>& idt) {
  assert(idt.size() > 0);
  int dim = idt.size();
#if TASCEL
  vector<int> range(dim, 0);
  for (int i = 0; i < dim; i++) {
    range[i] = idt[i] == HOL ? O : V;
  }
  int totalSize = 1;
  for (int i = 0; i < dim; i++) totalSize *= range[i];
  int handle = ga_create(CDBL, totalSize);
  ga_zero(handle);
#else
  //handle = NGA_Create(C_DBL, 1, totalSize, "NONAME", chunk); // 1-d NGA
  int range[dim];
  for (int i = 0; i < dim; i++) {
    range[i] = idt[i] == HOL ? O : V;
  }
  int chunk[dim];
  for (int i = 0; i < dim; i++) {
    chunk[i] = 1;
  }
  int handle = NGA_Create(C_DBL, dim, range, "NONAME", chunk);
  GA_Zero(handle);
#endif
  return handle;
}

Tensor Tensor2(IndexType i1, IndexType i2, int id) {
  vector<IndexType> vt;
  vt.push_back(i1);
  vt.push_back(i2);
  int hdl = allocGA(vt);
  Tensor t = Tensor(vt, hdl, id);
  return t;
};

void setIndexName2(Tensor& t, IndexName i1, IndexName i2) {
  assert(t.dim() == 2);
  vector<IndexName> vi;
  vi.push_back(i1);
  vi.push_back(i2);
  t.setIndexName(vi);
};

Tensor Tensor4(IndexType i1, IndexType i2, IndexType i3, IndexType i4, int id) {
  vector<IndexType> vt;
  vt.push_back(i1);
  vt.push_back(i2);
  vt.push_back(i3);
  vt.push_back(i4);
  int hdl = allocGA(vt);
  Tensor t = Tensor(vt, hdl, id);
  return t;
};

void setIndexName4(Tensor& t, IndexName i1, IndexName i2, IndexName i3, IndexName i4) {
  assert(t.dim() == 4);
  vector<IndexName> vi;
  vi.push_back(i1);
  vi.push_back(i2);
  vi.push_back(i3);
  vi.push_back(i4);
  t.setIndexName(vi);
};

Tensor::Tensor(const vector<IndexType>& idt, int handle, int id)
  : idt_(idt), handle_(handle), id_(id) {
    assert(idt_.size() > 0);
    dim_ = idt_.size();
    ranges_.resize(dim_);
    wbs_.resize(dim_);
    sps_.resize(dim_);
    for (int i = 0; i < dim_; i++) {
      if (idt_[i] == HOL) {
        ranges_[i] = O;
        wbs_[i] = wbO;
        sps_[i] = spO;
      }
      else if (idt_[i] == PAR) {
        ranges_[i] = V;
        wbs_[i] = wbV;
        sps_[i] = spV;
      }
      else assert(1);
    }
    // compute tensor size and tilesize
    tensorSize_ = 1;
    tileSize_ = 1;
    for (int i = 0; i < dim_; i++) {
      tensorSize_ *= ranges_[i];
      tileSize_ *= wbs_[i];
    }

    nget_ = 0;
    nacc_ = 0;
    ngaget_ = 0;
    ngaacc_ = 0;
    tget_ = 0;
  }

/* tensor-level operations for correctness checking */
void Tensor::getData(double* buf, int bufSize) {
  assert(bufSize == tensorSize_);
//  if (!me) cout << "get start";
#if TASCEL
  int lo = 0;
  int hi = bufSize - 1;
  ga_get(handle_, lo, hi, buf);
#else
  int lo[dim_];
  int hi[dim_];
  int ld[dim_];
  for (int i = 0; i < dim_; i++) {
    lo[i] = 0;
    hi[i] = ranges_[i] - 1;
  }
  for (int i = 1; i < dim_; i++) {
    ld[i - 1] = ranges_[i];
  }
  NGA_Get(handle_, lo, hi, buf, ld);
#endif
//  if (!me) cout << "get finish";
}

void Tensor::putData(double* buf, int bufSize) {
  assert(bufSize == tensorSize_);
#if TASCEL
  int lo = 0;
  int hi = bufSize - 1;
  ga_put(handle_, lo, hi, buf);
#else
  int lo[dim_];
  int hi[dim_];
  int ld[dim_];
  for (int i = 0; i < dim_; i++) {
    lo[i] = 0;
    hi[i] = ranges_[i] - 1;
  }
  for (int i = 1; i < dim_; i++) {
    ld[i - 1] = ranges_[i];
  }
  NGA_Put(handle_, lo, hi, buf, ld);
#endif
}

void Tensor::clrData() {
#if TASCEL
  ga_zero(handle_);
#else
  GA_Zero(handle_);
#endif
}

/* tile-level operations */
int computeTileIndex(const vector<int>& idv, const vector<int>& spv) {
  assert(idv.size() == spv.size());
  int index = 0;
  int stride = 1;
  int dim = idv.size();
  for (int i = dim - 1; i >= 0; i--) {
    index += idv[i] * stride;
    stride *= spv[i];
  }
  return index;
}

void Tensor::getTile(double*& buf, int bufSize, const vector<int>& idv, bool& inCache) {
#if TIMER
  MyTimer::start(GET);
#endif
  double gettime = -MPI_Wtime();

  assert(bufSize == tileSize_);

  int tileIndex = computeTileIndex(idv, sps_);

//  cout << "get " << id_ << " " << tileIndex << endl;
  if (dim_ == 2) nget2++;
  else nget4++;

#if TASCEL
  int lo = tileIndex * tileSize_;
  int hi = (tileIndex + 1) * tileSize_ - 1;

#if CACHING
  if (Cache.contains(id_, tileIndex)) {
    delete buf;
    buf = Cache.get(id_, tileIndex);
    inCache = true;
  }
  else {
    Profiler::inc(id_, tileIndex, dim_);
    ga_get(handle_, lo, hi, buf);
    inCache = Cache.add(id_, tileIndex, buf, bufSize);
  }
#else
  Profiler::inc(id_, tileIndex, dim_);
  ga_get(handle_, lo, hi, buf);
#endif

#else  // SCIOTO

  int lo[dim_];
  int hi[dim_];
  int ld[dim_];

  for (int i = 0; i < dim_; i++) {
    lo[i] = idv[i] * wbs_[i];
    hi[i] = (idv[i] + 1) * wbs_[i] - 1;    
  }


  for (int i = 1; i < dim_; i++) {
    ld[i - 1] = wbs_[i];
  }


#if CACHING
  if (Cache.contains(id_, tileIndex)) {
    if (dim_ == 2) nhit2++;
    else nhit4++;
    delete buf;
    buf = Cache.get(id_, tileIndex);
    inCache = true;
  }
  else {

      //Profiler::inc(id_, tileIndex, dim_);
      ngaget[id_] += 1;
    NGA_Get(handle_, lo, hi, buf, ld);
    inCache = Cache.add(id_, tileIndex, buf, bufSize);
    traffic += bufSize;
  }
#else

    //Profiler::inc(id_, tileIndex, dim_);
    //ngaget[id_] += 1;

  NGA_Get(handle_, lo, hi, buf, ld);
  #endif

#endif


#if TIMER
      //MyTimer::stop(GET);
#endif

  //gettime += MPI_Wtime();
  //nget[id_] += 1;
  //tget[id_] += gettime;
  //tget_ += gettime;
}

void Tensor::getTile2(double*& buf, int bufSize, const vector<int>& idvm, bool& fromCache) {
  vector<int> idv;
  for (int i = 0; i < ids_.size(); i++) {
    int val = idvm[ids_[i]];
    idv.push_back(val);
  }
  getTile(buf, bufSize, idv, fromCache);
}

void Tensor::accTile(double* buf, int bufSize, const vector<int>& idv, double coef) {
#if TIMER
  MyTimer::start(ACC);
#endif
  assert(bufSize == tileSize_);
#if TASCEL
  int tileIndex = computeTileIndex(idv, sps_);
  int lo = tileIndex * tileSize_;
  int hi = (tileIndex + 1) * tileSize_ - 1;
  //cout << "buf" << *buf << endl;
  ga_acc(handle_, lo, hi, buf);
#else
  int lo[dim_];
  int hi[dim_];
  int ld[dim_];
  for (int i = 0; i < dim_; i++) {
    lo[i] = idv[i] * wbs_[i];
    hi[i] = (idv[i] + 1) * wbs_[i] - 1;
  }
  for (int i = 1; i < dim_; i++) {
    ld[i - 1] = wbs_[i];
  }
  NGA_Acc(handle_, lo, hi, buf, ld, &coef);
#endif
#if TIMER
  MyTimer::stop(ACC);
#endif
}

void Tensor::accTile2(double* buf, int size, const vector<int>& idvm, double coef) {
  vector<int> idv;
  for (int i = 0; i < ids_.size(); i++) {
    int val = idvm[ids_[i]];
    idv.push_back(val);
  }
  accTile(buf, size, idv, coef);
}

void Tensor::setIndexName(const vector<IndexName>& ids) {
  assert(ids.size() == idt_.size());
  for (int i = 0; i < ids.size(); i++) {
    assert(ids[i] >= 0 && ids[i] < NIDX);
    if (ids[i] < NOCC) assert(idt_[i] == HOL);
    else if (ids[i] >= NOCC && ids[i] < NOCC + NVRT) assert(idt_[i] == PAR);
    else assert(1);
  }
  ids_ = ids;
};
}
