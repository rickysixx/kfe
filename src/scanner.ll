%{ /* -*- C++ -*- */
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <string>
#include <cmath>
#include "driver.hh"
#include "parser.hh"

// Work around an incompatibility in flex (at least versions
// 2.5.31 through 2.5.33): it generates code that does
// not conform to C89.  See Debian bug 333231
// <http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=333231>.
# undef yywrap
# define yywrap() 1

yy::parser::symbol_type check_keywords(std::string lexeme, yy::location& loc);
%}

%option noyywrap nounput batch debug noinput

id      [a-zA-Z][a-zA-Z_0-9]*
fpnum   [0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?
fixnum  (0|[1-9][0-9]*)\.?[0-9]*
num     {fpnum}|{fixnum}
blank   [ \t]

%{
  // Code run each time a pattern is matched.
  #define YY_USER_ACTION loc.columns(yyleng);
%}
%%
%{
  // A handy shortcut to the location held by the driver.
  yy::location& loc = drv.location;
  // Code run each time yylex is called.
  loc.step();
%}
{blank}+   loc.step(); // ignore blank characters
[\n]+      loc.lines(yyleng); loc.step(); // ignore newlines

"-"    return yy::parser::make_MINUS(loc);
"+"    return yy::parser::make_PLUS(loc);
"*"    return yy::parser::make_STAR(loc);
"/"    return yy::parser::make_SLASH(loc);
"="    return yy::parser::make_ASSIGN(loc);
"=="   return yy::parser::make_EQ(loc);
"!="   return yy::parser::make_NE(loc);
"<"    return yy::parser::make_LT(loc);
">"    return yy::parser::make_GT(loc);
"<="   return yy::parser::make_LE(loc);
">="   return yy::parser::make_GE(loc);
"("    return yy::parser::make_LPAREN(loc);
")"    return yy::parser::make_RPAREN(loc);
"["    return yy::parser::make_LSQUARE(loc);
"]"    return yy::parser::make_RSQUARE(loc);
";"    return yy::parser::make_SEMICOLON(loc);
","    return yy::parser::make_COMMA(loc);
":"    return yy::parser::make_COLON(loc);

{num} {
    errno = 0;
    double n = strtod(yytext, NULL);
    if (! (n!=HUGE_VAL && n!=-HUGE_VAL && errno != ERANGE))
        throw yy::parser::syntax_error (loc, "Float value is out of range: " + std::string(yytext));
    return yy::parser::make_NUMBER (n, loc);
}

{id}       return check_keywords(yytext, loc);
.          { throw yy::parser::syntax_error(loc, "invalid character: " + std::string(yytext)); }
<<EOF>>    return yy::parser::make_EOF(loc);
%%

yy::parser::symbol_type check_keywords(std::string lexeme, yy::location& loc)
{
    if (lexeme == "def") 
    {
        return yy::parser::make_DEF(loc);
    } 
    else if (lexeme == "extern") 
    {
        return yy::parser::make_EXTERN(loc);
    }
    else if (lexeme == "if")
    {
        return yy::parser::make_IF(loc);
    }
    else if (lexeme == "then")
    {
        return yy::parser::make_THEN(loc);
    }
    else if (lexeme == "else")
    {
        return yy::parser::make_ELSE(loc);
    }
    else if (lexeme == "for")
    {
        return yy::parser::make_FOR(loc);
    }
    else if (lexeme == "in")
    {
        return yy::parser::make_IN(loc);
    }
    else if (lexeme == "end")
    {
        return yy::parser::make_END(loc);
    }
    else if (lexeme == "var")
    {
        return yy::parser::make_VAR(loc);
    }
    else if (lexeme == "while")
    {
        return yy::parser::make_WHILE(loc);
    }
    else
    {
        return yy::parser::make_IDENTIFIER(yytext, loc);
    }
}

void driver::scan_begin()
{
    yy_flex_debug = trace_scanning;

    if (file.empty() || file == "-")
    {
        yyin = stdin;
    }
    else if (!(yyin = fopen(file.c_str(), "r")))
    {
        std::cerr << "cannot open " << file << ": " << strerror(errno) << '\n';
        exit (EXIT_FAILURE);
    }
}

void driver::scan_end()
{
    fclose(yyin);
}
