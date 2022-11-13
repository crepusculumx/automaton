cmake_minimum_required(VERSION 3.21)
project(automaton-test)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(../ Automaton)

#-----------------------------------------------------------------------------------------------------------------------
#copied from https://github.com/google/googletest/tree/main/googletest
include(FetchContent)
FetchContent_Declare(
        googletest
        # Specify the commit you depend on and update it regularly.
        URL https://github.com/google/googletest/archive/e2239ee6043f73722e7aa812a459f54a28552929.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
#-----------------------------------------------------------------------------------------------------------------------

enable_testing()

add_definitions(-DGTEST)

#-----------------------------------------------------------------------------------------------------------------------

include_directories(sources)

file(GLOB TEST_SOURCES "sources/*.cpp" "sources/Dfa/*.cpp" "sources/Nfa/*cpp" "sources/EpsilonNfa/*cpp" "sources/Re/*cpp")
foreach (ONE_TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(ONE_TEST_NAME "${ONE_TEST_SOURCE}" NAME)
    string(REPLACE ".cpp" "" ONE_TEST_NAME ${ONE_TEST_NAME})
    set(ONE_TEST_FULL_NAME "automaton-test-${ONE_TEST_NAME}")

    add_executable(${ONE_TEST_FULL_NAME} ${ONE_TEST_SOURCE})
    target_link_libraries(${ONE_TEST_FULL_NAME} gtest gtest_main automaton)
    add_test(NAME ${ONE_TEST_NAME} COMMAND ${ONE_TEST_FULL_NAME})
endforeach ()
#-----------------------------------------------------------------------------------------------------------------------