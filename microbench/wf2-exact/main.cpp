#include "wf2_graph.hpp"
#include "read_file.hpp"
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

int main(int argc, char** argv) {
  using namespace wf2;
  using Graph = LC_CSR_Graph;
  using GNode = Graph::GraphNode;

  galois::SharedMemSys G;
  galois::setActiveThreads(numGThreads);

  std::string filename = argv[1];

  auto t0 = std::chrono::high_resolution_clock::now();

  using edge_t = std::pair<uint64_t, uint64_t>;
  auto construct_graph = [] (uint64_t num_nodes, uint64_t num_edges, std::vector<edge_t>* edge_list, std::vector<Edge>& edges) { 
    return new Graph(num_nodes, num_edges, 
                    [edge_list](uint64_t n) { return edge_list[n].size(); },
                    [edge_list](uint64_t n, uint64_t e) { return edge_list[n][e].first; },
                    [edge_list, edges](uint64_t n, uint64_t e) {
                      uint64_t edge_id = edge_list[n][e].second;
                      return edges[edge_id]; 
                    });
  };


//  auto construct_graph_fast = [] () {
//
//  };

  Graph* graph = read_file<LC_CSR_Graph>(filename, construct_graph);

  auto t1 = std::chrono::high_resolution_clock::now();

  auto diff_ns = std::chrono::duration<uint64_t, std::nano>(t1-t0);
  uint64_t diff = std::chrono::duration_cast<std::chrono::milliseconds>(diff_ns).count();
  uint64_t diff_sec_part = diff / 1000;
  uint64_t diff_ms_part = diff % 1000;
  std::cout << "Graph_Construction_Time: " << diff_sec_part 
    << "." << diff_ms_part << " seconds" << std::endl;

  std::cout << "num nodes: " << graph->size() << std::endl;
  std::cout << "num nodes: " << graph->sizeEdges() << std::endl; 

  // galois::for_each(
  //   galois::iterate(graph->begin(), graph->end()),
  //   [&] (GNode n, auto&) {
  //     for (auto e : graph->edges(n)) {
  //       auto e_data = graph->getEdgeData(e);
  //       std::cout << graph_type_to_str(e_data.e_type) << ": ";
  //       std::cout << e_data.src() << " --> " << e_data.dst() << "\n";
  //     }
  //   },
  //   galois::loopname("print_csr_edges"));

//  WMD_pattern(graph);

  return 0;
}
