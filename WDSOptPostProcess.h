//
// Created by a1091793 on 12/6/17.
//

#ifndef WDS_OPT_WDSOPTPOSTPROCESS_H
#define WDS_OPT_WDSOPTPOSTPROCESS_H

#include "WDSOptimiser.hpp"
#include "WDSOptParameters.hpp"
#include "Population.hpp"

void
postProcessResults(WDSOptimiser & wds_eval, PopulationSPtr pop, WDSOptParameters & params);

#endif //WDS_OPT_WDSOPTPOSTPROCESS_H
