#include <string>
//#include <format>
#include "data_binify/fmt/format.hpp"
#include "./binify.hpp"
#include "dbg/print.h"
// still need these from system stl for now
#include <ostream>
#include <sstream>


// TODO determine this from compile PLATFORM
#define BINIFY_LITTLE_ENDIAN

namespace binify {
static std::string StatementToString(binify::ast::Statement statement) {
	switch(statement) {
		case ast::Statement::Align: return ".align";
		case ast::Statement::Blank: return ".blank";
		case ast::Statement::LittleEndian: return ".littleendian";
		case ast::Statement::BigEndian: return ".bigendian";
		case ast::Statement::AddressLen: return ".addresslen";
		case ast::Statement::Fixup: return ".fixup";
		case ast::Statement::Type: return ".type";
		case ast::Statement::AllowNan: return ".allownan";
		case ast::Statement::AllowInfinity: return ".allowinfinity";
		default: return "UNKNOWN";
	}
}
bool Binify::parse( std::string const& txt_, std::ostream* out_ )
{
	int result = 0;
	out = out_;
	try {
		symbolTable.clear();
		result = this->parseText(txt_);
		if (result != 0) { return false; }
		totalSize = offset;

		offset = 0;
		pass = 1;
		result = this->parseText(txt_);
		if (result != 0) { return false; }

	}
	catch (yy::parser::syntax_error& e)
	{
		// improve error messages by adding location information:
		int col = e.location.begin.column;
		int len = 1 + e.location.end.column - col;
		// TODO: The reported location is not entirely satisfying. Any
		// chances for improvement?
		log += fmt::format("{} in pass {} row {} col {}:\n", e.what(), pass, e.location.end.line, col).c_str();
		int markbegin = 0;
		int markend = 0;
		unsigned int curLen = 0;
		for (int i = 0; i < txt_.length(); ++i) {
			if (txt_[i] == '\n') {
				markend = i;
				if (curLen > (e.location.end.line - 5) && curLen < (e.location.end.line + 5)) {
					std::string result = txt_.substr(markbegin, markend - markbegin);
					if (curLen == e.location.end.line-1) {
						log += fmt::format("-----");
					}
					log += fmt::format("{} {}\n", curLen, result);
				}
				markbegin = (i + 1);
				++curLen;
			}
		}

		return false;
	}
	return true;
}

void Binify::SetSymbolToOffset( yy::parser::location_type& loc, std::string name )
{
	if( pass == 0 )
	{
		auto const it = symbolTable.find( name );
		if( it != symbolTable.end() )
			throw yy::parser::syntax_error(loc, fmt::format("WARNING: symbol '{}' not found", name));

		SetPass0Symbol(loc, name, offset);
	}
}

void Binify::SetPass0Symbol( yy::parser::location_type& loc, std::string name, int64_t i )
{
	if( pass == 0 )
	{
		SetSymbol(loc, name, i);
	}
}

void Binify::SetSymbol( yy::parser::location_type& loc, std::string name, int64_t i )
{
	if( debugMode )
		log += fmt::format("{} = {}", name.c_str(), i).c_str();

	symbolTable[name] = i;
}


int64_t Binify::LookupSymbol( yy::parser::location_type& loc, std::string name )
{
//	printf("Lookup( %s )\n", name );

	SymbolTable ::const_iterator it = symbolTable.find( name );
	if( it == symbolTable.end() )
	{
		if (pass == 0)
		{
			return 1; // ignore without error on first pass
		}

		throw yy::parser::syntax_error(loc, fmt::format("WARNING: symbol  '{}' not found", name));
		return 0;
	}

	return it->second;
}


// everything goes out through here.
void Binify::byteOut( uint8_t b ) {
	++offset;

	if( pass == 1 )
	{
		(*out) << b;
	}
}


// Dump out a multibyte value as a series of bytes.
// (value is assumed to be in native byteorder)
void Binify::valueOut( const void* value, int size )
{
	const uint8_t* p = (const uint8_t*)value;
	int i;
#ifdef BINIFY_LITTLE_ENDIAN
	if( byteOrder == ast::Statement::LittleEndian)
	{
		for( i=0; i<size; ++i )		// native order
			byteOut( p[i] );
	}
	else	// BigEndian
	{
		for( i=size-1; i>=0; --i )
			byteOut( p[i] );
	}
#else	// BINIFY_BIG_ENDIAN
	if( byteOrder == ast::Statement::LittleEndian )
	{
		for( i=size-1; i>=0; --i )
			byteOut( p[i] );
	}
	else	// BigEndian
	{
		for( i=0; i<size; ++i )	// native order
			byteOut( p[i] );
	}
#endif
}

void Binify::SetDefaultType( ast::Type type )
{
	defaultType = type;
}

void Binify::Statement( yy::parser::location_type& loc, binify::ast::Statement statement ) {
	switch(statement) {
		case binify::ast::Statement::LittleEndian:
		case binify::ast::Statement::BigEndian: byteOrder = statement; break;
		default:
			throw yy::parser::syntax_error(loc, fmt::format("Statement ({}) not the correct type", StatementToString(statement)));
	}
}
void Binify::IntStatement( yy::parser::location_type& loc, binify::ast::Statement statement, int64_t i) {
	switch(statement) {
		case binify::ast::Statement::Align: Align(i); break;
		case binify::ast::Statement::Blank: Blank(i); break;
		case binify::ast::Statement::AddressLen: SetAddressLen(loc, i); break;
		case binify::ast::Statement::Fixup: Fixup(loc, i); break;
		case binify::ast::Statement::AllowNan: AllowNan(i); break;
		case binify::ast::Statement::AllowInfinity: AllowInfinity(i); break;
		default:
			throw yy::parser::syntax_error(loc, fmt::format("IntStatement ({}) not the correct type", StatementToString(statement)));
	}
}

void Binify::TypeStatement( yy::parser::location_type& loc, binify::ast::Statement statement, ast::Type type) {
	if(statement != binify::ast::Statement::Type)
		throw yy::parser::syntax_error(loc, fmt::format("TypeStatement ({}) not the correct type", StatementToString(statement)));
	SetDefaultType(type);
}

void Binify::AllowNan( int64_t yesno )
{
	allowNan = (yesno != 0);
}
void Binify::AllowInfinity( int64_t yesno )
{
	allowInfinity = (yesno != 0);
}

// pad with zeros to get to the specified boundary
void Binify::Align( int64_t boundary )
{
	int64_t pad = boundary - (offset % boundary);

	if( pad==boundary ) pad = 0;

	while( pad-- ) byteOut(0);
}


// emit a run of zeros
void Binify::Blank( int64_t count )
{
	while( count-- ) byteOut(0);
}


void Binify::String(  yy::parser::location_type& loc, std::string sstr )
{
	const char* p;
	size_t i;

	char const* str = sstr.c_str();

	size_t len = strlen(str);
	if(!( len >= 2 )) throw yy::parser::syntax_error(loc, "String is not NULL terminated");
	if(!( str[0] == '\"' )) throw yy::parser::syntax_error(loc, "Strings must start with \"");
	if(!( str[len-1] == '\"' )) throw yy::parser::syntax_error(loc, "Strings must end with \"");

	p = str+1;
	i=1;
	while( i<len-1 )
	{
		char c = *p++;
		++i;
		if( c == '\\' )
		{
			if(!( i<len-1 )) throw yy::parser::syntax_error(loc, "Strings found \\ as last character");

			++i;
			switch( *p++ )
			{
				case '0': c='\0'; break;
				case 'n': c='\n'; break;
				case 't': c='\t'; break;
				case 'r': c='\r'; break;
				case '\"': c='\"'; break;
				case '\\': c='\\'; break;
			}
		}
		byteOut( c );
	}
}

template<typename T> T SafeConvert(std::string& log, uint64_t i)
{
	bool rangeErr = (std::is_signed<T>()) ?
			   (i < std::numeric_limits<T>::max() || i > std::numeric_limits<T>::max()) :
			   (i > std::numeric_limits<T>::max());

	if(rangeErr)
	{
		log += fmt::format("WARNING: value out of range for '{}'", typeid(T).name()).c_str();
	}

	return (T)i;
}

void Binify::IntValue(  yy::parser::location_type& loc, binify::ast::Type type, int64_t value) {
	switch(type) {
		case binify::ast::Type::U8: U8((uint8_t)value); break;
		case binify::ast::Type::U16: U16((uint16_t)value); break;
		case binify::ast::Type::U32: U32((uint32_t)value); break;
		case binify::ast::Type::U64: U64((uint64_t)value); break; // TODO bug here
		case binify::ast::Type::S8: S8((int8_t)value); break;
		case binify::ast::Type::S16: S16((int16_t)value); break;
		case binify::ast::Type::S32: S32((int32_t)value); break;
		case binify::ast::Type::S64: S64((int64_t)value); break;
		default:
			break;
	}
}
void Binify::FloatValue(  yy::parser::location_type& loc, binify::ast::Type type, double value) {
	switch(type) {
		case binify::ast::Type::Float: Float(value); break;
		case binify::ast::Type::Double: Double(value); break;
		default:
			break;
	}
}

void Binify::U8( uint64_t i )
{
	auto j = SafeConvert<uint8_t>(log, i & 0xff);
	byteOut( j );
}

void Binify::U16( uint64_t i )
{
	auto j = SafeConvert<uint16_t>(log, i & 0xffff);
	valueOut( &j, sizeof(j) );
}

void Binify::U32( uint64_t i )
{
	auto j = SafeConvert<uint32_t>(log, i & 0xffffffffll);
	valueOut( &j, sizeof(j) );
}

void Binify::U64( uint64_t i )
{
	auto j = SafeConvert<uint64_t>(log, i);
	valueOut( &j, sizeof(j) );
}

void Binify::S8( int64_t i )
{
	auto j = SafeConvert<int8_t>(log, i);
	byteOut( j );
}

void Binify::S16( int64_t i )
{
	auto j = SafeConvert<uint16_t>(log, i);
	valueOut( &j, sizeof(j) );
}

void Binify::S32( int64_t i )
{
	auto j = SafeConvert<uint32_t>(log, i);
	valueOut( &j, sizeof(j) );
}

void Binify::S64( int64_t i )
{
	auto j = SafeConvert<uint64_t>(log, i);
	valueOut( &j, sizeof(j) );
}

void Binify::Float( double d )
{
	if (!allowNan && std::isnan(d))
	{
		d = 0.0;
	}

	if (!allowInfinity && !std::isfinite(d))
	{
		if (d < 0)
		{
			d = (-std::numeric_limits<float>::max()) + 1;
		}
		else
		{
			d = std::numeric_limits<float>::max() - 1;
		}
	}

	bool rangeErr = (d < -std::numeric_limits<float>::max() || d > std::numeric_limits<float>::max());

	if(rangeErr)
	{
		log += "WARNING: value out of range for float\n";
	}

	auto j = (float)d;
	valueOut( &j, sizeof(j) );
}

void Binify::Double( double d )
{
	if (!allowNan && std::isnan(d))
	{
		d = 0.0;
	}
	if (!allowInfinity && !std::isfinite(d))
	{
		if (d < 0)
		{
			d = (-std::numeric_limits<double>::max()) + 1;
		}
		else
		{
			d = std::numeric_limits<double>::max() - 1;
		}
	}

	valueOut( &d, sizeof(double) );
}

void Binify::IntValueAsDefault(  yy::parser::location_type& loc, int64_t i )
{
	switch( defaultType )
	{
		case ast::Type::U8: 	U8( i ); break;
		case ast::Type::U16: 	U16( i ); break;
		case ast::Type::U32: 	U32( i ); break;
		case ast::Type::S8: 	S8( i ); break;
		case ast::Type::S16: 	S16( i ); break;
		case ast::Type::S32: 	S32( i ); break;
		case ast::Type::S64: 	S64( i ); break;
		case ast::Type::U64: 	{
			// this should fix bug with unsigned 64 bit constants
			union
			{
				int64_t i64;
				uint64_t ui64;
			} u;
			u.i64 = i;
			U64( u.ui64 );
			break;
		}
		default:
			throw yy::parser::syntax_error(loc, "IntDefault called from non integer type!");
	}
}

void Binify::FloatValueAsDefault(  yy::parser::location_type& loc, double f )
{
	// floats always output as floats, despite the default type.
	// only exception is if defaulttype is double.
	switch( defaultType )
	{
		case ast::Type::Double: Double( f ); break;
		default: Float( (float)f ); break;
	}
}

void Binify::SetAddressLen( yy::parser::location_type& loc, int64_t bits )
{
	if( (bits / 8) * 8 != bits)throw yy::parser::syntax_error(loc, "Address Length must be divisible by 8");
	if( bits < 0) throw yy::parser::syntax_error(loc, "Address Length must be positive");

	addressLen = bits;
}

void Binify::Fixup( yy::parser::location_type& loc, uint64_t i)
{
	Align( addressLen / 8 );

	if(pass == 1 && i >= totalSize)
		throw yy::parser::syntax_error(loc, fmt::format("Fixup offset {} is beyond end of the data ({})", i, totalSize));
	if(pass == 1) log += fmt::format("Fixup @ offset {:#x} value {}\n", offset, i);

	if (addressLen == 32) {
		U32(i);
	}
	else {
		U64(i);
	}
}


} // end namespace

// C api for binify
#include "data_binify/data_binify.h"
#include "memory/memory.h"
#include "cadt/vector.h"

struct Binify_Context
{
	CADT_VectorHandle data;
	Memory_Allocator * allocator;
};

EXTERN_C Binify_ContextHandle Binify_Create(char const * const in, Memory_Allocator* allocator) {
	std::string tmp;
	std::ostringstream dir;
	bool okay;
	binify::Binify *binny = nullptr;
	MEMORY_STACK_ALLOCATOR(_, sizeof(binify::Binify));

	auto* ctx = (Binify_Context*)MCALLOC(allocator, 1, sizeof(Binify_Context));
	if(ctx == nullptr) goto failexit;
	ctx->allocator = allocator;

	binny = (binify::Binify*) MCALLOC(_, 1, sizeof(binify::Binify));
	assert(binny);
	new(binny) binify::Binify();

	okay = binny->parse( in, &dir );
	if(!okay) {
		debug_printf("Binny error log:\n%s\n", binny->getLog().c_str());
		goto failexit;
	} else {
		if(!binny->getLog().empty())
			debug_printf("Binny log:\n%s\n", binny->getLog().c_str());
	}
	tmp = dir.str();
	ctx->data = CADT_VectorCreate(sizeof(uint8_t), ctx->allocator);
	CADT_VectorResize(ctx->data, tmp.size());
	memcpy(CADT_VectorData(ctx->data), tmp.data(), tmp.size());

	binny->~Binify();
	MFREE(_, binny);
	return ctx;

failexit:
	if(binny) {
		binny->~Binify();
	}
	MFREE(allocator, ctx);
	return nullptr;
}

EXTERN_C void Binify_Destroy( Binify_ContextHandle handle) {
	if(handle) {
		if(handle->data) CADT_VectorDestroy(handle->data);
		MFREE(handle->allocator, handle);
	}
}

EXTERN_C size_t Binify_BinarySize( Binify_ContextHandle handle ) {
	assert(handle != nullptr);
	return CADT_VectorSize(handle->data);

}
EXTERN_C uint8_t const *Binify_BinaryData( Binify_ContextHandle handle ) {
	assert(handle != nullptr);
	return (uint8_t const *)CADT_VectorData(handle->data);
}