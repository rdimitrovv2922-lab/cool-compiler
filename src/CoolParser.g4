parser grammar CoolParser;

options { tokenVocab=CoolLexer; }

/* -------------------------
   Program structure
   ------------------------- */

program
    : (class SEMI)+ EOF
    ;

class
    : CLASS TYPEID (INHERITS TYPEID)? LBRACE featureList RBRACE
    ;

featureList
    : (feature SEMI)*
    ;

/* -------------------------
   Features (attributes / methods)
   ------------------------- */

feature
    : OBJECTID COLON TYPEID (ASSIGN expr)?                     #attributeInit
    | OBJECTID COLON TYPEID                                  #attributeNoInit
    | OBJECTID LPAREN formalList? RPAREN COLON TYPEID LBRACE expr RBRACE  #method
    ;

formalList
    : formal (COMMA formal)*
    ;

formal
    : OBJECTID COLON TYPEID
    ;

/* -------------------------
   Expressions
   ------------------------- */

expr
    : OBJECTID ASSIGN expr                                     #assign
    | expr ATSIGN TYPEID DOT OBJECTID LPAREN actualList? RPAREN #staticDispatch
    | expr DOT OBJECTID LPAREN actualList? RPAREN              #dynamicDispatch
    | NOT expr                                                 #notExpr
    | comparisonExpr                                           #comparisonExprFallback
    ;

arithmeticExpr
    : arithmeticExpr PLUS term     #plus
    | arithmeticExpr MINUS term    #minus
    | term                         #arithmeticTerm 
    ;

term
    : term STAR factor             #mul
    | term SLASH factor            #div
    | factor                       #termFactor 
    ;

factor
    : TILDE factor                 #negate
    //| NOT factor                   #notExpr
    | ISVOID factor                #isvoidExpr
    | primaryExpr                  #factorPrimaryExpr
    ;

comparisonExpr
    : arithmeticExpr ( (LT | LE | EQUAL) arithmeticExpr )?
    ;

primaryExpr
    : OBJECTID LPAREN actualList? RPAREN          #funcCall
    | IF expr THEN expr ELSE expr FI              #ifExpr
    | WHILE expr LOOP expr POOL                   #whileExpr
    | LBRACE exprList RBRACE                      #block
    | LET letBinding (COMMA letBinding)* IN expr  #letExpr
    | CASE expr OF caseList ESAC                  #caseExpr
    | NEW TYPEID                                  #newExpr
    | LPAREN expr RPAREN                          #parenExpr
    | OBJECTID                                    #objId
    | INT_CONST                                   #intConst
    | STR_CONST                                   #stringConst
    | BOOL_CONST                                  #boolConst
    ;

/* END */

/* -------------------------
   Helper rules
   ------------------------- */

actualList
    : expr (COMMA expr)*
    ;

exprList
    : (expr SEMI)+
    ;

letBinding
    : OBJECTID COLON TYPEID (ASSIGN expr)?
    ;

caseList
    : caseBranch+
    ;

caseBranch
    : OBJECTID COLON TYPEID DARROW expr SEMI
    ;
