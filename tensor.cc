#include "tensor.h"
#define RRANK 0
#define DEBUG_TENSOR 0
namespace RRR{
    using namespace std;

    
// Constructor
    Tensor::Tensor(
	string tnsr_str,
	int* &idx_map,
	int* &tnsr_size,
	int* &virt_grid,
	Grid* &gg
	)
    {
	g = new Grid(gg->grid_dims,gg->pgrid);
	Grid* grid = g;

	grid_dims = grid->grid_dims;

	tensor_str = tnsr_str;
	dims = tensor_str.size();
	tensor_size = tnsr_size;

	// Set block range
	block_range = new int[dims];
	ctce_index_name = new int[dims];
	for(int i=0; i<dims; i++)
	{
	    block_range[i] = tensor_size[i] / virt_grid[i];
	}

	// pgrid
	pgrid = new int[grid_dims+1];
	memcpy(pgrid, g->pgrid, (grid_dims+1)*sizeof(int));

	//serial dimension for implementing serialization
	pgrid[grid_dims] = 1;
	vgrid = virt_grid;

	// Find out this processor's address (multi-dimensional) based on its rank
	//MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	rank = grid->rank;
	proc_addr = new int[grid_dims+1];
	memcpy(proc_addr, grid->proc_addr, grid_dims*sizeof(int));

	//the serial dimension addr
	proc_addr[grid_dims] = 0;

	// Compute block size
	block_size = 1;
	for(int i=0; i<dims; i++) block_size *= block_range[i];

	// index phase is same as virtual grid size
	index_phase = new int[dims];
	memcpy(index_phase, grid->pgrid, dims*sizeof(int));

	// Calculate tensor_range
	tensor_range = new int[dims];
	for(int i=0; i<dims; i++)
	    tensor_range[i] = ceil((float)tensor_size[i] / block_range[i]);

	sym_dims[0] = 0;
	sym_dims[1] = 0;

	// Initialize index maps
	index_dimension_map = new int[dims];
	memcpy(index_dimension_map, idx_map, dims*sizeof(int));


	//Initialize the reverse index map
	reverse_index_map = new int[grid_dims+1];
	for(int i = 0; i< grid_dims; i++)
	    reverse_index_map[i] = dims;

	for(int i =0; i<dims; i++)
	    reverse_index_map[index_dimension_map[i]] = i;
	


	assert_idmap_validity();
	SG_index_map = new int[dims];
	SG_index_map_permanent = new int[dims];

	// Initialize contraction map
	cntr_map = new int[dims];
	for(int i=0; i<dims; i++)
	    cntr_map[i] = 0;

	numSym = 0;
	numNonSym = 0;
	num_actual_tiles = 0;

	// Parse tensor string
	int count = 0;
	for(string::iterator it = tensor_str.begin(); it!=tensor_str.end(); ++it)
	{
	    // Non-contracting index involved in first symmetry
	    if(*it == 'a')
	    {
		SG_index_map[count] = SYM_GROUP_0;
		//cntr_map[count] = 0;
		if(numSym < 1) numSym = 1;
		sym_dims[0]++;
	    }

	    // Non-contracting index involved in second symmetry
	    if(*it == 'b')
	    {
		SG_index_map[count] = SYM_GROUP_1;
		//cntr_map[count] = 0;
		if(numSym < 2) numSym = 2;
		sym_dims[1]++;
	    }

	    // Contracting index involved in first symmetry
	    if(*it == 'A')
	    {
		SG_index_map[count] = SYM_GROUP_0;
		//cntr_map[count] = 1;
		if(numSym < 1) numSym = 1;
		sym_dims[0]++;
	    }

	    // Contracting index involved in second symmetry
	    if(*it == 'B')
	    {
		SG_index_map[count] = SYM_GROUP_1;
		//cntr_map[count] = 1;
		if(numSym < 2) numSym = 2;
		sym_dims[1]++;
	    }

	    // Non-contracting index involved in no symmetry
	    if(*it == 'c')
	    {
		SG_index_map[count] = NON_SYM;
		//cntr_map[count] = 0;
		numNonSym++;
	    }

	    // Contracting index involved in no symmetry
	    if(*it == 'C')
	    {
		SG_index_map[count] = NON_SYM;
		//cntr_map[count] = 1;
		numNonSym++;
	    }

	    // Non-contracting index involved in no symmetry
	    if(*it == 'd')
	    {
		SG_index_map[count] = NON_SYM;
		//cntr_map[count] = 0;
		numNonSym++;
	    }

	    // Contracting index involved in no symmetry
	    if(*it == 'D')
	    {
		SG_index_map[count] = NON_SYM;
		//cntr_map[count] = 2;
		numNonSym++;
	    }

	    count++;
	}

	memcpy(SG_index_map_permanent, SG_index_map, dims * sizeof(int));
    }



// Destructor
    Tensor::~Tensor()
    {
	delete[] block_range;
	delete[] pgrid;
	delete[] proc_addr; 
	delete[] index_phase;
	delete[] tensor_range;

	free_index_table();

	delete[] index_dimension_map; 
	delete[] SG_index_map;
	delete[] cntr_map;

	delete[] tensor_tiles; 
	delete[] tile_address;
	delete[] is_touched;
    }


// Assert if the idmap is valid while creating the tensor
    void Tensor::assert_idmap_validity()
    {
	for(int i=0; i<dims; i++)
	{
	    assert(index_dimension_map[i] <= grid_dims);
	}
    }

// Update physical grid
    void Tensor::update_pgrid(int n_dims, int* phy_grid)
    {
	// If the idmap is serial, update to the new serial value
	// Tensor will still stay serialized
	for(int i=0; i<dims; i++)
	{
	    if(index_dimension_map[i] == grid_dims)
		index_dimension_map[i] = n_dims;
	}
	grid_dims = n_dims;
	delete[] pgrid;
	pgrid = new int[n_dims + 1];
	memcpy(pgrid, phy_grid, n_dims * sizeof(int));
	pgrid[n_dims] = 1;
    }


// Compute the number of maximum possible tiles in the tensor at this processor
    int Tensor::compute_num_max_tiles(int* &idmap, int* &phy_grid)
    { 
	if(numSym > 0)
	{
	    for(int i=0; i< dims; i++)
	    {
		if(tensor_str.at(i) == 'a' || tensor_str.at(i) == 'A')
		    sym_ratio[0] = ceil((float)vgrid[i] / pgrid[idmap[i]]);
		if(tensor_str.at(i) == 'b' || tensor_str.at(i) == 'B')
		    sym_ratio[1] = ceil((float)vgrid[i] / pgrid[idmap[i]]);
	    }
	}

	// Calculate number of tiles
	int maxtiles = 1;

	for(int i=0; i<dims; i++)
	{
	    if(SG_index_map[i] == NON_SYM)
	    {
		maxtiles *= (int)ceil((float)vgrid[i] / phy_grid[idmap[i]]);
	    }
	}

	for(int i=0; i<numSym; i++)
	{
	    int ratio = sym_ratio[i];
	    int sym_dim = sym_dims[i];
	    int numerator = 1;
	    int denominator = 1;

	    for(int j=0; j<sym_dim; j++)
	    {
		numerator *= (ratio + j);
		denominator *= (j+1);
	    }
	    int symtiles = (int)(numerator/denominator);

	    maxtiles = maxtiles * symtiles;
	}
	return maxtiles;
    }

// Compute the number of maximum possible tiles in the tensor at this processor
    int Tensor::compute_num_max_tiles_rect(int* &idmap, int* &phy_grid)
    { 

	int sym_tiles_0 = 1;
	int denominator_0 = 1;
	int sym_tiles_1 = 1;
	int denominator_1 = 1;
	if(numSym > 0)
	{
	    for(int i=0; i< dims; i++)
	    {
		if(tensor_str.at(i) == 'a' || tensor_str.at(i) == 'A'){
		    int p =  ceil((float)vgrid[i] / pgrid[idmap[i]]);
		    sym_tiles_0 = sym_tiles_0 * (p + denominator_0) /denominator_0;
		    denominator_0++;
		}
		    

		if(tensor_str.at(i) == 'b' || tensor_str.at(i) == 'B')
		{
		    int p =  ceil((float)vgrid[i] / pgrid[idmap[i]]);
		    sym_tiles_1 = sym_tiles_1 * ( p + denominator_1) /denominator_1;
		    denominator_1++;
		}
	    }
	}

	// Calculate number of tiles
	int maxtiles = 1;

	for(int i=0; i<dims; i++)
	{
	    if(SG_index_map[i] == NON_SYM)
	    {
		maxtiles *= (int)ceil((float)vgrid[i] / phy_grid[idmap[i]]);
	    }
	}
	
	maxtiles = maxtiles * sym_tiles_0 * sym_tiles_1;
	return maxtiles;
    }
    
    
/*void Tensor::set_get_value(double (*value_function)(int* &indices))
  {
  get_value = Tensor::value_function;
  }*/

    
    //Initialize with spatial and spin symmetry
    void initialize_with_symmetry(vector<int> spatial_s[2], vector<int> spin_s[2]){

	    spatial_sym[0] = spatial_s[0];
	    spatial_sym[1] = spatial_s[1];

	    spin_sym[0] = spin_s[0];
	    spin_sym[1] = spin_s[1];

	    enable_spatial_sym = true;
	    enable_spin_sym = true;	    
	    enable_spin_restricted = true;
	    initialize();
    }
    

    void Tensor::initialize()
    {
	num_max_tiles = compute_num_max_tiles_rect(index_dimension_map, pgrid);

	// Initialize tile data and addresses
	tensor_tiles = new double[block_size * num_max_tiles];
	tile_address = new int [dims*num_max_tiles];

	int *local_indices = new int[dims]();
	int offset = 0;
	if(rank == RRANK && DEBUG_TENSOR) cout<<"This tensor  occupies approximately "
					      <<(block_size*num_max_tiles*8/(1024*1024))<<"MB of memory."<<endl;
	generate_data(0, local_indices, offset);
	delete[] local_indices;
	
	init_index_table();
	fill_index_table();
    }


//initializes the index table which is an array of array of lists
//storing offsets required to access local blocks for a given index
    void Tensor::init_index_table()
    {
	int local[dims];
	list<int>** local_table = new list<int>*[dims];

	//obtain maximum numer of entries per dimension and create array
	// of list with that length
	for(int i = 0; i<dims ; i++)
	{
	    local[i] = ceil((float)vgrid[i]/pgrid[index_dimension_map[i]]) ;
	    list<int>* temp = new list<int>[local[i]];
	    local_table[i] = temp;
	}

	// initialize the index table
	local_range = local;
	index_table = local_table;
    }


// fills up the index table
    void Tensor::fill_index_table()
    {
	int virt_addr;
	int local_addr;

	//block scaling is used to produce the block number from block address


	int block_number = 0;

	is_touched = new int[num_actual_tiles];
	for(int i = 0; i<num_actual_tiles; i++)
	{
	    int* addr = tile_address + i*dims;
	    block_number = get_block_number(addr, dims, vgrid);
	    for(int d =0; d<dims; d++)
	    {
		// Set virtual address at this dimension to 0, if the index is already contracted.
		// This will limit the range for sub-tensors
		// A contracted index is unnecessary for sub-tensor computations
		virt_addr = tile_address[i*dims +d];

		if(SG_index_map[d] == CONTRACTED)
		{
		    virt_addr = 0;
		}

		local_addr = virt_addr / pgrid[index_dimension_map[d]];

		index_table[d][local_addr].push_back(i);
	    }

	    block_addr_to_blk_num.insert ( std::pair<int,int>(block_number,i) );
	    block_number = 0;
	}
    }


    // fills up the index table
    void Tensor::fill_index_table_tmp()
    {
	int virt_addr;
	int local_addr;

	//block scaling is used to produce the block number from block address


	int block_number = 0;

	is_touched = new int[num_actual_tiles];
	for(int i = 0; i<num_actual_tiles; i++)
	{
	    int* addr = tile_address + i*dims;
	    block_number = get_block_number(addr, dims, vgrid);
	    for(int d =0; d<dims; d++)
	    {
		// Set virtual address at this dimension to 0, if the index is already contracted.
		// This will limit the range for sub-tensors
		// A contracted index is unnecessary for sub-tensor computations
		//virt_addr = tile_address[i*dims +d];

		if(SG_index_map[d] == CONTRACTED)
		{
		    virt_addr = 0;
		}

		local_addr = virt_addr / pgrid[index_dimension_map[d]];

		index_table[d][local_addr].push_back(i);
	    }

	    block_addr_to_blk_num.insert ( std::pair<int,int>(block_number,i) );
	    block_number = 0;
	}
    }



// Free the memory allocated for index table
    void Tensor::free_index_table()
    {
	for(int i=0; i<dims ; i++)
	{
	    int local = ceil((float)vgrid[i]/pgrid[index_dimension_map[i]]);
	    for(int j=0; j<local ; j++)
	    {
		index_table[i][j].clear();
	    }
	    delete[] index_table[i];
	}
	delete index_table;
    }

    // Returns the number of tiles with address value indx_id in dimension dim
    // Stores addresses in virt_addr tile location in tile_location
    int Tensor::getTileAddresses(int dim, int indx_id, int* &tile_location, int* &virt_addr)
    {

	if(indx_id % pgrid[index_dimension_map[dim]] != proc_addr[index_dimension_map[dim]])
	    return 0;

	int local_addr = indx_id / pgrid[index_dimension_map[dim]];
	int num_tiles = index_table[dim][local_addr].size();

	
	int* v_addr = new int[dims * num_tiles];
	int* t_location = new int[num_tiles];

	list<int>::const_iterator iterator;
	int counter = 0;

	for (iterator = index_table[dim][local_addr].begin();
	     iterator != index_table[dim][local_addr].end(); ++iterator) {

	    //fill out virtual addresses for the set of tiles
	    for(int i = 0; i< dims; i++)
	    {
		v_addr[i + dims * counter] = tile_address[i+(*iterator)*dims];
	    }

	    t_location[counter] = *iterator;

	    counter++;

	}

	virt_addr = v_addr;
	tile_location = t_location;
	return num_tiles;

	
    }
    

// Returns the number of tiles with address value indx_id in dimension dim
// Stores the tiles in tile_block and addresses in virt_addr
    int Tensor::getTiles(int dim, int indx_id, double* &tile_block, int* &virt_addr)
    {

	if(indx_id % pgrid[index_dimension_map[dim]] != proc_addr[index_dimension_map[dim]])
	    return 0;
	int local_addr = indx_id / pgrid[index_dimension_map[dim]];
	int num_tiles = index_table[dim][local_addr].size();

	double* tiles = new double[num_tiles * block_size];
	double* dest = tiles;

	int* v_addr = new int[dims * num_tiles];


	list<int>::const_iterator iterator;
	int counter = 0;

	for (iterator = index_table[dim][local_addr].begin();
	     iterator != index_table[dim][local_addr].end(); ++iterator) {

	    memcpy (dest, tensor_tiles + block_size*(*iterator), block_size*sizeof(double));
	    //for(int l = 0; l<block_size; l++)
	    //cout<<dest[l];
	    dest += block_size;

	    //fill out virtual addresses for the set of tiles
	    for(int i = 0; i< dims; i++)
	    {
		v_addr[i + dims * counter] = tile_address[i+(*iterator)*dims];
	    }

	    counter++;

	}
	tile_block = tiles;
	virt_addr = v_addr;
	return num_tiles;

	//return 0;

    }

/*return the position of the tile if it exists else return -1*/
    int Tensor::getTile(int* tile_addr)
    {

	int* addr = tile_address;
	for(int i = 0; i< num_actual_tiles; i++)
	{

	    if(is_address_equal(dims, tile_addr, addr))
	    {
		return i;
	    }

	    addr+=dims;
	}

	return -1;

    }



// Print all tile addresses in this local tensor 
    void Tensor::print_all_tile_addr()
    {
	for(int i=0; i<num_actual_tiles; i++)
	{
	    int* addr = tile_address + i*dims;
	    print_tile_addr(dims, addr);
	    cout << endl;
	}
    }


//
    void Tensor::printTiles(int dim, int indx_id)
    {

	if(indx_id % pgrid[index_dimension_map[dim]] != proc_addr[index_dimension_map[dim]])

	    return;

	int local_addr = indx_id/index_phase[index_dimension_map[dim]];

	cout <<endl<< " processor :[ " << proc_addr[0] << ", " << proc_addr[1] << ", " <<proc_addr[2] << ", " <<proc_addr[3] << "] " << endl;
	list<int>::const_iterator iterator;
	int counter = 0;
	//cout<<"\n"<<dim<<", "<<local_addr<<flush;
	for (iterator = index_table[dim][local_addr].begin();
	     iterator != index_table[dim][local_addr].end(); ++iterator)
	{
	    // cout<<"\n iterator "<<*iterator;
	    cout<<"\n [ ";
	    //fill out virtual addresses for the set of tiles
	    for(int i = 0; i< dims; i++)
	    {
		cout << tile_address[i+dims*(*iterator)]<<", ";
	    }
	    cout<<"]"<<endl;
	    for(int i = 0; i <block_size; i++)
	    {
		cout<<tensor_tiles[*iterator*block_size + i]<<",";
	    }
	    cout<<endl;
	    counter++;
	}
    }


//stores virtual address of the data block given a local data block
    void Tensor::getVirtualAddress(int* &cur_indices, int* &virtual_address, int* &processor_addr, int* &idmap, int* &phy_grid)
    {
	for(int i=0; i<dims;i++)
	{
	    virtual_address[i] = cur_indices[i]*phy_grid[idmap[i]] + processor_addr[idmap[i]];

	}
    }


    //checks non zero blocks for spatial symmetry. Do not know
    //exactly how this works but just immitating what ctce code
    //does
    bool Tensor::is_spatial_non_zero(int* tile_address, int sval){
	int lval = 0;
	for(int i = 0; i<dims; i++){
	    lval ^= spatial_sym[O_or_V[i]][tile_address[i]];
	}

	return ((!enable_spatil_sym) || lval==sval);

    }

    //checks non zero blocks for spin symmetry. Do not know
    //exactly how this works but just immitating what ctce code
    //does
    bool Tensor::is_spin_non_zero(int* tile_address)
    {
	int lval = 0;
	int rval = 0;
	assert(dims  % 2 == 0);
	for(int i =0; i<dims/2; i++) lval += spin_sym[O_or_V[i]][tile_address[i]];
	for(int i =dims/2; i<dims; i++) rval += spin_sym[O_or_V[i]][tile_address[i]];
	return ((!enable_spin_sym) || (lval==rval));
    }


    //checks non zero blocks for spin symmetry. Do not know
    //exactly how this works but just immitating what ctce code
    //does
    bool Tensor::is_spin_restricted_non_zero(int* tile_address, int sval)
    {
	
	int lval = 0;
	int rval = 0;
	assert(dims  % 2 == 0);
	for(int i =0; i<dims; i++) lval += spin_sym[O_or_V[i]][tile_address[i]];

	return ((!enable_spin_restricted) || (lval!=sval));
    }

    bool Tensor::is_sym_non_zero(int* tile_address)
    {
	if(!is_spatial_non_zero(tile_address)) return false;
	if(!is_spin_non_zero(tile_address)) return false;
	if(!is_spin_restricted_non_zero(tile_address)) return false;
	return true;

    }

//returns true if the data block is unique given the symmetry
    bool Tensor::isValid(int* &cur_indices, int* &processor_addr, int* &idmap, int* &phy_grid)
    {
	int* virtual_address = new int[dims]();
	getVirtualAddress(cur_indices, virtual_address, processor_addr, idmap, phy_grid);


	for(int i = 0; i<dims; i++)
	{
	    for(int j = i+1; j<dims;j++)
	    {
		if(virtual_address[i]<virtual_address[j] && SG_index_map[i] == SG_index_map[j] && SG_index_map[i] != NON_SYM)
		{
		    delete[] virtual_address;
		    return false;
		}
	    }
	}

	for(int i = 0; i<dims; i++)
	{
	    if(virtual_address[i] >= vgrid[i])
	    {
		delete[] virtual_address;
		return false;
	    }
	}

	bool ret_value = is_non_zero_sym(virtual_address);


	//if(rank==1) print_tile_addr(dims, virtual_address);

	delete[] virtual_address;
	return ret_value;
    }



// Generate sample values in the tensor
    void Tensor::generate_data(int cur_dim, int* &cur_indices, int &offset)
    {
	// Get the bound for current dimension based on what symmetry it is involved in
	int bound = get_bound(cur_dim, cur_indices, index_dimension_map, pgrid);


	for(int i=0; i<bound; i++)
	{
	    cur_indices[cur_dim] = i;

	    if(cur_dim == dims-1)
	    { // if this is the fastest varying dimension, fill in values
		if(isValid(cur_indices, proc_addr, index_dimension_map, pgrid))

		{
		    num_actual_tiles++;
		    // Set data in this block
		    double value = 0;
		    for(int j=0; j<block_size; j++)
		    {

			value = get_value(cur_indices);
			tensor_tiles[offset] = value;
			offset++;
		    }

		    // Set address of this block

		    for(int k=0; k<dims; k++)
		    {

			tile_address[k+dims*(offset/block_size-1)] = cur_indices[k]*pgrid[index_dimension_map[k]] + proc_addr[index_dimension_map[k]];
		    }
		}
	    }
	    else
	    { // otherwise recursively call for the next faster varying dimension
		generate_data(cur_dim+1, cur_indices, offset);
	    }
	}
    }


// Get number of tiles for a tensor at a particular processor and generate tile addresses for that
    int Tensor::get_num_tiles(int cur_dim, int* &cur_indices, int &offset, 
			      int* &idmap, int* &proc_address, int* &addresses, int* &phy_grid, int &num_tiles) 
    {
	//static int num_tiles = 0;

	// Get the bound for current dimension based on what symmetry it is involved in
	int bound = get_bound(cur_dim, cur_indices, idmap, phy_grid);
	for(int i=0; i<bound; i++) 
	{
	    cur_indices[cur_dim] = i;

	    if(cur_dim == dims-1) // if this is the fastest varying dimension
	    {   
		if(isValid(cur_indices, proc_address, idmap, phy_grid))
		{   
		    num_tiles++;
		    offset += block_size;
		    //cout << "offset  " << num_tiles << " = " << offset/block_size;        

		    // Set address of this block
		    for(int k=0; k<dims; k++) 
		    {   
			//if(rank == 1) cout << "k+dims*(offset/block_size-1) = " << k+dims*(offset/block_size-1) << endl; 
			addresses[k+dims*(offset/block_size-1)] = cur_indices[k]*phy_grid[idmap[k]] + proc_address[idmap[k]];
		    }   
		}   
	    }   
	    else 
	    { // otherwise recursively call for the next faster varying dimension
		get_num_tiles(cur_dim+1, cur_indices, offset, idmap, proc_address, addresses, phy_grid, num_tiles);
	    }   
	}
	//addresses = (int*)realloc(addresses, num_tiles*dims*sizeof(int));
	//int* old_proc_addr = new int[num_tiles * dims];
	//if(rank==1) cout << "Hi4" << endl;
	return num_tiles;
    }


// Fill data in the newly created sub-tensor
    void Tensor::fill_data(double* &blocks, int* &addresses)
    {
	tensor_tiles = blocks;
	tile_address = addresses;
    }


// Generate a value for a data-point in the tensor
    double Tensor::get_value(int* &indices) 
    {
	double value = 0;
	int x = 1;
	int* virtual_indices = new int[dims];
	getVirtualAddress(indices, virtual_indices, proc_addr, index_dimension_map, pgrid);

	for(int i=dims-1; i>=0; i--)
	{	
	    value += sin(virtual_indices[i]);
	}
	delete[] virtual_indices;
	return value;
    }


// Find the index bound for current dimension based on the symmetry
    int Tensor::get_bound(int cur_dim, int *local_indices, int* &idmap, int* &phy_grid) 
    {
	if(cur_dim == -1) return 1;
	char sym_id = tensor_str.at(cur_dim);
	if(sym_id == 'c' || sym_id == 'd' || sym_id == 'C' || sym_id == 'D' || cur_dim == 0) //If this dimension is not symmetric or is the first in a symmetry
	{
	    return ceil((float)vgrid[cur_dim] / phy_grid[idmap[cur_dim]]);
	}

	// Otherwise the bound is the current index in previous dimension in the symmetry
	for(int i = cur_dim-1; i >= 0; i--) 
	{
	    if( ((sym_id=='a' || sym_id=='A') && (tensor_str.at(i) == 'a' || tensor_str.at(i) == 'A')) ||
		((sym_id=='b' || sym_id=='B') && (tensor_str.at(i) == 'b' || tensor_str.at(i) == 'B')))
	    {
		return local_indices[i] + 1;
	    }
	}

	return ceil((float)vgrid[cur_dim] / phy_grid[idmap[cur_dim]]);
    }


// Marks the index as already contracted
    void Tensor::removecntrIndex(int indx)
    {
	SG_index_map[indx] = CONTRACTED;
    }


// gives a list of indices that need to be bounced
// it is used by get bouncer to receive addresses of
//the nodes that will send bouncing data
    list<int>* Tensor::get_bounce_indx(int indx)
    {
	assert(cntr_map[indx]>0);
	list<int>* bounce = new list<int>;

	// If the index does not belong to any symmetry group
	// then return null
	if (SG_index_map[indx] > 1)
	    //if (SG_index_map[indx] == 2)
	{
	    bounce->push_back(indx);
	    return bounce;
	}

	for(int i = 0; i< dims; i++)
	{
	    //if the index belongs to same symmetry group as indx
	    //then index i should be bounced
	    if(SG_index_map[indx] == SG_index_map[i])
	    {
		//if there are multiple contraction indiciies within the
		//same symmetry group. The relation between contraction
		//indicies must be preserved and hence such i should not
		//be greater than indx. For example A[i1>i2>k>i3>l>i4] where
 		//k and l are the contraction indicies then no need to bounce
		//A[i1,i2,i3,l,k,i4] since this will make l>k, but contracting
		//k>l suffice to produce correct result
		if(cntr_map[i] > 0 && i>indx)
		{
		    // bounce->remove(indx);
		    return bounce;
		}
		bounce->push_back(i);
	    }
	}
	// bounce->remove(indx);
	return bounce;
    }


//outputs symmetry group and contraction index information
//contraction index is 1 if it is part of some symmetry group
//if it is not then it is either 1 or 2. The numbers are used
//to identify the contraction index. In case of symmetry,
//it is not important to distinguish them
    void Tensor::printInfo()
    {
	cout<<endl<<"Index | Symmetry Group | Is Contraction |Index Dimension Map | Tensor Size | Vgrid\n";
	for(int i = 0; i<dims; i++)
	{
	    cout<<" "<<i<<"                "<<SG_index_map[i]<<"              "<<cntr_map[i]<<"                   "<<index_dimension_map[i]<<"               "<<tensor_size[i]<<"        "<<vgrid[i]<<"\n";

	}
    }


//returns number of processor addresses from which data needs to be
//bounced.THe array of addresses are put in bouncers. my_address holds
//the invoking nodes address, index_dimension_map gives the mapping
//from index to processor dimension
    int Tensor::get_bouncers(int index, int** &bouncers)
    {
	// Get the list of all dimensions which belong to the same symmetry group
	// as the contracting index
	list<int>* br_index = get_bounce_indx(index);
	int num_recieve_indicies = br_index->size();

	// Allocate space for the bouncers' addresses  
	int* physical_dimensions = new int[num_recieve_indicies];
	int** bouncers_temp = new int*[num_recieve_indicies];
	for(int i =0; i< num_recieve_indicies; i++)
	{
	    bouncers_temp[i] = new int[grid_dims];
	    //bouncers_temp[i] = new int[dims];
	}

	// Initialize the bouncers' addresses
	for(int i = 0; i<num_recieve_indicies; i++)
	{
	    memcpy(bouncers_temp[i], proc_addr, sizeof(int)*grid_dims);
	    //memcpy(bouncers_temp[i], proc_addr, sizeof(int)*dims);
	}

	int contract_index;
	int count = 0;

	// Find the processor grid dimension where these dimensions are mapped
	for(list<int>::iterator it = br_index->begin(); it != br_index->end(); ++it)
	{
	    if(*it == index)
		contract_index = count;

	    physical_dimensions[count] = index_dimension_map[*it];
	    count ++;
	}

	// For each of these indices, permute the initialized addresses to get
	// the bouncers' addresses
	for(int i = 0; i<num_recieve_indicies; i++)
	{
	    if(i < contract_index)
	    {
		int temp = bouncers_temp[i][physical_dimensions[i]];
		bouncers_temp[i][physical_dimensions[i]] = proc_addr[physical_dimensions[contract_index]];
		for(int j = i + 1; j <=contract_index; j++)
		{
		    int temp1 = bouncers_temp[i][physical_dimensions[j]];
		    bouncers_temp[i][physical_dimensions[j]] = temp;
		    temp = temp1;
		}
	    }
	    else if(i > contract_index)
	    {
		for(int j = contract_index; j <i; j++)
		{
		    bouncers_temp[i][physical_dimensions[j]] = proc_addr[physical_dimensions[j+1]];
		}
		bouncers_temp[i][physical_dimensions[i]] = proc_addr[physical_dimensions[contract_index]];
	    }
	}
	bouncers = bouncers_temp;

	delete[] physical_dimensions;
	br_index->clear();
	delete br_index;
	/*cout << rank << ":[ " << proc_addr[0] << ", " << proc_addr[1] << ", " <<proc_addr[2] << ", " <<proc_addr[3] << "] " << endl;
	  + for(int i =0;i<num_recieve_indicies;i++)
	  + {
	  + for(int d = 0; d<dims; d++)
	  + cout<<bouncers[i][d]<<" ";
	  + cout<<endl;
	  + }
	  + */
	return num_recieve_indicies;

    }

// Gets receiver addresses for current contracting dimension and index
    int Tensor::get_receivers(
	int contr_dim,
	int contr_idx,
	int** &receivers, //out
	int* &matching_indices //out
	)
    {
	// matching indices are the ones that are symmetric to the contraction indices
	int* match = new int[dims];     
	memset(match, 0, dims*sizeof(int));

	// Counts the number of processors that receive from this processor
	// and finds the indices that are symmetric to the contracting index
	int count = 0;
	//for(int i =0; i < dims; i++){
	//	if (SG_index_map[i] == SG_index_map[contr_dim] &&
	//        SG_index_map[i] <=1 &&

	for(int i =0; i < dims; i++)
	{
	    // index i is in the same symmetry group as the contracting dimension
	    if (((SG_index_map[i] == SG_index_map[contr_dim] &&
		  // Make sure that the index we are checking is a part of some symmetry group
		  SG_index_map[i] != NON_SYM)  ||  
		 // Or ensure this is the contracting dimension (then self sends)
		 i == contr_dim)  &&  
		// Make sure that this processor holds the contraction index
		proc_addr[index_dimension_map[i]] == contr_idx % pgrid[index_dimension_map[i]])
	    {
		// If there are multiple contraction indices belonging to the same symmetry group,
		// this condition maintains the order between these contraction indices
		if(contr_dim < i && cntr_map[i] > 0 )
		{
		    break;
		}
		match[i] = 1;
		count++;
	    }
	}

	//cout<<"count is"<<count<<endl;
	receivers = new int*[count];
	matching_indices = new int[count];

	// Identify the receiver addresses
	count = 0;
	for(int i =0; i < dims; i++)
	{
	    // If a receiver is identified for this dimension
	    if(match[i])
	    {
		// Initialize receiver address
		receivers[count] = new int[grid_dims];
		memcpy(receivers[count], proc_addr, grid_dims * sizeof(int));
			
		// Permute the initialized address to get the receiver address
		// depending on the position of this dimension respective of the contracting dimension
		if(i<contr_dim)
		{
		    for(int j = i; j< contr_dim; j++)
		    {
			receivers[count][index_dimension_map[j]] = receivers[count][index_dimension_map[j+1]];
		    }
		}
		if(i>contr_dim)
		{
		    for(int j = i; j> contr_dim; j--)
		    {
			receivers[count][index_dimension_map[j]] = receivers[count][index_dimension_map[j-1]];
		    }
		}

		receivers[count][index_dimension_map[contr_dim]] = contr_idx % pgrid[index_dimension_map[contr_dim]];
		matching_indices[count] = i;
		count++;
	    }
	}
	delete[] match;
	return count;
    }


// Extract a sub-tensor from the parent tensor w.r.t given contracting dimension
// data blocks are passed as input
    Tensor* Tensor::generate_tensor(int contr_dim,
				    double* &data_blocks,
				    int* &block_addresses,
				    int num_tiles)
    {
	int* new_vgrid = new int[dims];
	memcpy(new_vgrid, vgrid, dims * sizeof(int));
	new_vgrid[contr_dim] = 1;

	int* new_tensor_size = new int[dims];
	memcpy(new_tensor_size, tensor_size, dims * sizeof(int));
	new_tensor_size[contr_dim] = block_range[contr_dim];

	Tensor *X = new Tensor(tensor_str, index_dimension_map, new_tensor_size, new_vgrid, g);

	memcpy(X->cntr_map, cntr_map, dims*sizeof(int));
	memcpy(X->SG_index_map, SG_index_map, dims*sizeof(int));

	X->removecntrIndex(contr_dim);
	X->fill_data(data_blocks, block_addresses);
	X->set_num_actual_tiles(num_tiles);
	X->init_index_table();
	X->fill_index_table();
	//cout<<X->get_num_actual_tiles();
	return X;
    }

// Extract a sub-tensor from the parent tensor w.r.t given contracting dimension
// data blocks are passed as input
    Tensor* Tensor::generate_tensor_with_new_tiles(double* &data_blocks,
						   int* &block_addresses,
						   int num_tiles)
    {
	int* new_vgrid = new int[dims];
	memcpy(new_vgrid, vgrid, dims * sizeof(int));
	
	int* new_tensor_size = new int[dims];
	memcpy(new_tensor_size, tensor_size, dims * sizeof(int));
	

	Tensor *X = new Tensor(tensor_str, index_dimension_map, new_tensor_size, new_vgrid, g);

	memcpy(X->cntr_map, cntr_map, dims*sizeof(int));
	X->fill_data(data_blocks, block_addresses);
	X->set_num_actual_tiles(num_tiles);
	X->init_index_table();
	X->fill_index_table();
	//cout<<X->get_num_actual_tiles();
	return X;
    }

// Generate a deserialized subtensor
    Tensor* Tensor::generate_deserialized_tensor(int num_des_dims, int* &des_tensor_dims, int* &des_grid_dims, 
						 double* &data_blocks, int* &block_addresses, int num_tiles)
    {
	int* new_idmap = new int[dims];
	memcpy(new_idmap, index_dimension_map, dims*sizeof(int));
	for(int i=0; i<num_des_dims; i++)
	{
	    new_idmap[des_tensor_dims[i]] = des_grid_dims[i];
	}

	int* v_grid = new int[dims];
	memcpy(v_grid, vgrid, dims * sizeof(int));

	int* tnsr_size = new int[dims];
	memcpy(tnsr_size, tensor_size, dims * sizeof(int));

	Tensor *X = new Tensor(tensor_str, new_idmap, tnsr_size, v_grid, g);
	memcpy(X->cntr_map, cntr_map, dims * sizeof(int));
	X->fill_data(data_blocks, block_addresses);
	X->set_num_actual_tiles(num_tiles);

	// Remove symmetry from the deserialized dimension
	for(int i=0; i<num_des_dims; i++)
	{
	    int s = X->SG_index_map[des_tensor_dims[i]];
	    for(int j=0; j<X->dims; j++)
	    {
		if(X->SG_index_map[j] == s)
		{
		    X->SG_index_map[j] = NON_SYM;
		}
	    }
	}

	X->init_index_table();
	X->fill_index_table();
	return X;
    }

// Check if the given tensor has the same tile addresses as this tensor
// at this processor
    bool Tensor::check_same_tiles(Tensor* &T)
    {
	for(int i=0; i < T->num_actual_tiles; i++)
	{
	    int* addr = T->tile_address + i * T->dims;
	    if(tile_exists(addr) == -1)
	    {
		return false;
	    }
	}
	return true;
    }

// Check if the given tile exists in this tensor and return its index
    int Tensor::tile_exists(int* &search_addr)
    {
	for(int j=0; j < num_actual_tiles; j++)
	{ 
	    int* target_addr = tile_address + j * dims;
	    if(compare_addresses(search_addr, target_addr) == true)
	    {
		return j;
	    }
	}
	return -1;
    }

//set the boolean for this address to true
//only for debugging. remove later
//asserts if address not found
    int Tensor::touch_address(int* &search_addr)
    {
	int addr = get_tile_offset(search_addr);

	assert(addr>=0);
	int* addresss= tile_address + addr*dims;
	//if(rank == 0){cout<<"touching ";print_tile_addr(dims,addresss);}

	is_touched[addr] = 1;
	return 1;

    }

//sets boolean for all address to false
    void Tensor:: untouch_all_address(){

	for(int i = 0; i< num_actual_tiles; i++)
	{
	    is_touched[i] = 0;
	}
    }

//checks if all addresses have been touched
    int Tensor::is_all_touched(){

	for(int i = 0; i< num_actual_tiles; i++)
	{
	    if(is_touched[i] == 0)
	    {
		int* addresss= tile_address + i*dims;
		if(rank == 0)print_tile_addr(dims,addresss);
		//return 0;
	    }
	}
	return 1;

    }


//returns the pointer to the data block specified by block_address;
    int Tensor::get_tile_offset(int* &block_address)
    {
	std::map<int,int>::iterator offset;
	offset = block_addr_to_blk_num.find(get_block_number(block_address, dims, vgrid));
	if(offset == block_addr_to_blk_num.end())
	    return -1;
	else
	    return offset->second;
    }


// Compare two tile addresses
    bool Tensor::compare_addresses(int dims, int* &addr_1, int* &addr_2)
    {
	for(int d=0; d<dims; d++)
	{
	    if(addr_1[d] != addr_2[d])
		return false;
	}
	return true;
    }

// Compare two tile addresses
    bool Tensor::compare_addresses(int* &addr_1, int* &addr_2)
    {
	for(int d=0; d<dims; d++)
	{
	    if(addr_1[d] != addr_2[d])
		return false;
	}
	return true;
    }

// Check if the address satisfies tensor symmetry criterion.
// Ignores contraction indices while checking
    bool Tensor::satisfies_sym(int* &addr)
    {
	for(int i=0; i < dims; i++)
	{
	    if(SG_index_map_permanent[i] != NON_SYM)
	    {
		int sym_id = SG_index_map_permanent[i];
		int n = addr[i];
		for(int j=i; j < dims; j++)
		{
		    if(SG_index_map_permanent[j] == SG_index_map_permanent[i] &&
		       ((cntr_map[i] > 0 &&  cntr_map[j] > 0) || (cntr_map[i] == 0 &&  cntr_map[j] == 0))) 
		    {
			if(n >= addr[j])
			    n = addr[j];
			else
			    return false;
		    }
		}
	    }
	}
	return true;
    }

//
    void printList(list<int> bounce)
    {
	cout<<"The bouncing indices are :: ";

	for(list<int>::iterator i = bounce.begin(); i !=bounce.end(); ++i)
	{
	    cout<<*i<<", ";
	}
	cout<<endl;
    }


//
    void print_double_Array(int inner_max, int outer_max, int** bouncers)
    {
	for(int i = 0; i< inner_max; i++)
	{
	    cout<<"[";
	    for(int j = 0; j < outer_max; j++)
	    {
		cout<<bouncers[i][j]<<", ";
	    }
	    cout<<"]\n";
			    }
    }
    
    void Tensor::set_index_name(int* index_name){
	for(int i =0; i< dims; i++)
	    ctce_index_name[i] = index_name[i];
    }
    

}

