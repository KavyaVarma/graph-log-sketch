#include "wf2_graph.hpp"
#include "read_file.hpp"
#include "pattern.hpp"
#include <string>

int main(int argc, char** argv) {
  galois::SharedMemSys G;

  std::string filename = argv[1];
  wf2::Graph graph = read_file(filename);

  WMD_pattern(graph);

  return 0;
}
