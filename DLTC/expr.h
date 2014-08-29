#ifndef _EXPR_H_
#define _EXPR_H_

#include "dltc_tensor.h"
namespace DLTC{
class Summation { /* tC += coef * tA */
  private:
    Tensor tC_;
    Tensor tA_;
    double coef_;
  public:
    Summation(const Tensor& tC, const Tensor& tA, double coef);
    Summation() {};
    ~Summation() {};
    inline Tensor& tC()   { return tC_;   }
    inline Tensor& tA()   { return tA_;   }
    inline double coef()  { return coef_; }
    inline vector<IndexName> ext_ids() { return tC_.ids(); }
    IndexLoops createExtlps();
};

class Multiplication { /* tC += coef * tA *tB */
  private:
    Tensor tC_;
    Tensor tA_;
    Tensor tB_;
    double coef_;
    vector<IndexName> ext_ids_;  /* external indices */
    vector<IndexName> m_ids_;    /* external indices from tA */
    vector<IndexName> n_ids_;    /* external indices from tB */
    vector<IndexName> k_ids_;    /* contracted indices */
    vector<IndexName> all_ids_;  /* external + contraced indices */

    /* Collapse and transpose tensors into 2D-MM:
     * 1) for tile of tA, external indices first
     * 2) for tile of tB, contracted indices first
     * 3) within indices groups, virutal indices first
     * Ex: abij+=akil*bljk -> iajb+=iakl*kljb */
    vector<IndexName> ct_ids_; // iajb
    vector<IndexName> at_ids_; // iakl
    vector<IndexName> bt_ids_; // kljb

    uint64_t taskSize_;
    int taskNumExt_;
    int taskNumSum_;

  public:
    Multiplication(const Tensor& tC, const Tensor& tA, const Tensor& tB, double coef);
    Multiplication() {};
    ~Multiplication() {};
    inline Tensor& tC()   { return tC_;   }
    inline Tensor& tA()   { return tA_;   }
    inline Tensor& tB()   { return tB_;   }
    inline double coef()  { return coef_; }
    inline vector<IndexName> ext_ids() { return ext_ids_; }
    inline vector<IndexName> m_ids()   { return m_ids_;   }
    inline vector<IndexName> n_ids()   { return n_ids_;   }
    inline vector<IndexName> k_ids()   { return k_ids_;   }
    inline vector<IndexName> at_ids()  { return at_ids_;  }
    inline vector<IndexName> bt_ids()  { return bt_ids_;  }
    inline vector<IndexName> ct_ids()  { return ct_ids_;  }
    inline vector<IndexName> all_ids() { return all_ids_; }
    inline int order()    { return all_ids_.size(); }
    IndexLoops createExtlps();
    IndexLoops createSumlps();

    inline int taskNumAll()     { return taskNumExt_*taskNumSum_; };
    inline int taskNumExt()     { return taskNumExt_; };
    inline int taskNumSum()     { return taskNumSum_; };
    inline uint64_t taskSize()  { return taskSize_; };
};
}
#endif
