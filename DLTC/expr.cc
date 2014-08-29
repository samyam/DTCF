#include "expr.h"
namespace DLTC{
Summation::Summation(const Tensor& tC, const Tensor& tA, double coef)
  : tC_(tC), tA_(tA), coef_(coef) {
  }

IndexLoops Summation::createExtlps() {
  vector<int> lo(tC_.dim(), 0);
  IndexLoops lps = IndexLoops(tC_.ids(), lo, tC_.sps());
  return lps;
}

Multiplication::Multiplication(const Tensor& tC, 
    const Tensor& tA, const Tensor& tB, double coef)
: tC_(tC), tA_(tA), tB_(tB), coef_(coef) {

  ext_ids_.clear();
  m_ids_.clear();
  n_ids_.clear();
  k_ids_.clear();
  all_ids_.clear();
  at_ids_.clear();
  bt_ids_.clear();
  ct_ids_.clear();

  vector<int> tab(NIDX, 0);
  vector<IndexName> ida = tA_.ids(); 
  vector<IndexName> idb = tB_.ids();
  for (int i = 0; i < ida.size(); i++) tab[ida[i]] = 1; // from tA
  for (int i = 0; i < idb.size(); i++) tab[idb[i]] += 2; // from tB
  for (int i = 0; i < NIDX; i++) {
    if (tab[i] > 0 && tab[i] < 3) ext_ids_.push_back((IndexName)i); // ext inds
    if (tab[i] == 1) m_ids_.push_back((IndexName)i);
    if (tab[i] == 2) n_ids_.push_back((IndexName)i);
    if (tab[i] == 3) k_ids_.push_back((IndexName)i); // sum inds
    if (tab[i] > 0) all_ids_.push_back((IndexName)i);
  }
  ct_ids_ = m_ids_;
  ct_ids_.insert(ct_ids_.end(), n_ids_.begin(), n_ids_.end()); // mn
  at_ids_ = m_ids_;
  at_ids_.insert(at_ids_.end(), k_ids_.begin(), k_ids_.end()); // mk
  bt_ids_ = k_ids_;
  bt_ids_.insert(bt_ids_.end(), n_ids_.begin(), n_ids_.end()); // kn

  vector<int> wbm = getWB(m_ids_);
  vector<int> wbn = getWB(n_ids_);
  vector<int> wbk = getWB(k_ids_);
  uint64_t wm = 1;
  uint64_t wn = 1;
  uint64_t wk = 1;
  for (int i = 0; i < wbm.size(); i++) wm *= wbm[i];
  for (int i = 0; i < wbn.size(); i++) wn *= wbn[i];
  for (int i = 0; i < wbk.size(); i++) wk *= wbk[i];
  taskSize_ = wm * wn * wk;

  vector<int> spm = getSP(m_ids_);
  vector<int> spn = getSP(n_ids_);
  vector<int> spk = getSP(k_ids_);
  int sm = 1;
  int sn = 1;
  int sk = 1;
  for (int i = 0; i < spm.size(); i++) sm *= spm[i];
  for (int i = 0; i < spn.size(); i++) sn *= spn[i];
  for (int i = 0; i < spk.size(); i++) sk *= spk[i];
  taskNumExt_ = sm * sn;
  taskNumSum_ = sk;
}

IndexLoops Multiplication::createExtlps() {
  int n = ext_ids_.size();
  vector<int> lo(n, 0);
  vector<int> hi = getSP(ext_ids_);
  IndexLoops lps = IndexLoops(ext_ids_, lo, hi);
  return lps;
}

IndexLoops Multiplication::createSumlps() {
  int n = k_ids_.size();
  vector<int> lo(n, 0);
  vector<int> hi = getSP(k_ids_);
  IndexLoops lps = IndexLoops(k_ids_, lo, hi);
  return lps;
}
}
