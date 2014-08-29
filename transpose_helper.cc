#define TRANSPOSE_HELPER_FILE
#include "transpose_helper.h"
#include <iostream>
#define DEBUG_T 0
#define RRANK 0

namespace RRR{
    using namespace std;
/*creates a 2D matrix from a list of multidimensional tensor tiles
this function works in the following way. It takes each block and 
places its data in the appropriate position in the 2D matrix. Each 
tensor block has multiple indices, which can be divided into row and
column indices. The input tiles are arranges so that the row indices 
appear first and the column indices appear later. The input variable
num_row_dim gives the number of indices that corresponds to rows. 
Using this, a row rank and a column rank is generated for each tile
which is then used to indetify the correct position of the data in
each tile in the 2D matrix. */
void create_big_matrix(Tensor* &T, double* &tensor_tiles, int* &permuted_address,
		       int* &p_map_T, int num_tiles,int row_per_block, int col_per_block, 
		       int num_row_dim, map<int, map<int, int> >* &big_matrix_map, 
		       double* &big_matrix)
{
    big_matrix_map = new map<int, map<int, int> >();

    //Pack the individual blocks of C to a big 2D matrix with rows from A and columns from B
    //This packing allows calling of dgemm on similarly created matrix A and matrix B
    //The goal is to run a single parallel huge dgemm instead of running multiple dgemms
    int dims_T = T->get_dims();
    
    

    //create a permuted tensor grid for C based on the permutation map.
    //this allows easily accessing ranks of different tiles of C based on tile
    //address. Ranks of tiles are simply a single dimensional integer address
    //used to identify blocks

    int* grid_T = new int[dims_T];
    for(int d = 0; d< dims_T; d++)
    {
	grid_T[p_map_T[d]] = T->get_vgrid()[d];
    }
    Block_Grid* T_grid = new Block_Grid(dims_T, grid_T);    


    //temp_T_addr holds the address of the tile of C in the permuted order
    //permuted order is the one where the external indices of A appear first
    //then the external indices of B appear later for example C[i,m,n,j] 

    //where i and j are from A and m and n are from B gets permuted to C[i,j,m,n]
    int* temp_T_addr = new int[dims_T];

    if(DEBUG_T) {
	cout<<"Rank : "<<T->get_rank()<<" ";
	print_tile_addrs(dims_T, permuted_address, num_tiles);
	cout<<"Rank : "<<T->get_rank()<<" Num Row Dim "<<num_row_dim<<endl;
    }

    //for each tile in C create a map to the placement in the large matrix
    for(int i=0; i < num_tiles; i++)
    {
	
	int* T_addr = permuted_address + i * dims_T;
	//only the external indices of A are kept since these get
	//translated into row indices
	memset(temp_T_addr, 0 , dims_T * sizeof(int));

	//if(T->get_rank() == 0) cout<<"Num row dims is "<<num_row_dim<<endl;

	//find the ordering for column
	for(int j = 0; j<num_row_dim; j++)
	{
	    temp_T_addr[j] = T_addr[j];
	}

	int T_row_rank = T_grid->get_block_rank(temp_T_addr);
	//if(T->get_rank() == 0) cout<<"T row rank "<<T_row_rank<<endl;

	memset(temp_T_addr, 0 , dims_T * sizeof(int));
	
	//find the ordering for column
	for(int j = num_row_dim; j<dims_T; j++)
	{
	    temp_T_addr[j] = T_addr[j];
	}
	int T_column_rank = T_grid->get_block_rank(temp_T_addr);
	
	//insert the row and column address in the big_matrix_map
	if( big_matrix_map->find(T_row_rank) == big_matrix_map->end())
	{
	    //if(T->get_rank() == 0) cout<<"first if "<<T_column_rank<<endl;
	    map<int, int> col_T;
	    col_T[T_column_rank] = i;
	    //col_T.insert(std::pair<int,int>(T_column_rank, i));
	    big_matrix_map->insert(std::pair<int, map<int, int> >(T_row_rank, col_T));
	}
	else
	{
	    //if(T->get_rank() == 0) cout<<"else "<<T_column_rank<<endl;
	    big_matrix_map->find(T_row_rank)
		->second.insert(std::pair<int,int>(T_column_rank, i));
	}
    }

    big_matrix = new double[num_tiles * T->get_block_size()];
    if(DEBUG_T && T->get_rank() == 0) cout<<"size created "<<num_tiles * T->get_block_size()<<endl;
    int row_count =0;
    int column_count =0;

    int total_rows = big_matrix_map->size() * row_per_block;
    int total_column = big_matrix_map->begin()->second.size() * col_per_block;

    int previous_col_count, current_col_count;
    previous_col_count = total_column / col_per_block;

    //Copy data from the tile representation to the big C matrix
    for (std::map<int,map <int,int> >::iterator row_it=big_matrix_map->begin(); row_it!=big_matrix_map->end(); ++row_it)
    {
	map <int, int> col = row_it->second;
       	current_col_count = col.size();
	
	//makes sure the number of column stays the same
	assert(current_col_count == previous_col_count);
	previous_col_count = current_col_count;

	column_count = 0;
	//Copy a single column
	for (std::map<int,int>::iterator col_it=col.begin(); col_it!=col.end(); ++col_it)
	{
	    int tile_number = col_it->second;
	    double* current_tile = tensor_tiles + tile_number * T->block_size;

	    //Prints out the block indices in the order used to create the big matrix
	    if(DEBUG_T && T->get_rank()==RRANK)
	    {
		int row_rank = row_it->first;
		int col_rank = col_it->first;

		int *row_address, *col_address, *full_addr;
		T_grid->get_block_addr(row_rank, row_address);
		T_grid->get_block_addr(col_rank, col_address);
		
		full_addr = new int[dims_T];
		for(int i = 0; i<dims_T; i++)
		{
		    full_addr[i] = row_address[i] + col_address[i];
		}
		print_tile_addr(dims_T, full_addr);
	    }
	    
	    for(int row_num = row_count; row_num < row_count + row_per_block; row_num++)
	    {
		double* source = current_tile + (row_num - row_count) * col_per_block;
		double* dest = big_matrix + row_num * total_column + column_count;
		memcpy(dest, source, col_per_block * sizeof(double));
		
	    }

	    column_count += col_per_block;
	}
	    if(DEBUG_T && T->get_rank()==RRANK)
	    {
		cout<<endl;
	    }
	row_count += row_per_block;
    }

}


/*creates a 2D matrix from a list of multidimensional tensor tiles
this function works in the following way. It takes each block and 
places its data in the appropriate position in the 2D matrix. Each 
tensor block has multiple indices, which can be divided into row and
column indices. The input tiles are arranges so that the row indices 
appear first and the column indices appear later. The input variable
num_row_dim gives the number of indices that corresponds to rows. 
Using this, a row rank and a column rank is generated for each tile
which is then used to indetify the correct position of the data in
each tile in the 2D matrix. */
void create_big_matrix_nl(Tensor* &T, double* &tensor_tiles, int* &permuted_address,
			  int* &p_map_T, int num_tiles,int row_per_block, int col_per_block, 
			  int num_row_dim, map<int, map<int, int> >* &big_matrix_map, 
			  double* &big_matrix, map<int,int>* &address_to_location)
{
    big_matrix_map = new map<int, map<int, int> >();

    //Pack the individual blocks of C to a big 2D matrix with rows from A and columns from B
    //This packing allows calling of dgemm on similarly created matrix A and matrix B
    //The goal is to run a single parallel huge dgemm instead of running multiple dgemms
    int dims_T = T->get_dims();

    //create a permuted tensor grid for C based on the permutation map.
    //this allows easily accessing ranks of different tiles of C based on tile
    //address. Ranks of tiles are simply a single dimensional integer address
    //used to identify blocks
    int* grid_T = new int[dims_T];
    for(int d = 0; d< dims_T; d++)
    {
	grid_T[p_map_T[d]] = T->get_vgrid()[d];
    }
    Block_Grid* T_grid = new Block_Grid(dims_T, grid_T);    


    //temp_T_addr holds the address of the tile of C in the permuted order
    //permuted order is the one where the external indices of A appear first
    //then the external indices of B appear later for example C[i,m,n,j] 
    //where i and j are from A and m and n are from B gets permuted to C[i,j,m,n]
    int* temp_T_addr = new int[dims_T];

    int location, tile_rank;

    //for each tile in C create a map to the placement in the large matrix
    for(int i=0; i < num_tiles; i++)
    {
	
	int* T_addr = permuted_address + i * dims_T;
	//only the external indices of A are kept since these get
	//translated into row indices
	memset(temp_T_addr, 0 , dims_T * sizeof(int));
	
	//find the ordering for column
	for(int j = 0; j<num_row_dim; j++)
	{
	    temp_T_addr[j] = T_addr[j];
	}

	int T_row_rank = T_grid->get_block_rank(temp_T_addr);

	memset(temp_T_addr, 0 , dims_T * sizeof(int));
	
	//find the ordering for column
	for(int j = num_row_dim; j<dims_T; j++)
	{
	    temp_T_addr[j] = T_addr[j];
	}
	int T_column_rank = T_grid->get_block_rank(temp_T_addr);
	
	//tile rank gives the rank of the current tile under scrutiny
	tile_rank = T_grid->get_block_rank(T_addr);

	/* the address_to_location map needs to have a location
	 * for this tile. Otherwise something must have gone
	 * wrong*/
	assert(address_to_location->find(tile_rank) 
	       != address_to_location->end());
	
	/*Store the location of this tile. The location number is
	 * used to locate the data tile to create the big matrix*/
	location = address_to_location->find(tile_rank)->second;
       
	//insert the row and column address in the big_matrix_map
	if( big_matrix_map->find(T_row_rank) == big_matrix_map->end())
	{
	    map<int, int> col_T;
	    col_T[T_column_rank] = location;
	    //col_T.insert(std::pair<int,int>(T_column_rank, location));
	    big_matrix_map->insert(std::pair<int, map<int, int> >(T_row_rank, col_T));
	}
	else
	{
	    big_matrix_map->find(T_row_rank)
		->second.insert(std::pair<int,int>(T_column_rank, location));
	}
    }

    big_matrix = new double[num_tiles * T->get_block_size()];
    int row_count =0;
    int column_count =0;

    int total_rows = big_matrix_map->size() * row_per_block;
    int total_column = big_matrix_map->begin()->second.size() * col_per_block;

    int previous_col_count, current_col_count;
    previous_col_count = total_column / col_per_block;

    //Copy data from the tile representation to the big C matrix
    for (std::map<int,map <int,int> >::iterator row_it=big_matrix_map->begin(); row_it!=big_matrix_map->end(); ++row_it)
    {
	map <int, int> col = row_it->second;
       	current_col_count = col.size();
	
	//makes sure the number of column stays the same
	assert(current_col_count == previous_col_count);
	previous_col_count = current_col_count;

	column_count = 0;
	//Copy a single column
	for (std::map<int,int>::iterator col_it=col.begin(); col_it!=col.end(); ++col_it)
	{
	    int tile_number = col_it->second;
	    double* current_tile = tensor_tiles + tile_number * T->block_size;

	    //Prints out the block indices in the order used to create the big matrix
	    if(DEBUG_T)
	    {
		int row_rank = row_it->first;
		int col_rank = col_it->first;

		int *row_address, *col_address;
		T_grid->get_block_addr(row_rank, row_address);
		T_grid->get_block_addr(col_rank, col_address);
		
		print_tile_addr(dims_T, row_address);
		print_tile_addr(dims_T, col_address);
		cout<<endl;


	    }

	    for(int row_num = row_count; row_num < row_count + row_per_block; row_num++)
	    {
		double* source = current_tile + (row_num - row_count) * col_per_block;
		double* dest = big_matrix + row_num * total_column + column_count;
		memcpy(dest, source, col_per_block * sizeof(double));
		
	    }

	    column_count += col_per_block;
	}

	row_count += row_per_block;
    }

}

/*Convert the 2D matrix, back to the multidimensional tile*/
void revert_big_matrix(Tensor* &T, map<int, map<int, int> >* &big_matrix_map, 
		       double* &big_matrix, int row_per_block, int col_per_block, 
		       int* &p_map_T, double* &tensor_tiles)
{

    //Pack the individual blocks of C to a big 2D matrix with rows from A and columns from B
    //This packing allows calling of dgemm on similarly created matrix A and matrix B
    //The goal is to run a single parallel huge dgemm instead of running multiple dgemms
    int dims_T = T->get_dims();

    //create a permuted tensor grid for C based on the permutation map.
    //this allows easily accessing ranks of different tiles of C based on tile
    //address. Ranks of tiles are simply a single dimensional integer address
    //used to identify blocks
    int* grid_T = new int[dims_T];
    for(int d = 0; d< dims_T; d++)
    {
	grid_T[p_map_T[d]] = T->get_vgrid()[d];
    }
    Block_Grid* T_grid = new Block_Grid(dims_T, grid_T);    
    int row_count =0;
    int column_count =0;

    int total_rows = big_matrix_map->size() * row_per_block;
    int total_column = big_matrix_map->begin()->second.size() * col_per_block;

    //they are used to verify that the number of column blocks stays the same
    int previous_col_count, current_col_count;
    previous_col_count = total_column / col_per_block;

    //Copy data from the tile representation to the big C matrix
    for (std::map<int,map <int,int> >::iterator row_it=big_matrix_map->begin(); row_it!=big_matrix_map->end(); ++row_it)
    {
	map <int, int> col = row_it->second;
       	current_col_count = col.size();
	
	//makes sure the number of column stays the same
	assert(current_col_count == previous_col_count);
	previous_col_count = current_col_count;

	column_count = 0;
	//Copy a single column
	for (std::map<int,int>::iterator col_it=col.begin(); col_it!=col.end(); ++col_it)
	{
	    int tile_number = col_it->second;
	    double* current_tile = tensor_tiles + tile_number * T->block_size;

	    //Prints out the block indices in the order used to create the big matrix
	    if(DEBUG_T && T->get_rank()==0)
	    {
		int row_rank = row_it->first;
		int col_rank = col_it->first;

		int *row_address, *col_address;
		T_grid->get_block_addr(row_rank, row_address);
		T_grid->get_block_addr(col_rank, col_address);
		
		print_tile_addr(dims_T, row_address);
		print_tile_addr(dims_T, col_address);
		cout<<endl;


	    }

	    for(int row_num = row_count; row_num < row_count + row_per_block; row_num++)
	    {
		double* source = big_matrix + row_num * total_column + column_count;
		double* dest = current_tile + (row_num - row_count) * col_per_block;
		memcpy(dest, source, col_per_block * sizeof(double));
		
	    }

	    column_count += col_per_block;
	}

	row_count += row_per_block;
    }

}
}
