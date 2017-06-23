 
//
//  main.cpp
//  MetronamicaCalibrator
//
//  Created by a1091793 on 9/05/2016.
//  Copyright © 2016 University of Adelaide and Bushfire and Natural Hazards CRC. All rights reserved.
//


#include "WDSNSGAII.h"
#include "WDSOptParameters.hpp"
#include "WDSOptPostProcess.h"
#include "WDSOptGUICheckpoints.h"


int main(int argc, char * argv[]) {

    WDSOptParameters params;
    int err = processOptions(argc, argv, params);
    if (err != 0)
    {
        return EXIT_SUCCESS;
    }
    params.evaluator_id = 0;
    WDSOptimiser wds_eval(params);

    //Make the NSGAII
    boost::shared_ptr<NSGAIIObjs> nsgaii_obs = getNSGAIIForWDS(wds_eval);

    // Add the checkpoints
    createCheckpoints(nsgaii_obs->optimiser, params);

    // Initialise population
    PopulationSPtr pop(new Population);
    if (params.restart_pop_file.first == "no_seed")
    {
        pop = intialisePopulationRandomDVAssignment(params.pop_size, wds_eval.getProblemDefinitions(), nsgaii_obs->rng);
    }
    else
    {
        pop = restore_population(params.restart_pop_file.second);
    }
    nsgaii_obs->optimiser.getIntMutationOperator().setMutationInverseDVSize(pop->at(0));

    // Run the optimisation
    nsgaii_obs->optimiser(pop);
    
    //Postprocess the results
    postProcessResults(wds_eval, pop, params);


}
