cmake_minimum_required(VERSION 3.10)
project(Compile)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(MAIN_SOURCES main.cpp)
set(LEX_SOURCES
        tokenizer/token_type.h tokenizer/token_type.cpp
        tokenizer/token.h tokenizer/token.cpp
        tokenizer/state.cpp tokenizer/state.h
        tokenizer/lexer_exception.h
        tokenizer/lexer.h tokenizer/lexer.cpp tokenizer/state_table.h)

set(INCLUDE_DIRECTORIES tokenizer)

include_directories(tokenizer)

add_executable(Compile ${MAIN_SOURCES} ${LEX_SOURCES})