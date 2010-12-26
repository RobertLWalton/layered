// Layers Language Standard Input Parser
//
// File:	ll_input_parser.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Dec 25 02:03:36 EST 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

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
typedef min::packed_struct_updptr<input_parser>
    input_parser_ptr;

static min::uns32 input_parser_stub_disp[] =
{
    min::DISP ( & PAR::pass_struct::first ),
    min::DISP ( & input_parser::scanner ),
    min::DISP_END
};

min::packed_struct<input_parser> input_parser_type
    ( "input_parser",
      NULL, ::input_parser_stub_disp );

static min::uns32 input_parser_get
	( PAR::pass_ptr out, PAR::pass_ptr in );
PAR::pass_ptr PAR::standard::create_input_pass
    ( LEX::scanner_ptr scanner )
{
    input_parser_ptr pass =
        input_parser_type.new_stub();
    pass->scanner = scanner;
    pass->get = ::input_parser_get;
}

// Input Parser
// ----- ------

static min::uns32 input_parser_get
	( PAR::pass_ptr out, PAR::pass_ptr in )
{
    min::uns32 first, last, count = 0;
    input_parser_ptr pass = (input_parser_ptr) in;
    LEX::scanner_ptr scanner = pass->scanner;
    while ( true )
    {
        min::uns32 type =
	    LEX::scan ( first, last, scanner );
	switch ( type )
	{
	}
    }
    return count;
}
