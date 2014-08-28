#include "mpi.h"
#include "grid.h"
#include "tensor.h"
#include "contraction.h"
#include "redistribute.h"
using namespace std;

int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);
    int rank,np;
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    MPI_Comm_rank(MPI_COMM_WORLD,&np);

    int size = 128;
    int block_grid = 8;
    int grid_dim = 4;
    int* pgrid = new int[grid_dim];

    
	pgrid[0] = 2;
	pgrid[1] = 2;
	pgrid[2] = 2;
	pgrid[3] = 2;
    

    if(rank==0) cout<<"Grid about to initialized"<<endl;

    Grid* grid = new Grid(grid_dim, pgrid);
	
	int grid_dim2=4;
	int* pgrid2=new int[grid_dim2];
	pgrid2[0]=4;
	pgrid2[1]=4;
pgrid2[2]=1;
pgrid2[3]=1;
	
	Grid* ng = new Grid(grid_dim2, pgrid2);

    if(rank == 0) cout<<"Grid initialized"<<endl;

    int* size_A = new int[4];
    int* block_grid_A = new int[4];
    int* idmap_A = new int[4];

    size_A[0] = size;
    size_A[1] = size;
    size_A[2] = size;
    size_A[3] = size;

    if(rank == 0) cout<<"Size "<<size<<endl;

    block_grid_A[0] = block_grid;
    block_grid_A[1] = block_grid;
    block_grid_A[2] = block_grid;
    block_grid_A[3] = block_grid;

    if(rank == 0) cout<<"Block Grid "<<block_grid<<endl;

    idmap_A[0] = 0;
    idmap_A[1] = 1;
    idmap_A[2] = 2;
    idmap_A[3] = 3;

    int s = 4;

	
    int* idmaps = new int[4];
    idmaps[0]=0;	idmaps[1]=1;	idmaps[2]=2; idmaps[3]=3;
	
	int* idmaps2 = new int[4];
	idmaps2[0]=0; idmaps2[1]=1; idmaps2[2]=2; idmaps2[3]=3;

 
    Tensor* A = new Tensor("cccc", idmaps, size_A, block_grid_A, grid);
    A->initialize();

    if(rank == 0) cout<<"Tensor A initialized"<<endl;


	
	
	MPI_Barrier(MPI_COMM_WORLD);	
	double time =0;
	

	    if(rank == 0) cout << "Redistributing "<< endl;


	    time=-MPI_Wtime();
	    TensorRedistributor* redib = new TensorRedistributor(A,idmaps2,ng);
	redib->redistribute();
	    time+=MPI_Wtime();

	    MPI_Barrier(MPI_COMM_WORLD);
	    if (rank == 0) cout << "Redistribution  took : "<<time<<" seconds"<<endl;
	   
	

}
