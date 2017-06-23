//
// Created by a1091793 on 13/6/17.
//

#ifndef WDS_OPT_WDSNSGAII_H
#define WDS_OPT_WDSNSGAII_H

#include "NSGAII.hpp"
#include "WDSOptimiser.hpp"
#include <random>

class NSGAIIObjs
{
public:

    typedef std::mt19937 RNG;
    unsigned seed;
    RNG rng;
    NSGAII<RNG> optimiser;

    NSGAIIObjs(WDSOptimiser & wds_eval);
};

boost::shared_ptr<NSGAIIObjs>
getNSGAIIForWDS(WDSOptimiser & wds_eval);

#endif //WDS_OPT_WDSNSGAII_H
