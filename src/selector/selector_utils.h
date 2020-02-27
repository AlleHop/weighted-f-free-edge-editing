//
// Created by jonas on 06.09.19.
//

#ifndef WEIGHTED_F_FREE_EDGE_EDITING_SELECTOR_UTILS_H
#define WEIGHTED_F_FREE_EDGE_EDITING_SELECTOR_UTILS_H


#include <memory>

#include "SelectorI.h"
#include "../options.h"
#include "../Instance.h"
#include "../consumer/SubgraphStats.h"


namespace Selector {
    std::unique_ptr<SelectorI>
    make(Options::Selector selector, const std::shared_ptr<FinderI> &finder, const Instance &instance,
                   const VertexPairMap<bool> &marked, const SubgraphStats &subgraph_stats);

}

#endif //WEIGHTED_F_FREE_EDGE_EDITING_SELECTOR_UTILS_H
