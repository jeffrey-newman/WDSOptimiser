include_directories(${Boost_INCLUDE_DIRS} )
INCLUDE_DIRECTORIES(${ENLINK})
INCLUDE_DIRECTORIES(${PARALLEL-NSGAII})

# Define required for how hypervolume is calculated.
add_definitions(-DVARIANT=4)

IF (USE_QT)
    # Find includes in corresponding build directories
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    find_package(Qt5GUI)
    find_package(Qt5Charts)
    find_package(Qt5Widgets)
    find_package(Qt5Core)
    add_definitions(-DWITH_QT)
ENDIF()

SET(WDS_Optimiser_GUI_SRCS
        TimeSeriesQtChart.h
        TimeSeriesQtChart.cpp
        main.cpp
        nsgaiirunningpage.cpp
        nsgaiirunningpage.h
        optimisationwizardpage.cpp
        optimisationwizardpage.h
        optimiserthread.cpp
        optimiserthread.h
        MetricLinePlotQtSignal.cpp
        MetricLinePlotQtSignal.h
        WDSOptGUICheckpoints.cpp
        WDSOptGUICheckpoints.h
#        nsgaiiguimainwindow.ui
#        optimisationwizardpage.ui
)



add_executable(wdsoptgui ${WDS_Optimiser_GUI_SRCS} )
target_link_libraries(wdsoptgui ${Boost_LIBRARIES} ENLink Qt5::Widgets Qt5::Gui Qt5::Charts wdsoptcommon)
#IF(USE_VTK)
#    if(VTK_LIBRARIES)
#        target_link_libraries(wdsoptgui ${VTK_LIBRARIES})
#    else()
#        target_link_libraries(wdsoptgui vtkHybrid vtkWidgets)
#    endif()
#ENDIF()