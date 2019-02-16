cmake_minimum_required(VERSION 3.10)
project(Compile)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

include_directories(lib/cxxopts/include lib/poly_visitor/include)

set(MAIN_SOURCES main.cpp exception.h)
set(LEX_SOURCES
        tokenizer/lexerBuffer.cpp   tokenizer/lexerBuffer.cpp
        tokenizer/token_type.h tokenizer/token_type.cpp
        tokenizer/token.h tokenizer/token.cpp
        tokenizer/lexer.h tokenizer/lexer.cpp tokenizer/state_table.h)

set(PAR_SOURCES
        parser/parser.cpp parser/parser.h
        parser/printf_visitor.cpp parser/visitor.h
        parser/type_checker.cpp parser/type_checker.h
        parser/semantic_decl.h parser/semantic_decl.cpp)

set(NODE_SOURCE
        node/astnode.h
        node/node.cpp node/node.h
        node/table_symbol.h node/symbol_type.h
        node/symbol_fun.h node/symbol_var.h node/symbol.cpp)

set(GEN_SOURCES
        assembler/opcode.h assembler/opcode.cpp
        assembler/generator.cpp assembler/generator.h)

include_directories(tokenizer parser assembler node)

add_executable(Compile ${MAIN_SOURCES} ${LEX_SOURCES} ${PAR_SOURCES} ${GEN_SOURCES} ${NODE_SOURCE})