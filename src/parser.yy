%language "c++"
%skeleton "lalr1.cc" // skeleton for C++
%require "3.8" // %header is defined in bison 3.8
%header // generate a parser header file

%define api.token.constructor
%define api.location.file none // do not put location class in a separate file
%define api.value.type variant
%define parse.assert

%code requires {
  #include <string>
  #include <exception>
  #include <utility>
  class driver;
  class RootAST;
  class ExprAST;
  class FunctionAST;
  class SeqAST;
  class PrototypeAST;
  class IfExprNode;
  class ForExprAST;
  class VarExprAST;
  class WhileExprAST;
  class ArrayIndexingExprAST;
}

// The parsing context.
%param { driver& drv }

%locations

%define parse.trace
%define parse.error verbose

%code {
#include "driver.hh"
#include "operator.hh"
}

%define api.token.prefix {TOK_}
%token
  EOF  0  "end of file"
  SEMICOLON  ";"
  COMMA      ","
  MINUS      "-"
  PLUS       "+"
  STAR       "*"
  LT         "<"
  LE         "<="
  GT         ">"
  GE         ">="
  EQ         "=="
  NE         "!="
  EQUAL      "="
  SLASH      "/"
  LPAREN     "("
  RPAREN     ")"
  LSQUARE    "["
  RSQUARE    "]"
  EXTERN     "extern"
  DEF        "def"
  IF         "if"
  THEN       "then"
  ELSE       "else"
  COMPOUND   ":"
  FOR        "for"
  IN         "in"
  END        "end"
  VAR        "var"
  WHILE      "while"
;

%token <std::string> IDENTIFIER "id"
%token <double> NUMBER "number"

%type <ExprAST*> exp
%type <ExprAST*> idexp
%type <std::vector<ExprAST*>> optexp
%type <std::vector<ExprAST*>> explist
%type <RootAST*> program
%type <RootAST*> top
%type <FunctionAST*> definition
%type <PrototypeAST*> external
%type <PrototypeAST*> proto
%type <std::vector<std::string>> idseq
%type <IfExprNode*> ifexpr
%type <ForExprAST*> forexpr
%type <ExprAST*> step
%type <VarExprAST*> varexpr
%type <ExprAST*> assignment
%type <std::vector<std::pair<std::string, ExprAST*>>> varlist
%type <std::pair<std::string, ExprAST*>> pair
%type <WhileExprAST*> whileexpr
%type <ArrayIndexingExprAST*> arrayindexexpr

%left ":";
%precedence "=";
%left "<" ">" "<=" ">=" "==" "!=";
%left "+" "-";
%left "*" "/";

%%
%start startsymb;

startsymb
  : program { drv.root = $1; }
;

program
  : %empty          { $$ = new SeqAST(nullptr, nullptr); }
  | top ";" program { $$ = new SeqAST($1, $3); }

top
  : %empty     { $$ = nullptr; }
  | definition { $$ = $1; }
  | external   { $$ = $1; }
  | exp        { $$ = $1; $1->toggle(); }
;

definition
  : "def" proto exp { $$ = new FunctionAST($2, $3); 
                      $2->noemit(); }
;

external
  : "extern" proto { $$ = $2; }
;

proto
  : "id" "(" idseq ")" { $$ = new PrototypeAST($1,$3); }
;

idseq
  : %empty     { std::vector<std::string> args; $$ = args; }
  | "id" idseq { $2.insert($2.begin(), $1); $$ = $2; }
;

exp
  : "-" exp      { $$ = new UnaryExprAST(convertStringToOperator("-"), $2); }
  | exp "+" exp  { $$ = new BinaryExprAST(convertStringToOperator("+"), $1, $3); }
  | exp "-" exp  { $$ = new BinaryExprAST(convertStringToOperator("-"), $1, $3); }
  | exp "*" exp  { $$ = new BinaryExprAST(convertStringToOperator("*"), $1, $3); }
  | exp "/" exp  { $$ = new BinaryExprAST(convertStringToOperator("/"), $1, $3); }
  | exp "<" exp  { $$ = new BinaryExprAST(convertStringToOperator("<"), $1, $3); }
  | exp "<=" exp { $$ = new BinaryExprAST(convertStringToOperator("<="), $1, $3); }
  | exp ">" exp  { $$ = new BinaryExprAST(convertStringToOperator(">"), $1, $3); }
  | exp ">=" exp { $$ = new BinaryExprAST(convertStringToOperator(">="), $1, $3); }
  | exp "==" exp { $$ = new BinaryExprAST(convertStringToOperator("=="), $1, $3); }
  | exp "!=" exp { $$ = new BinaryExprAST(convertStringToOperator("!="), $1, $3); }
  | exp ":" exp  { $$ = new BinaryExprAST(convertStringToOperator(":"), $1, $3); }
  | ifexpr         { $$ = $1; }
  | forexpr        { $$ = $1; }
  | whileexpr      { $$ = $1; }
  | varexpr        { $$ = $1; }
  | assignment     { $$ = $1; }
  | idexp          { $$ = $1; }
  | arrayindexexpr { $$ = $1; }
  | "(" exp ")"    { $$ = $2; }
  | "number"       { $$ = new NumberExprAST($1); }
;

idexp
  : "id"                { $$ = new VariableExprAST($1); }
  | "id" "(" optexp ")" { $$ = new CallExprAST($1,$3); }
;

ifexpr
  : "if" exp "then" exp "end"            { $$ = new IfExprNode($2, $4, nullptr); }
  | "if" exp "then" exp "else" exp "end" { $$ = new IfExprNode($2, $4, $6); }
;

forexpr
  : "for" "id" "=" exp "," exp step "in" exp "end" { $$ = new ForExprAST($2, $4, $6, $7, $9); }
;

step
  : %empty  { $$ = nullptr; }
  | "," exp { $$ = $2; }
;

optexp
  : %empty  { std::vector<ExprAST*> args; args.push_back(nullptr); $$ = args; }
  | explist { $$ = $1; }
;

explist
  : exp { 
          std::vector<ExprAST*> args; 
          args.push_back($1); 
          $$ = args; 
        }
  | exp "," explist { $3.insert($3.begin(), $1); $$ = $3; }
;

varexpr
  : "var" varlist "in" exp "end"    { $$ = new VarExprAST($2, $4); }
;

varlist
  : pair             { std::vector<std::pair<std::string, ExprAST*>> list; list.push_back($1); $$ = list; }
  | pair "," varlist { $3.insert($3.begin(), $1); $$ = $3; }
;

pair
  : "id"                  { $$ = std::pair($1, new NumberExprAST(0.0)); }
  | "id" "=" exp          { $$ = std::pair($1, $3); }
  | "id" "[" "number" "]" { $$ = std::pair($1, new ArrayInitExprAST($1, static_cast<unsigned int>($3))); }
;

assignment
  : "id" "=" exp { $$ = new BinaryExprAST(convertStringToOperator("="), new VariableExprAST($1), $3); }
  | arrayindexexpr "=" exp { $$ = new BinaryExprAST(convertStringToOperator("="), $1, $3); }
;

whileexpr
  : "while" exp "in" exp "end" { $$ = new WhileExprAST($2, $4); }
;

arrayindexexpr
  : "id" "[" exp "]" { $$ = new ArrayIndexingExprAST($1, $3); }
;
%%

void yy::parser::error(const location_type& location, const std::string& message)
{
    std::cerr << location << ": " << message << '\n';
}
