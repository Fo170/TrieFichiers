cmake_minimum_required(VERSION 3.16)
project(TrieFichiers VERSION 1.0.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

file(COPY ico/app.ico DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ico)
file(COPY lang/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/lang)

if(WIN32)
    if(NOT CMAKE_RC_COMPILER)
        set(CMAKE_RC_COMPILER windres)
    endif()
    set(RC_OUT "${CMAKE_CURRENT_BINARY_DIR}/app_icon.o")
    add_custom_command(
        OUTPUT ${RC_OUT}
        COMMAND ${CMAKE_RC_COMPILER}
                -i "${CMAKE_CURRENT_SOURCE_DIR}/app.rc"
                -o ${RC_OUT}
        DEPENDS app.rc
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
    set(RC_FILES ${RC_OUT})
endif()

add_executable(TrieFichiers
    main.cpp
    MainWindow.cpp
    Project.cpp
    ComponentToolbox.cpp
    UpdateChecker.cpp
    LangueManager.cpp
    FolderCleaner.cpp
    CleanupDialog.cpp
    StripDialog.cpp
    resources.qrc
    ${RC_FILES}
)

target_link_libraries(TrieFichiers PRIVATE Qt6::Widgets Qt6::Network)
target_include_directories(TrieFichiers PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
