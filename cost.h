#include "tensor.h"
#include <ctime>
#include <sys/time.h>

using namespace std;



template<class BidirectionalIterator>
bool next_partial_permutation(BidirectionalIterator first,                           
                              BidirectionalIterator last, int middle)
{
    std::reverse(first + middle, last);
    return std::next_permutation(first, last);
}


class cost
{
private:
    Tensor* A1;
    Tensor* B1;
    Tensor* C1;
    int dimA, dimB, dimC,grid_dims;
    Grid* G1;
	Grid* G2;
    bool f,changed;

public:

    cost(Tensor* &A, Tensor* &B, Tensor* &C, int dims_A, int dims_B, int dims_C, Grid* &G, int griddims);


    double calcCost(int dimsA, int dimsB, int dimsC, Tensor* &A, Tensor* &B, Tensor* &C, int griddims, Grid* &G);


    bool check_redistr(Tensor* &T, Tensor* &C, string* &t, string* &c);


    double bcost(int iter,double MA,double MB,double tw, Grid* &G, int griddims, int* &ddopairs, char tnsr);

    double getclock();

    void best_Cost();

    void Grid_Comb(int arr[], int data[], int start, int end, int index, int r, int nprocs);



    void best_Cost_GGrid();

    bool check_space(Tensor* &A, int dimsC, int griddims, Grid* &G);

};
