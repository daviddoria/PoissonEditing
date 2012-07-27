cmake_minimum_required(VERSION 2.6)

PROJECT(PoissonEditing)
ENABLE_TESTING()

# needed to allow default template arguments in function templates
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x")

# Where to copy executables when 'make install' is run
SET( INSTALL_DIR ${CMAKE_INSTALL_PREFIX} )

#### ITK ####
FIND_PACKAGE(ITK 4 REQUIRED  ITKCommon ITKIOImageBase ITKDistanceMap ITKIOPNG ITKIOMeta
ITKImageIntensity ITKImageFeature ITKMathematicalMorphology ITKBinaryMathematicalMorphology)
INCLUDE(${ITK_USE_FILE})

#### Eigen and UMFPACK ####

# Tell CMake to also look in the source directory to find some .cmake files (Eigen3 and Umfpack)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_MODULE_PATH})

# On Fedora (15) make sure to install the packages suitesparse* to get UMFPACK
# On Ubuntu (11.4) make sure to install the packages libsuitesparse-dev to get UMFPACK

# Set the CMake variable to the value of the environment variable.
# If the build is absolutely fresh, this will be the case if the CMake variable EIGEN3_INCLUDE_DIR is not set.
if(NOT EIGEN3_INCLUDE_DIR)
  set(EIGEN3_INCLUDE_DIR $ENV{EIGEN3_INCLUDE_DIR})
  message("Set EIGEN3_INCLUDE_DIR to ${EIGEN3_INCLUDE_DIR} from environment variable of the same name.")
else()
  message("EIGEN3_INCLUDE_DIR is ${EIGEN3_INCLUDE_DIR}")
endif()

FIND_PACKAGE(Eigen3 REQUIRED) #requires FindEigen3.cmake to be in the source directory
include_directories(${EIGEN3_INCLUDE_DIR})

#UMFPACK
# TODO: Currently if this find_package occurs in a parent directory, a strange libamd error occurs.
#if(NOT UMFPACK_LIBRARIES) # Maybe this guard fixes the above error?
  FIND_PACKAGE(Umfpack REQUIRED)
  include_directories(${UMFPACK_INCLUDES})
#endif()

# Submodules
if(NOT TARGET Mask)
  add_subdirectory(Mask)
  include_directories(${Mask_includes})
endif()

if(NOT ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  set(PoissonEditing_includes ${Mask_includes} ${EIGEN3_INCLUDE_DIR} ${UMFPACK_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
endif()

# Even though this is a header-only libary, create a library so the linking can be done more easily in parents.
add_custom_target(PoissonEditing PoissonEditing.hpp)
#target_link_libraries(PoissonEditing Mask QtHelpers umfpack)

SET(PoissonEditing_BuildTests OFF CACHE BOOL "PoissonEditing_BuildTests")
if(PoissonEditing_BuildTests)
  add_subdirectory(Testing)
endif()

SET(PoissonEditing_BuildDrivers OFF CACHE BOOL "PoissonEditing_BuildDrivers")
if(PoissonEditing_BuildDrivers)
  add_subdirectory(Drivers)
endif()
