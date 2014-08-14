#include "contraction.h"
#include "grid.h"

using namespace std;


// Generate a mapping of a block of C to the list of pairs of block addresses of A and B
// that need to be multiplied to generate this block of C
void Contraction::CRCT_generate_map(vector<pair<int,int>> contr_list)
{
	Grid* C_grid = new Grid(C->dims, C->vgrid);
	int num_contr_idx = (A->dims + B->dims - C->dims) / 2;
	int num_A_ext_idx = A->dims - num_contr_idx;
	int num_B_ext_idx = B->dims - num_contr_idx;

	/*int j=0;
	pair<int,int>* contr_list_arr = new pair<int,int>[contr_list.size()];
	for(vector<pair<int,int>>::iterator it = contr_list.begin(); it != contr_list.end(); it++)
	{
		contr_list_arr[j++] = *it;
	}*/
	
	int* C_block_addr = new int[C->dims];
	for(int i=0; i < C->num_actual_tiles; i++)
	{
		memcpy(C_block_addr, C->tile_address + i * C->dims, C->dims * sizeof(int));
		int block_num = C_grid->get_proc_rank(C_block_addr);
		CRCT_map[block_num] = CRCT_enumerate_AB_addrs(C_block_addr, num_contr_idx, contr_list);
	}

	delete[] C_block_addr;
}


// Print the Correctness Check map
void Contraction::CRCT_print_map()
{
	Grid* C_grid = new Grid(C->dims, C->vgrid);
	int* C_block_addr = new int[C->dims];
	
	cout << "CRCT_MAP: " << endl;
	for(map<int, list<pair<int*,int*>>>::iterator it = CRCT_map.begin(); it != CRCT_map.end(); it++)
	{
		C_grid->get_proc_addr(it->first, C_block_addr);
		print_tile_addr(C->dims, C_block_addr);
		cout << endl;
		for(list<pair<int*,int*>>::iterator lit = it->second.begin(); lit != it->second.end(); lit++)
		{
			print_tile_addr(A->dims, lit->first);
			print_tile_addr(B->dims, lit->second);
			cout << endl;
		}
		cout << endl;
	}
	cout << fflush;
}


// For a block of C, enumerate all blocks of A and B that need to be multiplied to generate
// this block of C. Return a list of pairs of addresses of such blocks of A and B.
list<pair<int*,int*>> Contraction::CRCT_enumerate_AB_addrs(int* &C_addr, int num_contr_idx, vector<pair<int,int>> contr_list)
{
	// Find the number of blocks along contracting indices
	int* contr_dim_sizes = new int[num_contr_idx];
	for(int i=0; i<num_contr_idx; i++)
	{
		int ai = contr_list[i].first; 
		contr_dim_sizes[i] = A->vgrid[ai];
	}

	// Fill up external index values in dummy addresses of A and B
	// Only contracting index values will change
	int *A_addr = new int[A->dims];
	int *B_addr = new int[B->dims];
	for(int j=0; j<A->dims; j++) A_addr[j] = -1;
	for(int j=0; j<B->dims; j++) B_addr[j] = -1;

	for(int c=0; c < C->dims; c++)
	{
		int a = 0, b = 0;
		for(a=0; a < A->dims; a++)
		{
			if(C->contr_dim_str[c].compare(A->contr_dim_str[a]) == 0)
				break;
		}
		for(b=0; b < B->dims; b++)
		{
			if(C->contr_dim_str[c].compare(B->contr_dim_str[b]) == 0)
				break;
		}
		A_addr[a] = C_addr[c];
		B_addr[b] = C_addr[c];
	}

	// Get a list of pairs of A and B addresses and return it
	list<pair<int*,int*>> AB_addr_list = list<pair<int*,int*>>();
	CRCT_get_AB_addr(AB_addr_list, A_addr, B_addr, contr_list, 0, num_contr_idx, contr_dim_sizes);

	return AB_addr_list;
}

// Recursively enumerate addresses of A and B
void Contraction::CRCT_get_AB_addr(list<pair<int*,int*>> &AB_addr_list, int* &A_addr, int* &B_addr, 
						vector<pair<int,int>> contr_list, int k, int num_contr_idx, int* &contr_dim_sizes)
{
	if(contr_list.empty())
	{
		int* A_enum_addr = new int[A->dims]; 
		int* B_enum_addr = new int[B->dims]; 

		memcpy(A_enum_addr, A_addr, A->dims * sizeof(int));
		memcpy(B_enum_addr, B_addr, B->dims * sizeof(int));

		pair<int*,int*> AB_pair(A_enum_addr, B_enum_addr);
		if(A->satisfies_sym(A_enum_addr) == true && B->satisfies_sym(B_enum_addr) == true)
			AB_addr_list.push_back(AB_pair);
	}
	else
	{
		pair<int,int> p = contr_list[k];
		int n = contr_dim_sizes[k];
		int a = p.first;
		int b = p.second;

		for(int i=0; i<n; i++)
		{
			int* A_enum_addr = new int[A->dims]; 
			int* B_enum_addr = new int[B->dims]; 

			memcpy(A_enum_addr, A_addr, A->dims * sizeof(int));
			memcpy(B_enum_addr, B_addr, B->dims * sizeof(int));

			A_enum_addr[a] = i;
			B_enum_addr[b] = i;

			if(k+1 == num_contr_idx)
			{
				pair<int*,int*> AB_pair(A_enum_addr, B_enum_addr);
				if(A->satisfies_sym(A_enum_addr) == true && B->satisfies_sym(B_enum_addr) == true)
					AB_addr_list.push_back(AB_pair);
			}
			else
			{
				CRCT_get_AB_addr(AB_addr_list, A_enum_addr, B_enum_addr, contr_list, k+1, num_contr_idx, contr_dim_sizes);
			}
		}
	}
}


// Check correctness by removing the observed AB address pairs from the correctness check map
void Contraction::CRCT_check_blocks(int* &addrs_A, int num_blocks_A, 
		int* &addrs_B, int num_blocks_B)
{
	int* addr_A = new int[A->dims];
	int* addr_B = new int[B->dims];
	int* addr_C = new int[C->dims];

	Grid* C_grid = new Grid(C->dims, C->vgrid);

	for(int i=0; i < num_blocks_A; i++)
	{
		memcpy(addr_A, addrs_A + i * A->dims, A->dims * sizeof(int));
		for(int j=0; j < num_blocks_B; j++)
		{
			memcpy(addr_B, addrs_B + j * B->dims, B->dims * sizeof(int));

			// Generate C address that addr_A and addr_B will create
			for(int a=0; a < A->dims; a++)
			{
				if(A->cntr_map[a] == 0) // if this index of A is external (exists in C)
				{
					for(int c=0; c < C->dims; c++)
					{
						if(C->contr_dim_str[c].compare(A->contr_dim_str[a]) == 0)
						{
							addr_C[c] = addr_A[a];
						}
					}
				}
			}
			for(int b=0; b < B->dims; b++)
			{
				if(B->cntr_map[b] == 0) // if this index of B is external (exists in C)
				{
					for(int c=0; c < C->dims; c++)
					{
						if(C->contr_dim_str[c].compare(B->contr_dim_str[b]) == 0)
						{
							addr_C[c] = addr_B[b];
						}
					}
				}
			}

			int C_block_num = C_grid->get_proc_rank(addr_C);

			// Find this block of C in correctness check map
			list<pair<int*,int*>>* AB_list = &(CRCT_map[C_block_num]);
			// Find this pair of A,B addresses in the list associated with this block of C
			// and remove the pair
			list<pair<int*,int*>>::iterator erase_it;
			bool found = false;
			for(list<pair<int*,int*>>::iterator it = AB_list->begin(); it != AB_list->end(); it++)
			{
				if(A->compare_addresses(addr_A, it->first)==true &&
						B->compare_addresses(addr_B, it->second)==true )
				{
					erase_it = it;
					found = true;
				}
			}
			if(found)
			{
				AB_list->erase(erase_it);
			}
			else
			{
				cout << "Correctness check FAILED because pairs that should not be multiplied were multiplied" << endl;
			}
		}
	}

	delete[] addr_A;
	delete[] addr_B;
	delete[] addr_C;
}


// Check if all AB pairs are removed from the correctness check map and
// computation is complete
void Contraction::CRCT_final_validation()
{
	Grid* C_grid = new Grid(C->dims, C->vgrid);
	int* addr_C = new int[C->dims]; 

	bool success = true;
	for(map<int, list<pair<int*,int*>>>::iterator it = CRCT_map.begin(); it != CRCT_map.end(); it++)
	{
		C_grid->get_proc_addr(it->first, addr_C);
		if(!(it->second).empty())
		{
			success = false;
			cout << "Correctness check FAILED because some pairs were not multiplied" << endl;
		}
	}

	//CRCT_print_map();

	if(success)
	{
		cout << "Correctness check SUCCESS" << endl;
	}

}

