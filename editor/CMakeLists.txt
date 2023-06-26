cmake_minimum_required(VERSION 3.13) 
project(StrideIDE) 

set(EDITOR_SRC 
  main.cpp
  localmanagementdialog.cpp 
  projectwindow.cpp 
  codeeditor.cpp 
  languagehighlighter.cpp
  linenumberarea.cpp
  configdialog.cpp
  savechangeddialog.cpp
  errormarker.cpp
  searchwidget.cpp
  striderootmanagementdialog.cpp
  tooltip.cpp
  codemodel.cpp
  autocompletemenu.cpp
)

set(EDITOR_HEADERS
  localmanagementdialog.hpp
  projectwindow.h
  codeeditor.h
  languagehighlighter.h
  linenumberarea.h
  configdialog.h
  savechangeddialog.h
  errormarker.h
  searchwidget.h
  striderootmanagementdialog.h
  tooltip.hpp
  codemodel.hpp
  autocompletemenu.hpp
)

set(EDITOR_UI
  localmanagementdialog.ui
  projectwindow.ui
  configdialog.ui
  savechangeddialog.ui
  searchwidget.ui
  striderootmanagementdialog.ui
  )

find_package(Qt6 6.5.0 REQUIRED COMPONENTS
    Core Widgets WebEngineCore WebEngineWidgets Qml
)
qt_standard_project_setup()

qt_add_resources(QT_RESOURCES qmlfiles.qrc)

# TODO add Windows icon
#if(WIN32)
#  set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/qt5app.rc")
#endif()


add_executable(StrideIDE ${EDITOR_SRC} ${EDITOR_HEADERS} ${APP_ICON_RESOURCE_WINDOWS})

set_target_properties(StrideIDE PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
target_include_directories(StrideIDE PUBLIC ../parser .)
target_link_libraries(StrideIDE StrideParser StrideCodegen 
  Qt6::Core Qt6::Widgets Qt6::WebEngineCore Qt6::WebEngineWidgets Qt6::Qml)

set_property(TARGET StrideIDE PROPERTY CXX_STANDARD 17)
set_property(TARGET StrideIDE PROPERTY CXX_STANDARD_REQUIRED ON)
# TODO add Icon
if(APPLE)
#  set(MACOSX_BUNDLE_ICON_FILE qt5app.icns)
  
#  set(APP_ICON_MACOSX ${CMAKE_CURRENT_SOURCE_DIR}/qt5app.icns)
#  set_source_files_properties(${APP_ICON_MACOSX} PROPERTIES
#         MACOSX_PACKAGE_LOCATION "Resources")
  
elseif(UNIX)
endif()

