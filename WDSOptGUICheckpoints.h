//
// Created by a1091793 on 12/6/17.
//

#ifndef WDS_OPT_WDSOPTCHECKPOINTS_H
#define WDS_OPT_WDSOPTCHECKPOINTS_H

#include "WDSOptParameters.hpp"
#include "NSGAII.hpp"

template<typename RNG>
void
createCheckpoints(NSGAII<RNG> & optimiser, WDSOptParameters & params);

template<typename RNG>
void
createCheckpointsQtGUI(NSGAII<RNG> & optimiser, WDSOptParameters & params);

#endif //WDS_OPT_WDSOPTCHECKPOINTS_H
