#ifndef _COST_H_
#define _COST_H_

#include "tensor.h"
#include "helper.h"
#include <ctime>
#include <sys/time.h>


namespace RRR{
using namespace std;



class cost
{
private:
    Tensor* A;
    Tensor* B;
    Tensor* C;
    int dimA, dimB, dimC,dimG,nprocs,minCost;
    Grid* G;
    bool found; 	

public:

    	cost(Tensor* &Ainit, Tensor* &Binit, Tensor* &Cinit, Grid* &Ginit);	
   	bool bestCost(int* &idmapA, int* &idmapB, int* &idmapC, int* &ndimG, int* &npgrid);
    	void contract_2D_2D_2D(int* &idmapA, int* &idmapB, int* &idmapC, int* &ndimG, int* &npgrid);
    	
		void contract_4D_4D_4D(int* &idmapA, int* &idmapB, int* &idmapC, int* &ndimG, int* &npgrid);
	void Grid_Comb_4D(int arr[], int data[], int start, int end, int index, int r,int* &idmapA, int* &idmapB, int* &idmapC, int* &npgrid);
	void best_Cost_GGrid_4D(int curGrid[], int* &curidmapA, int* &curidmapB, int* &curidmapC);
	
	void contract_4D_2D_4D(int* &idmapA, int* &idmapB, int* &idmapC, int* &ndimG, int* &npgrid);
	void Grid_Comb_4Db(int arr[], int data[], int start, int end, int index, int r,int* &idmapA, int* &idmapB, int* &idmapC, int* &npgrid);
	void best_Cost_GGrid_4Db(int* &curGrid, int* &curidmapA, int* &curidmapB, int* &curidmapC);

void contract_4D_4D_2D(int* &idmapA, int* &idmapB, int* &idmapC, int* &ndimG, int* &npgrid);
	void Grid_Comb_4Dc(int arr[], int data[], int start, int end, int index, int r,int* &idmapA, int* &idmapB, int* &idmapC, int* &npgrid);
	void best_Cost_GGrid_4Dc(int* &curGrid, int* &curidmapA, int* &curidmapB, int* &curidmapC);	
	
void contract_2D_4D_4D(int* &idmapA, int* &idmapB, int* &idmapC, int* &ndimG, int* &npgrid);
	void Grid_Comb_4Dd(int arr[], int data[], int start, int end, int index, int r,int* &idmapA, int* &idmapB, int* &idmapC, int* &npgrid);
	void best_Cost_GGrid_4Dd(int* &curGrid, int* &curidmapA, int* &curidmapB, int* &curidmapC);

	void contract_4D_2D_2D(int* &idmapA, int* &idmapB, int* &idmapC, int* &ndimG, int* &npgrid);
	void contract_2D_4D_2D(int* &idmapA, int* &idmapB, int* &idmapC, int* &ndimG, int* &npgrid);
	void contract_2D_2D_4D(int* &idmapA, int* &idmapB, int* &idmapC, int* &ndimG, int* &npgrid);		
};
}
#endif
