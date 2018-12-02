cmake_minimum_required(VERSION 3.10)
project(Compile)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(MAIN_SOURCES main.cpp)
set(LEX_SOURCES
        tokenizer/lexerBuffer.cpp   tokenizer/lexerBuffer.cpp
        tokenizer/token_type.h tokenizer/token_type.cpp
        tokenizer/token.h tokenizer/token.cpp
        tokenizer/lexer.h tokenizer/lexer.cpp tokenizer/state_table.h)

set(PAR_SOURCES
        parser/node.cpp parser/node.h
        parser/parser.cpp parser/parser.h
        parser/visitor.cpp parser/visitor.h exception.h)

include_directories(tokenizer parser include-lib)

add_executable(Compile ${MAIN_SOURCES} ${LEX_SOURCES} ${PAR_SOURCES})