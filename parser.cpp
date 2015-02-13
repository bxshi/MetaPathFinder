#include "mpf.h"

vector<vector<uint32_t>> termList;

/**
* Function for tuple comparison
*/
bool compare_tuple(tuple<uint32_t,uint16_t>& i, tuple<uint32_t, uint16_t>& j) {
  return(get<0>(i) > get<0>(j));
}

double cosine_sim(vector<tuple<uint32_t, uint16_t>>& a, vector<tuple<uint32_t, uint16_t>>& b) {
  double asquare = 0;
  double bsquare = 0;
  double res = 0;

  size_t i = 0;
  size_t j = 0;

  while(i < a.size() && j < b.size()) {
    if (get<0>(a[i]) < get<0>(b[j])) {
      asquare += pow(get<1>(a[i]), 2);
      i++;
    } else if (get<0>(a[i]) > get<0>(b[j])) {
      bsquare += pow(get<1>(b[j]),2);
      j++;
    } else {
      res += get<1>(a[i]) * get<1>(b[j]);
      asquare += pow(get<1>(a[i]),2);
      bsquare += pow(get<1>(b[j]),2);
      i++;
      j++;
    }
  }

  if(i < a.size()) {
    for(;i < a.size(); i++) {
      asquare += pow(get<1>(a[i]),2);
    }
  }
  if(j < b.size()) {
    for(; j < b.size(); j++) {
      bsquare += pow(get<1>(b[j]),2);
    }
  }

  res = res / (sqrt(asquare) * sqrt(bsquare));
  assert(res <= 1.0);
  return res;
}

double cosine_sim(vector<uint32_t>& a, vector<uint32_t>& b) {
  double asquare = 0;
  double bsquare = 0;
  double res = 0;

  size_t i = 0;
  size_t j = 0;

  while(i < a.size() && j < b.size()) {
    if (a[i] < (b[j])) {
      asquare += pow(1, 2);
      i++;
    } else if ((a[i]) > (b[j])) {
      bsquare += pow(1,2);
      j++;
    } else {
      res += (1) * (1);
      asquare += pow(1,2);
      bsquare += pow(1,2);
      i++;
      j++;
    }
  }

  if(i < a.size()) {
    for(;i < a.size(); i++) {
      asquare += pow(1,2);
    }
  }
  if(j < b.size()) {
    for(; j < b.size(); j++) {
      bsquare += pow(1,2);
    }
  }

  res = res / (sqrt(asquare) * sqrt(bsquare));
  assert(res <= 1.0);
  return res;

}


int main(int argc, char** argv) {

  auto start_time = chrono::high_resolution_clock::now();

  ifstream term_input(argv[1]);

  // Read words

  termList.resize(MAX_DOC_ID);

  uint32_t max_term_id=0;
  string line;
  while (getline(term_input, line)) {
    size_t pos = 0;
    uint32_t key = 0;
    for(size_t i = 0; i < line.size(); i++) { // last element is always comma
      if (line[i] == ',') {
        uint32_t val = atoi(line.substr(pos, i).c_str());
        if(pos == 0) {
          key = val;
        } else {
          if (val > max_term_id) {
            max_term_id = val;
          }
          termList[key].push_back(val);
        }
        pos = i + 1;
      }
    }
    termList[key].shrink_to_fit();
  }
  term_input.close();

  auto duration = chrono::high_resolution_clock::now() - start_time;
  cout << "Terms are loaded, took " << chrono::duration_cast<chrono::microseconds>(duration).count() << "ms" << endl;
  cout << "#paper: " << termList.size() << endl;
  cout << "Largest term id: " << max_term_id << endl;

  ostringstream oss;
  ofstream output;
  output.open(argv[3], ofstream::trunc);
  uint64_t cnt = 0;

  // Read endpoint pairs
  ifstream edge_input(argv[2]);
  while (getline(edge_input, line)) { // mpath src dst
    istringstream is(line);
    uint16_t mpathType;
    uint32_t src;
    uint32_t target;

    is >> mpathType >> src >> target;

    // Calculate cosine similarity
    double score = cosine_sim(termList[src], termList[target]);
    oss << mpathType << "," << score << endl;
    cnt++;
    if (cnt % 5000 == 0) {
      output << oss.str();
      oss.str("");
      oss.clear();
    }
  }

  output.close();

  return 0;
}