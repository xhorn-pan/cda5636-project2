/* On my honor, I have neither given nor received unauthorized aid on this
 * assignment
 * Name: Xinghua Pan
 * UFID: 95160902
 */

#include <algorithm>
#include <bitset>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

struct seq_fre
{
  unsigned long code;
  int seq, fre;
  friend bool operator<(const seq_fre& lhs, const seq_fre& rhs)
  {
    if (lhs.fre == rhs.fre) {
      return lhs.seq > lhs.seq;
    } else {
      return lhs.fre < rhs.fre;
    }
  };
  friend bool operator==(const seq_fre& lhs, const seq_fre& rhs)
  {
    return lhs.code == rhs.code;
  };
};

enum fmt_type : unsigned long long
{
  RLE,
  BITMASK,
  ONE_MM,
  TWO_CMM,
  TWO_MM,
  DM,
  ORIG,
  FIN
};
struct compress_code
{
  fmt_type fmt;
  unsigned long long cc;
  std::size_t size;
  friend bool operator<(const compress_code& lhs, const compress_code& rhs)
  {
    if (lhs.size == rhs.size) {
      return lhs.fmt > rhs.fmt;
    }
    return lhs.size < rhs.size;
  };
  friend std::ostream& operator<<(std::ostream& os, const compress_code& cc)
  {
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

void
read_compressed(std::string in_file,
                std::string* compressed,
                std::vector<std::string>* dicts)
{
  std::ifstream file(in_file);
  std::string line;
  bool rd = false;
  while (!file.eof()) {
    getline(file, line);
    if (line[line.length() - 1] == '\r') { // check crlf
      line = line.substr(0, line.length() - 1);
    }
    if (line == "xxxx") {
      rd = true;
      continue;
    }
    if (rd) {
      dicts->push_back(line);
    } else {
      compressed->append(line);
    }
  }
};

void
read_uncompressed(std::string in_file,
                  std::vector<std::string>* codes,
                  std::vector<std::string>* dicts)
{

  std::ifstream file(in_file);
  std::string line;
  int seq = 0;
  std::vector<seq_fre> vs;
  std::vector<seq_fre>::iterator it;
  while (!file.eof()) {
    getline(file, line);
    if (line[line.length() - 1] == '\r') { // check crlf
      line = line.substr(0, line.length() - 1);
    }
    codes->push_back(line);
    unsigned long bu = std::bitset<32>(line).to_ulong();
    seq_fre sf_{ bu, seq, 1 };
    it = find(vs.begin(), vs.end(), sf_);
    if (it != vs.end()) {
      it->fre += 1;
    } else {
      vs.push_back(sf_);
    }
    seq += 1;
  }

  std::sort(vs.begin(), vs.end());
  std::reverse(vs.begin(), vs.end());

  for (auto it = vs.begin(); it != vs.begin() + 8; ++it) {
    dicts->push_back(std::bitset<32>(it->code).to_string());
  }
}

void
decompress(std::string* code, std::vector<std::string>* dicts)
{

  std::istringstream iss(*code);
  std::string prev;
  bool fin = false;
  // std::cerr << "decoding" << std::endl;
  while (!fin) {
    char fmt_t[4];
    iss.get(fmt_t, 4);
    fmt_type ft = static_cast<fmt_type>(std::bitset<3>(fmt_t).to_ullong());

    switch (ft) {
      case fmt_type::RLE: {
        char rep[3];
        iss.get(rep, 3);
        auto rep_i = std::bitset<2>(rep).to_ullong();
        for (std::size_t i = 0; i <= rep_i; ++i) {
          std::cout << prev << std::endl;
        }
        break;
      }
      case fmt_type::BITMASK: {
        char sloc[6], bitmask[5], di[4];
        iss.get(sloc, 6);
        iss.get(bitmask, 5);
        iss.get(di, 4);
        auto dii = std::bitset<5>(di).to_ullong();
        auto d = dicts->at(dii);
        auto ulld = std::bitset<64>(d).to_ullong();
        auto loc_i = std::bitset<5>(sloc).to_ullong();
        auto bm = std::bitset<5>(bitmask).to_ullong();
        unsigned long long diff = bm << (28 - loc_i);

        auto dc = ulld ^ diff;
        prev = std::bitset<32>(dc).to_string();
        std::cout << prev << std::endl;
        break;
      }
      case fmt_type::ONE_MM: {
        char mloc[6], di[4];
        iss.get(mloc, 6);
        iss.get(di, 4);
        auto loc_i = std::bitset<5>(mloc).to_ullong();
        auto dii = std::bitset<5>(di).to_ullong();
        unsigned long long diff = 1 << (31 - loc_i);
        auto d = dicts->at(dii);
        auto ulld = std::bitset<64>(d).to_ullong();
        auto dc = ulld ^ diff;
        prev = std::bitset<32>(dc).to_string();
        std::cout << prev << std::endl;

        break;
      }
      case fmt_type::TWO_MM: {
        char mloc1[6], mloc2[6], di[4];
        iss.get(mloc1, 6);
        iss.get(mloc2, 6);
        iss.get(di, 4);
        auto loc_i1 = std::bitset<5>(mloc1).to_ullong();
        auto loc_i2 = std::bitset<5>(mloc2).to_ullong();
        auto dii = std::bitset<5>(di).to_ullong();
        auto d = dicts->at(dii);
        auto ulld = std::bitset<64>(d).to_ullong();

        unsigned long long diff1 = 1 << (31 - loc_i1);
        unsigned long long diff2 = 1 << (31 - loc_i2);

        auto dc = ulld ^ diff1 ^ diff2;
        prev = std::bitset<32>(dc).to_string();
        std::cout << prev << std::endl;

        break;
      }
      case fmt_type::TWO_CMM: {
        char mloc[6], di[4];
        iss.get(mloc, 6);
        iss.get(di, 4);
        auto loc_i = std::bitset<5>(mloc).to_ullong();
        auto dii = std::bitset<5>(di).to_ullong();
        unsigned long long diff = 3 << (30 - loc_i);
        auto d = dicts->at(dii);
        auto ulld = std::bitset<64>(d).to_ullong();
        auto dc = ulld ^ diff;
        prev = std::bitset<32>(dc).to_string();
        std::cout << prev << std::endl;
        break;
      }
      case fmt_type::DM: {
        char di[4];
        iss.get(di, 4);
        auto dii = std::bitset<5>(di).to_ullong();
        prev = dicts->at(dii);
        std::cout << prev << std::endl;
        break;
      }
      case fmt_type::ORIG: {
        char orig_c[33];
        iss.get(orig_c, 33);
        prev = std::string(orig_c);
        std::cout << prev << std::endl;
        break;
      }
      case fmt_type::FIN: {
        fin = true;
        break;
      }
      default:
        fin = true;
        break;
    }
  }
}

std::string
compress1(std::string* code, std::vector<std::string>* dicts)
{
  std::vector<compress_code> ccs;
  for (std::size_t dict_idx = 0; dict_idx != dicts->size(); ++dict_idx) {
    auto code_l = std::bitset<64>(*code).to_ulong();
    auto dict_l = std::bitset<64>(dicts->at(dict_idx)).to_ulong();
    auto diff = code_l ^ dict_l;
    std::string diff_s = std::bitset<32>(diff).to_string();
    std::vector<int> diff_ones;
    for (std::size_t i = 0; i != diff_s.size(); ++i) {
      if (diff_s[i] == '1')
        diff_ones.push_back(i);
    }
    auto ds = diff_ones.size();
    if (ds > 4) { // 110 continues to next dict
      unsigned long long cc = (fmt_type::ORIG << 32) ^ code_l;
      ccs.push_back(compress_code{ fmt_type::ORIG, cc, 35 });
    } else if (ds >= 3) {
      auto distance = diff_ones[ds - 1] - diff_ones[0];
      if (distance > 4) { // 110 continues to next dict
        unsigned long long cc = (fmt_type::ORIG << 32) ^ code_l;
        ccs.push_back(compress_code{ fmt_type::ORIG, cc, 35 });
      } else { // 001 , get the bitmask
        unsigned long bm = diff >> (32 - 4 - diff_ones[0]);
        unsigned long cc = (fmt_type::BITMASK << (5 + 4 + 3)) ^
                           (diff_ones[0] << (4 + 3)) ^ (bm << 3) ^ dict_idx;
        ccs.push_back(compress_code{ fmt_type::BITMASK, cc, 15 });
      }
    } else if (ds == 2) {
      auto distance = diff_ones[1] - diff_ones[0];
      if (distance > 4) { // 100
        unsigned long cc = (fmt_type::TWO_MM << (5 + 5 + 3)) ^
                           (diff_ones[0] << (5 + 3)) ^ (diff_ones[1] << 3) ^
                           dict_idx;
        ccs.push_back(compress_code{ fmt_type::TWO_MM, cc, 16 });
      } else if (distance > 1) { // 001 get bitmask
        unsigned long bm = diff >> (32 - 4 - diff_ones[0]);
        unsigned long cc = (fmt_type::BITMASK << (5 + 4 + 3)) ^
                           (diff_ones[0] << (4 + 3)) ^ (bm << 3) ^ dict_idx;
        ccs.push_back(compress_code{ fmt_type::BITMASK, cc, 15 });
      } else { // distance==1, 011
        unsigned long cc =
          (fmt_type::TWO_CMM << (5 + 3)) ^ (diff_ones[0] << 3) ^ dict_idx;
        ccs.push_back(compress_code{ fmt_type::TWO_CMM, cc, 11 });
      }
    } else if (ds == 1) { // 010
      unsigned long cc =
        (fmt_type::ONE_MM << (5 + 3)) ^ (diff_ones[0] << 3) ^ dict_idx;
      ccs.push_back(compress_code{ fmt_type::ONE_MM, cc, 11 });
    } else { // ds = 0, 101
      unsigned long cc = (fmt_type::DM << 3) ^ dict_idx;
      ccs.push_back(compress_code{ fmt_type::DM, cc, 6 });
    }
  } // end for each dict
  std::sort(ccs.begin(), ccs.end());
  std::stringstream ss;
  ss << ccs[0];
  return ss.str();
}

std::string
compress(std::vector<std::string>* codes, std::vector<std::string>* dicts)
{
  std::stringstream ss;
  std::string prev = "";
  int rep = -1;
  // std::cerr << "Code:" << std::endl;
  for (std::size_t ci = 0; ci != codes->size(); ++ci) {
    if (codes->at(ci) == prev) {
      if (rep == 3) {
        compress_code pc{ fmt_type::RLE, (unsigned long)rep, 5 };
        ss << pc;
        rep = -1;
      } else {
        rep += 1;
      }
      continue;
    } else {
      if (rep >= 0) {
        compress_code pc{ fmt_type::RLE, (unsigned long)rep, 5 };
        ss << pc;
        rep = -1;
      } // else {
      prev = codes->at(ci);
      auto cc = compress1(&prev, dicts);
      ss << cc;
    }
  }
  return ss.str(); //
}

int
main(int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: \t" << argv[0] << " 1 : compression" << std::endl;
    std::cerr << "\t" << argv[0] << " 2 : decompression" << std::endl;
    return 1;
  }

  if (atoi(argv[1]) == 1) {
    std::ofstream out("cout.txt");
    std::cout.rdbuf(out.rdbuf());
    std::vector<std::string> codes;
    std::vector<std::string> dicts;

    read_uncompressed("original.txt", &codes, &dicts);
    std::string compressed = compress(&codes, &dicts);
    for (std::size_t i = 0; i < compressed.length(); i += 32) {
      std::cout << std::setw(32) << std::setfill('1') << std::left
                << compressed.substr(i, 32) << std::endl;
    }
    std::cout << "xxxx" << std::endl;
    for (auto dict : dicts) {
      std::cout << dict << std::endl;
    }
  } else if (atoi(argv[1]) == 2) {
    std::ofstream out("dout.txt");
    std::cout.rdbuf(out.rdbuf());
    std::string compressed;
    std::vector<std::string> dicts;
    read_compressed("compressed.txt", &compressed, &dicts);
    decompress(&compressed, &dicts);
  } else {
    std::cerr << "Usage: \t" << argv[0] << " 1 : compression" << std::endl;
    std::cerr << "\t" << argv[0] << " 2 : decompression" << std::endl;
    return 1;
  }
  return 0;
}
