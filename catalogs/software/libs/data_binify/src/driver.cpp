#include "driver.hpp"
#include "parser.hpp"

driver::driver ()
	: trace_parsing (false), trace_scanning (false)
{
}

int driver::parseText (std::string const& txt_)
{
	text = txt_;
	location.initialize (&text);
	scan_begin ();
	yy::parser parser(*this);
#if YYDEBUG != 0
	parser.set_debug_level (trace_parsing);
#endif
	int res = parser.parse();
	scan_end ();

	return res;
}