// Layers Language Parser Input Test
//
// File:	ll_parser_input_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun Jan 23 01:05:12 EST 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	External Runtime
//	Program Construction Test

// Usage and Setup
// ----- --- -----

# include <ll_parser.h>
# include <iostream>
# include <cassert>
# define PAR ll::parser
using std::cout;

int main ( int argc )
{
    PAR::init_parser();
    PAR::default_parser->trace = PAR::TRACE_INPUT;
    PAR::default_parser->print->err = & std::cout;
    PAR::parse();
}
