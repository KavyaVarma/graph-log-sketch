#include <string>


template<typename Graph>
class WF2_Graph {
  Graph graph;

public:
  WF2_Graph(Graph g) : graph(g) {}

  template<typename GraphElem>
  GraphElem lookup(uint64_t id) {
    // find vertex/edge of GraphElem type with given unique id
  }

  template<typename GraphElem, typename Op>
  void do_all(Op f) {
    // execute f on each vertex/edge of GraphElem type
  }

  template<typename EdgeType, typename Op>
  void iter_edges(Op f, uint64_t id) {
    // call f on list of out-edges of vertex id with type EdgeType
  }

  void print_graph(std::string filename) {
    // print graph edges in sorted order
  }
};
