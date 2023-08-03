#ifndef _READ_FILE_HPP_
#define _READ_FILE_HPP_

#include "timer.hpp"
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <unordered_map>

namespace wf2
{

  std::vector<std::string> split(std::string &line, char delim, uint64_t size = 0)
  {
    uint64_t ndx = 0, start = 0, end = 0;
    std::vector<std::string> tokens(size);

    for (; end < line.length(); end++)
    {
      if ((line[end] == delim) || (line[end] == '\n'))
      {
        tokens[ndx] = line.substr(start, end - start);
        start = end + 1;
        ndx++;
      }
    }

    tokens[size - 1] = line.substr(start, end - start); // flush last token
    return tokens;
  }

  template <typename Graph, typename GNode>
  std::unordered_map<uint64_t, uint64_t> read_file(Graph &graph, std::string inputNodeFilename)
  {
    Timer timer;

    std::ifstream file(inputNodeFilename);
    if (!file.is_open())
    {
      std::cout << "Failed to open graph file: " << inputNodeFilename << std::endl;
    }

    std::vector<Vertex> vertices;
    std::vector<Edge> edges;

    std::string line;
    while (std::getline(file, line))
    {
      if (line[0] == '#')
        continue;

      std::vector<std::string> tokens = split(line, ',', 10);
      if (tokens[0] == "Person")
      {
        PersonVertex person(tokens);
        Vertex v(person);
        vertices.push_back(v);
      }
      else if (tokens[0] == "ForumEvent")
      {
        ForumEventVertex forum_event(tokens);
        Vertex v(forum_event);
        vertices.push_back(v);
      }
      else if (tokens[0] == "Forum")
      {
        ForumVertex forum(tokens);
        Vertex v(forum);
        vertices.push_back(v);
      }
      else if (tokens[0] == "Publication")
      {
        PublicationVertex pub(tokens);
        Vertex v(pub);
        vertices.push_back(v);
      }
      else if (tokens[0] == "Topic")
      {
        TopicVertex topic(tokens);
        Vertex v(topic);
        vertices.push_back(v);
      }
      else if (tokens[0] == "Sale")
      {
        SaleEdge sale(tokens);
        PurchaseEdge purchase(tokens);
        Edge e1(sale);
        Edge e2(purchase);
        edges.push_back(e1);
        edges.push_back(e2);
      }
      else if (tokens[0] == "Author")
      {
        AuthorEdge author(tokens);
        Edge e(author);
        edges.push_back(e);
      }
      else if (tokens[0] == "Includes")
      {
        IncludesEdge includes(tokens);
        Edge e(includes);
        edges.push_back(e);
      }
      else if (tokens[0] == "HasTopic")
      {
        HasTopicEdge topic(tokens);
        Edge e(topic);
        edges.push_back(e);
      }
      else if (tokens[0] == "HasOrg")
      {
        HasOrgEdge org(tokens);
        Edge e(org);
        edges.push_back(e);
      }
    }

    timer.lap("[read_file] File read");

    uint64_t num_nodes = vertices.size();
    uint64_t num_edges = edges.size();
    std::unordered_map<uint64_t, uint64_t> id_to_node_index;
    using edge_t = std::pair<uint64_t, uint64_t>;
    std::vector<edge_t> *edge_list = new std::vector<edge_t>[num_nodes];

    for (uint64_t i = 0; i < num_nodes; i++)
    {
      uint64_t id = vertices[i].id();
      id_to_node_index[id] = i;
    }

    timer.lap("[read_file] id_to_node_index map constructed");

    for (uint64_t i = 0; i < num_edges; i++)
    {
      uint64_t src_global_id = edges[i].src();
      uint64_t dst_global_id = edges[i].dst();
      uint64_t src_id = id_to_node_index[src_global_id];
      uint64_t dst_id = id_to_node_index[dst_global_id];
      edge_list[src_id].push_back(std::make_pair(dst_id, i));
    }

    // std::ofstream myfile ("data.el");
    // if (myfile.is_open())
    // {
    //   for (uint64_t i = 0; i < num_nodes; i++) {
    //     for (auto e: edge_list[i])
    //       myfile << i << " " << e.first << " 1\n";
    //   }
    //   myfile.close();
    // }
    // else std::cout << "Unable to open file";

    timer.lap("[read_file] edge_list constructed");

    Edge **edges_grouped = new Edge *[num_nodes];
    for (uint64_t n = 0; n < num_nodes; n++)
    {
      edges_grouped[n] = new Edge[edge_list[n].size()];
      for (uint64_t e = 0; e < edge_list[n].size(); e++)
      {
        uint64_t edge_id = edge_list[n][e].second;
        edges_grouped[n][e] = edges[edge_id];
      }
    }

    // Graph* graph = construct_graph(num_nodes, num_edges, edge_list, edges);
    // Graph* graph = construct_graph(inputGRFilename);
    delete[] edge_list;

    timer.lap("[read_file] graph constructed");

    galois::do_all(
        galois::iterate(graph.allNodesWithEdgesRange()),
        [&](const GNode &gNode)
        {
          Vertex &node = graph.getData(gNode);
          node.setVertexType(vertices[graph.getGID(gNode)].v_type);
          node.setID(vertices[graph.getGID(gNode)].id());

          switch (node.v_type)
          {
          case wf2::TYPES::PERSON:
          {
            node.v.person.glbid = vertices[graph.getGID(gNode)].v.person.glbid;
            break;
          }
          case wf2::TYPES::FORUMEVENT:
          {
            node.v.forum_event.glbid = vertices[graph.getGID(gNode)].v.forum_event.glbid;
            node.v.forum_event.forum = vertices[graph.getGID(gNode)].v.forum_event.forum;
            node.v.forum_event.date = vertices[graph.getGID(gNode)].v.forum_event.date;

            break;
          }
          case wf2::TYPES::FORUM:
          {
            node.v.forum.glbid = vertices[graph.getGID(gNode)].v.forum.glbid;
            break;
          }
          case wf2::TYPES::PUBLICATION:
          {
            node.v.publication.glbid = vertices[graph.getGID(gNode)].v.publication.glbid;
            node.v.publication.date = vertices[graph.getGID(gNode)].v.publication.date;

            break;
          }
          case wf2::TYPES::TOPIC:
          {
            node.v.topic.glbid = vertices[graph.getGID(gNode)].v.topic.glbid;
            node.v.topic.lat = vertices[graph.getGID(gNode)].v.topic.lat;
            node.v.topic.lon = vertices[graph.getGID(gNode)].v.topic.lon;

            break;
          }
          }
        });

    // for (uint64_t i = 0; i < num_nodes; i++) {
    //   Vertex& v = graph.getData(i);
    //   // v = vertices[i];
    // }

    timer.lap("[read_file] node data stored in graph");

    // return WF2_Graph(graph, id_to_node_index);
    return id_to_node_index;
  }

} // namespace wf2

#endif
