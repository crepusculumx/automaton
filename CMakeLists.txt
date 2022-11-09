cmake_minimum_required(VERSION 3.21)
project(automaton)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

file(GLOB AUTOMATON_SOURCES "include/automaton/*.hpp")

add_library(automaton INTERFACE)
target_sources(automaton INTERFACE ${AUTOMATON_SOURCES})
target_include_directories(automaton INTERFACE include/
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include/automaton>)