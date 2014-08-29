#ifndef _PARAM_H_
#define _PARAM_H_
namespace DLTC{
enum ExprType  { MUL, SUM };
enum IndexType { HOL, PAR };

#if CHECK
enum IndexName {
  H1, H2, H3, H4, H5, H6,
  P1, P2, P3, P4, P5, P6
};
#define NIDX 12
#define NOCC 6  /* number of occupied orbital */
#define NVRT 6  /* number of virtual orbital */
#define NMULT 1
#define NSUMM 1
#else
#define NIDX 24
#define NOCC 12
#define NVRT 12
enum IndexName{
  h1,h2,h3,h4,h5,h6,h7,h8,h9,h10,h11,h12,
  p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12};
#define NMULT 30
#define NSUMM 30
#endif

class Config {
  public:
    static int O;
    static int V;
    static int spO;
    static int spV;
    static int wbO;
    static int me;
    static int nprocs;
    static int nthrds;
    static int taskNum;
};
}
extern int O;
extern int V;
extern int spO;
extern int spV;
extern int wbO;
extern int wbV;
extern int me;
extern int nprocs;
extern int nthrds;
extern int tasknum;

#endif
