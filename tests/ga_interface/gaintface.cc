
#include<iostream>
#include "mpi.h"
#include "tensor.h"
#include "cc.h"

extern task_class_t TaskClass;
using namespace RRR;

int create_tensor4D(int size, int block_grid, RRR::Grid* &grid, RRR::Tensor* &t){

    int* size_A = new int[4];
    int* block_grid_A = new int[4];
    int* idmap_A = new int[4];
    
    size_A[0] = size;
    size_A[1] = size;
    size_A[2] = size;
    size_A[3] = size;
    
    block_grid_A[0] = block_grid;
    block_grid_A[1] = block_grid;
    block_grid_A[2] = block_grid;
    block_grid_A[3] = block_grid;

    idmap_A[0] = 0;
    idmap_A[1] = 1;
    idmap_A[2] = 2;
    idmap_A[3] = 3;
    t = new Tensor("cccc", idmap_A, size_A, block_grid_A, grid);
    return 0;
}
extern vector<int> nget;
extern vector<int> ngaget;
extern vector<double> tget;
extern double traffic;
extern int nget2, nhit2;
extern int nget4, nhit4;

int O,V,wbO,wbV,spO,spV;
extern int me, nprocs, nthrds;
int main(int argc, char* argv[]){
  nget2 = 0;
  nhit2 = 0;
  nget4 = 0;
  nhit4 = 0;
  traffic = 0;

    MPI_Init(&argc, &argv);
    int rank,np;

    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&np);
    me = rank;
    nprocs = np;

    if (!me) cout << "SCIOTO" << endl;
    GA_Initialize();
    TaskClass = gtc_task_class_register(sizeof(DLTC::TaskDesc), DLTC::TaskFunc); // register function
    //TaskClass = gtc_task_class_register(sizeof(DLTC::TaskDesc), DLTC::TaskFunc); // register function
    //if(rank==0) cout<<"Grid about to initialized"<<endl;
    int size = atoi(argv[1]);
    int block_grid = atoi(argv[2]);
    int grid_dim = atoi(argv[3]);
    int* pgrid = new int[grid_dim];
    
    if(rank ==0) cout<<"Tensor Size is "<<size<<" and block grid size is "<<block_grid<<endl;
    for(int i =0; i< grid_dim; i++)
    {
	pgrid[i] = atoi(argv[4+i]);
    }
    if(rank==0) cout<<"Grid about to initialized"<<endl;

    Grid* grid = new Grid(grid_dim, pgrid);

    if(rank == 0) cout<<"Grid initialized G["<<pgrid[0]<<", "<<pgrid[1]<<", "<<pgrid[2]<<", "<<pgrid[3]<<"]"<<endl;
    
    Tensor* A;
    create_tensor4D(size,block_grid,grid,A);	

    if(rank == 0) cout<<"RRR tensor Created"<<endl;
    //////////////////////////////////////////////////////////////////////////
    /////////////////////////Initializng DLTC Tensor//////////////////////////
    //////////////////////////////////////////////////////////////////////////

    O = V = size;
    wbO = wbV = size/block_grid;
    spO = spV = block_grid;
    if(rank == 0) cout<<"Creating DLTC Tensor"<<endl;

    DLTC::Tensor DT = DLTC::Tensor4(DLTC::HOL, DLTC::HOL, DLTC::HOL, DLTC::HOL,0);
    DLTC::setIndexName4(DT,DLTC::h1, DLTC::h2, DLTC::h3, DLTC::h4);
    if(rank == 0) cout<<"DLTC Tensor Created"<<endl;
    double* data = new double[V*V*V*V];
    int count =0;
    for(int i =0; i< V; i++)
	for(int j =0; j< V; j++)
	    for(int k =0; k< V; k++)
		for(int l =0; l< V; l++)
    {
    	data[count] = i+j+k+l;
	count ++;
    }
    DT.putData(data,V*V*V*V);

    if(rank == 0) cout<<"DLTC Tensor data put"<<endl;
    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////Initializing RRR tensor from DLTC Tensor////////////////
    ////////////////////////////////////////////////////////////////////////////
    
    A->initialize_from_ga_tensor(DT);
    if(rank == 0) cout<<"RRR Tensor data put"<<endl;
    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    /////////////////////////Correctness check/////////////////////////////////

    double* ct = A->get_tensor_tiles();
    int* ca = A->get_tile_addresses();
    int nt = A->get_num_actual_tiles();
    for(int ii = 0; ii<nt; ii++)
    {
	int count = 0;
	//cout<<"["<<ca[0]<<","<<ca[1]<<","<<ca[2]<<","<<ca[3]<<"]"<<endl;
	for(int i = ca[0]*wbO; i< (ca[0]+1)*wbO; i++)
	    for(int j = ca[1]*wbO; j< (ca[1]+1)*wbO; j++)
		for(int k = ca[2]*wbO; k< (ca[2]+1)*wbO; k++)
		    for(int l = ca[3]*wbO; l< (ca[3]+1)*wbO; l++)
		    {
			double d = i+j+k+l;

			if(ct[count] != d)
			    cout<<"Failed ["<<i<<","<<j<<","<<k<<","<<l<<"] = "<<ct[count]<<endl;

			count++;
		    }

	ca += 4;
	ct += A->get_block_size();

    }
    if (rank == 0) cout<<"Passed if no Failed Printed"<<endl;
    


    GA_Terminate();
    MPI_Finalize();

    return 0;
}
