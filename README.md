# COOL Compiler Front End

A C++ compiler-construction project for the COOL language. This repository combines the lexical analyzer and parser, implemented with ANTLR4 and C++.

## Project status

The lexer and parser stages are implemented. The compiler project is incomplete: **semantic analysis and code generation have not been implemented**. This repository does not claim to be a complete compiler.

## Components

- **Lexer:** recognizes COOL keywords, identifiers, literals, operators, comments, and lexical errors. String escapes are decoded and token values are exposed to the parser.
- **Parser:** checks COOL class, feature, and expression syntax, then prints the parsed program in the course's tree format. Syntax errors are reported with the source file and line.
- **Drivers:** `build/lexer` prints tokens from standard input; `build/parser` reads one source file and reports its syntax tree or parse errors.

## Build

The provided Docker image contains C++23, Java, ANTLR4 4.13.2, and the ANTLR4 C++ runtime.

```powershell
docker build -t cool-compiler .
```

## Run tests

```powershell
docker run --rm --entrypoint bash cool-compiler -lc "bash tools/test-all.sh"
```

The regression suite contains 78 lexer cases and 104 parser cases.

## Run the tools

Lex source text from PowerShell:

```powershell
Get-Content -Raw .\program.cl | docker run --rm -i --entrypoint ./build/lexer cool-compiler
```

Parse a file (mount the current directory so the container can read it):

```powershell
docker run --rm -v "${PWD}:/workspace/input" --entrypoint ./build/parser cool-compiler /workspace/input/program.cl
```

## Layout

```text
src/CoolLexer.g4             ANTLR lexer grammar
src/CoolParser.g4            ANTLR parser grammar
src/drivers/LexerDriver.cpp  Token-stream driver
src/drivers/ParserDriver.cpp Parse-tree and error driver
tests/lexer/                 Lexer input/output fixtures
tests/parser/                Parser input/output fixtures
tools/                       CMake and build/test scripts
```

ANTLR-generated files and build products are created in `gen/` and `build/`; they are excluded from version control.

## Acknowledgment

Developed as the first two stages of a compiler-construction course project for COOL, using ANTLR4.
