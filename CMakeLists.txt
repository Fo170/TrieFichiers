cmake_minimum_required(VERSION 3.16)
project(ApplicationVide VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

file(COPY ico/app.ico DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ico)
file(COPY lang/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/lang)

# Pre-compile .rc to .o at configure time (path workaround for windres)
set(WINDRES "C:/Qt/Tools/mingw1310_64/bin/windres.exe")
set(RC_OUT "${CMAKE_CURRENT_BINARY_DIR}/app_icon.o")
if(NOT EXISTS "${RC_OUT}")
    execute_process(
        COMMAND ${WINDRES} -i "${CMAKE_CURRENT_SOURCE_DIR}/app.rc" -o "${RC_OUT}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endif()

add_executable(ApplicationVide
    main.cpp
    MainWindow.cpp
    Project.cpp
    ComponentToolbox.cpp
    UpdateChecker.cpp
    LangueManager.cpp
    resources.qrc
    ${RC_OUT}
)

target_link_libraries(ApplicationVide PRIVATE Qt6::Widgets Qt6::Network)
target_include_directories(ApplicationVide PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
