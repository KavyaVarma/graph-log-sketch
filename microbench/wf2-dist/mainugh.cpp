#include "wf2_graph.hpp"
#include "read_file.hpp"
#include "timer.hpp"
#include "pattern.hpp"
#include "galois/Galois.h"
#include "DistBench/Input.h"
#include "DistBench/Start.h"
#include "galois/DistGalois.h"
#include "galois/gstl.h"
#include "llvm/Support/CommandLine.h"
#include "DistBench/Input.h"
#include "DistBench/Start.h"
#include "galois/AtomicHelpers.h"
#include "galois/DReducible.h"
#include "galois/DTerminationDetector.h"
#include "galois/DistGalois.h"
#include "galois/Galois.h"
#include "galois/Version.h"
#include "galois/graphs/GenericPartitioners.h"
#include "galois/graphs/GluonSubstrate.h"
#include "galois/graphs/MiningPartitioner.h"
#include "galois/gstl.h"
#include "galois/runtime/Tracer.h"
#include "galois/substrate/SimpleLock.h"
#include "llvm/Support/CommandLine.h"
#include <string>
#include <chrono>
#include <iostream>
#include <vector>


// namespace cll = llvm::cl;

// static const cll::opt<std::uint64_t> numGThreads("t", cll::desc("<num threads>"), cll::init(1));
// static cll::opt<std::string> graphName("graphName", cll::desc("Name of the input graph"), cll::init("temp"));

/* TODO This is a really stupid hardcoding for a stupid error. FIXXX */
// static const cll::opt<std::string> inputGRFile(
//   "gf", cll::desc("<input gr file>"), cll::init("../graphs/data.gr"));
// static const cll::opt<std::string> inputNodeFile(
//   "nf", cll::desc("<node file>"), cll::init("../graphs/data.01.csv"));
std::string inputGRFile = std::string("../graphs/data.gr");
std::string inputNodeFile = std::string("../graphs/data.01.csv");

using namespace wf2;
using Graph = galois::graphs::DistGraph<Vertex, void>;
// using Graph = galois::graphs::DistGraph<Vertex, Edge>;
using GNode = Graph::GraphNode;

std::unique_ptr<galois::graphs::GluonSubstrate<Graph>> syncSubstrate;
auto& net = galois::runtime::getSystemNetworkInterface();
std::unique_ptr<Graph> hg;


struct SyncPhase1 {
    typedef Vertex ValTy;

    // Extract Vector of dests! Tell what to communicate:
    // For a node, tell what part of NodeData to Communicate
    static ValTy extract(uint32_t, const struct Vertex& node) {
        Vertex vec_ver(node);
          // std::cout<<"owned";
        return vec_ver;
    }

    // Reduce() -- what do after master receive
    // Reduce will send the data to the host
    // Masters = receivers, so will update master dests
    static bool reduce(uint32_t, struct Vertex& node, Vertex y) {
        node.setID(y.id());
        return true;
    }

    static void reset(uint32_t, struct Vertex& node) {
        
    }

    static void setVal(uint32_t, struct Vertex& node, ValTy y) {
        
    }

    static bool extract_batch(unsigned, uint8_t*, size_t*, DataCommMode*) { return false; }
    static bool extract_batch(unsigned, uint8_t*) { return false; }
    static bool extract_reset_batch(unsigned, uint8_t*, size_t*, DataCommMode*) { return false; }
    static bool extract_reset_batch(unsigned, uint8_t*) { return false; }
    static bool reset_batch(size_t, size_t) { return false; }
    static bool reduce_batch(unsigned, uint8_t*, DataCommMode) { return false; }
    static bool reduce_mirror_batch(unsigned, uint8_t*, DataCommMode) { return false; }
    static bool setVal_batch(unsigned, uint8_t*, DataCommMode) { return false; }
};
galois::DynamicBitSet bitset_dests;

struct BitsetPhase1 {
    static constexpr bool is_vector_bitset() { return false; }
    static constexpr bool is_valid() { return true; }
    static galois::DynamicBitSet& get() { return bitset_dests; }
    static void reset_range(size_t begin, size_t end) {
        bitset_dests.reset(begin, end);
    }
};
  

int main(int argc, char** argv) {

  // cll::ParseCommandLineOptions(argc, argv);
  // galois::setActiveThreads(numGThreads);
  // std::cout << "threads: " << numGThreads << std::endl;

  galois::DistMemSys G;
  /* TODO From where do I need to get these values?? */
  DistBenchStart(argc, argv, "name", "desc", "url");
  galois::runtime::getHostBarrier().wait();

  using edge_t = std::pair<uint64_t, uint64_t>;
  // auto construct_graph = [&] (uint64_t num_nodes, uint64_t num_edges, std::vector<edge_t>* edge_list, std::vector<Edge>& edges) { 
  //   return new Graph(num_nodes, num_edges, 
  //                   [edge_list](uint64_t n) { return edge_list[n].size(); },
  //                   [edge_list](uint64_t n, uint64_t e) { return edge_list[n][e].first; },
  //                   [edge_list, edges](uint64_t n, uint64_t e) {
  //                     uint64_t edge_id = edge_list[n][e].second;
  //                     return edges[edge_id]; 
  //                   });
  // };

  // auto construct_graph = [&] (std::string inputGRFile) {
  //   return new Graph(inputGRFile, 0, 1);
  // };

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
  std::tie(hg, syncSubstrate) = distGraphInitialization<Vertex, void>();
        galois::runtime::getHostBarrier().wait();
  read_file<Graph, GNode>(*hg, inputNodeFile.c_str());
  syncSubstrate->sync<writeDestination, readSource, SyncPhase1, BitsetPhase1, false>("Phase1Communication");
  timer.lap("Graph construction");

  // WMD_pattern(graph);

  Graph& hg_ref = *hg;
  bitset_dests.resize(hg_ref.size());

  // uint32_t num_hosts = hg->getNumHosts();
	uint64_t host_id = galois::runtime::getSystemNetworkInterface().ID;

  galois::do_all(
        galois::iterate(hg_ref.allNodesWithEdgesRange()),
        [&](const GNode& gNode) {
          auto node = hg_ref.getData(gNode);
          // std::cout << graph_type_to_str(node.v_type);
          
          if(node.v_type == TYPES::FORUMEVENT) {
            bool topic_1 = false;
            bool topic_2 = false;
            if(node.id() == 1655573355554454431) {
              std::cout << "hello\n";
              for (auto e : hg_ref.edges(gNode)) {
                auto edge_node = hg_ref.getData(hg_ref.getEdgeDst(e));
                auto edge_dest = hg_ref.getEdgeDst(e);
                std::cout << hg_ref.isOwned(hg_ref.getGID(edge_dest)) << " " << hg_ref.getGID(edge_dest) << " " << edge_node.id() << "\n";
              }
            }
            for (auto e : hg_ref.edges(gNode)) {
              auto edge_node = hg_ref.getData(hg_ref.getEdgeDst(e));
              if( edge_node.v_type == TYPES::TOPIC ) {
                if ( edge_node.id() == 0 )
                 std::cout << "oof\n";
                if ( edge_node.id() == 69871376 )
                  topic_1 = true;
                if ( edge_node.id() == 1049632 )
                  topic_2 = true;
              }
            }
            if ( topic_1  )
              std::cout<<"here\n";
            if ( topic_2  )
              std::cout<<"there\n";
          }
        },
        galois::steal());



  // timer.lap("Pattern matching");

  return 0;
}
