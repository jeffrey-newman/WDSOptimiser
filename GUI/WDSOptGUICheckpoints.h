//
// Created by a1091793 on 12/6/17.
//

#ifndef WDS_OPT_WDSOPTCHECKPOINTS_H
#define WDS_OPT_WDSOPTCHECKPOINTS_H

#include "../WDSOptParameters.hpp"
#include "NSGAII.hpp"
#include "optimiserthread.h"

template<typename RNG>
void
createCheckpointsQtGUI(NSGAII<RNG> & optimiser, WDSOptParameters & params, OptimiserThread * thread);

#endif //WDS_OPT_WDSOPTCHECKPOINTS_H
