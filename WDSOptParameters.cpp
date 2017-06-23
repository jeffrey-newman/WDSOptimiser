//
//  ZonalPolicyUtility.hpp
//  ZonalOptimiser
//
//  Created by a1091793 on 5/10/2016.
//  Copyright © 2016 University of Adelaide and Bushfire and Natural Hazards CRC. All rights reserved.
//


#include "WDSOptParameters.hpp"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>


WDSOptParameters::WDSOptParameters()
        : cost_calculated(false),
          resilience_calculated(false),
          pressure_constraint(false),
          head_constraint(false),
          velocity_constraint(false)
{

}


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
                ("working-dir,d", po::value<std::string>(&params.working_dir.first)->default_value(deafult_working_dir.string()), "path of directory for storing temp files during running")
                ("opt-file,g", po::value<std::string>(&params.opt_file.first), "name of geoproject file (without full path), relative to template geoproject directory. Needs to be in top level at the moment")
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
        pathify_mk(params.save_dir);
        pathify(params.opt_file);
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

