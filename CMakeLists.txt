cmake_minimum_required(VERSION 2.6)

PROJECT(PoissonEditing)
ENABLE_TESTING()

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x") # needed to allow default template arguments in function templates

# Where to copy executables when 'make install' is run
SET( INSTALL_DIR ${CMAKE_INSTALL_PREFIX} )

#### ITK ####
FIND_PACKAGE(ITK 4 REQUIRED  ITKCommon ITKIOImageBase ITKDistanceMap
ITKImageIntensity ITKImageFeature ITKMathematicalMorphology ITKBinaryMathematicalMorphology)
INCLUDE(${ITK_USE_FILE})

# Let our GUI directory find our core files
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

#### Eigen and UMFPACK ####

# Tell CMake to also look in the source directory to find some .cmake files (Eigen3 and Umfpack)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR} ${CMAKE_MODULE_PATH})

# On Fedora (15) make sure to install the packages suitesparse* to get UMFPACK
# On Ubuntu (11.4) make sure to install the packages libsuitesparse-dev to get UMFPACK

FIND_PACKAGE(Eigen3 REQUIRED) #requires FindEigen3.cmake to be in the source directory

include_directories(${EIGEN3_INCLUDE_DIR} ${EIGEN3_INCLUDE_DIR}/unsupported/)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DEIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET")

#UMFPACK
FIND_PACKAGE(Umfpack REQUIRED)
include_directories(${UMFPACK_INCLUDES})

# Submodules
if(NOT TARGET Mask)
  add_subdirectory(Mask)
  include_directories(${Mask_includes})
endif()

# GUI
SET(PoissonEditing_BuildGUI ON CACHE BOOL "Build GUI?")
if(PoissonEditing_BuildGUI)
  add_subdirectory(GUI)
endif()

# ADD_EXECUTABLE(PoissonFillingLaplacian PoissonFillingLaplacian.cpp)
# TARGET_LINK_LIBRARIES(PoissonFillingLaplacian ${ITK_LIBRARIES} umfpack
# libHelpers libITKHelpers libMask)
# INSTALL( TARGETS PoissonFillingLaplacian RUNTIME DESTINATION ${INSTALL_DIR} )
# 
# ADD_EXECUTABLE(PoissonFillingVariational PoissonFillingVariational.cpp)
# TARGET_LINK_LIBRARIES(PoissonFillingVariational ${ITK_LIBRARIES} umfpack
# libHelpers libITKHelpers libMask)
# INSTALL( TARGETS PoissonFillingVariational RUNTIME DESTINATION ${INSTALL_DIR} )

ADD_EXECUTABLE(PoissonFillingPoisson PoissonFillingPoisson.cpp)
TARGET_LINK_LIBRARIES(PoissonFillingPoisson ${ITK_LIBRARIES} umfpack
Mask)
INSTALL( TARGETS PoissonFillingPoisson RUNTIME DESTINATION ${INSTALL_DIR} )

# ADD_EXECUTABLE(PoissonFillingNeumann PoissonFillingNeumann.cpp)
# TARGET_LINK_LIBRARIES(PoissonFillingNeumann ${ITK_LIBRARIES} umfpack
# libHelpers libITKHelpers libMask)
# INSTALL( TARGETS PoissonFillingNeumann RUNTIME DESTINATION ${INSTALL_DIR} )

#SET(PoissonEditingNonInteractive ON CACHE BOOL "Build the Poisson editing non interactive version.")
#if(PoissonEditingNonInteractive)
  #ADD_EXECUTABLE(PoissonEditingNonInteractive PoissonEditingNonInteractive.cpp)
  #TARGET_LINK_LIBRARIES(PoissonEditingNonInteractive ${ITK_LIBRARIES} umfpack HelpersLibrary )
  #set_target_properties(PoissonEditing PROPERTIES COMPILE_DEFINITIONS "${DEFINITIONS}")
  #INSTALL( TARGETS PoissonEditingNonInteractive RUNTIME DESTINATION ${INSTALL_DIR} )
  #message("EXECUTABLE_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH}")
  #ADD_TEST(NAME PoissonEditingTest COMMAND ${EXECUTABLE_OUTPUT_PATH}/PoissonEditing ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Source.png ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Mask.png editing.png)
  #ADD_TEST(PoissonEditingTest ${EXECUTABLE_OUTPUT_PATH}/PoissonEditing ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Source.png ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Mask.png PoissonEditingOutput.mhd)
#endif()


# SET(PoissonCloningNonInteractive ON CACHE BOOL "Build the Poisson cloning non interactive version")
# if(PoissonCloningNonInteractive)
#   ADD_EXECUTABLE(PoissonCloningNonInteractive PoissonCloningNonInteractive.cpp Helpers.cpp IndexComparison.cpp)
#   TARGET_LINK_LIBRARIES(PoissonCloningNonInteractive ${ITK_LIBRARIES} umfpack HelpersLibrary )
#   #set_target_properties(PoissonCloning PROPERTIES COMPILE_DEFINITIONS "${DEFINITIONS}")
#   INSTALL( TARGETS PoissonCloningNonInteractive RUNTIME DESTINATION ${INSTALL_DIR} )
#   #ADD_TEST(NAME PoissonCloningTest COMMAND ${EXECUTABLE_OUTPUT_PATH}/PoissonCloning ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Source.png ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Mask.png ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Target.png cloning.png)
# endif()


# 
# ADD_EXECUTABLE(DerivativesToImage DerivativesToImage.cxx Helpers.cpp IndexComparison.cpp)
# TARGET_LINK_LIBRARIES(DerivativesToImage ${ITK_LIBRARIES} ${Libraries})
# set_target_properties(DerivativesToImage PROPERTIES COMPILE_DEFINITIONS "${DEFINITIONS}")
# INSTALL( TARGETS DerivativesToImage RUNTIME DESTINATION ${INSTALL_DIR} )
# 
# ADD_EXECUTABLE(LaplacianToImage LaplacianToImage.cxx Helpers.cpp IndexComparison.cpp)
# TARGET_LINK_LIBRARIES(LaplacianToImage ${ITK_LIBRARIES} umfpack)
# #set_target_properties(LaplacianToImage PROPERTIES COMPILE_DEFINITIONS "${DEFINITIONS}")
# INSTALL( TARGETS LaplacianToImage  RUNTIME DESTINATION ${INSTALL_DIR} )
# ADD_TEST(LaplacianToImageTest ${EXECUTABLE_OUTPUT_PATH}/LaplacianToImage ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Source.png ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Mask.png ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Laplacian.mhd LaplacianToImageOutput.mhd)
