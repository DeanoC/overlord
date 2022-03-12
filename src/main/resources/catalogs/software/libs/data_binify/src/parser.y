/* binify */
/* C++ parser interface */
%output  "parser.cpp"
%defines "parser.hpp"

%skeleton "lalr1.cc"
%require "3.0"

/* add parser members (scanner, cb) and yylex parameters (loc, scanner) */
%parse-param  {yy::scanner* scanner} {binify::ParserOutput* cb}
%locations

/* increase usefulness of error messages */
%define parse.error verbose

/* assert correct cleanup of semantic value objects */
%define parse.assert

%define api.value.type variant
%define api.token.prefix {T_}

%token END     			0   		"end of file"
%token <int64_t> 		INTNUM 		"int64"
%token <double>			FPNUM		"double"
%token <std::string> 	STRING 		"string"
%token <std::string> 	IDENTIFIER 	"identifier"

%token <binify::ast::Statement> Align Blank LittleEndian BigEndian AddressLen
                                Fixup Type AllowNan AllowInfinity
%token <binify::ast::Type> U8 U16 U32 U64 S8 S16 S32 S64 Float Double

%left '-' '+'
%left '*' '/'
%left '|'
%left '(' ')'
%left NEG
%left '='

%type <int64_t> intexp
%type <double> fpexp
%type <binify::ast::Type> type

%code requires {
    #include <string>

    #include "ast.hpp"
    #include "location.hh"

    namespace yy {
        class scanner;
    };

    namespace binify {
    // results
    struct ParserOutput {
    	virtual void IntDefault( int64_t i ) = 0;
		virtual void FloatDefault( double f ) = 0;
		virtual void String( std::string str ) = 0;
		virtual void Float( double d ) = 0;
		virtual void Double( double d ) = 0;
		virtual void U8( uint64_t i ) = 0;
		virtual void U16( uint64_t i ) = 0;
		virtual void U32( uint64_t i ) = 0;
		virtual void U64( uint64_t i ) = 0;
		virtual void S8( int64_t i ) = 0;
		virtual void S16( int64_t i ) = 0;
		virtual void S32( int64_t i ) = 0;
		virtual void S64( int64_t i ) = 0;
		virtual void SetDefaultType( binify::ast::Type type ) = 0;
		virtual void SetByteOrder( binify::ast::Statement order ) = 0;
		virtual void AllowNan( int64_t yesno ) = 0;
		virtual void AllowInfinity( int64_t yesno ) = 0;
		virtual void Align( int64_t boundary ) = 0;
		virtual void Blank( int64_t count ) = 0;
		virtual void SetAddressLen( int64_t bits ) = 0;
		virtual void Fixup(uint64_t i) = 0;
		virtual void SetSymbolToOffset( std::string name ) = 0;
		virtual void SetSymbol( std::string name, int64_t i ) = 0;
		virtual void SetPass0Symbol( std::string name, int64_t i ) = 0;
		virtual int64_t LookupSymbol( std::string name ) = 0;
    };
	} // end namespace 

}

/* inserted near top of source file */
%code {
    #include "scanner.h"

    #undef yylex
    #define yylex scanner->lex
}

%%
%start input;

input:	%empty
		| input line
;

line:	'\n'					
		| run '\n'			
	;


run:	item 
		| run ',' item
;


type:	U8			{ $$=binify::ast::Type::U8; }
		| U16		{ $$=binify::ast::Type::U16; }
		| U32		{ $$=binify::ast::Type::U32; }
		| U64		{ $$=binify::ast::Type::U64; }
		| S8		{ $$=binify::ast::Type::S8; }
		| S16		{ $$=binify::ast::Type::S16; }
		| S32		{ $$=binify::ast::Type::S32; }
		| S64		{ $$=binify::ast::Type::S64; }
		| Float		{ $$=binify::ast::Type::Float; }
		| Double	{ $$=binify::ast::Type::Double; }
;

item:	'(' U8 ')' intexp		{ cb->U8( $4 ); }
		| '(' U16 ')' intexp	{ cb->U16( $4 ); }
		| '(' U32 ')' intexp	{ cb->U32( $4 ); }
		| '(' U64 ')' intexp	{ cb->U64( $4 ); }
		| '(' S8 ')' intexp		{ cb->S8( $4 ); }
		| '(' S16 ')' intexp	{ cb->S16( $4 ); }
		| '(' S32 ')' intexp	{ cb->S32( $4 ); }
		| '(' S64 ')' intexp	{ cb->S64( $4 ); }
		| '(' Float ')' fpexp	{ cb->Float( $4 ); }
		| '(' Double ')' fpexp	{ cb->Double( $4 ); }		
		| intexp		{ cb->IntDefault( $1 ); }
		| fpexp			{ cb->FloatDefault( $1 ); }
		| STRING		{ cb->String( $1 ); }
		| Align intexp		{ cb->Align( $2 ); }
		| Blank intexp		{ cb->Blank( $2 ); }
		| AllowNan intexp	{ cb->AllowNan( $2 ); }
		| AllowInfinity intexp	{ cb->AllowInfinity( $2 ); }
		| AddressLen intexp	{ cb->SetAddressLen( $2 ); }
		| Fixup intexp          { cb->Fixup( $2 ); }
		| Type type		{ cb->SetDefaultType( $2 ); }
		| LittleEndian		{ cb->SetByteOrder( binify::ast::Statement::LittleEndian ); }
		| BigEndian		{ cb->SetByteOrder( binify::ast::Statement::BigEndian ); }
	        | '*' IDENTIFIER '*' intexp { cb->SetPass0Symbol( $2, $4 ); }
		| IDENTIFIER ':'	{ cb->SetSymbolToOffset( $1 ); }
		| IDENTIFIER '=' intexp { cb->SetSymbol( $1, $3 ); }

;

intexp: INTNUM					{ $$=$1; }
		| intexp '-' intexp		{ $$=$1-$3; }
		| intexp '+' intexp		{ $$=$1+$3; }
		| '(' intexp ')'		{ $$=$2; }
		| intexp '|' intexp		{ $$=$1|$3; }
		| IDENTIFIER			{ $$ = cb->LookupSymbol( $1 ); }
;

fpexp: FPNUM			{ $$=$1; }
	| fpexp '+' fpexp	{ $$=$1+$3; }
	| fpexp '-' fpexp	{ $$=$1-$3; }
	| fpexp '*' fpexp	{ $$=$1*$3; }
	| fpexp '/' fpexp	{ $$=$1/$3; }
	| '-' fpexp %prec NEG	{ $$=-$2; }
	| '(' fpexp ')'		{ $$=$2; }

;



%%

void yy::parser::error(const parser::location_type& l, const std::string& m)
{
    throw yy::parser::syntax_error(l, m);
}