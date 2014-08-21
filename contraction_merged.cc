#include "contraction.h"
#define CCHECK 0
#define RRANK 0
#define DEBUG_T 0
#define DISPLAY_TIME 0
#define COST_EVAL 0
using namespace std;

Contraction::Contraction(Grid* &g)
{
	grid = g;
	rank = grid->rank;
	num_procs = grid->nprocs;

    grid_dims = grid->grid_dims;
    serial = grid_dims;
    pgrid = new int[grid_dims + 1];
    my_address = new int[grid_dims + 1];
    memcpy(pgrid, grid->pgrid, grid_dims * sizeof(int));
    memcpy(my_address, grid->proc_addr, grid_dims * sizeof(int));
    pgrid[grid_dims] = 1;
    my_address[grid_dims] = 0;
}

Contraction::Contraction(Tensor* &a, Tensor* &b, Tensor* &c, Grid* &g)
{
	grid = g;
	rank = grid->rank;
	num_procs = grid->nprocs;

    A = a;
    B = b;
    C = c;

    //transpose flags for A and B for local dgemm
    t_A = 0;
    t_B = 0;


    //stores mapping information regarding transposing blocks of A and B and C before dgemm can be called
    p_map_A = new int[A->dims];
    p_map_B = new int[B->dims];
    p_map_C = new int[C->dims];

    //stores transpose mapping information regarding blocks of C after local dgemm is called
    inv_p_map_C = new int[C->dims];
    inverse_block_range_C = new int[C->dims];

    //holds information on if indicies of C are from A or from B
    is_C_indx_ext_A_or_ext_B = new int[C->dims];
    local_num_dgemm = 0;
    total_num_dgemm = 0;
    num_self_sends = 0;
    grid_dims = grid->grid_dims;
    serial = grid_dims;
    pgrid = new int[grid_dims + 1];
    my_address = new int[grid_dims + 1];
    memcpy(pgrid, grid->pgrid, grid_dims * sizeof(int));
	memcpy(my_address, grid->proc_addr, grid_dims * sizeof(int));
    pgrid[grid_dims] = 1;
    my_address[grid_dims] = 0;
}

Contraction::~Contraction()
{
    delete[] p_map_A;
    delete[] p_map_B;
    delete[] p_map_C;
    delete[] inv_p_map_C;

    delete[] is_C_indx_ext_A_or_ext_B;
    delete[] pgrid;
    delete[] my_address;

    delete[] A_to_C_map;
    delete[] B_to_C_map;
}

//returns a permutation map that combines the affect of 
//applying map1 and map2 consecutively
void Contraction::compose_p_maps(Tensor* &T, int* &map1, int* &map2, int* &composed_map)
{

    for(int i = 0; i<T->dims; i++)
    {
        composed_map[i] = map2[map1[i]];
    }
}


//generates permutation maps based on symmetry for bounced blocks
void Contraction::get_symm_permutation(Tensor* &T, int* &tile_address, int* &sym_permutation)
{

    for(int i =0; i<T->dims; i++)
    {
        sym_permutation[i] = i;
    }
    for(int i =0; i<T->dims; i++)
    {
        for(int j = i+1; j<T->dims; j++)
        {
            //if one component of the tile address is less than the other
            //and the pair of tile address belong to the same symmetry group
            //then such tile address must have been created by flipping the 
            //indices in the original tile

            if((tile_address[i] < tile_address[j])
                    && T->SG_index_map_permanent[i] == T->SG_index_map_permanent[j] 
                    && T->SG_index_map_permanent[i] <= 1)
            {
                sym_permutation[i] = j;
                sym_permutation[j] = i;
                break;
            }

        }

    }
}

//generates permutation maps for A, B and C for doing transpose before local dgemm can be performed
//also generates the size of external and internal indices
void Contraction::generate_permutation_map(Tensor* &A, Tensor* &B, Tensor* &C, list<pair<int,int>> &contr_list)
{


    //-----------------------Find if the external indices are mixed in C--------//
    //stores if the first index of C is from A or from B
    /*    int first = is_C_indx_ext_A_or_ext_B[0];
          bool mixed = false;

    //has encountered stores if external indices from A or B or both have been encountered
    //0 is for external index from A and 1 is for external index from B
    bool has_encountered[2] = {false, false};

    for(int i = 0; i<C->dims; i++)
    {	
    //if both external indices from A and B have been encountered and the index currently encountered
    //is same as the first index then it must mean that the indices are mixed.
    //C has to be transposed
    if(has_encountered[0] && has_encountered[1] && is_C_indx_ext_A_or_ext_B[i] == first)
    {
    mixed = true;
    break;
    }

    //set the index encountered at i to be true
    has_encountered[is_C_indx_ext_A_or_ext_B[i] - 1] = true;
    }*/

    //----------------------------------------------------------------------//

    int num_external_A = 0;
    int num_external_B = 0;
    int num_contr_indx= 0;

    //contraction size for A and B
    int nk_a = 1;
    int nk_b = 1;


    /*This is a really dumb implementation of permutation map generator.
      It works but will not generate the most efficient transposes. It always 
      generates transpsosed blocks of the form [e1,e2,e3,k1,k2,k3]*/


    for(int i = 0; i<A->dims; i++)
    {
        //if i is an external index then map it to the number of external indicies discovered so far
        if(A->cntr_map[i] == 0)
        {
            //computes the size of the external index in B
            n_a = n_a * A->block_range[i];

            p_map_A[i] = num_external_A;

            //map Cs corresponding external index to same as that of A
            p_map_C[A_to_C_map[i]] = num_external_A;

            //stores the block range of the transposed block of C for inverse mapping
            inverse_block_range_C[num_external_A] = C->block_range[A_to_C_map[i]];

            //stores the inverse mapping from transposed block of C to original block of C
            inv_p_map_C[num_external_A] = A_to_C_map[i];

            num_external_A++;


        }else{
            nk_a = nk_a*A->block_range[i];
        }
    }


    for(int i = 0; i<B->dims; i++)
    {
        if(B->cntr_map[i] == 0)
        {
            //computes the size of the external index in B
            n_b = n_b * B->block_range[i];

            p_map_B[i] = num_external_B;

            //map Cs corresponding external index to same as that of B
            //offsetted by the total number of external indices of A
            p_map_C[B_to_C_map[i]] = num_external_A + num_external_B;

            //stores the block range of the transposed block of C for inverse mapping
            inverse_block_range_C[num_external_A + num_external_B] = C->block_range[B_to_C_map[i]];

            //stores the inverse mapping from transposed block of C to original block of C
            inv_p_map_C[num_external_A + num_external_B] = B_to_C_map[i];

            num_external_B++;

        }
        else
        {
            nk_b = nk_b * B->block_range[i];
        }
    }

    //makes sure the size of the contracting indices are equal in A and B
    assert(nk_a == nk_b);
    n_k = nk_a;

    for (std::list<pair<int,int>>::iterator it = contr_list.begin(); it != contr_list.end(); it++)
    {
        p_map_A[(*it).first] = num_external_A + num_contr_indx;
        p_map_B[(*it).second] = num_external_B + num_contr_indx;;
        num_contr_indx++;
    }	
}

// Asserts validity of specified contraction
void Contraction::assert_contr_validity(Tensor* &A, Tensor* &B)
{
    // Check if all external indices (non-contracting indices) are orthogonal
    for(int i=0; i < A->dims; i++)
    {  
        int dim_A = A->index_dimension_map[i];
        if(dim_A != serial)
        {
            int index_B = -1;
            for(int j=0; j < B->dims; j++)
            {
                if(B->index_dimension_map[j] == dim_A  &&  B->index_dimension_map[j] != A->dims)
                {
                    index_B = j;
                }
            }
	    //index_B is -1 noly if there is an index of A is overlapping 
	    //with index of B. We want the assertion to be false only 
	    //if external indices overlap. Internal indices can be handled 
	    //with reduction
            if(index_B != -1 && !(A->cntr_map[i] == 1 && B->cntr_map[index_B] ==1))
            {
		
                assert(A->cntr_map[i] != B->cntr_map[index_B]);
            }
        }
    }

    //When there is no reduction
    // Check if same contracting index in different tensors does not overlap
    for(int i=0; i<A->dims; i++)
    {
        for(int j=0; j<B->dims; j++)
        {
            if(A->cntr_map[i] == B->cntr_map[j] &&  A->index_dimension_map[i] != serial  &&  B->index_dimension_map[j] != serial)
            {
                if(A->contr_dim_str[i].compare(B->contr_dim_str[j]) == 0)
                {
                    assert(A->index_dimension_map[i] != B->index_dimension_map[j]);
                }
            }
        }
    }

    // Find out number of contracting indices in each tensor, and check if they are equal
    int num_contr_ind_A = 0, num_contr_ind_B = 0;
    for(int i=0; i< A->get_dims(); i++)
        if(A->cntr_map[i] > 0) num_contr_ind_A++;
    for(int i=0; i< B->get_dims(); i++)
        if(B->cntr_map[i] > 0) num_contr_ind_B++;

    assert(num_contr_ind_A == num_contr_ind_B);
}

//stores the address of C corresponding to address of A and B in C_address
void Contraction::get_C_address(int* &A_address,int dims_a, int* &B_address, int dims_b, int* &C_address, int dims_c)
{
    C_address = new int[dims_c];

    //finds the common indicies between A and C
    for(int it_a = 0; it_a<dims_a; it_a++)
    {
        int map_A = A_to_C_map[it_a];
        if( map_A != -1) 
        {
            C_address[map_A] = A_address[it_a];
        } 
    }

    //finds the common indicies between B and C
    for(int it_b = 0; it_b<dims_b; it_b++)
    {
        int map_B = B_to_C_map[it_b];
        if( map_B != -1) 
        {
            C_address[map_B] = B_address[it_b];
        } 
    }
}


void Contraction::get_C_addresses(Tensor* &T_A, Tensor* &T_B, Tensor* &T_C, int* &C_addresses, map<int,int>* &address_to_location)
{

    address_to_location = new map<int,int>();
    C_addresses = new int[T_C->dims * T_C->num_actual_tiles];

    int* A_addresses = T_A->tile_address;
    int* B_addresses = T_B->tile_address;

    int A_num_tiles = T_A->num_actual_tiles;
    int B_num_tiles = T_B->num_actual_tiles;

    //stores the permuted address of A and B
    int* A_perm_address = new int[T_A->dims * A_num_tiles];
    int* B_perm_address = new int[T_B->dims * B_num_tiles];

    /*Created permuted addresses of A and B to create the permuted
     * addresses of C*/
    int* source = A_addresses;
    int* dest = A_perm_address;
    
    for(int i_a = 0; i_a<A_num_tiles; i_a++)
    {
	for(int d = 0; d< T_A->dims; d++)
	{
	    dest[p_map_A[d]] = source[d];
	} 

	source += T_A->dims;
	dest += T_A->dims;
    }    


    source = B_addresses;
    dest = B_perm_address;
    
    for(int i_b = 0; i_b<B_num_tiles; i_b++)
    {
	for(int d = 0; d< T_B->dims; d++)
	{
	    dest[p_map_B[d]] = source[d];
	} 

	source += T_B->dims;
	dest += T_B->dims;
    } 

    /*Create permuted address of C. Unpermute the address to find
     * where theses address are stored in the C tensor. If the
     * address is found then store it in the address map. Also
     * fill up Cs addresses along the way*/
    int* C_perm_address = new int[T_C->dims * sizeof(int)];;
    int* C_address = new int[T_C->dims * sizeof(int)];

    //Use grid_C to find rank of a given address of C within the C_grid
    int C_dims = T_C->dims;
    int* C_grid = new int[C_dims];
    for(int d = 0; d< C_dims; d++)
    {
	C_grid[p_map_C[d]] = T_C->get_vgrid()[d];
    }
   
    Block_Grid* grid_C = new Block_Grid(C_dims, C_grid);

    int* source_A = A_perm_address;
    int* source_B = B_perm_address;
    
    dest = C_addresses;

    /*Create addresses of C based on addresses of A then looks for
     * it in C tensor. Creates a map of these address to where
     * they are located. The addresses are converted to address
     * rank within the C tensor. These ranks are used as keys for
     * the map*/
    for(int i_a = 0; i_a<A_num_tiles; i_a++)
    {
	source_B = B_perm_address;
	for(int i_b = 0; i_b<B_num_tiles; i_b++)
	{
	    
	    for(int j = 0; j<(A->dims-num_cntr_indices); j++)
	    {
		C_perm_address[j] = source_A[j];
	    }

	    for(int j = 0; j<(B->dims-num_cntr_indices); j++)
	    {
		C_perm_address[j + A->dims-num_cntr_indices] = source_B[j];
	    }

	    for(int d = 0; d< T_C->dims; d++)
	    {
		C_address[inv_p_map_C[d]] = C_perm_address[d];
	    }

	    int c_rank = grid_C->get_block_rank(C_perm_address);

	    /*if this address has not been put in the map already
	     * then find the location of this address and also
	     * copy it to C_addresses*/
	    if(address_to_location->find(c_rank) == address_to_location->end())
	    {
		
		int C_location = T_C->getTile(C_address);
		
		//address not found. Something went wrong Shieeetttt!
		assert(C_location != -1);
		
		//copy the address 
		memcpy(dest, C_perm_address, T_C->dims * sizeof(int));
		dest+= T_C->dims;

		//address was found
		(*address_to_location)[c_rank] = C_location;
	    }


	    source_B += T_B->dims;
	}
	source_A += T_A->dims;
    }

}


// Extracts blocks from the serialized input tensor that are required
// for computing blocks in C along a dimension that is serialized in the
// input tensor but distributed in C
int Contraction::get_blks_from_serialized(int num_dim, string* &ext_dim, int* &ext_grid_dim, Tensor* &T, double* &blocks, int* &addresses)
{

    // Find the external dimension number in T
    int* ext_T = new int[num_dim];
    list<int>* sym_list = new list<int>[num_dim];

    for(int j=0; j<num_dim; j++)
    {
        for(int i=0; i<T->dims; i++)
        {
            if(ext_dim[j].compare(T->contr_dim_str[i]) == 0)
            {
                ext_T[j] = i;

                // Once the external dimension is found in T, check if it is symmetric.
                // If yes, find the dimensions symmetric to it
                sym_list[j] = list<int>();
                if(T->SG_index_map[i] < 2) // this is a symmetric dimension
                {
                    for(int k=0; k < T->dims; k++)
                    {
                        if(k != i  &&  T->SG_index_map[k] == T->SG_index_map[i])
                        {
                            sym_list[j].push_back(k);
                        }
                    }
                }
            }
        }
    }

    // Find the external dimension number in C
    int* ext_C = new int[num_dim];
    for(int j=0; j<num_dim; j++)
    {
        for(int i=0; i<C->dims; i++)
        {
            if(ext_dim[j].compare(C->contr_dim_str[i]) == 0)
            {
                ext_C[j] = i;
            }
        }
    }


    // The grid dimension where this external dimension of C will be distributed
    ext_grid_dim = new int[num_dim];
    for(int i=0; i<num_dim; i++)
    {
        ext_grid_dim[i] = C->index_dimension_map[ext_C[i]];
    }
    delete[] ext_C;

    return deserialize(T, blocks, addresses, num_dim, ext_T, ext_grid_dim, sym_list);
}

int Contraction::deserialize(Tensor* &T, double* &blocks, int* &addresses, int num_dim, int* &ext_T, int* &ext_grid_dim, list<int>* &sym_list)
{


    // Find number of blocks to retain and mark them for retention
    bool* retain = new bool[T->num_actual_tiles];
    // If the block is retained due to symmetry, save the dimension symmetric with the deserializing dimesion
    int* retain_sym = new int[T->num_actual_tiles]; 
    // Mark blocks for duplication in this flag array, store the duplicate block's address
    int** duplicate_addr = new int*[T->num_actual_tiles];
    // Maintain tile number for duplication
    list<int> duplicates = list<int>();

    for(int i=0; i < T->num_actual_tiles; i++) 
    {
        retain[i] = true;
        retain_sym[i] = -1;
    }
    int retain_count = 0;
    for(int i=0; i < T->num_actual_tiles; i++)
    {
        int* addr = T->tile_address + i * T->dims;
        // Check if this block is meant to be retained
        for(int j=0; j<num_dim; j++)
        {
            // Do not retain the block if it should not belong here after deserialization
            if(addr[ext_T[j]] % pgrid[ext_grid_dim[j]] != my_address[ext_grid_dim[j]])
            {
                retain[i] = false;
                break;
            }
        }

        // Check if this block should be retained due to symmetry
        for(int j=0; j<num_dim; j++)
        {
            for(list<int>::iterator it = sym_list[j].begin(); it != sym_list[j].end(); it++)
            {
                if(addr[*it] % pgrid[ext_grid_dim[j]] == my_address[ext_grid_dim[j]])
                {
                    // If already marked retained, that means the block should be 
                    // retained due to its current address as well as its symmetric address
                    // make copy of this block with permuted address
                    if(retain[i] == true  &&  addr[*it] != addr[ext_T[j]])
                    {
                        // Only implementing for 2D tensors for now
                        duplicate_addr[i] = new int[T->dims];
                        duplicate_addr[i][*it] = addr[*it];
                        duplicate_addr[i][ext_T[j]] = addr[ext_T[j]];
                        duplicates.push_back(i);
                    }

                    // Mark for retention due to symmetry
                    retain[i] = true;
                    retain_sym[i] = *it;
                    break;
                }
            }
        }

        if(retain[i] == true)
        {
            retain_count++;
        }
    }
    retain_count += duplicates.size();

    // Copy blocks to retain to a new buffer
    blocks = new double[T->block_size * retain_count];
    addresses = new int[T->dims * retain_count];
    int block_offset = 0, addr_offset = 0;
    for(int i=0; i < T->num_actual_tiles; i++)
    {
        if(retain[i] == true)
        {
            // Copy memory block
            memcpy(blocks + block_offset, T->tensor_tiles + i * T->block_size, T->block_size * sizeof(double));
            block_offset += T->block_size;

            // Copy block address, and permute it as per the symmetry, if this block was retained due to symmetry
            int* addr = addresses + addr_offset;
            memcpy(addr, T->tile_address + i * T->dims, T->dims * sizeof(int));
            if(retain_sym[i] != -1) // If this block was retained due to symmetry, permute its address
            {
                int dim = -1;
                for(int j=0; j<num_dim; j++) // Find the deserializing dimension
                {
                    if(T->SG_index_map[ext_T[j]] == T->SG_index_map[retain_sym[i]])
                    {
                        dim = j;
                    }
                }
                assert(dim != -1);
                // Swap the indices such that the retained block has correct address
                swap(&addr[dim], &addr[retain_sym[i]]);
            }

            addr_offset += T->dims;
        }
    }

    // Copy blocks to be duplicated
    for(list<int>::iterator it = duplicates.begin(); it != duplicates.end(); it++)
    {
        int i = *it; // tile number

        memcpy(blocks + block_offset, T->tensor_tiles + i * T->block_size, T->block_size * sizeof(double));
        block_offset += T->block_size;

        memcpy(addresses + addr_offset, duplicate_addr[i], T->dims * sizeof(int));
        addr_offset += T->dims;
    }

    // Free memory
    delete[] sym_list;
    delete[] retain;
    delete[] retain_sym;
    delete[] duplicate_addr;

    return retain_count;
}

// Deserialize the 2D input tensor T if the other input tensor S is 4D and the output is 2D
Tensor* Contraction::deserialize_422(Tensor* &T, Tensor* &S)
{
    double* blocks;
    int* addresses;
    int num_dim = 2;
    int *des_tensor_dims = new int[2];
    int *des_grid_dims = new int[2];
    list<int>* sym_list = new list<int>[num_dim];

    // Search for contracting dimensions in 4D tensor and find what dimension of
    // physical grid they are distributed. 
    // Distribute corresponding contracting dimensions of the 2D input tensor 
    // along the same physical dimensions
    for(int i=0; i < T->dims; i++)
    {
        des_tensor_dims[i] = i;
        for(int j=0; j < S->dims; j++)
        {
            if(T->contr_dim_str[i].compare(S->contr_dim_str[j]) == 0)
            {
                des_grid_dims[i] = S->index_dimension_map[j];
            }
        }
    }

    if(T->SG_index_map[0] == T->SG_index_map[1] && T->SG_index_map[0] < 2)
    {
        sym_list[0] = list<int>();
        sym_list[1] = list<int>();
        sym_list[0].push_back(1);
        sym_list[1].push_back(0);
    }

    int num_blocks = deserialize(T, blocks, addresses, num_dim, des_tensor_dims, des_grid_dims, sym_list);

    return T->generate_deserialized_tensor(num_dim, des_tensor_dims, des_grid_dims, blocks, addresses, num_blocks);
}


// Collect data at instigator
// Processors send to instigator and instigator collects the data
int Contraction::instigate_collection(Tensor* &X, 
        int contr_dim, 
        int contr_idx,
        double* &blocks,
        int * &block_addrs)
{

    int send_addr_count, send_data_count;
    timer1 -= MPI_Wtime();
    send_to_instigator(X, contr_dim, contr_idx, send_addr_count, send_data_count);
    timer1 += MPI_Wtime();

    return recv_at_instigator(X, contr_dim, contr_idx,  blocks, block_addrs, send_addr_count, send_data_count);
}


// Send data to instigator
void Contraction::send_to_instigator(
        Tensor* &X,
        int contr_dim, 
        int contr_idx,
        int &count_addr_sends,
        int &count_data_sends
        ) 
{

    // Check if this node is responsible for sending bounce data
    int** receivers;
    int* matching_indices;

    count_addr_sends = 0;
    count_data_sends = 0;

    // Find receivers for this processor
    num_receivers = X->get_receivers(contr_dim, contr_idx, receivers, matching_indices);

    if(num_receivers == 0)
        return;

    // Get receivers' ranks
    grid->get_proc_ranks(num_receivers, receivers, current_receivers); 
    //if(rank == 1) cout << rank << " num_recvrs = " << num_receivers << " first = " << current_receivers[0] << endl << fflush;

    // Since ranks are available, delete the memory allocated for receiver addresses
    for(int i=0; i < num_receivers; i++)
    {
        delete[] receivers[i];
    }
    delete[] receivers;


    //count the number of self sends
    num_self_sends = 0;
    for(int j = 0; j< num_receivers; j++)
    {
        if(current_receivers[j] == rank)
        {
            num_self_sends++;
        }
        else
        {
            count_addr_sends++;
        }
    }

    //if(rank == rank) cout << rank << " num_self_sends = " << num_self_sends << endl << fflush;

    // Allocate MPI_Requests so that we can wait on them later
    send_req_addr = new MPI_Request[count_addr_sends];
    send_req_data = new MPI_Request[count_addr_sends];

    // Allocate memory for data that is already present in this processor
    if(num_self_sends != 0)
    {
        self_adr_sizes = new   int[num_self_sends];
        self_buffers = new double*[num_self_sends];
        self_addresses = new  int*[num_self_sends];
        self_data_sizes = new  int[num_self_sends];
    }
    else
    {
        self_adr_sizes = 0; 
        self_buffers = 0;
        self_addresses = 0;
        self_data_sizes = 0;
    }

    num_self_sends = 0;
    count_addr_sends = 0;
    int addr_tag = 1;
    int data_tag = 2;

    for(int i=0; i < num_receivers; i++)
    {

        double* blocks;
        int* block_addrs;
        int data_size, addr_size;
        MPI_Comm comm = MPI_COMM_WORLD;    
        //if(rank == 0) cout<<"matching index of reciever "<<i<<"is "<<matching_indices[i]<<endl;
        //if(rank==0)X->print_all_tile_addr();
        // Retrieve data to be sent 
        int num_tiles = X->getTiles(matching_indices[i], contr_idx, blocks, block_addrs);
        //if(rank == rank) cout<<rank<< " num tiles from reciever "<<i<<" is "<<num_tiles<<endl;

        // Permute virtual block address to match the contraction dimension
        if(matching_indices[i]  < contr_dim) // matching index comes before contracting dimension
        {
            for(int j = 0; j< num_tiles; j++)
            {
                for(int d = matching_indices[i]; d<contr_dim; d++)
                {
                    block_addrs[j * grid_dims + d] = block_addrs[j * grid_dims + d + 1] ;
                }
                block_addrs[j * grid_dims + contr_dim] = contr_idx;
            }
        }

        if(matching_indices[i]  > contr_dim) // matching index comes after contracting dimension
        {
            for(int j = 0; j< num_tiles; j++)
            {
                int temp = block_addrs[ j*grid_dims + matching_indices[i]];
                for(int d = matching_indices[i]; d>contr_dim; d--)
                {
                    block_addrs[j * grid_dims + d] = block_addrs[j * grid_dims + d - 1] ;
                }
                block_addrs[j * grid_dims + contr_dim] = temp;
            }
        }

        // Compute size of addresses
        addr_size = num_tiles * X->get_dims() ;

        // Size of data
        data_size = num_tiles * X->block_size;

        // Find receiver rank
        int receiver = current_receivers[i];//get_proc_rank(receivers[i], grid_dims, pgrid);
        //cout << rank << " receiver = "<<receiver<< endl;

        // Send addresses and blocks if receivers are not this processor (sender)
        if(receiver != rank)
        {
            // Send addresses
            //cout << grid->get_proc_addr_str(rank) << "  sending addr_size " << addr_size << " to " << grid->get_proc_addr_str(receiver) << endl;
			//cout << rank << "  sending addr_size " << addr_size << " to " << receiver << endl;
			MPI_Isend(block_addrs, addr_size, MPI_INT, receiver, addr_tag, comm, &send_req_addr[count_addr_sends]);
            count_addr_sends++;

            // Send Data
            if(addr_size)
            {
				//cout << grid->get_proc_addr_str(rank) << "  sending data_size " << data_size << " to " << grid->get_proc_addr_str(receiver) << endl;
                //cout << rank << "  sending data_size " << data_size << " to " << receiver << endl;
                MPI_Isend(blocks, data_size, MPI_DOUBLE, receiver, data_tag, comm, &send_req_data[count_data_sends]);
                count_data_sends++;
            }
        }
        else // if receiver is this processor (sender)
        { 
            self_buffers[num_self_sends] = blocks;
            self_addresses[num_self_sends] = block_addrs;
            self_data_sizes[num_self_sends]  = data_size;
            self_adr_sizes[num_self_sends] = addr_size;

            //if(rank==0){ printGetTiles(blocks, block_addrs, X->block_size, self_adr_sizes[num_self_sends]*sizeof(int)/X->dims, X->dims); }
            num_self_sends++;
        }
    }
    delete[] matching_indices;
}


// Receive data at instigator
int Contraction::recv_at_instigator (
        Tensor* &X, 
        int contr_dim, 
        int contr_idx, 
        double* &blocks,    // out
        int* &block_addrs,  // out
        int send_addr_count, 
        int send_data_count) 
{

    // Find the processor dimension along which the contracting dim of tensor is distributed
    int pindex = X->index_dimension_map[contr_dim];

    // If this processor is not an instigator, 
    // then wait for all the sends from this processor to finish
    if(contr_idx % pgrid[pindex] != my_address[pindex])
    {
        // Wait for address sends
        MPI_Status* statuses_addr = new MPI_Status[send_addr_count];
        MPI_Waitall(send_addr_count, send_req_addr, statuses_addr);

        // Wait for data blocks sends
        MPI_Status* statuses_data = new MPI_Status[send_data_count];
        MPI_Waitall(send_data_count, send_req_data, statuses_data);

        // Free memory
        if(send_addr_count > 0)
        {
            delete[] send_req_addr;
            delete[] statuses_addr;
        }
        if(send_data_count > 0)
        {
            delete[] send_req_data;
            delete[] statuses_data;
        }

        return 0;
    }

    // Find all the processors to get the data from
    int** bouncers;
    int* senders;
    num_senders = X->get_bouncers(contr_dim, bouncers);

	grid->get_proc_ranks(num_senders, bouncers, senders);
    current_senders = senders;

    // Free memory allocated for bouncer addresses, now that ranks are stored
    for(int i=0; i<num_senders; i++) 
    {
        delete[] bouncers[i];
    }   
    delete[] bouncers;

    // Sizes of addresses and blocks to be received
    int addr_size[num_senders];
    int data_size[num_senders];
    memset(addr_size, num_senders, 0);
    memset(data_size, num_senders, 0);

    // Temporarily hold data and addresses from each senders at locations pointed by these
    int** temp_block_addrs = new int*[num_senders];
    double** temp_blocks = new double*[num_senders];

    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Status status;
    int tag;

    // Probe the address messages sent and find out the number of blocks that are coming
    for(int i=0; i<num_senders; i++) 
    {
        if(senders[i] != rank)
        {
            //if(rank == rank) cout << rank << "  Probing for msg from " << senders[i] << endl;
            MPI_Probe(senders[i], 1, MPI_COMM_WORLD, &status);
            MPI_Get_count(&status, MPI_INT, &addr_size[i]);

            // Allocate memory for blocks and addresses using this count
            if(addr_size[i] != MPI_UNDEFINED)
            {
                data_size[i] = (addr_size[i]/X->dims)*X->block_size;
                temp_block_addrs[i] = new int[addr_size[i]];
                temp_blocks[i] = new double[data_size[i]];
            }
        }
    }  

    // Receive
    for(int i=0; i<num_senders; i++) 
    {
        MPI_Comm comm = MPI_COMM_WORLD;
        if(senders[i] != rank)
        {
            // Receive addresses
            if(addr_size[i] != MPI_UNDEFINED)
            {
                tag = 1; 
				//cout << grid->get_proc_addr_str(rank) << " receiving addr_size " << addr_size[i] << " to " << grid->get_proc_addr_str(senders[i]) << endl;
                //cout << rank << "  receiving addr_size " << addr_size[i] << " from " << senders[i] << endl;
                MPI_Recv(temp_block_addrs[i], addr_size[i], MPI_INT, senders[i], tag, comm, &status);
            }

            // Receive data blocks
            if(addr_size[i] != MPI_UNDEFINED & data_size[i] != 0)
            {
                tag = 2;
                //cout << grid->get_proc_addr_str(rank) << " receiving data_size " << data_size[i] << " to " <<grid->get_proc_addr_str(senders[i]) << endl;
				//cout << rank << "  receiving data_size " << data_size[i] << " from " << senders[i] << endl;
                MPI_Recv(temp_blocks[i], data_size[i], MPI_DOUBLE, senders[i], tag, comm, &status);
            }
        }
    }  

    // Wait for sends from this processor to finish
    MPI_Status* statuses_addr = new MPI_Status[send_addr_count];
    MPI_Waitall(send_addr_count, send_req_addr, statuses_addr);
    delete[] statuses_addr;

    MPI_Status* statuses_data = new MPI_Status[send_data_count];
    MPI_Waitall(send_data_count, send_req_data, statuses_data);
    delete[] statuses_data;

    // Count the total size of blocks and addresses received at the instigator (this processor)
    int total_block_size = 0;
    int total_address_size = 0;

    for(int i = 0; i<num_senders; i++)
    {
        if(senders[i] != rank)
        {
            total_block_size+=data_size[i];    
            total_address_size+=addr_size[i];
        }
    }

    // Merge the blocks that resided locally into the same set, removing duplicates.
    // Duplicates are found in different block sets because same blocks can be picked up
    // by multiple calls to get_tiles() at diagonals.
    double* out_blocks;
    int* out_addr;

    timer2 -= MPI_Wtime();
    int total_self_tiles = merge_self_send_blocks(X, out_blocks, out_addr);
    timer2 += MPI_Wtime();

    // Add these number of blocks to what we received
    total_block_size+= total_self_tiles * X->block_size;
    total_address_size+= total_self_tiles * X->dims;

    // Allocate memory for the total data collected at the instigator (this processor)
    blocks = new double[total_block_size];
    block_addrs = new int[total_address_size];

    // Temporary pointers for copying
    double* dest = blocks;
    int* dest_addr = block_addrs;

    timer3 -= MPI_Wtime();
    // Combine all the data recieved from remote processors
    for(int i = 0; i <num_senders; i++)
    {
        if(senders[i] != rank)
        {
            memcpy(dest, temp_blocks[i], data_size[i]*sizeof(double));
            memcpy(dest_addr, temp_block_addrs[i], addr_size[i]*sizeof(int));
            dest += data_size[i];
            dest_addr += addr_size[i];
        }
    }


    // Add the local (self-send) blocks to the combined data
    memcpy(dest, out_blocks, total_self_tiles * X->block_size * sizeof(double));
    memcpy(dest_addr, out_addr, total_self_tiles * X->dims*sizeof(int));
    dest += total_self_tiles * X->block_size;
    dest_addr += total_self_tiles * X->dims;
    timer3 += MPI_Wtime();
    /*
       if(rank==0) 
       {
       printGetTiles(blocks, block_addrs, X->block_size, total_block_size/X->block_size , X->dims);
       cout<<total_block_size/X->block_size;
       }
       */

    // Free temp blocks
    for(int i=0; i<num_senders; i++) 
    {
        if(senders[i] != rank  &&  addr_size[i] != MPI_UNDEFINED)
        {
            delete[] temp_block_addrs[i]; 
            delete[] temp_blocks[i]; 
        }
    }
    delete[] temp_block_addrs;
    delete[] temp_blocks;

    // Return total number of blocks collected at the instigator
    return total_block_size/X->block_size;
}


// Merges multiple block sets into one removing duplicates
// Returns number of block copied
int Contraction::merge_self_send_blocks(Tensor* &T, double* &out_blocks, int* &out_addr)
{

    // Count the total number of blocks
    int count = 0;
    for(int i=0; i<num_self_sends; i++)
    {
        count += self_adr_sizes[i] / T->dims;
    }

    // Allocate size for merged block and virtual address storage
    out_blocks = new double[count * T->block_size];
    out_addr = new int[count * T->dims];

    // Create temp pointers for out_blocks and out_addr
    double* temp_block_ptr = out_blocks;
    int* temp_addr_ptr = out_addr;

    // Create a hash map with block number as key and block memory address as value
    map<int, double*> block_map;

    // Count the copied blocks
    int distinct_count = 0;

    // Parse each block set
    for(int i=0; i<num_self_sends; i++)
    {
        int num_blocks = self_adr_sizes[i] / T->dims;

        // Go through all blocks in this set and check if they exist in the merged set
        // (check if their addresses are in block_map)
        for(int j=0; j<num_blocks; j++)
        {
            int* block_addr = self_addresses[i] + j*T->dims;
            int block_number = get_block_number(block_addr, T->dims, T->vgrid); 

            // If this block is not already copied, copy it. Otherwise, ignore and move forward.
            if(block_map.find(block_number) == block_map.end())
            {
                // Copy block
                double* block = self_buffers[i] + (j * T->block_size);
                memcpy(temp_block_ptr, block, T->block_size * sizeof(double));
                temp_block_ptr += T->block_size;

                // Copy address
                memcpy(temp_addr_ptr, block_addr, T->dims * sizeof(int));
                temp_addr_ptr += T->dims;

                distinct_count++;

                // Insert the pair in the map indicating it is now present in out_blocks
                block_map.insert(pair<int, double*>(block_number, block));
            }
        }
    }

    // Free memory
    delete[] self_data_sizes;
    delete[] self_adr_sizes;
    for(int i=0; i<num_self_sends; i++)
    {
        delete self_buffers[i];
        delete self_addresses[i];
    }

    // Shrink the memory allocated for output blocks and addresses if duplicates were found
    if(count != distinct_count)
    {
        out_blocks = (double*)realloc(out_blocks, distinct_count * T->block_size * sizeof(double));
        out_addr = (int*)realloc(out_addr, distinct_count * T->dims * sizeof(int));
    }

    return distinct_count;
}

void Contraction::rotate_forward_receive(Tensor* &T, int forward_receive_dim, 
					 int &source_num_blocks,
					 double* &source_data, int* &source_addr, 
					 int &receive_num_blocks,
					 double* &receive_data, int* &receive_addr)
{

    int block_size = T->block_size;
    int dims = T->dims;
    
    MPI_Request req_num, req_data, req_addr;
    MPI_Status recv_st_num, recv_st_data, recv_st_addr;
    MPI_Status send_st_num, send_st_data, send_st_addr;

    int next = next_processor(forward_receive_dim);
    int prev = prev_processor(forward_receive_dim);

    //Send the number of nodes to be sent from the current node and 
    //Get the number of blocks to be received from the previous node
    MPI_Isend(&source_num_blocks, 1, MPI_INT, next, 0, MPI_COMM_WORLD, &req_num);
    MPI_Recv(&receive_num_blocks, 1, MPI_INT, prev, 0, MPI_COMM_WORLD, &recv_st_num);
    MPI_Wait(&req_num, &send_st_num);

    receive_data = new double[receive_num_blocks * block_size];
    receive_addr = new int[receive_num_blocks * dims]; 

    //Send the data and address from the current node to the next node
    MPI_Isend(source_data, source_num_blocks * block_size, 
	      MPI_DOUBLE, next, 1, MPI_COMM_WORLD, &req_data);

    MPI_Isend(source_addr, source_num_blocks * dims, 
	      MPI_INT, next, 2, MPI_COMM_WORLD, &req_addr);

    
    //Receive the data and address from the previous node on the current node.
    MPI_Recv(receive_data, receive_num_blocks * block_size, 
	     MPI_DOUBLE, prev, 1, MPI_COMM_WORLD, &recv_st_num);

    MPI_Recv(receive_addr, receive_num_blocks * dims, 
	     MPI_INT, prev, 2, MPI_COMM_WORLD, &recv_st_num);

    //Wait for all the send to complete
    MPI_Wait(&req_data, &send_st_data);
    MPI_Wait(&req_addr, &send_st_addr);

    }

 
void Contraction::forward_int(Tensor* A, int* &data, int size, int k, int cdim, bool is_instigator)
{
    int mapped_cdim = A->index_dimension_map[cdim];
	int next = next_processor(mapped_cdim);
	int prev = prev_processor(mapped_cdim);
	if(rank != next)
    {
        if(is_instigator) // If this is the instigator
        {
            //cout << get_proc_addr_str(grid_dims, pgrid, rank) << " instigator  sending to " << get_proc_addr_str(grid_dims, pgrid, next_processor(mapped_cdim)) << endl;
			//cout << rank << "  sending to " << next << endl; 
			MPI_Send(data, size, MPI_INT, next, 1, MPI_COMM_WORLD);
        }
        else 
        {
            MPI_Status st;
            //cout << get_proc_addr_str(grid_dims, pgrid, rank) << "  recving fr " << get_proc_addr_str(grid_dims, pgrid, prev_processor(mapped_cdim)) << endl; 
            //cout << rank << "  recving fr " << prev << endl; 
			MPI_Recv(data, size, MPI_INT, prev, 1, MPI_COMM_WORLD, &st);
			if(k % pgrid[A->index_dimension_map[cdim]] != (my_address[A->index_dimension_map[cdim]] + 1) % pgrid[A->index_dimension_map[cdim]])
            {
                //cout << get_proc_addr_str(grid_dims, pgrid, rank) << "  sending to " << get_proc_addr_str(grid_dims, pgrid, next_processor(mapped_cdim)) << endl; 
				//cout << rank << "  sending to " << next << endl; 
				MPI_Send(data, size, MPI_INT, next, 1, MPI_COMM_WORLD);
            }
        }
    }
}

void Contraction::forward_double(Tensor* A, double* &data, int size, int k, int cdim, bool is_instigator)
{
    int mapped_cdim = A->index_dimension_map[cdim];
	int next = next_processor(mapped_cdim);
	int prev = prev_processor(mapped_cdim);
	if(rank != next)
    {
        if(is_instigator) // If this is the instigator
        {
			MPI_Send(data, size, MPI_DOUBLE, next, 1, MPI_COMM_WORLD);
        }
        else 
        {
            MPI_Status st;
			MPI_Recv(data, size, MPI_DOUBLE, prev, 1, MPI_COMM_WORLD, &st);
            if(k % pgrid[A->index_dimension_map[cdim]] != (my_address[A->index_dimension_map[cdim]] + 1) % pgrid[A->index_dimension_map[cdim]])
            {
				MPI_Send(data, size, MPI_DOUBLE, next, 1, MPI_COMM_WORLD);
            }
        }
    }
}

// Finds out blocks required right now for instigation from a serialized tensor
int Contraction::get_deserialized_blocks(Tensor* &T, double* &blocks, int* &addresses, int* &des_dims, int* &des_grid_dims, int &num_dims)
{

    // Find all external indices in A that are serialized in A but distributed in C
    string* ext_T = new string[T->dims];
    des_dims = new int[T->dims];
    num_dims = 0;
    for(int i=0; i<C->dims; i++)
    {
        // If the index is distributed in C, search for it in A
        if(C->index_dimension_map[i] != serial)
        {
            for(int j=0; j<T->dims; j++)
            {
                // If the index is found in A and is serial
                if(C->contr_dim_str[i] == T->contr_dim_str[j]  &&  T->index_dimension_map[j] == serial)
                {
                    des_dims[num_dims] = j;
                    ext_T[num_dims] = T->contr_dim_str[j];
                    num_dims++;
                }
            }
        }
    }

    int num_deserialized_blocks = 0;
    if(num_dims > 0)
    {
        num_deserialized_blocks = get_blks_from_serialized(num_dims, ext_T, des_grid_dims, T, blocks, addresses);
    }
    delete[] ext_T;
    return num_deserialized_blocks;
}

// Performs recursive SUMMA

void Contraction::rec_summa(Tensor* &A, Tensor* &B, double* &C_buffer, 
			    list<pair<int,int>> contr_list, 
			    pair<int,int> prev_cdim1, pair<int,int> prev_cdim2)
{

    if(contr_list.empty())
    {
	if(DEBUG_T && rank == RRANK) grid->print_proc_addr(RRANK);
	if(DEBUG_T && rank == RRANK) cout<<"Entering"<<endl<<fflush;
	
	int num_blocks_A = A->num_actual_tiles;
	int num_blocks_B = B->num_actual_tiles;
	
	double* blocks_A = A->tensor_tiles;
	double* blocks_B = B->tensor_tiles;
	
	int* block_addr_A = A->tile_address;
	int* block_addr_B = B->tile_address;

	
	transpose_and_dgemm_preserve(num_blocks_A, num_blocks_B, blocks_A, blocks_B, block_addr_A, block_addr_B, C_buffer);
	    return;
	    
	}
	// List should not be empty here
	assert(contr_list.empty() == false);


    // Get the contracting dimension pair to work on
    pair<int,int> p = contr_list.front();
    contr_list.pop_front();

    int cdim_A = p.first;
    int cdim_B = p.second;

    // Find the upper bound for contraction index k
    int k_bound = A->vgrid[cdim_A];
    /*cout<<"current contr index is "<<cdim_A
      <<" and previous contr indx is "<<prev_cdim2.first<<endl;
      cout<<"current SG map is  "<<A->SG_index_map[cdim_A]
      <<" and previous SG map is "<<A->SG_index_map[prev_cdim2.first]<<endl;*/
    
    if(prev_cdim1.first != -1  
       &&  A->SG_index_map[cdim_A] == A->SG_index_map_permanent[prev_cdim1.first])
    {
        //cout<<"sym 1"<<endl;
        k_bound = prev_cdim1.second+1;
    }
    if(prev_cdim2.first != -1  
       &&  A->SG_index_map[cdim_A] == A->SG_index_map_permanent[prev_cdim2.first])
    {
        //cout<<"sym 2"<<endl;
        k_bound = prev_cdim2.second+1;
    }

    // Contraction loop

    for(int k = 0; k < k_bound; k++)
    {
        // Start measuring communication time
        comm_time -= MPI_Wtime();

        //---------------------------------instigate the collection of symmetric blocks----------------------------//

        double *blocks_A, *blocks_B;
        int *block_addr_A, *block_addr_B;
        instigation_time -= MPI_Wtime();
        int num_collected_A = instigate_collection(A, cdim_A, k, blocks_A, block_addr_A);
        //if(rank==1)    printGetTiles(blocks_A, block_addr_A, A->block_size, num_collected_A, A->dims);

        timer4 -=MPI_Wtime();
        int num_collected_B = instigate_collection(B, cdim_B, k, blocks_B, block_addr_B);
        timer4 +=MPI_Wtime();
        instigation_time += MPI_Wtime();
        //cout << rank << " num_collected_A= " << num_collected_A<< " num_collected_B = " << num_collected_B << endl;

        //if(rank==4)    printGetTiles(blocks_B, block_addr_B, B->block_size, num_collected_B, B->dims);
        bool is_instigator_A = (k % pgrid[A->index_dimension_map[cdim_A]] == my_address[A->index_dimension_map[cdim_A]]);
        bool is_instigator_B = (k % pgrid[B->index_dimension_map[cdim_B]] == my_address[B->index_dimension_map[cdim_B]]);


        //------------------------------------------------------------------------------------------------------------//



        //--------------------------------------------- Communicate A forward----------------------------------------//

        // Forward number of blocks
        int* num_blocks_ptr_A = new int[1];
        num_blocks_ptr_A[0] = num_collected_A;

        block_size_forward_time -= MPI_Wtime();
        forward_int(A, num_blocks_ptr_A, 1, k, cdim_A, is_instigator_A); 
        block_size_forward_time += MPI_Wtime();

        int num_blocks_A = num_blocks_ptr_A[0];

        // Allocate memory
        if(num_collected_A == 0)
        {
            blocks_A = new double[A->block_size * num_blocks_A];
            block_addr_A = new int[A->dims * num_blocks_A];
        }

        address_forward_time -= MPI_Wtime();
        // Forward addresses
        forward_int(A, block_addr_A, num_blocks_A * A->dims, k, cdim_A, is_instigator_A);  
        address_forward_time += MPI_Wtime();

        data_forward_time -= MPI_Wtime();
        // Forward blocks
        forward_double(A, blocks_A, num_blocks_A * A->block_size, k, cdim_A, is_instigator_A);
        data_forward_time += MPI_Wtime();

        //------------------------------------------------------------------------------------------------------------//



        //--------------------------------------------- Communicate B forward----------------------------------------//

        // Forward number of blocks
        int* num_blocks_ptr_B = new int[1];
        num_blocks_ptr_B[0] = num_collected_B; 

        block_size_forward_time -= MPI_Wtime();
        forward_int(B, num_blocks_ptr_B, 1, k, cdim_B, is_instigator_B); 
        block_size_forward_time += MPI_Wtime();

        int num_blocks_B = num_blocks_ptr_B[0];

        // Allocate memory
        if(num_collected_B == 0)
        {
            blocks_B = new double[B->block_size * num_blocks_B];
            block_addr_B = new int[B->dims * num_blocks_B];
        }

        address_forward_time -= MPI_Wtime();
        // Forward addresses
        forward_int(B, block_addr_B, num_blocks_B * B->dims, k, cdim_B, is_instigator_B);  
        address_forward_time += MPI_Wtime();

        data_forward_time -= MPI_Wtime();
        // Forward blocks
        forward_double(B, blocks_B, num_blocks_B * B->block_size, k, cdim_B, is_instigator_B);
        data_forward_time += MPI_Wtime();

        // Stop measuring communication time
        comm_time += MPI_Wtime();


        //---------------------------------------Base Case Do the local dgemm ---------------------------------------//
	

        // Compute
        if(contr_list.empty())
        {
            transpose_and_dgemm(num_blocks_A, num_blocks_B, blocks_A, blocks_B, block_addr_A, block_addr_B, C_buffer);
        }

        //---------------------------------------------------------------------------------------------------------//

        //--------------------------------recursive Case Create sub tensors and call SUMMA--------------------------//
        else
        {
            // Generate sub-tensors from the accumulated data after contracting current contr_dimension
            Tensor* sub_A = A->generate_tensor(cdim_A, blocks_A, block_addr_A, num_blocks_A);
            Tensor* sub_B = B->generate_tensor(cdim_B, blocks_B, block_addr_B, num_blocks_B);

            // Create pairs of contracting dimensions with current contr_idx and pass them on
            // for contracting next index. They will be used if the next contracting dimensions
            // are symmetric with this one.
            pair<int,int> p1, p2;
            if((A->SG_index_map[cdim_A] == A->SG_index_map[prev_cdim1.first] 
                        || prev_cdim1.first == -1) &&  A->SG_index_map[cdim_A] ==0)
            {
                //if (rank ==0) cout<<"creating sym pair 1 with k "<<k<<endl;
                p1 = make_pair(cdim_A, k);
            }
            else
            {
                p1 = prev_cdim1;
            }

            if((A->SG_index_map[cdim_A] == A->SG_index_map[prev_cdim2.first]
                        || prev_cdim1.first == -1) &&  A->SG_index_map[cdim_A] ==1)
            {
                //if(rank == 0)cout<<"creating sym pair 2 with k "<<k<<endl;
                p2 = make_pair(cdim_A, k);
            }
            else
            {
                p2 = prev_cdim2;
            }

            // Recursuvely call rec_summa for next contracting dimension
            rec_summa(sub_A, sub_B, C_buffer, contr_list, p1, p2);

            // Delete the subtensors once their job is done
            sub_A->~Tensor();
            sub_B->~Tensor();
        }
    } 
}

/* Works exactly same as transpose_and_dgemm, but it also
 * preserves the blocks_A, blocks_B and the addresses on
 * return. Furthermore unlike transpose_and_dgemm, the addresses
 * are transposed before using*/
void Contraction::transpose_and_dgemm_preserve(int num_blocks_A, int num_blocks_B, double* &blocks_A, double* &blocks_B, int* &block_addr_A, int* &block_addr_B, double* &C_buffer)
{
	// Check correctness
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



    // Contract each block in sub-tensor A with each block in sub-tensor B

    // Transpose blocks of B here because the outer for-loop requires the tranposed blocks of B
    // multiple times
    double* tr_blocks_buf_B =new double[num_blocks_B * B->block_size]; 



    //-------------------------------------B-transpose---------------------------------------------//

    //#pragma omp parallel for private(source_B, dest_B, source_addr_B)
    //transposes each block of B one at a time
    tr_time -= MPI_Wtime();

    int* perm_address_B = new int[num_blocks_B * B->dims];
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


    map<int, map<int, int> >* big_matrix_map_B;
    int num_row_dim_B = B->dims - num_cntr_indices;
    if(DEBUG_T && rank == RRANK) cout<<"Printing B"<<endl;
    create_big_matrix(B, tr_blocks_buf_B, perm_address_B,
		      p_map_B, num_blocks_B ,n_b, n_k, 
		      num_row_dim_B, big_matrix_map_B, blocks_B);
    
    //delete[] tr_blocks_buf_B;

    tr_time += MPI_Wtime();

    double* tr_blocks_buf_A =new double[num_blocks_A * A->block_size]; 
    tr_time -= MPI_Wtime();

    int* perm_address_A = new int[num_blocks_A * A->dims];
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

    map<int, map<int, int> >* big_matrix_map_A;
    int num_row_dim_A = A->dims - num_cntr_indices;

/*    if(rank == RRANK)
    {
	cout<<"As actual address"<<endl;
	print_tile_addrs(A->dims, block_addr_A, num_blocks_A);
	cout<<endl<<"As permuted address"<<endl;
	print_tile_addrs(A->dims, perm_address_A, num_blocks_A);
	
	cout<<endl<<"As premutation map"<<endl;
	print_tile_addr(A->dims, p_map_A);

	cout<<endl<<"Printing A"<<endl;
	}*/
    create_big_matrix(A, tr_blocks_buf_A, perm_address_A,
		      p_map_A, num_blocks_A, n_a, n_k, 
		      num_row_dim_A, big_matrix_map_A, blocks_A);
  
   tr_time += MPI_Wtime();

    //if(rank==0) printGetTiles(blocks_A, block_addr_A, A->block_size, num_blocks_A, A->dims);
   
    C->untouch_all_address();

    //if(!rank) cout<<"product of blocks of A and B"<<num_blocks_A * num_blocks_B<<endl;
    //if(!rank) cout<<"Num threads = "<<omp_get_num_threads()<<endl<<fflush;
    //delete[] tr_blocks_buf_A;

    double* location_of_C = C->get_tensor_tiles();

    int num_row_blocks = big_matrix_map_A->size();
    int num_col_blocks = big_matrix_map_B->size();
    int num_cntr_blocks = num_blocks_A / num_row_blocks;
    // dgemm
    if(rank == RRANK && DEBUG_T)
    {
	cout<<" i : "<<num_row_blocks * n_a<<endl;
	cout<<" j : "<<num_col_blocks * n_b<<endl;
	cout<<" k : "<<num_cntr_blocks *n_k<<endl;

	cout<<"i x k : "<<num_row_blocks * n_a * num_cntr_blocks *n_k<<endl; 
	cout<<"j x k : "<<num_col_blocks * n_b * num_cntr_blocks *n_k<<endl;

	cout<<"A_buf : "<<num_blocks_A * A->block_size<<endl;
	cout<<"B_buf : "<<num_blocks_B * B->block_size<<endl;

	cout<<"i x j : "<<num_col_blocks * n_b * num_row_blocks *n_a<<endl;

	cout<<"C_buf : "<<C->num_actual_tiles * C->block_size<<endl;

	}
    comp_time -= MPI_Wtime();    
    kevin_dgemm(num_row_blocks*n_a,  n_b * num_col_blocks, n_k * num_cntr_blocks, blocks_A, blocks_B, C_buffer, 0, 0, 1.0);
    // kevin_dgemm(n_a * num_row_blocks, n_b * num_col_blocks, n_k * num_cntr_blocks, A_buff, B_buff, C_buffer, 0, 0, 1.0);
    comp_time += MPI_Wtime();

    revert_big_matrix(A, big_matrix_map_A, 
		      blocks_A, n_a, n_k, 
		      p_map_A, tr_blocks_buf_A);
    
    
      delete[] blocks_A;
    blocks_A = tr_blocks_buf_A;

    tr_blocks_buf_B = new double[num_blocks_B * B->block_size]; 
    revert_big_matrix(B, big_matrix_map_B, 
		      blocks_B, n_b, n_k, 
		      p_map_B, tr_blocks_buf_B);
    
    
    delete[] blocks_B;
    blocks_B = tr_blocks_buf_B;

    delete[] perm_address_A;
    delete[] perm_address_B;


}

// Perform transpose as required on input tensor blocks and call dgemm
void Contraction::transpose_and_dgemm(int num_blocks_A, int num_blocks_B, double* &blocks_A, double* &blocks_B, int* &block_addr_A, int* &block_addr_B, double* &C_buffer)
{
     

    // Contract each block in sub-tensor A with each block in sub-tensor B

    // Transpose blocks of B here because the outer for-loop requires the tranposed blocks of B
    // multiple times
    double* tr_blocks_buf_B =new double[num_blocks_B * B->block_size]; 

    int *perm_address_A = new int[A->dims*num_blocks_A]; 
    int *perm_address_B = new int[B->dims*num_blocks_B]; 


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


    map<int, map<int, int> >* big_matrix_map_B;
    int num_row_dim_B = B->dims - num_cntr_indices;
    if(rank == RRANK && DEBUG_T) {
	cout<<"Num blocks B received is "<<num_blocks_B<<endl;
	print_tile_addrs(B->dims, block_addr_B, num_blocks_B);
    }
    create_big_matrix(B, tr_blocks_buf_B, perm_address_B,
		      p_map_B, num_blocks_B ,n_b, n_k, 
		      num_row_dim_B, big_matrix_map_B, blocks_B);

    delete[] tr_blocks_buf_B;

    tr_time += MPI_Wtime();

    double* tr_blocks_buf_A =new double[num_blocks_A * A->block_size]; 
    tr_time -= MPI_Wtime();


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

    map<int, map<int, int> >* big_matrix_map_A;
    int num_row_dim_A = A->dims - num_cntr_indices;
    create_big_matrix(A, tr_blocks_buf_A, perm_address_A,
		      p_map_A, num_blocks_A, n_a, n_k, 
		      num_row_dim_A, big_matrix_map_A, blocks_A);
  
   tr_time += MPI_Wtime();

    //if(rank==0) printGetTiles(blocks_A, block_addr_A, A->block_size, num_blocks_A, A->dims);
   
    C->untouch_all_address();

    //if(!rank) cout<<"product of blocks of A and B"<<num_blocks_A * num_blocks_B<<endl;
    //if(!rank) cout<<"Num threads = "<<omp_get_num_threads()<<endl<<fflush;
    delete[] tr_blocks_buf_A;


    int num_row_blocks = big_matrix_map_A->size();
    int num_col_blocks = big_matrix_map_B->size();
    int num_cntr_blocks = num_blocks_A / num_row_blocks;
//    if(rank == 0) cout<<"Num col blocks B is "<<num_col_blocks<<endl;
    // dgemm
    comp_time -= MPI_Wtime();
    kevin_dgemm(num_row_blocks*n_a,  n_b * num_col_blocks, n_k * num_cntr_blocks, blocks_A, blocks_B, C_buffer, 0, 0, 1.0);
    //    kevin_dgemm(n_a*num_blocks_A, n_b*num_blocks_B, n_k, blocks_A, blocks_B, C_buffer, 0, 0, 1.0);
    comp_time += MPI_Wtime();
    
    delete[] blocks_A;
    delete[] blocks_B;


}

// Perform 2D matrix multiplication

void Contraction::kevin_dgemm(int n_a, int n_b, int n_k, double* &A_buf, double* &B_buf, double* &C_buf, int at, int bt, double alpha) 
{

//#pragma omp critical
    local_num_dgemm++;
    
    if(rank == RRANK && DEBUG_T) cout<<endl<<local_num_dgemm<<"th Dgemm with na : "<<n_a<<" n_b : "<<n_b<<" and n_k : "<<n_k<<"."<<endl;

    dgemm_time -= MPI_Wtime();
    local_contraction(n_a, n_b, n_k, A_buf, B_buf, C_buf, at, bt, alpha);
    dgemm_time += MPI_Wtime();


    /*#if HOPPER
      char N = 'N';
      char T = 'T';
      double one = 1.0;
      double zero = 0.0;
      dgemm_((at==0) ? &N : &T, (bt==0) ? &N : &T,
      &n, &n, &n, &one, A, &n, B, &n, &one, C, &n);
#endif*/
}

// Returns the rank of the previous processor in given dimension
int Contraction::prev_processor(int dim)
{
    int* proc_addr = new int[grid_dims];
    memcpy(proc_addr, my_address, grid_dims*sizeof(int));
    proc_addr[dim] = (proc_addr[dim] - 1 + pgrid[dim]) % pgrid[dim];
	int r = grid->get_proc_rank(proc_addr);
    delete[] proc_addr;
    return r;
}

// Returns the rank of the next processor in given dimension
int Contraction::next_processor(int dim)
{
    int* proc_addr = new int[grid_dims];
    memcpy(proc_addr, my_address, grid_dims*sizeof(int));
    proc_addr[dim] = (proc_addr[dim] + 1) % pgrid[dim];
    int r = grid->get_proc_rank(proc_addr);
	delete[] proc_addr;
    return r;
}


// Parses given contraction string and return the number of dimensions
int Contraction::parse_contr_str(string contr_str, string* &out)
{

    int dims = 0;
    out = new string[1 + count(contr_str.begin(), contr_str.end(), ',')];
    char *str = new char[contr_str.length() + 1];
    strcpy(str, contr_str.c_str());
    char *s = strtok(str, ", ");
    while(s != NULL)
    {
        out[dims] = string(s);
        dims++;
        s = strtok(NULL, ", ");
    }
    delete[] str;
    return dims;
}


// Check if redistribution is required for an input tensor
bool Contraction::check_redistr(Tensor* &T, Tensor* &C, int* &new_idmap, string* &t, string* &c)
{

    // Initialize new idmap to the default value (old idmap)
    new_idmap = new int[T->dims];
    memcpy(new_idmap, T->index_dimension_map, T->dims * sizeof(int)); 

    //bool redistr_flag = false;

    for(int i=0; i < C->dims; i++)
    {
        for(int k =0; k < T->dims; k++)
        {
            // Find the external dimension k in T with the analogous dimension i in C
            if(C->contr_dim_str[i].compare(T->contr_dim_str[k]) == 0)
            {
                // Their index maps should be same... 
                // If not, redistribute T so that both of their physical dimension is same
                if(C->index_dimension_map[i] != new_idmap[k]  &&  new_idmap[k] != serial)
                {
                    // redistribution of T is needed
                    int temp = new_idmap[k];
                    for(int l=0; l<T->dims; l++)
                    {
                        if(new_idmap[l] == C->index_dimension_map[i])
                        {
                            new_idmap[l] = temp;
                        }
                    }
                    new_idmap[k] = C->index_dimension_map[i];
                    //redistr_flag = true;

                    //if(rank==0) cout<< C->contr_dim_str[i] << "  " << T->contr_dim_str[k] << " i= " << i << " k= "<<k << " c_idmap= " <<C->index_dimension_map[i] << " t_idmap= " <<T->index_dimension_map[k] << "  new_idmap[i] = " << new_idmap[i] << "  new_idmap[k]= " << new_idmap[k] <<endl;
                    break;
                }
            }
        }
    }
    //if(rank==0){ cout << endl << " new_idmap = " ; print_tile_addr(A->dims, new_idmap); cout << endl;}
    return redistr_flag; 
}


// Check if the new idmaps are not aligning same contraction indices along one dimension,
// figure out new idmap by remapping contraction indices
void Contraction::realign_new_idmap(int* &idmap_A, int* &idmap_B, vector<pair<int,int>> &contr_list,
									bool &redistr_A, bool &redistr_B)
{
	// Check if same contracting indices are mapped along same dimension
	bool aligned = false;
	for(vector<pair<int,int>>::iterator it = contr_list.begin(); it != contr_list.end(); it++)
	{
		if(idmap_A[it->first] == idmap_B[it->second])
		{
			aligned = true;
			break;
		}
	}	

	if(aligned)
	{
		// Set idmap for contraction indices to -1
		for(int i=0; i < A->dims; i++)
		{
			if(A->cntr_map[i] > 0)
				idmap_A[i] = -1;
		}
		for(int i=0; i < B->dims; i++)
		{
			if(B->cntr_map[i] > 0)
				idmap_B[i] = -1;
		}

		//if(rank==0) {
		//	cout << "idmaps: " << endl;
		//	print_tile_addr(A->dims, idmap_A);
		//	print_tile_addr(B->dims, idmap_B);
		//	cout << endl;
		//}
		
		// Create a list of unmapped dimensions
		vector<int> unmapped_dims;
		for(int i=0; i < grid->grid_dims; i++)
		{
			int j=0;
			for(j=0; j< A->dims; j++)
			{
				if(idmap_A[j] == i  &&  A->cntr_map[j] == 0)
					break;
			}
			int k=0;
			for(k=0; k< B->dims; k++)
			{
				if(idmap_B[k] == i  &&  B->cntr_map[k] == 0)
					break;
			}

			if(j == A->dims && k == B->dims) // no external index is mapped to this dimension
			{
				//if(rank==0) cout << "Adding " << i << " to unmapped dims list" << endl;
				unmapped_dims.push_back(i);
			}
		}

		// Find the dimensions where A and B's external indices are mapped
		vector<int> A_ext_dims;
		vector<int> B_ext_dims;
		for(int i=0; i < A->dims; i++)
		{
			if(A->cntr_map[i] == 0)
				A_ext_dims.push_back(idmap_A[i]);
		}
		for(int i=0; i < B->dims; i++)
		{
			if(B->cntr_map[i] == 0)
				B_ext_dims.push_back(idmap_B[i]);
		}

		//if(rank==0){
		//	for(int i=0; i< contr_list.size(); i++){
		//		cout << " contr_list["<<i<<"]= " << contr_list[i].first << " " << contr_list[i].second << endl;
		//	}
		//}

		for(int i=0; i<unmapped_dims.size(); i++)
		{
			int dim = unmapped_dims[i];
			//if(rank==0) cout << i << " c[i].first= "<<contr_list[i].first << " set to " << dim << endl; 
			//if(rank==0) cout << i << " c[i+1].second= "<<contr_list[(i+1) % contr_list.size()].second << " set to " << dim << endl; 
			idmap_A[contr_list[i].first] = dim;
			idmap_B[contr_list[(i+1) % contr_list.size()].second] = dim;
		}

		for(int i=0; i < A->dims; i++)
		{
			// If any contraction index of A is still unmapped, find the dimension 
			// where B's external index is mapped and use it
			if(idmap_A[i] == -1)
			{
				//if(rank==0) cout << "idmap_A["<<i<<"]= "<<B_ext_dims[0];
				idmap_A[i] = B_ext_dims[0];
				B_ext_dims.erase(B_ext_dims.begin());
			}
		}
		for(int i=0; i < B->dims; i++)
		{
			// If any contraction index of B is still unmapped, find the dimension 
			// where A's external index is mapped and use it
			if(idmap_B[i] == -1)
			{
				//if(rank==0) cout << "idmap_B["<<i<<"]= "<<A_ext_dims[0];
				idmap_B[i] = A_ext_dims[0];
				A_ext_dims.erase(A_ext_dims.begin());
			}
		}
	}

	// Check if idmaps are changed
	redistr_A = false; 
	redistr_B = false; 

	for(int i=0; i < A->dims; i++)
	{
		if(A->index_dimension_map[i] != idmap_A[i])
		{
			redistr_A = true;
		}
	}
	for(int i=0; i < B->dims; i++)
	{
		if(B->index_dimension_map[i] != idmap_B[i])
		{
			redistr_B = true;
		}
	}
}

//gives the mapping of external indicies in the input to external
//indicies in the output. This mapping is used to find tile address
//of the output tensor
void Contraction::fill_input_to_output_map(string* &input, int input_dims, string* &output, int output_dims, int* &input_to_output_map, int ext_input)
{

    input_to_output_map = new int[input_dims];
    for(int da = 0; da<input_dims; da++)
    {
        string temp_a = input[da];
        input_to_output_map[da] = -1;
        for(int dc = 0; dc<output_dims; dc++)
        {
            string temp_c = output[dc];
            if(temp_a.compare(temp_c)==0)
            {
                input_to_output_map[da] = dc;

                //sets the index dc on C as external index from ext_input
                is_C_indx_ext_A_or_ext_B[dc] = ext_input;

            }
        }
    }
}


void Contraction::reduction(int* &reduction_dims, double* &reduction_buffer, int count, int allORreduce)
{
    int* color_code = new int[grid_dims];
    memcpy(color_code, my_address, grid_dims*sizeof(int));
    
    /*color code is used to generate color rank. All nodes along 
    * the same dimension will have the same color rank*/
    for(int i = 0; i< grid_dims; i++)
    {
	if(reduction_dims[i])
	{
	    color_code[i] = 0;
	}
    }

    /*Color is the color rank. Its used to create intracomms
     * within which reduction needs to occur*/
    MPI_Comm reduction_comm;
    int color = grid->get_proc_rank(color_code);

    // Split the communicator into recv groups
    MPI_Comm_split(MPI_COMM_WORLD, color, rank, &reduction_comm);

    /*Based on the input parameter do either an inplace all reduce or
     * a reduce at itracomms root*/
    if(allORreduce)
    {
	if (rank==0) cout<<"The reduction buffer size is : "<<count<<endl;
	MPI_Allreduce(MPI_IN_PLACE, reduction_buffer, count, MPI_DOUBLE, 
	       MPI_SUM, reduction_comm);
    }
    else{
	int local_rank;
	MPI_Comm_rank(reduction_comm, &local_rank);

	if(!local_rank)
	{
	    MPI_Reduce(MPI_IN_PLACE, reduction_buffer, count, MPI_DOUBLE, 
		       MPI_SUM, 0, reduction_comm);
	}else{
	    double useless;
	    MPI_Reduce(reduction_buffer, &useless, count, MPI_DOUBLE, 
		       MPI_SUM, 0, reduction_comm);

	}
    }

}

/*This function does a pipelined broadcast of data buffer and
 * address buffer along the dimensions specified by rotate_dim.*/
void Contraction::rotate(Tensor* &T_input, Tensor*& T_output, list<pair<int,int>> &contr_list,
			 int is_rotate_A, list<int> &rotate_dims, Tensor* &T_rotate, 
			 double* &data_buffer, int* &address_buffer, int &count)
{

    //finds the current rotation dimension and the grid length along 
    //those dimensions
    int rotate_dim = rotate_dims.front();
    rotate_dims.pop_front();
    int grid_bound = pgrid[rotate_dim];


    //sets the buffers for the data to be sent
    double* old_data_buf = data_buffer;
    int* old_addr_buf = address_buffer;
    int old_count = count;
    
    //these are pointers storing address of data that will be received
    double* new_data_buf;
    int* new_addr_buf;
    int new_count;

    //if(rank ==0) cout<<"Cout Grid Bound..."<<grid_bound<<"."<<endl;
    //rotates along current dimension
    for(int i = 0; i<grid_bound; i++)
    {
	/* A full set of rotation implies that all data pass
	through the current node. That means that if the data
	required by the node is the data held by itself then there
	is no need to receive the data. THis case corresponds to
	when the iterator is 0. For all other iterator, a forward and 
	receieve is done.*/
	if(i>0) 
	{
	    
	    rotate_forward_receive(T_rotate,rotate_dim,old_count,
				   old_data_buf,old_addr_buf,
				   new_count, 
				   new_data_buf, new_addr_buf);
	    delete[] old_data_buf;
	    delete[] old_addr_buf;
	    
	    
	    old_data_buf = new_data_buf;
	    old_addr_buf = new_addr_buf;
	    old_count = new_count;

	}
	else
       	{
	    new_data_buf = old_data_buf;
	    new_addr_buf = old_addr_buf;
	    new_count = old_count;
	}

  /*recursively rotates the remaining dimensions
   * inrotate_dims. The old buffers need to now hold the data that
   * is received after the rotations.*/
	if(!rotate_dims.empty())
	{
	    //if(rank ==0) cout<<"rotate not empty..."<<endl;
	    rotate(T_input, T_output, contr_list, 
		   is_rotate_A, rotate_dims, T_rotate, 
		   new_data_buf, new_addr_buf, new_count);
	    
	    old_data_buf = new_data_buf;
	    old_addr_buf = new_addr_buf;
	    old_count = new_count;

	}
	/*if there are no remaining rotation dimension, then that
	 * mean we have entered the innermost loop, if the
	 * recursive call was written as loop nests. Here we do
	 * the appropriate rsumma and reductions to perform the
	 * contraction*/
	else  
	{
	    //if(rank ==0) cout<<"rotate empty... i is"<<i<<"."<<endl;
	    Tensor *T_A, *T_B, *T_C;
	    
	    T_C = T_output;
	    if(is_rotate_A)
	    {
		if(rank ==0) cout<<"Rotate A"<<endl;
		T_A = T_rotate->generate_tensor_with_new_tiles(new_data_buf,new_addr_buf,
							       new_count);
		T_B = T_input;
	    }
	    else{
		if(rank ==0) cout<<"Rotate B with num tiles "<<new_count<<endl;
		T_B = T_rotate->generate_tensor_with_new_tiles(new_data_buf,new_addr_buf,
							       new_count);
		T_A = T_input;
	    }

	    int* permuted_address;
	    map<int,int>* address_to_location;
	    get_C_addresses(T_A, T_B, T_C, permuted_address, address_to_location);

	    //cmbines individial tiles of C into a 2D matrix called big_matrix_C
	    std::map<int, std::map<int, int> >* big_matrix_map_C;
	    double* big_matrix_C;

	    bm_time -= MPI_Wtime();
	    int num_row_dim_C = T_C->dims - (T_A->dims - num_cntr_indices);
	    create_big_matrix_nl(T_C, T_C->tensor_tiles, permuted_address,
			      p_map_C, address_to_location->size(), n_a, n_b, 
			      num_row_dim_C, big_matrix_map_C, big_matrix_C, address_to_location);
    	    bm_time += MPI_Wtime();


	    // Recursive SUMMA
	    pair<int,int> p1(-1,-1);
	    pair<int,int> p2(-1,-1);
	    //if(rank ==0) cout<<"Calling rsumma..."<<endl;
	    timer5 -= MPI_Wtime();
	    rec_summa(T_A, T_B, big_matrix_C, contr_list, p1, p2);
	    timer5 += MPI_Wtime();
	    
	    bm_time -= MPI_Wtime();
	    //changes the 2D representation to individial tiles
	    revert_big_matrix(C, big_matrix_map_C, 
			      big_matrix_C, n_a, n_b, 
			      p_map_C, T_C->tensor_tiles);
    	    bm_time += MPI_Wtime();

	}
    }

    /*The rotaion call must return the final data held by this
     * node. This is done below*/
    data_buffer = new_data_buf;
    address_buffer = new_addr_buf;
    count = new_count;

    /*Recursive call of rotate will keep poping the elements of
     * the list. But when the recursive calls return, we want the
     * list to be in the same state that it was before recursively
     * calling rotate*/
    rotate_dims.push_front(rotate_dim);
    
}

// Perform contraction
void Contraction::contract(string contr_str_A, string contr_str_B, string contr_str_C)
{
    int checkpoint =0;
    if(rank==RRANK && DEBUG_T) cout << "Checkpoint " <<(checkpoint++)<< endl; 
    total_time = 0;
    total_time -= MPI_Wtime();
    timer1 = 0.0;
    timer2 = 0.0;
    timer3 = 0.0;
    timer4 = 0.0;
    timer5 = 0.0;
    timer6 = 0.0;
    bm_time = 0.0;


    data_forward_time =0.0;
    block_size_forward_time = 0.0;

    comm_time = 0;
    comp_time = 0;
    tr_time = 0;

    //if(rank==RRANK && DEBUG_T) cout << "A[" << contr_str_A << "]  x  B[" << contr_str_B << "]  =  C[" << contr_str_C << "]" << endl;

    if(rank==RRANK && DEBUG_T) cout << "Checkpoint " <<(checkpoint++)<< endl; 

    //dgemm parameters initialize
    n_a = n_b = n_k = 1;

    
    // Parse contraction strings of the given three tensors
    dims_A = parse_contr_str(contr_str_A, A->contr_dim_str);
    dims_B = parse_contr_str(contr_str_B, B->contr_dim_str);
    dims_C = parse_contr_str(contr_str_C, C->contr_dim_str);

    if(rank==RRANK && DEBUG_T) cout << "Checkpoint " <<(checkpoint++)<< endl;     
    // Gives the mapping of external indices in the input to external indices in the output. 
    // This mapping is used to find tile address of the output tensor
    fill_input_to_output_map(A->contr_dim_str, dims_A, C->contr_dim_str, dims_C, A_to_C_map, EXT_A);
    fill_input_to_output_map(B->contr_dim_str, dims_B, C->contr_dim_str, dims_C, B_to_C_map, EXT_B);

    if(rank==RRANK && DEBUG_T) cout << "Checkpoint " <<(checkpoint++)<< endl; 
    // Identify contracting indices in A and B
    list<pair<int,int>> contr_list = list<pair<int,int>>();
    list<pair<int,int>> DDO_list = list<pair<int,int>>();
    list<pair<int,int>> DDA_list = list<pair<int,int>>();
    
    if(rank==RRANK && DEBUG_T) cout << "Checkpoint " <<(checkpoint++)<< endl; 

    
    //if(rank==0 && COST_EVAL)
    //{
    //	cost* eval = new cost(A,B,C, dims_A, dims_B, dims_C, grid, grid_dims);
    //	int* new_idmap_A = new int[dims_A];
    //	int* new_idmap_B = new int[dims_B];
    //	int* new_idmap_C = new int[dims_C];
    //	eval->best_Cost(new_idmap_A,new_idmap_B,new_idmap_C);
    //	
    //}


    int cntr_id = 1;
    for(int i=0; i<dims_A; i++)
    {
        for(int j=0; j<dims_B; j++)
        {
            if(A->contr_dim_str[i].compare(B->contr_dim_str[j]) == 0)
            {
                // i in A and j in B are contracting indices
                pair<int,int> p(i, j);
                contr_list.push_back(p);
                A->cntr_map[i] = cntr_id;
                B->cntr_map[j] = cntr_id;
		cntr_id++;
            }
        }
    }
    
    if(rank==RRANK && DEBUG_T) cout << "Checkpoint " <<(checkpoint++)<< endl; 
    int need_reduction = 0;
    num_cntr_indices = contr_list.size();

    //generates the permutation maps required for local contraction
    generate_permutation_map(A,B,C, contr_list);

    // Generate C_AB address map for correctness checking
    if(CCHECK) CRCT_generate_map(contr_list);

        redist_time = -MPI_Wtime();
    // Check if redistribution is required for input tensors
    int *new_idmap_A, *new_idmap_B;
    check_redistr(A, C, new_idmap_A, A->contr_dim_str, C->contr_dim_str);
    check_redistr(B, C, new_idmap_B, B->contr_dim_str, C->contr_dim_str);
	bool redistr_A, redistr_B;
	realign_new_idmap(new_idmap_A, new_idmap_B, contr_list, redistr_A, redistr_B);

    if(rank==0 && DEBUG_T) 
    {
        cout << " Before Redistribution: " ;
        print_tile_addr(A->dims, A->index_dimension_map);
        print_tile_addr(B->dims, B->index_dimension_map);
        print_tile_addr(C->dims, C->index_dimension_map);
        
		cout << endl << " New maps: ";
        print_tile_addr(A->dims, new_idmap_A);
        print_tile_addr(B->dims, new_idmap_B);
        cout << endl << fflush;
    }

    // Perform redistribution if required
    if(redistr_A)
    {
		A->redistribute(new_idmap_A);
    }

    if(redistr_B)
    {
		B->redistribute(new_idmap_B);
    }
    redist_time += MPI_Wtime();

    if(rank==0 && DEBUG_T) 
    {
        cout << " After Redistribution: " ;
        print_tile_addr(A->dims, A->index_dimension_map);
        print_tile_addr(B->dims, B->index_dimension_map);
        print_tile_addr(C->dims, C->index_dimension_map);
        cout << endl;
    }
    
    // Assert contraction validity
    assert_contr_validity(A, B);



    if(rank==RRANK && DEBUG_T) cout << "Checkpoint " <<(checkpoint++)<< endl; 
    //Generate RSUMMA(DDO-Distributed Distributed Orthogonal) and 
    //Reduction(DDA-Distributed Distributed Aligned)V contraction lists
    int* reduction_dims = new int[grid_dims];
    memset(reduction_dims, 0, grid_dims*sizeof(int));

    if(rank==RRANK && DEBUG_T) cout << "Checkpoint " <<(checkpoint++)<< endl; 
    for (std::list<pair<int,int>>::iterator cntr_it=contr_list.begin(); cntr_it != contr_list.end(); ++cntr_it)
    {
	int da = A->index_dimension_map[(*cntr_it).first];
	int db = B->index_dimension_map[(*cntr_it).second] ;
	if(da == db )
	{

	    DDA_list.push_back(*cntr_it);
	    if(da != grid_dims)
	    {
		if(rank ==0) cout<<"The reduction dimension is : "<<da<<endl;
		reduction_dims[da] = 1;
		need_reduction = 1;
	    }
	}
	else{
	    DDO_list.push_back(*cntr_it);
	}
    }
    
    if(rank==RRANK && DEBUG_T) cout << "Checkpoint " <<(checkpoint++)<< endl; 

    if(rank==RRANK && DEBUG_T) cout << "Checkpoint " <<(checkpoint++)<< endl; 

// Identify contracting indices in A and B
    list<int> rotate_dims;
    
    
/*Finds the DC dimensions for the external indices. Two indices
 * are DC if they are external indices and they both map to the
 * same physical dimension*/
     for(int i=0; i<dims_A; i++)
    {
        for(int j=0; j<dims_B; j++)
        {
            if(A->contr_dim_str[i].compare(B->contr_dim_str[j]) != 0 && 
	       !A->cntr_map[i] && !B->cntr_map[j])
            {

		if(A->index_dimension_map[i] == B->index_dimension_map[j] && A->index_dimension_map[i] != serial)
		{
		    rotate_dims.push_back(A->index_dimension_map[i]);
		    //cout<<"The rotation dimension is "<<A->index_dimension_map[i]<<"."<<endl;
		}
            }
        }
    }

     if(rank==RRANK && DEBUG_T) cout << "Checkpoint " <<(checkpoint++)<< endl; 
     
  // Transpose blocks of C as required for this contraction
    double tr_C_time = -MPI_Wtime();
    double* block_to_transpose = new double[C->block_size];
    double* transpose_block;

    //stores the permuted tile_address
    int* permuted_address = new int[C->num_actual_tiles * C->dims];

    if(rank==RRANK && DEBUG_T) cout << "Checkpoint " <<(checkpoint++)<< endl; 
    for(int i=0; i < C->num_actual_tiles; i++)
    {
        transpose_block =  C->tensor_tiles + i*C->block_size;
        memcpy(block_to_transpose, transpose_block, C->block_size * sizeof(double));
        transpose(block_to_transpose, transpose_block, p_map_C, C->dims, C->block_range);
	
	//generates the permuted tile address
	int* dest_address = permuted_address + i* C->dims;
	int* source_address = C->tile_address + i* C->dims;
	for(int d = 0; d< C->dims; d++)
	{
	    dest_address[p_map_C[d]] = source_address[d]; 
	}

    }
    tr_C_time += MPI_Wtime();
  
    if(DEBUG_T && rank == RRANK) cout<<"Rotate dims size is "<<rotate_dims.size()<<"."<<endl;
       if(rank == RRANK && DEBUG_T) 
       {
	   cout<<"Permutation Map of A"<<endl;
	   print_tile_addr(A->dims,p_map_A);
	   cout<<endl<<"Permutation Map of B"<<endl;
	   print_tile_addr(B->dims,p_map_B);
	   cout<<endl<<"Permutation Map of C"<<endl;
	   print_tile_addr(C->dims,p_map_C);

       }

       if(rank==RRANK && DEBUG_T) cout << "Checkpoint " <<(checkpoint++)<< endl; 
    //--------------------------------------------------------------------------------//
    if(rotate_dims.size())
     {
     
	 double* data_B = new double[B->num_actual_tiles * B->block_size];
	 int* address_B = new int[B->num_actual_tiles * B->dims];
	 memcpy(data_B, B->tensor_tiles, B->num_actual_tiles * B->block_size *sizeof(double));
	 memcpy(address_B, B->tile_address, B->num_actual_tiles * B->dims *sizeof(int));
	 if(rank==RRANK && DEBUG_T) cout << "Checkpoint " <<(checkpoint++)<< endl; 

	 timer5 = MPI_Wtime() -MPI_Wtime();
	 rotate(A, C, DDO_list, 
		0, rotate_dims, B, 
		data_B, address_B, B->num_actual_tiles);	 
	 
     }
     else
     {

	 //if there are no contraction indices and we are just computing outer product
	 if(contr_list.empty())
	 {
	     for(int i=0; i< A->dims; i++)
	     {
		 n_a *= A->block_range[i]; 
	     }

	     for(int i=0; i< B->dims; i++)
	     {
		 n_b *= B->block_range[i]; 
	     }
		
	     comp_time -= MPI_Wtime();
	     kevin_dgemm(n_a * A->num_actual_tiles, n_b * B->num_actual_tiles, n_k, 
			 A->tensor_tiles, B->tensor_tiles, C->tensor_tiles, 0, 0, 1.0);
	     comp_time += MPI_Wtime();
	
	     if(CCHECK) CRCT_check_blocks(A->tile_address, A->num_actual_tiles, B->tile_address, B->num_actual_tiles);
	     return;
	 }


	 if(rank==RRANK && DEBUG_T) cout << "Checkpoint " <<(checkpoint++)<< endl; 
	 //cmbines individial tiles of C into a 2D matrix called big_matrix_C
	 std::map<int, std::map<int, int> >* big_matrix_map_C;
	 double* big_matrix_C;
	 int num_row_dim_C = C->dims - (A->dims - num_cntr_indices);
	 
	 if(rank ==0)
	 {
	     /*cout<<"All tiles"<<endl;
	     C->print_all_tile_addr();
	     cout<<"Permuted tile addresses"<<endl;
	     int_tile_addrs(C->dims, permuted_address, C->num_actual_tiles);*/
	 } 


	 if(rank==RRANK && DEBUG_T) cout << "Checkpoint " <<(checkpoint++)<< endl; 
	 bm_time -= MPI_Wtime();
	 if(DEBUG_T && C->get_rank() == RRANK) cout<<endl<<"Printing C"<<endl<<fflush;
	 create_big_matrix(C, C->tensor_tiles, permuted_address,
			   p_map_C, C->num_actual_tiles ,n_a, n_b, 
			   num_row_dim_C, big_matrix_map_C, big_matrix_C);
	 
	 
	 delete[] block_to_transpose;
	 bm_time += MPI_Wtime();
	        	    
	 // Recursive SUMMA
	 pair<int,int> p1(-1,-1);
	 pair<int,int> p2(-1,-1);
	 
	 if(rank==RRANK && DEBUG_T) cout << "Checkpoint " <<(checkpoint++)<< endl; 
	 timer5 =- MPI_Wtime();
	 rec_summa(A, B, big_matrix_C, DDO_list, p1, p2);
	 timer5 += MPI_Wtime();

	 timer6 =- MPI_Wtime();
	 //Does reduction with all Reduce
	 if(need_reduction)
	 {
	     reduction(reduction_dims, big_matrix_C, C->num_actual_tiles * C->block_size, 1);
	 }
	 timer6 += MPI_Wtime();
     
	 //------------------------Reduction and Rotation Done ------------------------//
	
	 if(DEBUG_T && rank == RRANK) cout<<"Printing C"<<endl<<fflush;
	 bm_time -= MPI_Wtime();
	 //changes the 2D representation to individial tiles
	 revert_big_matrix(C, big_matrix_map_C, 
			   big_matrix_C, n_a, n_b, 
			   p_map_C, C->tensor_tiles);
	 bm_time += MPI_Wtime();
 
     }
    //---------------------------------------------------------------------------------//
    if(rank==RRANK && DEBUG_T) cout << "Checkpoint " <<(checkpoint++)<< endl; 

 // Transpose back blocks of C to their original form
    tr_C_time -= MPI_Wtime();

    block_to_transpose = new double[C->block_size];
    for(int i=0; i < C->num_actual_tiles; i++)
    {
        transpose_block =  C->tensor_tiles + i*C->block_size;
        memcpy(block_to_transpose, transpose_block, C->block_size * sizeof(double));
        transpose(block_to_transpose, transpose_block, inv_p_map_C, C->dims, C->block_range);
    }
    delete[] block_to_transpose;

    if(rank==RRANK && DEBUG_T) cout << "Checkpoint " <<(checkpoint++)<< endl;        
    tr_C_time += MPI_Wtime();
   
    redist_time -=MPI_Wtime();
    redist_time += MPI_Wtime();


    tr_time += tr_C_time;
    total_time += MPI_Wtime();

    if(CCHECK) CRCT_final_validation();

    if(DISPLAY_TIME)
    {
	MPI_Reduce(&timer1, &m_timer1, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&timer2, &m_timer2, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&timer3, &m_timer3, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&timer4, &m_timer4, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&timer5, &m_timer5, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&timer6, &m_timer6, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    
	MPI_Reduce(&total_time, &max_total_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&redist_time, &max_redist_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&comm_time, &max_comm_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&instigation_time, &max_instigation_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&forward_receive_time, &max_forward_receive_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&block_size_forward_time, &max_block_size_forward_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(
            &address_forward_time, &max_address_forward_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&data_forward_time, &max_data_forward_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

	MPI_Reduce(&comp_time, &max_comp_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&tr_time, &max_tr_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&bm_time, &max_bm_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&local_num_dgemm, &total_num_dgemm, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	print_time_flops();
    }
    // Reset the contraction map of input tensors
    //commenting for debugging uncomment later
    memset(A->cntr_map, 0, dims_A);
    memset(B->cntr_map, 0, dims_B);
}

void Contraction::display_times(){

	MPI_Reduce(&timer1, &m_timer1, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&timer2, &m_timer2, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&timer3, &m_timer3, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&timer4, &m_timer4, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&timer5, &m_timer5, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&timer6, &m_timer6, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    
	MPI_Reduce(&total_time, &max_total_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&redist_time, &max_redist_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&comm_time, &max_comm_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&instigation_time, &max_instigation_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&forward_receive_time, &max_forward_receive_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&block_size_forward_time, &max_block_size_forward_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(
            &address_forward_time, &max_address_forward_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&data_forward_time, &max_data_forward_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

	MPI_Reduce(&comp_time, &max_comp_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&tr_time, &max_tr_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&bm_time, &max_bm_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	MPI_Reduce(&local_num_dgemm, &total_num_dgemm, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	print_time_flops();

}
// Print information about the performance of this contraction
void Contraction::print_time_flops()
{
    if(rank==0) 
    {
        /*cout << "Redistribution time     = " <<        max_redist_time <<endl;
        cout << "Deserialize A,B,C       = " <<        m_timer6 <<endl;
        cout << "Instigation time        = " <<        max_instigation_time << endl;
        cout << "Instigation time B      = " <<        m_timer4             << endl;
        cout << "Instigation time A      = " <<        (max_instigation_time - m_timer4)  << endl;

        cout << "Instigate Send          = " <<        m_timer1 << endl;
        cout << "Instigate Receive       = " <<        (max_instigation_time - m_timer1) << endl;
        cout << "Merge Self Sends        = " <<        m_timer2<< endl;
        cout << "Combine Data            = " <<        m_timer3<< endl;

        cout<<endl;
        cout << "Interger Forward time   = " <<        max_block_size_forward_time << endl;
        cout << "Address Forward time    = " <<        max_address_forward_time << endl;
        cout << "Data Forward time       = " <<        max_data_forward_time << endl;
        cout<<endl;
        cout << "Forward Receive time    = " <<        max_forward_receive_time << endl;
        */
	cout << "Communication time      = " <<        max_comm_time << endl;
        cout << "Computation time        = " <<        max_comp_time << endl;
	cout << "Transpose Time          = " <<        max_tr_time <<endl;
        cout << "Big Transpose Time      = " <<        max_bm_time <<endl;
        cout << "RSUMMA time             = " <<        m_timer5 <<endl;
        cout << "Reduction time          = " <<        m_timer6 <<endl;
        cout << "Total time              = " <<        max_total_time<<endl;
        cout << "# dgemms                = " <<        total_num_dgemm << endl;

        /*
           cout << "Timers      = " <<      m_timer6<< endl;
           */



        

        double flops = 1.0;

        flops *= total_num_dgemm ;
        flops *= n_a * n_b * n_k * 2.0;
        flops /= max_comp_time;

        double gflops = flops / (1000.0 * 1000 * 1000);

        /*cout << "GFLOPS              = " << gflops << endl;
	  cout << "GFLOPS per rank     = " << gflops / num_procs << endl;*/

    }
}

