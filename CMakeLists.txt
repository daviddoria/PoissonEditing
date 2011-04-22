cmake_minimum_required(VERSION 2.6)

PROJECT(PoissonEditing)

FIND_PACKAGE(ITK)
if(ITK_FOUND)
  message("Found ITK")
  INCLUDE(${ITK_USE_FILE})
endif(ITK_FOUND)

FIND_PACKAGE(VXL QUIET)
if(VXL_FOUND)
  message("Found VXL")
  INCLUDE(${VXL_CMAKE_DIR}/UseVXL.cmake)
  INCLUDE_DIRECTORIES(/home/doriad/src/vxl/v3p/netlib/)
endif(VXL_FOUND)

# Eigen and UMFPACK

# Tell CMake to also look in the ImageCompleter source directory to find some .cmake files (Eigen3, Umfpack, and FFTW)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})

# On Fedora make sure to install the packages suitesparse* to get UMFPACK
FIND_PACKAGE(Eigen3 QUIET) #requires FindEigen3.cmake to be in the source directory

if(EIGEN3_FOUND)
  include_directories(${EIGEN3_INCLUDE_DIR} ${EIGEN3_INCLUDE_DIR}/unsupported/)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DEIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET")

  #UMFPACK
  FIND_PACKAGE(Umfpack REQUIRED)
  include_directories(${UMFPACK_INCLUDES})
  include_directories(${EIGEN3_INCLUDE_DIR}/unsupported/)

  #BLAS
  FIND_PACKAGE(BLAS REQUIRED)
endif(EIGEN3_FOUND)

# Check libraries
if(NOT(EIGEN3_FOUND OR VXL_FOUND))
  message(FATAL_ERROR "Either Eigen or VNL must be found!")
else(NOT(EIGEN3_FOUND OR VXL_FOUND))
  message("Eigen: " ${EIGEN3_FOUND} " VNL: " ${VNL_FOUND})
endif(NOT(EIGEN3_FOUND OR VXL_FOUND))

# User options
if(EIGEN3_FOUND)
  set(USE_EIGEN OFF CACHE BOOL "Use Eigen?")
endif(EIGEN3_FOUND)

if(VNL_FOUND)
  set(USE_VNL OFF CACHE BOOL "Use VNL?")
endif(VNL_FOUND)

if(USE_VNL AND USE_EIGEN)
  message(FATAL_ERROR "You must choose either Eigen or VNL!")
endif(USE_VNL AND USE_EIGEN)

if(NOT(USE_VNL OR USE_EIGEN))
  message(FATAL_ERROR "You must choose either Eigen or VNL!")
endif(NOT(USE_VNL OR USE_EIGEN))

if(USE_VNL)
  set(Libraries "vnl")
endif(USE_VNL)

if(USE_EIGEN)
  set(Libraries "umfpack")
endif(USE_EIGEN)

# Setup executables
ADD_EXECUTABLE(PoissonEditing PoissonEditingExample.cpp Helpers.cpp IndexComparison.cpp)
TARGET_LINK_LIBRARIES(PoissonEditing ${ITK_LIBRARIES} ${Libraries})

ADD_EXECUTABLE(PoissonCloning PoissonCloningExample.cpp Helpers.cpp IndexComparison.cpp)
TARGET_LINK_LIBRARIES(PoissonCloning ${ITK_LIBRARIES} ${Libraries})

ADD_EXECUTABLE(DerivativesToImage DerivativesToImage.cxx Helpers.cpp IndexComparison.cpp)
TARGET_LINK_LIBRARIES(DerivativesToImage ${ITK_LIBRARIES} ${Libraries})

ADD_EXECUTABLE(LaplacianToImage LaplacianToImage.cxx Helpers.cpp IndexComparison.cpp)
TARGET_LINK_LIBRARIES(LaplacianToImage ${ITK_LIBRARIES} ${Libraries})