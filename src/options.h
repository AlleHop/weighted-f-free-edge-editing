//
// Created by jonas on 24.09.19.
//

#ifndef WEIGHTED_F_FREE_EDGE_EDITING_OPTIONS_H
#define WEIGHTED_F_FREE_EDGE_EDITING_OPTIONS_H


#include <iostream>
#include <yaml-cpp/yaml.h>


namespace Options {
    enum class Selector {
        FirstEditable, LeastWeight, MostMarked
    };

    std::istream &operator>>(std::istream &in, Selector &selector);

    std::ostream &operator<<(std::ostream &os, Selector selector);

    YAML::Emitter &operator<<(YAML::Emitter &out, Selector selector);


    enum class FSG {
        P3, P4, P4C4, P5, P5C5, P6, P6C6, C4_C5_2K2, C4_C5_P5_Bowtie_Necktie
    };

    std::istream &operator>>(std::istream &in, FSG &fsg);

    std::ostream &operator<<(std::ostream &os, FSG fsg);

    YAML::Emitter &operator<<(YAML::Emitter &out, FSG fsg);


    enum class LB {
        No, Greedy, LocalSearch, LinearProgram
    };

    std::istream &operator>>(std::istream &in, LB &lower_bound);

    std::ostream &operator<<(std::ostream &os, LB lower_bound);

    YAML::Emitter &operator<<(YAML::Emitter &out, LB lower_bound);


    enum class SolverType {
        FPT, ILP
    };

    std::istream &operator>>(std::istream &os, SolverType &type);

    std::ostream &operator<<(std::ostream &os, SolverType type);

    YAML::Emitter &operator<<(YAML::Emitter &out, SolverType type);
}


#endif //WEIGHTED_F_FREE_EDGE_EDITING_OPTIONS_H