%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "3.2"
%defines

%define api.token.constructor
%define api.location.file none
%define api.value.type variant
%define parse.assert

%code requires {
  #include <string>
  #include <exception>
  class driver;
  class RootAST;
  class ExprAST;
  class FunctionAST;
  class SeqAST;
  class PrototypeAST;
  class IfExprNode;
}

// The parsing context.
%param { driver& drv }

%locations

%define parse.trace
%define parse.error verbose

%code {
#include "driver.hh"
}

%define api.token.prefix {TOK_}
%token
  END  0  "end of file"
  SEMICOLON  ";"
  COMMA      ","
  MINUS      "-"
  PLUS       "+"
  STAR       "*"
  LT         "<"
  GT         ">"
  SLASH      "/"
  LPAREN     "("
  RPAREN     ")"
  EXTERN     "extern"
  DEF        "def"
  IF         "if"
  THEN       "then"
  ELSE       "else"
  FI         "fi"
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
%type <IfExprNode*> ifexp

%%
%start startsymb;

startsymb:
program             { drv.root = $1; }

program:
  %empty               { $$ = new SeqAST(nullptr,nullptr); }
|  top ";" program     { $$ = new SeqAST($1,$3); };

top:
%empty                 { $$ = nullptr; }
| definition           { $$ = $1; }
| external             { $$ = $1; }
| exp                  { $$ = $1; $1->toggle(); };

definition:
  "def" proto exp      { $$ = new FunctionAST($2,$3); $2->noemit(); };

external:
  "extern" proto       { $$ = $2; };

proto:
  "id" "(" idseq ")"   { $$ = new PrototypeAST($1,$3); };

idseq:
  %empty               { std::vector<std::string> args;
                         $$ = args; }
| "id" idseq           { $2.insert($2.begin(),$1); $$ = $2; };


%left "<" ">";
%left "+" "-";
%left "*" "/";

exp:
  exp "+" exp          { $$ = new BinaryExprAST('+',$1,$3); }
| exp "-" exp          { $$ = new BinaryExprAST('-',$1,$3); }
| exp "*" exp          { $$ = new BinaryExprAST('*',$1,$3); }
| exp "/" exp          { $$ = new BinaryExprAST('/',$1,$3); }
| exp "<" exp          { $$ = new BinaryExprAST('<', $1, $3); }
| exp ">" exp          { $$ = new BinaryExprAST('>', $1, $3); }
| ifexp                { $$ = $1; }
| idexp                { $$ = $1; }
| "(" exp ")"          { $$ = $2; }
| "number"             { $$ = new NumberExprAST($1); };

idexp:
  "id"                 { $$ = new VariableExprAST($1); }
| "id" "(" optexp ")"  { $$ = new CallExprAST($1,$3); };

ifexp:
  "if" exp "then" exp "else" exp "fi" { $$ = new IfExprNode($2, $4, $6); }

optexp:
%empty                 { std::vector<ExprAST*> args;
                         args.push_back(nullptr);
			 $$ = args;
                       }
| explist              { $$ = $1; };

explist:
  exp                  { std::vector<ExprAST*> args;
                         args.push_back($1);
			 $$ = args;
                       }
| exp "," explist      { $3.insert($3.begin(), $1); $$ = $3; };

%%

void yy::parser::error(const location_type& location, const std::string& message)
{
  std::cerr << location << ": " << message << '\n';
}
