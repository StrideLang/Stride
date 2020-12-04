cmake_minimum_required(VERSION 3.13) 
project(StrideIDE) 

set(CMAKE_CXX_STANDARD 11)

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

find_package(Qt5 5.7.0 REQUIRED COMPONENTS
    Core Widgets WebEngine WebEngineWidgets Qml
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

qt5_add_resources(QT_RESOURCES qmlfiles.qrc)

add_executable(StrideIDE ${EDITOR_SRC} ${EDITOR_HEADERS})

target_include_directories(StrideIDE PUBLIC ../parser .)
target_link_libraries(StrideIDE StrideParser StrideCodegen 
  Qt5::Core Qt5::Widgets Qt5::WebEngine Qt5::WebEngineWidgets Qt5::Qml)
