#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <cmath>

#define MAX_ID 13860000

using namespace std;



enum NodeType {
  Author = 1, Paper = 2, Venue = 3, Term = 4, None = 0
};

vector<uint32_t> bfs_lookup(uint32_t src, vector<NodeType> &nodeDict, vector<vector<uint32_t>> &edgeDict, vector<NodeType> &mPath) {

  vector<uint32_t> frontier;
  vector<bool> visited;
  visited.resize(MAX_ID, false);

  for (size_t i = 0; i < mPath.size() - 1; i++) {
    NodeType currentType = mPath.at(i);
    NodeType nextType = mPath.at(i + 1);


    if (i == 0) { // Set frontier to all nodes that connect to src without type filtering
      if (nodeDict.at(src) != currentType) {
        return frontier;
      }
      frontier.push_back(src);
    }
    // Set frontier to visited
    for (size_t j = 0; j < frontier.size(); j++) {
      visited[frontier.at(j)] = true;
    }

    // Get new frontier
    vector<uint32_t> newFrontier;
    for (size_t j = 0; j < frontier.size(); j++) {
      vector<uint32_t> &tmpList = edgeDict.at(frontier.at(j));
      for (size_t z = 0; z < tmpList.size(); z++) { // for each element in next node
        if (nodeDict.at(tmpList.at(z)) == nextType && // type meet
            !visited.at(tmpList.at(z))) { // never visited
          newFrontier.push_back(tmpList.at(z));
        }
      }
    }
    frontier = newFrontier;
  }

  return frontier;

}

vector<vector<NodeType>> gen_metapath(uint32_t length, vector<NodeType>& candidates) {
  vector<vector<NodeType>> mPath;
  uint cnt = 0;
  for(size_t path_len = 2; path_len <= length; path_len++) {
    for(size_t pos = 0; pos < pow(candidates.size(), path_len); pos++) {
      size_t val = pos;
      for(size_t j = 0; j < path_len; j++) {
        mPath[cnt].push_back(candidates[val % candidates.size()]);
        val = val / candidates.size();
      }
    }
  }
  return mPath;
}

int main() {

  auto start_time = chrono::high_resolution_clock::now();

  vector<NodeType> nodeList;
  nodeList.resize(MAX_ID);

  vector<vector<uint32_t>> edgeList;
  edgeList.reserve(MAX_ID);

  // Load author id
  {
    std::ifstream authors("/data/bshi/dblp/author_id.csv");
    string line;
    while (getline(authors, line)) {
      size_t pos = 0;
      for (size_t i = 1; i < line.size(); i++) {
        if (line[i] == '\"') {
          pos = i - 1;
          break;
        }
      }
      nodeList[atoi(line.substr(1, pos).c_str())] = NodeType::Author;
//      cout << 1 << " " << pos << " " << atoi(line.substr(1, pos).c_str()) << " " << nodeList[atoi(line.substr(1, pos).c_str())] << endl;
    }
  }

  // Load paper id
  {
    std::ifstream authors("/data/bshi/dblp/article_id.csv");
    string line;
    while (getline(authors, line)) {
      size_t pos = 0;
      for (size_t i = 1; i < line.size(); i++) {
        if (line[i] == '\"') {
          pos = i - 1;
          break;
        }
      }
      nodeList[atoi(line.substr(1, pos).c_str())] = NodeType::Paper;
//      cout << 1 << " " << pos << " " << atoi(line.substr(1, pos).c_str()) << " " << nodeList[atoi(line.substr(1, pos).c_str())] << endl;
    }
  }

  // Load venue id
  {
    std::ifstream authors("/data/bshi/dblp/venue_id.csv");
    string line;
    while (getline(authors, line)) {
      size_t pos = 0;
      for (size_t i = 1; i < line.size(); i++) {
        if (line[i] == '\"') {
          pos = i - 1;
          break;
        }
      }
      nodeList[atoi(line.substr(1, pos).c_str())] = NodeType::Venue;
//      cout << 1 << " " << pos << " " << atoi(line.substr(1, pos).c_str()) << " " << nodeList[atoi(line.substr(1, pos).c_str())] << endl;
    }
  }

  // Load edges
  {
    std::ifstream edges("/data/bshi/dblp/edges.csv");
    string line;
    while (getline(edges, line)) {
      size_t pos = 0;
      for (size_t i = 0; i < line.size(); i++) {
        if (line[i] == ',') {
          pos = i;
          break;
        }
      }

      uint32_t node1 = uint32_t(atoi(line.substr(0, pos - 1).c_str()));
      uint32_t node2 = uint32_t(atoi(line.substr(0, pos + 1).c_str()));
      edgeList[node1].push_back(node2);
      edgeList[node2].push_back(node1);
    }
  }

  auto duration = start_time - chrono::high_resolution_clock::now();
  cout << "Node types and edges are loaded, took " << chrono::duration_cast<chrono::microseconds>(duration).count() << endl;

  vector<NodeType> cand;
  cand.push_back(NodeType::Author);
  cand.push_back(NodeType::Paper);
  cand.push_back(NodeType::Venue);
  vector<vector<NodeType>> metaPath = gen_metapath(6, cand);

  cout << "Generated " << metaPath.size() << " meta paths" << endl;

  cout << "Hello, World!" << endl;
  return 0;
}