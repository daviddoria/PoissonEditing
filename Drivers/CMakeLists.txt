# Allow headers in tests to be included like
# #include "PoissonEditing.h" rather than needing
# #include "PoissonEditing/PoissonEditing.h"
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

# Editing
ADD_EXECUTABLE(PoissonFill PoissonEditing.cpp)
TARGET_LINK_LIBRARIES(PoissonFill ${ITK_LIBRARIES} umfpack Mask)
set_target_properties(PoissonFill PROPERTIES COMPILE_DEFINITIONS "${DEFINITIONS}")
INSTALL( TARGETS PoissonFill RUNTIME DESTINATION ${INSTALL_DIR} )

# Cloning
ADD_EXECUTABLE(PoissonCloning PoissonCloning.cpp)
TARGET_LINK_LIBRARIES(PoissonCloning ${ITK_LIBRARIES} umfpack Mask)
INSTALL( TARGETS PoissonCloning RUNTIME DESTINATION ${INSTALL_DIR} )

ADD_EXECUTABLE(PoissonFillingPoisson PoissonFillingPoisson.cpp)
TARGET_LINK_LIBRARIES(PoissonFillingPoisson ${ITK_LIBRARIES} umfpack Mask)
INSTALL( TARGETS PoissonFillingPoisson RUNTIME DESTINATION ${INSTALL_DIR} )

ADD_EXECUTABLE(PoissonFillingNeumann PoissonFillingNeumann.cpp)
TARGET_LINK_LIBRARIES(PoissonFillingNeumann ${ITK_LIBRARIES} umfpack Mask)
INSTALL( TARGETS PoissonFillingNeumann RUNTIME DESTINATION ${INSTALL_DIR} )

ADD_EXECUTABLE(DerivativesToImage DerivativesToImage.cxx)
TARGET_LINK_LIBRARIES(DerivativesToImage ${ITK_LIBRARIES} Mask umfpack)
set_target_properties(DerivativesToImage PROPERTIES COMPILE_DEFINITIONS "${DEFINITIONS}")
INSTALL( TARGETS DerivativesToImage RUNTIME DESTINATION ${INSTALL_DIR} )

ADD_EXECUTABLE(LaplacianToImage LaplacianToImage.cxx)
TARGET_LINK_LIBRARIES(LaplacianToImage ${ITK_LIBRARIES} umfpack Mask)
INSTALL( TARGETS LaplacianToImage  RUNTIME DESTINATION ${INSTALL_DIR} )

ADD_EXECUTABLE(PoissonFillingVariational PoissonFillingVariational.cpp)
TARGET_LINK_LIBRARIES(PoissonFillingVariational ${ITK_LIBRARIES} umfpack Mask)
INSTALL( TARGETS PoissonFillingVariational RUNTIME DESTINATION ${INSTALL_DIR} )
