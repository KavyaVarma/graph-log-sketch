// ****************************************************
//                   dist_engine.h
// ****************************************************
#pragma once
#include "wf2_graph.hpp"
#include "read_file.hpp"
#include "timer.hpp"
#include "pattern.hpp"
#include "DistBench/Input.h"
#include "DistBench/Start.h"
#include "galois/AtomicHelpers.h"
#include "galois/DReducible.h"
#include "galois/DTerminationDetector.h"
#include "galois/DistGalois.h"
#include "galois/Galois.h"
#include "galois/Version.h"
#include "galois/graphs/GenericPartitioners.h"
#include "galois/graphs/GluonSubstrate.h"
#include "galois/graphs/MiningPartitioner.h"
#include "galois/gstl.h"
#include "galois/runtime/Tracer.h"
#include "galois/substrate/SimpleLock.h"
#include "llvm/Support/CommandLine.h"
#include "pangolin/BfsMining/embedding_list.h"
#include "pangolin/res_man.h"

#include <iostream>

#define DEBUG 0
#define BENCH 1

// Command Line
namespace cll = llvm::cl;

static cll::opt<std::string> graphName("graphName", cll::desc("Name of the input graph"), cll::init("temp"));

typedef galois::graphs::DistGraph<wf2::Vertex, wf2::Edge> Graph;
typedef typename Graph::GraphNode PGNode;
using DGAccumulatorTy = galois::DGAccumulator<uint64_t>;
using GAccumulatorTy = galois::GAccumulator<uint64_t>;
DGAccumulatorTy *global_num_tri_ptr;
GAccumulatorTy *thread_num_tri_ptr;

// ##################################################################
//                      GLOBAL (aka host) VARS
// ##################################################################
galois::DynamicBitSet bitset_dests;
galois::DynamicBitSet bitset_requested_edges;
std::unique_ptr<galois::graphs::GluonSubstrate<Graph>> syncSubstrate;
auto &net = galois::runtime::getSystemNetworkInterface();

std::shared_ptr<galois::substrate::SimpleLock> lock_ptr = std::make_shared<galois::substrate::SimpleLock>();
std::shared_ptr<galois::substrate::SimpleLock> print_lock_ptr = std::make_shared<galois::substrate::SimpleLock>();

/*
    Proximity to NYC Topic
    40.67,-73.94
    TODO : How in the world do I get this as global information?
    Some type of global metadata type thing?
*/
bool proximity_to_nyc(const wf2::TopicVertex &A)
{
    double lon_miles = 0.91 * std::abs(A.lon - (-73.94));
    double lat_miles = 1.15 * std::abs(A.lat - 40.67);
    double distance = std::sqrt(lon_miles * lon_miles + lat_miles * lat_miles);
    return distance <= 30.0;
}

void print_graph(Graph &hg_ref)
{
    print_lock_ptr->lock();
    printf("Host %d / %d:\n", net.ID, net.Num);
    galois::do_all(
        galois::iterate(hg_ref.allNodesWithEdgesRange()), // hg = local subgraph
        [&](const PGNode &node) {                         // Plug current guy in here
            std::stringstream sout;
            sout << "\tSrc: " << node << "; GLOBAL = " << hg_ref.getGID(node) << std::endl;
            sout << "\tDsts: " << std::endl;
            for (auto i = hg_ref.edge_begin(node); i != hg_ref.edge_end(node); i++)
            {
                auto ldst = hg_ref.getEdgeDst(i);
                sout << "\t\tLocal = " << ldst << ", global = " << hg_ref.getGID(ldst) << std::endl;
            }
            std::cout << sout.str() << std::endl;
        },
        galois::steal());
    print_lock_ptr->unlock();
    galois::runtime::getHostBarrier().wait();
}

/*
    Sync the Vertex information from masters to mirrors
*/
struct SyncVertices
{
    typedef wf2::Vertex ValTy;

    static wf2::Vertex extract(uint32_t, struct wf2::Vertex &node)
    {
        wf2::Vertex vec_ver(node);
        return vec_ver;
    }

    static bool reduce(uint32_t, struct wf2::Vertex &node, wf2::Vertex y)
    {
        return false;
    }

    static void reset(uint32_t, struct wf2::Vertex &node)
    {
    }

    static void setVal(uint32_t, struct wf2::Vertex &node, wf2::Vertex y)
    {
        node.setVertexType(y.v_type);
        node.setID(y.id());

        switch (node.v_type)
        {
        case wf2::TYPES::PERSON:
        {
            node.v.person.glbid = y.v.person.glbid;
            break;
        }
        case wf2::TYPES::FORUMEVENT:
        {
            node.v.forum_event.glbid = y.v.forum_event.glbid;
            node.v.forum_event.forum = y.v.forum_event.forum;
            node.v.forum_event.date = y.v.forum_event.date;

            break;
        }
        case wf2::TYPES::FORUM:
        {
            node.v.forum.glbid = y.v.forum.glbid;
            break;
        }
        case wf2::TYPES::PUBLICATION:
        {
            node.v.publication.glbid = y.v.publication.glbid;
            node.v.publication.date = y.v.publication.date;

            break;
        }
        case wf2::TYPES::TOPIC:
        {
            node.v.topic.glbid = y.v.topic.glbid;
            node.v.topic.lat = y.v.topic.lat;
            node.v.topic.lon = y.v.topic.lon;

            break;
        }
        }
    }

    static bool extract_batch(unsigned, uint8_t *, size_t *, DataCommMode *) { return false; }
    static bool extract_batch(unsigned, uint8_t *) { return false; }
    static bool extract_reset_batch(unsigned, uint8_t *, size_t *, DataCommMode *) { return false; }
    static bool extract_reset_batch(unsigned, uint8_t *) { return false; }
    static bool reset_batch(size_t, size_t) { return false; }
    static bool reduce_batch(unsigned, uint8_t *, DataCommMode) { return false; }
    static bool reduce_mirror_batch(unsigned, uint8_t *, DataCommMode) { return false; }
    static bool setVal_batch(unsigned, uint8_t *, DataCommMode) { return false; }
};

/*
    Sync forum 2A 2B Jihad information gathered -> broadcast
    Sync the person ammunition information -> broadcast
    Sync the publication + organization + topic information -> broadcast
    Sync the forum event information on NYC -> reduce + broadcast
*/
struct SyncPhase1
{
    typedef wf2::Vertex ValTy;

    static wf2::Vertex extract(uint32_t, struct wf2::Vertex &node)
    {
        wf2::Vertex vec_ver(node);
        return vec_ver;
    }

    static bool reduce(uint32_t, struct wf2::Vertex &node, wf2::Vertex y)
    {
        if (node.v_type == wf2::TYPES::FORUMEVENT)
        {
            node.v.forum_event.nyc_topic = node.v.forum_event.nyc_topic | y.v.forum_event.nyc_topic;
            node.v.forum_event.jihad_topic = node.v.forum_event.jihad_topic | y.v.forum_event.jihad_topic;
        }
        return true;
    }

    static void reset(uint32_t, struct wf2::Vertex &node)
    {
    }

    static void setVal(uint32_t, struct wf2::Vertex &node, wf2::Vertex y)
    {
        if (node.v_type == wf2::TYPES::FORUMEVENT)
        {
            node.v.forum_event.forum2A = y.v.forum_event.forum2A;
            node.v.forum_event.forum2B = y.v.forum_event.forum2B;
            node.v.forum_event.jihad_topic = y.v.forum_event.jihad_topic;
            node.v.forum_event.nyc_topic = y.v.forum_event.nyc_topic;
        }
        else if (node.v_type == wf2::TYPES::PERSON)
        {
            node.v.person.ammo_sp = y.v.person.ammo_sp;
        }
        else if (node.v_type == wf2::TYPES::PUBLICATION)
        {
            node.v.publication.has_org_near_nyc = y.v.publication.has_org_near_nyc;
            node.v.publication.ee_topic = y.v.publication.ee_topic;
        }
    }

    static bool extract_batch(unsigned, uint8_t *, size_t *, DataCommMode *) { return false; }
    static bool extract_batch(unsigned, uint8_t *) { return false; }
    static bool extract_reset_batch(unsigned, uint8_t *, size_t *, DataCommMode *) { return false; }
    static bool extract_reset_batch(unsigned, uint8_t *) { return false; }
    static bool reset_batch(size_t, size_t) { return false; }
    static bool reduce_batch(unsigned, uint8_t *, DataCommMode) { return false; }
    static bool reduce_mirror_batch(unsigned, uint8_t *, DataCommMode) { return false; }
    static bool setVal_batch(unsigned, uint8_t *, DataCommMode) { return false; }
};

/*
    Sync the min time information -> reduce + broadcast
    Sync the Jihad information to all forum events -> reduce broadcast -> reduce + broadcast
    Sync the person with ee publication -> broadcast
*/
struct SyncPhase2
{
    typedef wf2::Vertex ValTy;

    static wf2::Vertex extract(uint32_t, struct wf2::Vertex &node)
    {
        wf2::Vertex vec_ver(node);
        return vec_ver;
    }

    static bool reduce(uint32_t, struct wf2::Vertex &node, wf2::Vertex y)
    {
        if (node.v_type == wf2::TYPES::FORUMEVENT)
        {
            if (y.v.forum_event.forum_min_time != 0)
            {
                node.v.forum_event.forum_min_time = y.v.forum_event.forum_min_time;
            }
            node.v.forum_event.jihad_topic = node.v.forum_event.jihad_topic | y.v.forum_event.jihad_topic;
            node.v.forum_event.nyc_topic = node.v.forum_event.nyc_topic | y.v.forum_event.nyc_topic;;
        };
        return true;
    }

    static void reset(uint32_t, struct wf2::Vertex &node)
    {
    }

    static void setVal(uint32_t, struct wf2::Vertex &node, wf2::Vertex y)
    {
        if (node.v_type == wf2::TYPES::FORUM)
        {
            node.v.forum.min_time = y.v.forum.min_time;
        }
        else if (node.v_type == wf2::TYPES::PERSON)
        {
            node.v.person.ee_sp = y.v.person.ee_sp;
        }
        else if (node.v_type == wf2::TYPES::FORUMEVENT)
        {
            node.v.forum_event.forum_min_time = y.v.forum_event.forum_min_time;
            node.v.forum_event.jihad_topic = y.v.forum_event.jihad_topic;
            node.v.forum_event.nyc_topic = y.v.forum_event.nyc_topic;
        }
    }

    static bool extract_batch(unsigned, uint8_t *, size_t *, DataCommMode *) { return false; }
    static bool extract_batch(unsigned, uint8_t *) { return false; }
    static bool extract_reset_batch(unsigned, uint8_t *, size_t *, DataCommMode *) { return false; }
    static bool extract_reset_batch(unsigned, uint8_t *) { return false; }
    static bool reset_batch(size_t, size_t) { return false; }
    static bool reduce_batch(unsigned, uint8_t *, DataCommMode) { return false; }
    static bool reduce_mirror_batch(unsigned, uint8_t *, DataCommMode) { return false; }
    static bool setVal_batch(unsigned, uint8_t *, DataCommMode) { return false; }
};

/*
    Main
*/
int main(int argc, char **argv)
{

    double e2e_start = 0;
    double e2e_end = 0;

    double mk_graph_start = 0;
    double mk_graph_end = 0;

    double node_sync_start = 0;
    double node_sync_end = 0;

    double calculate_1_start = 0;
    double calculate_1_end = 0;

    double sync_1_start = 0;
    double sync_1_end = 0;

    double calculate_2_start = 0;
    double calculate_2_end = 0;

    double sync_2_start = 0;
    double sync_2_end = 0;

    double calculate_3_start = 0;
    double calculate_3_end = 0;

    if (BENCH)
        e2e_start = MPI_Wtime();

    // Initialize Galois Runtime
    galois::DistMemSys G;
    DistBenchStart(argc, argv, "name", "desc", "url");

    // Initialize Graph
    if (BENCH)
    {
        galois::runtime::getHostBarrier().wait();
        mk_graph_start = MPI_Wtime();
    }
    std::unique_ptr<Graph> hg;
    std::tie(hg, syncSubstrate) = distGraphInitialization<wf2::Vertex, wf2::Edge>();
    std::unordered_map<uint64_t, uint64_t> id_to_node_index = wf2::read_file<Graph, GNode>(*hg, "../graphs/data.01.csv");

    if (BENCH)
    {
        galois::runtime::getHostBarrier().wait();
        mk_graph_end = MPI_Wtime();
        std::cout << "Time_Graph_Creation, " << mk_graph_end - mk_graph_start << "\n";
    }

    Graph &hg_ref = *hg;

    // TODO Fix this
    uint32_t num_hosts = 2; // hg->numHosts;
    uint64_t host_id = galois::runtime::getSystemNetworkInterface().ID;

    std::ofstream file;
    file.open(graphName + "_" + std::to_string(num_hosts) + "procs_id" + std::to_string(host_id));
    file << "#####   Stat   #####" << std::endl;
    file << "host " << host_id << " total edges: " << hg->sizeEdges() << std::endl;

    /*
        Sync the vertex information
    */
    if (BENCH)
    {
        galois::runtime::getHostBarrier().wait();
        node_sync_start = MPI_Wtime();
    }

    syncSubstrate->sync<writeSource, readDestination, SyncVertices>("SyncNodes");

    if (BENCH)
    {
        galois::runtime::getHostBarrier().wait();
        node_sync_end = MPI_Wtime();
        std::cout << "Sync_Master_Mirror_Data, " << node_sync_end - node_sync_start << "\n";
    }

    /*
        ForumEvent 2A + 2B + Jihad
        Identify Ammunition SubPattern
        Identify publications with an organization that is in close proximity to NYC
            + publications with topic EE
    */

    if (BENCH)
    {
        galois::runtime::getHostBarrier().wait();
        calculate_1_start = MPI_Wtime();
    }

    galois::do_all(
        galois::iterate(hg_ref.allNodesWithEdgesRange()),
        [&](const GNode &gNode)
        {
            auto &node = hg_ref.getData(gNode);

            // Forum 2A + 2B + Jihad topic -> altering master nodes
            if (node.v_type == wf2::TYPES::FORUMEVENT)
            {
                bool topic_2A_1 = false;
                bool topic_2A_2 = false;
                bool topic_2B_1 = false;
                bool topic_2B_2 = false;
                bool topic_2B_3 = false;
                bool jihad_topic = false;
                for (auto e : hg_ref.edges(gNode))
                {
                    auto &edge_node = hg_ref.getData(hg_ref.getEdgeDst(e));
                    auto &edge_data = hg_ref.getEdgeData(e);
                    if (edge_node.v_type == wf2::TYPES::TOPIC)
                    {
                        if (edge_node.id() == 69871376)
                            topic_2A_1 = true;
                        if (edge_node.id() == 1049632)
                            topic_2A_2 = true;
                        if (edge_node.id() == 771572)
                            topic_2B_1 = true;
                        if (edge_node.id() == 179057)
                            topic_2B_2 = true;
                        if (edge_node.id() == 127197)
                            topic_2B_3 = true;
                        if (edge_node.id() == 44311) {
                            jihad_topic = true;
                        }
                    }
                }
                if (topic_2A_1 && topic_2A_2)
                {
                    node.v.forum_event.forum2A = true;
                }
                if (topic_2B_1 && topic_2B_2 && topic_2B_3)
                {
                    node.v.forum_event.forum2B = true;
                }
                node.v.forum_event.jihad_topic = jihad_topic;
            }

            // Ammunition Subpattern -> altering master nodes
            if (node.v_type == wf2::TYPES::PERSON)
            {
                std::set<uint64_t> buyers;
                for (auto e : hg_ref.edges(gNode))
                {
                    auto &edge_node = hg_ref.getData(hg_ref.getEdgeDst(e));
                    auto &edge_data = hg_ref.getEdgeData(e);

                    if (edge_data.e_type == wf2::TYPES::SALE && edge_node.v_type == wf2::TYPES::PERSON)
                    {
                        if (edge_data.e.sale.product == 185785)
                            buyers.insert(edge_data.e.sale.buyer);
                    }
                }
                if (buyers.size() > 1)
                    node.v.person.ammo_sp = true;
            }

            // Publication affiliated with organizations close to NYC -> altering master nodes
            if (node.v_type == wf2::TYPES::PUBLICATION)
            {
                for (auto e : hg_ref.edges(gNode))
                {
                    auto &edge_node = hg_ref.getData(hg_ref.getEdgeDst(e));
                    auto &edge_data = hg_ref.getEdgeData(e);

                    if (edge_node.v_type == wf2::TYPES::TOPIC)
                    {
                        if (proximity_to_nyc(edge_node.v.topic))
                            node.v.publication.has_org_near_nyc = true;
                        if (edge_node.id() == 43035)
                            node.v.publication.ee_topic = true;
                    }
                }
            }
        
            // Forum is NYC type -> altering mirror nodes
            if (node.v_type == wf2::TYPES::FORUM) {
                bool has_nyc_topic = false;
                for (auto e : hg_ref.edges(gNode))
                {
                    auto &edge_node = hg_ref.getData(hg_ref.getEdgeDst(e));
                    auto &edge_data = hg_ref.getEdgeData(e);

                    if (edge_node.v_type == wf2::TYPES::TOPIC)
                    {
                        if (edge_node.id() == 60) {
                            has_nyc_topic = true;
                        }
                    }
                }

                for (auto e : hg_ref.edges(gNode))
                {
                    auto &edge_node = hg_ref.getData(hg_ref.getEdgeDst(e));
                    auto edge_data = hg_ref.getEdgeData(e);

                    if (edge_node.v_type == wf2::TYPES::FORUMEVENT)
                    {
                        edge_node.v.forum_event.nyc_topic = has_nyc_topic;
                    }
                }
            }
        },
        galois::steal());

    if (BENCH)
    {
        galois::runtime::getHostBarrier().wait();
        calculate_1_end = MPI_Wtime();
        std::cout << "Calculate 1, " << calculate_1_end - calculate_1_start << "\n";
    }

    /*
        Sync the Forum 2A + 2B + Jihad + NYC Information
        Sync the person ammunition seller information
        Sync the publication + organization information
        Reduce + Broadcast!
    */
    if (BENCH)
    {
        galois::runtime::getHostBarrier().wait();
        sync_1_start = MPI_Wtime();
    }

    syncSubstrate->sync<writeDestination, readSource, SyncPhase1>("Sync_1_Reduce");
    syncSubstrate->sync<writeSource, readDestination, SyncPhase1>("Sync_1_Broadcast");

    if (BENCH)
    {
        galois::runtime::getHostBarrier().wait();
        sync_1_end = MPI_Wtime();
        std::cout << "Sync 1, " << sync_1_end - sync_1_start << "\n";
    }

    /*
        Min time for applicable Forums + topic information propagation
        Find people having authored relevant publications
    */

    if (BENCH)
    {
        galois::runtime::getHostBarrier().wait();
        calculate_2_start = MPI_Wtime();
    }

    galois::do_all(
        galois::iterate(hg_ref.allNodesWithEdgesRange()),
        [&](const GNode &gNode)
        {
            auto &node = hg_ref.getData(gNode);

            // Min time for applicable forums -> altering master + mirror nodes
            // Propagate Jihad information from one forum event node to all those from the same forum -> altering mirror nodes
            if (node.v_type == wf2::TYPES::FORUM)
            {
                time_t min_time = node.v.forum.min_time;
                bool has_2a_event = false;
                bool has_jihad_topic = false;
                bool has_nyc_topic = false;
                for (auto e : hg_ref.edges(gNode))
                {
                    auto &edge_node = hg_ref.getData(hg_ref.getEdgeDst(e));
                    auto &edge_data = hg_ref.getEdgeData(e);
                    if (edge_node.v_type == wf2::TYPES::FORUMEVENT)
                    {
                        if (edge_node.v.forum_event.forum2B == true)
                            min_time = min_time == 0 ? edge_node.v.forum_event.date : std::min(min_time, edge_node.v.forum_event.date);
                        has_2a_event = has_2a_event | edge_node.v.forum_event.forum2A;
                        has_jihad_topic = has_jihad_topic | edge_node.v.forum_event.jihad_topic;
                        has_nyc_topic = has_nyc_topic | edge_node.v.forum_event.nyc_topic;
                    }
                }
                if (has_2a_event == true)
                {
                    // TODO - do we need this line below??
                    node.v.forum.min_time = min_time;
                    for (auto e : hg_ref.edges(gNode))
                    {
                        auto &edge_node = hg_ref.getData(hg_ref.getEdgeDst(e));
                        auto &edge_data = hg_ref.getEdgeData(e);
                        if (edge_node.v_type == wf2::TYPES::FORUMEVENT)
                        {
                            edge_node.v.forum_event.forum_min_time = min_time;
                            edge_node.v.forum_event.jihad_topic = has_jihad_topic;
                            edge_node.v.forum_event.nyc_topic = has_nyc_topic;
                        }
                    }
                }
            }
            // Person authoring a publication with the relevant requirements -> altering the master nodes
            else if (node.v_type == wf2::TYPES::PERSON)
            {
                for (auto e : hg_ref.edges(gNode))
                {
                    auto &edge_node = hg_ref.getData(hg_ref.getEdgeDst(e));
                    auto &edge_data = hg_ref.getEdgeData(e);

                    if (edge_data.e_type == wf2::TYPES::AUTHOR && edge_node.v_type == wf2::TYPES::PUBLICATION)
                    {
                        if (edge_node.v.publication.has_org_near_nyc && edge_node.v.publication.ee_topic)
                            node.v.person.ee_sp = true;
                    }
                }
            }
        },
        galois::steal());

    if (BENCH)
    {
        galois::runtime::getHostBarrier().wait();
        calculate_2_end = MPI_Wtime();
        std::cout << "Calculate 2, " << calculate_2_end - calculate_2_start << "\n";
    }

    /*
        Sync the Min time information into forum events
        Sync the person person authoring a significant publication information
    */
    if (BENCH)
    {
        galois::runtime::getHostBarrier().wait();
        sync_2_start = MPI_Wtime();
    }

    syncSubstrate->sync<writeDestination, readSource, SyncPhase2>("Sync_2_Reduce");
    syncSubstrate->sync<writeSource, readDestination, SyncPhase2>("Sync_2_Broadcast");

    if (BENCH)
    {
        galois::runtime::getHostBarrier().wait();
        sync_2_end = MPI_Wtime();
        std::cout << "Sync 2, " << sync_2_end - sync_2_start << "\n";
    }

    /*
        Find important persons and the transaction dates that are relevant to certian purchases
    */

    if (BENCH)
    {
        galois::runtime::getHostBarrier().wait();
        calculate_3_start = MPI_Wtime();
    }

    galois::do_all(
        galois::iterate(hg_ref.allNodesWithEdgesRange()),
        [&](const GNode &gNode)
        {
            auto &node = hg_ref.getData(gNode);
            // Person making an interesting purchase
            if (node.v_type == wf2::TYPES::PERSON)
            {
                bool ESP = false;
                time_t latest_BB = 0, latest_PC = 0, latest_AMO = 0;

                for (auto e : hg_ref.edges(gNode))
                {
                    auto &edge_node = hg_ref.getData(hg_ref.getEdgeDst(e));
                    auto &edge_data = hg_ref.getEdgeData(e);
                    if (edge_data.e_type == wf2::TYPES::PURCHASE)
                    {
                        if (edge_data.e.purchase.product == 2869238)
                        {
                            // bath bomb
                            latest_BB = std::max(latest_BB, edge_data.e.purchase.date);
                        }
                        else if (edge_data.e.purchase.product == 271997)
                        {
                            // pressure cooker
                            latest_PC = std::max(latest_PC, edge_data.e.purchase.date);
                        }
                        else if (edge_data.e.purchase.product == 185785)
                        {
                            // ammunition
                            if (edge_data.e.purchase.date > latest_AMO && edge_node.v.person.ammo_sp)
                            {
                                latest_AMO = edge_data.e.purchase.date;
                            }
                        }
                        else if (edge_data.e.purchase.product == 11650)
                        {
                            // electronics
                            if (!ESP)
                                ESP = edge_node.v.person.ee_sp;
                        }
                    }
                }

                time_t trans_date = std::min(std::min(latest_BB, latest_PC), latest_AMO);

                if (ESP)
                {
                    node.v.person.trans_date = trans_date;
                }

                if (ESP && trans_date != 0)
                {
                    for (auto e : hg_ref.edges(gNode))
                    {
                        auto &edge_node = hg_ref.getData(hg_ref.getEdgeDst(e));
                        auto &edge_data = hg_ref.getEdgeData(e);
                        if (edge_data.e_type == wf2::TYPES::AUTHOR && edge_node.v_type == wf2::TYPES::FORUMEVENT) {
                            if(edge_node.v.forum_event.jihad_topic && edge_node.v.forum_event.nyc_topic) {
                                if(edge_node.v.forum_event.forum_min_time != 0 && edge_node.v.forum_event.forum_min_time < trans_date)
                                    std::cout << "Found a person " << node.id() << " !!\n\n\n\n";
                            }
                        }
                    }
                }
            }
        },
        galois::steal());

    if (BENCH)
    {
        galois::runtime::getHostBarrier().wait();
        calculate_3_end = MPI_Wtime();
        std::cout << "Calculate 3, " << calculate_3_end - calculate_3_start << "\n";
    }

    return 0;
}