/* binify */
%option noyywrap nounput noinput batch 8bit warn nodefault
/*%option debug*/

%option     outfile="scanner.cpp"
%option header-file="scanner.hpp"

%{ /* -*- C++ -*- */
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstring> // strerror
#include <string>
#include "ast.hpp"
#include "driver.hpp"
#include "parser.hpp"

#define yyterminate() return yy::parser::make_END(loc);

yy::parser::symbol_type convert_INTNUM(const std::string &s, int base, const yy::parser::location_type& loc);
yy::parser::symbol_type convert_FPNUM(const std::string &s, const yy::parser::location_type& loc);
yy::parser::symbol_type convert_STATEMENT(const std::string &s, const yy::parser::location_type& loc);

%}

DIGIT		[0-9]
HEXDIGIT 	[a-fA-F0-9]
LETTER		[a-zA-Z_]
BLANK 		[ \t]

%%
%{
  // A handy shortcut to the location held by the driver.
  yy::location& loc = drv.location;
  // Code run each time yylex is called.
  loc.step ();
%}

{DIGIT}+									return convert_INTNUM (yytext, 10, loc);
0x{HEXDIGIT}+								return convert_INTNUM (yytext, 16, loc);
[-]{DIGIT}+									return convert_INTNUM (yytext, 10, loc);
[-]0x{HEXDIGIT}+							return convert_INTNUM (yytext, 16, loc);
0b[10]+										return convert_INTNUM (yytext,  2, loc);
("-"|"+")?([0-9]*"."[0-9]+([eE]("+"|"-")[0-9]+)?)|(?i:nan)|(?i:inf)|(?i:infinity) 	return convert_FPNUM(yytext, loc);
("-"|"+")?[0-9]+[eE]("+"|"-")[0-9]+ 		return convert_FPNUM(yytext, loc);

[uU]8		                                return yy::parser::make_TYPE(binify::ast::Type::U8, loc);
[uU]16		                                return yy::parser::make_TYPE(binify::ast::Type::U16, loc);
[uU]32		                                return yy::parser::make_TYPE(binify::ast::Type::U32, loc);
[uU]64		                                return yy::parser::make_TYPE(binify::ast::Type::U64, loc);
[sS]8		                                return yy::parser::make_TYPE(binify::ast::Type::S8, loc);
[sS]16		                                return yy::parser::make_TYPE(binify::ast::Type::S16, loc);
[sS]32		                                return yy::parser::make_TYPE(binify::ast::Type::S32, loc);
[sS]64		                                return yy::parser::make_TYPE(binify::ast::Type::S64, loc);
"float"		                                return yy::parser::make_TYPE(binify::ast::Type::Float, loc);
"double"	                                return yy::parser::make_TYPE(binify::ast::Type::Double, loc);
".".[a-z]*					                return convert_STATEMENT(yytext, loc);

"//".*                      {/* eat comments */}
#.*                         {/* eat comments */}
{BLANK}+                    loc.step();
[\n]+						loc.lines(yyleng);  loc.step();
"="       return yy::parser::make_ASSIGN(loc);
"|"       return yy::parser::make_PIPE(loc);
"("       return yy::parser::make_LPAREN(loc);
")"       return yy::parser::make_RPAREN(loc);
":"       return yy::parser::make_COLON(loc);
"+"       return yy::parser::make_PLUS(loc);
"-"       return yy::parser::make_MINUS(loc);
"*"       return yy::parser::make_STAR(loc);
"/"       return yy::parser::make_SLASH(loc);
","       return yy::parser::make_COMMA(loc);
"<"       return yy::parser::make_LANGLE(loc);
">"       return yy::parser::make_RANGLE(loc);

"\"".*"\""					return yy::parser::make_STRING(yytext, loc);
{LETTER}({LETTER}|{DIGIT})*	return yy::parser::make_IDENTIFIER(yytext, loc);

.	{ throw yy::parser::syntax_error(loc, "invalid character: " + std::string(yytext)); }

%%

yy::parser::symbol_type convert_INTNUM (const std::string &s, int base, const yy::parser::location_type& loc) {
  errno = 0;
  int64_t n = std::stoull(s, NULL, base);
  return yy::parser::make_INTNUM(n, loc);
}

yy::parser::symbol_type convert_FPNUM (const std::string &s, const yy::parser::location_type& loc) {
  errno = 0;
  double n = std::stod(s, NULL);
  return yy::parser::make_FPNUM (n, loc);
}
yy::parser::symbol_type convert_STATEMENT (const std::string &s, const yy::parser::location_type& loc) {
  errno = 0;
  binify::ast::Statement statement;
  if(s == ".align") statement = binify::ast::Statement::Align;
  else if(s == ".blank") statement = binify::ast::Statement::Blank;
  else if(s == ".addresslen") statement = binify::ast::Statement::AddressLen;
  else if(s == ".littleendian") statement = binify::ast::Statement::LittleEndian;
  else if(s == ".bigendian") statement = binify::ast::Statement::BigEndian;
  else if(s == ".fixup") statement = binify::ast::Statement::Fixup;
  else if(s == ".type") statement = binify::ast::Statement::Type;
  else if(s == ".allownan") statement = binify::ast::Statement::AllowNan;
  else if(s == ".allowinfinity") statement = binify::ast::Statement::AllowInfinity;
  else {
    printf("%s\n", ("Lexer: unknown statement(" + s +")\n").c_str());
  }
  return yy::parser::make_STATEMENT (statement, loc);
}

void driver::scan_begin (){
 yy_switch_to_buffer(yy_scan_string(text.c_str()));
}

void driver::scan_end ()
{
}