#include <string>


class WF2_Graph_API {
public:
  WF2_Graph_API(std::string filename);

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

};
