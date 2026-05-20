cmake_minimum_required(VERSION 3.24)

project(MiniSpire VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(MINISPIRE_BUILD_APP "Build the SFML desktop game" ON)
option(MINISPIRE_BUILD_TESTS "Build core logic tests" ON)

add_library(mini_spire_core
    src/core/Core.cpp
)

target_include_directories(mini_spire_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(mini_spire_core PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(mini_spire_core PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(mini_spire_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(MINISPIRE_BUILD_APP)
    find_package(SFML 2.6...<3.0 COMPONENTS graphics window system audio QUIET)

    if(SFML_FOUND)
        add_executable(mini_spire
            src/main.cpp
            src/app/GameApp.cpp
            src/app/Scenes.cpp
            src/app/UI.cpp
        )

        target_include_directories(mini_spire PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
        target_link_libraries(mini_spire
            PRIVATE
                mini_spire_core
                sfml-graphics
                sfml-window
                sfml-system
                sfml-audio
        )

        if(MSVC)
            target_compile_options(mini_spire PRIVATE /W4 /permissive- /utf-8)
            set_property(TARGET mini_spire PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:mini_spire>")
        else()
            target_compile_options(mini_spire PRIVATE -Wall -Wextra -Wpedantic)
        endif()
    else()
        message(WARNING "SFML 2.6 was not found. The core library/tests will still build. Install SFML with vcpkg or set SFML_DIR to build the desktop game.")
    endif()
endif()

if(MINISPIRE_BUILD_TESTS)
    enable_testing()

    add_executable(mini_spire_core_tests
        tests/core_tests.cpp
    )
    target_link_libraries(mini_spire_core_tests PRIVATE mini_spire_core)

    if(MSVC)
        target_compile_options(mini_spire_core_tests PRIVATE /W4 /permissive- /utf-8)
    else()
        target_compile_options(mini_spire_core_tests PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME mini_spire_core_tests COMMAND mini_spire_core_tests)
endif()
