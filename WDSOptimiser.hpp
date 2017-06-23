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
#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include "Evaluation.hpp"
#include "WDSOptParameters.hpp"

class WDSOptimiser : public ObjectivesAndConstraintsBase
{
private:
    
    WDSOptParameters params;

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
                    boost::filesystem::path const & destination,
                    std::ostream & logging);


    //Copies entire directory - so that each geoproject is running in a different directory.
    bool copyFilesInDir(
            boost::filesystem::path const & source,
            boost::filesystem::path const & destination,
            std::ostream & logging
    );


    //Reads an entire file into string in memory
    std::string readfile(std::string file_name);




    
public:
    WDSOptimiser(WDSOptParameters& _params);

    
    ~WDSOptimiser();


    void
    runEPANETCalcObjectivesAndConstraints(std::ofstream & logging_file, std::vector<int> dv_vals);



    std::pair<std::vector<double>, std::vector<double> > &
    operator()(const std::vector<double>  & real_decision_vars, const std::vector<int> & int_decision_vars,
               boost::filesystem::path save_path = "no_path", boost::filesystem::path _logfile = "unspecified");




    std::pair<std::vector<double>, std::vector<double> > &
    operator()(const std::vector<double>  & real_decision_vars, const std::vector<int> & int_decision_vars);

    
    ProblemDefinitionsSPtr getProblemDefinitions();
    
};


//TODO:
// 1. manipulate geoproject with seed numbers
// 2. Masks for objective maps to exlcude areas of summing up


#endif /* ZonalPolicyOptimiser_hpp */
