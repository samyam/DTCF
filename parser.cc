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

using namespace std;

struct Array {
  string name;
  vector<string> dims;
  string t_str;
  int* idmap;
  string inds;
  int n_dims;
};

struct Contraction {
  Array C;
  Array A;
  Array B;
  int pgrid_dims;
};

map<string, int> RangeMap;
int nRange = 0;
map<string, string> IndexMap;
int nIndex = 0;
map<string, Array> ArrayMap;
int nArray = 0;
map<string, int> SymMap;
int nSym = 0;
vector<Contraction> Contractions;

bool skip(string& line) { // check if we skip this line
  if (line.size() <= 0) return true; // empty
  if (line[0] == '{' || line[0] == '}' || line[0] == '#') return true; // comments
  return false;
}

vector<string>
split(const string& in, char delim = ' ') { // tokenizer
  vector<string> out;
  stringstream ss(in);
  string token;
  while (true) {
    getline(ss, token, delim);
    if (ss.fail()) break;
    if (token.size() > 0) {
      token.erase(token.find_last_not_of(",;")+1);
      out.push_back(token);
    }
  }
  return out;
}

// range Oa, Ob = 10;
void parseRangeStmt(vector<string>& vs) {
  int n = vs.size();
  int x = atoi(vs[n - 1].c_str());
  for (int i = 1; i < n - 2; i++) {
    RangeMap[vs[i]] = x;
    nRange ++;
  }
}

// index h1, h2, h3 = Oa;
void parseIndexStmt(vector<string>& vs) {
  int n = vs.size();
  string x = vs[n - 1];
  for (int i = 1; i < n - 2; i++) {
    IndexMap[vs[i]] = x;
    nIndex ++;
  }
}

// arr_name(arr_dims : arr_symm) where arr_dims is [x0,x1,...][y0,y1,...] and arr_symm is a string
// example: v_oovv([O,O][V,V]:(0,1)(2,3));
Array parseArray(const string& str) {
  Array arr;
  int n = str.size();
  int p1 = str.find("(");
  if (p1 != string::npos) arr.name = str.substr(0, p1);
  int p2 = str.find(":");

  // Find number of tensor dimensions
  int n_dims = 0;
  string t;
  int p4 = str.find(")");
  if(p2 != string::npos)
    t = str.substr(p1+1,p2-p1); 
  else
    t = str.substr(p1+1,p4-p1); 
  n_dims += std::count(t.begin(), t.end(), 'O');
  n_dims += std::count(t.begin(), t.end(), 'V');
  
  // Create tensor string
  char* t_str = new char[n_dims+1]; // +1 for '\0'
  if(n_dims == 2)
    strcpy(t_str, "cc");
  if(n_dims == 4)
    strcpy(t_str, "cccc");
  if (p2 != string::npos) {
    string sym = str.substr(p2 + 1, n - p2 - 2);
    string pair1, pair2;
    int p3 = sym.find(")");
    pair1 = sym.substr(1, p3 - 1);  // first symmetry
    pair2 = sym.substr(p3 +1 , sym.size() - p3 - 1); // second symmetry
    // process first symmetry
    if(pair2.compare("") != 0) pair2 = pair2.substr(1,pair2.size()-2);
    int comma = pair1.find(",");
    int d11 = atoi(pair1.substr(0,1).c_str());
    int d12 = atoi(pair1.substr(comma+1,1).c_str());
    t_str[d11] = 'a';
    t_str[d12] = 'a';
    // process second symmetry
    int d21, d22;
    if(pair2.compare("") != 0) {
      comma = pair2.find(",");
      d21 = atoi(pair2.substr(0,1).c_str());
      d22 = atoi(pair2.substr(comma+1,1).c_str());
      t_str[d21] = 'b';
      t_str[d22] = 'b';
    }
  }
  arr.t_str = string(t_str);
  
  // Create idmap
  arr.idmap = new int[n_dims];
  int serial;
  if(n_dims == 2) {
    for(int i=0; i<n_dims; i++)
      arr.idmap[i] = serial;
  }
  else if(n_dims == 4) {
    for(int i=0; i<n_dims; i++)
      arr.idmap[i] = i;
  }

  if (p1 != string::npos || p2 != string::npos) {
    string tmp = str.substr(p1 + 1, p2 - p1 - 1);
    int nc = tmp.size();
    string dname("");
    for (int i = 0; i < nc; i++) {
      if (tmp[i] == '[') continue;
      if (tmp[i] == ']' || tmp[i] == ',') {
        arr.dims.push_back(dname);
        dname = "";
      }
      else {
        dname += tmp[i];
      }
    }
  }
  return arr;
}

void parseArrayStmt(vector<string>& vs) {
  for (int i = 1; i < vs.size(); i++) {
    ArrayMap[vs[i]] = parseArray(vs[i]);
  }
}


// parse array and index names in the expression
// example: v_oovv[h1,h2,p1,p2]
Array parseArray2(const string& str, int &n_dims) {
  Array arr;
  int n = str.size();
  int p1 = str.find("[");
  if (str[0] == '(') arr.name = str.substr(1, p1 - 1);
  else arr.name = str.substr(0, p1);
  int p2 = str.find("]");
  arr.inds = str.substr(p1 + 1, p2 - p1 - 1);
  n_dims = std::count(arr.inds.begin(), arr.inds.end(), ',') + 1;
  //cout << arr.name << ",\"" << arr.inds << "\"" << endl;
  return arr;
}

void parseExprStmt(vector<string>& vs) {
  int n = vs.size();
  if (vs[n-2] == "+") return; // skip addition expression
  if (vs.size() != 5) return; // skip non-binary expression
  //  cout << vs[0] << " " << vs[2] << " " << vs[4] << endl;
  Contraction exp;
  int dims_A, dims_B, dims_C;
  exp.C = parseArray2(vs[0], dims_C);
  exp.A = parseArray2(vs[2], dims_A);
  exp.B = parseArray2(vs[4], dims_B);
  exp.C.n_dims = dims_C; exp.A.n_dims = dims_A; exp.B.n_dims = dims_B;
  exp.pgrid_dims = max(dims_A, dims_B);
  Contractions.push_back(exp);
}

void parseInputEquations()
{
  ifstream fin("_uhf_t1.eq");
  string line;
  while (getline(fin,line)) {
    if (skip(line)) continue;
    vector<string> vs = split(line);
    if (vs[0] == "range") parseRangeStmt(vs);
    else if (vs[0] == "index") parseIndexStmt(vs);
    else if (vs[0] == "array") parseArrayStmt(vs);
    else {
      parseExprStmt(vs);
    }
  }
  fin.close();
}
 
void fprint_grid_update(int i, int g_dims, std::ofstream &fout)
{
    string tab = "    ";
    if(Contractions[i].pgrid_dims == g_dims) {
      if(Contractions[i].A.n_dims == 2)
        fout << tab << Contractions[i].A.name << "->update_pgrid(" << g_dims << ", pgrid_" << g_dims << ");" << endl;
      if(Contractions[i].B.n_dims == 2)
        fout << tab << Contractions[i].B.name << "->update_pgrid(" << g_dims << ", pgrid_" << g_dims << ");" << endl;
      if(Contractions[i].C.n_dims == 2)
        fout << tab << Contractions[i].C.name << "->update_pgrid(" << g_dims << ", pgrid_" << g_dims << ");" << endl << endl;
    }
}
 
void  writeCodeToFile() {
  ofstream fout("CCSD.cc");
  
  string tab  = "    ";
  string tab2 = "        ";

  // Start writing code to a file
  fout << "#include \"mpi.h\"" << endl;
  fout << "#include \"tensor.h\"" << endl; 
  fout << "#include \"contraction.h\"" << endl;
  fout << "#include \"redistribute.h\"" << endl;
  fout << "using namespace std;" << endl << endl;
  fout << "int main(int argc, char* argv[]) {" << endl;
  fout << tab << "MPI_Init(&argc, &argv);" << endl;

  // Declare all tensor sizes
  for (map<string, int>::const_iterator it = RangeMap.begin(); it != RangeMap.end(); it++) {
    fout << tab << "int " << it->first << " = " << "32"/*it->second*/ << ";" << endl;
  }
  fout << endl;

  // Create physical grids
  fout << tab << "int* pgrid_2 = new int[2];" << endl;
  fout << tab << "pgrid_2[0] = 4;" << endl;
  fout << tab << "pgrid_2[1] = 4;" << endl << endl;
  fout << tab << "int* pgrid_4 = new int[2];" << endl;
  fout << tab << "pgrid_4[0] = 2;" << endl;
  fout << tab << "pgrid_4[1] = 2;" << endl;
  fout << tab << "pgrid_4[2] = 2;" << endl;
  fout << tab << "pgrid_4[3] = 2;" << endl << endl;
  
  // Declare all tensors and initialize them
  for (map<string, Array>::const_iterator it = ArrayMap.begin(); it != ArrayMap.end(); it++) {
    //string name = it->first;
    Array arr = it->second;

    // Declare Tensor size
    fout << tab << "int* size_" << arr.name << " = new int[" << arr.dims.size() << "];" << endl;

    // Assign sizes to size variable
    for (int i = 0; i < arr.dims.size(); i++) {
      fout << tab << "size_" << arr.name << "[" << i << "] = " << arr.dims[i] << ";" << endl;
    }
 
    // Declare idmap
    fout << tab << "int* idmap_" << arr.name << " = new int[" << arr.dims.size() << "];" << endl;
    // Create idmap
    for (int i = 0; i < arr.dims.size(); i++) {
      int idmap = arr.dims.size() == 2 ? 2 : i;
      fout << tab << "idmap_" << arr.name << "[" << i << "] = " << idmap << ";" << endl;
    }

    // Declare vgrid
    fout << tab << "int* vgrid_" << arr.name << " = new int[" << arr.dims.size() << "];" << endl;
    // Create idmap
    for (int i = 0; i < arr.dims.size(); i++) {
      fout << tab << "vgrid_" << arr.name << "[" << i << "] = size_" << arr.name << "[" << i << "]/4;" << endl;
    }

    // Declare tensor and initialize it
    fout << tab << "Tensor* " << arr.name << " = new Tensor(\"" 
         << arr.t_str << "\""		// Tensor string
	 << ", idmap_" << arr.name  	// Idmap
	 << ", size_"  << arr.name      // tensor size
         << ", vgrid_" << arr.name  	// vgrid
	 << ", " << arr.dims.size()     // pgrid_size
	 << ", pgrid_" << arr.dims.size() // pgrid
	 << ");" << endl;

    // Initialize tensor
    fout << tab << arr.name << "->initialize();" << endl << endl;
  }

  //  Create contractions and execute them
  //  Contraction* C1 = new Contraction(A, B, C, grid_dims, pgrid);
  //  C1->contract("a1,k1", "b1,k1", "a1,b1");
  for (int i = 0; i < Contractions.size(); i++) {
    fprint_grid_update(i, 4, fout);
    fprint_grid_update(i, 2, fout);
     
    fout << tab << "Contraction* C" << i << " = new Contraction(" << 
      Contractions[i].A.name << ", " << Contractions[i].B.name << ", " << Contractions[i].C.name << 
      ", " << Contractions[i].pgrid_dims << ", pgrid_" << Contractions[i].pgrid_dims << ");" << endl;
    fout << tab << "C" << i << "->contract(" << 
      " \"" << Contractions[i].A.inds << "\"" <<
      ", \"" << Contractions[i].B.inds << "\"" << 
      ", \"" << Contractions[i].C.inds << "\");" << endl << endl;
  }

  fout << tab << "MPI_Finalize();" << endl;
  fout << "}" << endl;

  fout.close();

}

int main(int argc, char *argv[]) {

  parseInputEquations();

  writeCodeToFile();
}

