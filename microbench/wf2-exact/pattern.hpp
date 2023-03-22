#ifndef _PATTERN_HPP_
#define _PATTERN_HPP_

#include "galois/Galois.h"
#include "wf2_graph.hpp"
#include <unordered_map>

namespace wf2 {

template<typename Graph>
void forumPattern_2B(Graph* g, std::unordered_map<int64_t, time_t>& forums2) {
  
}


template<typename Graph>
void forumPattern_2A(Graph* g, std::unordered_map<int64_t, time_t>& forums2) {
  using GNode = typename Graph::GraphNode;

  galois::for_each(
    galois::iterate(g->begin(), g->end()),
    [&] (GNode n, auto&) {
      for (auto e : g->edges(n)) {
        auto e_data = g->getEdgeData(e);
        if (e_data.e_type != TYPES::INCLUDES) {
          continue;
        }

        // for each includes edge in graph
        
      }
    });
}

template<typename Graph>
void WMD_pattern(Graph* g) {
  std::unordered_map<uint64_t, time_t> forums2;

  forumPattern_2A(g, forums2);

  // for each includes edge (F -> FE):
  //   - forumPattern_2A
  //   - on each HasTopic edge with FE as src, run Lambda2A
  //   - if FE points to topics 69871376 and 1049632, 
  //     add F of FE to forums2

  // for each F in forums2:
  //   - forumPattern_2B
  //   - 
}

} // namespace wf2

#endif
