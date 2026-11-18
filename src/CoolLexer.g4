lexer grammar CoolLexer;

@lexer::header {
#include <cctype>
#include <cstdio>
#include <map>
#include <string>
}

@lexer::members {
    const unsigned MAX_STR_CONST = 1024;

    std::map<int,int> str_error_lines;

    std::map<int, bool> bool_values;
    void assoc_bool_with_token(bool value) { bool_values[tokenStartCharIndex] = value; }
    bool get_bool_value(int idx) { return bool_values.at(idx); }

    //---------string---------------------------------------------
    std::map<int, std::string> string_values;
    void assoc_string_with_token(const std::string &value) {
        std::string escaped;
        escaped.reserve(value.size() * 2);
        for (unsigned char c : value) {
            switch (c) {
                case '\n': escaped += "\\n"; break;
                case '\\': escaped += "\\\\"; break;
                case '"':  escaped += "\\\""; break;
                case '\t': escaped += "\\t"; break;
                case '\f': escaped += "\\f"; break;
                case '\b': escaped += "\\b"; break;  
                default:
                    if (std::isprint(c))
                        escaped.push_back(c);
                    else {
                        char buf[10];
                        snprintf(buf, sizeof(buf), "<0x%02x>", c);
                        escaped += buf;
                    }
                    break;
            }
        }
        string_values[tokenStartCharIndex] = escaped;
    }

    std::string get_string_value(int idx) {
        auto it = string_values.find(idx);
        return it != string_values.end() ? it->second : "";
    }

    //------------------error----------------------------------------
    std::map<int, std::string> error_messages;
    void assoc_error_with_token(const std::string &msg) { error_messages[tokenStartCharIndex] = msg; }
    std::string get_error_message(int idx) {
        auto it = error_messages.find(idx);
        return it != error_messages.end() ? it->second : "";
    }

    //---------------------int----------------------------------------
    std::map<int, std::string> int_values;
    void assoc_int_with_token(const std::string &value){
        int_values[tokenStartCharIndex] = value;
    }
    std::string get_int_value(int idx){
        auto it = int_values.find(idx);
        return it != int_values.end() ? it->second : "";    
    }

    //---------------------------object--------------------------------
    std::map<int, std::string> obj_values;
    void assoc_obj_with_token(const std::string &msg) {
        obj_values[tokenStartCharIndex] = msg;
    }
    std::string get_obj_value(int idx){
        auto it = obj_values.find(idx);
        return it != obj_values.end() ? it->second : "";
    }

    std::string unterminated_string_error(const std::string &raw, bool ends_at_newline) {
        size_t content_end = raw.size();
        if (ends_at_newline && content_end > 0 && raw[content_end - 1] == '\n') {
            --content_end;
            if (content_end > 1 && raw[content_end - 1] == '\r')
                --content_end;
        }

        size_t value_length = 0;
        for (size_t i = 1; i < content_end; ++i) {
            char c = raw[i];
            if (c == '\0')
                return "String contains null character";

            if (c == '\\' && i + 1 < content_end) {
                char next = raw[++i];
                if (next == '\0')
                    return "String contains escaped null character";
            }

            if (++value_length > MAX_STR_CONST)
                return "String constant too long";
        }

        return ends_at_newline
            ? "String contains unescaped new line"
            : "Unterminated string at EOF";
    }
}

// Punctuation and operators ---------------------------------------------

SEMI   : ';';
DARROW : '=>';
ASSIGN : '<-';
LE     : '<=';
LT     : '<';
EQUAL  : '=';
PLUS   : '+';
MINUS  : '-';
STAR   : '*';
SLASH  : '/';
TILDE  : '~';
LPAREN : '(';
RPAREN : ')';
LBRACE : '{';
RBRACE : '}';
COLON  : ':';
COMMA  : ',';
DOT    : '.';
ATSIGN : '@';

// Keywords ---------------------------------------------------------------

CLASS    : [cC][lL][aA][sS];
ELSE     : [eE][lL][sS][eE] ;
FI       : [fF][iI] ;
IF       : [iI][fF] ;
IN       : [iI][nN];
INHERITS : [iI][nN][hH][eE][rR][iI][tT][sS];
ISVOID   : [iI][sS][vV][oO][iI][dD];
LET      : [lL][eE][tT];
LOOP     : [lL][oO][oO][pP];
POOL     : [pP][oO][oO][lL];
THEN     : [tT][hH][eE][nN];
WHILE    : [wW][hH][iI][lL][eE];
CASE     : [cC][aA][sS][eE];
ESAC     : [eE][sS][aA][cC];
NEW      : [nN][eE][wW];
OF       : [oO][fF];
NOT      : [nN][oO][tT];

// Boolean literals are case-insensitive after their required lowercase first letter.
BOOL_CONST
    : [t][Rr][Uu][Ee]  { assoc_bool_with_token(true); }
    | [f][Aa][Ll][Ss][Ee] { assoc_bool_with_token(false); }
    ;

// Identifiers and integers ----------------------------------------------

TYPEID   : [A-Z][a-zA-Z0-9_]* {
        assoc_obj_with_token(getText());    
    }
    ;

OBJECTID : [a-z][a-zA-Z0-9_]* {
        assoc_obj_with_token(getText());    
    }
    ;

INT_CONST
    : [0-9]+ {
        assoc_int_with_token(getText());
    }
    ;

// Comments and whitespace ------------------------------------------------

LINE_COMMENT : '--' ~[\r\n]* -> skip;

BLOCK_COMMENT_START : '(*' -> pushMode(COMMENT), more;

WS : [ \t\r\n\f]+ -> skip;

// Lexical errors ---------------------------------------------------------

UNTERMINATED_STR_NEWLINE
    : '"' ( '\\' . | ~["\\\n] )* '\r'? '\n'
      {
        setType(CoolLexer::ERROR);
        assoc_error_with_token(unterminated_string_error(getText(), true));
      }
    ;

UNTERMINATED_STR_EOF
    : '"' ( '\\' . | '\\' | ~["\\\n] )* EOF
      {
        setType(CoolLexer::ERROR);
        assoc_error_with_token(unterminated_string_error(getText(), false));
      }
    ;

UNMATCHED_BR_END
    : '*)' 
        {
            setType(CoolLexer::ERROR);
            assoc_error_with_token("Unmatched *)");
        }
    ;
    
ERROR
    : . {
        setType(CoolLexer::ERROR);
        char c ;
        c = getText()[0]; 
        if (std::isprint(static_cast<unsigned char>(c))) {
            assoc_error_with_token("Invalid symbol \"" + getText() + "\""); 
        } else {
            char buf[10];
            snprintf(buf, sizeof(buf), "<0x%02x>", static_cast<unsigned char>(c));
            assoc_error_with_token(std::string("Invalid symbol \"") + buf + "\"");
        }   
    } 
    ;

// String constants -------------------------------------------------------

STR_CONST
    : '"' STR_CHARS* '"' 
      {{
          std::string raw = getText();
          std::string result;
          bool error = false;

          for (size_t i = 1; i + 1 < raw.size(); ++i) {
              char c = raw[i];
                
                if (c == '\0') {
                    setType(CoolLexer::ERROR);
                    assoc_error_with_token("String contains null character");
                    error = true;
                    break;
                }

                if (c == '\\') {
                    if (i + 1 >= raw.size() - 1) {
                        setType(CoolLexer::ERROR);
                        assoc_error_with_token("String contains unescaped new line");
                        error = true;
                        break;
                    } 

                    char next = raw[++i];

                    if (next == '\0') {
                        setType(CoolLexer::ERROR);
                        assoc_error_with_token("String contains escaped null character");
                        error = true;
                        break;
                    }

                    switch (next) {
                        case 'b': c = '\b'; break;
                        case 't': c = '\t'; break;
                        case 'n': c = '\n'; break;
                        case 'f': c = '\f'; break;
                        case '\\': c = '\\'; break;
                        case '"': c = '"'; break;
                        default: c = next; break;
                    }
                    result.push_back(c);
                }
                else if (c == '\n') {
                    setType(CoolLexer::ERROR);
                    assoc_error_with_token("String contains unescaped new line");
                    error = true;
                    break;
                }
                else {
                    result.push_back(c);
                }

                if (result.size() > MAX_STR_CONST) {
                    setType(CoolLexer::ERROR);
                    assoc_error_with_token("String constant too long");
                    int startLine = getLine();
                    str_error_lines[tokenStartCharIndex] = startLine;
                    error = true;
                    break;
                }
            }

            if (!error)
                 assoc_string_with_token(result);
        }}
    ;

fragment STR_CHARS
    : '\\' .
    | ~["\\\n]
    ;

mode COMMENT;

    COMMENT_START : '(*' -> pushMode(COMMENT), more;
    COMMENT_END   : '*)' -> popMode, skip;
    COMMENT_CHAR  : . -> more;
    UNCLOSED_COMMENT
        : EOF {
            setType(CoolLexer::ERROR);
            assoc_error_with_token("Unmatched (*");
            str_error_lines[tokenStartCharIndex] = getLine() - 1;
        }
    ;
