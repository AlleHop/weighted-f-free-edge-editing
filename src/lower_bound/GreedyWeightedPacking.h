#ifndef WEIGHTED_F_FREE_EDGE_EDITING_GREEDYWEIGHTEDPACKING_H
#define WEIGHTED_F_FREE_EDGE_EDITING_GREEDYWEIGHTEDPACKING_H


#include <queue>

#include "../definitions.h"
#include "LowerBoundI.h"
#include "../Instance.h"
#include "../forbidden_subgraphs/SubgraphC4P4.h"


namespace lower_bound {
    template <Options::FSG SetOfForbiddenSubgraphs>
    class GreedyWeightedPacking : public LowerBoundI {
    private:
        using Subgraph = SubgraphT<SetOfForbiddenSubgraphs>;
        using Finder = typename Subgraph::Finder;

        const Graph &m_graph;
        const VertexPairMap<Cost> &m_costs;
        const VertexPairMap<bool> &m_marked;
        VertexPairMap<Cost> m_costs_remaining;

        std::vector<std::pair<Cost, Subgraph>> m_subgraph_heap;

        Finder finder;
    public:
        GreedyWeightedPacking(const Instance &instance, const VertexPairMap<bool> &marked) :
                m_graph(instance.graph), m_costs(instance.costs), m_marked(marked),
                m_costs_remaining(m_costs.size()) {}

        Cost calculate_lower_bound(Cost k) override {
            m_subgraph_heap.clear();
            Cost lower_bound = 0;
            Cost max_min_cost = std::numeric_limits<Cost>::min();

            // Initialize remaining costs with the editing costs.
            // Previously this was `m_costs_remaining = m_costs`, which led to large amounts of memory consumption.
            // TODO: Investigate memory usage.
            for (auto uv : Graph::VertexPairs(m_costs.size()))
                m_costs_remaining[uv] = m_costs[uv];


            bool early_exit = finder.find(m_graph, [&](Subgraph subgraph) {
                Cost initial_min_cost = subgraph.calculate_min_cost(m_costs_remaining, m_marked);

                m_subgraph_heap.emplace_back(initial_min_cost, std::move(subgraph));

                max_min_cost = std::max(max_min_cost, initial_min_cost);
                return max_min_cost > k;
            });

            // If a single subgraph has an editing cost larger than k, or has an invalid edititing cost (i.e. only has
            // edits that are either marked, or lead to a conversion to another forbidden subgraph), the current
            // instance is no longer solvable.
            if (early_exit)
                return max_min_cost;

            std::make_heap(m_subgraph_heap.begin(), m_subgraph_heap.end());

            while (!m_subgraph_heap.empty() && lower_bound < k) {
                std::pop_heap(m_subgraph_heap.begin(), m_subgraph_heap.end());
                const auto &[cost, subgraph] = m_subgraph_heap.back();

                // m_cost_remaining may be updated after cost has been calculated.
                Cost current_min_cost = subgraph.calculate_min_cost(m_costs_remaining, m_marked);

                if (current_min_cost == 0) {
                    // The subgraph cannot be inserted.
                    m_subgraph_heap.pop_back();
                } else if (current_min_cost == cost) {
                    // The editing cost is maximal from all remaining subgraphs in the queue. Increase the lower bound
                    // and update the remaining cost matrix.
                    lower_bound += current_min_cost;

                    for (auto uv : subgraph.non_converting_edits()) {
                        m_costs_remaining[uv] -= current_min_cost;
                    }
                    m_subgraph_heap.pop_back();
                } else {
                    // The editing cost is no longer up to date. The subgraph may be inserted in the future.
                    m_subgraph_heap.back().first = current_min_cost;
                    std::push_heap(m_subgraph_heap.begin(), m_subgraph_heap.end());
                }
            }

            return lower_bound;
        }
    };

    template class GreedyWeightedPacking<Options::FSG::C4P4>;
}

#endif //WEIGHTED_F_FREE_EDGE_EDITING_GREEDYWEIGHTEDPACKING_H
