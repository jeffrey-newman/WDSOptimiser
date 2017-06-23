/*
//
// Created by a1091793 on 12/6/17.
//
 */

//
//  ZonalPolicyOptimiser.hpp
//  ZonalOptimiser
//
//  Created by a1091793 on 4/10/2016.
//  Copyright © 2016 University of Adelaide and Bushfire and Natural Hazards CRC. All rights reserved.
//

#include "WDSOptimiser.hpp"

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


#include "WDSOptParameters.hpp"
#include "ENLink.h"


    //Copies entire directory - so that each geoproject is running in a different directory.
    bool WDSOptimiser::copyDir(   boost::filesystem::path const & source,
                    boost::filesystem::path const & destination,
                    std::ostream & logging)
    {
        if(params.is_logging) logging << "Copying directory " << source << " to " << destination << std::endl;
        namespace fs = boost::filesystem;
        try
        {
            // Check whether the function call is valid
            if(!fs::exists(source) || !fs::is_directory(source))
            {
                if(params.is_logging) logging << "Source directory " << source.string()
                                              << " does not exist or is not a directory." << '\n';
                return false;
            }
            if(fs::exists(destination))
            {
                if(params.is_logging) logging << "Destination directory " << destination.string()
                                              << " already exists." << '\n';
                return false;
            }
            // Create the destination directory
            fs::copy_directory(source, destination);
            if(!fs::exists(destination) || !fs::is_directory(destination))
            {
                if(params.is_logging) logging << "Unable to create destination directory"
                                              << destination.string() << '\n';
                return false;
            }
        }
        catch(fs::filesystem_error const & e)
        {
            if(params.is_logging) logging << e.what() << '\n';
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
                if(params.is_logging) logging << "descending into: " << current << std::endl;
                if(fs::is_directory(current))
                {
                    // Found directory: Recursion
                    if(!copyDir(current, destination / current.filename(), logging))
                    {
                        return false;
                    }
                }
                else
                {
                    // Found file: Copy
                    if(params.is_logging) logging << "Copying " << current << " to " << destination / current.filename() << std::endl;
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
    bool WDSOptimiser::copyFilesInDir(
            boost::filesystem::path const & source,
            boost::filesystem::path const & destination,
            std::ostream & logging
    )
    {
        if(params.is_logging) logging << "Copying files in " << source << " to " << destination << std::endl;
        namespace fs = boost::filesystem;
        try
        {
            // Check whether the function call is valid
            if(!fs::exists(source) || !fs::is_directory(source))
            {
                if(params.is_logging) logging << "Source directory " << source.string()
                                              << " does not exist or is not a directory." << '\n';
                return false;
            }
            if(!fs::exists(destination) || !fs::is_directory(destination))
            {
                if(params.is_logging) logging << "Destination directory " << destination.string()
                                              << " does not exist or is not a directory." << '\n';
                return false;
            }

        }
        catch(fs::filesystem_error const & e)
        {
            if(params.is_logging) logging << e.what() << '\n';
            return false;
        }
        // Iterate through the source directory
        for(fs::directory_iterator file(source); file != fs::directory_iterator(); ++file )
        {
            try
            {
                fs::path current(file->path());
                if(!(fs::is_directory(current)))  // Should we be checking for other things?
                {
                    // Found file: Copy
                    if(params.is_logging) logging << "Copying " << current << " to " << destination / current.filename() << std::endl;
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

    //Reads an entire file into string in memory
    std::string
    WDSOptimiser::readfile(std::string file_name)
    {
        std::ifstream t(file_name.c_str());
        t.seekg(0, std::ios::end);
        size_t size = t.tellg();
        std::string buffer(size, ' ');
        t.seekg(0);
        t.read(&buffer[0], size);
        return buffer;
    }





    WDSOptimiser::WDSOptimiser( WDSOptParameters& _params)
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
        std::ofstream logging_file;
        if (params.is_logging)
        {
            std::string filename = "logWorker" + std::to_string(params.evaluator_id)
                                   + "_Constructor_" + boost::posix_time::to_simple_string(
                    boost::posix_time::second_clock::local_time()) +
                                   ".log";
            this->logfile = params.save_dir.second / filename;

            logging_file.open(this->logfile.string().c_str(), std::ios_base::app);
            if (!logging_file.is_open())
            {
                params.is_logging = false;
                std::cout << "attempt to log failed\n";
            }
        }
        boost::scoped_ptr<boost::timer::auto_cpu_timer> t(nullptr);
        if (params.is_logging) t.reset(new boost::timer::auto_cpu_timer(logging_file));

        // Copy project directory into working directory
        std::string temp_dir_template = "WDSOpt_worker" + std::to_string(params.evaluator_id) + "_%%%%-%%%%";
        params.working_dir.second = boost::filesystem::unique_path(params.working_dir.second / temp_dir_template);
        params.working_dir.first = params.working_dir.second.string();
        if (!(boost::filesystem::exists(params.working_dir.second)))
        {
            boost::filesystem::create_directories(params.working_dir.second);
        }
        boost::filesystem::path copy_to = params.working_dir.second / params.opt_file.second.filename();
        boost::filesystem::copy_file(params.opt_file.second, copy_to);


        // get paths of important files in working directory.
        params.opt_file.second = copy_to;


        analysis_id = createAnalysisOveride(params.opt_file.second.string().c_str(), params.working_dir.second.string().c_str(), "", "", 0);
        if (analysis_id == -1)
        {
            std::cerr << "Intialisation failed. Check path of .opt file\n";
            throw std::runtime_error("Invalid .opt file");
        }
        int numDVs;
        int* dv_bounds = getDVBounds(analysis_id, &numDVs);
        this->int_upperbounds.resize(numDVs); // Dont know if this is neccesary. Assign might resize accordingly.
        this->int_upperbounds.assign(dv_bounds, dv_bounds+numDVs);
        this->int_lowerbounds.resize(numDVs);
        this->int_lowerbounds.assign(numDVs,0);
        this->real_lowerbounds.resize(0);
        this->real_upperbounds.resize(0);

        // Make the problem defintions and intialise the objectives and constraints struct.
        prob_defs.reset(new ProblemDefinitions(real_lowerbounds, real_upperbounds, int_lowerbounds, int_upperbounds, minimise_or_maximise, num_constraints));
        int num_objectives = 0;
        if (isPipeCapitalCostCalculated(analysis_id))
        {
            params.cost_calculated = true;
            this->minimise_or_maximise.push_back(MINIMISATION);
            ++num_objectives;
        }
        if(isNetworkResilienceCalculated(analysis_id))
        {
            params.resilience_calculated = true;
            this->minimise_or_maximise.push_back(MAXIMISATION);
            ++num_objectives;
        }
        int num_constraints = 0;
        if (isPressureViolationCalculated(analysis_id))
        {
            params.pressure_constraint = true;
            ++num_constraints;
        }
        if (isHeadViolationCalculated(analysis_id))
        {
            params.head_constraint = true;
            ++num_constraints;
        }
        if (isVelocityViolationCalculated(analysis_id))
        {
            params.velocity_constraint = true;
            ++num_constraints;
        }
        objectives_and_constrataints.first.resize(num_objectives);
        objectives_and_constrataints.second.resize(num_constraints);

        if (params.is_logging)
        {
            logging_file.close();
        }

    }

    WDSOptimiser::~WDSOptimiser()
    {
        if (boost::filesystem::exists(params.working_dir.second))
        {
            std::this_thread::sleep_for (std::chrono::seconds(1));
            boost::filesystem::remove_all(params.working_dir.second);
        }
    }

    void
    WDSOptimiser::runEPANETCalcObjectivesAndConstraints(std::ofstream & logging_file, std::vector<int> dv_vals)
    {
        int err_code = runEN(this->analysis_id, dv_vals.data());
        int obj_index = 0;
        if (isPipeCapitalCostCalculated(analysis_id))
        {
            objectives_and_constrataints.first[obj_index] = getPipeCapitalCost(analysis_id);
            ++obj_index;
        }
        if(isNetworkResilienceCalculated(analysis_id))
        {
            objectives_and_constrataints.first[obj_index] = getNetworkResilience(analysis_id);
            ++obj_index;
        }
        int constraint_index = 0;
        if (isPressureViolationCalculated(analysis_id))
        {
            objectives_and_constrataints.second[constraint_index] = getSumPressureTooHigh(analysis_id)
                                                                    + getSumPressureTooLow(analysis_id);
            ++constraint_index;
        }
        if (isHeadViolationCalculated(analysis_id))
        {
            objectives_and_constrataints.second[constraint_index] = getSumHeadTooHigh(analysis_id)
                                                                    + getSumHeadTooLow(analysis_id);
            ++constraint_index;
        }
        if (isVelocityViolationCalculated(analysis_id))
        {
            objectives_and_constrataints.second[constraint_index] = getSumVelocityTooHigh(analysis_id);
            ++constraint_index;
        }
    }


    std::pair<std::vector<double>, std::vector<double> > &
    WDSOptimiser::operator()(const std::vector<double>  & real_decision_vars, const std::vector<int> & int_decision_vars,
               boost::filesystem::path save_path, boost::filesystem::path _logfile)
    {
        boost::filesystem::path initial_path = boost::filesystem::current_path();
        boost::filesystem::current_path(params.working_dir.second);

        std::ofstream logging_file;
        if (params.is_logging)
        {
            if ( _logfile == "unspecified")
            {
                std::string filename = "logWorker" + std::to_string(params.evaluator_id)
                                       + "_EvalNo" + std::to_string(eval_count) + "_"
                                       +
                                       boost::posix_time::to_simple_string(
                                               boost::posix_time::second_clock::local_time()) +
                                       ".log";
                this->logfile = params.save_dir.second / filename;
            }
            else
            {
                this->logfile = _logfile;
            }

            logging_file.open(this->logfile.string().c_str(), std::ios_base::app);
            if (!logging_file.is_open())
            {
                params.is_logging = false;
                std::cout << "attempt to log failed\n";
            }
            logging_file << "Evaluation started\n";
            logging_file << "number real decision vars : " << real_decision_vars.size() << "\n";

        }

        bool do_save = false;
        if (save_path != "no_path") do_save = true;
        if (do_save && !boost::filesystem::exists(save_path)) boost::filesystem::create_directory(save_path);


        boost::scoped_ptr<boost::timer::auto_cpu_timer> t(nullptr);
        if (params.is_logging) t.reset(new boost::timer::auto_cpu_timer(logging_file));

        std::ofstream objectives_fs;
        if (do_save)
        {
            boost::filesystem::path epanet_logfile = save_path / "calculation_log.log";
            doLogF(analysis_id, epanet_logfile.string().c_str());
        }

        runEPANETCalcObjectivesAndConstraints(logging_file, int_decision_vars);


        if (do_save)
        {
            // print value of each replicate objectives.
            //TODO: SAve the inp file to the save directory
            boost::filesystem::path epanet_log_file = save_path / "epanet_log.rpt";
            saveAnalysisF(analysis_id, epanet_log_file.string().c_str());

            //SAve obj and constraint values to save directory
            boost::filesystem::path objectives_file = save_path / "metrics_for_each_replicate.txt";
            objectives_fs.open(objectives_file.string().c_str());
            if (objectives_fs.is_open())
            {
                int obj_index = 0;
                if (isPipeCapitalCostCalculated(analysis_id))
                {
                    objectives_fs << "Capital cost = " << objectives_and_constrataints.first[obj_index] << "\n";
                    ++obj_index;
                }
                if(isNetworkResilienceCalculated(analysis_id))
                {
                    objectives_fs << "Resilience = " << objectives_and_constrataints.first[obj_index] << "\n";
                    ++obj_index;
                }
                int constraint_index = 0;
                if (isPressureViolationCalculated(analysis_id))
                {
                    objectives_fs << "Pressure violation = " << objectives_and_constrataints.second[constraint_index] << "\n";
                    ++constraint_index;
                }
                if (isHeadViolationCalculated(analysis_id))
                {
                    objectives_fs << "Head violation = " << objectives_and_constrataints.second[constraint_index] << "\n";
                    ++constraint_index;
                }
                if (isVelocityViolationCalculated(analysis_id))
                {
                    objectives_fs << "Velocity violation = " << objectives_and_constrataints.second[constraint_index] << "\n";
                    ++constraint_index;
                }
                objectives_fs.close();
            }
        }


        ++eval_count;

        if (params.is_logging) logging_file.close();

        boost::filesystem::remove_all(previous_logfile);
        previous_logfile = logfile;

        boost::filesystem::current_path(initial_path);

        return (objectives_and_constrataints);
    }



    std::pair<std::vector<double>, std::vector<double> > &
    WDSOptimiser::operator()(const std::vector<double>  & real_decision_vars, const std::vector<int> & int_decision_vars)
    {
        boost::filesystem::path save_path = "no_path";
        boost::filesystem::path logfile = "unspecified";
        this->operator()(real_decision_vars, int_decision_vars, save_path, logfile);
        return (objectives_and_constrataints);
    }

    ProblemDefinitionsSPtr
    WDSOptimiser::getProblemDefinitions()
    {
        return (prob_defs);
    }




