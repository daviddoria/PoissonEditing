cmake_minimum_required(VERSION 2.6)

PROJECT(PoissonEditing)
ENABLE_TESTING()

# Where to copy executables when 'make install' is run
SET( INSTALL_DIR ${CMAKE_INSTALL_PREFIX} )

##### QT ######
#INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})

#### ITK ####
FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${ITK_USE_FILE})

# Eigen and UMFPACK

# Tell CMake to also look in the source directory to find some .cmake files (Eigen3 and Umfpack)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR} ${CMAKE_MODULE_PATH})

#You must set the EIGEN3_INCLUDE_DIR in the 'Advanced' CMake options to /home/doriad/src/eigen or wherever you have extracted Eigen3.

# On Fedora (15) make sure to install the packages suitesparse* to get UMFPACK
# On Ubuntu (11.4) make sure to install the packages libsuitesparse-dev to get UMFPACK

FIND_PACKAGE(Eigen3 REQUIRED) #requires FindEigen3.cmake to be in the source directory

include_directories(${EIGEN3_INCLUDE_DIR} ${EIGEN3_INCLUDE_DIR}/unsupported/)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DEIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET")

#UMFPACK
FIND_PACKAGE(Umfpack REQUIRED)
include_directories(${UMFPACK_INCLUDES})
include_directories(${EIGEN3_INCLUDE_DIR}/unsupported/)

QT4_WRAP_UI(FileSelectorUISrcs FileSelector.ui FileSelectionWidget.ui)
QT4_WRAP_CPP(FileSelectorMOCSrcs FileSelector.h FileSelectionWidget.h)
add_library(FileSelectorLib FileSelector.cpp FileSelectionWidget.cpp ${FileSelectorUISrcs} ${FileSelectorMOCSrcs})

SET(PoissonEditingNonInteractive ON CACHE BOOL "Build the Poisson editing non interactive version.")
if(PoissonEditingNonInteractive)
  ADD_EXECUTABLE(PoissonEditingNonInteractive PoissonEditingNonInteractive.cpp Helpers.cpp IndexComparison.cpp)
  TARGET_LINK_LIBRARIES(PoissonEditingNonInteractive ${ITK_LIBRARIES} umfpack)
  #set_target_properties(PoissonEditing PROPERTIES COMPILE_DEFINITIONS "${DEFINITIONS}")
  INSTALL( TARGETS PoissonEditingNonInteractive RUNTIME DESTINATION ${INSTALL_DIR} )
  message("EXECUTABLE_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH}")
  #ADD_TEST(NAME PoissonEditingTest COMMAND ${EXECUTABLE_OUTPUT_PATH}/PoissonEditing ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Source.png ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Mask.png editing.png)
  #ADD_TEST(PoissonEditingTest ${EXECUTABLE_OUTPUT_PATH}/PoissonEditing ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Source.png ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Mask.png PoissonEditingOutput.mhd)
endif()

SET(PoissonEditingInteractive CACHE BOOL "Build the Poisson editing interactive version")
if(PoissonEditingInteractive)
  QT4_WRAP_UI(PoissonEditingUISrcs PoissonEditingGUI.ui)
  QT4_WRAP_CPP(PoissonEditingMOCSrcs PoissonEditingGUI.h)

  ADD_EXECUTABLE(PoissonEditingInteractive PoissonEditingInteractive.cpp PoissonEditingGUI.cxx Helpers.cpp HelpersQt.cpp Mask.cpp IndexComparison.cpp ${PoissonEditingUISrcs} ${PoissonEditingMOCSrcs})
  TARGET_LINK_LIBRARIES(PoissonEditingInteractive ${ITK_LIBRARIES} umfpack ${QT_LIBRARIES} FileSelectorLib)
  #set_target_properties(PoissonCloning PROPERTIES COMPILE_DEFINITIONS "${DEFINITIONS}")
  INSTALL( TARGETS PoissonEditingInteractive RUNTIME DESTINATION ${INSTALL_DIR} )
  #ADD_TEST(NAME PoissonCloningTest COMMAND ${EXECUTABLE_OUTPUT_PATH}/PoissonCloning ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Source.png ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Mask.png ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Target.png cloning.png)
endif()

SET(PoissonCloningNonInteractive ON CACHE BOOL "Build the Poisson cloning non interactive version")
if(PoissonCloningNonInteractive)
  ADD_EXECUTABLE(PoissonCloningNonInteractive PoissonCloningNonInteractive.cpp Helpers.cpp IndexComparison.cpp)
  TARGET_LINK_LIBRARIES(PoissonCloningNonInteractive ${ITK_LIBRARIES} umfpack)
  #set_target_properties(PoissonCloning PROPERTIES COMPILE_DEFINITIONS "${DEFINITIONS}")
  INSTALL( TARGETS PoissonCloningNonInteractive RUNTIME DESTINATION ${INSTALL_DIR} )
  #ADD_TEST(NAME PoissonCloningTest COMMAND ${EXECUTABLE_OUTPUT_PATH}/PoissonCloning ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Source.png ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Mask.png ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Target.png cloning.png)
endif()

SET(PoissonCloningInteractive CACHE BOOL "Build the Poisson cloning interactive version")
if(PoissonCloningInteractive)
  ADD_EXECUTABLE(PoissonCloningInteractive PoissonCloningInteractive.cpp Helpers.cpp IndexComparison.cpp)
  TARGET_LINK_LIBRARIES(PoissonCloningInteractive ${ITK_LIBRARIES} umfpack ${QT_LIBRARIES})
  #set_target_properties(PoissonCloning PROPERTIES COMPILE_DEFINITIONS "${DEFINITIONS}")
  INSTALL( TARGETS PoissonCloningInteractive RUNTIME DESTINATION ${INSTALL_DIR} )
  #ADD_TEST(NAME PoissonCloningTest COMMAND ${EXECUTABLE_OUTPUT_PATH}/PoissonCloning ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Source.png ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Mask.png ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Target.png cloning.png)
endif()

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
