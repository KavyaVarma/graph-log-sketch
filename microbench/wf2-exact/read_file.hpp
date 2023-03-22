#ifndef _READ_FILE_HPP_
#define _READ_FILE_HPP_

#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>

namespace wf2 {

std::vector<std::string> split(std::string& line, char delim, uint64_t size = 0) {
  uint64_t ndx = 0, start = 0, end = 0;
  std::vector<std::string> tokens(size);

  for ( ; end < line.length(); end ++)  {
    if ( (line[end] == delim) || (line[end] == '\n') ) {
       tokens[ndx] = line.substr(start, end - start);
       start = end + 1;
       ndx ++;
    } 
  }

  tokens[size - 1] = line.substr(start, end - start);     // flush last token
  return tokens;
}

template<typename Graph, typename GraphConstructor>
Graph* read_file(std::string filename, GraphConstructor construct_graph) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cout << "Failed to open graph file: " << filename << std::endl;
  }

  std::vector<Vertex> vertices;
  std::vector<Edge> edges;

  std::string line;
  while (std::getline(file, line)) {
    if (line[0] == '#') continue;

    std::vector<std::string> tokens = split(line, ',', 10);
    if (tokens[0] == "Person") {
      PersonVertex person(tokens);
      Vertex v(person);
      vertices.push_back(v);
    } else if (tokens[0] == "ForumEvent") {
      ForumEventVertex forum_event(tokens);
      Vertex v(forum_event);
      vertices.push_back(v);
    } else if (tokens[0] == "Forum") {
      ForumVertex forum(tokens);
      Vertex v(forum);
      vertices.push_back(v);
    } else if (tokens[0] == "Publication") {
      PublicationVertex pub(tokens);
      Vertex v(pub);
      vertices.push_back(v);
    } else if (tokens[0] == "Topic") {
      TopicVertex topic(tokens);
      Vertex v(topic);
      vertices.push_back(v);
    } else if (tokens[0] == "Sale") {
      SaleEdge sale(tokens);
      PurchaseEdge purchase(tokens);
      Edge e1(sale);
      Edge e2(purchase);
      edges.push_back(e1);
      edges.push_back(e2);
    } else if (tokens[0] == "Author") {
      AuthorEdge author(tokens);
      Edge e(author);
      edges.push_back(e);
    } else if (tokens[0] == "Includes") {
      IncludesEdge includes(tokens);
      Edge e(includes);
      edges.push_back(e);
    } else if (tokens[0] == "HasTopic") {
      HasTopicEdge topic(tokens);
      Edge e(topic);
      edges.push_back(e);
    } else if (tokens[0] == "HasOrg") {
      HasOrgEdge org(tokens);
      Edge e(org);
      edges.push_back(e);
    }
  }

  uint64_t num_nodes = vertices.size();
  uint64_t num_edges = edges.size();
  std::unordered_map<uint64_t, uint64_t> id_to_node_id;
  using edge_t = std::pair<uint64_t, uint64_t>;
  std::vector<edge_t>* edge_list = new std::vector<edge_t>[num_nodes];

  for (uint64_t i = 0; i < num_nodes; i++) {
    uint64_t id = vertices[i].id();
    id_to_node_id[id] = i;
  }

  for (uint64_t i = 0; i < num_edges; i++) {
    uint64_t src_id = edges[i].src();
    uint64_t dst_id = edges[i].dst();
    uint64_t vertex_id = id_to_node_id[src_id];
    edge_list[vertex_id].push_back(std::make_pair(dst_id, i));
  }

//  Graph* graph = new Graph(num_nodes, num_edges, 
//                    [edge_list](uint64_t n) { return edge_list[n].size(); },
//                    [edge_list](uint64_t n, uint64_t e) { return edge_list[n][e].first; },
//                    [edge_list, edges](uint64_t n, uint64_t e) {
//                        uint64_t edge_id = edge_list[n][e].second;
//                        return edges[edge_id]; });

  Graph* graph = construct_graph(num_nodes, num_edges, edge_list, edges);
  delete [] edge_list;

  for (uint64_t i = 0; i < num_nodes; i++) {
    Vertex& v = graph->getData(i);
    v = vertices[i];
  }

  return graph;
}

} // namespace wf2

#endif

