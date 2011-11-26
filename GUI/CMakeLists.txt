# If either interactive executable is going to be built, we need Qt, and for the file selector to be built.
if(PoissonEditingInteractive OR PoissonCloningInteractive)

  FIND_PACKAGE(Qt4 REQUIRED)
  INCLUDE(${QT_USE_FILE})

  # Let Qt find it's MOCed files
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

#   QT4_WRAP_UI(FileSelectorUISrcs FileSelector.ui FileSelectionWidget.ui)
#   QT4_WRAP_CPP(FileSelectorMOCSrcs FileSelector.h FileSelectionWidget.h)
#   add_library(FileSelectorLibrary FileSelector.cpp FileSelectionWidget.cpp ${FileSelectorUISrcs} ${FileSelectorMOCSrcs})
  QT4_WRAP_UI(FileSelectorUISrcs FileSelectionWidget.ui)
  QT4_WRAP_CPP(FileSelectorMOCSrcs FileSelectionWidget.h ImageFileSelector.h Panel.h)
  add_library(FileSelectorLibrary ImageFileSelector.cpp Panel.cpp FileSelectionWidget.cpp ${FileSelectorUISrcs} ${FileSelectorMOCSrcs})
endif()

SET(PoissonEditingInteractive CACHE BOOL "Build the Poisson editing interactive version")
if(PoissonEditingInteractive)
  message("Building PoissonEditingInteractive")
  QT4_WRAP_UI(PoissonEditingUISrcs PoissonEditingGUI.ui)
  QT4_WRAP_CPP(PoissonEditingMOCSrcs PoissonEditingGUI.h)

  ADD_EXECUTABLE(PoissonEditingInteractive PoissonEditingInteractive.cpp  PoissonEditingGUI.cxx HelpersQt.cpp ${PoissonEditingUISrcs} ${PoissonEditingMOCSrcs})
  TARGET_LINK_LIBRARIES(PoissonEditingInteractive ${ITK_LIBRARIES} umfpack ${QT_LIBRARIES} FileSelectorLibrary HelpersLibrary )
  #set_target_properties(PoissonCloning PROPERTIES COMPILE_DEFINITIONS "${DEFINITIONS}")
  INSTALL( TARGETS PoissonEditingInteractive RUNTIME DESTINATION ${INSTALL_DIR} )
  #ADD_TEST(NAME PoissonCloningTest COMMAND ${EXECUTABLE_OUTPUT_PATH}/PoissonCloning ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Source.png ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Mask.png ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Target.png cloning.png)
endif()

SET(PoissonCloningInteractive CACHE BOOL "Build the Poisson cloning interactive version")
if(PoissonCloningInteractive)
  message("Building PoissonCloningInteractive")
  QT4_WRAP_UI(PoissonCloningUISrcs PoissonCloningGUI.ui)
  QT4_WRAP_CPP(PoissonCloningMOCSrcs PoissonCloningGUI.h)
  ADD_EXECUTABLE(PoissonCloningInteractive PoissonCloningInteractive.cpp PoissonCloningGUI.cxx HelpersQt.cpp ${PoissonCloningUISrcs} ${PoissonCloningMOCSrcs})
  TARGET_LINK_LIBRARIES(PoissonCloningInteractive ${ITK_LIBRARIES} umfpack ${QT_LIBRARIES} FileSelectorLibrary HelpersLibrary)
  #set_target_properties(PoissonCloning PROPERTIES COMPILE_DEFINITIONS "${DEFINITIONS}")
  INSTALL( TARGETS PoissonCloningInteractive RUNTIME DESTINATION ${INSTALL_DIR} )
  #ADD_TEST(NAME PoissonCloningTest COMMAND ${EXECUTABLE_OUTPUT_PATH}/PoissonCloning ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Source.png ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Mask.png ${CMAKE_SOURCE_DIR}/Testing/data/F16/F16Target.png cloning.png)
endif()
