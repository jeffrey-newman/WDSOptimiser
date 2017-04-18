//
//  ZonalPolicyOptimiser.hpp
//  ZonalOptimiser
//
//  Created by a1091793 on 4/10/2016.
//  Copyright © 2016 University of Adelaide and Bushfire and Natural Hazards CRC. All rights reserved.
//

#ifndef ZonalPolicyOptimiser_hpp
#define ZonalPolicyOptimiser_hpp

#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <string>
#include <sstream>
#include <fstream>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds
#include <tuple>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp> // include Boost, a C++ library
#include <boost/date_time.hpp>
#include <boost/optional.hpp>
#include <boost/timer/timer.hpp>
#include "Evaluation.hpp"


#include "WDSUtility.hpp"
#include "ENLink.h"

class WDSOptimiser : public ObjectivesAndConstraintsBase
{
private:
    
    WDSOptParameters params;

    boost::filesystem::path working_project;

    boost::filesystem::path logfile;
    boost::filesystem::path previous_logfile;

    int eval_count;
    int num_constraints;
    int analysis_id;
    std::vector<double>  real_lowerbounds;
    std::vector<double>  real_upperbounds;
    std::vector<int>  int_lowerbounds;
    std::vector<int>  int_upperbounds;
    std::vector<MinOrMaxType>  minimise_or_maximise;
    ProblemDefinitionsSPtr prob_defs;
    std::pair<std::vector<double>, std::vector<double> > objectives_and_constrataints;

    
    //Copies entire directory - so that each geoproject is running in a different directory.
    bool copyDir(   boost::filesystem::path const & source,
                    boost::filesystem::path const & destination )
    {
//        std::cout << "Copying " << source << " to " << destination << std::endl;
        namespace fs = boost::filesystem;
        try
        {
            // Check whether the function call is valid
            if(!fs::exists(source) || !fs::is_directory(source))
            {
                std::cerr << "Source directory " << source.string()
                          << " does not exist or is not a directory." << '\n';
                return false;
            }
            if(fs::exists(destination))
            {
                std::cerr << "Destination directory " << destination.string()
                          << " already exists." << '\n';
                return false;
            }
            // Create the destination directory
            fs::copy_directory(source, destination);
            if(!fs::exists(destination) || !fs::is_directory(destination))
            {
                std::cerr << "Unable to create destination directory"
                          << destination.string() << '\n';
                return false;
            }
        }
        catch(fs::filesystem_error const & e)
        {
            std::cerr << e.what() << '\n';
            return false;
        }
        // Iterate through the source directory
        for(
                fs::directory_iterator file(source);
                file != fs::directory_iterator(); ++file
                )
        {

            try
            {
                fs::path current(file->path());
//                std::cout << "descending into: " << current << std::endl;
                if(fs::is_directory(current))
                {
                    // Found directory: Recursion
                    if(!copyDir(current, destination / current.filename()))
                    {
                        return false;
                    }
                }
                else
                {
                    // Found file: Copy
//                    std::cout << "Copying " << current << " to " << destination / current.filename() << std::endl;
                    fs::copy_file(current, destination / current.filename());
                }
            }
            catch(fs::filesystem_error const & e)
            {
                std:: cerr << e.what() << '\n';
            }
        }
        return true;
    }

    //Copies entire directory - so that each geoproject is running in a different directory.
    bool copyFilesInDir(
            boost::filesystem::path const & source,
            boost::filesystem::path const & destination
    )
    {
        namespace fs = boost::filesystem;
        try
        {
            // Check whether the function call is valid
            if(!fs::exists(source) || !fs::is_directory(source))
            {
                std::cerr << "Source directory " << source.string()
                          << " does not exist or is not a directory." << '\n';
                return false;
            }
            if(!fs::exists(destination) || !fs::is_directory(destination))
            {
                std::cerr << "Destination directory " << destination.string()
                          << " does not exist or is not a directory." << '\n';
                return false;
            }

        }
        catch(fs::filesystem_error const & e)
        {
            std::cerr << e.what() << '\n';
            return false;
        }
        // Iterate through the source directory
        for(
                fs::directory_iterator file(source);
                file != fs::directory_iterator(); ++file
                )
        {
            try
            {
                fs::path current(file->path());
                if(!(fs::is_directory(current)))  // Should we be checking for other things?
                {
                    // Found file: Copy
                    fs::copy_file(current, destination / current.filename());
                }
            }
            catch(fs::filesystem_error const & e)
            {
                std:: cerr << e.what() << '\n';
            }
        }
        return true;
    }



    
public:
    WDSOptimiser( WDSOptParameters& _params)
    : params(_params),
      eval_count(0),
      num_constraints(1),
      analysis_id(-1),
      real_lowerbounds(0),
      real_upperbounds(0),
      int_lowerbounds(0),
      int_upperbounds(0),
      minimise_or_maximise(1, MINIMISATION)
    {
            // Copy project directory into working directory
            std::string temp_dir_template = "MDSOpt_worker" + std::to_string(params.evaluator_id) + "_%%%%-%%%%";
            params.working_dir.second = boost::filesystem::unique_path(params.working_dir.second / temp_dir_template);
            params.working_dir.first = params.working_dir.second.string();
            copyDir(params.template_project_dir.second, params.working_dir.second);

        // get paths of important files in working directory.
        working_project = params.working_dir.second / params.rel_path_opt;


        analysis_id = createAnalysis(working_project.string().c_str());
        int numDVs;
        int* dv_bounds = getDVBounds(analysis_id, &numDVs);
        this->int_upperbounds.resize(numDVs); // Dont know if this is neccesary. Assign might resize accordingly.
        this->int_upperbounds.assign(dv_bounds, dv_bounds+numDVs);
        this->int_lowerbounds.resize(numDVs);
        this->int_lowerbounds.assign(numDVs,0);
        this->real_lowerbounds.resize(0);
        this->real_upperbounds.resize(0);
        this->minimise_or_maximise.resize(2, MINIMISATION);

        // Make the problem defintions and intialise the objectives and constraints struct.
        prob_defs.reset(new ProblemDefinitions(real_lowerbounds, real_upperbounds, int_lowerbounds, int_upperbounds, minimise_or_maximise, num_constraints));
        objectives_and_constrataints.first.resize(2);
        objectives_and_constrataints.second.resize(1);

    }
    
    ~WDSOptimiser()
    {
        if (boost::filesystem::exists(params.working_dir.second))
        {
            std::this_thread::sleep_for (std::chrono::seconds(1));
            boost::filesystem::remove_all(params.working_dir.second);
        }
    }

    void
    runEPANET(std::ofstream & logging_file, std::vector<int> dv_vals)
    {
        int err_code = runEN(this->analysis_id, dv_vals.data());
    }
    
    

    void
    calcObjectivesAndConstraints(std::ofstream & logging_file)
    {
        objectives_and_constrataints.first[0] = getPipeCapitalCost(analysis_id);
        objectives_and_constrataints.first[1] = getNetworkResilience(analysis_id);
        objectives_and_constrataints.second[0] = getSumHeadTooHigh(analysis_id);
    }


    std::pair<std::vector<double>, std::vector<double> > &
    operator()(const std::vector<double>  & real_decision_vars, const std::vector<int> & int_decision_vars,
               bool do_save = false, boost::filesystem::path save_path = "no_path")
    {
        boost::filesystem::path initial_path = boost::filesystem::current_path();
        boost::filesystem::current_path(params.working_dir.second);
        std::string filename = "logWorker" + std::to_string(params.evaluator_id)
                               + "_EvalNo" + std::to_string(eval_count) + "_"
                               + boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time()) + ".log";
        logfile = params.save_dir.second / filename;
        std::ofstream logging_file;
        if (params.is_logging)
        {
            logging_file.open(logfile.c_str(), std::ios_base::app);
            if (!logging_file.is_open())
            {
                params.is_logging = false;
                std::cout << "attempt to log failed\n";
            }
        }

        runEPANET(logging_file, int_decision_vars);
        calcObjectivesAndConstraints(logging_file);

            
            if (do_save)
            {
                if (save_path == "no_path")
                {
                    boost::filesystem::path save_directory_p = working_project.parent_path().filename();
                    std::string save_directory_s = save_directory_p.string() + "_save";
                    save_path = working_project.parent_path().parent_path() / save_directory_s.c_str();
                }

                //            if (!boost::filesystem::exists(save_replicate_path)) boost::filesystem::create_directory(save_replicate_path);
                if (boost::filesystem::exists(save_path)) boost::filesystem::remove_all(save_path);
                copyDir(params.working_dir.second, save_path);
            }

            if (do_save)
            {
                // print value of each replicate objectives.
                //TODO: SAve the inp file to the save directory

                //TODO: SAve obj and constraint values to save directory

            }

        
        ++eval_count;
        
        if (params.is_logging) logging_file.close();
        
        boost::filesystem::remove_all(previous_logfile);
        previous_logfile = logfile;
        
        boost::filesystem::current_path(initial_path);

        return (objectives_and_constrataints);
    }



    std::pair<std::vector<double>, std::vector<double> > &
    operator()(const std::vector<double>  & real_decision_vars, const std::vector<int> & int_decision_vars)
    {
        this->operator()(real_decision_vars, int_decision_vars, false, "no_path");
        return (objectives_and_constrataints);
    }
    
    ProblemDefinitionsSPtr getProblemDefinitions()
    {
        return (prob_defs);
    }


    
};


//TODO:
// 1. manipulate geoproject with seed numbers
// 2. Masks for objective maps to exlcude areas of summing up


#endif /* ZonalPolicyOptimiser_hpp */
