#include "mpi.h"
#include "../../grid.h"
#include "../../tensor.h"
#include "../../contraction.h"
#include "../../grid_redib.h"
using namespace RRR;

int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);
    int rank,np;
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    MPI_Comm_rank(MPI_COMM_WORLD,&np);

    int grid_dim = 4;
    int* pgrid = new int[grid_dim];
    for(int i =0; i< grid_dim; i++)
    {
    pgrid[i] = atoi(argv[1+i]);
    }
    if(rank==0) cout<<"Grid about to initialized"<<endl;

    Grid* grid = new Grid(grid_dim, pgrid);

    if(rank == 0) cout<<"Grid initialized"<<endl;

    int* size_A = new int[4];
    int* block_grid_A = new int[4];
    int* idmap_A = new int[4];

    size_A[0] = 104;
    size_A[1] = 104;
    size_A[2] = 48;
    size_A[3] = 48;

    block_grid_A[0] = 26;
    block_grid_A[1] = 26;
    block_grid_A[2] = 12;
    block_grid_A[3] = 12;

    idmap_A[0] = 0;
    idmap_A[1] = 1;
    idmap_A[2] = 2;
    idmap_A[3] = 3;

    Tensor* A = new Tensor("aabb", idmap_A, size_A, block_grid_A, grid);
    A->initialize();

    if(rank == 0) cout<<"Tensor A initialized"<<endl;

    int* size_B = new int[4];
    int* block_grid_B = new int[4];
    int* idmap_B = new int[4];

    size_B[0] = 48;
    size_B[1] = 48;
    size_B[2] = 104;
    size_B[3] = 104;

    block_grid_B[0] = 12;
    block_grid_B[1] = 12;
    block_grid_B[2] = 26;
    block_grid_B[3] = 26;

    idmap_B[0] = 0;
    idmap_B[1] = 1;
    idmap_B[2] = 2;
    idmap_B[3] = 3;

    Tensor* B = new Tensor("aabb", idmap_B, size_B, block_grid_B, grid);
    B->initialize();

    if(rank == 0) cout<<"Tensor B initialized"<<endl;

    int* size_C = new int[4];
    int* block_grid_C = new int[4];
    int* idmap_C = new int[4];

    size_C[0] = 48;
    size_C[1] = 48;
    size_C[2] = 48;
    size_C[3] = 48;

    block_grid_C[0] = 12;
    block_grid_C[1] = 12;
    block_grid_C[2] = 12;
    block_grid_C[3] = 12;

    idmap_C[0] = 0;
    idmap_C[1] = 1;
    idmap_C[2] = 2;
    idmap_C[3] = 3;

    Tensor* C = new Tensor("aabb", idmap_C, size_C, block_grid_C, grid);
    C->initialize();

    if(rank == 0) cout<<"Tensor C initialized"<<endl;

    double time =0.0;
    if(rank==0) cout<<"C [h10,h11,h1,h2] = A [p7,p8,h1,h2 ] x B [h10,h11,p7,p8 ]"<<endl;
    Contraction* C0 = new Contraction(A, B, C, grid);
    time =-MPI_Wtime();
    C0->contract( "p7,p8,h1,h2", "h10,h11,p7,p8", "h10,h11,h1,h2");

    time +=MPI_Wtime();
    C0->display_times();
    if(rank==0) cout << "Total Time ="<<time<<endl<<endl;
    MPI_Finalize();
}
