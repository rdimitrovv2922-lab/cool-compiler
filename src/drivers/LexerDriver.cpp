#include <cctype>
#include <iostream>
#include <string>
#include <vector>

#include "antlr4-runtime/antlr4-runtime.h"

#include "CoolLexer.h"

using namespace std;
using namespace antlr4;

string cool_token_to_string(Token *token) {
    auto token_type = token->getType();

    switch (token_type) {
        case static_cast<size_t>(-1) : return "EOF";

        case CoolLexer::SEMI: return "';'";
        case CoolLexer::COLON: return "':'";
        case CoolLexer::COMMA: return "','";
        case CoolLexer::LPAREN: return "'('";
        case CoolLexer::RPAREN: return "')'";
        case CoolLexer::LBRACE: return "'{'";
        case CoolLexer::RBRACE: return "'}'";
        case CoolLexer::PLUS: return "'+'";
        case CoolLexer::MINUS: return "'-'";
        case CoolLexer::STAR: return "'*'";
        case CoolLexer::SLASH: return "'/'";
        case CoolLexer::TILDE: return "'~'";
        case CoolLexer::LT: return "'<'";
        case CoolLexer::EQUAL: return "'='";
        case CoolLexer::DOT:    return "'.'";
        case CoolLexer::ATSIGN: return "'@'";


        case CoolLexer::LE: return "LE";
        case CoolLexer::ASSIGN: return "ASSIGN";
        case CoolLexer::DARROW: return "DARROW";

        case CoolLexer::INT_CONST: return "INT_CONST";
        case CoolLexer::STR_CONST: return "STR_CONST";
        case CoolLexer::BOOL_CONST: return "BOOL_CONST";

        case CoolLexer::CLASS:     return "CLASS";
        case CoolLexer::ELSE:      return "ELSE";
        case CoolLexer::FI:        return "FI";
        case CoolLexer::IF:        return "IF";
        case CoolLexer::IN:        return "IN";
        case CoolLexer::INHERITS:  return "INHERITS";
        case CoolLexer::ISVOID:    return "ISVOID";
        case CoolLexer::LET:       return "LET";
        case CoolLexer::LOOP:      return "LOOP";
        case CoolLexer::POOL:      return "POOL";
        case CoolLexer::THEN:      return "THEN";
        case CoolLexer::WHILE:     return "WHILE";
        case CoolLexer::CASE:      return "CASE";
        case CoolLexer::ESAC:      return "ESAC";
        case CoolLexer::OF:        return "OF";
        case CoolLexer::NEW:       return "NEW";
        case CoolLexer::NOT:       return "NOT";

        case CoolLexer::TYPEID:    return "TYPEID";
        case CoolLexer::OBJECTID:  return "OBJECTID";

        case CoolLexer::ERROR:     return "ERROR";
        
        default : return "<Invalid Token>: " + token->toString();
    }
}
static int compute_end_line(CoolLexer *lexer, Token *token, ANTLRInputStream *input) {
    size_t start = token->getStartIndex();
    size_t stop = token->getStopIndex();
    int line = token->getLine();

    if (start == INVALID_INDEX || stop == INVALID_INDEX)
        return line;

    if (token->getType() == CoolLexer::ERROR) {
        int idx = token->getStartIndex();
        if (lexer->str_error_lines.count(idx)) {
            return lexer->str_error_lines[idx];
        }
        if (lexer->get_error_message(idx) == "String contains unescaped new line") {
            return line;
        }
    }
    
    std::string text = input->getText(misc::Interval(start, stop));

    for (char c : text) {
        if (c == '\n')
            ++line;
    }

    return line;
}


void dump_cool_token(CoolLexer *lexer, ostream &out, Token *token) {
    if (token->getType() == static_cast<size_t>(-1)) {
        return;
    }

    auto input = dynamic_cast<antlr4::ANTLRInputStream*>(lexer->getInputStream());
    int endLine = compute_end_line(lexer, token, input);

    out << "#" << endLine << " " << cool_token_to_string(token);

    auto token_type = token->getType();
    auto token_start_char_index = token->getStartIndex();
    switch (token_type) {
        case CoolLexer::BOOL_CONST:
            out << " "
                << (lexer->get_bool_value(token_start_char_index) ? "true"
                                                                : "false");
            break;
        case CoolLexer::STR_CONST:
            out << " \"" << lexer->get_string_value(token_start_char_index) << "\"";
            break;
        case CoolLexer::ERROR: {
            std::string msg = lexer->get_error_message(token_start_char_index);
            std::string escaped;
            for (char c : msg) {
                if (c == '\\') escaped += "\\\\";
                else if (c == 0) escaped += "<0x00>";
                else escaped += c;
            }
            out << ": " << escaped;
            break;
            }
        case CoolLexer::INT_CONST: {
            out << ' ' << lexer->get_int_value(token_start_char_index);
            break;
        }
        case CoolLexer::OBJECTID: {
            out << ' ' << lexer->get_obj_value(token_start_char_index);
            break;
        }
        case CoolLexer::TYPEID: {
            out << ' ' << lexer->get_obj_value(token_start_char_index);
            break;
        }
    }
    out << endl;
}

int main(int argc, const char *argv[]) {
    ANTLRInputStream input(cin);
    CoolLexer lexer(&input);

    CommonTokenStream tokenStream(&lexer);

    tokenStream.fill();

    vector<Token *> tokens = tokenStream.getTokens();
    for (Token *token : tokens) {
        try {
            dump_cool_token(&lexer, cout, token);
        }
        catch (const std::out_of_range &e) {
            cerr << "Runtime error: missing map key for token "
                << token->getText() << " (type " << token->getType() << ")\n";
            }
    }

    return 0;
}
