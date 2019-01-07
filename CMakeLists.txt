cmake_minimum_required(VERSION 3.10)
project(Compile)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(MAIN_SOURCES main.cpp exception.h)
set(LEX_SOURCES
        tokenizer/lexerBuffer.cpp   tokenizer/lexerBuffer.cpp
        tokenizer/token_type.h tokenizer/token_type.cpp
        tokenizer/token.h tokenizer/token.cpp
        tokenizer/lexer.h tokenizer/lexer.cpp tokenizer/state_table.h)

set(PAR_SOURCES
        parser/astnode.h
        parser/node.cpp parser/node.h
        parser/parser.cpp parser/parser.h
        parser/printf_visitor.cpp parser/visitor.h
        parser/table_symbol.h parser/symbol_type.h parser/symbol_fun.h parser/symbol_var.h parser/symbol.cpp
        parser/type_checker.cpp parser/type_checker.h
        parser/semantic_decl.h parser/semantic_decl.cpp)

include_directories(tokenizer parser include-lib)

add_executable(Compile ${MAIN_SOURCES} ${LEX_SOURCES} ${PAR_SOURCES})