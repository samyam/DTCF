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
   	
}


int main(int argc, char* argv[])
{

    
    ofstream fout("temp_makefile");

    // create a file-reading object
    ifstream fin;
    fin.open("Makefile"); // open a file
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
	    if(strcmp (token[0],"MAIN") == 0)
	    {
		fout<<"MAIN = "<<argv[1]<<endl;
	    }
	    else if(strcmp (token[0],"EXE") == 0)
	    {
		fout<<"MAIN = "<<argv[2]<<endl;
	    }
	    else
	    {
		fout<<buf<<endl;
	    }
	}
    }
    fout.close();

}

