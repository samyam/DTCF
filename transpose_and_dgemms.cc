#include "contraction.h"
#include "omp.h"
#define CCHECK 0
#define DEBUG_TR 0
namespace RRR{

    using namespace std;
// Perform transpose as required on input tensor blocks and call dgemm
    void Contraction::transpose_and_dgemm_blockwise(int num_blocks_A, int num_blocks_B, double* &blocks_A, double* &blocks_B, int* &block_addr_A, int* &block_addr_B, double* &C_buffer, int delete_buffers)
    {
    
	if(num_blocks_B == 0 || num_blocks_A == 0) return;
	// Contract each block in sub-tensor A with each block in sub-tensor B

    	if(CCHECK) {
	    CRCT_check_blocks(block_addr_A, num_blocks_A, block_addr_B, num_blocks_B);

	    /*cout << "A blocks: ";
	      for(int i=0; i<num_blocks_A; i++)
	      {
	      int* addr = block_addr_A + i*A->dims;
	      print_tile_addr(A->dims, addr);
	      }
	      cout << endl << "B blocks: ";
	      for(int i=0; i<num_blocks_B; i++)
	      {
	      int* addr = block_addr_B + i*B->dims;
	      print_tile_addr(B->dims, addr);
	      }
	      cout << endl;*/
	}

	

	// Transpose blocks of B here because the outer for-loop requires the tranposed blocks of B
	// multiple times
	double* tr_blocks_buf_B =new double[num_blocks_B * B->block_size]; 

	int *perm_address_A = new int[A->dims*num_blocks_A]; 
	int *perm_address_B = new int[B->dims*num_blocks_B]; 
	int tg = 0;

	if(DEBUG_TR && rank==rank)	cout<<"Rank "<<rank<<".  TG"<<tg++<<endl;


	//-------------------------------------B-transpose---------------------------------------------//

	//#pragma omp parallel for private(source_B, dest_B, source_addr_B)
	//transposes each block of B one at a time
	tr_time -= MPI_Wtime();


	for(int j=0; j<num_blocks_B; j++)
	{

	    double* source_B = blocks_B + j * B->block_size;
	    double* dest_B = tr_blocks_buf_B + j * B->block_size;
	    int* source_addr_B  = block_addr_B + j * B->dims;

	    int* sym_perm_B = new int[B->dims];
	    int* sym_composed_B = new int[B->dims];

	    //gets the permutation due to flipping of indices since       
	    //only unqie blocks are stored
	    get_symm_permutation(B, source_addr_B, sym_perm_B);

	    //compose the permutations required due to symmetry with permutation 
	    //to align blocks for dgemm
	    compose_p_maps(B, sym_perm_B, p_map_B, sym_composed_B);

	    //transposes a block of B
	    transpose(source_B, dest_B, sym_composed_B, B->dims, B->block_range);
	
	    //generates the permuted tile address
	    int* dest_address = perm_address_B + j* B->dims;
	    int* source_address = block_addr_B + j * B->dims;
	    for(int d = 0; d< B->dims; d++)
	    {
		dest_address[p_map_B[d]] = source_address[d]; 
	    }


	    delete[] sym_perm_B;
	    delete[] sym_composed_B;

	}
	if(DEBUG_TR && rank==rank)	cout<<"Rank "<<rank<<".  Num Blocks B"<<num_blocks_B<<endl;
	if(DEBUG_TR && rank==rank)	cout<<"Rank "<<rank<<".  TG"<<tg++<<endl;

	if(delete_buffers)
	    delete[] blocks_B;


	tr_time += MPI_Wtime();

	double* tr_blocks_buf_A =new double[num_blocks_A * A->block_size]; 
	tr_time -= MPI_Wtime();


	if(DEBUG_TR && rank==rank)	cout<<"Rank "<<rank<<".  TG"<<tg++<<endl;

	for(int j=0; j<num_blocks_A; j++)
	{

	    double* source_A = blocks_A + j * A->block_size;
	    double* dest_A = tr_blocks_buf_A + j * A->block_size;
	    int* source_addr_A  = block_addr_A + j * A->dims;

	    int* sym_perm_A = new int[A->dims];
	    int* sym_composed_A = new int[A->dims];

	    //gets the permutation due to flipping of indices since 
	    //only unqie blocks are stored
	    get_symm_permutation(A, source_addr_A, sym_perm_A);

	    //composes the permutations required due to symmetry with permutation 
	    //to align blocks for dgemm
	    compose_p_maps(A, sym_perm_A, p_map_A, sym_composed_A);

	    //transposes a block of B
	    transpose(source_A, dest_A, sym_composed_A, A->dims, A->block_range);

	    //generates the permuted tile address
	    int* dest_address = perm_address_A + j* A->dims;
	    int* source_address = block_addr_A + j * A->dims;
	    for(int d = 0; d< A->dims; d++)
	    {
		dest_address[p_map_A[d]] = source_address[d]; 
	    }

	    delete[] sym_perm_A;
	    delete[] sym_composed_A;

	}

	if(DEBUG_TR && rank==rank)	cout<<"Rank "<<rank<<".  TG"<<tg++<<endl;
    
	tr_time += MPI_Wtime();

	//if(rank==0) printGetTiles(blocks_A, block_addr_A, A->block_size, num_blocks_A, A->dims);
   
	C->untouch_all_address();

	//if(!rank) cout<<"product of blocks of A and B"<<num_blocks_A * num_blocks_B<<endl;
	//if(!rank) cout<<"Num threads = "<<omp_get_num_threads()<<endl<<fflush;
	if(delete_buffers)
	    delete[] blocks_A;
	
	comp_time -= MPI_Wtime();
	if(num_blocks_A>0 && num_blocks_B>0){

/*#pragma omp parallel for
	    for(int b = 0; b< num_blocks_A * num_blocks_B; b++){

		int i = b / num_blocks_B;
		int j = b % num_blocks_B;

		int* address_A = block_addr_A + i * A->dims;
		int* address_B = block_addr_B + j * B->dims;
		int* address_C;

	        get_C_address(address_A, A->dims, address_B, B->dims, address_C, C->dims);
		int location_of_C = C->getTile(address_C);

		//if this location exists and is not an empty block
		if(location_of_C != -1){
		    double* block_A = tr_blocks_buf_A + i* A->block_size;
		    double* block_B = tr_blocks_buf_B + j * B->block_size;		    
		    double* block_C = C_buffer + location_of_C * C->block_size;
		    
		    kevin_dgemm(n_a , n_b , n_k, block_A, block_B, C_buffer, 0, 0, 1.0);
		}
		}*/
	}
	
	comp_time += MPI_Wtime();
	
	delete[] tr_blocks_buf_B;
	delete[] tr_blocks_buf_A;
	
    }
}
