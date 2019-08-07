//
// Created by jonas on 29.07.19.
//

#ifndef WEIGHTED_F_FREE_EDGE_EDITING_GREEDYLOWERBOUND_H
#define WEIGHTED_F_FREE_EDGE_EDITING_GREEDYLOWERBOUND_H


#include "../interfaces/LowerBoundI.h"
#include "../Instance.h"
#include "../graph/Subgraph.h"

namespace LowerBound {
    class GreedyLowerBound : public LowerBoundI {
        const Graph &graph;
        const VertexPairMap<Cost> &costs;
        const VertexPairMap<bool> &forbidden;
    public:
        class State : public StateI {
            std::unique_ptr<StateI> copy() override {
                return std::make_unique<State>(*this);
            }
        };

        GreedyLowerBound(const Instance &instance, const VertexPairMap<bool> &forbidden,
                         std::shared_ptr<FinderI> finder) : LowerBoundI(std::move(finder)), graph(instance.graph),
                                                            costs(instance.costs), forbidden(forbidden) {}

        /**
         * Calculates a lower bound on the costs required to solve the current instance.
         *
         * Greedily inserts forbidden subgraphs into the bound. Higher minimum editing costs are preferred. A subgraph
         * is not inserted if it shares an editable vertex pair with a subgraph already in the bound.
         *
         * @param state Not used
         * @param k Not used
         * @return A lower bound on the costs required to solve the current instance.
         */
        Cost result(StateI &state, Cost k) override {

            // Find all forbidden subgraphs with editable vertex pairs
            // The cost for a single forbidden subgraph is the minimum edit cost for an editable vertex pair
            std::vector<std::pair<Cost, Subgraph>> subgraphs;
            finder->find([&](Subgraph &&subgraph) {
                Cost min_cost = cost(subgraph, forbidden, costs);
                subgraphs.emplace_back(min_cost, std::move(subgraph));
                return false;
            });

            // Sort subgraphs with decreasing costs
            std::sort(subgraphs.begin(), subgraphs.end(),
                      [](const auto &lhs, const auto &rhs) { return lhs.first > rhs.first; });

            Cost bound_size = 0;
            VertexPairMap<bool> is_in_bound(graph, false);

            // Insert forbidden subgraphs with decreasing minimum edit cost into the bound
            // Only insert a subgraph if it does not share an editable vertex pair with a subgraph already in the bound
            for (const auto&[cost, subgraph] : subgraphs) {
                bool touches_bound = subgraph.for_all_unmarked_vertex_pairs(forbidden, [&](VertexPair uv) {
                    return is_in_bound[uv];
                });

                if (!touches_bound) {
                    bound_size += cost;
                    subgraph.for_all_unmarked_vertex_pairs(forbidden, [&](VertexPair uv) {
                        is_in_bound[uv] = true;
                        return false;
                    });
                }
            }

            return bound_size;
        }

        std::unique_ptr<StateI> initialize(Cost k) override { return std::make_unique<State>(); }

        void before_mark_and_edit(StateI &state, VertexPair uv) override {}

        void after_mark_and_edit(StateI &state, VertexPair uv) override {}

        void before_mark(StateI &state, VertexPair uv) override {}

        void after_mark(StateI &state, VertexPair uv) override {}

        void before_edit(StateI &state, VertexPair uv) override {}

        void after_edit(StateI &state, VertexPair uv) override {}

        void after_unmark(StateI &state, VertexPair uv) override {}
    };
}


#endif //WEIGHTED_F_FREE_EDGE_EDITING_GREEDYLOWERBOUND_H
