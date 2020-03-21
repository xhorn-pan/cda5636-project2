/* On my honor, I have neither given nor received unauthorized aid on this
 * assignment */

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cassert>
#include <bitset>

#define MAX_INT 1 << 32

unsigned int binary_to_unsigned(std::string binary, int b_size) {
  unsigned int d = 0;
  for (int i=b_size-1; i>=0; --i) {
    char c = binary[i];
    if(c == '1') {
      d = (d << 1) + 1;
    } else {
      d = d << 1;
    }
  }
  return d;
}

int get_loc(unsigned int diff) {
  unsigned int d = MAX_INT / diff;
  return d;
}

std::string unsigned_to_binary(unsigned int ui, int b_size) {
  std::stringstream ss;
  unsigned int d;
  for(int c=0; c < b_size; ++c) {
    d = ui >> c;
    if( d & 1) {
      ss.put('1');
    } else {
      ss.put('0');
    }
  }
  return ss.str();
}

struct seq_fre {
  unsigned int ui;
  int seq, fre;
  friend bool operator<(const seq_fre &lhs, const seq_fre &rhs) {
    if(lhs.fre == rhs.fre) {
      return lhs.seq > lhs.seq;
    } else {
      return lhs.fre < rhs.fre;
    }
  };
  friend bool operator==(const seq_fre &lhs, const seq_fre &rhs) {
    return lhs.ui == rhs.ui;
  };
};


void read_input(std::string in_file,
                std::vector<std::string> &code,
                std::vector<seq_fre> &dicts) {
  std::cout << in_file << std::endl;
  std::ifstream file(in_file);
  std::string line;
  int seq = 0;
  std::vector<seq_fre> vs;
  std::vector<seq_fre>::iterator it;
  while(getline(file, line)) {
    //std::cout << "line " << ++seq << ": \t" << line << std::endl;
    assert(line.size() == 32);
    code.push_back(line);
    unsigned int bu = binary_to_unsigned(line, 32);
    seq_fre sf_{bu, seq, 1};
    it = find(vs.begin(), vs.end(), sf_);
    if(it != vs.end()) {
      it->fre += 1;
    } else {
      vs.push_back(sf_);
    }
    seq += 1;
  }

  std::sort(vs.begin(), vs.end());
  std::reverse(vs.begin(), vs.end());

  for(auto it = vs.begin(); it != vs.begin()+8; ++it) {
    //std::cout << it->seq << ", " << it->fre << ", " << it->ui << std::endl;
    dicts.push_back(*it);
  }
}

// remove right end 0 of binary form
inline unsigned int rm_zero(unsigned int orig) {
  while(!(orig % 2)) {
    orig >>= 1;
  }
  return orig;
}

/* void compress(vector code, vector dict)
for each line in code
  for each d in dict
     int diff = line xor d;
     if diff == 0 => 101 with d.idx to binary
     else if diff = 1; 010 + get location of diff
     else if diff = 3; 011 + get location of diff
     else if diff < 16 001 + location
     else
     new_diff = diff >> 1
     if new_diff = 1 => 100 + locations
     else -> 119 + line
 */
int main(int argc, char *argv[])
{
  if(argc != 2) {
    std::cerr << "Usage: \t" << argv[0] << " 1 : compression" << std::endl;
    std::cerr << "\t" << argv[0] << " 2 : decompression" << std::endl;
    return 1;
  }

  std::vector<std::string> codes;
  std::vector<seq_fre> dicts;

  read_input("original.txt", &codes, &dicts);
  //std::cout << unsigned_to_binary(83, 32) << std::endl;
  return 0;
}
