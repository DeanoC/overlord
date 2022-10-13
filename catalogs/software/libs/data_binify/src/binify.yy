/* binify */
%skeleton "lalr1.cc"
%require "3.5.1"

%output  "parser.cpp"
%defines "parser.hpp"

%define api.token.raw
%define api.token.constructor
%define api.value.type variant
%locations
%code requires {

#include "ast.hpp"
#include <string>
class driver;
}

// The parsing context.
%param { driver& drv }

%define parse.trace false
%define parse.error verbose
%define parse.lac full
%define parse.assert true

%code {
#include "ast.hpp"
#include "driver.hpp"
}

%define api.token.prefix {T_}
%token
  ASSIGN  "="
  MINUS   "-"
  PLUS    "+"
  STAR    "*"
  SLASH   "/"
  PIPE    "|"
  LPAREN  "("
  RPAREN  ")"
  COMMA   ","
  COLON   ":"
  LANGLE  "<"
  RANGLE  ">"
;
%token END     		    0   		        "end of file"
%token <int64_t> 	    INTNUM 		        "int64"
%token <double>		    FPNUM		        "double"
%token <std::string> 	STRING 		        "string"
%token <std::string> 	IDENTIFIER 	        "identifier"
%token <binify::ast::Statement> STATEMENT
%token <binify::ast::Type> TYPE

%left MINUS PLUS
%left STAR SLASH
%left BAR
%left LPAREN RPAREN PIPE LANGLE RANGLE
%left ASSIGN
%left COMMA COLON

%type <int64_t> intexp
%type <double> fpexp

%%
%start input;

input:	%empty
		| input run
;


run:	item
		| run COMMA item
;

item:	LANGLE IDENTIFIER RANGLE ASSIGN intexp      { drv.SetPass0Symbol( @1, $2, $5 ); }
		| IDENTIFIER COLON	                        { drv.SetSymbolToOffset( @1, $1 ); }
		| IDENTIFIER ASSIGN intexp                  { drv.SetSymbol( @1, $1, $3 ); }
		| STATEMENT intexp                          { drv.IntStatement(@1, $1, $2); }
		| STATEMENT TYPE                            { drv.TypeStatement(@1, $1, $2); }
		| LPAREN TYPE RPAREN intexp	                { drv.IntValue( @1, $2, $4 ); }
        | LPAREN TYPE RPAREN fpexp	                { drv.FloatValue( @1, $2, $4 ); }
		| intexp		                            { drv.IntValueAsDefault( @1, $1 ); }
		| fpexp			                            { drv.FloatValueAsDefault( @1, $1 ); }
		| STRING		                            { drv.String( @1, $1 ); }
;

intexp: INTNUM				                { $$= $1; }
		| intexp MINUS intexp		        { $$= $1 - $3; }
		| intexp PLUS intexp		        { $$= $1 + $3; }
		| LPAREN intexp RPAREN		        { $$= $2; }
		| intexp PIPE intexp		        { $$= $1 | $3; }
		| IDENTIFIER			            { $$ = drv.LookupSymbol( @1, $1 ); }
;

fpexp: 	FPNUM			                    { $$ = $1; }
	    | fpexp PLUS fpexp	                { $$ = $1 + $3; }
        | fpexp MINUS fpexp	                { $$ = $1 - $3; }
	    | LPAREN fpexp RPAREN   		    { $$ = $2; }
	    | fpexp STAR fpexp	                { $$ = $1 * $3; }
	    | fpexp SLASH fpexp	                { $$ = $1 / $3; }

;

%%

void yy::parser::error(const parser::location_type& l, const std::string& m)
{
    throw yy::parser::syntax_error(l, m);
}