#ifndef _PATTERN_HPP_
#define _PATTERN_HPP_

#include "galois/Galois.h"
#include "galois/FlatMap.h"
#include "wf2_graph.hpp"
#include <algorithm>
#include <iostream>
#include <vector>
#include <set>

namespace wf2 {

bool proximity(const TopicVertex& A, const TopicVertex& B) {
  double lon_miles = 0.91 * std::abs(A.lon - B.lon);
  double lat_miles = 1.15 * std::abs(A.lat - B.lat);
  double distance = std::sqrt( lon_miles * lon_miles + lat_miles * lat_miles );
  return distance <= 30.0;
}

template<typename Graph>
bool forum_2_subpattern(time_t trans_date, uint64_t forum_event, 
    const galois::flat_map<uint64_t, time_t>& forums2, WF2_Graph<Graph>& g) {
  ForumEventVertex fev;
  bool found = g.template lookup_node<ForumEventVertex>(forum_event, fev);

  if (!found || forums2.find(fev.forum) == forums2.end()) return false;
  time_t forum_date = forums2.at(fev.forum);
  return (trans_date > forum_date);
}

template<typename Graph>
bool forum_1_subpattern(uint64_t forum_event, std::set<uint64_t>& jihad_forums, WF2_Graph<Graph>& g) {
  bool jihad_topic = false;
  g.template iter_edges<ForumEventVertex, HasTopicEdge>(
    forum_event,
    TYPES::FORUMEVENT,
    TYPES::HASTOPIC,
    [&] (ForumEventVertex fev, const std::vector<HasTopicEdge>& topics) {
      for (const auto& t : topics) {
        // topic is jihad
        if (t.topic == 44311) {
          jihad_topic = true;
          return;
        }
      }
    });

  if (!jihad_topic) return false;

  ForumEventVertex fev;
  bool found = g.template lookup_node<ForumEventVertex>(forum_event, fev);
  if (!found) return false;

  bool nyc_topic = false;
  g.template iter_edges<ForumVertex, HasTopicEdge>(
    fev.forum,
    TYPES::FORUM,
    TYPES::HASTOPIC,
    [&] (ForumVertex fv, const std::vector<HasTopicEdge>& topics) {
      for (const auto& t : topics) {
        // topic is NYC
        if (t.topic == 60) {
          nyc_topic = true;
          return;
        }
      }
    });

  if (nyc_topic) {
    auto insert = jihad_forums.insert(fev.forum);
    return !insert.second;
  }

  return false;
}

template<typename Graph>
bool forumEvent_subpattern(const std::vector<AuthorEdge>& events, time_t trans_date, 
    const galois::flat_map<uint64_t, time_t>& forums2, WF2_Graph<Graph>& g) {
  std::set<uint64_t> jihad_forums;
  bool forum_1 = false, forum_2 = false;

  for (const auto& ev : events) {
    // ev.item must be forum event authored by ev.author
    if (ev.dst_type != TYPES::FORUMEVENT) continue;

    if (!forum_1) forum_1 = forum_1_subpattern(ev.item, jihad_forums, g);
    if (!forum_2) forum_2 = forum_2_subpattern(trans_date, ev.item, forums2, g);

    if (forum_1 && forum_2) return true;
  }

  return false;
}

template<typename Graph>
bool electronics_subpattern(uint64_t seller_id, WF2_Graph<Graph>& g) {
  bool match = false;
  g.template iter_edges<PersonVertex, AuthorEdge>(
    seller_id,
    TYPES::PERSON,
    TYPES::AUTHOR,
    [&] (PersonVertex seller, const std::vector<AuthorEdge>& pubs) {
      for (const auto& p : pubs) {
        if (p.dst_type != TYPES::PUBLICATION) continue;
        
        bool contains_topic = false;
        g.template iter_edges<PublicationVertex, HasTopicEdge>(
          p.item,
          TYPES::PUBLICATION,
          TYPES::HASTOPIC,
          [&] (PublicationVertex pub, const std::vector<HasTopicEdge>& topics) {
            for (const auto& t : topics) {
              // topic is electrical engineering
              if (t.topic == 43035) {
                contains_topic = true;
                return;
              }
            }
          });
        
        if (!contains_topic) continue;

        g.template iter_edges<PublicationVertex, HasOrgEdge>(
          p.item,
          TYPES::PUBLICATION,
          TYPES::HASORG,
          [&] (PublicationVertex pub, const std::vector<HasOrgEdge>& orgs) {
            TopicVertex NYC;
            bool found = g.template lookup_node(60, NYC);
            if (!found) {
              std::cout << "failed to find NYC topic vertex" << std::endl;
              return;
            }

            for (const auto& o : orgs) {
              TopicVertex org_topic;
              found = g.template lookup_node<TopicVertex>(o.organization, org_topic);
              if (found && proximity(org_topic, NYC)) {
                match = true;
                return;
              } 
            }
          });

        if (match) return;
      }
    });

  return match;
}

template<typename Graph>
bool ammunition_subpattern(uint64_t buyer_id, uint64_t seller_id, WF2_Graph<Graph>& g) {
  bool match = false;
  g.template iter_edges<PersonVertex, SaleEdge>(
    seller_id,
    TYPES::PERSON,
    TYPES::SALE,
    [&] (PersonVertex seller, const std::vector<SaleEdge>& sales) {
      for (const auto& sale : sales) {
        // sale of ammunition to person who is not buyer
        if (sale.product == 185785 && sale.buyer != buyer_id) {
          match = true; 
          return;
        }
      }
    });
  return match;
}

template<typename Graph>
time_t transEvents(const std::vector<PurchaseEdge>& purchases, WF2_Graph<Graph>& g) {
  bool ESP = false;
  time_t latest_BB = 0, latest_PC = 0, latest_AMO = 0;

  for (const auto& p : purchases) {
    if (p.product == 2869238) {
      // bath bomb
      latest_BB = std::max(latest_BB, p.date);
    } else if (p.product == 271997) {
      // pressure cooker
      latest_PC = std::max(latest_PC, p.date);
    } else if (p.product == 185785) {
      // ammunition
      if (p.date > latest_AMO && ammunition_subpattern(p.buyer, p.seller, g)) { 
        latest_AMO = p.date;
      }
    } else if (p.product == 11650) {
      // electronics
      if (!ESP) ESP = electronics_subpattern(p.seller, g);
    }
  }

  time_t trans_date = std::min(std::min(latest_BB, latest_PC), latest_AMO); // list params?
  if (trans_date == 0 || !ESP) return 0;

  return trans_date;
}

bool forumPattern_2B(const std::vector<HasTopicEdge>& topics) {
  bool topic_1 = false;
  bool topic_2 = false;
  bool topic_3 = false;

  for (const auto& t : topics) { 
    if (t.topic == 771572) {
      // Williamsburg
      topic_1 = true;
    } else if (t.topic == 179057) {
      // Explosion
      topic_2 = true;
    } else if (t.topic == 127197) {
      // Bomb
      topic_3 = true;
    }
  }

  return topic_1 && topic_2 && topic_3;
}

bool forumPattern_2A(const std::vector<HasTopicEdge>& topics) {
  bool topic_1 = false;
  bool topic_2 = false;

  for (const auto& t : topics) { 
    if (t.topic == 69871376) {
      // Outdoors
      topic_1 = true;
    } else if (t.topic == 1049632) {
      // Prospect Park
      topic_2 = true;
    }
  }

  return topic_1 && topic_2;
}

void insert_min_time(galois::flat_map<uint64_t, time_t>& forums, uint64_t forum, time_t time) {
  if (forums.find(forum) != forums.end()) {
    forums[forum] = std::min(time, forums[forum]);
  } else {
    forums[forum] = time;
  }
}

template<typename Graph>
void WMD_pattern(WF2_Graph<Graph>& g) {
  galois::flat_map<uint64_t, time_t> forums2;

  // forumPattern_2A and forumPattern_2B
  g.template iter_edges<ForumEventVertex, HasTopicEdge>(
    TYPES::FORUMEVENT,
    TYPES::HASTOPIC, 
    [&] (ForumEventVertex fev, const std::vector<HasTopicEdge>& topics) {
      if (forumPattern_2A(topics)) {
        forums2[fev.forum] = kNullValue<time_t>;
      }
    });

  g.template iter_edges<ForumEventVertex, HasTopicEdge>(
    TYPES::FORUMEVENT,
    TYPES::HASTOPIC,
    [&] (ForumEventVertex fev, const std::vector<HasTopicEdge>& topics) {
      if (forums2.find(fev.forum) != forums2.end() && forumPattern_2B(topics)) {
        insert_min_time(forums2, fev.forum, fev.date);
      }
    });

  // trans dates
  galois::flat_map<uint64_t, time_t> persons;
  g.template iter_edges<PersonVertex, PurchaseEdge>(
    TYPES::PERSON,
    TYPES::PURCHASE,
    [&] (PersonVertex person, const std::vector<PurchaseEdge>& purchases) {
      time_t trans_date = transEvents(purchases, g);
      if (trans_date == 0) return;

      persons[person.id] = trans_date;
    });

  // forumEvent_subpattern
  galois::do_all(
    galois::iterate(persons),
    [&] (auto kv) {
      uint64_t person_id = kv.first;
      time_t trans_date = kv.second;
      g.template iter_edges<PersonVertex, AuthorEdge>(
        person_id,
        TYPES::PERSON,
        TYPES::AUTHOR,
        [&] (PersonVertex person, const std::vector<AuthorEdge>& events) {
          if (forumEvent_subpattern(events, trans_date, forums2, g)) {
            std::cout << "Found person matching pattern! " << person.id << std::endl;
          }
        });
    });
}

} // namespace wf2

#endif
