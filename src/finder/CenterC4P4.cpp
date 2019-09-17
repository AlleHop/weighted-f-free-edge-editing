//
// Created by jonas on 03.07.19.
//


#include "CenterC4P4.h"

#include "../interfaces/FinderI.h"


namespace Finder {
    /**
     * Calls callback for all P_4's and C_4's.
     *
     * @param callback
     * @return
     */
    bool CenterC4P4::find(SubgraphCallback callback) {
        return find(callback, neighbors(graph), non_neighbors(graph), valid_edge(graph), valid_non_edge(graph));
    }

    /**
     * Calls callback for all P_4's and C_4's. Subgraphs sharing a vertex pair with the graph forbidden are ignored.
     *
     * @param forbidden
     * @param callback
     * @return
     */
    bool CenterC4P4::find(const Graph &forbidden, SubgraphCallback callback) {
        return find(callback, neighbors(graph, forbidden), non_neighbors(graph, forbidden), valid_edge(graph, forbidden), valid_non_edge(graph, forbidden));
    }

    /**
     * Calls callback for all P_4's and C_4's having both u and v as vertices.
     *
     * @param uv
     * @param callback
     * @return
     */
    bool CenterC4P4::find_near(VertexPair uv, SubgraphCallback callback) {
        return find_near(uv, callback,neighbors(graph), non_neighbors(graph), valid_edge(graph), valid_non_edge(graph));
    }

    /**
     * Calls callback for all P_4's and C_4's having both u and v as vertices. Subgraphs sharing a vertex pair with the graph forbidden are ignored.
     *
     * @param uv
     * @param forbidden
     * @param callback
     * @return
     */
    bool CenterC4P4::find_near(VertexPair uv, const Graph &forbidden, SubgraphCallback callback) {
        return find_near(uv, callback, neighbors(graph, forbidden), non_neighbors(graph, forbidden), valid_edge(graph, forbidden), valid_non_edge(graph, forbidden));
    }

    void CenterC4P4::to_yaml(YAML::Emitter &out) const {
        using namespace YAML;
        out << BeginMap;
        out << Key << "name" << Value << "CenterC4P4";
        out << Key << "forbidden_subgraphs" << Value << Options::FSG::P4C4;
        out << EndMap;
    }

    template<typename F, typename G, typename H, typename I>
    bool CenterC4P4::find(const SubgraphCallback &callback, F neighbors, G non_neighbors, H valid_edge, I valid_non_edge) {

        for (Vertex u : graph.vertices()) {
            V = neighbors(u);
            for (Vertex v : Graph::iterate(V)) {

                A = neighbors(u) & non_neighbors(v);
                B = neighbors(v) & non_neighbors(u);

                for (Vertex a : Graph::iterate(A)) {
                    for (Vertex b : Graph::iterate(B)) {

                        // assert that {a, u, v, b} is either a P_4 or a C_4
                        assert(valid_edge({a, u})); assert(valid_non_edge({a, v})); /*assert(valid({a, b}));*/ assert(valid_edge({u, v})); assert(valid_non_edge({u, b})); assert(valid_edge({v, b}));

                        if (valid_non_edge({a, b}) && a < b) {
                            // P_4
                            if (callback(Subgraph{a, u, v, b})) return true;
                        }
                        if (valid_edge({a, b}) && a < std::min({u, v, b}) && u < b) {
                            // C_4
                            if (callback(Subgraph{a, u, v, b})) return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    template<typename F, typename G, typename H, typename I>
    bool CenterC4P4::find_near(VertexPair uv, const SubgraphCallback &callback, F neighbors, G non_neighbors, H valid_edge,
                   I valid_non_edge) {

        auto [u, v] = uv;
        assert(u < v);

        if (valid_non_edge(uv)) {
            // case P_4 u, v are at positions 1 and 4 in the path
            A = neighbors(u) & non_neighbors(v);
            B = neighbors(v) & non_neighbors(u);

            for (Vertex a : Graph::iterate(A)) {
                for (Vertex b : Graph::iterate(B)) {
                    if (valid_edge({a, b})) {

                        // assert that {u, a, b, v} is a P_4
                        assert(valid_edge({u, a})); assert(valid_non_edge({u, b})); assert(valid_non_edge({u, v})); assert(valid_edge({a, b})); assert(valid_non_edge({a, v})); assert(valid_edge({b, v}));

                        // P_4
                        if (callback(Subgraph{u, a, b, v})) return true;
                    }
                }
            }

            // case P_4, C_4 u, v are at positions 1 and 3 in the path
            A = neighbors(u) & neighbors(v);
            for (Vertex a : Graph::iterate(A)) {
                B = neighbors(v) & non_neighbors(a);
                for (Vertex b : Graph::iterate(B)) {

                    // assert that {u, a, v, b} is either a C_4 or P_4
                    assert(valid_edge({u, a})); assert(valid_non_edge({u, v})); /*assert(valid({u, b}));*/ assert(valid_edge({a, v})); assert(valid_non_edge({a, b})); assert(valid_edge({v, b}));

                    if (valid_edge({u, b}) && u < std::min({a, v, b}) && a < b) {
                        // C_4
                        if (callback(Subgraph{u, a, v, b})) return true;
                    } else if (valid_non_edge({u, b})) {
                        // P_4
                        if (callback(Subgraph{u, a, v, b})) return true;
                    }
                }
            }

            // case P_4, C_4 u, v are at positions 2 and 4 in the path
            B = neighbors(u) & neighbors(v);
            for (Vertex b : Graph::iterate(B)) {
                A = neighbors(u) & non_neighbors(b);
                for (Vertex a : Graph::iterate(A)) {

                    // assert that {a, u, b, b} is either a C_4 or P_4
                    assert(valid_edge({a, u})); assert(valid_non_edge({a, b})); /*assert(valid({a, v}));*/ assert(valid_edge({u, b})); assert(valid_non_edge({u, v})); assert(valid_edge({b, v}));

                    if (valid_edge({a, v}) && a < std::min({u, b, v})) {
                        // C_4
                        if (callback(Subgraph{a, u, b, v})) return true;
                    } else if (valid_non_edge({a, v})) {
                        // P_4
                        if (callback(Subgraph{a, u, b, v})) return true;
                    }
                }
            }

        } else if (valid_edge(uv)) {
            // case C_4 u, v are at positions 1 and 4 in the path
            A = neighbors(u) & non_neighbors(v);
            B = neighbors(v) & non_neighbors(u);

            for (Vertex a : Graph::iterate(A)) {
                for (Vertex b : Graph::iterate(B)) {
                    if (valid_edge({a, b}) && u < std::min({a, b, v}) && a < v) {

                        // assert that {u, a, b, v} is a C_4
                        assert(valid_edge({u, a})); assert(valid_non_edge({u, b})); assert(valid_edge({u, v})); assert(valid_edge({a, b})); assert(valid_non_edge({a, v})); assert(valid_edge({b, v}));

                        // C_4
                        if (callback(Subgraph{u, a, b, v})) return true;
                    }
                }
            }

            // case C_4, P_4 u, v are at positions 2 and 3 in the path
            A = neighbors(u) & non_neighbors(v);
            B = neighbors(v) & non_neighbors(u);

            for (Vertex a : Graph::iterate(A)) {
                for (Vertex b : Graph::iterate(B)) {

                    // assert that {a, u, v, b} is either a C_4 or a P_4
                    assert(valid_edge({a, u})); assert(valid_non_edge({a, v})); /*assert(valid({a, b}));*/ assert(valid_edge({u, v})); assert(valid_non_edge({u, b})); assert(valid_edge({v, b}));

                    if (valid_edge({a, b}) && a < std::min({u, v, b})) {
                        // C_4
                        if (callback(Subgraph{a, u, v, b})) return true;
                    } else if (valid_non_edge({a, b})) {
                        // P_4
                        if (callback(Subgraph{a, u, v, b})) return true;
                    }
                }
            }

            // case C_4, P_4 u, v are at positions 1 and 2 in the path
            A = neighbors(v) & non_neighbors(u);

            for (Vertex a : Graph::iterate(A)) {
                B = neighbors(a) & non_neighbors(v);
                B[u] = false;

                for (Vertex b : Graph::iterate(B)) {

                    // assert that {u, v, a, b} is either a C_4 or a P_4
                    assert(valid_edge({u, v})); assert(valid_non_edge({u, a})); /*assert(valid({u, b}));*/ assert(valid_edge({v, a})); assert(valid_non_edge({v, b})); assert(valid_edge({a, b}));

                    if (valid_edge({u, b}) && u < std::min({v, a, b}) && v < b) {
                        // C_4
                        if (callback(Subgraph{u, v, a, b})) return true;
                    } else if (valid_non_edge({u, b})) {
                        // P_4
                        if (callback(Subgraph{u, v, a, b})) return true;
                    }
                }
            }

            // case C_4, P_4 u, v are at positions 3 and 4 in the path
            A = neighbors(u) & non_neighbors(v);

            for (Vertex a : Graph::iterate(A)) {
                B = neighbors(a) & non_neighbors(u);
                B[v] = false;

                for (Vertex b : Graph::iterate(B)) {

                    // assert that {b, a, u, b} is either a C_4 or P_4
                    assert(valid_edge({b, a})); assert(valid_non_edge({b, u})); /*assert(valid({b, v}));*/ assert(valid_edge({a, u})); assert(valid_non_edge({a, v})); assert(valid_edge({u, v}));

                    if (valid_edge({b, v}) && b < std::min({a, u, v})) {
                        // C_4
                        if (callback(Subgraph{b, a, u, v})) return true;
                    } else if (valid_non_edge({b, v})) {
                        // P_4
                        if (callback(Subgraph{b, a, u, v})) return true;
                    }
                }
            }
        }
        return false;
    }
}
