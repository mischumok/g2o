INCLUDE(${QT_USE_FILE})

QT4_WRAP_UI(UI_HEADERS base_main_window.ui base_properties_widget.ui)
QT4_WRAP_CPP(UI_SOURCES main_window.h properties_widget.h)

# ${QT_INCLUDE_DIR} does not appear to work; therefore have to
# manually rig it; also have to add QtCore directly because qglviewer
# doesn't seem to handle the Qt directories properly

INCLUDE_DIRECTORIES(${QGLVIEWER_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${QT_QTCORE_INCLUDE_DIR}/..)

ADD_LIBRARY(viewer_library ${G2O_LIB_TYPE}
  g2o_viewer_api.h
  g2o_qglviewer.cpp g2o_qglviewer.h
  primitives.cpp primitives.h
  main_window.cpp main_window.h
  stream_redirect.cpp stream_redirect.h
  gui_hyper_graph_action.cpp gui_hyper_graph_action.h
  properties_widget.cpp properties_widget.h
  viewer_properties_widget.cpp viewer_properties_widget.h
  ${UI_HEADERS} ${UI_SOURCES}
)
SET_TARGET_PROPERTIES(viewer_library PROPERTIES OUTPUT_NAME ${LIB_PREFIX}viewer)

ADD_EXECUTABLE(g2o_viewer
  g2o_viewer.cpp
)

SET_TARGET_PROPERTIES(g2o_viewer PROPERTIES OUTPUT_NAME g2o_viewer${EXE_POSTFIX})

TARGET_LINK_LIBRARIES(viewer_library core g2o_cli_library ${QGLVIEWER_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTOPENGL_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY})
IF(GLUT_FOUND)
  TARGET_LINK_LIBRARIES(viewer_library ${GLUT_glut_LIBRARY})
ENDIF()
TARGET_LINK_LIBRARIES(g2o_viewer viewer_library)

INSTALL(TARGETS g2o_viewer
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)

INSTALL(TARGETS viewer_library
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)

FILE(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
INSTALL(FILES ${headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/g2o/apps/g2o_viewer)
INSTALL(FILES ${UI_HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/g2o/apps/g2o_viewer)
