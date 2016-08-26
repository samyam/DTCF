#include "mpi.h"
#include "grid.h"
#include "tensor.h"
#include "contraction.h"
#include "redistribute.h"
using namespace std;

int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    int* pgrid = new int[4];
    pgrid[0] = 4;
    pgrid[1] = 4;
    pgrid[2] = 4;
    pgrid[3] = 4;
    Grid* grid = new Grid(4, pgrid);

    int* size_A = new int[4];
    int* block_grid_A = new int[4];
    int* idmap_A = new int[4];

    size_A[0] = 64;
    size_A[1] = 64;
    size_A[2] = 64;
    size_A[3] = 64;

    block_grid_A[0] = 8;
    block_grid_A[1] = 8;
    block_grid_A[2] = 8;
    block_grid_A[3] = 8;

    idmap_A[0] = 0;
    idmap_A[1] = 1;
    idmap_A[2] = 2;
    idmap_A[3] = 3;

    Tensor* A = new Tensor("cccc", idmap_A, size_A, block_grid_A, grid);
    A->initialize();

    int* size_B = new int[4];
    int* block_grid_B = new int[4];
    int* idmap_B = new int[4];

    size_B[0] = 32;
    size_B[1] = 32;
    size_B[2] = 32;
    size_B[3] = 32;

    block_grid_B[0] = 4;
    block_grid_B[1] = 4;
    block_grid_B[2] = 4;
    block_grid_B[3] = 4;

    idmap_B[0] = 0;
    idmap_B[1] = 1;
    idmap_B[2] = 2;
    idmap_B[3] = 3;

    Tensor* B = new Tensor("cccc", idmap_B, size_B, block_grid_B, grid);
    B->initialize();

    int* size_C = new int[4];
    int* block_grid_C = new int[4];
    int* idmap_C = new int[4];

    size_C[0] = 32;
    size_C[1] = 32;
    size_C[2] = 32;
    size_C[3] = 32;

    block_grid_C[0] = 4;
    block_grid_C[1] = 4;
    block_grid_C[2] = 4;
    block_grid_C[3] = 4;

    idmap_C[0] = 0;
    idmap_C[1] = 1;
    idmap_C[2] = 2;
    idmap_C[3] = 3;

    Tensor* C = new Tensor("cccc", idmap_C, size_C, block_grid_C, grid);
    C->initialize();

    if(rank==0) cout << endl << endl << "Contraction 0: " << endl;
    if(rank==0) cout<<"C [a,b,m,n ] = A [a,b,k,l ] x B [m,n,k,l ]";
    Contraction* C0 = new Contraction(A, B, C, grid);
    C0->contract( "a,b,k,l", "m,n,k,l", "a,b,m,n");

    MPI_Finalize();
}
