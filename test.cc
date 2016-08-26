#define TEST_FILE
#include "test.h"

using namespace RRR;



//IMPORTANT : Before running this test make sure that the MPI sends
//and receives are commented out.
void rect_identify_receivers(Grid* &g)
{
    // Set grid and tensor dimensions
    int grid_dims = g->grid_dims;
    int dims_A = 4;
    int dims_B = 4;
    int dims_C = 4;
    int N = 32;
    int vgrid_side = 8;

    int serial = grid_dims;
    
    int size = g->nprocs, rank = g->rank;

    // Set physical and virtual grid dimensions

    int *vgrid_A = new int[dims_A];
    int *vgrid_B = new int[dims_B];
    int *vgrid_C = new int[dims_C];

    int* pgrid = g->pgrid;

    assert_pgrid_size(size, grid_dims, pgrid);

    vgrid_A[0] = vgrid_side;
    vgrid_A[1] = vgrid_side;
    vgrid_A[2] = vgrid_side;
    vgrid_A[3] = vgrid_side;

    vgrid_B[0] = vgrid_side;
    vgrid_B[1] = vgrid_side;
    vgrid_B[2] = vgrid_side;
    vgrid_B[3] = vgrid_side;

    vgrid_C[0] = vgrid_side;
    vgrid_C[1] = vgrid_side;
    vgrid_C[2] = vgrid_side;
    vgrid_C[3] = vgrid_side;

    // Create idmap for Tensor A and B
    int* idmap_A = new int[dims_A];
    int* idmap_B = new int[dims_B];
    int* idmap_C = new int[dims_C];

    idmap_A[0] = 0;
    idmap_A[1] = 1;
    idmap_A[2] = 2;
    idmap_A[3] = 3;

    idmap_B[0] = 0;//serial;
    idmap_B[1] = 1;//serial;
    idmap_B[2] = 2;
    idmap_B[3] = 3;

    idmap_C[0] = 0;
    idmap_C[1] = 1;
    idmap_C[2] = 2;
    idmap_C[3] = 3;


    // Set Tensor sizes
    int *size_A = new int[dims_A];
    int *size_B = new int[dims_B];
    int *size_C = new int[dims_C];
    for(int i=0; i<dims_A; i++) 
    {

        size_A[i] = N;

    }
    for(int i=0; i<dims_B; i++) 
    {

        size_B[i] = N;
    }
    for(int i=0; i<dims_C; i++) 
    {

        size_C[i] = N;
    }

    double time = 0.0, tensor_time = 0.0, max_tensor_time = 0.0;

    // Create input tensors
    time=- MPI_Wtime();
    tensor_time = -MPI_Wtime();
    max_tensor_time;

    Tensor *A0 = new Tensor("cdaa", idmap_A, size_A, vgrid_A, g);
    A0->initialize();

    Tensor *B0 = new Tensor("cdaa", idmap_B, size_B, vgrid_B, g);
    B0->initialize();

    Tensor *C0 = new Tensor("ccdd", idmap_C, size_C, vgrid_C, g);
    C0->initialize();

    Contraction* CC0 = new Contraction(A0, B0, C0, g);
    int contr_dim = 3;
    int contr_idx = 0;
    int count_addr_sends = 0;
    int count_data_sends = 0;

    A0->set_cntr_map(contr_dim,contr_idx);

    for(int r = 0; r< size; r++){
	if(rank == rank && r == rank)
	{
	    
	    cout<<" Processor address is : "<<g->get_proc_addr_str(rank)<<endl;
	    cout<<" Rank :"<<rank<<endl;
	    CC0->send_to_instigator_rect(A0, contr_dim, contr_idx, count_addr_sends, count_data_sends);
	    cout<<endl<<endl;
	
	}
	cout<<"";
	MPI_Barrier(MPI_COMM_WORLD);

    }


    A0->~Tensor();
    B0->~Tensor();
    C0->~Tensor();
    CC0->~Contraction();
}

void rect_grid_instigation(Grid* &g)
{
    // Set grid and tensor dimensions
    int grid_dims = g->grid_dims;
    int dims_A = 4;
    int serial = grid_dims;
	
    int size = g->nprocs, rank = g->rank;
    int N = 32;
    int vgrid_side = 16;

    // Set physical and virtual grid dimensions

    int *vgrid_A = new int[dims_A];
    
    int* pgrid = g->pgrid;

    assert_pgrid_size(size, grid_dims, pgrid);

    vgrid_A[0] = vgrid_side;
    vgrid_A[1] = vgrid_side;
    vgrid_A[2] = vgrid_side;
    vgrid_A[3] = vgrid_side;

    // Create idmap for Tensor A and B
    int* idmap_A = new int[dims_A];
    idmap_A[0] = 0;
    idmap_A[1] = 1;
    idmap_A[2] = 2;
    idmap_A[3] = 3;


    // Set Tensor sizes
    int *size_A = new int[dims_A];
    for(int i=0; i<dims_A; i++) 
    {

        size_A[i] = N;

    }
    
    Tensor *A0 = new Tensor("aabb", idmap_A, size_A, vgrid_A, g);
    A0->initialize();
    
    int** senders;
    int cdim = 1;
    int cntr_indx = 5;
    A0->set_cntr_map(cdim,cntr_indx);
    //cout<<"Hello"<<endl;
    for(int r = 0; r< size;r++){
	if(rank == r){
	    //cout<<" Processor address is : "<<g->get_proc_addr_str(rank)<<endl;
	    //cout<<" Rank :"<<rank<<". Senders are : "<<endl;

	    int num_senders = A0->get_rect_bouncers(cdim,cntr_indx,senders);
	    //cout<<"Number of Senders is "<<num_senders<<endl;
	    if(num_senders > 0){
		cout<<" Processor address is : "<<g->get_proc_addr_str(rank)<<endl;
		cout<<" Rank :"<<rank<<". Senders are : "<<endl;
		cout<<"Number of Senders is "<<num_senders<<endl;
		for(int i = 0; i< num_senders; i++)
		    print_tile_addr(grid_dims, senders[i]);
		cout<<endl<<endl<<endl;
	    }
	}
	MPI_Barrier(MPI_COMM_WORLD);

    }
			
    A0->~Tensor();
}


void contr_rotate(Grid* &g, int vgrid_side, int N)
{
    // Set grid and tensor dimensions
    int grid_dims = g->grid_dims;
    int dims_A = 4;
    int dims_B = 4;
    int dims_C = 6;

    int serial = grid_dims;
	
    int size = g->nprocs, rank = g->rank;

    if(rank == 0) cout << "Contraction 1:" << endl;

    // Set physical and virtual grid dimensions

    int *vgrid_A = new int[dims_A];
    int *vgrid_B = new int[dims_B];
    int *vgrid_C = new int[dims_C];

    int* pgrid = g->pgrid;

    assert_pgrid_size(size, grid_dims, pgrid);

    vgrid_A[0] = vgrid_side;
    vgrid_A[1] = vgrid_side;
    vgrid_A[2] = vgrid_side;
    vgrid_A[3] = vgrid_side;

    vgrid_B[0] = vgrid_side;
    vgrid_B[1] = vgrid_side;
    vgrid_B[2] = vgrid_side;
    vgrid_B[3] = vgrid_side;

    vgrid_C[0] = vgrid_side;
    vgrid_C[1] = vgrid_side;
    vgrid_C[2] = vgrid_side;
    vgrid_C[3] = vgrid_side;
    vgrid_C[4] = vgrid_side;
    vgrid_C[5] = vgrid_side;

    // Create idmap for Tensor A and B
    int* idmap_A = new int[dims_A];
    int* idmap_B = new int[dims_B];
    int* idmap_C = new int[dims_C];

    idmap_A[0] = 0;
    idmap_A[1] = 1;
    idmap_A[2] = 2;
    idmap_A[3] = 3;

    idmap_B[0] = 0;//serial;
    idmap_B[1] = serial;//serial;
    idmap_B[2] = serial;
    idmap_B[3] = 3;

    idmap_C[0] = 0;
    idmap_C[1] = 1;
    idmap_C[2] = 2;
    idmap_C[3] = 3;
    idmap_C[4] = serial;
    idmap_C[5] = serial;


    // Set Tensor sizes
    int *size_A = new int[dims_A];
    int *size_B = new int[dims_B];
    int *size_C = new int[dims_C];
    for(int i=0; i<dims_A; i++) 
    {

        size_A[i] = N;

    }
    for(int i=0; i<dims_B; i++) 
    {

        size_B[i] = N;
    }
    for(int i=0; i<dims_C; i++) 
    {

        size_C[i] = N;
    }

    double time = 0.0, tensor_time = 0.0, max_tensor_time = 0.0;

    // Create input tensors
    time=- MPI_Wtime();
    tensor_time = -MPI_Wtime();
    max_tensor_time;

    Tensor *A0 = new Tensor("cccc", idmap_A, size_A, vgrid_A, g);
    A0->initialize();

    Tensor *B0 = new Tensor("cccc", idmap_B, size_B, vgrid_B, g);
    B0->initialize();

    Tensor *C0 = new Tensor("cccccc", idmap_C, size_C, vgrid_C, g);
    C0->initialize();

    time=- MPI_Wtime();

    tensor_time += MPI_Wtime();

    MPI_Reduce(&tensor_time, &max_tensor_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    if(rank==0) cout << "Tensor Creation time = " << max_tensor_time << endl;

    if(rank==0) cout << "A[a1,a2,a3,k1] * B[k1,b1,b2,b3] = C0[a1,a2,a3,b3,b1,b2]"<<endl;

    // Create contraction and run

    time=-MPI_Wtime();
    Contraction* CC0 = new Contraction(A0, B0, C0, g);
    CC0->contract("a1,a2,a3,k1", "k1,b1,b2,b3", "a1,a2,a3,b3,b1,b2");
    time+=MPI_Wtime();
    if(rank ==0) cout<<"Total time          = "<<time<<endl<<endl;

    /*    A0->~Tensor();
	  B0->~Tensor();
	  C0->~Tensor();
	  CC0->~Contraction();*/
    
}

/*void rotation_test(Grid* &g, int vgrid_side, int N)
  {
  // Set grid and tensor dimensions
  int grid_dims = g->grid_dims;
  int dims_A = 4;
  int dims_B = 3;
  int dims_C = 3;

  int serial = grid_dims;
	
  int size = g->nprocs, rank = g->rank;

  if(rank == 0) cout << "25D SUMMA:" << endl;

  // Set physical and virtual grid dimensions

  int *vgrid_A = new int[dims_A];
  int *vgrid_B = new int[dims_B];
  int *vgrid_C = new int[dims_C];

  int* pgrid = g->pgrid;

  assert_pgrid_size(size, grid_dims, pgrid);

  vgrid_A[0] = vgrid_side;
  vgrid_A[1] = vgrid_side;
  vgrid_A[2] = vgrid_side;
  vgrid_A[3] = vgrid_side;
   
  vgrid_B[0] = vgrid_side;
  vgrid_B[1] = vgrid_side;
  vgrid_B[2] = vgrid_side;

  vgrid_C[0] = vgrid_side;
  vgrid_C[1] = vgrid_side;
  vgrid_C[2] = vgrid_side;
   
  // Create idmap for Tensor A and B
  int* idmap_A = new int[dims_A];
  int* idmap_B = new int[dims_B];
  int* idmap_C = new int[dims_C];

  idmap_A[0] = 0;
  idmap_A[1] = 1;
  idmap_A[2] = 2;
  idmap_A[3] = 3;


  idmap_B[0] = 0;
  idmap_B[1] = 2;
  idmap_B[2] = 3;

  idmap_C[0] = 0;
  idmap_C[1] = 1;
  idmap_C[2] = 2;


  // Set Tensor sizes
  int *size_A = new int[dims_A];
  int *size_B = new int[dims_B];
  int *size_C = new int[dims_C];
  for(int i=0; i<dims_A; i++) 
  {

  size_A[i] = N;

  }
  for(int i=0; i<dims_B; i++) 
  {

  size_B[i] = N;
  }
  for(int i=0; i<dims_C; i++) 
  {

  size_C[i] = N;
  }

  double time = 0.0, tensor_time = 0.0, max_tensor_time = 0.0;

  // Create input tensors
  time=- MPI_Wtime();
  tensor_time = -MPI_Wtime();
  max_tensor_time;

  Tensor *A = new Tensor("aabb", idmap_A, size_A, vgrid_A, g);
  A->initialize();

  Tensor *B0 = new Tensor("ccc", idmap_B, size_B, vgrid_B, g);
  B0->initialize();

  Tensor *C0 = new Tensor("ccc", idmap_C, size_C, vgrid_C, g);
  C0->initialize();

  time=- MPI_Wtime();

  tensor_time += MPI_Wtime();


  // Create contraction and run
  list<int> rotate_dims;
  rotate_dims.push_back(0);
  rotate_dims.push_back(1);

  time=-MPI_Wtime();
  Contraction* CC0 = new Contraction(A, B0, C0, g);
  double* data = A->get_tensor_tiles();
  int* addr = A->get_tile_addresses();
  int cc = A->get_num_actual_tiles();
  if(rank==0)
  cout<<"Start rotation test"<<endl;    

  // CC0->rotate(rotate_dims, A, 
//		data,addr,
//	cc);
time+=MPI_Wtime();
}*/


void two_point_five_D_SUMMA(Grid* &g, int vgrid_side, int N)
{
    // Set grid and tensor dimensions
    int grid_dims = g->grid_dims;
    int dims_A = 4;
    int dims_B = 3;
    int dims_C = 3;

    int serial = grid_dims;
	
    int size = g->nprocs, rank = g->rank;

    if(rank == 0) cout << "25D SUMMA:" << endl;

    // Set physical and virtual grid dimensions

    int *vgrid_A = new int[dims_A];
    int *vgrid_B = new int[dims_B];
    int *vgrid_C = new int[dims_C];

    int* pgrid = g->pgrid;

    assert_pgrid_size(size, grid_dims, pgrid);

    vgrid_A[0] = vgrid_side;
    vgrid_A[1] = vgrid_side;
    vgrid_A[2] = vgrid_side;
    vgrid_A[3] = vgrid_side;
   
    vgrid_B[0] = vgrid_side;
    vgrid_B[1] = vgrid_side;
    vgrid_B[2] = vgrid_side;

    vgrid_C[0] = vgrid_side;
    vgrid_C[1] = vgrid_side;
    vgrid_C[2] = vgrid_side;
   
    // Create idmap for Tensor A and B
    int* idmap_A = new int[dims_A];
    int* idmap_B = new int[dims_B];
    int* idmap_C = new int[dims_C];

    idmap_A[0] = 0;
    idmap_A[1] = 1;
    idmap_A[2] = 2;
    idmap_A[3] = 3;


    idmap_B[0] = 0;
    idmap_B[1] = 2;
    idmap_B[2] = 3;

    idmap_C[0] = 0;
    idmap_C[1] = 1;
    idmap_C[2] = 2;


    // Set Tensor sizes
    int *size_A = new int[dims_A];
    int *size_B = new int[dims_B];
    int *size_C = new int[dims_C];
    for(int i=0; i<dims_A; i++) 
    {

        size_A[i] = N;

    }
    for(int i=0; i<dims_B; i++) 
    {

        size_B[i] = N;
    }
    for(int i=0; i<dims_C; i++) 
    {

        size_C[i] = N;
    }

    double time = 0.0, tensor_time = 0.0, max_tensor_time = 0.0;

    // Create input tensors
    time=- MPI_Wtime();
    tensor_time = -MPI_Wtime();
    max_tensor_time;

    Tensor *A0 = new Tensor("cccc", idmap_A, size_A, vgrid_A, g);
    A0->initialize();

    Tensor *B0 = new Tensor("ccc", idmap_B, size_B, vgrid_B, g);
    B0->initialize();

    Tensor *C0 = new Tensor("ccc", idmap_C, size_C, vgrid_C, g);
    C0->initialize();

    time=- MPI_Wtime();

    tensor_time += MPI_Wtime();

    if(rank==0) cout << "Tensor Creation time = " << max_tensor_time << endl;

    if(rank==0) cout << "A[a1,a2,k1,k2] * B[k1,b1,k2] = C0[a1,a2,b1]"<<endl;

    // Create contraction and run

    time=-MPI_Wtime();
    Contraction* CC0 = new Contraction(A0, B0, C0, g);
    CC0->contract("a1,a2,k1,k2", "k1,b1,k2", "a1,a2,b1");
    time+=MPI_Wtime();
    if(rank ==0) cout<<"Total time          = "<<time<<endl<<endl;

    A0->~Tensor();
    B0->~Tensor();
    C0->~Tensor();
    CC0->~Contraction();

}


void redistribute_test(Grid* &g, int vgrid_side, int N)
{
    int dims_A = 4;
    int *vgrid_A = new int[dims_A];
    int s = g->grid_dims;
   
    //cout<<"Setting V-grid"<<endl;
    vgrid_A[0] = vgrid_side;
    vgrid_A[1] = vgrid_side;
    vgrid_A[2] = vgrid_side;
    vgrid_A[3] = vgrid_side;
    
    int n = 23;
    int** idmaps = new int*[n];
    for(int i=0; i<n; i++) idmaps[i] = new int[dims_A];

    idmaps[0][0]=0;	idmaps[0][1]=1;	idmaps[0][2]=2; idmaps[0][3]=3;
    idmaps[1][0]=3;	idmaps[1][1]=2;	idmaps[1][2]=1; idmaps[1][3]=0;
    idmaps[2][0]=2;	idmaps[2][1]=0;	idmaps[2][2]=3; idmaps[2][3]=1;
    idmaps[3][0]=0;	idmaps[3][1]=1;	idmaps[3][2]=2; idmaps[3][3]=3;
    idmaps[4][0]=0;	idmaps[4][1]=s;	idmaps[4][2]=2; idmaps[4][3]=3;
    idmaps[5][0]=0;	idmaps[5][1]=2;	idmaps[5][2]=s; idmaps[5][3]=s;
    idmaps[6][0]=0;	idmaps[6][1]=s;	idmaps[6][2]=3; idmaps[6][3]=2;
    idmaps[7][0]=0;	idmaps[7][1]=s;	idmaps[7][2]=2; idmaps[7][3]=3;
    idmaps[8][0]=0;	idmaps[8][1]=s;	idmaps[8][2]=2; idmaps[8][3]=s;
    idmaps[9][0]=0;	idmaps[9][1]=1;	idmaps[9][2]=s; idmaps[9][3]=s;
    idmaps[10][0]=s;idmaps[10][1]=1;idmaps[10][2]=s;idmaps[10][3]=s;
    idmaps[11][0]=3;idmaps[11][1]=s;idmaps[11][2]=s;idmaps[11][3]=s;
    idmaps[12][0]=0;idmaps[12][1]=1;idmaps[12][2]=2;idmaps[12][3]=3;
	
    idmaps[13][0]=1;idmaps[13][1]=0;idmaps[13][2]=3;idmaps[13][3]=2;
    idmaps[14][0]=1;idmaps[14][1]=s;idmaps[14][2]=s;idmaps[14][3]=s;
    idmaps[15][0]=2;idmaps[15][1]=s;idmaps[15][2]=0;idmaps[15][3]=s;
    idmaps[16][0]=2;idmaps[16][1]=3;idmaps[16][2]=0;idmaps[16][3]=1;
    idmaps[17][0]=3;idmaps[17][1]=2;idmaps[17][2]=0;idmaps[17][3]=1;
    idmaps[18][0]=3;idmaps[18][1]=s;idmaps[18][2]=s;idmaps[18][3]=1;
    idmaps[19][0]=2;idmaps[19][1]=s;idmaps[19][2]=s;idmaps[19][3]=0;
    idmaps[20][0]=3;idmaps[20][1]=2;idmaps[20][2]=1;idmaps[20][3]=0;
    idmaps[21][0]=1;idmaps[21][1]=3;idmaps[21][2]=2;idmaps[21][3]=0;
    idmaps[22][0]=1;idmaps[22][1]=3;idmaps[22][2]=s;idmaps[22][3]=0;

    //cout<<"Setting tensor size"<<endl;
    int *size_A = new int[dims_A];
    for(int i=0; i<dims_A; i++) 
        size_A[i] = N;
    
    //cout<<"Creating tensor"<<endl;
    Tensor* A = new Tensor("cdcd", idmaps[0], size_A, vgrid_A, g); 
    //cout<<"Initializing tensor"<<endl;
    A->initialize();
    

    for(int i=0; i<22; i++)
    {
	MPI_Barrier(MPI_COMM_WORLD);
	cout << "Redistributing from " << i << " to " << i+1 << endl;
    	A->redistribute(idmaps[i+1]);
    }
}


void big_matrix_transpose_test(Grid* &g, int vgrid_side, int N)
{

    // Set grid and tensor dimensions
    int grid_dims = g->grid_dims;
    int dims_A = 4;
    int serial = grid_dims;
	
    int size = g->nprocs, rank = g->rank;

    if(rank == 0) cout << "Contraction 1:" << endl;

    // Set physical and virtual grid dimensions

    int *vgrid_A = new int[dims_A];
    int* pgrid = g->pgrid;

    assert_pgrid_size(size, grid_dims, pgrid);

    vgrid_A[0] = vgrid_side;
    vgrid_A[1] = vgrid_side;
    vgrid_A[2] = vgrid_side;
    vgrid_A[3] = vgrid_side;

    // Create idmap for Tensor A and B
    int* idmap_A = new int[dims_A];

    idmap_A[0] = 0;
    idmap_A[1] = 1;
    idmap_A[2] = 2;
    idmap_A[3] = 3;

    // Set Tensor sizes
    int *size_A = new int[dims_A];

    for(int i=0; i<dims_A; i++) 
    {
        size_A[i] = N;
    }

    int block_side = N/vgrid_side;
    int row_per_block = block_side*block_side;
    int col_per_block = row_per_block;
    int num_row_dim = 2;

    Tensor *A0 = new Tensor("aabb", idmap_A, size_A, vgrid_A, g);
    A0->initialize();

    if(rank == 0)
    {

	int num_tiles = A0->get_num_actual_tiles();
	int* tile_addrs = A0->get_tile_addresses();
	double* tensor_tiles = A0->get_tensor_tiles();

	print_tile_addrs(4, tile_addrs, num_tiles);

	cout<<"Starting big matrix dump"<<endl;

	//combines individial tiles of C into a 2D matrix called big_matrix_C
	std::map<int, std::map<int, int> >* big_matrix_map;
	double* big_matrix;

	//the big matrix is stored in big_matrix
	create_big_matrix(A0, tensor_tiles, tile_addrs, idmap_A, num_tiles, 
			  row_per_block, col_per_block, num_row_dim, 
			  big_matrix_map, big_matrix);

	///-----------------Correctness Checking ------------------------///
	/*To check correctness we revert the big matrix to the 
	  multidimensional tiles
	  and see if this is the same as tensor_tiles*/
	
	cout<<"Starting rever big matrix dump"<<endl;

	double* new_tensor_tiles = new double[A0->get_num_actual_tiles()*A0->get_block_size()* sizeof(double)];
	revert_big_matrix(A0,  big_matrix_map, 
			  big_matrix, row_per_block, col_per_block, 
			  idmap_A, new_tensor_tiles);

	
	int is_equal = are_equal_blocks(A0->get_block_size(), tensor_tiles, new_tensor_tiles, A0->get_num_actual_tiles());
	if(!is_equal)
	{
	    cout<<"Big Trouble!!!!"<<endl;
	}
	else
	{
	    cout<<"Fine and Dandy!!!!"<<endl;
	}

       
    }
   

}

void reduction_test(Grid* &g)
{

    int size,rank;
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    int grid_dims = g->grid_dims;
    int* reduction_map = new int[grid_dims];

    memset(reduction_map,0,sizeof(int)*grid_dims);
    reduction_map[2] = 1;
    reduction_map[3] = 1;

    double* data = new double[524288];
    for(int i = 0; i < 524288; i++)
    {
	data[i] = (double)i;
    }

    Contraction* red = new Contraction(g);
    
    double t =-MPI_Wtime();
    red->reduction(reduction_map, data, 524288, 1);
    t+=MPI_Wtime();
    
    if(rank==0) cout<<" The reduction time is : "<<t<<endl;

    //if((g->proc_addr[0] == 0) )//&& (g->proc_addr[2] == 0))
    /*{
      print_tile_addr(grid_dims, g->proc_addr);
      print_block_data(g->nprocs, data);
      }*/
}


void contraction_1(Grid* &g, int vgrid_side, int N)
{
    // Set grid and tensor dimensions
    int grid_dims = g->grid_dims;
    int dims_A = 4;
    int dims_B = 4;
    int dims_C = 4;

    int serial = grid_dims;
	
    int size = g->nprocs, rank = g->rank;

    if(rank == 0) cout << "Contraction 1:" << endl;

    // Set physical and virtual grid dimensions

    int *vgrid_A = new int[dims_A];
    int *vgrid_B = new int[dims_B];
    int *vgrid_C = new int[dims_C];

    int* pgrid = g->pgrid;

    assert_pgrid_size(size, grid_dims, pgrid);

    vgrid_A[0] = vgrid_side;
    vgrid_A[1] = vgrid_side;
    vgrid_A[2] = vgrid_side;
    vgrid_A[3] = vgrid_side;

    vgrid_B[0] = vgrid_side;
    vgrid_B[1] = vgrid_side;
    vgrid_B[2] = vgrid_side;
    vgrid_B[3] = vgrid_side;

    vgrid_C[0] = vgrid_side;
    vgrid_C[1] = vgrid_side;
    vgrid_C[2] = vgrid_side;
    vgrid_C[3] = vgrid_side;

    // Create idmap for Tensor A and B
    int* idmap_A = new int[dims_A];
    int* idmap_B = new int[dims_B];
    int* idmap_C = new int[dims_C];

    idmap_A[0] = 2;
    idmap_A[1] = 3;
    idmap_A[2] = 0;
    idmap_A[3] = 1;

    idmap_B[0] = 0;//serial;
    idmap_B[1] = 1;//serial;
    idmap_B[2] = 2;
    idmap_B[3] = 3;

    idmap_C[0] = 2;
    idmap_C[1] = 1;
    idmap_C[2] = 0;
    idmap_C[3] = 3;


    // Set Tensor sizes
    int *size_A = new int[dims_A];
    int *size_B = new int[dims_B];
    int *size_C = new int[dims_C];
    for(int i=0; i<dims_A; i++) 
    {

        size_A[i] = N;

    }
    for(int i=0; i<dims_B; i++) 
    {

        size_B[i] = N;
    }
    for(int i=0; i<dims_C; i++) 
    {

        size_C[i] = N;
    }

    double time = 0.0, tensor_time = 0.0, max_tensor_time = 0.0;

    // Create input tensors
    time=- MPI_Wtime();
    tensor_time = -MPI_Wtime();
    max_tensor_time;

    Tensor *A0 = new Tensor("cdaa", idmap_A, size_A, vgrid_A, g);
    A0->initialize();

    Tensor *B0 = new Tensor("cdaa", idmap_B, size_B, vgrid_B, g);
    B0->initialize();

    Tensor *C0 = new Tensor("ccdd", idmap_C, size_C, vgrid_C, g);
    C0->initialize();

    time=- MPI_Wtime();

    tensor_time += MPI_Wtime();

    MPI_Reduce(&tensor_time, &max_tensor_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    if(rank==0) cout << "Tensor Creation time = " << max_tensor_time << endl;

    if(rank==0) cout << "A[a1,k1,b1>k2] * B[k2,a2,k1>b2] = C0[a1,a2,b1,b2]"<<endl;

    // Create contraction and run

    time=-MPI_Wtime();
    Contraction* CC0 = new Contraction(A0, B0, C0, g);
    CC0->contract("a1,k1,b1,k2", "k2,a2,k1,b2", "a1,a2,b1,b2");
    time+=MPI_Wtime();
    if(rank ==0) cout<<"Total time          = "<<time<<endl<<endl;

    A0->~Tensor();
    B0->~Tensor();
    C0->~Tensor();
    CC0->~Contraction();
}

/*
  void contraction_2(int* &pgrid_input, int vgrid_side, int N)
  {
  // Set grid and tensor dimensions
  int grid_dims = 4;
  int dims_A = 4;
  int dims_B = 4;
  int dims_C = 4;

  int serial = grid_dims;

  int size, rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if(rank == 0) cout << "Contraction 2:" << endl;

  // Set physical and virtual grid dimensions

  int *vgrid_A = new int[dims_A];
  int *vgrid_B = new int[dims_B];
  int *vgrid_C = new int[dims_C];

  int* pgrid = pgrid_input;

  assert_pgrid_size(size, grid_dims, pgrid);

  vgrid_A[0] = vgrid_side;
  vgrid_A[1] = vgrid_side;
  vgrid_A[2] = 16;
  vgrid_A[3] = 16;

  vgrid_B[0] = vgrid_side;
  vgrid_B[1] = vgrid_side;
  vgrid_B[2] = 16;
  vgrid_B[3] = 16;

  vgrid_C[0] = vgrid_side;
  vgrid_C[1] = vgrid_side;
  vgrid_C[2] = vgrid_side;
  vgrid_C[3] = vgrid_side;

  // Create idmap for Tensor A and B
  int* idmap_A = new int[dims_A];
  int* idmap_B = new int[dims_B];
  int* idmap_C = new int[dims_C];

  idmap_A[0] = 0;
  idmap_A[1] = 1;
  idmap_A[2] = 2;
  idmap_A[3] = 3;

  idmap_B[0] = 2;//serial;
  idmap_B[1] = 3;//serial;
  idmap_B[2] = 0;
  idmap_B[3] = 1;

  idmap_C[0] = 0;
  idmap_C[1] = 1;
  idmap_C[2] = 2;
  idmap_C[3] = 3;


  // Set Tensor sizes
  int *size_A = new int[dims_A];
  int *size_B = new int[dims_B];
  int *size_C = new int[dims_C];
  for(int i=0; i<dims_A; i++) 
  {

  size_A[i] = N;

  }
  for(int i=0; i<dims_B; i++) 
  {

  size_B[i] = N;
  }
  for(int i=0; i<dims_C; i++) 
  {

  size_C[i] = N;
  }

  double time = 0.0, tensor_time = 0.0, max_tensor_time = 0.0;


  time=- MPI_Wtime();

  tensor_time = -MPI_Wtime();

  Tensor *A1 = new Tensor("aabb", idmap_A, size_A, vgrid_A, grid_dims, pgrid);
  A1->initialize();

  Tensor *B1 = new Tensor("aabb", idmap_B, size_B, vgrid_B, grid_dims, pgrid);
  B1->initialize();

  Tensor *C1 = new Tensor("aabb", idmap_C, size_C, vgrid_C, grid_dims, pgrid);
  C1->initialize();

  tensor_time += MPI_Wtime();


  MPI_Reduce(&tensor_time, &max_tensor_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  if(rank==0) cout << "Tensor Creation time = " << max_tensor_time << endl;

  time=-MPI_Wtime();
  Contraction* CC1 = new Contraction(A1, B1, C1, grid_dims, pgrid);

  if(rank==0) cout << "A[a1>k1,a2>k2] * B[b1>b2,k1>k2] = C0[a1>a2,b1>b2]"<<endl;

  CC1->contract("a1,a2,k1,k2", "b1,b2,k1,k2", "a1,a2,b1,b2");

  time+=MPI_Wtime();

  if(rank ==0) cout<<"Total time          = "<<time<<endl<<endl;

  A1->~Tensor();
  B1->~Tensor();
  C1->~Tensor();
  CC1->~Contraction();
  }

  void contraction_3(int* &pgrid_input, int vgrid_side, int N)
  {
  // Set grid and tensor dimensions
  int grid_dims = 4;
  int dims_A = 4;
  int dims_B = 4;
  int dims_C = 4;

  int serial = grid_dims;

  int size, rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if(rank == 0) cout << "Contraction 3:" << endl;

  // Set physical and virtual grid dimensions

  int *vgrid_A = new int[dims_A];
  int *vgrid_B = new int[dims_B];
  int *vgrid_C = new int[dims_C];

  int* pgrid = pgrid_input;

  assert_pgrid_size(size, grid_dims, pgrid);

  vgrid_A[0] = vgrid_side;
  vgrid_A[1] = vgrid_side;
  vgrid_A[2] = vgrid_side;
  vgrid_A[3] = vgrid_side;

  vgrid_B[0] = vgrid_side;
  vgrid_B[1] = vgrid_side;
  vgrid_B[2] = vgrid_side;
  vgrid_B[3] = vgrid_side;

  vgrid_C[0] = vgrid_side;
  vgrid_C[1] = vgrid_side;
  vgrid_C[2] = vgrid_side;
  vgrid_C[3] = vgrid_side;

  // Create idmap for Tensor A and B
  int* idmap_A = new int[dims_A];
  int* idmap_B = new int[dims_B];
  int* idmap_C = new int[dims_C];

  idmap_A[0] = 0;
  idmap_A[1] = 1;
  idmap_A[2] = 2;
  idmap_A[3] = 3;

  idmap_B[0] = 0;//serial;
  idmap_B[1] = 1;//serial;
  idmap_B[2] = 2;
  idmap_B[3] = 3;

  idmap_C[0] = 0;
  idmap_C[1] = 1;
  idmap_C[2] = 2;
  idmap_C[3] = 3;


  // Set Tensor sizes
  int *size_A = new int[dims_A];
  int *size_B = new int[dims_B];
  int *size_C = new int[dims_C];
  for(int i=0; i<dims_A; i++) 
  {

  size_A[i] = N;

  }
  for(int i=0; i<dims_B; i++) 
  {

  size_B[i] = N;
  }
  for(int i=0; i<dims_C; i++) 
  {

  size_C[i] = N;
  }

  double time = 0.0, tensor_time = 0.0, max_tensor_time = 0.0;



  time=- MPI_Wtime();

  tensor_time = -MPI_Wtime();

  Tensor *A2 = new Tensor("aabb", idmap_A, size_A, vgrid_A, grid_dims, pgrid);
  A2->initialize();

  Tensor *B2 = new Tensor("cdcd", idmap_B, size_B, vgrid_B, grid_dims, pgrid);
  B2->initialize();

  Tensor *C2 = new Tensor("cdcd", idmap_C, size_C, vgrid_C, grid_dims, pgrid);
  C2->initialize();

  tensor_time += MPI_Wtime();


  MPI_Reduce(&tensor_time, &max_tensor_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  if(rank==0) cout << "Tensor Creation time = " << max_tensor_time << endl;

  if(rank==0) cout << "A[a1>k1,a2>k2] * B[k2,b1,k1,b2] = C0[a1,b1,a2,b2]"<<endl;
  time=-MPI_Wtime();
  Contraction* CC2 = new Contraction(A2, B2, C2, grid_dims, pgrid);
  CC2->contract("a1,k1,a2,k2", "k2,b1,k1,b2", "a1,b1,a2,b2");

  time+=MPI_Wtime();

  if(rank ==0) cout<<"Total time          = "<<time<<endl<<endl;

  A2->~Tensor();
  B2->~Tensor();
  C2->~Tensor();
  CC2->~Contraction();

  }



  void contraction_4(int* &pgrid_input, int vgrid_side, int N)
  {
  // Set grid and tensor dimensions
  int grid_dims = 4;
  int dims_A = 4;
  int dims_B = 4;
  int dims_C = 4;

  int serial = grid_dims;

  int size, rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if(rank == 0) cout << "Contraction 4:" << endl;

  // Set physical and virtual grid dimensions

  int *vgrid_A = new int[dims_A];
  int *vgrid_B = new int[dims_B];
  int *vgrid_C = new int[dims_C];

  int* pgrid = pgrid_input;

  assert_pgrid_size(size, grid_dims, pgrid);

  vgrid_A[0] = vgrid_side;
  vgrid_A[1] = vgrid_side;
  vgrid_A[2] = vgrid_side;
  vgrid_A[3] = vgrid_side;

  vgrid_B[0] = vgrid_side;
  vgrid_B[1] = vgrid_side;
  vgrid_B[2] = vgrid_side;
  vgrid_B[3] = vgrid_side;

  vgrid_C[0] = vgrid_side;
  vgrid_C[1] = vgrid_side;
  vgrid_C[2] = vgrid_side;
  vgrid_C[3] = vgrid_side;

  // Create idmap for Tensor A and B
  int* idmap_A = new int[dims_A];
  int* idmap_B = new int[dims_B];
  int* idmap_C = new int[dims_C];

  idmap_A[0] = 0;
  idmap_A[1] = 1;
  idmap_A[2] = 2;
  idmap_A[3] = 3;

  idmap_B[0] = 0;//serial;
  idmap_B[1] = 1;//serial;
  idmap_B[2] = 2;
  idmap_B[3] = 3;

  idmap_C[0] = 0;
  idmap_C[1] = 1;
  idmap_C[2] = 2;
  idmap_C[3] = 3;


  // Set Tensor sizes
  int *size_A = new int[dims_A];
  int *size_B = new int[dims_B];
  int *size_C = new int[dims_C];
  for(int i=0; i<dims_A; i++) 
  {

  size_A[i] = N;

  }
  for(int i=0; i<dims_B; i++) 
  {

  size_B[i] = N;
  }
  for(int i=0; i<dims_C; i++) 
  {

  size_C[i] = N;
  }

  double time = 0.0, tensor_time = 0.0, max_tensor_time = 0.0;



  time=- MPI_Wtime();

  tensor_time = -MPI_Wtime();

  Tensor *A2 = new Tensor("aabb", idmap_A, size_A, vgrid_A, grid_dims, pgrid);
  A2->initialize();

  Tensor *B2 = new Tensor("cdbb", idmap_B, size_B, vgrid_B, grid_dims, pgrid);
  B2->initialize();

  Tensor *C2 = new Tensor("cdcd", idmap_C, size_C, vgrid_C, grid_dims, pgrid);
  C2->initialize();

  tensor_time += MPI_Wtime();


  MPI_Reduce(&tensor_time, &max_tensor_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  if(rank==0) cout << "Tensor Creation time = " << max_tensor_time << endl;

  if(rank==0) cout << "A[a1>k1,a2>k2] * B[k2,b1,k1>b2] = C0[a1,b1,a2,b2]"<<endl;
  time=-MPI_Wtime();
  Contraction* CC2 = new Contraction(A2, B2, C2, grid_dims, pgrid);
  CC2->contract("a1,k1,a2,k2", "k2,b1,k1,b2", "a1,b1,a2,b2");

  time+=MPI_Wtime();

  if(rank ==0) cout<<"Total time          = "<<time<<endl<<endl;

  A2->~Tensor();
  B2->~Tensor();
  C2->~Tensor();
  CC2->~Contraction();

  }



  void contraction_5(int* &pgrid_input, int vgrid_side, int N)
  {
  // Set grid and tensor dimensions
  int grid_dims = 2;
  int dims_A = 2;
  int dims_B = 2;
  int dims_C = 2;

  int serial = grid_dims;

  int size, rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if(rank == 0) cout << "Contraction 5:" << endl;

  // Set physical and virtual grid dimensions

  int *vgrid_A = new int[dims_A];
  int *vgrid_B = new int[dims_B];
  int *vgrid_C = new int[dims_C];

  int* pgrid = pgrid_input;

  assert_pgrid_size(size, grid_dims, pgrid);

  vgrid_A[0] = vgrid_side;
  vgrid_A[1] = vgrid_side;

  vgrid_B[0] = vgrid_side;
  vgrid_B[1] = vgrid_side;

  vgrid_C[0] = vgrid_side;
  vgrid_C[1] = vgrid_side;

  // Create idmap for Tensor A and B
  int* idmap_A = new int[dims_A];
  int* idmap_B = new int[dims_B];
  int* idmap_C = new int[dims_C];

  idmap_A[0] = 0;
  idmap_A[1] = 1;

  idmap_B[0] = 0;//serial;
  idmap_B[1] = 1;//serial;

  idmap_C[0] = 0;
  idmap_C[1] = 1;


  // Set Tensor sizes
  int *size_A = new int[dims_A];
  int *size_B = new int[dims_B];
  int *size_C = new int[dims_C];
  for(int i=0; i<dims_A; i++) 
  {

  size_A[i] = N;

  }
  for(int i=0; i<dims_B; i++) 
  {

  size_B[i] = N;
  }
  for(int i=0; i<dims_C; i++) 
  {

  size_C[i] = N;
  }

  double time = 0.0, tensor_time = 0.0, max_tensor_time = 0.0;



  time=- MPI_Wtime();

  tensor_time = -MPI_Wtime();

  Tensor *A2 = new Tensor("aa", idmap_A, size_A, vgrid_A, grid_dims, pgrid);
  A2->initialize();

  Tensor *B2 = new Tensor("aa", idmap_B, size_B, vgrid_B, grid_dims, pgrid);
  B2->initialize();

  Tensor *C2 = new Tensor("cd", idmap_C, size_C, vgrid_C, grid_dims, pgrid);
  C2->initialize();

  tensor_time += MPI_Wtime();


  MPI_Reduce(&tensor_time, &max_tensor_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  if(rank==0) cout << "Tensor Creation time = " << max_tensor_time << endl;

  if(rank==0) cout << "A[a1>k1] * B[k1>b1] = C0[a1,b1]"<<endl;
  time=-MPI_Wtime();
  Contraction* CC2 = new Contraction(A2, B2, C2, grid_dims, pgrid);
  CC2->contract("a1,k1", "k1,b1", "a1,b1");

  time+=MPI_Wtime();

  if(rank ==0) cout<<"Total time          = "<<time<<endl<<endl;

  //A2->~Tensor();
  //B2->~Tensor();
  //C2->~Tensor();
  //CC2->~Contraction();

  }


  void contraction_6(int* &pgrid_input, int vgrid_side, int N)
  {
  // Set grid and tensor dimensions
  int grid_dims = 4;
  int dims_A = 4;
  int dims_B = 4;
  int dims_C = 4;

  int serial = grid_dims;

  int size, rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if(rank == 0) cout << "Contraction 6:" << endl;

  // Set physical and virtual grid dimensions

  int *vgrid_A = new int[dims_A];
  int *vgrid_B = new int[dims_B];
  int *vgrid_C = new int[dims_C];

  int* pgrid = pgrid_input;

  assert_pgrid_size(size, grid_dims, pgrid);

  vgrid_A[0] = vgrid_side;
  vgrid_A[1] = vgrid_side;
  vgrid_A[2] = vgrid_side;
  vgrid_A[3] = vgrid_side;

  vgrid_B[0] = vgrid_side;
  vgrid_B[1] = vgrid_side;
  vgrid_B[2] = vgrid_side;
  vgrid_B[3] = vgrid_side;

  vgrid_C[0] = vgrid_side;
  vgrid_C[1] = vgrid_side;
  vgrid_C[2] = vgrid_side;
  vgrid_C[3] = vgrid_side;

  // Create idmap for Tensor A and B
  int* idmap_A = new int[dims_A];
  int* idmap_B = new int[dims_B];
  int* idmap_C = new int[dims_C];

  idmap_A[0] = 0;
  idmap_A[1] = 1;
  idmap_A[2] = 2;
  idmap_A[3] = 3;

  idmap_B[0] = 0;//serial;
  idmap_B[1] = 1;//serial;
  idmap_B[2] = 2;
  idmap_B[3] = 3;

  idmap_C[0] = 0;
  idmap_C[1] = 1;
  idmap_C[2] = 2;
  idmap_C[3] = 3;


  // Set Tensor sizes
  int *size_A = new int[dims_A];
  int *size_B = new int[dims_B];
  int *size_C = new int[dims_C];
  for(int i=0; i<dims_A; i++) 
  {

  size_A[i] = N;

  }
  for(int i=0; i<dims_B; i++) 
  {

  size_B[i] = N;
  }
  for(int i=0; i<dims_C; i++) 
  {

  size_C[i] = N;
  }

  double time = 0.0, tensor_time = 0.0, max_tensor_time = 0.0;



  time=- MPI_Wtime();

  tensor_time = -MPI_Wtime();

  Tensor *A2 = new Tensor("aabb", idmap_A, size_A, vgrid_A, grid_dims, pgrid);
  A2->initialize();

  Tensor *B2 = new Tensor("aabb", idmap_B, size_B, vgrid_B, grid_dims, pgrid);
  B2->initialize();

  Tensor *C2 = new Tensor("cdcd", idmap_C, size_C, vgrid_C, grid_dims, pgrid);
  C2->initialize();

  tensor_time += MPI_Wtime();


  MPI_Reduce(&tensor_time, &max_tensor_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  if(rank==0) cout << "Tensor Creation time = " << max_tensor_time << endl;

  if(rank==0) cout << "A[a1>k1,a2>k2] * B[k2>b1,k1>b2] = C0[a1,b1,a2,b2]"<<endl;
  time=-MPI_Wtime();
  Contraction* CC2 = new Contraction(A2, B2, C2, grid_dims, pgrid);
  CC2->contract("a1,k1,a2,k2", "k2,b1,k1,b2", "a1,b1,a2,b2");

  time+=MPI_Wtime();

  if(rank ==0) cout<<"Total time          = "<<time<<endl<<endl;

  A2->~Tensor();
  B2->~Tensor();
  C2->~Tensor();
  CC2->~Contraction();

  }


  void contraction_7(int* &pgrid_input, int vgrid_side, int N)
  {
  // Set grid and tensor dimensions
  int grid_dims = 4;
  int dims_A = 4;
  int dims_B = 2;
  int dims_C = 4;

  int serial = grid_dims;

  int size, rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if(rank == 0) cout << "Contraction 7:" << endl;

  // Set physical and virtual grid dimensions
    
  int *vgrid_A = new int[dims_A];
  int *vgrid_B = new int[dims_B];
  int *vgrid_C = new int[dims_C];

  int* pgrid = pgrid_input;

  assert_pgrid_size(size, grid_dims, pgrid);

  vgrid_A[0] = vgrid_side;
  vgrid_A[1] = vgrid_side;
  vgrid_A[2] = vgrid_side;
  vgrid_A[3] = vgrid_side;

  vgrid_B[0] = vgrid_side;
  vgrid_B[1] = vgrid_side;

  vgrid_C[0] = vgrid_side;
  vgrid_C[1] = vgrid_side;
  vgrid_C[2] = vgrid_side;
  vgrid_C[3] = vgrid_side;

  // Create idmap for Tensor A and B
  int* idmap_A = new int[dims_A];
  int* idmap_B = new int[dims_B];
  int* idmap_C = new int[dims_C];

  idmap_A[0] = 2;
  idmap_A[1] = 3;
  idmap_A[2] = 1;
  idmap_A[3] = 0;

  idmap_B[0] = serial;
  idmap_B[1] = serial;

  idmap_C[0] = 1;
  idmap_C[1] = 0;
  idmap_C[2] = 2;
  idmap_C[3] = 3;

  // Set Tensor sizes
  int *size_A = new int[dims_A];
  int *size_B = new int[dims_B];
  int *size_C = new int[dims_C];
  for(int i=0; i<dims_A; i++) 
  {
  size_A[i] = N;
  }
  for(int i=0; i<dims_B; i++) 
  {
  size_B[i] = N;
  }
  for(int i=0; i<dims_C; i++) 
  {
  size_C[i] = N;
  }

  // Create input tensors
  double time = 0.0, tensor_time = 0.0, max_tensor_time = 0.0;

  time=- MPI_Wtime();

  tensor_time = -MPI_Wtime();

  Tensor *A = new Tensor("ccbb", idmap_A, size_A, vgrid_A, grid_dims, pgrid);
  A->initialize();

  Tensor *B = new Tensor("aa", idmap_B, size_B, vgrid_B, grid_dims, pgrid);
  B->initialize();

  Tensor *C = new Tensor("ccdd", idmap_C, size_C, vgrid_C, grid_dims, pgrid);
  C->initialize();

  tensor_time += MPI_Wtime();


  MPI_Reduce(&tensor_time, &max_tensor_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  if(rank==0) cout << "Tensor Creation time = " << max_tensor_time << endl;

  if(rank==0) cout << "A[a1,a2,a3>k1] * B[b1>k1] = C0[a1,a2,b1,a3]"<<endl;

  time=-MPI_Wtime();
  Contraction* C1 = new Contraction(A, B, C, grid_dims, pgrid);
  C1->contract("a1,a2,a3,k1", "b1,k1", "a1,a2,b1,a3");
  time+=MPI_Wtime();

  if(rank ==0) cout<<"Total time          = "<<time<<endl<<endl;

  A->~Tensor();
  B->~Tensor();
  C->~Tensor();
  C1->~Contraction();
  }



  void contraction_4_1(int* &pgrid_input, int vgrid_side, int N)
  {
  // Set grid and tensor dimensions
  int grid_dims = 4;
  int dims_A = 2;
  int dims_B = 4;
  int dims_C = 4;

  int serial = grid_dims;

  int size, rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if(rank == 0) cout << "Contraction 4:" << endl;

  // Set physical and virtual grid dimensions

  int *vgrid_A = new int[dims_A];
  int *vgrid_B = new int[dims_B];
  int *vgrid_C = new int[dims_C];

  int* pgrid = pgrid_input;

  assert_pgrid_size(size, grid_dims, pgrid);

  vgrid_A[0] = vgrid_side;
  vgrid_A[1] = vgrid_side;

  vgrid_B[0] = vgrid_side;
  vgrid_B[1] = vgrid_side;
  vgrid_B[2] = vgrid_side;
  vgrid_B[3] = vgrid_side;

  vgrid_C[0] = vgrid_side;
  vgrid_C[1] = vgrid_side;
  vgrid_C[2] = vgrid_side;
  vgrid_C[3] = vgrid_side;

  // Create idmap for Tensor A and B
  int* idmap_A = new int[dims_A];
  int* idmap_B = new int[dims_B];
  int* idmap_C = new int[dims_C];

  idmap_A[0] = serial;
  idmap_A[1] = serial;

  idmap_B[0] = 0;//serial;
  idmap_B[1] = 1;//serial;
  idmap_B[2] = 2;
  idmap_B[3] = 3;

  idmap_C[0] = 0;
  idmap_C[1] = 1;
  idmap_C[2] = 2;
  idmap_C[3] = 3;


  // Set Tensor sizes
  int *size_A = new int[dims_A];
  int *size_B = new int[dims_B];
  int *size_C = new int[dims_C];
  for(int i=0; i<dims_A; i++) 
  {
  size_A[i] = N;
  }
  for(int i=0; i<dims_B; i++) 
  {
  size_B[i] = N;
  }
  for(int i=0; i<dims_C; i++) 
  {
  size_C[i] = N;
  }

  // Create input tensors
  double time=- MPI_Wtime();
  double tensor_time = -MPI_Wtime();
  double max_tensor_time;

  Tensor *A3 = new Tensor("cd", idmap_A, size_A, vgrid_A, grid_dims, pgrid);
  A3->initialize();

  Tensor *B3 = new Tensor("cdaa", idmap_B, size_B, vgrid_B, grid_dims, pgrid);
  B3->initialize();

  Tensor *C3 = new Tensor("cdaa", idmap_C, size_C, vgrid_C, grid_dims, pgrid);
  C3->initialize();

  time=- MPI_Wtime();

  tensor_time += MPI_Wtime();

  MPI_Reduce(&tensor_time, &max_tensor_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  if(rank==0) cout << "Tensor Creation time = " << max_tensor_time << endl;

  // Create contraction and run

  time=-MPI_Wtime();
  Contraction* CC3 = new Contraction(A3, B3, C3, grid_dims, pgrid);
  CC3->contract("a1,k1", "k1,a2,b1,b2", "a1,a2,b1,b2");
  time+=MPI_Wtime();
  if(rank ==0) cout<<"Total time          = "<<time<<endl<<endl;

  A3->~Tensor();
  B3->~Tensor();
  C3->~Tensor();
  CC3->~Contraction();
  }

  void contraction_5_1(int* &pgrid_input, int vgrid_side, int N)
  {
  // Set grid and tensor dimensions
  int grid_dims = 4;
  int dims_A = 2;
  int dims_B = 4;
  int dims_C = 4;

  int serial = grid_dims;

  int size, rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if(rank == 0) cout << "Contraction 5:" << endl;

  // Set physical and virtual grid dimensions

  int *vgrid_A = new int[dims_A];
  int *vgrid_B = new int[dims_B];
  int *vgrid_C = new int[dims_C];

  int* pgrid = pgrid_input;

  assert_pgrid_size(size, grid_dims, pgrid);

  vgrid_A[0] = vgrid_side;
  vgrid_A[1] = vgrid_side;

  vgrid_B[0] = vgrid_side;
  vgrid_B[1] = vgrid_side;
  vgrid_B[2] = vgrid_side;
  vgrid_B[3] = vgrid_side;

  vgrid_C[0] = vgrid_side;
  vgrid_C[1] = vgrid_side;
  vgrid_C[2] = vgrid_side;
  vgrid_C[3] = vgrid_side;

  // Create idmap for Tensor A and B
  int* idmap_A = new int[dims_A];
  int* idmap_B = new int[dims_B];
  int* idmap_C = new int[dims_C];

  idmap_A[0] = serial;
  idmap_A[1] = serial;

  idmap_B[0] = 0;//serial;
  idmap_B[1] = 1;//serial;
  idmap_B[2] = 2;
  idmap_B[3] = 3;

  idmap_C[0] = 0;
  idmap_C[1] = 1;
  idmap_C[2] = 2;
  idmap_C[3] = 3;


  // Set Tensor sizes
  int *size_A = new int[dims_A];
  int *size_B = new int[dims_B];
  int *size_C = new int[dims_C];
  for(int i=0; i<dims_A; i++) 
  {
  size_A[i] = N;
  }
  for(int i=0; i<dims_B; i++) 
  {
  size_B[i] = N;
  }
  for(int i=0; i<dims_C; i++) 
  {
  size_C[i] = N;
  }


  double time = 0.0, tensor_time = 0.0, max_tensor_time = 0.0;

  time=- MPI_Wtime();

  tensor_time = -MPI_Wtime();

  Tensor *A4 = new Tensor("cd", idmap_A, size_A, vgrid_A, grid_dims, pgrid);
  A4->initialize();

  Tensor *B4 = new Tensor("cdaa", idmap_B, size_B, vgrid_B, grid_dims, pgrid);
  B4->initialize();

  Tensor *C4 = new Tensor("cdcd", idmap_C, size_C, vgrid_C, grid_dims, pgrid);
  C4->initialize();

  tensor_time += MPI_Wtime();


  MPI_Reduce(&tensor_time, &max_tensor_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  if(rank==0) cout << "Tensor Creation time = " << max_tensor_time << endl;

  time=-MPI_Wtime();
  Contraction* CC4 = new Contraction(A4, B4, C4, grid_dims, pgrid);
  CC4->contract("k1,b1", "a1,a2,k1,a3", "a1,a2,a3,b1");

  time+=MPI_Wtime();

  if(rank ==0) cout<<"Total time          = "<<time<<endl<<endl;

  A4->~Tensor();
  B4->~Tensor();
  C4->~Tensor();
  CC4->~Contraction();
  }



  void contraction_2d_2d(int* &pgrid_input, int vgrid_side, int N)
  {
  // Set grid and tensor dimensions
  int grid_dims = 2;
  int dims_A = 2;
  int dims_B = 2;
  int dims_C = 2;

  int serial = grid_dims;

  int size, rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // Set physical and virtual grid dimensions
  int *pgrid = pgrid_input;
  int *vgrid_A = new int[dims_A];
  int *vgrid_B = new int[dims_B];
  int *vgrid_C = new int[dims_C];

  assert_pgrid_size(size, grid_dims, pgrid);

  vgrid_A[0] = vgrid_side;
  vgrid_A[1] = vgrid_side;

  vgrid_B[0] = vgrid_side;
  vgrid_B[1] = vgrid_side;

  vgrid_C[0] = vgrid_side;
  vgrid_C[1] = vgrid_side;

  // Create idmap for Tensor A and B
  int* idmap_A = new int[dims_A];
  int* idmap_B = new int[dims_B];
  int* idmap_C = new int[dims_C];

  idmap_A[0] = serial;
  idmap_A[1] = serial;

  idmap_B[0] = serial;
  idmap_B[1] = serial;

  idmap_C[0] = 2;
  idmap_C[1] = 3;

  // Set Tensor sizes
  int *size_A = new int[dims_A];
  int *size_B = new int[dims_B];
  int *size_C = new int[dims_C];
  for(int i=0; i<dims_A; i++) 
  {
  size_A[i] = N;
  }
  for(int i=0; i<dims_B; i++) 
  {
  size_B[i] = N;
  }
  for(int i=0; i<dims_C; i++) 
  {
  size_C[i] = N;
  }

  double time = 0.0, tensor_time = 0.0, max_tensor_time = 0.0;

  time=- MPI_Wtime();

  tensor_time = -MPI_Wtime();

  // Create input tensors
  Tensor *A = new Tensor("aa", idmap_A, size_A, vgrid_A, grid_dims, pgrid);
  A->initialize();

  Tensor *B = new Tensor("aa", idmap_B, size_B, vgrid_B, grid_dims, pgrid);
  B->initialize();

  Tensor *C = new Tensor("cd", idmap_C, size_C, vgrid_C, grid_dims, pgrid);
  C->initialize();

  tensor_time += MPI_Wtime();


  MPI_Reduce(&tensor_time, &max_tensor_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  if(rank==0) cout << "Tensor Creation time = " << max_tensor_time << endl;

  time=-MPI_Wtime();

  Contraction* C1 = new Contraction(A, B, C, grid_dims, pgrid);
  C1->contract("a1,k1", "b1,k1", "a1,b1");


  time+=MPI_Wtime();
  C1->~Contraction();

  if(rank ==0) cout<<"Total time          = "<<time<<endl<<endl;

  A->~Tensor();
  B->~Tensor();
  C->~Tensor();
  }

  void contraction_2d_4d()
  {
  // Set grid and tensor dimensions
  int grid_dims = 4;
  int dims_A = 4;
  int dims_B = 2;
  int dims_C = 4;

  int serial = grid_dims;

  int size, rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // Set physical and virtual grid dimensions
  int *pgrid = new int[grid_dims];
  int *vgrid_A = new int[dims_A];
  int *vgrid_B = new int[dims_B];
  int *vgrid_C = new int[dims_C];

  pgrid[0] = 1;
  pgrid[1] = 1;
  pgrid[2] = 4;
  pgrid[3] = 4;

  assert_pgrid_size(size, grid_dims, pgrid);

  vgrid_A[0] = 8;
  vgrid_A[1] = 8;
  vgrid_A[2] = 8;
  vgrid_A[3] = 8;

  vgrid_B[0] = 8;
  vgrid_B[1] = 8;

  vgrid_C[0] = 8;
  vgrid_C[1] = 8;
  vgrid_C[2] = 8;
  vgrid_C[3] = 8;

  // Create idmap for Tensor A and B
  int* idmap_A = new int[dims_A];
  int* idmap_B = new int[dims_B];
  int* idmap_C = new int[dims_C];

  idmap_A[0] = 2;
  idmap_A[1] = 3;
  idmap_A[2] = 1;
  idmap_A[3] = 0;

  idmap_B[0] = serial;
  idmap_B[1] = serial;

  idmap_C[0] = 1;
  idmap_C[1] = 0;
  idmap_C[2] = 2;
  idmap_C[3] = 3;

  //int* new_pgrid = new int[dims];
  //  new_pgrid[0] = 1;
  //  new_pgrid[1] = 8;
  //  new_pgrid[2] = 1;
  //  new_pgrid[3] = 8;


  // Set Tensor sizes
  int *size_A = new int[dims_A];
  int *size_B = new int[dims_B];
  int *size_C = new int[dims_C];
  for(int i=0; i<dims_A; i++) 
  {
  size_A[i] = 32;
  }
  for(int i=0; i<dims_B; i++) 
  {
  size_B[i] = 32;
  }
  for(int i=0; i<dims_C; i++) 
  {
  size_C[i] = 32;
  }

  // Create input tensors
  Tensor *A = new Tensor("aabb", idmap_A, size_A, vgrid_A, grid_dims, pgrid);
  A->initialize();

  Tensor *B = new Tensor("aa", idmap_B, size_B, vgrid_B, grid_dims, pgrid);
  B->initialize();

  Tensor *C = new Tensor("aacd", idmap_C, size_C, vgrid_C, grid_dims, pgrid);
  C->initialize();

  Contraction* C1 = new Contraction(A, B, C, grid_dims, pgrid);
  C1->contract("a1,a2,a3,k1", "b1,k1", "a1,a2,b1,a3");
  C1->~Contraction();

  A->~Tensor();
  B->~Tensor();
  C->~Tensor();
  }

  void test_transpose(int N, int ii0, int i1, int i2, int i3)
  {

  int size, rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int grid_dims = 4;
  int dims_A = 4;

  int *pgrid = new int[grid_dims];
  int *vgrid_A = new int[dims_A];
  int* idmap_A = new int[dims_A];
  int *size_A = new int[dims_A];

  pgrid[0] = 1;
  pgrid[1] = 1;
  pgrid[2] = 1;
  pgrid[3] = 1;

  vgrid_A[0] = 1;
  vgrid_A[1] = 1;
  vgrid_A[2] = 1;
  vgrid_A[3] = 1;

  idmap_A[0] = 0;
  idmap_A[1] = 1;
  idmap_A[2] = 2;
  idmap_A[3] = 3;

  size_A[0] = N;
  size_A[1] = N;
  size_A[2] = N;
  size_A[3] = N;


  // Create input tensors
  Tensor *A = new Tensor("aabb", idmap_A, size_A, vgrid_A, grid_dims, pgrid);
  A->initialize();
  int block_size = A->get_block_size();    
  double* initial_tile = new double[A->get_block_size()];
  int* block_range = A->get_block_range();

  for(int i0 = 0; i0<block_range[0]; i0++)    
  for(int i1 = 0; i1<block_range[1]; i1++)
  for(int i2 = 0; i2<block_range[2]; i2++)
  for(int i3 = 0; i3<block_range[3]; i3++)
  initial_tile[i0 * block_range[3]*block_range[2]*block_range[1]
  + i1 * block_range[3]*block_range[2]
  + i2 * block_range[3] + i3] = (double)(i0 + i1 + i2 + i3);


  double* permuted_tile = new double[A->get_block_size()];

  int* permutation_map = new int[dims_A];

  permutation_map[0] = ii0;
  permutation_map[1] = i1;
  permutation_map[2] = i2;
  permutation_map[3] = i3;

  for(int i0 = 0; i0<block_range[permutation_map[0]]; i0++)
  for(int i1 = 0; i1<block_range[permutation_map[1]]; i1++)
  for(int i2 = 0; i2<block_range[permutation_map[2]]; i2++)
  for(int i3 = 0; i3<block_range[permutation_map[3]]; i3++)
  permuted_tile[i0 * block_range[permutation_map[3]]*block_range[permutation_map[2]]*block_range[permutation_map[1]]
  + i1 * block_range[permutation_map[3]]*block_range[permutation_map[2]]
  + i2 * block_range[permutation_map[3]] + i3] = i0 + i1 + i2 + i3;

  double* transpose_output_tile = new double[A->get_block_size()];

  double time =-MPI_Wtime();
  transpose(initial_tile, transpose_output_tile, permutation_map, A->get_dims(), block_range);
  time +=MPI_Wtime();
  cout<<"Transpose Time is : "<<time<<endl;

  if(is_equal_blocks(N*N*N*N, permuted_tile, transpose_output_tile)){
  cout<<"Passed"<<endl;
  }
  //    cout<<"Initial Tile : "<<endl;
  //print_block_data(block_size, initial_tile);
  //cout<<"Permuted Tile : "<<endl;
  //print_block_data(block_size, permuted_tile);
  //cout<<"Transpose Tile : "<<endl;
  //print_block_data(block_size, transpose_output_tile);

  }

  void test_contract_4d_4d()
  {


  int dims = 4;
  int size, rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // Set physical and virtual grid dimensions
  int *pgrid = new int[dims];
  int *vgrid_A = new int[dims];
  int *vgrid_B = new int[dims];
  int *vgrid_C = new int[dims];

  pgrid[0] = 1;
  pgrid[1] = 1;
  pgrid[2] = 4;
  pgrid[3] = 4;


  vgrid_A[0] = 2;
  vgrid_A[1] = 2;
  vgrid_A[2] = 8;
  vgrid_A[3] = 8;

  vgrid_B[0] = 4;
  vgrid_B[1] = 4;
  vgrid_B[2] = 8;
  vgrid_B[3] = 8;

  vgrid_C[0] = 4;
  vgrid_C[1] = 8;
  vgrid_C[2] = 4;
  vgrid_C[3] = 8;

  // Create idmap for Tensor A and B
  int* idmap_A = new int[dims];
  int* idmap_B = new int[dims];
  int* idmap_C = new int[dims];

  idmap_C[0] = 0;
  idmap_C[1] = 2;
  idmap_C[2] = 3;
  idmap_C[3] = 1;

  idmap_A[0] = 0;
  idmap_A[1] = 2;
  idmap_A[2] = 1;
  idmap_A[3] = 3;

  idmap_B[0] = 0;
  idmap_B[1] = 2;
  idmap_B[2] = 1;
  idmap_B[3] = 3;

  // Set Tensor sizes
  int *size_A = new int[dims];
  int *size_B = new int[dims];
  int *size_C = new int[dims];

  for(int i=0; i<dims; i++) 
  {
  size_A[i] = size_B[i] = size_C[i] = 16;
  }

  // Create input tensors
  Tensor *A = new Tensor("abab", idmap_A, size_A, vgrid_A, dims, pgrid);
  A->initialize();

  Tensor *B = new Tensor("abab", idmap_B, size_B, vgrid_B, dims, pgrid);
  B->initialize();

  Tensor *C = new Tensor("abab", idmap_C, size_C, vgrid_C, dims, pgrid);
  C->initialize();

  Contraction* C1 = new Contraction(A, B, C, dims, pgrid);
  //    C1->contract("a1,k1,a2,k2", "c1,k1,c2,k2", "c1,a1,a2,c2");
  }

  void serialize_test()
  {
  int dims = 2;
  int grid_dims = 4;

  int size, rank;
  int serial = grid_dims;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // Set physical and virtual grid dimensions
  int *pgrid = new int[grid_dims];
  int *vgrid_A = new int[dims];
  int *vgrid_B = new int[dims];
  int *vgrid_C = new int[dims];

  pgrid[0] = 1;
  pgrid[1] = 1;
  pgrid[2] = 4;
  pgrid[3] = 4;

  assert_pgrid_size(size, grid_dims, pgrid);

  vgrid_A[0] = 8;
  vgrid_A[1] = 8;

  int* idmap_A = new int[dims];
  idmap_A[0] = 2;
  idmap_A[1] = 3;

  int* new_idmap = new int[dims];
  new_idmap[0] = serial;
  new_idmap[1] = 3;

  int *size_A = new int[dims];
  for(int i=0; i<dims; i++) 
  {
  size_A[i] = 16;
  }

  Tensor *A = new Tensor("ab", idmap_A, size_A, vgrid_A, grid_dims, pgrid);
  A->initialize();

  TensorRedistributor* td = new TensorRedistributor(A, new_idmap, grid_dims, pgrid);
  td->redistribute();
  }

  void multiple_contr_test()
  {
  int Oa = 120, Ov = 120;

  // Set grid and tensor dimensions
  int grid_dims_2 = 2;
  int grid_dims_4 = 4;
  int dims_A1 = 2;
  int dims_A2 = 2;
  int dims_C1 = 2;
  int dims_A3 = 4;
  int dims_D3 = 4;

  int serial_2d = grid_dims_2;
  int serial_4d = grid_dims_4;

  int size, rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // Set physical and virtual grid dimensions
  int *pgrid_2d = new int[grid_dims_2];
  int *pgrid_4d = new int[grid_dims_4];
  int *vgrid_A1 = new int[dims_A1];
  int *vgrid_A2 = new int[dims_A2];
  int *vgrid_C1 = new int[dims_C1];
  int *vgrid_A3 = new int[dims_A3];
  int *vgrid_D3 = new int[dims_D3];

  pgrid_2d[0] = 4;
  pgrid_2d[1] = 4;

  pgrid_4d[0] = 2;
  pgrid_4d[1] = 2;
  pgrid_4d[2] = 2;
  pgrid_4d[3] = 2;

  assert_pgrid_size(size, grid_dims_2, pgrid_2d);
  assert_pgrid_size(size, grid_dims_4, pgrid_4d);

  vgrid_A1[0] = 8;
  vgrid_A1[1] = 8;

  vgrid_A2[0] = 8;
  vgrid_A2[1] = 8;

  vgrid_C1[0] = 8;
  vgrid_C1[1] = 8;

  vgrid_A3[0] = 8;
  vgrid_A3[1] = 8;
  vgrid_A3[2] = 8;
  vgrid_A3[3] = 8;

  vgrid_D3[0] = 8;
  vgrid_D3[1] = 8;
  vgrid_D3[2] = 8;
  vgrid_D3[3] = 8;

  // Create idmap for Tensor A and A2
  int* idmap_A1 = new int[dims_A1];
  int* idmap_A2 = new int[dims_A2];
  int* idmap_C1 = new int[dims_C1];
  int* idmap_A3 = new int[dims_A3];
  int* idmap_D3 = new int[dims_D3];

  idmap_A1[0] = serial_2d;
  idmap_A1[1] = serial_2d;

  idmap_A2[0] = serial_2d;
  idmap_A2[1] = serial_2d;

  idmap_C1[0] = serial_2d;
  idmap_C1[1] = serial_2d;

  idmap_A3[0] = 0;
  idmap_A3[1] = 1;
  idmap_A3[2] = 2;
  idmap_A3[3] = 3;

  idmap_D3[0] = 0;
  idmap_D3[1] = 1;
  idmap_D3[2] = 2;
  idmap_D3[3] = 3;

  // Set Tensor sizes
  int *size_A1 = new int[dims_A1];
  int *size_A2 = new int[dims_A2];
  int *size_C1 = new int[dims_C1];
  int *size_A3 = new int[dims_A3];
  int *size_D3 = new int[dims_D3];
  for(int i=0; i<dims_A1; i++) 
  size_A1[i] = Oa;
  for(int i=0; i<dims_A2; i++) 
  size_A2[i] = Oa;
  for(int i=0; i<dims_C1; i++) 
  size_C1[i] = Oa;

  size_A3[0] = Oa;
  size_A3[1] = Oa;
  size_A3[2] = Ov;
  size_A3[3] = Ov;

  size_D3[0] = Oa;
  size_D3[1] = Oa;
  size_D3[2] = Ov;
  size_D3[3] = Ov;

  // Create input tensors
  Tensor *A1 = new Tensor("aa", idmap_A1, size_A1, vgrid_A1, grid_dims_2, pgrid_2d);
  A1->initialize();

  Tensor *A2 = new Tensor("aa", idmap_A2, size_A2, vgrid_A2, grid_dims_2, pgrid_2d);
  A2->initialize();

  Tensor *C1 = new Tensor("cd", idmap_C1, size_C1, vgrid_C1, grid_dims_2, pgrid_2d);
  C1->initialize();

  Contraction* contr1 = new Contraction(A1, A2, C1, grid_dims_2, pgrid_2d);
  //    contr1->contract("a1,k1", "b1,k1", "a1,b1");
  contr1->~Contraction();

  Tensor *A3 = new Tensor("cdcd", idmap_A3, size_A3, vgrid_A3, grid_dims_4, pgrid_4d);
  A3->initialize();

  Tensor *D3 = new Tensor("cdcd", idmap_D3, size_D3, vgrid_D3, grid_dims_4, pgrid_4d);
  D3->initialize();

  C1->update_pgrid(grid_dims_4, pgrid_4d);

  Contraction* contr2 = new Contraction(C1, A3, D3, grid_dims_4, pgrid_4d);
  // contr2->contract("a1,k1", "b1,k1,b2,b3", "a1,b1,b2,b3");
  contr2->~Contraction();
  }*/
