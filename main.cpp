#include <iostream>
#include <fstream>
#include <ostream>
#include <sstream>
#include <vector>
#include <chrono>
#include <cmath>
#include <thread>
#include <cstring>

#define MAX_ID 5908600
#define MAX_THREAD 40
#define PORTION 0.0002
#define NODETYPE_BASE 5

using namespace std;

uint8_t max_depth=6;
uint8_t min_depth=2;

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
//struct arg argList[MAX_THREAD];

//vector<vector<vector<uint64_t>>> global_result;
//bool **global_visited;

vector<uint32_t> bfs_lookup(uint32_t src, vector<NodeType> &nodeDict,
    vector<vector<uint32_t>> &edgeDict, vector<NodeType> &mPath, bool *visited) {
  vector<uint32_t> frontier;

  memset(visited, false, sizeof(bool) * MAX_ID);

  for (size_t i = 0; i < mPath.size() - 1; i++) {
    NodeType currentType = mPath.at(i);
    NodeType nextType = mPath.at(i + 1);


    if (i == 0) { // Set frontier to all nodes that connect to src without type filtering
      if (nodeDict.at(src) != currentType) {
        return frontier;
      }
      frontier.push_back(src);
      visited[src] = true;
//      cout << "i = 0 " << frontier.size() << " fronter " << frontier[0] << endl;
    }

    if(frontier.size() == 0) { // early terminate if there is no nodes to visit
      return frontier;
    }

    // Set frontier to visited
//    for (size_t j = 0; j < frontier.size(); j++) {
//      visited[frontier.at(j)] = true;
////      cout << "visited " << frontier.at(j) << " is " << visited[frontier.at(j)] << endl;
//    }

    // Get new frontier
    vector<uint32_t> newFrontier;
    for (size_t j = 0; j < frontier.size(); j++) {
//        cout << j << " ";
//        cout << frontier.at(j) << " ";
//        cout << edgeDict.at(frontier.at(j)).size() << endl;
      try{
        vector<uint32_t> &tmpList = edgeDict.at(frontier.at(j));
        for (size_t z = 0; z < tmpList.size(); z++) { // for each element in next node
//          cout << "ind " << z << " candidate " << tmpList.at(z) << " cond1 " << bool(nodeDict.at(tmpList.at(z)) == nextType) << " cond2 " << bool(!visited.at(tmpList.at(z))) << endl;
          if (nodeDict.at(tmpList.at(z)) == nextType && // type meet
              !visited[tmpList[z]]) { // never visited
            newFrontier.push_back(tmpList.at(z));
            visited[tmpList.at(z)] = true; // set as visited to avoid duplicate dest
          }
        }
      } catch(const exception& e) {
        cout << e.what() << endl;
      }

    }
    frontier = newFrontier;
//    for (int k = 0; k < frontier.size(); ++k) {
//      cout << "new frontier " << k << " " << frontier[k] << endl;
//    }
  }

  return frontier;

}

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

string path_to_string(vector<NodeType> nodevec) {
  ostringstream ostr;
  for(size_t i = 0; i < nodevec.size(); i++) {
    switch(nodevec[i]) {
      case NodeType::None:
        ostr << "None_";
        break;
      case NodeType::Author:
        ostr << "Author_";
        break;
      case NodeType::Paper:
        ostr << "Paper_";
        break;
      case NodeType::Venue:
        ostr << "Venue_";
        break;
      case NodeType::Term:
        ostr << "Term_";
    }
  }
  return ostr.str();
}


void dfs_lookup(uint32_t root, uint32_t src, uint16_t mpath, uint8_t depth, uint16_t pid, vector<vector<uint64_t>>& resVec, vector<bool>& visited) {
  if(src >= MAX_ID) {
    cout << src << ">="<< MAX_ID<<endl;
  }
  if(depth < max_depth) {
    // new metapath from root to src
//    cout <<"root "<<root<<" src " << src<<" mpath "<<path_to_string(decode(mpath)) <<" depth " << depth <<" pid "<<pid<<endl;
    visited[src] = true;
    for (size_t i = 0; i < edgeList[src].size(); ++i) {
      if (!visited[edgeList[src][i]]){ // not visited
        uint16_t new_mpath = mpath + nodeList[edgeList[src][i]] * pow(NODETYPE_BASE, depth + 1);
        if(nodeList[edgeList[src][i]] == Paper) { // a qualified endpoint, save it
          uint64_t item = (uint64_t(root) << 32) + (edgeList[src][i]);
          resVec[new_mpath].push_back(item);
//          cout << "root " << (item >> 32) << " end " << ((item << 32) >> 32) << " path " << path_to_string(decode(new_mpath)) <<endl;
//          cout << path_to_string(decode(new_mpath)) << " size " << global_result[pid][new_mpath].size();
        }
        dfs_lookup(root, edgeList[src][i], new_mpath, depth + 1, pid, resVec, visited);
      }
    }
    visited[src] = false;
  }
}

void newWorker(uint16_t pid) {
  {
    auto start_time = chrono::high_resolution_clock::now();

    // result vector for each thread
    vector<vector<uint64_t >> resVec;
    resVec.resize(1100);
    vector<bool> visited;
    visited.resize(MAX_ID, false);

    for (size_t i = pid; i < nodeList.size(); i += MAX_THREAD) {
      if (nodeList[i] == Paper && ((double) rand() / (double) RAND_MAX) <= PORTION) {
        dfs_lookup(i, i, nodeList[i], 0, pid, resVec, visited);

        uint64_t cnt = 0;
        for (size_t j = 0; j < resVec[pid].size(); j++) {
          cnt += resVec[j].size();
        }
//      cout << "get " << cnt <<" nodes" <<endl;
      }
    }

    auto duration = chrono::high_resolution_clock::now() - start_time;
    cout << "thread pid " << pid << " took " << chrono::duration_cast<chrono::microseconds>(duration).count() << endl;

    cout << "start saving result to disk" << endl;
    start_time = chrono::high_resolution_clock::now();
    ostringstream oss;
    for (size_t mpath = 0; mpath < 1100; mpath++) {
      for (size_t pos = 0; pos < resVec[mpath].size(); pos++) {
        oss << mpath << " " << (resVec[mpath][pos] >> 32) << " " << ((resVec[mpath][pos] << 32) >> 32) << endl;
      }
    }

    ofstream output;
    ostringstream filename;
    filename << "./result_pid_" << pid;
    output.open(filename.str(), ofstream::trunc);
    output << oss.str();
    oss.str("");
    oss.clear();
    output.close();

    duration = chrono::high_resolution_clock::now() - start_time;
    cout << "save pid " << pid << " took " << chrono::duration_cast<chrono::microseconds>(duration).count() << endl;
    for (int s = 0; s < resVec.size(); ++s) {
      vector<uint64_t >().swap(resVec[s]);
    }

    vector<vector<uint64_t>>().swap(resVec);
    cout << pid << " vector released\n";
  }
}

vector<vector<NodeType>> gen_metapath(uint32_t min_length, uint32_t length, vector<NodeType>& candidates) {
  vector<vector<NodeType>> mPath;
  uint cnt = 0;
  for(size_t path_len = min_length; path_len <= length; path_len++) {
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
  filename << path_to_string(metaPath[args.mpath_pos]);

  filename << args.partition;
//  cout << args.partition << endl;

  ofstream output;
  output.open(filename.str(), ofstream::trunc);
  ostringstream buf;
  size_t log_cnt= 0;


  bool *is_visited = (bool *)malloc(MAX_ID * sizeof(bool));
  memset(is_visited, false, sizeof(bool) * MAX_ID);
  for (size_t i = args.partition; i < nodeList.size(); i += MAX_THREAD) {
    try{
      if(nodeList[i] == metaPath[args.mpath_pos][0] && ((double)rand() / (double)RAND_MAX) <= PORTION) {
//        auto start_time = chrono::high_resolution_clock::now();
        vector<uint32_t> res = bfs_lookup(i, *args.nodeListPtr, *args.edgeListPtr, metaPath[args.mpath_pos], is_visited);
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
          if(log_cnt % 5000 == 0) {
            output << buf.str();
            buf.str("");
            buf.clear();
          }
        }
      }
    } catch (exception& e) {

    }
  }

  output << buf.str();
  buf.str("");
  buf.clear();
  output.close();

}

bool skip_metapath(vector<NodeType>& mPath) {

  if(mPath[0] != Paper || mPath[mPath.size()-1] != Paper) {
    return true;
  }

  for(size_t i = 0; i < mPath.size() - 1; i++) {
    if (mPath[i] == Author) {
      if ((mPath[i+1] == Author) || mPath[i+1] == Venue) {
        return true;
      }
    }
    if (mPath[i] == Venue) {
      if ((mPath[i+1] == Venue) || (mPath[i+1] == Author)) {
        return true;
      }
    }
  }

  return false;
}

int main(int args, char** argv) {

  if(args != 3) {
    cout << "Usage: ./MetaPathFinder metaPathMinLen metaPathMaxLen\n";
    return 233;
  } else {
    min_depth = uint8_t(atoi(argv[1]));
    max_depth = uint8_t(atoi(argv[2]));
  }

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
      uint32_t node2 = uint32_t(atoi(line.substr(pos + 1, line.size()).c_str()));
      edgeList[node1].push_back(node2);
      edgeList[node2].push_back(node1);
//      cout << node1 << "->" << node2 << endl;
    }
  }

  auto duration = chrono::high_resolution_clock::now() - start_time;
  cout << "Node types and edges are loaded, took " << chrono::duration_cast<chrono::microseconds>(duration).count() << endl;
  cout << "edge nodes " << edgeList.size() << endl;

  for(size_t i = 0; i < MAX_THREAD; i++) {
    threadList[i] = thread(newWorker, i);
  }

  for(size_t i = 0; i < MAX_THREAD; i++) {
    threadList[i].join();
  }

////    for(int j = metaPath.size() - 1; j >= 0; j--) {
//  for (size_t j = 0; j < metaPath.size(); j++) {
//    if(!skip_metapath(metaPath[j])){
////    if(metaPath[j][0] == NodeType::Paper && metaPath[j][metaPath[j].size()-1] == NodeType::Paper){
//      cout << "start computing " << path_to_string(metaPath[j]) << endl;
//      start_time = chrono::high_resolution_clock::now();
//      for(size_t i = 0; i < MAX_THREAD; i++) {
//        argList[i].partition = i;
//        argList[i].mpath_pos = j;
//        argList[i].nodeListPtr = &nodeList;
//        argList[i].edgeListPtr = &edgeList;
//        threadList[i] = thread(worker, ref(argList[i]));
//      }
//
//      for(size_t i = 0; i < MAX_THREAD; i++) {
//        threadList[i].join();
//      }
//
//      duration = chrono::high_resolution_clock::now() - start_time;
//      cout << "calculation took " << chrono::duration_cast<chrono::microseconds>(duration).count() << endl;
//    } else {
//      cout << "skip " << path_to_string(metaPath[j]) << endl;
//    }
//  }


  cout << "Hello, World!" << endl;
  return 0;
}