cmake_minimum_required(VERSION 3.16)
project(TimetableChain VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warning flags across all targets
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ==============================================================================
# Target: core (Pure C++17 Static Library - Backend)
# ==============================================================================
set(CORE_SOURCES
    core/src/sha256.cpp
    core/src/timetable.cpp
    core/src/dataset.cpp
    core/src/constraint_checker.cpp
    core/src/scorer.cpp
    core/src/generator.cpp
    core/src/block.cpp
    core/src/blockchain.cpp
)

add_library(core STATIC ${CORE_SOURCES})
target_include_directories(core PUBLIC core/include)

# ==============================================================================
# Target: tests (CTest Test Executable with Plain Asserts)
# ==============================================================================
# ==============================================================================
# Target: tests (CTest Test Executable with Plain Asserts)
# ==============================================================================
enable_testing()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_main.cpp")
    add_executable(tests tests/test_main.cpp)
    target_link_libraries(tests PRIVATE core)
    if(NOT MSVC)
        target_compile_options(tests PRIVATE -UNDEBUG)
    endif()
    add_test(NAME CoreTests COMMAND tests)
endif()

# ==============================================================================
# Target: cli (Optional Tiny Text Program "timetable" - Demo + Verifier)
# ==============================================================================
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cli/main.cpp")
    add_executable(timetable cli/main.cpp)
    target_link_libraries(timetable PRIVATE core)
endif()

# ==============================================================================
# Target: server (Native HTTP REST API Server - Web Backend)
# ==============================================================================
add_executable(timetable-server server/src/server_main.cpp)
target_include_directories(timetable-server PRIVATE server/include)
target_link_libraries(timetable-server PRIVATE core)
if(NOT MSVC)
    target_link_libraries(timetable-server PRIVATE pthread)
endif()

# ==============================================================================
# Qt6 Package & Setup (Desktop GUI & GUI Integration Tests)
# ==============================================================================
find_package(Qt6 COMPONENTS Widgets Gui Core QUIET)

if(Qt6_FOUND)
    message(STATUS "Qt6 found (${Qt6_VERSION}): enabling timetable-gui and gui_test")
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)

    set(GUI_HEADERS
        gui/include/mainwindow.hpp
        gui/include/generator_page.hpp
        gui/include/manual_builder_page.hpp
        gui/include/blockchain_page.hpp
        gui/include/data_page.hpp
        gui/include/schedule_grid.hpp
        gui/include/slot_dialog.hpp
        gui/include/comparison_dialog.hpp
        gui/include/tamper_dialog.hpp
        gui/include/block_detail_dialog.hpp
        gui/include/toast.hpp
        gui/include/theme.hpp
    )

    set(GUI_LIB_SOURCES
        gui/src/mainwindow.cpp
        gui/src/generator_page.cpp
        gui/src/manual_builder_page.cpp
        gui/src/blockchain_page.cpp
        gui/src/data_page.cpp
        gui/src/schedule_grid.cpp
        gui/src/slot_dialog.cpp
        gui/src/comparison_dialog.cpp
        gui/src/tamper_dialog.cpp
        gui/src/block_detail_dialog.cpp
        gui/src/toast.cpp
        gui/src/theme.cpp
        gui/resources/styles.qrc
    )

    add_executable(gui_test tests/gui_test.cpp ${GUI_LIB_SOURCES} ${GUI_HEADERS})
    target_include_directories(gui_test PRIVATE gui/include)
    target_compile_definitions(gui_test PRIVATE QT_NO_KEYWORDS)
    if(NOT MSVC)
        target_compile_options(gui_test PRIVATE -UNDEBUG)
    endif()
    target_link_libraries(gui_test PRIVATE core Qt6::Widgets Qt6::Gui Qt6::Core)
    add_test(NAME GuiIntegrationTests COMMAND gui_test)

    add_executable(timetable-gui gui/src/main.cpp ${GUI_LIB_SOURCES} ${GUI_HEADERS})
    target_include_directories(timetable-gui PRIVATE gui/include)
    target_compile_definitions(timetable-gui PRIVATE QT_NO_KEYWORDS)
    target_link_libraries(timetable-gui PRIVATE core Qt6::Widgets Qt6::Gui Qt6::Core)
else()
    message(STATUS "Qt6 not found: skipping desktop GUI targets (timetable-gui, gui_test).")
endif()
