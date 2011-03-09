cmake_minimum_required(VERSION 2.6)

PROJECT(PoissonEditingVNL)

FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${ITK_USE_FILE})

FIND_PACKAGE(VXL REQUIRED)
INCLUDE(${VXL_CMAKE_DIR}/UseVXL.cmake)

# Eigen and UMFPACK

include_directories(/media/portable/src/Eigen /media/portable/src/Eigen/unsupported)

set(CMAKE_MODULE_PATH /media/portable/src/Eigen/cmake)

FIND_PACKAGE(Umfpack REQUIRED)
FIND_PACKAGE(BLAS REQUIRED)
include_directories(${UMFPACK_INCLUDES})
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DEIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET")

INCLUDE_DIRECTORIES(/home/doriad/src/vxl/v3p/netlib/)
#
# ADD_EXECUTABLE(PoissonEditing PoissonEditingExample.cpp Helpers.cpp Variable.cpp)
# TARGET_LINK_LIBRARIES(PoissonEditing ${ITK_LIBRARIES} vnl)
#
# ADD_EXECUTABLE(PoissonCloning PoissonCloningExample.cpp Helpers.cpp Variable.cpp)
# TARGET_LINK_LIBRARIES(PoissonCloning ${ITK_LIBRARIES} vnl)

ADD_EXECUTABLE(DerivativeToImage DerivativeToImageExample.cpp Helpers.cpp Variable.cpp)
TARGET_LINK_LIBRARIES(DerivativeToImage ${ITK_LIBRARIES} vnl umfpack)