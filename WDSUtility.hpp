//
//  ZonalPolicyUtility.hpp
//  ZonalOptimiser
//
//  Created by a1091793 on 5/10/2016.
//  Copyright © 2016 University of Adelaide and Bushfire and Natural Hazards CRC. All rights reserved.
//

#ifndef WDSUtility_hpp
#define WDSUtility_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include <sstream>
#include <csignal>
#include <boost/program_options.hpp>
#include <boost/foreach.hpp>
#include "Pathify.hpp"

#include "NSGAII.hpp"
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



struct WDSOptParameters
{
    CmdLinePaths template_project_dir; // path to template project
    CmdLinePaths working_dir;
    std::string  rel_path_opt; // relative path of opt file from template project directory)
    bool is_logging = false;
    int pop_size; // For the GA
    int max_gen_hvol;  // Termination condition for the GA
    int max_gen;
    int save_freq;
    CmdLinePaths save_dir;
    int evaluator_id = 0;
    std::vector<int> rand_seeds { 1000,1001,1002,1003,1004,1005,1006,1007,1008,1009, 1010, 1011, 1012, 1013, 1014, 1015, 1016, 1017, 1018, 1019, 1020 };
    CmdLinePaths restart_pop_file;
};

std::string
userHomeDir() {
    std::string path;
    char const *home = getenv("HOME");
    if (home or ((home = getenv("USERPROFILE")))) {
        path.append(home);
    } else {
        char const *hdrive = getenv("HOMEDRIVE"),
                *hpath = getenv("HOMEPATH");
        assert(hdrive);  // or other error handling
        assert(hpath);
        path.append(std::string(hdrive) + hpath);
    }
    return path;
}

std::pair<std::string, std::string> at_option_parser(std::string const&s)
{
    if ('@' == s[0])
        return std::make_pair(std::string("cfg-file"), s.substr(1));
    else
        return std::pair<std::string, std::string>();
}

int
processOptions(int argc, char * argv[], WDSOptParameters & params)
{

    try
    {

        boost::filesystem::path deafult_working_dir = boost::filesystem::path(userHomeDir()) / ".geonamicaZonalOpt/working_dir";

        namespace po = boost::program_options;
        po::options_description desc("Allowed options");
        desc.add_options()
        ("help,h", "produce help message")
        ("template,t", po::value<std::string>(&params.template_project_dir.first), "path to template geoproject directory")
        ("working-dir,d", po::value<std::string>(&params.working_dir.first)->default_value(deafult_working_dir.string()), "path of directory for storing temp files during running")
        ("opt-file,g", po::value<std::string>(&params.rel_path_opt), "name of geoproject file (without full path), relative to template geoproject directory. Needs to be in top level at the moment")
        ("is-logging,s", po::value<bool>(&params.is_logging), "TRUE or FALSE whether to log the evaluation")
        ("save-dir,v", po::value<std::string>(&params.save_dir.first)->default_value(boost::filesystem::current_path().string()), "path of the directory for writing results and outputs to")
        ("pop-size,p", po::value<int>(&params.pop_size)->default_value(415), "Population size of the NSGAII")
        ("max-gen-no-hvol-improve,x", po::value<int>(&params.max_gen_hvol)->default_value(50), "maximum generations with no improvement in the hypervolume metric - terminaation condition")
        ("max-gen,y", po::value<int>(&params.max_gen)->default_value(500), "Maximum number of generations - termination condition")
        ("save-freq,q", po::value<int>(&params.save_freq)->default_value(1), "how often to save first front, hypervolume metric and population")
        ("reseed,r", po::value<std::string>(&params.restart_pop_file.first)->default_value("no_seed"), "File with saved population as initial seed population for GA")
        ("cfg-file,c", po::value<std::string>(), "can be specified with '@name', too");

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).extra_parser(at_option_parser).run(), vm);

        if (vm.count("help"))
        {
            std::cout << desc << "\n";
            return 1;
        }

        if (vm.count("cfg-file")) {
            // Load the file and tokenize it
            std::ifstream ifs(vm["cfg-file"].as<std::string>().c_str());
            if (!ifs) {
                std::cout << "Could not open the cfg file\n";
                return 1;
            }
            po::store(po::parse_config_file(ifs, desc), vm);
        }


        po::notify(vm);
        pathify_mk(params.working_dir);
        pathify(params.template_project_dir);
        pathify_mk(params.save_dir);
        if (params.restart_pop_file.first != "no_seed")
        {
            pathify(params.restart_pop_file);
        }


    }
    catch(std::exception& e)
    {
      std::cerr << "Error: " << e.what() << "\n";
      return 1;
    }
    catch(...)
    {
      std::cerr << "Unknown error!" << "\n";
      return 1;
    }
    
    return (0);
    
}

template<typename RNG>
void
createCheckpoints(NSGAII<RNG> & optimiser, WDSOptParameters & params)
{
    boost::shared_ptr<SavePopCheckpoint> save_pop(new SavePopCheckpoint(params.save_freq, params.save_dir.second));
    boost::shared_ptr<SaveFirstFrontCheckpoint> save_front(new SaveFirstFrontCheckpoint(params.save_freq, params.save_dir.second));
    boost::shared_ptr<Hypervolume> hvol(new Hypervolume(1, params.save_dir.second, Hypervolume::TERMINATION, params.max_gen_hvol));
//    boost::shared_ptr<MetricLinePlot> hvol_plot(new MetricLinePlot(hvol));
    boost::shared_ptr<MaxGenCheckpoint> maxgen(new MaxGenCheckpoint(params.max_gen));
    std::string mail_subj("Hypervolume of front from Zonal calibrator ");
    boost::shared_ptr<MailCheckpoint> mail(new MailCheckpoint(10, hvol, mail_subj));
    std::string jeffs_address("jeffrey.newman@adelaide.edu.au");
    mail->addAddress(jeffs_address);
    boost::shared_ptr<SignalCheckpoint> signal_handler(new SignalCheckpoint(SIGINT));
    
//    boost::shared_ptr<PlotFrontVTK> plotfront(new PlotFrontVTK);
    optimiser.add_checkpoint(save_pop);
    optimiser.add_checkpoint(save_front);
    optimiser.add_checkpoint(hvol);
    optimiser.add_checkpoint(mail);
//    optimiser.add_checkpoint(hvol_plot);
//    optimiser.add_checkpoint(plotfront);
    optimiser.add_checkpoint(maxgen);
    optimiser.add_checkpoint(signal_handler);
}


#include "WDSOptimiser.hpp"

void
postProcessResults(WDSOptimiser & wds_eval, PopulationSPtr pop, WDSOptParameters & params)
{
    Population & first_front = pop->getFronts()->at(0);
    
    int i = 0;
    BOOST_FOREACH(IndividualSPtr ind, first_front)
    {
        std::vector<double> objectives;
        std::vector<double> constraints;
        boost::filesystem::path save_ind_dir = params.save_dir.second / ("individual_" + std::to_string(i++));
        if (!boost::filesystem::exists(save_ind_dir)) boost::filesystem::create_directory(save_ind_dir);
        std::tie(objectives, constraints) = wds_eval(ind->getRealDVVector(), ind->getIntDVVector(), true, save_ind_dir);
        ind->setObjectives(objectives);
        ind->setConstraints(constraints);
        std::cout << *ind << std::endl;
    }
    
    boost::filesystem::path save_file = params.save_dir.second / "final_front.xml";
    std::ofstream ofs(save_file.c_str());
    assert(ofs.good());
    boost::archive::xml_oarchive oa(ofs);
    oa << BOOST_SERIALIZATION_NVP(first_front);
    
    boost::filesystem::path save_file2 = params.save_dir.second /  "final_front.txt";
    std::ofstream ofs2(save_file2.c_str());
    assert(ofs2.good());
    ofs2 << pop;
}


//void
//cleanup(ZonalPolicyParameters & params)
//{
//    if (params.wine_prefix_path.first != "do not test") {
//
//        boost::filesystem::path symlinkpath("~/.wine/dosdevices");
//        symlinkpath = symlinkpath / params.wine_drive_letter;
////Check is symbolic link for wine J: exists.
//        boost::filesystem::file_status lnk_status = boost::filesystem::status(symlinkpath);
//        if ((boost::filesystem::exists(lnk_status)))
//        {
//            boost::filesystem::remove_all(symlinkpath);
//        }
//    }
//
//    if (boost::filesystem::exists(params.working_dir.second))
//    {
//        boost::filesystem::remove_all(params.working_dir.second);
//    }

//}

#endif /* ZonalPolicyUtility_hpp */
