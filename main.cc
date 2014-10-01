#include "mpi.h"
#include "tensor.h"
#include "contraction.h"
#include "redistribute.h"
#include "test.h"
//
//#ifdef __bgq__
//#include <mpix.h>
//#else
//#warning This test should be run on a Blue Gene.
//#endif
//
#define TEST 0

using namespace RRR;

int** generate_Addr(int dimension, int max_blocks)
{

    int** t_addr = (int**)malloc(dimension * sizeof(int*));
    int address_space[dimension * max_blocks];

    for(int i = 0; i<dimension; i++)
    {
        t_addr[i] = &address_space[i * max_blocks];
    }

    return t_addr;
}

int main(int argc, char* argv[]) {

    MPI_Init(&argc, &argv);

   
 //cout<<"Hello"<<endl;    
 //cout<<"Getting MPI size and rank"<<endl;
    int size,rank;
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    //cout<<"Setting pgrid"<<endl;
    int* pgrid = new int[5];
    pgrid[0] = 2;//atoi(argv[1]);
    pgrid[1] = 2;//atoi(argv[1]);
    pgrid[2] = 2;//atoi(argv[2]);
    pgrid[3] = 2;//atoi(argv[2]);
    pgrid[4] = 1;//atoi(argv[2]);

    //cout<<"Initializing grid"<<endl;
    Grid* grid = new Grid(4, pgrid);

    //cout<<"getting parameters"<<endl;
    //int ctr = atoi(argv[5]);
    //int vgrid_side = atoi(argv[3]);
    //int N = atoi(argv[4]);
    //
    //
   ////assert(N % vgrid_side == 0);
    //if(rank == 0)
    //{
    //    cout << "#nodes = "<<size;
    //    cout << "   Physical grid: ";
    //    print_tile_addr(4,pgrid);
    //    cout << "   Vgrid side = " << vgrid_side;
    //    cout << "   N = " << N;
    //    cout << endl << endl;
    //}

    if(!rank)
	cout<<"Calling test program : Identify Receivers"<<endl;

   
   contraction_1(grid,8,32);
    
    
    MPI_Finalize();
    return 0;

}
/*
int main(int argc, char* argv[]) 
{
    cout<<"Hello"<<endl;
    MPI_Init(&argc, &argv);

       int size,rank;
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    int* pgrid = new int[4];
    pgrid[0] = atoi(argv[1]);
    pgrid[1] = atoi(argv[1]);
    pgrid[2] = atoi(argv[2]);
    pgrid[3] = atoi(argv[2]);

    Grid* grid = new Grid(4, pgrid);

    int ctr = atoi(argv[5]);
    int vgrid_side = atoi(argv[3]);
    int N = atoi(argv[4]);
    //assert(N % vgrid_side == 0);
    if(rank == 0)
    {
        cout << "#nodes = "<<size;
        cout << "   Physical grid: ";
        print_tile_addr(4,pgrid);
        cout << "   Vgrid side = " << vgrid_side;
        cout << "   N = " << N;
        cout << endl << endl;
    }

    if(ctr==1)
    {
	cout<<"Calling test program"<<endl;
		redistribute_test(grid, vgrid_side, N);
    }     
   //contraction_1(grid, vgrid_side, N);
    /*if(ctr==2)
        contraction_2(pgrid, vgrid_side, N);
    if(ctr==3)
        contraction_3(pgrid, vgrid_side, N);
    if(ctr==4)
        contraction_4(pgrid, vgrid_side, N);
    if(ctr==6)
        contraction_6(pgrid, vgrid_side, N);
    if(ctr==7)
        contraction_7(pgrid, vgrid_side, N);
    if(ctr==5)
    {
        int* ppgrid = new int[2];
        ppgrid[0] = atoi(argv[1]);
        ppgrid[1] = atoi(argv[2]);
        contraction_5(ppgrid, vgrid_side, N);
    }
    if(ctr==0)
    {
        contraction_1(pgrid, vgrid_side, N);
        contraction_2(pgrid, vgrid_side, N);
        contraction_3(pgrid, vgrid_side, N);
        contraction_4(pgrid, vgrid_side, N);
        contraction_5(pgrid, vgrid_side, N);
    }
    

    MPI_Finalize();
   return 0;
}
*/

