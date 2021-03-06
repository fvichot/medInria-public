################################################################################
#
# medInria
#
# Copyright (c) INRIA 2013. All rights reserved.
# See LICENSE.txt for details.
# 
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.
#
################################################################################

## #############################################################################
## Embed medPch
## #############################################################################

if(${PROJECT_NAME}_USE_PRECOMPILED_HEADERS)
  file(GLOB MEDPLUGINSPCH_H
    medPluginsPch/*.h
    )
  
  file(GLOB MEDPLUGINSPCH_CPP
    medPluginsPch/*.cpp
    )
endif()


## #############################################################################
## Options
## #############################################################################

option(BUILD_ALL_PLUGINS 
  "Build all Medinria plugins (overrides individual selections)" 
  ON
  )

option(BUILD_EXAMPLE_PLUGINS
  "Enable this if you want to build the examples" 
  OFF
  )

option(BUILD_COMPOSITEDATASET_PLUGIN 
  "Enable this if you want to build compositeDataSet plugin"   
  OFF
  )


## #############################################################################
## list plugins
## #############################################################################

## ADD here any new plugin with a default compilation value, 
## you want to include in the build, 
set(PLUGIN_LIST
  medVtkFibersData                          ON
  itkDataImage                              ON
  vtkDataMesh                               ON
  itkDataDiffusionGradientList              ON
  itkDataTensorImage                        ON
  medSegmentation                           ON
  itkFilters                                ON
  qtdcmDataSource                           ON
  itkDataSHImage                            ON
  undoRedoRegistration                      ON
  itkProcessRegistrationDiffeomorphicDemons ON
  medVtkView                                ON
  )   


## #############################################################################
## process plugins
## #############################################################################

## see cmake/module/add_plugin.cmake
add_plugins(${PLUGIN_LIST})

if(BUILD_COMPOSITEDATASET_PLUGIN)
  add_subdirectory(medCompositeDataSets)          
endif() 

if (BUILD_EXAMPLE_PLUGINS)
  add_subdirectory(examples)
endif()
