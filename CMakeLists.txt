 
cmake_minimum_required(VERSION 3.0)
PROJECT(WDS-Opt)
set (CMAKE_CXX_STANDARD 11)
#Variable for location of parallel-nsgaII-backend

FIND_PACKAGE(Boost REQUIRED COMPONENTS filesystem system program_options mpi serialization timer chrono date_time)
find_package(GDAL)
find_package(MPI REQUIRED)
include_directories(${MPI_CXX_INCLUDE_PATH})

SET(USE_VTK CACHE BOOL "Set to on to use vtk visualisation")
IF (USE_VTK)
        find_package(VTK REQUIRED)
                IF(VTK_FOUND)
                include(${VTK_USE_FILE})
                add_definitions(-DWITH_VTK)
                ENDIF()
ENDIF()


set(ENLINK "Not Specified" CACHE PATH "Path to ENLink root directory (source code)")
set(PARALLEL-NSGAII "Not Specified" CACHE PATH "Path to Jeff's parallel NSGAII code")

# Define required for how hypervolume is calculated.
add_definitions(-DVARIANT=4)


include_directories(${Boost_INCLUDE_DIRS} )
INCLUDE_DIRECTORIES(${ENLINK})
INCLUDE_DIRECTORIES(${PARALLEL-NSGAII})


if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  add_definitions(-DCLANG)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  add_definitions(-DGNU)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  add_definitions(-DINTEL)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  add_definitions(-DMSVC)
endif()

add_subdirectory(${ENLINK} ${CMAKE_CURRENT_BINARY_DIR}/ENLink)

SET(prll-nsga-src
        "${PARALLEL-NSGAII}/Checkpoint.hpp"
        "${PARALLEL-NSGAII}/Comparator.hpp"
        "${PARALLEL-NSGAII}/Crossover.hpp"
        "${PARALLEL-NSGAII}/DebsCrowdingDistance.hpp"
        "${PARALLEL-NSGAII}/DebsNondominatedSorting.hpp"
        "${PARALLEL-NSGAII}/Evaluation.hpp"
        "${PARALLEL-NSGAII}/Individual.hpp"
        "${PARALLEL-NSGAII}/Merge.hpp"
        "${PARALLEL-NSGAII}/Mutation.hpp"
        "${PARALLEL-NSGAII}/NSGAII.hpp"
        "${PARALLEL-NSGAII}/ParallelEvaluator.hpp"
        "${PARALLEL-NSGAII}/Population.hpp"
        "${PARALLEL-NSGAII}/Population.cpp"
        "${PARALLEL-NSGAII}/ProblemDefinitions.hpp"
        "${PARALLEL-NSGAII}/Selection.hpp"
        "${PARALLEL-NSGAII}/Types.hpp"

        "${PARALLEL-NSGAII}/Checkpoints/MaxGenCheckpoint.hpp"
        "${PARALLEL-NSGAII}/Checkpoints/SavePopCheckpoint.hpp"
        "${PARALLEL-NSGAII}/Checkpoints/SerialiseCheckpoint.hpp"
        "${PARALLEL-NSGAII}/Checkpoints/PlotFronts.hpp"
        "${PARALLEL-NSGAII}/Checkpoints/colours.hpp"
        "${PARALLEL-NSGAII}/Checkpoints/ResetMutationXoverFlags.hpp"
        "${PARALLEL-NSGAII}/Checkpoints/MailCheckpoint.hpp"
        "${PARALLEL-NSGAII}/Checkpoints/SaveFirstFrontCheckpoint.hpp"
        "${PARALLEL-NSGAII}/Checkpoints/SignalCheckpoint.hpp"
        "${PARALLEL-NSGAII}/Checkpoints/SignalHandler.cpp"
        "${PARALLEL-NSGAII}/Checkpoints/SignalHandler.hpp"

        "${PARALLEL-NSGAII}/Serialization/SerializeBoostPath.hpp"
        "${PARALLEL-NSGAII}/Serialization/SerializeRng.hpp"

        "${PARALLEL-NSGAII}/Metrics/hv.c"
        "${PARALLEL-NSGAII}/Metrics/hv.h"
        "${PARALLEL-NSGAII}/Metrics/Hypervolume.hpp"
    )


link_directories(${Boost_LIBRARY_DIRS})


##################################################################################################################
#                                                                                                                #
#                 Parallel NSGAII for optimising DWS              #
#                                                                                                                #
#                                                                                                                #
##################################################################################################################

SET(parrallel_wds_opt
        MainPar.cpp
        WDSOptimiser.hpp
        WDSUtility.hpp
        Pathify.hpp
    )

add_executable(wdsoptpar ${parrallel_wds_opt} ${prll-nsga-src} )
target_link_libraries(wdsoptpar  ${Boost_LIBRARIES} ${MPI_LIBRARIES} ENLink)

IF(USE_VTK)
    if(VTK_LIBRARIES)
        target_link_libraries(wdsoptpar ${VTK_LIBRARIES})
    else()
        target_link_libraries(wdsoptpar vtkHybrid vtkWidgets)
    endif()
ENDIF()


##################################################################################################################
#                                                                                                                #
#                 Serial NSGAII for optimising WDS                 #
#                                                                                                                #
#                                                                                                                #
##################################################################################################################


SET(serial_wds_opt
        MainSer.cpp
        WDSOptimiser.hpp
        WDSUtility.hpp
        Pathify.hpp
    )



add_executable(wdsoptser ${serial_wds_opt} ${prll-nsga-src} )
target_link_libraries(wdsoptser  ${Boost_LIBRARIES} ENLink)

IF(USE_VTK)
    if(VTK_LIBRARIES)
        target_link_libraries(wdsoptser ${VTK_LIBRARIES})
    else()
        target_link_libraries(wdsoptser vtkHybrid vtkWidgets)
    endif()
ENDIF()



