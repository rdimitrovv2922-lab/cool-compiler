#include <cctype>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "antlr4-runtime/antlr4-runtime.h"

#include "CoolLexer.h"
#include "CoolParser.h"
#include "CoolParserBaseVisitor.h"

using namespace std;
using namespace antlr4;

namespace fs = filesystem;

string indent = "";

/// Converts token to coursework-specific string representation.
string cool_token_to_string(CoolLexer *lexer, Token *token) {
    auto token_type = token->getType();

    // clang-format off
    switch (token_type) {
        case static_cast<size_t>(-1) : return "EOF";

        case CoolLexer::SEMI   : return "';'";
        case CoolLexer::LBRACE : return "'{'";
        case CoolLexer::RBRACE : return "'}'";
        case CoolLexer::LPAREN : return "'('";
        case CoolLexer::COMMA  : return "','";
        case CoolLexer::RPAREN : return "')'";
        case CoolLexer::COLON  : return "':'";
        case CoolLexer::ATSIGN : return "'@'";
        case CoolLexer::DOT    : return "'.'";
        case CoolLexer::PLUS   : return "'+'";
        case CoolLexer::MINUS  : return "'-'";
        case CoolLexer::STAR   : return "'*'";
        case CoolLexer::SLASH  : return "'/'";
        case CoolLexer::TILDE  : return "'~'";
        case CoolLexer::LT     : return "'<'";
        case CoolLexer::EQUAL  : return "'='";

        case CoolLexer::DARROW : return "DARROW";
        case CoolLexer::ASSIGN : return "ASSIGN";
        case CoolLexer::LE     : return "LE";

        case CoolLexer::CLASS  : return "CLASS";
        case CoolLexer::ELSE   : return "ELSE";
        case CoolLexer::FI     : return "FI";
        case CoolLexer::IF     : return "IF";
        case CoolLexer::IN     : return "IN";
        case CoolLexer::INHERITS : return "INHERITS";
        case CoolLexer::ISVOID : return "ISVOID";
        case CoolLexer::LET    : return "LET";
        case CoolLexer::LOOP   : return "LOOP";
        case CoolLexer::POOL   : return "POOL";
        case CoolLexer::THEN   : return "THEN";
        case CoolLexer::WHILE  : return "WHILE";
        case CoolLexer::CASE   : return "CASE";
        case CoolLexer::ESAC   : return "ESAC";
        case CoolLexer::NEW    : return "NEW";
        case CoolLexer::OF     : return "OF";
        case CoolLexer::NOT    : return "NOT";

        case CoolLexer::BOOL_CONST : return "BOOL_CONST";
        case CoolLexer::INT_CONST  : return "INT_CONST = " + token->getText();
        case CoolLexer::STR_CONST  : return "STR_CONST";
        case CoolLexer::TYPEID     : return "TYPEID = " + token->getText();
        case CoolLexer::OBJECTID   : return "OBJECTID = " + token->getText();
        case CoolLexer::ERROR      : return "ERROR";

        default : return "<Invalid Token>: " + token->toString();
    }
    // clang-format on
}

class TreePrinter : public CoolParserBaseVisitor {
  private:
    CoolLexer *lexer_;
    CoolParser *parser_;
    string file_name_;

  public:
    TreePrinter(CoolLexer *lexer, CoolParser *parser, const string &file_name)
        : lexer_(lexer), parser_(parser), file_name_(file_name) {}

    any visitProgram(CoolParser::ProgramContext *ctx) override {
        Token *lastReal = parser_->getTokenStream()->LT(-1);

        while (lastReal->getType() == Token::EOF
            || lastReal->getChannel() == Token::HIDDEN_CHANNEL) {
            lastReal = parser_->getTokenStream()->LT(lastReal->getTokenIndex() - 1);
        }

        cout << "#" << lastReal->getLine() << endl;

        cout << "_program" << endl;

        indent += "  ";
        visitChildren(ctx);
        indent.resize(indent.size() - 2);  // remove 2 spaces

        return any{};
    }

    any visitClass(CoolParser::ClassContext *ctx) override {
        cout << indent << "#" << ctx->getStop()->getLine() << endl;
        cout << indent << "_class" << endl;

        cout << indent << "  " << ctx->TYPEID(0)->getText() << endl;

        if (ctx->TYPEID().size() == 2)
            cout << indent << "  " << ctx->TYPEID(1)->getText() << endl;
        else
            cout << indent << "  Object" << endl;

        cout << indent <<  "  \"" << file_name_ << '"' << endl;
        cout << indent << "  (" << endl;

        if (ctx->featureList()) {
            indent += "  ";
            visit(ctx->featureList());
            indent.resize(indent.size() - 2);  // remove 2 spaces
        }

         
        cout << indent << "  )" << endl;
        return any{};
    }

    any visitFeatureList(CoolParser::FeatureListContext *ctx) override {
        for (auto f : ctx->feature()) {
            visit(f);
        }

        return any{};
    }

    any visitAttributeInit(CoolParser::AttributeInitContext *ctx) override {
        int line = ctx->getStart()->getLine();
        cout << indent << "#" << ctx->getStop()->getLine() << endl;
        cout << indent << "_attr" << endl;

        indent += "  "; // increase indent 
        cout << indent << ctx->OBJECTID()->getText() << endl;
        cout << indent << ctx->TYPEID()->getText() << endl;

        if (ctx->expr()) {
            int expr_line = ctx->expr()->getStart()->getLine();
            cout << indent << "#" << expr_line << endl;
            cout << indent << "_expr" << endl;

            indent += "  ";
            visit(ctx->expr());
            indent.resize(indent.size() - 2);  // remove 2 spaces
        } else {
            cout << indent << "#" << line << endl;
            cout << indent << "_no_expr" << endl;
        }
        
        cout << indent << ": _no_type" << endl;

        indent.resize(indent.size() - 2);  // remove indent
        return any{};
    }

    any visitAttributeNoInit(CoolParser::AttributeNoInitContext *ctx) override {
        int line = ctx->getStart()->getLine();
        cout << indent << "#" << line << endl;
        cout << indent << "_attr" << endl;

        indent += "  "; // increase indent
        cout << indent << ctx->OBJECTID()->getText() << endl;
        cout << indent << ctx->TYPEID()->getText() << endl;

        cout << indent << "#" << line << endl;
        cout << indent << "_no_expr" << endl;

        cout << indent << ": _no_type" << endl;

        indent.resize(indent.size() - 2);  // remove indent
        return any{};
    }

    any visitMethod(CoolParser::MethodContext *ctx) override {
        cout << indent  << "#" << ctx->getStop()->getLine() << endl;
        cout << indent << "_method" << endl;

        indent += "  "; // increase indent
        cout << indent << ctx->OBJECTID()->getText() << endl;

        if (ctx->formalList()) {
            visit(ctx->formalList());
        }

        if (ctx->TYPEID()) {
            cout << indent << ctx->TYPEID()->getText() << endl;
        }

        if (ctx->expr()) {
            if (auto sd = dynamic_cast<CoolParser::StaticDispatchContext*>(ctx->expr())) {
                visitStaticDispatch(sd);
            } else if (auto dd = dynamic_cast<CoolParser::DynamicDispatchContext*>(ctx->expr())) {
                visitDynamicDispatch(dd);
            } else if (auto obj = dynamic_cast<CoolParser::ObjIdContext*>(ctx->expr())) {
                visitObjId(obj);
            } else {
                visit(ctx->expr());
            }
        }

        indent.resize(indent.size() - 2);  // remove indent
        return any{};
    }

    any visitFormalList(CoolParser::FormalListContext *ctx) override {
        for (auto f : ctx->formal()) {
            visit(f);
        }
        return any{};
    }

    any visitFormal(CoolParser::FormalContext *ctx) override {
        int line = ctx->getStart()->getLine();
        cout << indent << "#" << ctx->getStop()->getLine() << endl;
        cout << indent << "_formal" << endl;

        indent += "  "; // increase indent
        cout << indent  << ctx->OBJECTID()->getText() << endl;
        cout << indent << ctx->TYPEID()->getText() << endl;

        indent.resize(indent.size() - 2);  // remove indent
        return any{};
    }

    any visitAssign(CoolParser::AssignContext *ctx) override {
        int rhs_line = ctx->expr()->getStart()->getLine();
        cout << indent << "#" << rhs_line << endl;
        cout << indent << "_assign" << endl;

        indent += "  "; // increase indent
        cout << indent << ctx->OBJECTID()->getText() << endl;
        
        visit(ctx->expr());
        
        indent.resize(indent.size() - 2);  // remove indent
        cout << indent << ": _no_type" << endl;
        return any{};
    }

    any visitObjId(CoolParser::ObjIdContext *ctx) override {
        int line = ctx->getStart()->getLine();
        cout << indent << "#" << line << endl;
        cout << indent << "_object" << endl;

        indent += "  "; // increase indent
        cout << indent  << ctx->OBJECTID()->getText() << endl;

        indent.resize(indent.size() - 2);  // remove 2 spaces
        cout << indent << ": _no_type" << endl;

        //indent.resize(indent.size() - 2);  // remove indent
        return any{};
    }

    any visitStaticDispatch(CoolParser::StaticDispatchContext *ctx) override {
        int line = ctx->getStart()->getLine();
        cout << indent << "#" << line << endl;
        cout << indent << "_static_dispatch" << endl;

        indent += "  ";
        visit(ctx->expr());
        indent.resize(indent.size() - 2);  // remove 2 spaces  

        indent += "  "; // increase indent
        cout << indent  << ctx->TYPEID()->getText() << endl;
        cout << indent  << ctx->OBJECTID()->getText() << endl;

        cout << indent << "(" << endl;
        if (ctx->actualList()) {
            visit(ctx->actualList());
        }   

        cout << indent << ")" << endl;

        indent.resize(indent.size() - 2);  // remove indent
        cout << indent << ": _no_type" << endl;
        return any{};
    }

    any visitDynamicDispatch(CoolParser::DynamicDispatchContext *ctx) override {
        int line = ctx->getStart()->getLine();
        cout << indent << "#" << line << endl;
        cout << indent << "_dispatch" << endl;
        
        indent += "  ";
        visit(ctx->expr());
        indent.resize(indent.size() - 2);  // remove 2 spaces
         
        indent += "  "; // increase indent       
        cout << indent  << ctx->OBJECTID()->getText() << endl;

        cout << indent << "(" << endl;
        if (ctx->actualList()) {
            visit(ctx->actualList());
        }    
        cout << indent << ")" << endl;

        indent.resize(indent.size() - 2);  // remove indent
        cout << indent << ": _no_type" << endl;
        return any{};
    }

    any visitActualList(CoolParser::ActualListContext *ctx) override {
        for (auto e : ctx->expr()) {
            visit(e);
        }
        return any{};
    }

    any visitFuncCall(CoolParser::FuncCallContext *ctx) override {
        int line = ctx->getStart()->getLine();
        cout << indent << "#" << line << endl;
        cout << indent << "_dispatch" << endl;

        indent += "  ";
        cout << indent << "#" << line << endl;
        cout << indent << "_object" << endl;

        indent += "  ";
        cout << indent << "self" << endl;
        indent.resize(indent.size() - 2);  // remove 2 spaces

        cout << indent << ": _no_type" << endl;
        indent.resize(indent.size() - 2);  // remove 2 spaces
         
        indent += "  "; // increase indent       
        cout << indent  << ctx->OBJECTID()->getText() << endl;

        cout << indent << "(" << endl;
        if (ctx->actualList()) {
            visit(ctx->actualList());
        }    
        cout << indent << ")" << endl;

        indent.resize(indent.size() - 2);  // remove indent
        cout << indent << ": _no_type" << endl;
        return any{};
    }

    any visitIfExpr(CoolParser::IfExprContext *ctx) override {
        int line = ctx->getStart()->getLine();
        cout << indent << "#" << ctx->getStop()->getLine() << endl;
        cout << indent << "_cond" << endl;

        indent += "  ";
        for (auto expr : ctx->expr()) {
            visit(expr);
        }
        
        indent.resize(indent.size() - 2);  // remove 2 spaces
        cout << indent << ": _no_type" << endl;
        return any{};
    }

    /*any visitWhileExpr(CoolParser::WhileExprContext *ctx) override {
        int line = ctx->getStart()->getLine();
        cout << indent << "#" << ctx->getStop()->getLine() << endl;
        cout << indent << "_loop" << endl;

        indent += "  ";
        cout << indent << "#" << ctx->getStop()->getLine() << endl;
        cout << indent << "_comp" << endl;
        indent += "  ";
        visit(ctx->expr(0));
        indent.resize(indent.size() - 2);  // remove 2 spaces
        cout << indent << ": _no_type" << endl;

        //indent += "  ";
        visit(ctx->expr(1));

        indent.resize(indent.size() - 2);  // remove 2 spaces
        cout << indent << ": _no_type" << endl;
        return any{};
    }*/

    any visitWhileExpr(CoolParser::WhileExprContext *ctx) override {
        cout << indent << "#" << ctx->getStop()->getLine() << endl;
        cout << indent << "_loop" << endl;

        indent += "  ";

        auto cond = ctx->expr(0);
        if (auto comp = dynamic_cast<CoolParser::ComparisonExprContext*>(cond)) {
            cout << indent << "#" << cond->getStart()->getLine() << endl;
            cout << indent << "_comp" << endl;
            indent += "  ";
            visitComparisonExpr(comp);  
            indent.resize(indent.size() - 2);
            cout << indent << ": _no_type" << endl;
        } else {
            visit(cond);
        }

        visit(ctx->expr(1));

        indent.resize(indent.size() - 2);
        cout << indent << ": _no_type" << endl;

        return any{};
    }
    
    any visitExprList(CoolParser::ExprListContext *ctx) override {
        for (auto expr : ctx->expr()) {
            visit(expr);
        }
        return any{};
    }

    any visitBlock(CoolParser::BlockContext *ctx) override {
        int line = ctx->getStart()->getLine();
        cout << indent << "#" << ctx->getStop()->getLine() << endl;
        cout << indent << "_block" << endl;

        indent += "  ";
        if (ctx->exprList()) {
            visit(ctx->exprList());
        }
        
        indent.resize(indent.size() - 2);  // remove 2 spaces
        cout << indent << ": _no_type" << endl;
        return any{};
    }

    any visitLetExpr(CoolParser::LetExprContext *ctx) override {
        int line = ctx->getStop()->getLine();
        cout << indent << "#" << line << endl;
        cout << indent << "_let" << endl;
        auto firstBinding = ctx->letBinding(0);
        visit(firstBinding);
        
        size_t stacked = 0;
        if(ctx->letBinding(1)){
            for (size_t i = 1; i < ctx->letBinding().size(); ++i) {
                stacked++;
                cout << indent << "#" << line << endl;
                cout << indent << "_let" << endl;
                visit(ctx->letBinding(i));           
            }
        }

        visit(ctx->expr());

        for(size_t i = 0; i < stacked; ++i){
            indent.resize(indent.size() - 2);
            cout << indent << ": _no_type" << endl;
        }

        indent.resize(indent.size() - 2); // remove 2 spaces
        cout << indent << ": _no_type" << endl;
        return any{};
    }

    any visitLetBinding(CoolParser::LetBindingContext *ctx) override {
        int line = ctx->getStart()->getLine();
        indent += "  ";
        cout << indent << ctx->OBJECTID()->getText() << endl;
        cout << indent << ctx->TYPEID()->getText() << endl;

        if (ctx->expr()) {
            /*int expr_line = ctx->expr()->getStart()->getLine();
            cout << indent << "#" << expr_line << endl;
            cout << indent << "_expr" << endl;*/

            //indent += "  ";
            visit(ctx->expr());
            //indent.resize(indent.size() - 2);  // remove 2 spaces
        } else {
            cout << indent << "#" << line << endl;
            cout << indent << "_no_expr" << endl;
            cout << indent << ": _no_type" << endl;
        }
        
        //cout << indent << ": _no_type" << endl;
        return any{};
    }

    any visitCaseBranch(CoolParser::CaseBranchContext *ctx) override {
        int line = ctx->getStart()->getLine();
        cout << indent << "#" << ctx->getStop()->getLine() << endl;
        cout << indent << "_branch" << endl;

        indent += "  "; // increase indent
        cout << indent  << ctx->OBJECTID()->getText() << endl;
        cout << indent << ctx->TYPEID()->getText() << endl;

        if (ctx->expr()) {
            visit(ctx->expr());
        }        

        indent.resize(indent.size() - 2);  // remove indent
        return any{};
    }

    any visitCaseList(CoolParser::CaseListContext *ctx) override {
        for (auto branch : ctx->caseBranch()) {
            visit(branch);
        }
        return any{};
    }

    any visitCaseExpr(CoolParser::CaseExprContext *ctx) override {
        int line = ctx->getStart()->getLine();
        cout << indent << "#" << ctx->getStop()->getLine() << endl;
        cout << indent << "_typcase" << endl;

        indent += "  ";
        visit(ctx->expr());
        indent.resize(indent.size() - 2);  // remove 2 spaces  

        indent += "  ";
        if (ctx->caseList()) {
            visit(ctx->caseList());
        }   
        indent.resize(indent.size() - 2);  // remove 2 spaces  

        //indent.resize(indent.size() - 2);  // remove indent
        cout << indent << ": _no_type" << endl;
        return any{};
    } 

    any visitIntConst(CoolParser::IntConstContext *ctx) override {
        int line = ctx->getStart()->getLine();
        cout << indent << "#" << line << endl;
        cout << indent << "_int" << endl;

        indent += "  "; // increase indent
        cout << indent  << ctx->INT_CONST()->getText() << endl;

        indent.resize(indent.size() - 2);  // remove 2 spaces
        cout << indent << ": _no_type" << endl;

        return any{};
    }

    any visitStringConst(CoolParser::StringConstContext *ctx) override {
        int line = ctx->getStart()->getLine();
        cout << indent << "#" << line << endl;
        cout << indent << "_string" << endl;

        indent += "  "; // increase indent
        cout << indent  << ctx->STR_CONST()->getText() << endl;

        indent.resize(indent.size() - 2);  // remove 2 spaces
        cout << indent << ": _no_type" << endl;

        return any{};
    }

    any visitBoolConst(CoolParser::BoolConstContext *ctx) override {
        int line = ctx->getStart()->getLine();
        cout << indent << "#" << line << endl;
        cout << indent << "_bool" << endl;

        std::string value = ctx->BOOL_CONST()->getText();
        int booleanValue = (value == "true") ? 1 : 0;
        indent += "  "; // increase indent
        cout << indent  << booleanValue << endl;

        indent.resize(indent.size() - 2);  // remove 2 spaces
        cout << indent << ": _no_type" << endl;

        return any{};
    }

    any visitPlus(CoolParser::PlusContext *ctx) override {
        int line = ctx->getStart()->getLine();
        cout << indent << "#" << line << endl;
        cout << indent << "_plus" << endl;

        indent += "  "; // increase indent
        visit(ctx->arithmeticExpr());
        visit(ctx->term());
        indent.resize(indent.size() - 2);  // remove 2 spaces
        cout << indent << ": _no_type" << endl;

        return any{};
    }

    any visitMul(CoolParser::MulContext *ctx) override {
        int line = ctx->getStart()->getLine();
        cout << indent << "#" << line << endl;
        cout << indent << "_mul" << endl;

        indent += "  "; // increase indent
        visit(ctx->term());
        visit(ctx->factor());
        indent.resize(indent.size() - 2);  // remove 2 spaces
        cout << indent << ": _no_type" << endl;

        return any{};
    }

    any visitMinus(CoolParser::MinusContext *ctx) override {
        int line = ctx->getStart()->getLine();
        cout << indent << "#" << line << endl;
        cout << indent << "_sub" << endl;

        indent += "  "; // increase indent
        visit(ctx->arithmeticExpr());
        visit(ctx->term());
        indent.resize(indent.size() - 2);  // remove 2 spaces
        cout << indent << ": _no_type" << endl;

        return any{};
    }

    any visitDiv(CoolParser::DivContext *ctx) override {
        int line = ctx->getStart()->getLine();
        cout << indent << "#" << line << endl;
        cout << indent << "_divide" << endl;

        indent += "  "; // increase indent
        visit(ctx->term());
        visit(ctx->factor());
        indent.resize(indent.size() - 2);  // remove 2 spaces
        cout << indent << ": _no_type" << endl;

        return any{};
    }

    any visitNegate(CoolParser::NegateContext *ctx) override {
        int line = ctx->getStart()->getLine();
        cout << indent << "#" << line << endl;
        cout << indent << "_neg" << endl;

        indent += "  "; // increase indent
        visit(ctx->factor());
        indent.resize(indent.size() - 2);  // remove 2 spaces
        cout << indent << ": _no_type" << endl;

        return any{};
    }

    any visitComparisonExpr(CoolParser::ComparisonExprContext *context) override {
        int line = context->getStart()->getLine();
        
        std::string op;
        if (context->LT()) op = "_lt";
        else if (context->LE()) op = "_leq";
        else if (context->EQUAL()) op = "_eq";
        else {
            return visit(context->arithmeticExpr(0));
        }

        cout << indent << "#" << line << endl;
        cout << indent << op << endl;

        indent += "  ";
        visit(context->arithmeticExpr(0));   // left side
        visit(context->arithmeticExpr(1));   // right side
        indent.resize(indent.size() - 2);

        cout << indent << ": _no_type" << endl;

        return any{};
    }

    any visitIsvoidExpr(CoolParser::IsvoidExprContext *context) override {
        int line = context->getStart()->getLine();
        cout << indent << "#" << line << endl;
        cout << indent << "_isvoid" << endl;

        indent += "  "; // increase indent
        visit(context->factor());
        indent.resize(indent.size() - 2);  // remove 2 spaces
        cout << indent << ": _no_type" << endl;

        return any{};
    }

    any visitNewExpr(CoolParser::NewExprContext *context) override {
        int line = context->getStart()->getLine();
        cout << indent << "#" << line << endl;
        cout << indent << "_new" << endl;

        indent += "  "; // increase indent
        cout << indent  << context->TYPEID()->getText() << endl;

        indent.resize(indent.size() - 2);  // remove 2 spaces
        cout << indent << ": _no_type" << endl;

        return any{};
    }

    any visitNotExpr(CoolParser::NotExprContext *context) override {
        int line = context->getStart()->getLine();
        cout << indent << "#" << line << endl;
        cout << indent << "_comp" << endl;

        indent += "  "; // increase indent
        //visit(context->factor());
        visit(context->expr());
        indent.resize(indent.size() - 2);  // remove 2 spaces
        cout << indent << ": _no_type" << endl;

        return any{};
    }

  public:
    void print() { visitProgram(parser_->program()); }
};

class ErrorPrinter : public BaseErrorListener {
  private:
    string file_name_;
    CoolLexer *lexer_;
    bool has_error_ = false;

  public:
    ErrorPrinter(const string &file_name, CoolLexer *lexer)
        : file_name_(file_name), lexer_(lexer) {}

    virtual void syntaxError(Recognizer *recognizer, Token *offendingSymbol,
                             size_t line, size_t charPositionInLine,
                             const std::string &msg,
                             std::exception_ptr e) override {
        has_error_ = true;
        cout << '"' << file_name_ << "\", line " << line
             << ": syntax error at or near "
             << cool_token_to_string(lexer_, offendingSymbol) << endl;
    }

    bool has_error() const { return has_error_; }
};

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        cerr << "Expecting exactly one argument: name of input file" << endl;
        return 1;
    }

    auto file_path = argv[1];
    ifstream fin(file_path);

    auto file_name = fs::path(file_path).filename().string();

    ANTLRInputStream input(fin);
    CoolLexer lexer(&input);

    CommonTokenStream tokenStream(&lexer);

    CoolParser parser(&tokenStream);

    ErrorPrinter error_printer(file_name, &lexer);

    parser.removeErrorListener(&ConsoleErrorListener::INSTANCE);
    parser.addErrorListener(&error_printer);

    // This will trigger the error_printer, in case there are errors.
    parser.program();
    parser.reset();

    if (!error_printer.has_error()) {
        TreePrinter(&lexer, &parser, file_name).print();
    } else {
        cout << "Compilation halted due to lex and parse errors" << endl;
    }

    return 0;
}
