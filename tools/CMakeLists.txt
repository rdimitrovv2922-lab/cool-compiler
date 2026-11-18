cmake_minimum_required(VERSION 3.16)
project(cool_compiler_frontend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(SRC_DIR "${PROJECT_ROOT}/src")
set(GEN_DIR "${PROJECT_ROOT}/gen")
set(BUILD_DIR "${PROJECT_ROOT}/build")
set(DRIVERS_DIR "${SRC_DIR}/drivers")
file(MAKE_DIRECTORY "${GEN_DIR}")

set(ANTLR4_JAR "/usr/local/lib/antlr-4.13.2-complete.jar" CACHE FILEPATH "Path to the ANTLR 4 complete jar")
if(NOT EXISTS "${ANTLR4_JAR}")
  message(FATAL_ERROR "ANTLR4 jar not found; set ANTLR4_JAR")
endif()
find_package(Java COMPONENTS Runtime REQUIRED)
set(ANTLR4_TOOL_ARGS -Xmx500M -cp "${ANTLR4_JAR}" org.antlr.v4.Tool -Dlanguage=Cpp)

set(ANTLR4_RUNTIME_INCLUDE_DIR "/usr/local/include/antlr4-runtime" CACHE PATH "Directory containing ANTLR runtime headers")
if(NOT EXISTS "${ANTLR4_RUNTIME_INCLUDE_DIR}/antlr4-runtime.h")
  find_path(ANTLR4_RUNTIME_INCLUDE_DIR NAMES antlr4-runtime.h PATH_SUFFIXES antlr4-runtime)
endif()
if(NOT ANTLR4_RUNTIME_INCLUDE_DIR)
  message(FATAL_ERROR "ANTLR runtime headers not found; set ANTLR4_RUNTIME_INCLUDE_DIR")
endif()

set(ANTLR4_RUNTIME_LIBRARY "/usr/local/lib/libantlr4-runtime.a" CACHE FILEPATH "Path to the ANTLR runtime library")
if(NOT EXISTS "${ANTLR4_RUNTIME_LIBRARY}")
  find_library(ANTLR4_RUNTIME_LIBRARY NAMES antlr4-runtime libantlr4-runtime.a)
endif()
if(NOT ANTLR4_RUNTIME_LIBRARY)
  message(FATAL_ERROR "ANTLR runtime library not found; set ANTLR4_RUNTIME_LIBRARY")
endif()

# Generate and build the lexer first so its token vocabulary is available to the parser.
set(LEXER_SOURCE "${GEN_DIR}/CoolLexer.cpp")
set(LEXER_HEADER "${GEN_DIR}/CoolLexer.h")
set(LEXER_TOKENS "${GEN_DIR}/CoolLexer.tokens")
add_custom_command(
  OUTPUT "${LEXER_SOURCE}"
  BYPRODUCTS "${LEXER_HEADER}" "${GEN_DIR}/CoolLexer.interp" "${LEXER_TOKENS}"
  COMMAND ${Java_JAVA_EXECUTABLE} ${ANTLR4_TOOL_ARGS} "${SRC_DIR}/CoolLexer.g4" -o "${GEN_DIR}"
  DEPENDS "${SRC_DIR}/CoolLexer.g4"
  COMMENT "Generating COOL lexer"
  VERBATIM
)
add_custom_target(gen_lexer DEPENDS "${LEXER_SOURCE}")
add_library(lexer_gen_code STATIC "${LEXER_SOURCE}")
add_dependencies(lexer_gen_code gen_lexer)
target_include_directories(lexer_gen_code PUBLIC "${GEN_DIR}" "${ANTLR4_RUNTIME_INCLUDE_DIR}")
target_link_libraries(lexer_gen_code PUBLIC "${ANTLR4_RUNTIME_LIBRARY}")

set(PARSER_SOURCES
  "${GEN_DIR}/CoolParser.cpp"
  "${GEN_DIR}/CoolParserBaseVisitor.cpp"
  "${GEN_DIR}/CoolParserVisitor.cpp"
)
add_custom_command(
  OUTPUT ${PARSER_SOURCES}
  BYPRODUCTS
    "${GEN_DIR}/CoolParser.h"
    "${GEN_DIR}/CoolParser.interp"
    "${GEN_DIR}/CoolParser.tokens"
    "${GEN_DIR}/CoolParserBaseVisitor.h"
    "${GEN_DIR}/CoolParserVisitor.h"
  COMMAND ${Java_JAVA_EXECUTABLE} ${ANTLR4_TOOL_ARGS} -visitor -no-listener
          -lib "${GEN_DIR}" "${SRC_DIR}/CoolParser.g4" -o "${GEN_DIR}"
  DEPENDS "${SRC_DIR}/CoolParser.g4" "${LEXER_TOKENS}"
  COMMENT "Generating COOL parser"
  VERBATIM
)
add_custom_target(gen_parser DEPENDS ${PARSER_SOURCES})
add_dependencies(gen_parser gen_lexer)
add_library(parser_gen_code STATIC ${PARSER_SOURCES})
add_dependencies(parser_gen_code gen_parser)
target_include_directories(parser_gen_code PUBLIC "${GEN_DIR}" "${ANTLR4_RUNTIME_INCLUDE_DIR}")
target_link_libraries(parser_gen_code PUBLIC "${ANTLR4_RUNTIME_LIBRARY}")

add_executable(lexer "${DRIVERS_DIR}/LexerDriver.cpp")
target_link_libraries(lexer PRIVATE lexer_gen_code)
set_target_properties(lexer PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${BUILD_DIR}")

add_executable(parser "${DRIVERS_DIR}/ParserDriver.cpp")
target_link_libraries(parser PRIVATE lexer_gen_code parser_gen_code)
set_target_properties(parser PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${BUILD_DIR}")
