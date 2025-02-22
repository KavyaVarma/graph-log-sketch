//===------------------------------------------------------------*- C++ -*-===//
//
//                            The AGILE Workflows
//
//===----------------------------------------------------------------------===//
// ** Pre-Copyright Notice
//
// This computer software was prepared by Battelle Memorial Institute,
// hereinafter the Contractor, under Contract No. DE-AC05-76RL01830 with the
// Department of Energy (DOE). All rights in the computer software are reserved
// by DOE on behalf of the United States Government and the Contractor as
// provided in the Contract. You are authorized to use this computer software
// for Governmental purposes but it is not to be released or distributed to the
// public. NEITHER THE GOVERNMENT NOR THE CONTRACTOR MAKES ANY WARRANTY, EXPRESS
// OR IMPLIED, OR ASSUMES ANY LIABILITY FOR THE USE OF THIS SOFTWARE. This
// notice including this sentence must appear on any copies of this computer
// software.
//
// ** Disclaimer Notice
//
// This material was prepared as an account of work sponsored by an agency of
// the United States Government. Neither the United States Government nor the
// United States Department of Energy, nor Battelle, nor any of their employees,
// nor any jurisdiction or organization that has cooperated in the development
// of these materials, makes any warranty, express or implied, or assumes any
// legal liability or responsibility for the accuracy, completeness, or
// usefulness or any information, apparatus, product, software, or process
// disclosed, or represents that its use would not infringe privately owned
// rights. Reference herein to any specific commercial product, process, or
// service by trade name, trademark, manufacturer, or otherwise does not
// necessarily constitute or imply its endorsement, recommendation, or favoring
// by the United States Government or any agency thereof, or Battelle Memorial
// Institute. The views and opinions of authors expressed herein do not
// necessarily state or reflect those of the United States Government or any
// agency thereof.
//
//                    PACIFIC NORTHWEST NATIONAL LABORATORY
//                                 operated by
//                                   BATTELLE
//                                   for the
//                      UNITED STATES DEPARTMENT OF ENERGY
//                       under Contract DE-AC05-76RL01830
//===----------------------------------------------------------------------===//

#include "main.h"
#include "graph.h"

using namespace agile::workflow1;

int main(int argc, char *argv[]) {
  printf("Begin Wf1 Galois!\n");
  galois::Timer timer;
  timer.start();

  galois::SharedMemSys G;  // init galois memory

  // Handle handle;
  Graph_t graph;
  std::string dataFile = argv[1];
  EdgeType * Edges = new EdgeType(LARGE); 
  GlobalIDType * GlobalIDS = new GlobalIDType(MEDIUM);

  graph["Edges"] = (uint64_t) Edges;  // Vertex to Edges mapping, key is vertex encoding
  graph["GlobalIDS"] = (uint64_t) GlobalIDS;   // Vertices, key is vertex encoding

  RF_args_t args;
  args.Edges_OID     = graph["Edges"];
  args.GlobalIDS_OID = graph["GlobalIDS"];
  memcpy(args.filename, dataFile.c_str(), dataFile.size() + 1);

  readFile(args);

  /********** CREATE COMPRESSED EDGE ARRAY AND VERTEX ARRAY **********/
  uint64_t num_vertices = GlobalIDS->size();
  uint64_t num_edges = Edges->size();

  CSR(graph, num_vertices, num_edges);
  timer.stop();
  printf("Time for graph construction = %lf\n", (double) timer.get_usec() / 1000000);

  CSR_t * csr = (CSR_t *) graph["CSR"];
  printf("Total number of vertices = %lu\n", csr->size());
  printf("Total number of edges    = %lu\n", Edges->size());

  free((EdgeType *) Edges);
  free((GlobalIDType *) GlobalIDS);
  free((VertexType *) graph["Vertices"]);
  free((EdgeType *) graph["VertexEdges"]);
  free((CSR_t *) graph["CSR"]);
  return 0;
}
