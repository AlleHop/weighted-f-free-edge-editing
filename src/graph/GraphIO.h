//
// Created by jonas on 02.07.19.
//

#ifndef CONCEPT_GRAPHIO_H
#define CONCEPT_GRAPHIO_H

#include <string>
#include <fstream>

#include "Graph.h"
#include "VertexPairMap.h"
#include "../Instance.h"

class GraphIO {
public:
    struct Format {
        struct Metis {
        };
    };

    /**
     * Reads a graph from path.
     *
     * Assumes that the file is in metis format with fmt == 1 and that the edges are from the upper triangular adjaceny
     * matrix of a fully connected graph.
     *
     * @param path
     * @return
     */
    static Instance read_graph(const std::string &path, double multiplier = 1) {
        using Real = double;

        std::ifstream file(path);

        if (!file) throw std::runtime_error("could not open file");

        Vertex n, m, fmt;
        std::string line;
        std::getline(file, line);
        std::stringstream(line) >> n >> m >> fmt;


        Graph G(n);
        VertexPairMap<Cost> edit_costs(G.size());

        if (fmt == 0) {
            edit_costs = VertexPairMap<Cost>(G.size(), 1);
            for (Vertex u = 1; u <= n; ++u) {
                std::getline(file, line);
                std::stringstream ss(line);
                Vertex v;
                while (ss >> v) {
                    if (u <= v) {
                        G.setEdge({u - 1, v - 1});
                    }
                }
            }
        } else if (fmt == 1) {
            for (Vertex u = 1; u <= n; ++u) {
                std::getline(file, line);
                std::stringstream ss(line);
                Vertex v;
                Real weight;
                while (ss >> v >> weight) {
                    if (u <= v) {
                        VertexPair edge(u - 1, v - 1);
                        edit_costs[edge] = static_cast<Cost>(std::ceil(std::abs(weight) * multiplier));
                        if (weight > 0) G.setEdge(edge);
                        else if (weight <= 0) G.clearEdge(edge);
                        /*
                        else {
                            std::cerr << "zero edge " << edge << " " << weight << "\n";
                            std::cerr << line << "\n";
                            throw std::runtime_error("0 weight edge detected");
                        }
                        */
                    }
                }
            }
        } else {
            throw std::runtime_error("fmt not supported");
        }


        return {path, G, edit_costs};
    }

    template<typename Weight>
    static void write_graph(const std::string &path, const Graph &graph, const VertexPairMap<Weight> &weights) {
        write_graph(path, graph, weights, Format::Metis());
    }

    template<typename Weight>
    static void
    write_graph(const std::string &path, const Graph &graph, const VertexPairMap<Weight> &weights, Format::Metis) {
        std::ofstream file(path);

        if (!file) throw std::runtime_error("could not open file");

        Vertex n = graph.size();

        file << n << " " << n * (n - 1) / 2 << " " << 1 << "\n";
        for (Vertex u : graph.vertices()) {
            for (Vertex v : graph.vertices()) {
                if (u >= v) continue;
                file << (v + 1) << " " << (graph.hasEdge({u, v}) ? 1 : -1) * weights[{u, v}]
                     << " ";
            }
            file << "\n";
        }
    }
};


#endif //CONCEPT_GRAPHIO_H
