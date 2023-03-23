cmake_minimum_required(VERSION 3.11)

project(CodinGame)

set( CMAKE_CXX_COMPILER_ID_REQUIRED GNU )
set( CMAKE_CXX_STANDARD 17 )
add_compile_options( -lm -lpthread -ldl -lcrypt )

add_subdirectory(puzzles)
add_subdirectory(code-golf)
add_subdirectory(optimization)
add_subdirectory(bot-programming)
