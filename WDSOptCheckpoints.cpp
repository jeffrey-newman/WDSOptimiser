//
// Created by a1091793 on 12/6/17.
//


#include "WDSOptGUICheckpoints.h"
#include <csignal>
#include "Checkpoints/SavePopCheckpoint.hpp"
#include "Checkpoints/MaxGenCheckpoint.hpp"
#include "Checkpoints/PlotFronts.hpp"
#include "Metrics/Hypervolume.hpp"
#include "Checkpoints/ResetMutationXoverFlags.hpp"
#include "Checkpoints/MetricLinePlot.hpp"
#include "Checkpoints/MaxGenCheckpoint.hpp"
#include "Checkpoints/MailCheckpoint.hpp"
#include "Checkpoints/SaveFirstFrontCheckpoint.hpp"
#include "Checkpoints/SignalCheckpoint.hpp"

template<typename RNG>
void
createCheckpoints(NSGAII<RNG> & optimiser, WDSOptParameters & params)

{
    boost::shared_ptr<SavePopCheckpoint> save_pop(new SavePopCheckpoint(params.save_freq, params.save_dir.second));
    boost::shared_ptr<SaveFirstFrontCheckpoint> save_front(new SaveFirstFrontCheckpoint(params.save_freq, params.save_dir.second));
    boost::shared_ptr<Hypervolume> hvol(new Hypervolume(1, params.save_dir.second, Hypervolume::TERMINATION, params.max_gen_hvol));
//#if defined(WITH_VTK)
//    boost::shared_ptr<MetricLinePlot> hvol_plot(new MetricLinePlot(hvol));
//#endif
    boost::shared_ptr<MaxGenCheckpoint> maxgen(new MaxGenCheckpoint(params.max_gen));
    std::string mail_subj("Hypervolume of front from Zonal calibrator ");
    boost::shared_ptr<MailCheckpoint> mail(new MailCheckpoint(params.save_freq, hvol, mail_subj));
    std::string jeffs_address("jeffrey.newman@adelaide.edu.au");
    mail->addAddress(jeffs_address);
    boost::shared_ptr<SignalCheckpoint> signal_handler(new SignalCheckpoint(SIGINT));

    boost::shared_ptr<PlotFrontVTK> plotfront(new PlotFrontVTK);

    optimiser.add_checkpoint(save_pop);
    optimiser.add_checkpoint(save_front);
    optimiser.add_checkpoint(hvol);
    optimiser.add_checkpoint(mail);
//#if defined(WITH_VTK)
//    optimiser.add_checkpoint(hvol_plot);
//#endif
    optimiser.add_checkpoint(plotfront);
    optimiser.add_checkpoint(maxgen);
    optimiser.add_checkpoint(signal_handler);
}




template void
createCheckpoints(NSGAII<std::mt19937> & optimiser, WDSOptParameters & params);

