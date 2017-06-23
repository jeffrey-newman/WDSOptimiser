//
//  ZonalPolicyUtility.hpp
//  ZonalOptimiser
//
//  Created by a1091793 on 5/10/2016.
//  Copyright © 2016 University of Adelaide and Bushfire and Natural Hazards CRC. All rights reserved.
//

#ifndef WDSUtility_hpp
#define WDSUtility_hpp

#include "Pathify.hpp"

struct WDSOptParameters
{
    CmdLinePaths template_project_dir; // path to template project
    CmdLinePaths working_dir;
    CmdLinePaths save_dir;
    CmdLinePaths opt_file; // relative path of opt file from template project directory)
    bool is_logging = false;
    int pop_size; // For the GA
    int max_gen_hvol;  // Termination condition for the GA
    int mail_hvol_gen;
    int max_gen;
    int save_freq;
    int evaluator_id = 0;
    CmdLinePaths restart_pop_file;

    bool cost_calculated;
    bool resilience_calculated;
    bool pressure_constraint;
    bool head_constraint;
    bool velocity_constraint;

    WDSOptParameters();


};

int
processOptions(int argc, char * argv[], WDSOptParameters & params);





#endif /* WDSUtility_hpp */
