#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include <string.h>
#include <vector>
#include <sstream>
#include <algorithm>
#include <map>
#include<list>
#include <cstring>
#include <stdlib.h>

using std::ifstream;


const int MAX_CHARS_PER_LINE = 512;
const int MAX_TOKENS_PER_LINE = 20;
const char* const DELIMITER = " ";

using namespace std;

struct contract{
    string v_C;
    string v_A;
    string v_B;
    
    string str_C;
    string str_A;
    string str_B;
};

string filename;
vector<int> grid;
list<string> tensor_list;
map<string, vector<int> > size;
map<string, vector<int> > block_grid;
map<string, vector<int> > idmap;
map<string, string> symmetry;
vector<contract> contractions;


void  writeCodeToFile_CAST() {
    
    string full_filename = filename;
    full_filename.append(".cc");

    ofstream fout(full_filename.c_str());

    string tab  = "    ";
    string tab2 = "        ";

    // Start writing code to a file
    fout << "#include \"mpi.h\"" << endl;
    fout << "#include \"../grid.h\"" << endl;
    fout << "#include \"../tensor.h\"" << endl; 
    fout << "#include \"../contraction.h\"" << endl;
    fout << "#include \"../redistribute.h\"" << endl;
    fout << "using namespace std;" << endl << endl;
    fout << "int main(int argc, char* argv[])" << endl << "{" << endl;
    fout << tab << "MPI_Init(&argc, &argv);" << endl;
    fout << tab << "int rank,np;" << endl;
    fout << tab << "MPI_Comm_rank(MPI_COMM_WORLD,&rank);" << endl << endl;
    fout << tab << "MPI_Comm_rank(MPI_COMM_WORLD,&np);" << endl << endl;
    fout << tab << "int size = atoi(argv[1]);" << endl;
    fout << tab << "int block_grid = atoi(argv[2]);" <<endl;
    fout << tab <<  "int grid_dim = atoi(argv[3]);" <<endl;

//   fout << tab << "if(rank==0) cout<<\"Cntr Name : "<<full_filename<<"\"<<endl;" <<endl;
//   fout << tab << "if(rank==0) cout<<\"NP : \"<<np;" <<endl;
//   fout << tab << "if(rank==0) cout<<\"  Grid Dim : \"<<grid_dim<<endl;" <<endl;
//   fout << tab << "if(rank==0) cout<<\"Size : \"<<size;" <<endl;
//   fout << tab << "if(rank==0) cout<<\"  Block Grid : \"<<block_grid<<endl;" <<endl;


    // Create physical grids
    //fout << tab << "int* pgrid_2 = new int[2];" << endl;
    //fout << tab << "pgrid_2[0] = 4;" << endl;
    //fout << tab << "pgrid_2[1] = 4;" << endl << endl;
    fout << tab << "int* pgrid = new int[grid_dim];" << endl;
	
    fout << tab << "for(int i =0; i< grid_dim; i++)"<<endl;
    fout << tab <<  "{"<<endl;

	fout<< tab << "pgrid[i] = atoi(argv[4+i]);"<<endl;

	fout << tab << "}"<<endl;
	fout << tab <<"if(rank==0) cout<<\"Grid about to initialized\"<<endl;"<<endl<<endl;
    fout << tab << "Grid* grid = new Grid(grid_dim, pgrid);" << endl << endl;
	fout << tab <<"if(rank == 0) cout<<\"Grid initialized\"<<endl;"<<endl<<endl;
    // Declare all tensors and initialize them
    for (map<string, vector<int> >::const_iterator it = idmap.begin(); it != idmap.end(); it++) {
	    
	string name = it->first;
	int tensor_dim = (it->second).size();
	    
	vector<int> tensor_size = size[name];
	vector<int> tensor_idmap = idmap[name];
	vector<int> tensor_block_grid = block_grid[name];
	string tensor_string = symmetry[name];

	// Declare Tensor size
	fout << tab << "int* size_" << name << " = new int[" << tensor_dim << "];" << endl;
	// Declare Block Grid
	fout << tab << "int* block_grid_" << name << " = new int[" << tensor_dim << "];" << endl;
	// Declare Tensor Idmap						    
	fout << tab << "int* idmap_" << name << " = new int[" << tensor_dim << "];" << endl<<endl;
	

	if(tensor_size.size() == 1)
	{
	    for(int i =0; i< tensor_dim; i++)
	    {
		fout << tab << "size_" << name << "[" << i << "] = size;"<<endl;
	    }
	}
	else
	{
	    for(int i =0; i< tensor_dim; i++)
	    {
		fout << tab << "size_" << name << "[" << i << "] = " << tensor_size[i] <<";"<<endl;
	    }
	
	}
	fout<<endl;

	if(tensor_block_grid.size() <= 1)
	{
	    for(int i =0; i< tensor_dim; i++)
	    {
		fout << tab << "block_grid_" << name << "[" << i << "] = block_grid;"<<endl;
	    }
	
	}
	else
	{
	    for(int i =0; i< tensor_dim; i++)
	    {
		fout << tab << "block_grid_" << name << "[" << i << "] = " << tensor_block_grid[i] <<";"<<endl;
	    }

	}
	fout<<endl;

	for(int i =0; i< tensor_dim; i++)
	{
	    fout << tab << "idmap_" << name << "[" << i << "] = " << tensor_idmap[i] <<";"<<endl;
	}
	fout<<endl;
	
	// Declare tensor and initialize it
	fout << tab << "Tensor* " << name << " = new Tensor(\"" 
	     << tensor_string<<"\""	        // Tensor string
	     << ", idmap_" << name  	        // Idmap
	     << ", size_"  << name              // tensor size
	     << ", block_grid_" << name  	// vgrid
	     << ", grid);" << endl;		// Physical grid

	// Initialize tensor
	fout << tab << name << "->initialize();" << endl << endl;
	fout << tab <<"if(rank == 0) cout<<\"Tensor "<<name<<" initialized\"<<endl;"<<endl<<endl;
    }

    fout<<tab<<"double time =0.0;"<<endl;
    for (int i = 0; i < contractions.size(); i++) {

	//fout << tab << "if(rank==0) cout << endl << endl << \"Contraction " << i << ": \" << endl;" << endl;
	fout << tab << "if(rank==0) cout<<\""
	     << "C [" << contractions[i].str_C << " ] = "
	     << "A [" << contractions[i].str_A << " ] x "
	     << "B [" << contractions[i].str_B << " ]\"<<endl;"<<endl;
	
	fout << tab << "Contraction* C" << i << " = new Contraction(" << 
	    contractions[i].v_A << ", " << contractions[i].v_B << ", " << contractions[i].v_C << ", grid);" << endl;
	fout<<tab<<"time =-MPI_Wtime();"<<endl;
	fout << tab << "C" << i << "->contract(" << 
	    " \"" << contractions[i].str_A << "\"" <<
	    ", \"" << contractions[i].str_B << "\"" << 
	    ", \"" << contractions[i].str_C << "\");" << endl << endl;
	fout<<tab<<"time +=MPI_Wtime();"<<endl;
	fout<<tab<<"C" << i << "->display_times();"<<endl;
	fout << tab << "if(rank==0) cout << \"Total Time =\"<<time<<endl<<endl;"<<endl;
	
    }
	
    fout << tab << "MPI_Finalize();" << endl;
    fout << "}" << endl;
    	
    fout.close();
	
}

void reset()
{
    filename = "";
    grid.erase(grid.begin(),grid.end());
    tensor_list.erase(tensor_list.begin(),tensor_list.end());
    size.erase(size.begin(),size.end());
    block_grid.erase(block_grid.begin(),block_grid.end());
    idmap.erase(idmap.begin(),idmap.end());
    symmetry.erase(symmetry.begin(),symmetry.end());
    contractions.erase(contractions.begin(),contractions.end());
 
}

int main()
{


    // create a file-reading object
    ifstream fin;
    fin.open("data.txt"); // open a file
    if (!fin.good()) 
	return 1; // exit if file not found
  
    // read each line of the file
    while (!fin.eof())
    {
	// read an entire line into memory
	char buf[MAX_CHARS_PER_LINE];
	fin.getline(buf, MAX_CHARS_PER_LINE);
    
	// parse the line into blank-delimited tokens
	int n = 0; // a for-loop index
    
	// array to store memory addresses of the tokens in buf
	const char* token[MAX_TOKENS_PER_LINE] = {}; // initialize to 0
    
	// parse the line
	token[0] = strtok(buf, DELIMITER); // first token
	if (token[0]) // zero if line is blank
	{
	    for (n = 1; n < MAX_TOKENS_PER_LINE; n++)
	    {
		token[n] = strtok(0, DELIMITER); // subsequent tokens
		if (!token[n]) break; // no more tokens
	    }
	}

	if(n>0)
	{
	    if(strcmp (token[0],"NEWFILE:") == 0)
	    {
		writeCodeToFile_CAST();
		reset();
		string nn(token[1]);
		filename = nn;
		
	    } 
	    
	    if(strcmp (token[0],"Grid:") == 0)
	    {
	   
		for (int i = 1; i < n; i++) 
		    grid.push_back(atoi(token[i]));;

	    }
	
	    if(strcmp (token[0],"Size:") == 0)
	    {
		string name(token[1]);
		tensor_list.push_back(name);

		for (int i = 2; i < n; i++) {
		    size[name].push_back(atoi(token[i]));

		}
	    }

	   
	    if(strcmp (token[0],"Symmetry:") == 0)
	    {
		string name(token[1]);
		string tensor_string(token[2]);
	
		symmetry[name] = tensor_string;

		   
	    }
	    
	    if(strcmp (token[0],"Block_Grid:") == 0)
	    {
		string name(token[1]);
	   
		for (int i = 2; i < n; i++) {
		    block_grid[name].push_back(atoi(token[i]));

		}

	    }

	    if(strcmp (token[0],"Idmap:") == 0)
	    {
		string name(token[1]);
	   
		for (int i = 2; i < n; i++) {
		    idmap[name].push_back(atoi(token[i]));

		}

	    }

	    if(strcmp (token[0],"CntrString:") == 0)
	    {
		contract cntr;
		cntr.v_C = string(token[1]);
		cntr.v_A = string(token[6]);
		cntr.v_B = string(token[11]);

		cntr.str_C = string(token[3]);
		cntr.str_A = string(token[8]);
		cntr.str_B = string(token[13]);
		contractions.push_back(cntr);   	
	    }
	   
	}
    }

    writeCodeToFile_CAST();
}

