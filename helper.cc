#define HELPER_FILE
//#define VECTOR_WIDTH 8
#include "helper.h"
#include <iostream>

using namespace std;

// Find an element in the given array and return its index
// return -1 if not found
int search_in_arr(int num, int* &arr, int arr_size)
{
	for(int i=0; i<arr_size; i++)
	{
		if(arr[i] == num)
			return i;
	}
	return -1;
}


// Returns rank of a processor given its multi-dimensional address
int get_proc_rank(int* &address, int dims, int* &grid)
{
	int x = 1;
	int rank = 0;
	for(int i=dims-1; i>=0; i--) 
	{
		rank += address[i] * x;
		x *= grid[i];
	}
	return rank;
}

// Finds out ranks of n processors given their multi-dimensional addresses
void get_proc_ranks(int n, int** &addresses, int dims, int* &grid, int* &ranks)
{

	ranks = new int[n];
	for(int r=0; r<n; r++) 
	{
		ranks[r] = get_proc_rank(addresses[r], dims, grid);
	}
}

// Computes a processor's multi-dimensional address given its rank
void get_proc_addr(int* &address, int dims, int* &grid, int rank)
{  
	int x = 1;
	for(int i=0; i<dims; i++)
		x *= grid[i]; 

	for(int i=0; i<dims; i++)
	{
		x /= grid[i];
		address[i] = rank/x;
		rank -= address[i] * x;
	}
}

// Computes n processors' multi-dimensional addresses given their ranks
void get_proc_addrs(int n, int** &addresses, int dims, int* &grid, int* &ranks)
{
	for(int r=0; r<n; r++) 
	{
		get_proc_addr(addresses[r], dims, grid, ranks[r]);
	}
}

// Prints address of a processor given its rank
void print_proc_addr(int dims, int* &grid, int rank)
{
	cout << "Node Address is : " << get_proc_addr_str(dims, grid, rank) << endl;
}

// Returns the string of the processor's multi-dimensional address, given its rank
string get_proc_addr_str(int dims, int* &grid, int rank)
{
	stringstream proc_addr;
	int x = 1;
	for(int i=0; i<dims; i++)
		x *= grid[i]; 

	proc_addr << "[ ";
	for(int i=0; i<dims; i++)
	{
		x /= grid[i];
		int address = rank/x;
		proc_addr<<address<<", ";

		rank -= address * x;
	}
	proc_addr<<"]";
	return proc_addr.str();
}

// Prints the address of a tile
void print_tile_addr(int dims, int* &addr)
{
	cout << "<";
	for(int i=0; i<dims; i++)
	{
		cout << addr[i];
		if(i < dims - 1)
		{ 
			cout << ", ";
		}
	}
	cout <<">  ";
}


// Prints tile addresses
void print_tile_addrs(int dims, int* &addr, int num_tiles)
{
    for(int j = 0;j<num_tiles;j++)
    { 
	cout << "<";
	for(int i=0; i<dims; i++)
	{
		cout << addr[j*dims + i];
		if(i < dims - 1)
		{ 
			cout << ", ";
		}
	}
	cout <<">  ";
    }
}

//checks if addresses in addr1 and addr2 are the same
void are_tile_addrs_equal(int dims, int* &addr1, int* &addr2, int num_tiles)
{
    int found;
    for(int j = 0;j<num_tiles;j++)
    { 
	//Check if the jth address is in addr2
	found = 0;
	for(int k = 0;k<num_tiles;k++)
	{ 
	    //Check if jth address of addr1 is same as kth address of addr2
	    int found_here = 1;
	    for(int i=0; i<dims; i++)
	    {
		if(addr1[j*dims + i] != addr2[k*dims + i])
		{
		    found_here = 0;
		}
	    }

	    //The addresses are same
	    if(found_here)
	    {
		found = 1;
		break;
	    }

	}
	
	//The jth address in addr1 was not found in addr2
	if(!found)
	{
	    cout<<"The addresses are different."<<endl;
	    return;
	}

    }

    //All addresses were found.
    cout<<"The addresses are the same."<<endl;
}

int is_address_equal(int dims, int* addr1, int* addr2)
{
    for(int d = 0; d< dims; d++)
    {
	if(addr1[d] != addr2[d]) return 0;
    }
    return 1;
}

void print_block_data(int block_size, double* &block_addr)
{
	cout << "<";
	for(int i=0; i<block_size; i++)
	{
	    cout << block_addr[i] <<", ";
	}
	cout <<">  "<<endl;
}

int is_equal_blocks(int block_size, double* &block_addr1, double* &block_addr2)
{

	for(int i=0; i<block_size; i++)
	{
		if(block_addr1[i] !=block_addr2[i])
		{
			return 0;
		}
	}
	return 1;
}

int are_equal_blocks(int block_size, double* &blocks_addr1, double* &blocks_addr2, int num_blocks)
{
    for(int i =0; i< num_blocks; i++)
    {
	double* b1 = blocks_addr1 + i * block_size;
	double* b2 = blocks_addr2 + i * block_size;
	
	if(!is_equal_blocks(block_size, b1, b2)){
	    return 0;
	}
    }
    
    return 1;
}


// Swaps two integers
void swap(int* a, int* b)
{
	int t = *a;
	*a = *b;
	*b = t;
}


void printGetTiles(double* data_block, int* tile_address, int blocksize, int num_blocks, int dims)
{
	for(int iterator = 0; iterator<num_blocks; iterator++)
	{
		cout<<"Virtual Address is :[";

		for(int i =0; i<dims; i++)
			cout<<tile_address[i+dims*iterator]<<", ";
		cout<<"]"<<endl;

		//for(int b =0; b<blocksize; b++)
		//	cout<<data_block[iterator*blocksize+b] << " ";
		//cout<<endl<<endl;
	}
}  


// Finds block number of a block in the block grid based on its virtual address
int get_block_number(int* &block_addr, int dims, int* &vgrid)
{
	return get_proc_rank(block_addr, dims, vgrid);
}


// Finds block address from the block number in block grid
void get_block_addr(int* &block_addr, int dims, int* &vgrid, int block_number)
{
	get_proc_addr(block_addr, dims, vgrid, block_number);
}

// Assert if the number of nodes allocated and the size of pgrid defined in code are equal
void assert_pgrid_size(int size, int dims, int* &pgrid)
{
	int pgrid_size = 1;
	for(int i = 0; i < dims; i++)
	{
		pgrid_size *= pgrid[i];
	}
	//cout<<"Size is "<<size<<"pgrid_size is "<<pgrid_size<<endl;
	assert(size == pgrid_size);
}



///does element wise transpose based on the permutation map
//change it to vectorwise transpose if the permutation map allows it
void transpose(double* &input_buffer, 
		double* &output_buffer, 
		int* permutation_map,
		int dimension,
		int* block_widths
		)
{
	int VECTOR_WIDTH;
	//needs to be OPTIMIZED

	//maps the processor grid to the indicies based on permutation map
	int* inverse_map = new int[dimension];


	int* initial_offsets = new int[dimension];
	int* permuted_offsets = new int[dimension];


	for(int d = 0; d< dimension; d++)
	{
		inverse_map[permutation_map[d]] = d;
	}

	int num_last_matching_p_map = 0;

	//counts the number of last indicies that match with the original layout
	//memcpy can be used to copy them directly
	for(int d = dimension -1; d >=0; d--)
	{
		if(permutation_map[d] == d)
		{
			num_last_matching_p_map++;
		}
		else
		{
			break;
		}
	}
	

	//offsets used to caculate original address of the data
	initial_offsets[dimension-1] = 1;
	initial_offsets[dimension - 2] = block_widths[dimension-1];

	//offsets used to calculate final address of the data
	permuted_offsets[dimension -1] = 1;
	permuted_offsets[dimension - 2] = block_widths[inverse_map[dimension-1]];

	for(int d = dimension -3 ; d >= 0 ; d--)
	{
		initial_offsets[d] = initial_offsets[d+1] * block_widths[d+1];
		permuted_offsets[d] = permuted_offsets[d+1] * block_widths[inverse_map[d+1]];
	}

	//start the transpose code
	//perform a single memcopy if no transpose is required
	if(num_last_matching_p_map == dimension)
	{
		int data_size = initial_offsets[0] * block_widths[0] * sizeof(double);
		memcpy(output_buffer, input_buffer,  data_size);

		//create a massive loop and do memcopies
	}else if(num_last_matching_p_map > 0){


		//total number of indices that needs to be nested before 
		//memcopy can be invoked 
		int num_nested_loops = dimension - num_last_matching_p_map;

		//total number of iterations points in the num_nested_loops
		int num_total_iterations = 1;

		//calculate num_total_iterations
		for(int index = 0; index<num_nested_loops; index++)
		{
			num_total_iterations *= block_widths[index];
		}	

		//size of data that can be memcopied
		int data_size = initial_offsets[num_nested_loops-1] * sizeof(double);

		//to tile the combined iteration space to reduce arithmetic operations
		int last_tile = block_widths[num_nested_loops-1];

		//iterate through each iteration point in num_total_iterations
#ifdef __bgq__
#pragma omp parallel for schedule(static,1) num_threads(16)
#else
#pragma omp parallel for schedule(static,1)
#endif
		for(int combined_index = 0; combined_index<num_total_iterations; combined_index+=last_tile)
		{

			int t_combined_index = combined_index;

			int* data_index = new int[dimension];

			t_combined_index = t_combined_index / last_tile;

#pragma unroll
			for(int i = num_nested_loops-2; i>=0;i--)
			{
				data_index[i] = t_combined_index % block_widths[i];
				t_combined_index = t_combined_index / block_widths[i];
			}

			int source_address = 0;
			int destination_address = 0;

			//compute source and destination address	 
#pragma unroll   
			for(int i = 0; i<num_nested_loops-1; i++)
			{
				source_address += data_index[i]*initial_offsets[i];

				destination_address += data_index[i]*permuted_offsets[permutation_map[i]];

			}

			source_address -= initial_offsets[num_nested_loops-1];
			destination_address -= permuted_offsets[permutation_map[num_nested_loops-1]];

			//this tiling allows most part of the source and destination to be computed before hand
#pragma unroll
			for(int t1_ci = 0; t1_ci<last_tile; t1_ci++)
			{
				source_address += initial_offsets[num_nested_loops-1];
				destination_address += permuted_offsets[permutation_map[num_nested_loops-1]];

				memcpy(&output_buffer[destination_address],&input_buffer[source_address], data_size);
			}

			delete[] data_index;
		}

	}
	//transpose needs to be done element by element
	else
	{
		int* non_last_indices = new int[dimension-2];
		int count =0;

		//find the indices that are not the innermost in both original and transposed permutation maps
		for(int i = 0; i< dimension-1; i++)
		{
			if(i != inverse_map[dimension-1])
			{
				non_last_indices[count++] = i;
				//cout<<i<<" ";
			}
		}

		//memcopy can be invoked 
		int num_nested_loops = dimension - 2;

		//total number of iterations points in the num_nested_loops
		int num_total_iterations = 1;

		//calculate num_total_iterations
		for(int i = 0; i<num_nested_loops; i++)
		{
			num_total_iterations *= block_widths[non_last_indices[i]];
		}	
		
		/*Find the largest width for vector WIDTH. THis
		 * allows movement of data in larger chunks*/
		int in = inverse_map[dimension-1];
		for(int i = 32; i >=1; i--)
		  {
		      if(block_widths[dimension-1] % i == 0 && block_widths[in] % i == 0)
		      {
			  VECTOR_WIDTH = i;
			  break;
		      }
		  }
                //cout<<"Vector Width is :"<<VECTOR_WIDTH<<">"<<endl;
		//iterate through each iteration point in num_total_iterations
#ifdef __bgq__
#pragma omp parallel for schedule(static,16) num_threads(16)
#else
#pragma omp parallel for schedule(static,1)
#endif

		for(int combined_index = 0; combined_index<num_total_iterations; combined_index++)
		{

			//	    if(combined_index ==0) cout<<"total number of threads is "<<omp_get_num_threads()<<endl;

			int t_combined_index = combined_index;

			int* data_index = new int[dimension];

			//compute data index for combined index

			int source_address = 0;
			int destination_address = 0;
#pragma unroll
			for(int i = num_nested_loops-1; i>=0;i--)
			{
				//cout<<" "<<non_last_indices[i]<<" ";
				int ind = non_last_indices[i];
				data_index[ind] = t_combined_index % block_widths[ind];
				t_combined_index = t_combined_index / block_widths[ind];

				source_address += data_index[ind]*initial_offsets[ind];

				destination_address += data_index[ind]*permuted_offsets[permutation_map[ind]];

			}

			//print_tile_addr(4,data_index);
			//cout<<endl;
			//nli = new last index. A nested loop between permuted last index and old last index. 
			//Perhaps add another layer of tiling here
			int ind = inverse_map[dimension-1];
			for(int nli = 0; nli < block_widths[ind]; nli+=VECTOR_WIDTH)
			{
				//oli = old last index
				for(int oli = 0; oli < block_widths[dimension-1]; oli+=VECTOR_WIDTH)
				{

					for(int t1_nli = nli; t1_nli < nli + VECTOR_WIDTH; t1_nli++)
					{		


						int s_offset = source_address + t1_nli * initial_offsets[ind];
						int d_offset = destination_address+ t1_nli * permuted_offsets[permutation_map[ind]];

#pragma unroll
						for(int t1_oli = oli; t1_oli < oli + VECTOR_WIDTH; t1_oli++)
						{


							int ss_offset = t1_oli*initial_offsets[dimension-1]; 
							int dd_offset = t1_oli*permuted_offsets[permutation_map[dimension-1]];
							output_buffer[d_offset + dd_offset] = input_buffer[s_offset + ss_offset];

						}	
					}

				}
			}
		}


	}

}

//does element wise transpose of a block
//need to be optimized to do memcopy transpose if possible
void recursive_transpose(double* &input_buffer, 
		double* &output_buffer, 
		int* &initial_offsets, 
		int* &permuted_offsets, 
		int* &data_index,
		int* &permutation_map,
		int recursion_level,
		int dims,
		int* block_widths,
		int num_last_matching_p_map)
{
	//base case computes the source value of individual data elements based on their 
	//addressalso computes the destination address and writes the value from the 
	//source to the destination

	//cout<<"recursion level is : "<<recursion_level<<" and block width is "<<block_widths[recursion_level]<<endl;

	int loop_bound = block_widths[recursion_level];
	int current_recursion_level = recursion_level;


	if(recursion_level == dims -1 - num_last_matching_p_map)
	{
		int data_size = initial_offsets[recursion_level] * sizeof(double);	
		/*	    cout<<"permutation map : [";
				for(int i = 0; i<dims; i++)
				{
				cout<<permutation_map[i]<<" ";
				}
				cout<<"]"<<endl;
				*/
		for(int d = 0; d<loop_bound;d++)
		{
			int address =0;
			int source_address = 0;
			int destination_address = 0;

			data_index[current_recursion_level] = d;


			for(int i = 0; i<recursion_level; i++)
			{
				source_address += data_index[i]*initial_offsets[i];

				destination_address += data_index[i]*permuted_offsets[permutation_map[i]];

			}
			/*
			   cout<<"data_address : [";
			   for(int i = 0; i<dims; i++)
			   {
			   cout<<data_index[i]<<" ";
			   }
			   cout<<"] initial : [";
			   for(int i = 0; i<dims; i++)
			   {
			   cout<<initial_offsets[i]<<" ";
			   }
			   cout<<"] destination :"<<destination_address<<endl;
			   */
			memcpy(&output_buffer[destination_address],&input_buffer[source_address], data_size);

		}
		/*
		   cout<<"data_index [";
		   for(int i = 0; i<dims; i++)
		   {
		   cout<<data_index[i]<<" ";
		   }
		   cout<<"]"<<endl;
		   */
	}
	else{

		//recursion hasnt reached the last dimension so set the data index
		//call next level of recursion

		//cout<<"recursion level is : "<<recursion_level<<" and block width is "<<block_widths[recursion_level]<<endl;
		recursion_level++;
		for(int d = 0; d<loop_bound;d++)
		{
			data_index[current_recursion_level] = d;

			//cout<<"calling recursively"<<endl;
			recursive_transpose(input_buffer, output_buffer, initial_offsets, permuted_offsets, 
					data_index, permutation_map, recursion_level, dims, block_widths, num_last_matching_p_map);
		}
	}

}

// compute greatest common divisor
int gcd(int a, int b)
{
    if(a == b) return a;

    if(a == 1 || b == 1) return 1;
     
    if(a > b) return gcd(a-b,b);
    
    return gcd(a,b-a);

}

// compute lowest common multiple
int lcm(int a, int b)
{
    return a*b/gcd(a,b);
}


/*points_per_dim is an array of arrays. The inner dimension
 * represents the dimension of the processor grid while the outer
 * dimension represents the processor addresses along that
 * dimension. The size_per_dim stores the number of processor
 * along each dimension of points_per_dim. The
 * create_cross_products fills output which is an array of
 * processor addresses. The addresses created by this function is
 * a tuple of dims, given by the cross products of addresses along
 * each dimension given by points_per_dims.*/ 
void create_cross_product(int** &points_per_dim, int* &size_per_dim, 
			  int &dims, int cur_dim, 
			  int &offset, int* &current_addr,
			  int** &output){

    //for each receiver address along cur_dim
    for(int i = 0 ; i< size_per_dim[cur_dim]; i++)
    {

	current_addr[cur_dim] = points_per_dim[cur_dim][i];

	if(cur_dim == dims-1){

	    output[offset] = new int[dims];

	    memcpy(output[offset], current_addr, dims*sizeof(int));

	    offset ++;

	}else{
	    
	    create_cross_product(points_per_dim, size_per_dim, dims, cur_dim+1, offset, current_addr, output);
	}
    }

}
