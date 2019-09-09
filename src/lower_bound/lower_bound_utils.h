//
// Created by jonas on 04.09.19.
//

#ifndef WEIGHTED_F_FREE_EDGE_EDITING_LB_UTILS_H
#define WEIGHTED_F_FREE_EDGE_EDITING_LB_UTILS_H


#include "../Configuration.h"
#include "../interfaces/LowerBoundI.h"
#include "../Instance.h"
#include "../consumer/SubgraphStats.h"


namespace LowerBound {
    std::unique_ptr<LowerBoundI>
    make(Options::LB lower_bound, const std::shared_ptr<FinderI> &finder, const Instance &instance,
         const VertexPairMap<bool> &marked, const SubgraphStats &subgraph_stats);
}

#endif //WEIGHTED_F_FREE_EDGE_EDITING_LB_UTILS_H