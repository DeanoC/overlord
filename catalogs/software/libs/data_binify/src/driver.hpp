#pragma once
#include "core/core.h"
#include <string>
#include <map>
#include "ast.hpp"
#include "parser.hpp"

// Give Flex the prototype of yylex we want ...
# define YY_DECL yy::parser::symbol_type yylex (driver& drv)

// ... and declare it for the parser's sake.
YY_DECL;

class driver {
public:
	driver ();
	int result;

	// Run the parser on text.  Return 0 on success.
	int parseText (std::string const& txt_);

	// The text being parses
	std::string text;
	// Whether to generate parser debug traces.
	bool trace_parsing;

	// Handling the scanner.
	void scan_begin ();
	void scan_end ();
	// Whether to generate scanner debug traces.
	bool trace_scanning;
	// The token's location used by the scanner.
	yy::location location;

	virtual void IntValueAsDefault(  yy::parser::location_type& loc, int64_t i ) = 0;
	virtual void FloatValueAsDefault(  yy::parser::location_type& loc, double f ) = 0;
	virtual void String(  yy::parser::location_type& loc, std::string str ) = 0;
	virtual void IntValue( yy::parser::location_type& loc, binify::ast::Type type, int64_t value) = 0;
	virtual void FloatValue( yy::parser::location_type& loc, binify::ast::Type type, double value) = 0;

	virtual void Statement( yy::parser::location_type& loc, binify::ast::Statement statement ) = 0;
	virtual void IntStatement( yy::parser::location_type& loc, binify::ast::Statement statement, int64_t i) = 0;
	virtual void TypeStatement( yy::parser::location_type& loc, binify::ast::Statement statement, binify::ast::Type type) = 0;

	virtual void SetSymbolToOffset(  yy::parser::location_type& loc, std::string name ) = 0;
	virtual void SetSymbol(  yy::parser::location_type& loc, std::string name, int64_t i ) = 0;
	virtual void SetPass0Symbol(  yy::parser::location_type& loc, std::string name, int64_t i ) = 0;
	virtual int64_t LookupSymbol(  yy::parser::location_type& loc, std::string name ) = 0;
};
