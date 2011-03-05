cmake_minimum_required(VERSION 2.6)

PROJECT(PoissonEditingVNL)

FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${ITK_USE_FILE})

FIND_PACKAGE(VXL REQUIRED)
INCLUDE(${VXL_CMAKE_DIR}/UseVXL.cmake)

INCLUDE_DIRECTORIES(/home/doriad/src/vxl/v3p/netlib/)

ADD_EXECUTABLE(PoissonEditingVNL PoissonEditingExample.cpp PoissonEditing.cpp Helpers.cpp Variable.cpp)
TARGET_LINK_LIBRARIES(PoissonEditingVNL ${ITK_LIBRARIES} vnl)

ADD_EXECUTABLE(PoissonCloning PoissonCloningExample.cpp PoissonCloning.cpp Helpers.cpp Variable.cpp)
TARGET_LINK_LIBRARIES(PoissonCloning ${ITK_LIBRARIES} vnl)