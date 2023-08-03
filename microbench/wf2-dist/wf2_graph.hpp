#ifndef _WF2_GRAPH_HPP_
#define _WF2_GRAPH_HPP_

#include "galois/graphs/LC_CSR_64_Graph.h"
#include "galois/graphs/LS_LC_CSR_64_Graph.h"
#include "galois/graphs/DistributedGraph.h"
#include "galois/graphs/MiningPartitioner.h"
#include "galois/graphs/GenericPartitioners.h"
#include "galois/graphs/NewGeneric.h"
#include <ctime>
#include <cstdint>
#include <limits>
#include <string>
#include <type_traits>
#include <unordered_map>

namespace wf2
{

  enum class DATA_TYPES
  {
    UINT,
    DOUBLE,
    USDATE
  };

  template <typename ENC_t>
  constexpr ENC_t kNullValue = ENC_t();

  template <>
  constexpr uint64_t kNullValue<uint64_t> = std::numeric_limits<int64_t>::max();

  template <>
  constexpr time_t kNullValue<time_t> = std::numeric_limits<time_t>::max();

  template <>
  constexpr double kNullValue<double> = std::numeric_limits<double>::max();

  template <typename ENC_t, typename IN_t, DATA_TYPES DT>
  ENC_t encode(IN_t &in);

  template <>
  inline uint64_t encode<uint64_t, std::string, DATA_TYPES::UINT>(std::string &str)
  {
    uint64_t value;
    try
    {
      value = std::stoull(str);
    }
    catch (...)
    {
      value = kNullValue<uint64_t>;
    }
    return value;
  }

  template <>
  inline double encode<double, std::string, DATA_TYPES::DOUBLE>(std::string &str)
  {
    double value;
    try
    {
      value = std::stod(str);
    }
    catch (...)
    {
      value = kNullValue<double>;
    }
    return value;
  }

  template <>
  inline time_t encode<time_t, std::string, DATA_TYPES::USDATE>(std::string &str)
  {
    struct tm date
    {
    };
    date.tm_isdst = -1;
    strptime(str.c_str(), "%m/%d/%y", &date);
    time_t t;
    try
    {
      t = mktime(&date);
    }
    catch (...)
    {
      return kNullValue<uint64_t>;
    }
    return t;
  }

  enum class TYPES
  {
    PERSON,
    FORUM,
    FORUMEVENT,
    PUBLICATION,
    TOPIC,
    PURCHASE,
    SALE,
    AUTHOR,
    INCLUDES,
    HASTOPIC,
    HASORG,
    NONE
  };

  std::string graph_type_to_str(TYPES t)
  {
    switch (t)
    {
    case TYPES::PERSON:
      return std::string("PersonVertex");
    case TYPES::FORUMEVENT:
      return std::string("ForumEventVertex");
    case TYPES::FORUM:
      return std::string("ForumVertex");
    case TYPES::PUBLICATION:
      return std::string("PublicationVertex");
    case TYPES::TOPIC:
      return std::string("TopicVertex");
    case TYPES::PURCHASE:
      return std::string("PurchaseEdge");
    case TYPES::SALE:
      return std::string("SaleEdge");
    case TYPES::AUTHOR:
      return std::string("AuthorEdge");
    case TYPES::INCLUDES:
      return std::string("IncludesEdge");
    case TYPES::HASTOPIC:
      return std::string("HasTopicEdge");
    case TYPES::HASORG:
      return std::string("HasOrgEdge");
    default:
      return std::string("UnknownGraphType");
    }
  }

  class BaseVertex
  {
  public:
    uint64_t id;

    void setID(uint64_t id)
    {
      this->id = id;
    }
  };

  class PersonVertex : public BaseVertex
  {
  public:
    uint64_t glbid;
    
    bool ammo_sp;
    bool ee_sp;
    time_t trans_date;

    void setGlbid(uint64_t glbid)
    {
      this->glbid = glbid;
    }

    void setAmmoSP(uint64_t ammo_sp)
    {
      this->ammo_sp = ammo_sp;
    }

    void setEESP(uint64_t ammo_sp)
    {
      this->ee_sp = ee_sp;
    }

    void setTransDate(time_t trans_date)
    {
      this->trans_date = trans_date;
    }

    PersonVertex()
    {
      id = kNullValue<uint64_t>;
      glbid = kNullValue<uint64_t>;
      ammo_sp = false;
      ee_sp = false;
      trans_date = kNullValue<time_t>;
    }

    PersonVertex(std::vector<std::string> &tokens)
    {
      id = encode<uint64_t, std::string, DATA_TYPES::UINT>(tokens[1]);
      glbid = kNullValue<uint64_t>;
    }

    uint64_t key() { return id; }
  };

  class ForumEventVertex : public BaseVertex
  {
  public:
    uint64_t forum;
    time_t date;
    uint64_t glbid;

    bool forum2A;
    bool forum2B;
    bool jihad_topic;
    bool nyc_topic;

    time_t forum_min_time;

    void setForum(uint64_t forum)
    {
      this->forum = forum;
    }

    void setDate(time_t date)
    {
      this->date = date;
    }

    void setGlbid(uint64_t glbid)
    {
      this->glbid = glbid;
    }

    void setForum2A(bool forum2A) {
      this->forum2A = forum2A;
    }

    void setForum2B(bool forum2B) {
      this->forum2B = forum2B;
    }

    void setJihadTopic(bool jihad_topic) {
      this->jihad_topic = jihad_topic;
    }

    void setNYCTopic(bool nyc_topic) {
      this->nyc_topic = nyc_topic;
    }

    void setForumMinTime(bool forum_min_time) {
      this->forum_min_time = forum_min_time;
    }

    ForumEventVertex()
    {
      id = kNullValue<uint64_t>;
      forum = kNullValue<uint64_t>;
      date = kNullValue<time_t>;
      glbid = kNullValue<uint64_t>;
      forum2A = false;
      forum2B = false;
      jihad_topic = false;
      nyc_topic = false;
      forum_min_time = kNullValue<time_t>;
    }

    ForumEventVertex(std::vector<std::string> &tokens)
    {
      id = encode<uint64_t, std::string, DATA_TYPES::UINT>(tokens[4]);
      forum = encode<uint64_t, std::string, DATA_TYPES::UINT>(tokens[3]);
      date = encode<time_t, std::string, DATA_TYPES::USDATE>(tokens[7]);
      glbid = kNullValue<uint64_t>;
    }

    uint64_t key() { return id; }
  };

  class ForumVertex : public BaseVertex
  {
  public:
    uint64_t glbid;
    time_t min_time;


    void setGlbid(uint64_t glbid)
    {
      this->glbid = glbid;
    }

    void setMinDate(time_t date)
    {
      this->min_time = date;
    }

    ForumVertex()
    {
      id = kNullValue<uint64_t>;
      glbid = kNullValue<uint64_t>;
      min_time = kNullValue<time_t>;
    }

    ForumVertex(std::vector<std::string> &tokens)
    {
      id = encode<uint64_t, std::string, DATA_TYPES::UINT>(tokens[3]);
      glbid = kNullValue<uint64_t>;
    }

    uint64_t key() { return id; }
  };

  class PublicationVertex : public BaseVertex
  {
  public:
    time_t date;
    uint64_t glbid;

    bool has_org_near_nyc;
    bool ee_topic;

    void setGlbid(uint64_t glbid)
    {
      this->glbid = glbid;
    }

    void setDate(time_t date)
    {
      this->date = date;
    }

    void setHasOrgNearNYC(bool has_org_near_nyc)
    {
      this->has_org_near_nyc = has_org_near_nyc;
    }

    void setEETopic(bool ee_topic)
    {
      this->ee_topic = ee_topic;
    }

    PublicationVertex()
    {
      id = kNullValue<uint64_t>;
      date = kNullValue<time_t>;
      glbid = kNullValue<uint64_t>;
      has_org_near_nyc = false;
      ee_topic = false;
    }

    PublicationVertex(std::vector<std::string> &tokens)
    {
      id = encode<uint64_t, std::string, DATA_TYPES::UINT>(tokens[5]);
      date = encode<time_t, std::string, DATA_TYPES::USDATE>(tokens[7]);
      glbid = kNullValue<uint64_t>;
    }

    uint64_t key() { return id; }
  };

  class TopicVertex : public BaseVertex
  {
  public:
    double lat;
    double lon;
    uint64_t glbid;

    void setLat(double lat)
    {
      this->lat = lat;
    }

    void setLon(double lon)
    {
      this->lon = lon;
    }

    void setGlbid(uint64_t glbid)
    {
      this->glbid = glbid;
    }

    TopicVertex()
    {
      id = kNullValue<uint64_t>;
      lat = kNullValue<double>;
      lon = kNullValue<double>;
      glbid = kNullValue<uint64_t>;
    }

    TopicVertex(std::vector<std::string> &tokens)
    {
      id = encode<uint64_t, std::string, DATA_TYPES::UINT>(tokens[6]);
      lat = encode<double, std::string, DATA_TYPES::DOUBLE>(tokens[8]);
      lon = encode<double, std::string, DATA_TYPES::DOUBLE>(tokens[9]);
      glbid = kNullValue<uint64_t>;
    }

    uint64_t key() { return id; }
  };

  class NoneVertex : public BaseVertex
  {
  public:
    NoneVertex() {}
  };

  class PurchaseEdge
  {
  public:
    uint64_t buyer;  // vertex id
    uint64_t seller; // vertex id
    uint64_t product;
    time_t date;
    TYPES src_type;
    TYPES dst_type;

    PurchaseEdge()
    {
      buyer = kNullValue<uint64_t>;
      seller = kNullValue<uint64_t>;
      product = kNullValue<uint64_t>;
      date = kNullValue<time_t>;
      src_type = TYPES::NONE;
      dst_type = TYPES::NONE;
    }

    PurchaseEdge(std::vector<std::string> &tokens)
    {
      buyer = encode<uint64_t, std::string, DATA_TYPES::UINT>(tokens[2]);
      seller = encode<uint64_t, std::string, DATA_TYPES::UINT>(tokens[1]);
      product = encode<uint64_t, std::string, DATA_TYPES::UINT>(tokens[6]);
      date = encode<time_t, std::string, DATA_TYPES::USDATE>(tokens[7]);
      src_type = TYPES::PERSON;
      dst_type = TYPES::PERSON;
    }

    uint64_t key() { return buyer; }
    uint64_t src() { return buyer; }
    uint64_t dst() { return seller; }
  };

  class SaleEdge
  {
  public:
    uint64_t seller; // vertex id
    uint64_t buyer;  // vertex id
    uint64_t product;
    time_t date;
    TYPES src_type;
    TYPES dst_type;

    SaleEdge()
    {
      seller = kNullValue<uint64_t>;
      buyer = kNullValue<uint64_t>;
      product = kNullValue<uint64_t>;
      date = kNullValue<time_t>;
      src_type = TYPES::NONE;
      dst_type = TYPES::NONE;
    }

    SaleEdge(std::vector<std::string> &tokens)
    {
      seller = encode<uint64_t, std::string, DATA_TYPES::UINT>(tokens[1]);
      buyer = encode<uint64_t, std::string, DATA_TYPES::UINT>(tokens[2]);
      product = encode<uint64_t, std::string, DATA_TYPES::UINT>(tokens[6]);
      date = encode<time_t, std::string, DATA_TYPES::USDATE>(tokens[7]);
      src_type = TYPES::PERSON;
      dst_type = TYPES::PERSON;
    }

    uint64_t key() { return seller; }
    uint64_t src() { return seller; }
    uint64_t dst() { return buyer; }
  };

  class AuthorEdge
  {
  public:
    uint64_t author; // vertex id
    uint64_t item;   // vertex id
    TYPES src_type;
    TYPES dst_type;

    AuthorEdge()
    {
      author = kNullValue<uint64_t>;
      item = kNullValue<uint64_t>;
      src_type = TYPES::NONE;
      dst_type = TYPES::NONE;
    }

    AuthorEdge(std::vector<std::string> &tokens)
    {
      if (tokens[4] != "")
      {
        author = encode<uint64_t, std::string, DATA_TYPES::UINT>(tokens[1]);
        item = encode<uint64_t, std::string, DATA_TYPES::UINT>(tokens[4]);
        src_type = TYPES::PERSON;
        dst_type = TYPES::FORUMEVENT;
      }
      else
      {
        author = encode<uint64_t, std::string, DATA_TYPES::UINT>(tokens[1]);
        item = encode<uint64_t, std::string, DATA_TYPES::UINT>(tokens[5]);
        src_type = TYPES::PERSON;
        dst_type = TYPES::PUBLICATION;
      }
    }

    uint64_t key() { return author; }
    uint64_t src() { return author; }
    uint64_t dst() { return item; }
  };

  class IncludesEdge
  {
  public:
    uint64_t forum;       // vertex id
    uint64_t forum_event; // vertex id
    TYPES src_type;
    TYPES dst_type;

    IncludesEdge()
    {
      forum = kNullValue<uint64_t>;
      forum_event = kNullValue<uint64_t>;
      src_type = TYPES::NONE;
      dst_type = TYPES::NONE;
    }

    IncludesEdge(std::vector<std::string> &tokens)
    {
      forum = encode<uint64_t, std::string, DATA_TYPES::UINT>(tokens[3]);
      forum_event = encode<uint64_t, std::string, DATA_TYPES::UINT>(tokens[4]);
      src_type = TYPES::FORUM;
      dst_type = TYPES::FORUMEVENT;
    }

    uint64_t key() { return forum; }
    uint64_t src() { return forum; }
    uint64_t dst() { return forum_event; }
  };

  class HasTopicEdge
  {
  public:
    uint64_t item;  // vertex id
    uint64_t topic; // vertex id
    TYPES src_type;
    TYPES dst_type;

    HasTopicEdge()
    {
      item = kNullValue<uint64_t>;
      topic = kNullValue<uint64_t>;
      src_type = TYPES::NONE;
      dst_type = TYPES::NONE;
    }

    HasTopicEdge(std::vector<std::string> &tokens)
    {
      if (tokens[3] != "")
      {
        item = encode<uint64_t, std::string, DATA_TYPES::UINT>(tokens[3]);
        topic = encode<uint64_t, std::string, DATA_TYPES::UINT>(tokens[6]);
        src_type = TYPES::FORUM;
        dst_type = TYPES::TOPIC;
      }
      else if (tokens[4] != "")
      {
        item = encode<uint64_t, std::string, DATA_TYPES::UINT>(tokens[4]);
        topic = encode<uint64_t, std::string, DATA_TYPES::UINT>(tokens[6]);
        src_type = TYPES::FORUMEVENT;
        dst_type = TYPES::TOPIC;
      }
      else
      {
        item = encode<uint64_t, std::string, DATA_TYPES::UINT>(tokens[5]);
        topic = encode<uint64_t, std::string, DATA_TYPES::UINT>(tokens[6]);
        src_type = TYPES::PUBLICATION;
        dst_type = TYPES::TOPIC;
      }
    }

    uint64_t key() { return item; }
    uint64_t src() { return item; }
    uint64_t dst() { return topic; }
  };

  class HasOrgEdge
  {
  public:
    uint64_t publication;  // vertex id
    uint64_t organization; // vertex id
    TYPES src_type;
    TYPES dst_type;

    HasOrgEdge()
    {
      publication = kNullValue<uint64_t>;
      organization = kNullValue<uint64_t>;
      src_type = TYPES::NONE;
      dst_type = TYPES::NONE;
    }

    HasOrgEdge(std::vector<std::string> &tokens)
    {
      publication = encode<uint64_t, std::string, DATA_TYPES::UINT>(tokens[5]);
      organization = encode<uint64_t, std::string, DATA_TYPES::UINT>(tokens[6]);
      src_type = TYPES::PUBLICATION;
      dst_type = TYPES::TOPIC;
    }

    uint64_t key() { return publication; }
    uint64_t src() { return publication; }
    uint64_t dst() { return organization; }
  };

  class NoneEdge
  {
  public:
    NoneEdge() {}
  };

  struct Vertex
  {
    union VertexUnion
    {
      PersonVertex person;
      ForumEventVertex forum_event;
      ForumVertex forum;
      PublicationVertex publication;
      TopicVertex topic;
      NoneVertex none;

      VertexUnion() : none(NoneVertex()) {}
      VertexUnion(PersonVertex &p) : person(p) {}
      VertexUnion(ForumEventVertex &fe) : forum_event(fe) {}
      VertexUnion(ForumVertex &f) : forum(f) {}
      VertexUnion(PublicationVertex &pub) : publication(pub) {}
      VertexUnion(TopicVertex &top) : topic(top) {}
    };

    TYPES v_type;
    VertexUnion v;

    Vertex() : v_type(TYPES::NONE) {}
    Vertex(PersonVertex &p) : v_type(TYPES::PERSON), v(p) {}
    Vertex(ForumEventVertex &fe) : v_type(TYPES::FORUMEVENT), v(fe) {}
    Vertex(ForumVertex &f) : v_type(TYPES::FORUM), v(f) {}
    Vertex(PublicationVertex &pub) : v_type(TYPES::PUBLICATION), v(pub) {}
    Vertex(TopicVertex &top) : v_type(TYPES::TOPIC), v(top) {}

    uint64_t id()
    {
      switch (v_type)
      {
      case TYPES::PERSON:
        return v.person.key();
      case TYPES::FORUMEVENT:
        return v.forum_event.key();
      case TYPES::FORUM:
        return v.forum.key();
      case TYPES::PUBLICATION:
        return v.publication.key();
      case TYPES::TOPIC:
        return v.topic.key();
      default:
        return kNullValue<uint64_t>;
      }
    }

    void setID(u_int64_t id)
    {
      switch (v_type)
      {
      case TYPES::PERSON:
        v.person.setID(id);
      case TYPES::FORUMEVENT:
        v.forum_event.setID(id);
      case TYPES::FORUM:
        v.forum.setID(id);
      case TYPES::PUBLICATION:
        v.publication.setID(id);
      case TYPES::TOPIC:
        v.topic.setID(id);
      default:
        return;
      }
    }

    void setVertexType(TYPES ty)
    {
      this->v_type = ty;
    }
  };

  template <typename T>
  T get_node_data(const Vertex &ver) { return T(); }

  template <>
  PersonVertex get_node_data(const Vertex &ver) { return ver.v.person; }

  template <>
  ForumVertex get_node_data(const Vertex &ver) { return ver.v.forum; }

  template <>
  ForumEventVertex get_node_data(const Vertex &ver) { return ver.v.forum_event; }

  template <>
  PublicationVertex get_node_data(const Vertex &ver) { return ver.v.publication; }

  template <>
  TopicVertex get_node_data(const Vertex &ver) { return ver.v.topic; }

  struct Edge
  {
    union EdgeUnion
    {
      PurchaseEdge purchase;
      SaleEdge sale;
      AuthorEdge author;
      IncludesEdge includes;
      HasTopicEdge has_topic;
      HasOrgEdge has_org;
      NoneEdge none;

      EdgeUnion() : none(NoneEdge()) {}
      EdgeUnion(PurchaseEdge &p) : purchase(p) {}
      EdgeUnion(SaleEdge &s) : sale(s) {}
      EdgeUnion(AuthorEdge &a) : author(a) {}
      EdgeUnion(IncludesEdge &inc) : includes(inc) {}
      EdgeUnion(HasTopicEdge &top) : has_topic(top) {}
      EdgeUnion(HasOrgEdge &org) : has_org(org) {}
    };

    TYPES e_type;
    EdgeUnion e;

    Edge() : e_type(TYPES::NONE) {}
    Edge(int i) : e_type(TYPES::NONE) {}
    Edge(PurchaseEdge &p) : e_type(TYPES::PURCHASE), e(p) {}
    Edge(SaleEdge &s) : e_type(TYPES::SALE), e(s) {}
    Edge(AuthorEdge &a) : e_type(TYPES::AUTHOR), e(a) {}
    Edge(IncludesEdge &pub) : e_type(TYPES::INCLUDES), e(pub) {}
    Edge(HasTopicEdge &top) : e_type(TYPES::HASTOPIC), e(top) {}
    Edge(HasOrgEdge &org) : e_type(TYPES::HASORG), e(org) {}

    uint64_t id()
    {
      switch (e_type)
      {
      case TYPES::PURCHASE:
        return e.purchase.key();
      case TYPES::SALE:
        return e.sale.key();
      case TYPES::AUTHOR:
        return e.author.key();
      case TYPES::INCLUDES:
        return e.includes.key();
      case TYPES::HASTOPIC:
        return e.has_topic.key();
      case TYPES::HASORG:
        return e.has_org.key();
      default:
        return kNullValue<uint64_t>;
      }
    }

    uint64_t src()
    {
      switch (e_type)
      {
      case TYPES::PURCHASE:
        return e.purchase.src();
      case TYPES::SALE:
        return e.sale.src();
      case TYPES::AUTHOR:
        return e.author.src();
      case TYPES::INCLUDES:
        return e.includes.src();
      case TYPES::HASTOPIC:
        return e.has_topic.src();
      case TYPES::HASORG:
        return e.has_org.src();
      default:
        return kNullValue<uint64_t>;
      }
    }

    uint64_t dst()
    {
      switch (e_type)
      {
      case TYPES::PURCHASE:
        return e.purchase.dst();
      case TYPES::SALE:
        return e.sale.dst();
      case TYPES::AUTHOR:
        return e.author.dst();
      case TYPES::INCLUDES:
        return e.includes.dst();
      case TYPES::HASTOPIC:
        return e.has_topic.dst();
      case TYPES::HASORG:
        return e.has_org.dst();
      default:
        return kNullValue<uint64_t>;
      }
    }
  };

  template <typename T>
  T get_edge_data(const Edge &edge) { return T(); }

  template <>
  PurchaseEdge get_edge_data(const Edge &edge) { return edge.e.purchase; }

  template <>
  SaleEdge get_edge_data(const Edge &edge) { return edge.e.sale; }

  template <>
  AuthorEdge get_edge_data(const Edge &edge) { return edge.e.author; }

  template <>
  IncludesEdge get_edge_data(const Edge &edge) { return edge.e.includes; }

  template <>
  HasTopicEdge get_edge_data(const Edge &edge) { return edge.e.has_topic; }

  template <>
  HasOrgEdge get_edge_data(const Edge &edge) { return edge.e.has_org; }

  using LC_CSR_Graph = galois::graphs::LC_CSR_64_Graph<Vertex, Edge>::with_no_lockable<true>::type;
  using LS_LC_CSR_Graph = galois::graphs::LS_LC_CSR_64_Graph<Vertex, Edge>::with_no_lockable<true>::type;

  /* TODO make this edgetype again*/
  // using Dist_Graph = galois::graphs::MiningGraph<Vertex, Edge, MiningPolicyDegrees>;
  // using Dist_Graph = galois::graphs::NewDistGraphGeneric<Vertex, Edge, NoCommunication>;
  using Dist_Graph = galois::graphs::NewDistGraphGeneric<Vertex, void, NoCommunication>;

  template <typename Graph>
  class WF2_Graph
  {
    Graph *g;
    std::unordered_map<uint64_t, uint64_t> id_to_node_index;

    bool is_node(TYPES elem_t)
    {
      switch (elem_t)
      {
      case TYPES::PERSON:
      case TYPES::FORUM:
      case TYPES::FORUMEVENT:
      case TYPES::PUBLICATION:
      case TYPES::TOPIC:
        return true;
      default:
        return false;
      }
    }

    template <typename GraphElemType>
    TYPES graph_type_enum()
    {
      if (std::is_same<GraphElemType, PersonVertex>::value)
      {
        return TYPES::PERSON;
      }
      else if (std::is_same<GraphElemType, ForumVertex>::value)
      {
        return TYPES::FORUM;
      }
      else if (std::is_same<GraphElemType, ForumEventVertex>::value)
      {
        return TYPES::FORUMEVENT;
      }
      else if (std::is_same<GraphElemType, PublicationVertex>::value)
      {
        return TYPES::PUBLICATION;
      }
      else if (std::is_same<GraphElemType, TopicVertex>::value)
      {
        return TYPES::TOPIC;
      }
      else if (std::is_same<GraphElemType, PurchaseEdge>::value)
      {
        return TYPES::PURCHASE;
      }
      else if (std::is_same<GraphElemType, SaleEdge>::value)
      {
        return TYPES::SALE;
      }
      else if (std::is_same<GraphElemType, AuthorEdge>::value)
      {
        return TYPES::AUTHOR;
      }
      else if (std::is_same<GraphElemType, IncludesEdge>::value)
      {
        return TYPES::INCLUDES;
      }
      else if (std::is_same<GraphElemType, HasTopicEdge>::value)
      {
        return TYPES::HASTOPIC;
      }
      else if (std::is_same<GraphElemType, HasOrgEdge>::value)
      {
        return TYPES::HASORG;
      }
      else
      {
        return TYPES::NONE;
      }
    }

  public:
    WF2_Graph(Graph *g, std::unordered_map<uint64_t, uint64_t> &id_to_node_index) : g(g)
    {
      this->id_to_node_index.swap(id_to_node_index);
    }

    template <typename NodeType>
    bool lookup_node(uint64_t id, NodeType &node)
    {
      if (id_to_node_index.find(id) != id_to_node_index.end())
      {
        auto node_data = g->getData(id_to_node_index[id]);
        node = get_node_data<NodeType>(node_data);
        return true;
      }

      return false;
    }

    template <typename EdgeType>
    EdgeType lookup_edge(uint64_t src_id, uint64_t dst_id)
    {
      using GNode = typename Graph::GraphNode;
      EdgeType edge_data;

      galois::do_all(
          galois::iterate(*g),
          [&](GNode n)
          {
            for (auto e : g->edges(n))
            {
              auto &e_data = g->getEdgeData(e);
              if (e_data.src() == src_id && e_data.dst() == dst_id)
              {
                edge_data = get_edge_data<EdgeType>(e_data);
              }
            }
          });

      return edge_data;
    }

    template <typename GraphElemType, typename Op>
    void do_all(TYPES ty, Op f)
    {
      // execute f on each vertex/edge of type GraphElemType
      using GNode = typename Graph::GraphNode;
      if (is_node(ty))
      {
        galois::do_all(
            galois::iterate(*g),
            [&](GNode n)
            {
              auto &n_data = g->getData(n);
              if (n_data.v_type == ty)
              {
                f(get_node_data<GraphElemType>(n_data));
              }
            });
      }
      else
      {
        galois::do_all(
            galois::iterate(*g),
            [&](GNode n)
            {
              for (auto e : g->edges(n))
              {
                auto &e_data = g->getEdgeData(e);
                if (e_data.e_type == ty)
                {
                  f(get_edge_data<GraphElemType>(e_data));
                }
              }
            });
      }
    }

    template <typename NodeType, typename EdgeType, typename Op>
    void iter_edges(TYPES node_ty, TYPES edge_ty, Op f)
    {
      // call f on each vertex and its list of out-edges with type EdgeType
      using GNode = typename Graph::GraphNode;
      galois::do_all(
          galois::iterate(*g),
          [&](GNode n)
          {
            auto &n_data = g->getData(n);
            if (n_data.v_type != node_ty)
            {
              return;
            }

            std::vector<EdgeType> vec;
            for (auto e : g->edges(n))
            {
              auto &e_data = g->getEdgeData(e);
              if (e_data.e_type == edge_ty)
              {
                vec.push_back(get_edge_data<EdgeType>(e_data));
              }
            }

            f(get_node_data<NodeType>(n_data), vec);
          });
    }

    template <typename EdgeType, typename Op>
    void iter_edges(TYPES edge_ty, Op f)
    {
      // call f on each vertex and its list of out-edges with type EdgeType
      using GNode = typename Graph::GraphNode;
      galois::do_all(
          galois::iterate(*g),
          [&](GNode n)
          {
            auto &n_data = g->getData(n);
            std::vector<EdgeType> vec;
            for (auto e : g->edges(n))
            {
              auto &e_data = g->getEdgeData(e);
              if (e_data.e_type == edge_ty)
              {
                vec.push_back(get_edge_data<EdgeType>(e_data));
              }
            }

            f(n_data.id(), vec);
          });
    }

    template <typename NodeType, typename EdgeType, typename Op>
    void iter_edges(uint64_t id, TYPES node_ty, TYPES edge_ty, Op f)
    {
      // call f on list of out-edges with type EdgeType of vertex with given id
      NodeType node;
      std::vector<EdgeType> edges;
      if (id_to_node_index.find(id) != id_to_node_index.end())
      {
        uint64_t n = id_to_node_index[id];
        auto node_data = g->getData(n);
        if (node_data.v_type != node_ty)
          return;

        node = get_node_data<NodeType>(node_data);
        for (auto e : g->edges(n))
        {
          auto &edge_data = g->getEdgeData(e);
          if (edge_data.e_type == edge_ty)
          {
            edges.push_back(get_edge_data<EdgeType>(edge_data));
          }
        }
      }

      f(node, edges);
    }
  };

} // namespace wf2

#endif
