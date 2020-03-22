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
#include <iomanip>

struct seq_fre {
  unsigned long code;
  int seq, fre;
  friend bool operator<(const seq_fre &lhs, const seq_fre &rhs) {
    if(lhs.fre == rhs.fre) {
      return lhs.seq > lhs.seq;
    } else {
      return lhs.fre < rhs.fre;
    }
  };
  friend bool operator==(const seq_fre &lhs, const seq_fre &rhs) {
    return lhs.code == rhs.code;
  };
};

enum fmt_type : unsigned long long {RLE, BITMASK, ONE_MM, TWO_CMM, TWO_MM, DM, ORIG};
struct compress_code {
  fmt_type fmt;
  unsigned long long cc;
  std::size_t size;
  friend bool operator<(const compress_code& lhs, const compress_code & rhs) {
    if(lhs.size == rhs.size) {
      return lhs.fmt > rhs.fmt;
    }
    return lhs.size < rhs.size;
  };
  friend std::ostream& operator<<(std::ostream& os, const compress_code& cc) {
    switch (cc.fmt) {
    case fmt_type::RLE: {
      os << std::bitset<5>(cc.cc);
      break;
    }
    case fmt_type::BITMASK: {
      os << std::bitset<15>(cc.cc);
      break;
    }
    case fmt_type::ONE_MM: {
      os << std::bitset<11>(cc.cc);
      break;
    }
    case fmt_type::TWO_MM: {
      os << std::bitset<16>(cc.cc);
      break;
    }
    case fmt_type::TWO_CMM: {
      os << std::bitset<11>(cc.cc);
      break;
    }
    case fmt_type::DM: {
      os << std::bitset<6>(cc.cc);
      break;
    }
    case fmt_type::ORIG: {
      os << std::bitset<35>(cc.cc);
      break;
    }
    default:
      break;
    }
    return os;
  }
};

void read_input(std::string in_file,
                std::vector<std::string> *codes,
                std::vector<std::string> *dicts) {
  // ::cout << in_file << std::endl;
  std::ifstream file(in_file);
  std::string line;
  int seq = 0;
  std::vector<seq_fre> vs;
  std::vector<seq_fre>::iterator it;
  while(getline(file, line)) {
    //std::cout << "line " << ++seq << ": \t" << line << std::endl;
    assert(line.size() == 32);
    codes->push_back(line);
    unsigned long bu = std::bitset<32>(line).to_ulong();
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
    dicts->push_back(std::bitset<32>(it->code).to_string());
  }
}

std::string compress1(std::string *code, std::vector<std::string> *dicts) {
  std::vector<compress_code> ccs;
  for(std::size_t dict_idx=0; dict_idx != dicts->size(); ++dict_idx) {
    auto code_l = std::bitset<64>(*code).to_ulong();
    auto dict_l = std::bitset<64>(dicts->at(dict_idx)).to_ulong();
    auto diff = code_l ^ dict_l;
    std::string diff_s = std::bitset<32>(diff).to_string();
    std::vector<int> diff_ones;
    for(std::size_t i=0; i!= diff_s.size(); ++i) {
      if(diff_s[i] == '1')  diff_ones.push_back(i);
    }
    auto ds = diff_ones.size();
    if(ds > 4) { // 110 continues to next dict
      unsigned long long cc = (fmt_type::ORIG << 32) ^ code_l;
      ccs.push_back(compress_code{fmt_type::ORIG, cc, 35});
    } else if(ds >= 3) {
      auto distance = diff_ones[ds-1] - diff_ones[0];
      if(distance > 4) { // 110 continues to next dict
        unsigned long long cc = (fmt_type::ORIG << 32) ^ code_l;
        ccs.push_back(compress_code{fmt_type::ORIG, cc, 35});
      } else { // 001 , get the bitmask
        unsigned long bm = diff >> (32-4-diff_ones[0]);
        unsigned long cc = (fmt_type::BITMASK << (5+4+3)) ^ (diff_ones[0] << (4+3)) ^ (bm << 3) ^ dict_idx;
        ccs.push_back(compress_code{fmt_type::BITMASK, cc, 15});
      }
    } else if(ds == 2) {
      auto distance = diff_ones[1] - diff_ones[0];
      if (distance > 4) { // 100
        unsigned long cc = (fmt_type::TWO_MM << (5+5+3)) ^ (diff_ones[0] << (5+3)) ^ (diff_ones[1] << 3) ^ dict_idx;
        ccs.push_back(compress_code{fmt_type::TWO_MM, cc, 16});
      } else if(distance > 1) { // 001 get bitmask
        unsigned long bm = diff >> (32-4-diff_ones[0]);
        unsigned long cc = (fmt_type::BITMASK << (5+4+3)) ^ (diff_ones[0] << (4+3)) ^ (bm << 3) ^ dict_idx;
        ccs.push_back(compress_code{fmt_type::BITMASK, cc, 15});
      } else { // distance==1, 011
        unsigned long cc = (fmt_type::TWO_CMM << (5+3)) ^ (diff_ones[0] << 3) ^ dict_idx;
        ccs.push_back(compress_code{fmt_type::TWO_CMM, cc, 11});
      }
    } else if(ds == 1) { // 010
      unsigned long cc = (fmt_type::ONE_MM << (5+3)) ^ (diff_ones[0] << 3) ^ dict_idx;
      ccs.push_back(compress_code{fmt_type::ONE_MM, cc, 11});
    } else { // ds = 0, 101
      unsigned long cc = (fmt_type::DM << 3) ^ dict_idx;
      ccs.push_back(compress_code{fmt_type::DM, cc, 6});
    }
  } // end for each dict
  std::sort(ccs.begin(), ccs.end());
  std::stringstream ss;
  ss << ccs[0];
  return ss.str();
}

std::string compress(std::vector<std::string> *codes,
              std::vector<std::string> *dicts) {
  std::stringstream ss;
  std::string prev = "";
  int rep = -1;
  //compress_code prev_cc{fmt_type::RLE, 0, 5};
  std::cerr << "Code:" << std::endl;
  for(std::size_t ci=0; ci != codes->size(); ++ci) {
    // TODO: check RLE
    if(codes->at(ci) == prev) {
      //std::cout << prev << std::endl;
      if(rep ==3 ) {
        compress_code pc{fmt_type::RLE, (unsigned long)rep, 5};
        std::cerr << prev << " --> " << pc << std::endl;
        ss << pc;
        rep = -1;
      } else {
        rep += 1;
        std::cerr << prev << std::endl;
      }
      continue;
    } else {
      if(rep >= 0) {
        compress_code pc{fmt_type::RLE, (unsigned long)rep, 5};
        std::cerr << prev << " --> " << pc << std::endl;
        ss << pc;
        rep = -1;
      } //else {
        prev = codes->at(ci);
        auto cc = compress1(&prev, dicts);
        ss << cc;
        std::cerr << prev << " --> " << cc << std::endl;
        //}
    }
  }
  return ss.str();//
}

int main(int argc, char *argv[])
{
  if(argc != 2) {
    std::cerr << "Usage: \t" << argv[0] << " 1 : compression" << std::endl;
    std::cerr << "\t" << argv[0] << " 2 : decompression" << std::endl;
    return 1;
  }

  std::vector<std::string> codes;
  std::vector<std::string> dicts;

  read_input("original.txt", &codes, &dicts);

  std::string compressed = compress(&codes, &dicts);
  //std::cout << compressed << std::endl;
   for(std::size_t i=0; i < compressed.length(); i+=32){

     std::cout << std::setw(32) << std::setfill('1') << std::left
               << compressed.substr(i, 32) << std::endl;
   }
  std::cout << "xxxx" << std::endl;
  for (auto dict :dicts) {
    std::cout << dict << std::endl;
  }
  return 0;
}
