cmake_minimum_required(VERSION 2.6)

PROJECT(PoissonEditing)
ENABLE_TESTING()

# needed to allow default template arguments in function templates
if(UNIX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x")
endif()

# Where to copy executables when 'make install' is run
SET( INSTALL_DIR ${CMAKE_INSTALL_PREFIX} )

#### ITK ####
if(NOT ITK_FOUND)
  FIND_PACKAGE(ITK 4 REQUIRED ITKCommon ITKIOImageBase ITKDistanceMap ITKIOPNG ITKIOMeta
                              ITKImageIntensity ITKImageFeature ITKMathematicalMorphology
                              ITKBinaryMathematicalMorphology)
  INCLUDE(${ITK_USE_FILE})
endif()
set(PoissonEditing_libraries ${PoissonEditing_libraries} ${ITK_LIBRARIES})

#### Eigen ####
# Tell CMake to also look in the source directory to find some .cmake files
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_MODULE_PATH})

# Set the CMake variable to the value of the environment variable.
# If the build is absolutely fresh, this will be the case if the CMake variable EIGEN3_INCLUDE_DIR is not set.
if(NOT EIGEN3_INCLUDE_DIR)
  set(EIGEN3_INCLUDE_DIR $ENV{EIGEN3_INCLUDE_DIR})
  message("Set EIGEN3_INCLUDE_DIR to ${EIGEN3_INCLUDE_DIR} from environment variable of the same name.")
else()
  message("EIGEN3_INCLUDE_DIR is ${EIGEN3_INCLUDE_DIR}")
endif()

if(NOT EIGEN3_FOUND)
  FIND_PACKAGE(Eigen3 REQUIRED) #requires FindEigen3.cmake to be in the source directory
  include_directories(${EIGEN3_INCLUDE_DIR})
endif()

# Submodules
# Only include Mask if it is not already available
get_property(FoundMask GLOBAL PROPERTY MaskIncludeDirs SET)
if(NOT FoundMask)
  add_subdirectory(Mask)
endif()

get_property(MaskIncludeDirs GLOBAL PROPERTY MaskIncludeDirs)
set(PoissonEditing_include_dirs ${PoissonEditing_include_dirs} ${MaskIncludeDirs})
get_property(MaskLibs GLOBAL PROPERTY MaskLibs)
set(PoissonEditing_libraries ${PoissonEditing_libraries} ${MaskLibs})

# Give the compiler all of the required include directories
include_directories(${PoissonEditing_include_dirs})

# Allow this project to be detected and used as a submodule
set(PoissonEditing_include_dirs ${PoissonEditing_include_dirs} ${CMAKE_CURRENT_SOURCE_DIR})
set_property(GLOBAL PROPERTY PoissonEditingIncludeDirs ${PoissonEditing_include_dirs})
set_property(GLOBAL PROPERTY PoissonEditingLibs ${PoissonEditing_libraries})

# Build the tests if requested.
option(PoissonEditing_BuildTests "PoissonEditing_BuildTests" OFF)
if(PoissonEditing_BuildTests)
  add_subdirectory(Testing)
endif()

# Build the drivers if requested.
option(PoissonEditing_BuildDrivers "PoissonEditing_BuildDrivers" OFF)
if(PoissonEditing_BuildDrivers)
  add_subdirectory(Drivers)
endif()

# Add non-compiled files to the project
add_custom_target(PoissonEditingSources SOURCES
PoissonEditing.h
PoissonEditing.hpp
PoissonEditingNeumann.h
PoissonEditingNeumann.hpp
PoissonEditingVariational.h
PoissonEditingVariational.hpp
)

# Display where this code was used from (if it is used as a submodule, there may be multiple instances of this submodule in the project, only the first of which is used)
option(PoissonEditing_ShowSubmoduleLocation "Show the path from which PoissonEditing was used?" OFF)
if(PoissonEditing_ShowSubmoduleLocation)
  message("PoissonEditing used from ${CMAKE_CURRENT_SOURCE_DIR}")
endif(PoissonEditing_ShowSubmoduleLocation)
