#include "mpi.h"
#include "../grid.h"
#include "../tensor.h"
#include "../contraction.h"
#include "../redistribute.h"
using namespace std;

int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);
    int rank,np;
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    MPI_Comm_rank(MPI_COMM_WORLD,&np);

    int size = atoi(argv[1]);
    int block_grid = atoi(argv[2]);
    int grid_dim = atoi(argv[3]);
    int* pgrid = new int[grid_dim];

    for(int i =0; i< grid_dim; i++)
    {
	pgrid[i] = atoi(argv[4+i]);
    }

    if(rank==0) cout<<"Grid about to initialized"<<endl;

    Grid* grid = new Grid(grid_dim, pgrid);

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
 
    Tensor* A = new Tensor("cccc", idmap_A, size_A, block_grid_A, grid);
    A->initialize();

    if(rank == 0) cout<<"Tensor A initialized"<<endl;
	int n = 23;
    int** idmaps = new int*[n];
    for(int i=0; i<n; i++) idmaps[i] = new int[4];

    //0
	idmaps[0][0]=0;	idmaps[0][1]=1;	idmaps[0][2]=2; idmaps[0][3]=3;

	//1
	idmaps[1][0]=2;	idmaps[1][1]=3;	idmaps[1][2]=0; idmaps[1][3]=1;
	//2
	idmaps[2][0]=0;	idmaps[2][1]=1;	idmaps[2][2]=s; idmaps[2][3]=s;
	//3
	idmaps[3][0]=0;	idmaps[3][1]=1;	idmaps[3][2]=2; idmaps[3][3]=s;
	idmaps[4][0]=3;	idmaps[4][1]=1;	idmaps[4][2]=2; idmaps[4][3]=0;
	idmaps[5][0]=0;	idmaps[5][1]=1;	idmaps[5][2]=s; idmaps[5][3]=s;
	//4
	idmaps[6][0]=0;	idmaps[6][1]=1;	idmaps[6][2]=2; idmaps[6][3]=s;
	idmaps[7][0]=0;	idmaps[7][1]=1;	idmaps[7][2]=s; idmaps[7][3]=s;
	//5
	idmaps[8][0]=3;	idmaps[8][1]=2;	idmaps[8][2]=s; idmaps[8][3]=s;
	
	MPI_Barrier(MPI_COMM_WORLD);	
	double time =0;
	for(int i=1; i<5; i++)
	{

	    if(rank == 0) cout << "Redistributing from 0 to " << i << endl;


	    time=-MPI_Wtime();
	    A->redistribute(idmaps[i]);
	    time+=MPI_Wtime();

	    MPI_Barrier(MPI_COMM_WORLD);
	    if (rank == 0) cout << "Redistribution "<<i<<" took : "<<time<<" seconds"<<endl;
	    A->redistribute(idmaps[0]);
	}

}
