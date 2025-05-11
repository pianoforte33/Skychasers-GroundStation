cmake_minimum_required(VERSION 3.19)
project(QSerialMonitor LANGUAGES CXX)

set(CMAKE_AUTORCC ON) # used to insert images in the software

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets SerialPort Charts)

qt_standard_project_setup()

qt_add_executable(QSerialMonitor
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    logo.qrc
)


target_link_libraries(QSerialMonitor
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt6::SerialPort
        Qt6::Charts
)

include(GNUInstallDirs)

install(TARGETS QSerialMonitor
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET QSerialMonitor
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
