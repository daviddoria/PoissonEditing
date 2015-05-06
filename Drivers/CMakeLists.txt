# Allow headers in tests to be included like
# #include "PoissonEditing.h" rather than needing
# #include "PoissonEditing/PoissonEditing.h"
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

# Editing
ADD_EXECUTABLE(PoissonFill PoissonFill.cpp)
TARGET_LINK_LIBRARIES(PoissonFill ${ITK_LIBRARIES} ${PoissonEditing_libraries})
INSTALL( TARGETS PoissonFill RUNTIME DESTINATION ${INSTALL_DIR} )

# Cloning
ADD_EXECUTABLE(PoissonClone PoissonClone.cpp)
TARGET_LINK_LIBRARIES(PoissonClone ${ITK_LIBRARIES} ${PoissonEditing_libraries})
INSTALL( TARGETS PoissonClone RUNTIME DESTINATION ${INSTALL_DIR} )

ADD_EXECUTABLE(PoissonFillWithGuidance PoissonFillWithGuidance.cpp)
TARGET_LINK_LIBRARIES(PoissonFillWithGuidance ${ITK_LIBRARIES} ${PoissonEditing_libraries})
INSTALL( TARGETS PoissonFillWithGuidance RUNTIME DESTINATION ${INSTALL_DIR} )

ADD_EXECUTABLE(DerivativesToImage DerivativesToImage.cxx)
TARGET_LINK_LIBRARIES(DerivativesToImage ${ITK_LIBRARIES} ${PoissonEditing_libraries})
INSTALL( TARGETS DerivativesToImage RUNTIME DESTINATION ${INSTALL_DIR} )

ADD_EXECUTABLE(LaplacianToImage LaplacianToImage.cxx)
TARGET_LINK_LIBRARIES(LaplacianToImage ${ITK_LIBRARIES} ${PoissonEditing_libraries})
INSTALL( TARGETS LaplacianToImage  RUNTIME DESTINATION ${INSTALL_DIR} )
