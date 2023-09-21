/*
 * SpanningGTest.cpp
 *
 *  Created on: 03.09.2015
 *      Author: Henning
 */

#include <gtest/gtest.h>

#include <networkit/auxiliary/Log.hpp>
#include <networkit/graph/KruskalMSF.hpp>
#include <networkit/graph/RandomMaximumSpanningForest.hpp>
#include <networkit/graph/SpanningForest.hpp>
#include <networkit/graph/UnionMaximumSpanningForest.hpp>
#include <networkit/io/METISGraphReader.hpp>

namespace NetworKit {

class SpanningGTest : public testing::Test {};

TEST_F(SpanningGTest, testKruskalMinSpanningForest) {
    METISGraphReader reader;
    std::vector<std::string> graphs = {"karate", "jazz", "celegans_metabolic"};

    for (const auto &graphname : graphs) {
        std::string filename = "input/" + graphname + ".graph";
        Graph G = reader.read(filename);
        KruskalMSF msf(G);
        msf.run();
        Graph T = msf.getForest();

        // check that each node has an edge in the spanning tree (if it had one before)
        T.forNodes([&](node u) { EXPECT_TRUE(T.degree(u) > 0 || G.degree(u) == 0); });
    }
}

TEST_F(SpanningGTest, testKruskalMinSpanningForestSimple) {
    Graph g(5, true);
    g.addEdge(0, 1, 1);
    g.addEdge(1, 2, 1);
    g.addEdge(1, 3, 1);
    g.addEdge(3, 4, 1);
    g.addEdge(1, 4, 1);
    g.indexEdges();

    KruskalMSF msf(g);
    msf.run();
    Graph T = msf.getForest();

    // check that each node has an edge in the spanning tree (if it had one before)
    T.forNodes([&](node u) { EXPECT_TRUE(T.degree(u) > 0 || g.degree(u) == 0); });
}

TEST_F(SpanningGTest, testSpanningForest) {
    METISGraphReader reader;
    std::vector<std::string> graphs = {"karate", "jazz", "celegans_metabolic"};

    for (const auto &graphname : graphs) {
        std::string filename = "input/" + graphname + ".graph";
        Graph G = reader.read(filename);
        SpanningForest msf(G);
        msf.run();
        Graph T = msf.getForest();

        INFO("tree / graph edges: ", T.numberOfEdges(), " / ", G.numberOfEdges());

        // check that each node has an edge in the spanning tree (if it had one before)
        T.forNodes([&](node u) { EXPECT_TRUE(T.degree(u) > 0 || G.degree(u) == 0); });
    }
}

TEST_F(SpanningGTest, testRandomMaximumSpanningForest) {
    METISGraphReader reader;
    std::vector<std::string> graphs = {"karate", "jazz", "celegans_metabolic"};

    for (const auto &graphname : graphs) {
        std::string filename = "input/" + graphname + ".graph";
        Graph G = reader.read(filename);

        RandomMaximumSpanningForest rmsf(G);
        rmsf.run();
        Graph T = rmsf.getMSF();

        INFO("tree / graph edges: ", T.numberOfEdges(), " / ", G.numberOfEdges());

        // check that each node has an edge in the spanning tree (if it had one before)
        T.forNodes([&](node u) { EXPECT_TRUE(T.degree(u) > 0 || G.degree(u) == 0); });
        T.forEdges([&](node u, node v) { EXPECT_TRUE(rmsf.inMSF(u, v)); });
    }
}

TEST_F(SpanningGTest, testUnionMaximumSpanningForest) {
    METISGraphReader reader;
    std::vector<std::string> graphs = {"karate", "jazz", "celegans_metabolic"};

    for (const auto &graphname : graphs) {
        std::string filename = "input/" + graphname + ".graph";
        Graph G = reader.read(filename);

        UnionMaximumSpanningForest umsf(G);
        umsf.run();
        Graph T = umsf.getUMSF();

        INFO("tree / graph edges: ", T.numberOfEdges(), " / ", G.numberOfEdges());

        // check that each node has an edge in the spanning tree (if it had one before)
        T.forNodes([&](node u) { EXPECT_TRUE(T.degree(u) > 0 || G.degree(u) == 0); });
        T.forEdges([&](node u, node v) { EXPECT_TRUE(umsf.inUMSF(u, v)); });
    }
}

} /* namespace NetworKit */
