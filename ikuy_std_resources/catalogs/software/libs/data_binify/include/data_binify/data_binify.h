#pragma once
#include "core/core.h"
#include "memory/memory.h"

typedef struct Binify_Context *Binify_ContextHandle ;

EXTERN_C Binify_ContextHandle Binify_Create(char const * const in, Memory_Allocator* allocator);
EXTERN_C void Binify_Destroy( Binify_ContextHandle handle );
EXTERN_C size_t Binify_BinarySize( Binify_ContextHandle handle );
EXTERN_C uint8_t const *Binify_BinaryData( Binify_ContextHandle handle );

/* Binify is a data description DSL, for writing binary files loading with minimal
 * work but still keeping a version of the data that readable and understandable by
 * humans.
 *
 * These functions parse the DSL and produce a binary to store to disk, loading
 * these data as binary and applying pointer fixups will give you an in memory workable
 * copy of the data.
 * The DSL is fairly simple but quite powerful.
 * It allows and promotes comments in the text version, to explain what the data is for.
 * It also supports constants and variables allowing to produce a very low level
 * binary but still without too many magic numbers all over the place.
 *
 * The parser is 2 pass, the first pass works out where the data goes and the 2nd
 * fills it in.
 * Variables can be set on pass 0 or 1.
 * A pass 0 allows a computed constant. Set it to zero, increment the variable and
 * then it will be constant on pass 1. A pass 1 variable is only set and changes on pass 1
 *
 * writehelper adds functionality and a programmable interface to the dsl
 * it adds enums, flags, string tables, size calculations using the base DSL
 *
 * DSL:
 * types = 	U8					- unsigned 8 bit data type (0 to 255)
 * 					U16					- unsigned 16 bit data type (0 to 65,535)
 * 					U32					- unsigned 32 bit data type (0 to 4,294,967,295)
 * 					U64					- unsigned 64 bit data type (0 to 18,446,744,073,709,551,615)
 * 					S8					- signed 8 bit data type (-128 to 128)
 * 					S16					- signed 16 bit data type (-32,768 to 32,767)
 * 					S32					- signed 32 bit data type (-2,147,483,648 to 2,147,483,647)
 * 					S64					- signed 64 bit data type (-9223,372,036,854,775,808  t 9,223,372,036,854,775,807)
 * 					float				- 32 bit floating point
 * 					double			- 64 bit floating point
 * 					string			- UTF-8 without implicit NUL (add /0 it used with C strings)

 * data output (overflow will be warned)
 * 				// - rest of line will be ignored
 *				# - rest of line will be ignored
 * 				integer expression - the result of the expression will be stored as the default type
 * 				(type) integer expression - number will be stored as the specified type
 * 				"STRING" - output the exact STRING without a final /0
 * 				*VARIABLE* integer expression - set pass 0 VARIABLE (const in pass 1) to the integer expression
 * 				VARIABLE: - set the current fill offset into VARIABLE (pass 0)
 * 				VARIABLE = integer expression - set pass 1 VARIABLE to result of the expression
 * integer expressions
 * 					integer - any valid integer number
 *  				+ - ( )   basic math
 *  				| -  bitwise or
 *  				VARIABLE - look named VARIABLE
 * float expressions
 * 					real - any valid real number
 * 					+ - / * ( )  basic math
 *
 * directives =
 * 				.allow_nan [0|1]				- true to write float/double nans otherwise 0
 * 				.allow_infinity [0|1] 	- true to write float/double infinities otherwise MAX
 * 				.set_default_type type	- if no type specify this type is used (default U32)
 *				.align amount						- write blanks till aligned at amount in file
 *				.type										- if no type is specified this type is used (default U32)
 *				.littleendian						- write data as little endian (default)
 *				.bigendian							- write data as big endian
 *				.blank count						- write count 0 into file
 *				.addresslen							- treat fixup as 32 bit or 64 bit
 *				.fixup									- insert an offset as addresslen size on disk
 */
