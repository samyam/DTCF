#ifndef _CONTRACTION_H_
#define _CONTRACTION_H_

#include <vector>
#include <stdlib.h>
#include <mpi.h>
#include <list>
#include <string>
#include <algorithm>
#include <map>
#include "tensor.h"
#include "helper.h"
#include "redistribute.h"
#include "localcontract.h"
#include "omp.h"
#include "grid.h"
#include "block_grid.h"
#include "transpose_helper.h"
#include "cost.h"
#include "grid_redib.h"

#define EXT_A 1
#define EXT_B 2
#define CONTR_AB 0
namespace RRR{
// Ideally extend this class and make different contractions from it
class Contraction 
{
private:
    Grid* grid;
    int rank, num_procs; 
    int *my_address;

       
    int num_cntr_indices;

    Tensor *A, *B, *C;

    int dims_A, dims_B, dims_C;

    //permutation map for A, B and C before performing local dgemm
    //C also needs an inverse permutation map after the dgemm completess
    int *p_map_A, *p_map_B, *p_map_C, *inv_p_map_C;

    //stores the block range of C after it has been transposed
    //this pock range is used for inverse transposing C back to 
    //its original layout
    int* inverse_block_range_C;

    //stores if a given index of C is external index of A or B
    int* is_C_indx_ext_A_or_ext_B;

    //parameters for dgemm
    int n_a, n_b, n_k;

    //dgemm flags for A, B and C
    int t_A, t_B;

    MPI_Request *send_req_addr, *send_req_data;
    MPI_Request *recv_req;

    // This mapping is used to find tile address of the output tensor
    int* A_to_C_map;
    int* B_to_C_map;

    int num_senders;
    int num_receivers;
    int* current_senders;
    int* current_receivers;

    int grid_dims;
    int *pgrid;
    int serial;

    int* self_adr_sizes;
    double** self_buffers;
    int** self_addresses;
    int* self_data_sizes;    
    int num_self_sends;

    // Performance metrics
    int local_num_dgemm, total_num_dgemm;
    double dgemm_time;
    double redist_time, comm_time, tr_time, comp_time, bm_time, total_time;
    double max_redist_time, max_comm_time, max_tr_time, max_comp_time, max_bm_time, max_total_time;

    double instigation_time, forward_receive_time;
    double block_size_forward_time, address_forward_time, data_forward_time;

    double timer1, timer2, timer3, timer4, timer5, timer6;
    double m_timer1, m_timer2, m_timer3, m_timer4, m_timer5, m_timer6;

    double max_block_size_forward_time, max_address_forward_time, max_data_forward_time;

    double max_instigation_time, max_forward_receive_time;

    map<int, list<pair<int*,int*>>> CRCT_map;
    //returns a permutation map that combines the affect of 
    //applying map1 and map2 consecutively
    void compose_p_maps(Tensor* &T, int* &map1, int* &map2, int* &composed_map);
	

    //generates permutation maps based on symmetry for bounced blocks
    void get_symm_permutation(Tensor* &T, int* &tile_address, int* &sym_permutation);

    //generates permutation maps for A, B and C for doing transpose before local dgemm can be performed
    void generate_permutation_map(Tensor* &A, Tensor* &B, Tensor* &C, std::vector<pair<int,int>> &contr_list);

    //gives the mapping of external indicies in the input to external
    //indicies in the output. This mapping is used to find tile address
    //of the output tensor. Also assigns the indicies of input and output
    //as external index or internal index. Asigns the indicies of C
    //as external index of A or external index of B
    void fill_input_to_output_map(string* &input, int input_dims, string* &output, int output_dims, int* &input_to_output_map, int ext_input);

    //stores the address of C corresponding to address of A and B in C_address
    void get_C_address(int* &A_address,int dims_a, int* &B_address, int dims_b, int* &C_address, int dims_c);

    /*Using the input tensors T_A and T_B, figures out the
     * permuted addresses of T_C that can be generated by
     * contracting T_A and T_B. These addresses will be a
     * subset of addresses held by tensor T_C. The location of
     * these addresses is given the map
     * addresses_to_location */ 
    void get_C_addresses(Tensor* &T_A, Tensor* &T_B, Tensor* &T_C, int* &C_addresses, map<int,int>* &address_to_location);


    // Extracts blocks from the serialized input tensor that are required
    // for computing blocks in C along a dimension that is serialized in the
    // input tensor but distributed in C
    int get_blks_from_serialized(int num_dim, string* &ext_dim, int* &ext_grid_dim, Tensor* &T, double* &blocks, int* &addresses);

    // Finds out blocks required right now for instigation from a serialized tensor
    int get_deserialized_blocks(Tensor* &T, double* &blocks, int* &addresses, int* &des_dims, int* &des_grid_dims, int &num_dims);

    // Deserializes a tensor
    int deserialize(Tensor* &T, double* &blocks, int* &addresses, int num_dim, int* &ext_T, int* &ext_grid_dim, std::list<int>* &sym_list);

    // Deserialize the 2D input tensor T if the other input tensor S is 4D and the output is 2D
    Tensor* deserialize_422(Tensor* &T, Tensor* &S);
	
    // Asserts validity of specified contraction
    void assert_contr_validity(Tensor* &A, Tensor* &B);

    // Send data to instigator
    void send_to_instigator( Tensor* &X, int contr_dim, int contr_idx, int &count_addr_sends, int &count_data_sends); 

    // Receive data at instigator
    int recv_at_instigator(Tensor* &X, int contr_dim, int contr_idx,  double* &blocks, int* &block_addrs, int send_addr_count, int send_data_count);

    // Test for completion of bounce send receives of this processor
    void test_comm_completion();

    //waits until message sizes of all the senders have been recieved
    void initial_wait(int num_senders, MPI_Request* &recv_req, int* senders);

    // Merges multiple block sets into one removing duplicates
    // Returns number of block copied
    int merge_self_send_blocks(Tensor* &T, double* &out_blocks, int* &out_addr);

    void forward_int(Tensor* A, int* &data, int size, int k, int cdim, bool is_instigator);
	
    void forward_double(Tensor* A, double* &data, int size, int k, int cdim, bool is_instigator);

    //forwards the data it holds and recieves the data from the precvious node
    void rotate_forward_receive(Tensor* &T, int forward_receive_dim, 
				int &source_num_blocks,
				double* &source_data, int* &source_addr, 
				int &receive_num_blocks,
				double* &receive_data, int* &receive_addr);
	    
	
    // Returns the rank of the previous processor in given dimension
    int prev_processor(int dim);

    // Returns the rank of the next processor in given dimension
    int next_processor(int dim);

    // Parses given contraction string and return the number of dimensions
    int parse_contr_str(std::string contr_str, std::string* &out);

    // Check if redistribution is required for an input tensor
    void check_redistr(Tensor* &T, Tensor* &C, int* &new_idmap, std::string* &t, std::string* &c);

    // Check if the new idmaps are not aligning same contraction indices along one dimension,
    // figure out new idmap by remapping contraction indices
    void realign_new_idmap(int* &idmap_A, int* &idmap_B, vector<pair<int,int>> &contr_list,
			   bool &redistr_A, bool &redistr_B);

    // Performs recursive SUMMA
    void rec_summa(Tensor* &A, Tensor* &B, double* &C_buffer, std::vector<std::pair<int,int>> contr_list,
		   std::pair<int,int> prev_cdim1, std::pair<int,int> prev_cdim2);

    void rec_summa1(Tensor* &A, Tensor* &B, Tensor* &C, vector<std::pair<int,int>> contr_list,
		    std::pair<int,int> prev_cdim1, std::pair<int,int> prev_cdim2);
    void rec_summa2(Tensor* &A, Tensor* &B, Tensor* &C, vector<std::pair<int,int>> contr_list,
		    std::pair<int,int> prev_cdim1, std::pair<int,int> prev_cdim2);

    void temp_rec_summa(Tensor* &A, Tensor* &B, Tensor* &C, vector<pair<int,int>> contr_list, 
			pair<int,int> prev_cdim1, pair<int,int> prev_cdim2);
 
    // Perform transpose as required on input tensor blocks and call dgemm
    void transpose_and_dgemm_preserve(int num_blocks_A, int num_blocks_B, double* &blocks_A, double* &blocks_B, int* &block_addr_A, int* &block_addr_B, double* &C_buffer);

    // Perform transpose as required on input tensor blocks and call dgemm
    void transpose_and_dgemm(int num_blocks_A, int num_blocks_B, double* &blocks_A, double* &blocks_B, int* &block_addr_A, int* &block_addr_B, double* &C_buffer);

    // Perform 2D matrix multiplication
    void kevin_dgemm(int n_a, int n_b, int n_k, double* &A, double* &B, double* &C, int at, int bt, double alpha);

    // Print information about the performance of this contraction
    void print_time_flops();

    void CRCT_generate_map(vector<pair<int,int>> contr_list);

    void CRCT_print_map();

    list<pair<int*,int*>> CRCT_enumerate_AB_addrs(int* &C_addr, int num_contr_idx, vector<pair<int,int>> contr_list);

    void CRCT_check_blocks(int* &addrs_A, int num_blocks_A,	int* &addrs_B, int num_blocks_B);

    void CRCT_final_validation();


    void CRCT_get_AB_addr(list<pair<int*,int*>> &AB_addr_list, int* &A_addr, int* &B_addr, 
			  vector<pair<int,int>> contr_list, int k, int num_contr_idx, int* &contr_dim_sizes);


public:

    //Doesnt initialize anything
    Contraction(Grid* &g);

    // Constructor
    Contraction(Tensor* &a, Tensor* &b, Tensor* &c, Grid* &g);
        
    // Destructor
    ~Contraction();

    //Changes the grid if a redistribution is required for contraction
    void change_grid(Grid* &g);

    // Processors send to instigator and instigator collects the data
    int instigate_collection(Tensor* &X, int contr_dim, int contr_idx, double* &blocks, int* &clocks_addrs);

    // Contraction procedure
    void contract(std::string contr_str_A, std::string contr_str_B, std::string contr_str_C);

    /*Reduces the reduction buffer along
	 * reduction_dims.allORreucex is used to choose wheather
	 * to do a reduce or an all reduce. THe reduction is done
		     * in place.*/
    void reduction(int* &reduction_dims, double* &reduction_buffer, int count, int allORreducex);  

    void rotate(Tensor* &T_input, Tensor*& T_output, vector<pair<int,int>> &contr_list, 
		int is_rotate_A, list<int> &rotate_dims, Tensor* &T_rotate, 
		double* &data_buffer, int* &address_buffer, int &count);

    void display_times();



};
}
#endif
