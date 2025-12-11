cmake_minimum_required(VERSION 3.14)

# Project name
set(APP_NAME Pentelka)

project(${APP_NAME} VERSION 0.1 LANGUAGES CXX)

# Variables
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt6 only
find_package(Qt6 REQUIRED COMPONENTS Core Quick LinguistTools PrintSupport)

set(TS_FILES translations/pentelka_en_US.ts translations/pentelka_en.ts)

set(PROJECT_SOURCES
    main.cpp
    qml.qrc
    ${TS_FILES}

	painter/painter.cpp
)

# Executable
qt_add_executable(${APP_NAME}
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

# Translations
qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

# Resources
qt_add_resources(${APP_NAME} "app_resources"
    PREFIX "/"
    FILES
        ../assets/icon/pentelka_icon.png
)

# Link Qt6
target_link_libraries(${APP_NAME}
    PRIVATE Qt6::Core Qt6::Quick Qt6::PrintSupport
)

# Target properties (macOS & Windows)
set_target_properties(${APP_NAME} PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER penetelka.hrubos.dev
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Install
install(TARGETS ${APP_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Qt6 finalization
qt_import_qml_plugins(${APP_NAME})
qt_finalize_executable(${APP_NAME})
