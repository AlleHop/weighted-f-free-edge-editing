//
// Created by jonas on 15.07.19.
//

#ifndef CONCEPT_EDITOR_H
#define CONCEPT_EDITOR_H

#include <iostream>

#include "graph/Graph.h"
#include "graph/VertexPairMap.h"
#include "Instance.h"
#include "Configuration.h"
#include "Statistics.h"

#include "interfaces/SelectorI.h"
#include "interfaces/LowerBoundI.h"
#include "interfaces/ConsumerI.h"
#include "interfaces/FinderI.h"

#include "finder/finder_utils.h"
#include "lower_bound/lower_bound_utils.h"
#include "selector/lower_bound_utils.h"

#include "consumer/SubgraphStats.h"


class Editor {
private:
    Instance m_instance;
    VertexPairMap<bool> m_marked;

    std::unique_ptr<LowerBoundI> m_lower_bound;
    std::unique_ptr<SelectorI> m_selector;
    std::unique_ptr<SubgraphStats> m_subgraph_stats;

    std::vector<ConsumerI *> m_consumers;
    std::shared_ptr<FinderI> m_finder;

    std::vector<VertexPair> edits;
    bool m_found_solution;

    Statistics m_stats;

public:
    explicit Editor(Instance instance, Options::Selector selector, Options::FSG forbidden, Options::LB lower_bound) :
            m_instance(std::move(instance)), m_marked(m_instance.graph.size()), m_found_solution(false) {

        m_finder = Finder::make(forbidden, m_instance.graph);
        m_subgraph_stats = std::make_unique<SubgraphStats>(m_finder, m_instance, m_marked);
        m_consumers.emplace_back(m_subgraph_stats.get());

        m_selector = Selector::make(selector, m_finder, m_instance, m_marked);
        m_lower_bound = LowerBound::make(lower_bound, m_finder, m_instance, m_marked, *m_subgraph_stats);

        m_consumers.emplace_back(m_lower_bound.get());
        m_consumers.emplace_back(m_selector.get());
    }

    /**
     * Initializes statistics and consumers. Calls recursive editing function.
     *
     * @param k The maximimum allowed editing cost.
     * @param result_cb A callback which is called when a result is found (vector<VertexPair> -> void).
     * @param prune_cb A callback which is called when a branch is pruned ((Cost, Cost) -> void).
     * @return Whether a solution was found.
     */
    template<typename ResultCallback, typename PrunedCallback>
    bool edit(Cost k, ResultCallback result_cb, PrunedCallback prune_cb) {
        // init stats
        m_stats = Statistics(-k / 10, k, 10);

        for (auto &c : m_consumers) c->initialize();

        m_found_solution = false;
        edit_recursive(k, [&](const std::vector<VertexPair> &e) { result_cb(e); return false; }, prune_cb);
        return m_found_solution;
    }

    [[nodiscard]] const Statistics &stats() const {
        return m_stats;
    }

private:
    /**
     * Perform an edit step with remaining edit cost k.
     *
     * @param k The remaining editing cost.
     * @param result_cb A callback which is called when a result is found (vector<VertexPair> -> bool). If it returns true, the execution is stopped early.
     * @param prune_cb A callback which is called when a branch is pruned ((Cost, Cost) -> void).
     * @return Whether the execution was stopped early
     */
    template<typename ResultCallback, typename PrunedCallback>
    bool edit_recursive(Cost k, ResultCallback result_cb, PrunedCallback prune_cb) {
        const VertexPairMap<Cost> &costs = m_instance.costs;

        m_stats.calls(k)++;


        auto lb = m_lower_bound->result(k);
        if (k < lb) {
            // unsolvable, too few edits remaining
            prune_cb(k, lb);
            m_stats.prunes(k)++;
            return false;
        }

        auto problem = m_selector->result(k);

        if (problem.solved) {
            // solved
            m_found_solution = true;
            return result_cb(edits); // output graph
        } else if (k == 0) {
            // unsolved, no edits remaining
            prune_cb(0, 0);
            return false;
        }


        // recurse on problem pairs. keep vertex pairs marked between calls.
        bool return_value = false;
        for (VertexPair uv : problem.pairs) {
            assert(!m_marked[uv]);

            for (auto &c : m_consumers) c->push_state(k); // next_state(state)

            mark_and_edit_edge(uv);

            if (edit_recursive(k - costs[uv], result_cb, prune_cb)) return_value = true;

            unedit_edge(uv);

            for (auto &c : m_consumers) c->pop_state(); // destroy next_state

            if (return_value) break;
        }

        for (auto uv = problem.pairs.rbegin(); uv != problem.pairs.rend(); ++uv)
            if (m_marked[*uv])
                unmark_edge(*uv);

        return return_value;
    }

    /**
     * Mark the vertex pair uv and make the edit in the graph.
     * The consumers are informed before and after the actions are performed.
     *
     * @param uv
     */
    void mark_and_edit_edge(VertexPair uv) {
        assert(!m_marked[uv]);
        Graph &G = m_instance.graph;

        for (auto &c : m_consumers) c->before_mark_and_edit(uv);  // all - next_state
        for (auto &c : m_consumers) c->before_mark(uv);           // all - state

        m_marked[uv] = true;

        for (auto &c : m_consumers) c->after_mark(uv);            // all - state
        for (auto &c : m_consumers) c->before_edit(uv);           // subgraph_stats

        G.toggleEdge(uv);
        edits.push_back(uv);

        for (auto &c : m_consumers) c->after_edit(uv);            // subgraph_stats
        for (auto &c : m_consumers) c->after_mark_and_edit(uv);   // all - next_state
    }

    /**
     * Undo the edit in the graph.
     *
     * @param uv
     */
    void unedit_edge(VertexPair uv) {
        assert(m_marked[uv]);
        Graph &G = m_instance.graph;

        for (auto &c : m_consumers) c->before_edit(uv);           // subgraph_stats

        G.toggleEdge(uv);
        edits.pop_back();

        for (auto &c : m_consumers) c->after_edit(uv);            // subgraph_stats
    }

    /**
     * Unmark the vertex pair.
     *
     * @param uv
     */
    void unmark_edge(VertexPair uv) {
        assert(m_marked[uv]);
        m_marked[uv] = false;

        for (auto &c : m_consumers) c->after_unmark(uv);          // subgraph_stats
    }
};

#endif //CONCEPT_EDITOR_H
