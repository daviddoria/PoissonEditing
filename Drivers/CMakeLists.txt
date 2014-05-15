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

ADD_EXECUTABLE(PoissonFillingPoisson PoissonFillingPoisson.cpp)
TARGET_LINK_LIBRARIES(PoissonFillingPoisson ${ITK_LIBRARIES} ${PoissonEditing_libraries})
INSTALL( TARGETS PoissonFillingPoisson RUNTIME DESTINATION ${INSTALL_DIR} )

ADD_EXECUTABLE(PoissonFillingNeumann PoissonFillingNeumann.cpp)
TARGET_LINK_LIBRARIES(PoissonFillingNeumann ${ITK_LIBRARIES} ${PoissonEditing_libraries})
INSTALL( TARGETS PoissonFillingNeumann RUNTIME DESTINATION ${INSTALL_DIR} )

ADD_EXECUTABLE(DerivativesToImage DerivativesToImage.cxx)
TARGET_LINK_LIBRARIES(DerivativesToImage ${ITK_LIBRARIES} ${PoissonEditing_libraries})
INSTALL( TARGETS DerivativesToImage RUNTIME DESTINATION ${INSTALL_DIR} )

ADD_EXECUTABLE(LaplacianToImage LaplacianToImage.cxx)
TARGET_LINK_LIBRARIES(LaplacianToImage ${ITK_LIBRARIES} ${PoissonEditing_libraries})
INSTALL( TARGETS LaplacianToImage  RUNTIME DESTINATION ${INSTALL_DIR} )

ADD_EXECUTABLE(PoissonFillingVariational PoissonFillingVariational.cpp)
TARGET_LINK_LIBRARIES(PoissonFillingVariational ${ITK_LIBRARIES} ${PoissonEditing_libraries})
INSTALL( TARGETS PoissonFillingVariational RUNTIME DESTINATION ${INSTALL_DIR} )
