# Allow headers in tests to be included like
# #include "PoissonEditing.h" rather than needing
# #include "PoissonEditing/PoissonEditing.h"
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

# Create a directory to write the output images
make_directory(${CMAKE_BINARY_DIR}/Temp)

# Compile the comparison driver
add_executable(ImageCompare ImageCompare.cpp)
target_link_libraries(ImageCompare ${PoissonEditing_libraries})

# Ensure the code will instantiate with different types
add_executable(TypeTesting TypeTesting.cpp)
target_link_libraries(TypeTesting ${PoissonEditing_libraries})

# Test Poisson filling
add_test(NAME PoissonFillTest COMMAND ${CMAKE_BINARY_DIR}/Drivers/PoissonFill
         ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Source.png
         ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Mask.png ${CMAKE_BINARY_DIR}/Temp/F16_filled.png)
add_test(PoissonFillCompare ImageCompare ${CMAKE_BINARY_DIR}/Temp/F16_filled.png
                                         ${CMAKE_SOURCE_DIR}/Testing/baselines/F16_filled.png)

# Test Poisson cloning
add_test(NAME PoissonCloneTest COMMAND ${CMAKE_BINARY_DIR}/Drivers/PoissonClone
        ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Source.png
        ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Mask.png
        ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Target.png ${CMAKE_BINARY_DIR}/Temp/cloning.png)
add_test(PoissonCloneCompare ImageCompare ${CMAKE_BINARY_DIR}/Temp/cloning.png
                                          ${CMAKE_SOURCE_DIR}/Testing/baselines/cloning.png)

# Test reconstruction from Laplacian
add_test(LaplacianToImageTest ${CMAKE_BINARY_DIR}/Drivers/LaplacianToImage
        ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Source.png
        ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Mask.png
        ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Laplacian.mhd
        ${CMAKE_BINARY_DIR}/Temp/LaplacianToImage.png)
add_test(LaplacianToImageCompare ImageCompare ${CMAKE_BINARY_DIR}/Temp/LaplacianToImage.png
                                              ${CMAKE_SOURCE_DIR}/Testing/baselines/LaplacianToImage.png)
