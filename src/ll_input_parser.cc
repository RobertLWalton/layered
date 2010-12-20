// Layers Language Standard Input Parser
//
// File:	ll_input_parser.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Dec 20 13:57:13 EST 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Lines beginning with `    /// ' are the same as in
// the Standard Lexical Program appendix of ../doc/
// layered-introduction.tex.  As a consequence, this
// file uses 80 columns instead of the system default 56
// columns.

// Table of Contents
//
//	Usage and Setup
//	Input Parser Data
//	Input Parser

// Usage and Setup
// ----- --- -----

# include <ll_lexeme_standard.h>
# include <ll_parser_pass.h>
# include <ll_parser_standard.h>
# define LEX ll::lexeme
# define PAR ll::parser

// Input Parser Data
// ----- ------ ----

struct input_parser : public PAR::pass_struct
{
    LEX::scanner_ptr scanner;
};

static min::uns32 input_parser_stub_disp[] =
{
    min::DISP ( & PAR::pass_struct::first ),
    min::DISP ( & input_parser::scanner ),
    min::DISP_END
};

// min::packed_struct<input_parser> iptype
    // ( "input_parser",
      // NULL, ::input_parser_stub_disp );

// Input Parser
// ----- ------

PAR::pass_ptr PAR::standard::create_input_pass
    ( LEX::scanner_ptr scanner )
{
}
