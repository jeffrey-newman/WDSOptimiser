//
// Created by a1091793 on 12/6/17.
//

#include "WDSOptPostProcess.h"
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>

void
postProcessResults(WDSOptimiser & wds_eval, PopulationSPtr pop, WDSOptParameters & params)
{
    Population & first_front = pop->getFronts()->at(0);

    int i = 0;
    BOOST_FOREACH(IndividualSPtr ind, first_front)
                {
                    std::vector<double> objectives;
                    std::vector<double> constraints;
                    boost::filesystem::path save_ind_dir = params.save_dir.second / ("individual_" + std::to_string(i));
                    if (!boost::filesystem::exists(save_ind_dir)) boost::filesystem::create_directory(save_ind_dir);
                    std::tie(objectives, constraints) = wds_eval(ind->getRealDVVector(), ind->getIntDVVector(), save_ind_dir);
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