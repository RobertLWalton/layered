// Layered Languages Parser Operator Command Test
//
// File:	ll_parser_oper_command_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat May 24 09:21:17 PM EDT 2025
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

# include <ll_parser.h>
# include <ll_parser_bracketed.h>
# include <ll_parser_oper.h>
# include <ll_parser_standard.h>
# include <iostream>
# include <cassert>
# define PAR ll::parser
# define BRA ll::parser::bracketed
# define OP ll::parser::oper
# define PARSTD ll::parser::standard
using std::cout;

int main ( int argc, const char * argv[] )
{
    min::initialize();

    PAR::init ( PAR::default_parser, 0 );
    PARSTD::init_input ( PAR::default_parser );
    PARSTD::define_standard
        ( PAR::default_parser,
	    PARSTD::LABEL | PARSTD::CODE
	  | PARSTD::TEXT | PARSTD::MATH
	  | PARSTD::BLOCK | PARSTD::TOP_LEVEL
	  | PARSTD::CONCATENATOR
	  | PARSTD::ID | PARSTD::TABLE
	  | PARSTD::ALL_BRACKETS );

    PAR::init_input_stream
        ( PAR::default_parser, std::cin );
    PAR::init_printer_ostream
        ( PAR::default_parser, std::cout );
    PAR::init_line_format
        ( PAR::default_parser,
	  min::picture_line_format );
    PAR::parse();
}
