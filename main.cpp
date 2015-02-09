#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>

#define MAX_ID 13860000

using namespace std;



enum NodeType {Author=1, Paper=2, Venue=3, Term=4, None=0};

int main() {

  auto start_time = chrono::high_resolution_clock::now();

  vector<NodeType> nodeList;
  nodeList.resize(MAX_ID);

  // Load author id
  {
    std::ifstream authors("/data/bshi/dblp/article_id.csv");
    string line;
    while (getline(authors, line)) {
      size_t pos = 0;
      for (int i = 1; i < line.size(); i++) {
        if (line[pos] == '\"') {
          pos = i - 1;
          break;
        }
      }

      nodeList[atoi(line.substr(1, pos).c_str())] = NodeType::Author;
    }
  }

  // Load paper id
  {
    std::ifstream authors("/data/bshi/dblp/article_id.csv");
    string line;
    while (getline(authors, line)) {
      size_t pos = 0;
      for (int i = 1; i < line.size(); i++) {
        if (line[pos] == '\"') {
          pos = i - 1;
          break;
        }
      }

      nodeList[atoi(line.substr(1, pos).c_str())] = NodeType::Paper;
    }
  }

  // Load venue id
  {
    std::ifstream authors("/data/bshi/dblp/venue_id.csv");
    string line;
    while (getline(authors, line)) {
      size_t pos = 0;
      for (int i = 1; i < line.size(); i++) {
        if (line[pos] == '\"') {
          pos = i - 1;
          break;
        }
      }

      nodeList[atoi(line.substr(1, pos).c_str())] = NodeType::Venue;
    }
  }

  auto duration = start_time - chrono::high_resolution_clock::now();
  cout << "Node types are loaded, took " << chrono::duration_cast<chrono::microseconds>(duration).count() << endl;


  cout << "Hello, World!" << endl;
  return 0;
}