# Allow headers in tests to be included like
# #include "PoissonEditing.h" rather than needing
# #include "PoissonEditing/PoissonEditing.h"
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(TypeTesting TypeTesting.cpp)
target_link_libraries(TypeTesting Mask umfpack)

# ADD_TEST(NAME PoissonEditingTest COMMAND ${EXECUTABLE_OUTPUT_PATH}/PoissonEditing ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Source.png ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Mask.png editing.png)
# 
# ADD_TEST(PoissonEditingTest ${EXECUTABLE_OUTPUT_PATH}/PoissonEditing ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Source.png ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Mask.png PoissonEditingOutput.mhd)
# 
# ADD_TEST(NAME PoissonCloningTest COMMAND ${EXECUTABLE_OUTPUT_PATH}/PoissonCloning ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Source.png ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Mask.png ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Target.png cloning.png)
# 
# ADD_TEST(LaplacianToImageTest ${EXECUTABLE_OUTPUT_PATH}/LaplacianToImage ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Source.png ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Mask.png ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Laplacian.mhd LaplacianToImageOutput.mhd)
# 
