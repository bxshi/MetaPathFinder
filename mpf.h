#ifndef _MPF_H_
#define _MPF_H_

#include <vector>
#include <iostream>
#include <tuple>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <ostream>
#include <sstream>
#include <chrono>
#include <cmath>
#include <thread>
#include <cstring>

#define MAX_ID 5908600
#define MAX_THREAD 40
#define NODETYPE_BASE 5
#define MAX_DOC_ID 4373900

using namespace std;


enum NodeType {
  Author = 1, Paper = 2, Venue = 3, Term = 4, None = 0
};


inline uint16_t encode(vector<NodeType>& mPath) {
  uint16_t res = 0;
  for(size_t i = 0; i < mPath.size(); i++) {
    res += uint16_t(mPath[i]) * pow(NODETYPE_BASE, i);
  }
  return res;
}

inline vector<NodeType> decode(uint16_t val) {
  vector<NodeType> res;
  int tmp = val;
  while(tmp > 0) {
    res.push_back(NodeType(tmp % NODETYPE_BASE));
    tmp /= NODETYPE_BASE;
  }
  return res;
}

inline string path_to_string(bool brief, vector<NodeType> nodevec) {
  ostringstream ostr;
  for(size_t i = 0; i < nodevec.size(); i++) {
    switch(nodevec[i]) {
      case NodeType::None:
        ostr << (brief ? "?" : "None_");
        break;
      case NodeType::Author:
        ostr << (brief ? "A" : "Author_");
        break;
      case NodeType::Paper:
        ostr << (brief ? "P" : "Paper_");
        break;
      case NodeType::Venue:
        ostr << (brief ? "V" : "Venue_");
        break;
      case NodeType::Term:
        ostr << (brief ? "T" : "Term_");
    }
  }
  return ostr.str();
}

#endif