# Allow headers in tests to be included like
# #include "PoissonEditing.h" rather than needing
# #include "PoissonEditing/PoissonEditing.h"
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(TypeTesting TypeTesting.cpp)
target_link_libraries(TypeTesting ${PoissonEditing_libraries})

add_executable(TestPoissonFill TestPoissonFill.cpp)
target_link_libraries(TestPoissonFill ${PoissonEditing_libraries})

ADD_TEST(NAME PoissonFillTest COMMAND ${CMAKE_BINARY_DIR}/Drivers/PoissonFill
         ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Source.png
         ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Mask.png editing.png)

ADD_TEST(NAME PoissonCloneTest COMMAND ${CMAKE_BINARY_DIR}/Drivers/PoissonClone
        ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Source.png
        ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Mask.png
        ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Target.png cloning.png)

ADD_TEST(LaplacianToImageTest ${CMAKE_BINARY_DIR}/Drivers/LaplacianToImage
        ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Source.png
        ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Mask.png
        ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Laplacian.mhd LaplacianToImageOutput.mhd)

