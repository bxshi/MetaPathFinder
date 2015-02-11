#include "mpf.h"

using namespace std;

uint8_t max_depth=6;
uint8_t min_depth=2;

uint32_t min_range=0;
uint32_t max_range=0;
double portion = 0.0002;


struct arg{
  size_t partition;
  vector<NodeType> *nodeListPtr;
  vector<vector<uint32_t >> *edgeListPtr;
  size_t mpath_pos;
};

vector<vector<NodeType>> metaPath;
vector<NodeType> nodeList;
vector<uint32_t > paperList;
vector<vector<uint32_t>> edgeList;
thread threadList[MAX_THREAD];

bool **global_visited;

void dfs_lookup(uint32_t root, uint32_t src, uint16_t mpath, uint8_t depth, uint16_t pid, ostringstream& oss) {
  if(depth < max_depth) {
    // new metapath from root to src
//    cout <<"root "<<root<<" src " << src<<" mpath "<<path_to_string(decode(mpath)) <<" depth " << depth <<" pid "<<pid<<endl;
    global_visited[pid][src] = true;
    for (size_t i = 0; i < edgeList[src].size(); ++i) {
      if (!global_visited[pid][edgeList[src][i]]){ // not visited
        uint16_t new_mpath = mpath + nodeList[edgeList[src][i]] * pow(NODETYPE_BASE, depth + 1);
        if(nodeList[edgeList[src][i]] == Paper) { // a qualified endpoint, save it
//          uint64_t item = (uint64_t(root) << 32) + (edgeList[src][i]);
          oss << new_mpath << " " << root << " " << edgeList[src][i] << endl;
//          resVec[new_mpath].push_back(item);
//          cout << "root " << (item >> 32) << " end " << ((item << 32) >> 32) << " path " << path_to_string(decode(new_mpath)) <<endl;
//          cout << path_to_string(decode(new_mpath)) << " size " << global_result[pid][new_mpath].size();
        }
        dfs_lookup(root, edgeList[src][i], new_mpath, depth + 1, pid, oss);
      }
    }
    global_visited[pid][src] = false;
  }
}

void newWorker(uint16_t pid) {
  {
    auto start_time = chrono::high_resolution_clock::now();
    ostringstream oss;

    ofstream output;
    ostringstream filename;
    filename << "./result_pid_" << pid;
    output.open(filename.str(), ofstream::trunc);

    // result vector for each thread

    int cnt = 0;

    for (size_t i = pid + min_range; i < max_range; i += MAX_THREAD) {
      if (nodeList[paperList[i]] == Paper && ((double) rand() / (double) RAND_MAX) <= portion) {
        dfs_lookup(paperList[i], paperList[i], nodeList[paperList[i]], 0, pid, oss);
        cnt++;
//      cout << "get " << cnt <<" nodes" <<endl;
      }
      if (cnt % 2 == 0) {
        output << oss.str();
        oss.str("");
        oss.clear();
      }
    }

    auto duration = chrono::high_resolution_clock::now() - start_time;
    cout << "thread pid " << pid << " took " << chrono::duration_cast<chrono::microseconds>(duration).count() << endl;

    cout << "start saving result to disk" << endl;
    start_time = chrono::high_resolution_clock::now();


    output << oss.str();
    oss.str("");
    oss.clear();
    output.close();

    duration = chrono::high_resolution_clock::now() - start_time;
    cout << "save pid " << pid << " took " << chrono::duration_cast<chrono::microseconds>(duration).count() << endl;
    free(global_visited[pid]);

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

int main(int args, char** argv) {

  if(args != 6) {
    cout << "Usage: ./MetaPathFinder metaPathMinLen metaPathMaxLen minRange maxRange total\n";
    return 233;
  } else {
    min_depth = uint8_t(atoi(argv[1]));
    max_depth = uint8_t(atoi(argv[2]));
    min_range = atoi(argv[3]);
    max_range = atoi(argv[4]);
  }

  global_visited = (bool **)malloc(sizeof(bool *) * MAX_THREAD);
  for(size_t i = 0; i < MAX_THREAD; i++) {
    global_visited[i] = (bool *) malloc(sizeof(bool) * MAX_ID);
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
      paperList.push_back(atoi(line.substr(1, pos).c_str()));
      //Update max_range
//      cout << 1 << " " << pos << " " << atoi(line.substr(1, pos).c_str()) << " " << nodeList[atoi(line.substr(1, pos).c_str())] << endl;
    }
  }

  max_range = max_range > paperList.size() ? (uint32_t)paperList.size() : max_range;
  portion = double(atoi(argv[5])) / double(max_range - min_range);
  cout << "paperList.size " << paperList.size() << " min_range " << min_range << " max_range " << max_range << endl;

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

      uint32_t node1 = uint32_t(atoi(line.substr(0, pos).c_str()));
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

  return 0;
}