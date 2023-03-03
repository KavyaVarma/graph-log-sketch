#ifndef _WF2_GRAPH_HPP_
#define _WF2_GRAPH_HPP_

#include <limits>

namespace wk2_exact {

enum class DATA_TYPES {
  UINT,
  USDATE
};

template <>
constexpr uint64_t kNullValue<uint64_t> = std::numeric_limits<int64_t>::max();

template<> inline
uint64_t encode<uint64_t, std::string, UINT>(std::string &str) {
  uint64_t value;
  try { value = std::stoull(str); }
  catch(...) { value = kNullValue<uint64_t>; }
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

}

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
};

struct Edge {
  TYPES e_type;
  union EdgeUnion {
    PurchaseEdge purchase;
    SaleEdge sale;
    AuthorEdge author;
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
};

using LC_CSR_64_GRAPH = galois::graphs::LC_CSR_64_Graph<Vertex, Edge>::with_no_lockable<true>::type;

#endif