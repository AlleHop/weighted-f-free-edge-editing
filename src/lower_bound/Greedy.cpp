//
// Created by jonas on 29.07.19.
//


#include "Greedy.h"


namespace lower_bound {
    /**
     * Calculates a lower bound on the costs required to solve the current instance.
     *
     * Inserts forbidden subgraphs into the bound if possible. A subgraph is not inserted if it shares an editable
     * vertex pair with a subgraph already in the bound.
     *
     * Complexity:
     *      m = #forbidden subgraphs
     *      O(m) time
     *      O(1) additional space
     *
     * @param k Not used
     * @return A lower bound on the costs required to solve the current instance.
     */
    Cost Greedy::calculate_lower_bound(Cost k) {
        Cost bound_size = 0;
        m_used_in_bound.clear();

        // Only use find and not find_with_duplicates because the first version of the subgraph will be inserted.
        // This relies on the fact that all duplicate version have the same cost. Note: That may change.
        finder->find(m_graph, [&](Subgraph &&subgraph) {

            // Check if the subgraph is adjacent to one already used in the bound.
            bool touches_bound = finder->for_all_conversionless_edits(subgraph, [&](auto uv) {
                return !m_marked[uv] && m_used_in_bound[uv];
            });

            if (!touches_bound) {
                Cost cost = finder->calculate_min_cost(subgraph, m_marked, m_costs);
                if (cost == invalid_cost) {
                    bound_size = invalid_cost;
                    return true;
                }
                bound_size += cost;


                finder->for_all_conversionless_edits(subgraph, [&](auto uv) {
                    if (!m_marked[uv])
                        m_used_in_bound[uv] = true;
                    return false;
                });
            }
            return bound_size > k;
        });

        return bound_size;
    }
}
