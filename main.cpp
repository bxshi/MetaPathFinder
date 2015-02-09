#include <iostream>
#include <fstream>
#include <ostream>
#include <sstream>
#include <vector>
#include <chrono>
#include <cmath>
#include <thread>

#define MAX_ID 13860000
#define MAX_THREAD 40
#define PORTION 0.01

using namespace std;

enum NodeType {
  Author = 1, Paper = 2, Venue = 3, Term = 4, None = 0
};

struct arg{
  size_t partition;
  vector<NodeType> *nodeListPtr;
  vector<vector<uint32_t >> *edgeListPtr;
  size_t mpath_pos;
};

vector<vector<NodeType>> metaPath;
vector<NodeType> nodeList;
vector<vector<uint32_t>> edgeList;
thread threadList[MAX_THREAD];
struct arg argList[MAX_THREAD];

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
//      cout << "i = 0 " << frontier.size() << " " << frontier[0] << endl;
    }
    // Set frontier to visited
    for (size_t j = 0; j < frontier.size(); j++) {
      visited[frontier.at(j)] = true;
//      cout << "visited " << frontier.at(j) << " is " << visited[frontier.at(j)] << endl;
    }

    // Get new frontier
    vector<uint32_t> newFrontier;
    for (size_t j = 0; j < frontier.size(); j++) {
//        cout << j << " ";
//        cout << frontier.at(j) << " ";
//        cout << &edgeDict.at(frontier.at(j)) << endl;
      try{
        vector<uint32_t> &tmpList = edgeDict.at(frontier.at(j));
        for (size_t z = 0; z < tmpList.size(); z++) { // for each element in next node
          if (nodeDict.at(tmpList.at(z)) == nextType && // type meet
              !visited.at(tmpList.at(z))) { // never visited
            newFrontier.push_back(tmpList.at(z));
          }
        }
      } catch(const exception& e) {
        cout << e.what() << endl;
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
      vector<NodeType> tmpPath;
      for(size_t j = 0; j < path_len; j++) {
        tmpPath.push_back(candidates[val % candidates.size()]);
        val = val / candidates.size();
      }
      mPath.push_back(tmpPath);
      cnt++;
    }
  }
  return mPath;
}


void worker(struct arg &args) {

  ostringstream filename;
  filename << "./result_";
  for(size_t i = 0; i < metaPath[args.mpath_pos].size(); i++) {
    switch(metaPath[args.mpath_pos][i]) {
      case NodeType::None:
        filename << "None_";
        break;
      case NodeType::Author:
        filename << "Author_";
        break;
      case NodeType::Paper:
        filename << "Paper_";
        break;
      case NodeType::Venue:
        filename << "Venue_";
        break;
      case NodeType::Term:
        filename << "Term_";
    }
  }
  filename << args.partition;

  ofstream output;
  output.open(filename.str(), ofstream::trunc);
  ostringstream buf;
  size_t log_cnt= 0;
  for (size_t i = args.partition; i < nodeList.size(); i += MAX_THREAD) {
    try{
      if(nodeList[i] == metaPath[args.mpath_pos][0] && ((double)rand() / (double)RAND_MAX) <= PORTION) {
//        auto start_time = chrono::high_resolution_clock::now();
        vector<uint32_t> res = bfs_lookup(i, *args.nodeListPtr, *args.edgeListPtr, metaPath[args.mpath_pos]);
//        if(res.size() > 0){
//          auto duration = chrono::high_resolution_clock::now() - start_time;
//          cout << "calculation took " << chrono::duration_cast<chrono::microseconds>(duration).count() << endl;
//          cout << "size " << res.size() << endl;
//        }
        if(res.size() > 0) {
          buf << i << " ";
          for (int j = 0; j < res.size(); ++j) {
            buf << res[j];
            if(j < res.size() - 1){
              buf << " ";
            } else {
              buf << "\n";

            }
          }
          log_cnt++;
          if(log_cnt % 5000) {
            output << buf.str();
            buf = ostringstream();
          }
        }
      }
    } catch (exception& e) {

    }
  }

  output << buf.str();

  output.close();

}




int main() {

  srand(900207);

  nodeList.resize(MAX_ID);
  edgeList.resize(MAX_ID);

  auto start_time = chrono::high_resolution_clock::now();


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
//      cout << node1 << "->" << node2 << endl;
    }
  }

  auto duration = chrono::high_resolution_clock::now() - start_time;
  cout << "Node types and edges are loaded, took " << chrono::duration_cast<chrono::microseconds>(duration).count() << endl;
  cout << "edge nodes " << edgeList.size();

  vector<NodeType> cand;
  cand.push_back(NodeType::Paper);
  cand.push_back(NodeType::Author);
  cand.push_back(NodeType::Venue);
  cout << cand.size() << endl;
  metaPath = gen_metapath(6, cand);

  cout << "Generated " << metaPath.size() << " meta paths" << endl;

//  struct arg args;
//  thread t(worker, ref(args));
//  t.join();

  for (size_t j = 0; j < metaPath.size(); j++) {
    if(metaPath[j][0] == NodeType::Paper && metaPath[j][metaPath[j].size()-1] == NodeType::Paper){
      start_time = chrono::high_resolution_clock::now();
      for(size_t i = 0; i < MAX_THREAD; i++) {
        argList[i].partition = i;
        argList[i].mpath_pos = j;
        argList[i].nodeListPtr = &nodeList;
        argList[i].edgeListPtr = &edgeList;
        threadList[i] = thread(worker, ref(argList[i]));
      }

      for(size_t i = 0; i < MAX_THREAD; i++) {
        threadList[i].join();
      }

      duration = chrono::high_resolution_clock::now() - start_time;
      cout << "calculation took " << chrono::duration_cast<chrono::microseconds>(duration).count() << endl;
    }
  }

//  for (int j = metaPath.size() - 1; j >=0; --j) {
//    cout << "j=" << j <<endl;
//    for (int i = 0; i < nodeList.size(); ++i) {
//      try{
//        if(nodeList[i] == metaPath[j][0]) {
//          start_time = chrono::high_resolution_clock::now();
//          vector<uint32_t> res = bfs_lookup(i, nodeList, edgeList, metaPath[j]);
//          if(res.size() > 0){
//            duration = chrono::high_resolution_clock::now() - start_time;
//            cout << "calculation took " << chrono::duration_cast<chrono::microseconds>(duration).count() << endl;
//            cout << "size " << res.size() << endl;
//          }
//        }
//      } catch (exception& e) {
//
//      }
//    }
//  }

  cout << "Hello, World!" << endl;
  return 0;
}