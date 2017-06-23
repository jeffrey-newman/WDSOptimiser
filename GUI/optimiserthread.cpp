#include "optimiserthread.h"
#include <iostream>
#include "../WDSOptParameters.hpp"
#include "../WDSOptimiser.hpp"
#include "WDSOptGUICheckpoints.h"
#include "../WDSOptPostProcess.h"
#include "../WDSNSGAII.h"
#include <stdexcept>


OptimiserThread::OptimiserThread()
: count(0), params(new WDSOptParameters)
{

}

void OptimiserThread::optimise(double _val)
{
    QMutexLocker locker(&mutex);
    this->start();
}

void OptimiserThread::run()
{

    mutex.lock();

    WDSOptParameters params_t = *params;

    mutex.unlock();

    //Expensive calculation here.
    std::cout << "Running optimisation in thread\n";

    WDSOptimiser wds_eval(params_t);

    //Make the NSGAII
    boost::shared_ptr<NSGAIIObjs> nsgaii_obs = getNSGAIIForWDS(wds_eval);

    // Add the checkpoints
    createCheckpointsQtGUI(nsgaii_obs->optimiser, params_t, this);

    // Initialise population
    PopulationSPtr pop(new Population);
    if (params_t.restart_pop_file.first == "no_seed")
    {
        pop = intialisePopulationRandomDVAssignment(params_t.pop_size, wds_eval.getProblemDefinitions(), nsgaii_obs->rng);
    }
    else
    {
        pop = restore_population(params_t.restart_pop_file.second);
    }
    nsgaii_obs->optimiser.getIntMutationOperator().setMutationInverseDVSize(pop->at(0));

    // Run the optimisation
    nsgaii_obs->optimiser(pop);

    //Postprocess the results
    postProcessResults(wds_eval, pop, params_t);

    std::cout << "Finished running thread" << std::endl;

    //dummy for plotting (temp until feature complete)
    count += 1.0;
    std::uniform_real_distribution<double> uniform_dist(0.0,1.0);
    double result_t = nsgaii_obs->rng.operator()();

}

void OptimiserThread::initialise(int argc, char **argv)
{

    int err = processOptions(argc, argv, *params);
    if (err != 0)
    {
        throw std::runtime_error("Could not parse options. Check file and path");
    }
    params->evaluator_id = 0;

}

void OptimiserThread::relayNextHypervolumeMetric(int gen, double hypervolume_val)
{
    emit nextHypervolumeMetric(gen, hypervolume_val);
}