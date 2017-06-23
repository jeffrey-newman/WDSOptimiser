//
// Created by a1091793 on 12/6/17.
//

#include "WDSNSGAII.h"

NSGAIIObjs::NSGAIIObjs(WDSOptimiser & wds_eval):
        seed(std::chrono::system_clock::now().time_since_epoch().count()),
        rng(seed),
        optimiser(rng, wds_eval)
{

}

boost::shared_ptr<NSGAIIObjs>
getNSGAIIForWDS(WDSOptimiser & wds_eval)
{
    boost::shared_ptr<NSGAIIObjs> nsgaii_objs(new NSGAIIObjs(wds_eval));
    return nsgaii_objs;
}