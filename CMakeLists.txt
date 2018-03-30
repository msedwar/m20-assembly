cmake_minimum_required(VERSION 3.9)
project(m20_assembly)
set(CMAKE_CXX_STANDARD 14)

add_definitions(-Wall -Werror -pedantic --std=c++14)
if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG=1 -g)
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    add_definitions(-DNDEBUG=1)
endif()

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(SOURCES
        ${SRC_DIR}/Assembler.cpp
        ${SRC_DIR}/Lexer.cpp
        ${SRC_DIR}/Linker.cpp
        ${SRC_DIR}/Parser.cpp
        ${SRC_DIR}/Simulator.cpp
        ${SRC_DIR}/Token.cpp
        ${SRC_DIR}/Utils.cpp)
set(HEADERS
        ${SRC_DIR}/Assembler.h
        ${SRC_DIR}/Instruction.h
        ${SRC_DIR}/Lexer.h
        ${SRC_DIR}/Linker.h
        ${SRC_DIR}/Parser.h
        ${SRC_DIR}/Simulator.h
        ${SRC_DIR}/Token.h
        ${SRC_DIR}/Utils.h)

add_executable(assemble ${SRC_DIR}/assemble.cpp ${SOURCES} ${HEADERS})
add_executable(link ${SRC_DIR}/link.cpp ${SOURCES} ${HEADERS})
add_executable(simulate ${SRC_DIR}/simulate.cpp ${SOURCES} ${HEADERS})