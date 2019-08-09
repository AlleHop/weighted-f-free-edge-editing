//
// Created by jonas on 09.08.19.
//

#ifndef WEIGHTED_F_FREE_EDGE_EDITING_SUBGRAPHTESTS_H
#define WEIGHTED_F_FREE_EDGE_EDITING_SUBGRAPHTESTS_H


#include <random>
#include "../graph/Graph.h"
#include "../graph/Subgraph.h"
#include "Tests.h"

class SubgraphTests {
    std::mt19937 gen;
public:
    SubgraphTests(int seed = 0) : gen(seed) {}

    void vertexPairs_and_for_all_vertex_pairs_are_consistent() {
        Subgraph subgraph = random_subgraph(10, 20, gen);

        std::vector<VertexPair> a, b;

        for (VertexPair uv : subgraph.vertexPairs()) a.push_back(uv);

        subgraph.for_all_vertex_pairs([&](VertexPair uv) {
            b.push_back(uv);
            return false;
        });

        expect("vertexPairs() and for_all_vertex_pairs() produce the same output", a, b);
    }

    void vertices_and_iterator_are_consistent() {
        Subgraph subgraph = random_subgraph(10, 20, gen);

        std::vector<Vertex> a, b;

        for (Vertex u : subgraph.vertices()) a.push_back(u);

        for (Vertex u : subgraph) b.push_back(u);

        expect("vertices() and iterator produce the same output", a, b);
    }

    void unmarkedVertexPairs_and_for_all_unmarked_vertex_pairs_are_consistent(int n = 10, int N = 20) {
        Subgraph subgraph = random_subgraph(n, N, gen);
        VertexPairMap<bool> marked(N);
        for (int i = 0; i < N; ++i) {
            marked[random_vertex_pair(N, gen)] = true;
        }

        std::vector<VertexPair> a, b;

        for (VertexPair uv : subgraph.unmarkedVertexPairs(marked)) a.push_back(uv);

        subgraph.for_all_unmarked_vertex_pairs(marked, [&](VertexPair uv) {
            b.push_back(uv);
            return false;
        });

        expect("unmarkedVertexPairs() and for_all_unmarked_vertex_pairs() produce the same output", a, b);
    }

    void run() {
        std::cout << "\nSubgraphTests"
                     "\n-------------" << std::endl;
        vertices_and_iterator_are_consistent();
        vertexPairs_and_for_all_vertex_pairs_are_consistent();
        unmarkedVertexPairs_and_for_all_unmarked_vertex_pairs_are_consistent();
    }
};


#endif //WEIGHTED_F_FREE_EDGE_EDITING_SUBGRAPHTESTS_H
