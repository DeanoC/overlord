#pragma once

#include "core/core.h"
#include <unordered_map>
#include <string>
#include "driver.hpp"

namespace binify
{

class Binify : public driver
{
public:
	bool parse(std::string const& txt, std::ostream* out_);

	std::string const& getLog() const { return log; }

	// ParserOutput implementation
	void IntValueAsDefault(  yy::parser::location_type& loc, int64_t i ) override;
	void FloatValueAsDefault(  yy::parser::location_type& loc, double f ) override;
	void String(  yy::parser::location_type& loc, std::string str ) override;

	void IntValue(  yy::parser::location_type& loc, ast::Type type, int64_t value) override;
	void FloatValue(  yy::parser::location_type& loc, ast::Type type, double value) override;

	void Statement( yy::parser::location_type& loc, binify::ast::Statement statement ) override;
	void IntStatement( yy::parser::location_type& loc, binify::ast::Statement statement, int64_t i) override;
	void TypeStatement( yy::parser::location_type& loc, binify::ast::Statement statement, ast::Type type) override;

	void SetSymbolToOffset(  yy::parser::location_type& loc, std::string name ) override;
	void SetPass0Symbol(  yy::parser::location_type& loc, std::string name, int64_t i ) override;
	void SetSymbol(  yy::parser::location_type& loc,std::string name, int64_t i ) override;

	int64_t LookupSymbol(  yy::parser::location_type& loc, std::string name) override;

private:
	void SetDefaultType( ast::Type type );
	void AllowNan( int64_t yesno );
	void AllowInfinity( int64_t yesno );
	void Align( int64_t boundary );
	void Blank( int64_t count );
	void SetAddressLen( yy::parser::location_type& loc, int64_t bits );
	void Fixup(yy::parser::location_type& loc,  uint64_t i);
	void Float( double d );
	void Double( double d );
	void U8( uint64_t i );
	void U16( uint64_t i );
	void U32( uint64_t i );
	void U64( uint64_t i );
	void S8( int64_t i );
	void S16( int64_t i );
	void S32( int64_t i );
	void S64( int64_t i );

	std::ostream* out;

	void byteOut( uint8_t b );
	void valueOut( const void* value, int size );

	using SymbolTable = std::unordered_map< std::string, int64_t >;

	int64_t offset = 0;
	int pass = 0;

	int64_t totalSize = 0;

	SymbolTable symbolTable;

	// defaults
	ast::Type defaultType = ast::Type::U32;
	ast::Statement byteOrder = ast::Statement::LittleEndian;
	bool allowNan = true;
	bool allowInfinity = true;
	int64_t addressLen = 64;

	bool debugMode = false;
	std::string log;
};
}
