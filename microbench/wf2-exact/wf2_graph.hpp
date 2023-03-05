#ifndef _WF2_GRAPH_HPP_
#define _WF2_GRAPH_HPP_

#include "galois/graphs/LC_CSR_64_Graph.h"
#include <ctime>
#include <limits>
#include <string>

namespace wf2 {

enum class DATA_TYPES {
  UINT,
  DOUBLE,
  USDATE
};

template <>
const uint64_t kNullValue<uint64_t> = std::numeric_limits<int64_t>::max();

template <>
const time_t kNullValue<time_t> = std::numeric_limits<time_t>::max();

template <>
const double kNullValue<double> = std::numeric_limits<double>::max();

template<> inline
uint64_t encode<uint64_t, std::string, UINT>(std::string &str) {
  uint64_t value;
  try { value = std::stoull(str); }
  catch(...) { value = kNullValue<uint64_t>; }
  return value;
}

template<> inline
double encode<double, std::string, DOUBLE>(std::string &str) {
  double value;
  try { value = std::stod(str); }
  catch(...) { value = kNullValue<double>; }
  return value;
}

template<> inline
uint64_t encode<uint64_t, std::string, USDATE>(std::string &str) {
  uint64_t value = 0;
  struct tm date{};
  date.tm_isdst = -1;
  strptime(str.c_str(), "%m/%d/%y", &date);
  time_t t;
  try {
    t = mktime(&date);
  }
  catch(...) {
    return kNullValue<uint64_t>;
  }
  memcpy(&value, &t, sizeof(value));
  return value;
}


enum class TYPES {
  PERSON,
  FORUMEVENT,
  FORUM,
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


class PersonVertex {
public:
  uint64_t id;
  uint64_t glbid;

  PersonVertex () {
    id    = kNullValue<uint64_t>;
    glbid = kNullValue<uint64_t>;
  }

  PersonVertex (std::vector <std::string> & tokens) {
    id    = encode<uint64_t, std::string, UINT>(tokens[1]);
    glbid = kNullValue<uint64_t>;
  }

  uint64_t key() { return id; }
};

class ForumEventVertex {
public:
  uint64_t id;
  uint64_t forum;
  time_t   date;
  uint64_t glbid;

  ForumEventVertex () {
    id    = kNullValue<uint64_t>;
    forum = kNullValue<uint64_t>;
    date  = kNullValue<time_t>;
    glbid = kNullValue<uint64_t>;
  }

  ForumEventVertex (std::vector <std::string> & tokens) {
    id    = encode<uint64_t, std::string, UINT>  (tokens[4]);
    forum = encode<uint64_t, std::string, UINT>  (tokens[3]);
    date  = encode<time_t, std::string, USDATE>(tokens[7]);
    glbid = kNullValue<uint64_t>;
  }

  uint64_t key() { return id; }
};

class ForumVertex {
public:
  uint64_t id;
  uint64_t glbid; 

  ForumVertex () {
    id    = kNullValue<uint64_t>;
    glbid = kNullValue<uint64_t>;
  }

  ForumVertex (std::vector <std::string> & tokens) {
    id   = encode<uint64_t, std::string, UINT>(tokens[3]);
    glbid = kNullValue<uint64_t>;
  }

  uint64_t key() { return id; }
};

class PublicationVertex {
public:
  uint64_t id;
  time_t   date;
  uint64_t glbid;

  PublicationVertex () {
    id    = kNullValue<uint64_t>;
    date  = kNullValue<time_t>;
    glbid = kNullValue<uint64_t>;
  }

  PublicationVertex (std::vector <std::string> & tokens) {
    id    = encode<uint64_t, std::string, UINT>(tokens[5]);
    date  = encode<time_t, std::string, USDATE>(tokens[7]);
    glbid = kNullValue<uint64_t>;
  }

  uint64_t key() { return id; }
};

class TopicVertex {
public:
  uint64_t id;
  double   lat;
  double   lon;
  uint64_t glbid;

  TopicVertex () {
    id    = kNullValue<uint64_t>;
    lat   = kNullValue<double>;
    lon   = kNullValue<double>;
    glbid = kNullValue<uint64_t>;
  }

  TopicVertex (std::vector <std::string> & tokens) {
    id    = encode<uint64_t, std::string, UINT>(tokens[6]);
    lat   = encode<double, std::string, DOUBLE>(tokens[8]);
    lon   = encode<double, std::string, DOUBLE>(tokens[9]);
    glbid = kNullValue<uint64_t>;
  }

  uint64_t key() { return id; }
};

class NoneVertex {
public:
  NoneVertex () {}
};


class PurchaseEdge {
public:
  uint64_t buyer;            // vertex id
  uint64_t seller;           // vertex id
  uint64_t product;
  time_t   date;
  TYPES    src_type;
  TYPES    dst_type;

  PurchaseEdge () {
    buyer   = kNullValue<uint64_t>;
    seller  = kNullValue<uint64_t>;
    product = kNullValue<uint64_t>;
    date    = kNullValue<time_t>;
    src_type = NONE;
    dst_type = NONE;
  }

  PurchaseEdge (std::vector <std::string> & tokens) {
    buyer    = encode<uint64_t, std::string, UINT>  (tokens[2]);
    seller   = encode<uint64_t, std::string, UINT>  (tokens[1]);
    product  = encode<uint64_t, std::string, UINT>  (tokens[6]);
    date     = encode<time_t,   std::string, USDATE>(tokens[7]);
    src_type = PERSON;
    dst_type = PERSON;
  }

  uint64_t key() { return buyer; }
  uint64_t src() { return buyer; }
  uint64_t dst() { return seller; }
};

class SaleEdge {
public:
  uint64_t seller;           // vertex id
  uint64_t buyer;            // vertex id
  uint64_t product;
  time_t   date;
  TYPES    src_type;
  TYPES    dst_type;

  SaleEdge () {
    seller   = kNullValue<uint64_t>;
    buyer    = kNullValue<uint64_t>;
    product  = kNullValue<uint64_t>;
    date     = kNullValue<time_t>;
    src_type = NONE;
    dst_type = NONE;
  }

  SaleEdge (std::vector <std::string> & tokens) {
    seller   = encode<uint64_t, std::string, UINT>  (tokens[1]);
    buyer    = encode<uint64_t, std::string, UINT>  (tokens[2]);
    product  = encode<uint64_t, std::string, UINT>  (tokens[6]);
    date     = encode<time_t,   std::string, USDATE>(tokens[7]);
    src_type = PERSON;
    dst_type = PERSON;
  }

  uint64_t key() { return seller; }
  uint64_t src() { return seller; }
  uint64_t dst() { return buyer; }
};

class AuthorEdge {
public:
  uint64_t author;     // vertex id
  uint64_t item;       // vertex id
  TYPES    src_type;
  TYPES    dst_type;

  AuthorEdge () {
    author   = kNullValue<uint64_t>;
    item     = kNullValue<uint64_t>;
    src_type = NONE;
    dst_type = NONE;
  }

  AuthorEdge (std::vector <std::string> & tokens) {
    if (tokens[4] != "") {
      author   = encode<uint64_t, std::string, UINT>(tokens[1]);
      item     = encode<uint64_t, std::string, UINT>(tokens[4]);
      src_type = PERSON;
      dst_type = FORUMEVENT;
    } else {
      author   = encode<uint64_t, std::string, UINT>(tokens[1]);
      item     = encode<uint64_t, std::string, UINT>(tokens[5]);
      src_type = PERSON;
      dst_type = PUBLICATION;
    } 
  }

  uint64_t key() { return author; }
  uint64_t src() { return author; }
  uint64_t dst() { return item; }
};

class IncludesEdge {
public:
  uint64_t forum;            // vertex id
  uint64_t forum_event;      // vertex id
  TYPES    src_type;
  TYPES    dst_type;

  IncludesEdge () {
    forum       = kNullValue<uint64_t>;
    forum_event = kNullValue<uint64_t>;
    src_type    = NONE;
    dst_type    = NONE;
  }

  IncludesEdge (std::vector <std::string> & tokens) {
    forum       = encode<uint64_t, std::string, UINT>(tokens[3]);
    forum_event = encode<uint64_t, std::string, UINT>(tokens[4]);
    src_type    = FORUM;
    dst_type    = FORUMEVENT;
  }

  uint64_t key() { return forum; }
  uint64_t src() { return forum; }
  uint64_t dst() { return forum_event; }
};

class HasTopicEdge {
public:
  uint64_t item;      // vertex id
  uint64_t topic;     // vertex id
  TYPES    src_type;
  TYPES    dst_type;

  HasTopicEdge () {
    item     = kNullValue<uint64_t>;
    topic    = kNullValue<uint64_t>;
    src_type = NONE;
    dst_type = NONE;
  }

  HasTopicEdge (std::vector <std::string> & tokens) {
    if (tokens[3] != "") {
      item     = encode<uint64_t, std::string, UINT>(tokens[3]);
      topic    = encode<uint64_t, std::string, UINT>(tokens[6]);
      src_type = FORUM;
      dst_type = TOPIC;
    } else if (tokens[4] != "") {
      item     = encode<uint64_t, std::string, UINT>(tokens[4]);
      topic    = encode<uint64_t, std::string, UINT>(tokens[6]);
      src_type = FORUMEVENT;
      dst_type = TOPIC;
    } else {
      item     = encode<uint64_t, std::string, UINT>(tokens[5]);
      topic    = encode<uint64_t, std::string, UINT>(tokens[6]);
      src_type = PUBLICATION;
      dst_type = TOPIC;
    } 
  }

  uint64_t key() { return item; }
  uint64_t src() { return item; }
  uint64_t dst() { return topic; }
};

class HasOrgEdge {
public:
  uint64_t publication;      // vertex id
  uint64_t organization;     // vertex id
  TYPES    src_type;
  TYPES    dst_type;

  HasOrgEdge () {
    publication  = kNullValue<uint64_t>;
    organization = kNullValue<uint64_t>;
    src_type     = NONE;
    dst_type     = NONE;
  }

  HasOrgEdge (std::vector <std::string> & tokens) {
    publication  = encode<uint64_t, std::string, UINT>(tokens[5]);
    organization = encode<uint64_t, std::string, UINT>(tokens[6]);
    src_type     = PUBLICATION;
    dst_type     = TOPIC;
  }

  uint64_t key() { return publication; }
  uint64_t src() { return publication; }
  uint64_t dst() { return organization; }
};

class NoneEdge {
public:
  NoneEdge () {}
};


struct Vertex {
  TYPES v_type;
  union VertexUnion {
    PersonVertex person;
    ForumEventVertex forum_event;
    ForumVertex forum;
    PublicationVertex publication;
    TopicVertex topic;
    NoneVertex none;
  } v;

  Vertex () {
    v_type = NONE;
    v = NoneVertex();
  }

  Vertex (PersonVertex& person) {
    v_type = PERSON;
    v = person;
  }

  Vertex (ForumEventVertex& forum_event) {
    v_type = FORUMEVENT;
    v = forum_event;
  }

  Vertex (ForumVertex& forum) {
    v_type = FORUM;
    v = forum;
  }

  Vertex (PublicationVertex& publication) {
    v_type = PUBLICATION;
    v = publication;
  }

  Vertex (TopicVertex& topic) {
    v_type = TOPIC;
    v = topic;
  }

  uint64_t id() {
    switch (v_type) {
    case PERSON:
      return v.person.key();
    case FORUMEVENT:
      return v.forum_event.key();
    case FORUM:
      return v.forum.key();
    case PUBLICATION:
      return v.publication.key();
    case TOPIC:
      return v.topic.key();
    case NONE:
      return kNullValue<uint64_t>;
    }
  }
};

struct Edge {
  TYPES e_type;
  union EdgeUnion {
    PurchaseEdge purchase;
    SaleEdge sale;
    AuthorEdge author;
    IncludesEdge includes;
    HasTopicEdge has_topic;
    HasOrgEdge has_org;
    NoneEdge none;
  } e;

  Edge () {
    e_type = NONE;
    e = NoneVertex();
  }

  Edge (PurchaseEdge& purchase) {
    e_type = PURCHASE;
    e = purchase;
  }

  Edge (SaleEdge& sale) {
    e_type = SALE;
    e = sale;
  }

  Edge (AuthorEdge& author) {
    e_type = AUTHOR;
    e = author;
  }

  Edge (IncludesEdge& includes) {
    e_type = INCLUDES;
    e = includes;
  }

  Edge (HasTopicEdge& has_topic) {
    e_type = HASTOPIC;
    e = has_topic;
  }

  Edge (HasOrgEdge& has_org) {
    e_type = HASORG;
    e = has_org;
  }

  uint64_t id() {
    switch (e_type) {
    case PURCHASE:
      return e.purchase.key();
    case SALE:
      return e.sale.key();
    case AUTHOR:
      return e.author.key();
    case INCLUDES:
      return e.includes.key();
    case HASTOPIC:
      return e.has_topic.key();
    case HASORG:
      return e.has_org.key();
    case NONE:
      return kNullValue<uint64_t>;
    }
  }

  uint64_t src() {
    switch (e_type) {
    case PURCHASE:
      return e.purchase.src();
    case SALE:
      return e.sale.src();
    case AUTHOR:
      return e.author.src();
    case INCLUDES:
      return e.includes.src();
    case HASTOPIC:
      return e.has_topic.src();
    case HASORG:
      return e.has_org.src();
    case NONE:
      return kNullValue<uint64_t>;
    }
  }

  uint64_t dst() {
    switch (e_type) {
    case PURCHASE:
      return e.purchase.dst();
    case SALE:
      return e.sale.dst();
    case AUTHOR:
      return e.author.dst();
    case INCLUDES:
      return e.includes.dst();
    case HASTOPIC:
      return e.has_topic.dst();
    case HASORG:
      return e.has_org.dst();
    case NONE:
      return kNullValue<uint64_t>;
    }
  }
};

using Graph = galois::graphs::LC_CSR_64_Graph<Vertex, Edge>::with_no_lockable<true>::type;

}

#endif
