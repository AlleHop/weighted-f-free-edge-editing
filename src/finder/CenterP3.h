//
// Created by jonas on 23.07.19.
//

#ifndef WEIGHTED_F_FREE_EDGE_EDITING_CENTERP3_H
#define WEIGHTED_F_FREE_EDGE_EDITING_CENTERP3_H


#include "../interfaces/FinderI.h"


namespace Finder {
    class CenterP3 : public FinderI {

        Graph::AdjRow W;

    public:
        explicit CenterP3(const Graph &graph_ref) : FinderI(graph_ref), W(graph.size()) {}

        bool find(SubgraphCallback callback) override;

        bool find(const Graph &forbidden, SubgraphCallback callback) override;

        bool find_near(VertexPair uv, SubgraphCallback callback) override;

        bool find_near(VertexPair uv, const Graph &forbidden, SubgraphCallback callback) override;

        [[nodiscard]] Options::FSG forbidden_subgraphs() const override { return Options::FSG::P3; }

        [[nodiscard]] std::string name() const override { return "CenterP3"; }

        void to_yaml(YAML::Emitter &out) const override;

    private:
        template<typename F, typename G, typename H, typename I>
        bool find(const SubgraphCallback& callback, F neighbors, G non_neighbors, H valid_edge, I valid_non_edge);

        template<typename F, typename G, typename H, typename I>
        bool find_near(VertexPair uv, const SubgraphCallback& callback, F neighbors, G non_neighbors, H valid_edge, I valid_non_edge);

    };
}


#endif //WEIGHTED_F_FREE_EDGE_EDITING_CENTERP3_H
