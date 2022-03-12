/* binify */

%option     outfile="scanner.cpp"
%option header-file="scanner.hpp"

%option c++
%option 8bit warn nodefault
%option noyywrap nounput batch debug noinput

%{
    #define YY_NO_UNISTD_H
    #include <string>
    #include "ast.hpp"
    #include "parser.hpp"
    #include "scanner.h"

    // utility macros to simplify the actions
    #define YIELD_TOKEN(tok, val, type)                 \
                    yylval->build<type>(val);           \
                    return yy::parser::token::T_##tok;

    #define YY_TXT                  std::string(yytext, yyleng)
    #define YY_FNUM					std::stod(std::string(yytext, yyleng).c_str())
    #define YY_INUM(base)           std::stoull(std::string(yytext, yyleng).c_str(), 0, base)

    #define INT_TOKEN(base)     	YIELD_TOKEN(INTNUM, YY_INUM(base), int64_t)
    #define FLT_TOKEN()          	YIELD_TOKEN(FPNUM, YY_FNUM, double)
    #define TYPE_TOKEN(val)         YIELD_TOKEN(val, binify::ast::Type:: val, binify::ast::Type)
    #define STATEMENT_TOKEN(val)    YIELD_TOKEN(val, binify::ast::Statement:: val, binify::ast::Statement)
    #define STR_TOKEN(tok)          YIELD_TOKEN(tok, YY_TXT, std::string)

    // before executing an action, set the length of the location from
    // the length of the matched pattern:
    #define YY_USER_ACTION          yylloc->columns(yyleng);
%}
DIGIT		[0-9]
HEXDIGIT 	[a-fA-F0-9]
LETTER		[a-zA-Z_]
BLANK 		[ \t]

%%
%{
	// Code run each time yylex is called.
    yylloc->step();
%}


{DIGIT}+									INT_TOKEN(10)
0x{HEXDIGIT}+								INT_TOKEN(16)
[-]{DIGIT}+									INT_TOKEN(10)
[-]0x{HEXDIGIT}+							INT_TOKEN(16)
0b[10]+										INT_TOKEN(2)
("-"|"+")?([0-9]*"."[0-9]+([eE]("+"|"-")[0-9]+)?)|(?i:nan)|(?i:inf)|(?i:infinity) 	FLT_TOKEN()
("-"|"+")?[0-9]+[eE]("+"|"-")[0-9]+ 		FLT_TOKEN()

[uU]8		TYPE_TOKEN(U8)
[uU]16		TYPE_TOKEN(U16)
[uU]32		TYPE_TOKEN(U32)
[uU]64		TYPE_TOKEN(U64)
[sS]8		TYPE_TOKEN(S8)
[sS]16		TYPE_TOKEN(S16)
[sS]32		TYPE_TOKEN(S32)
[sS]64		TYPE_TOKEN(S64)
"float"		TYPE_TOKEN(Float)
"double"	TYPE_TOKEN(Double)

".align"		STATEMENT_TOKEN(Align)
".type"			STATEMENT_TOKEN(Type)
".littleendian"	STATEMENT_TOKEN(LittleEndian)
".bigendian"	STATEMENT_TOKEN(BigEndian)
".blank"		STATEMENT_TOKEN(Blank)
".addresslen"   STATEMENT_TOKEN(AddressLen)
".fixup"        STATEMENT_TOKEN(Fixup)
".allownan"     STATEMENT_TOKEN(AllowNan)
".allowinfinity" STATEMENT_TOKEN(AllowInfinity)

"//".*                      {/* eat comments */}
#.*                         {/* eat comments */}
{BLANK}+                    yylloc->step();
[\n]+						{ yylloc->lines(yyleng);  yylloc->step(); return '\n'; }
[=|():+\*,-]				{ yylloc->step(); return yytext[0]; }


"\"".*"\""					STR_TOKEN(STRING)
{LETTER}({LETTER}|{DIGIT})*	STR_TOKEN(IDENTIFIER)

.	{ throw yy::parser::syntax_error(*yylloc, "invalid character: " + std::string(yytext)); }

%%

yy::scanner::scanner(std::istream* in, std::ostream* out)
    : yyFlexLexer(in, out)
{
}

// Flex generates the code for `yy::scanner::lex` (see YY_DECL).

// This must be defined manually to prevent linker errors:
int yyFlexLexer::yylex()
{
    throw std::logic_error(
        "The yylex() exists for technical reasons and must not be used.");
}