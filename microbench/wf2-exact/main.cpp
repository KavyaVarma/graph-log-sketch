#include "wf2_graph.hpp"
#include "read_file.hpp"
#include "timer.hpp"
#include "pattern.hpp"
#include "galois/Galois.h"
#include "galois/gstl.h"
#include "llvm/Support/CommandLine.h"
#include <string>
#include <chrono>
#include <iostream>
#include <vector>


namespace cll = llvm::cl;

static const cll::opt<std::uint64_t> numGThreads("t", cll::desc("<num threads>"), cll::init(1));

static const cll::opt<std::string> inputFile(
  "f", cll::desc("<input file>"), cll::init(""));

int main(int argc, char** argv) {
  using namespace wf2;
  using Graph = LS_LC_CSR_Graph;
  using GNode = Graph::GraphNode;

  cll::ParseCommandLineOptions(argc, argv);
  galois::SharedMemSys G;
  galois::setActiveThreads(numGThreads);
  std::cout << "threads: " << numGThreads << std::endl;

  using edge_t = std::pair<uint64_t, uint64_t>;
  auto construct_graph = [&] (uint64_t num_nodes, uint64_t num_edges, std::vector<edge_t>* edge_list, std::vector<Edge>& edges) { 
    return new Graph(num_nodes, num_edges, 
                    [edge_list](uint64_t n) { return edge_list[n].size(); },
                    [edge_list](uint64_t n, uint64_t e) { return edge_list[n][e].first; },
                    [edge_list, edges](uint64_t n, uint64_t e) {
                      uint64_t edge_id = edge_list[n][e].second;
                      return edges[edge_id]; 
                    });
  };

  // auto construct_graph_fast = [] (uint64_t num_nodes, uint64_t num_edges, std::vector<edge_t>* edge_list, std::vector<Edge>& edges) {
  //   return new Graph(true, num_nodes, num_edges, 
  //                   [edge_list](uint64_t n) { return edge_list[n].size(); },
  //                   [edge_list](uint64_t n, uint64_t e) { return edge_list[n][e].first; },
  //                   [edge_list, edges](uint64_t n) {
  //                     uint64_t edge_id = edge_list[n][e].second;
  //                     return edges[edge_id]; 
  //                   });
  // };

  Timer timer;
  WF2_Graph<Graph> graph = read_file<Graph>(inputFile, construct_graph);
  timer.lap("Graph construction");

  WMD_pattern(graph);
  timer.lap("Pattern matching");

  return 0;
}
