// Layered Languages Standard Test
//
// File:	ll_parser_standard_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon May 26 08:32:42 PM EDT 2025
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

# include <ll_parser.h>
# include <ll_parser_standard.h>
# include <mexcom.h>
# include <iostream>
# include <cassert>
# define PAR ll::parser
# define PARSTD ll::parser::standard
using std::cout;

int main ( int argc, const char * argv[] )
{
    min::initialize();
    PAR::init ( PAR::default_parser, PARSTD::ALL );

    PAR::init_input_stream
        ( PAR::default_parser, std::cin,
	  min::marked_line_format );
    PAR::init_printer_ostream
        ( PAR::default_parser, std::cout );

    // Set to allow PRIM functions to print error
    // messages.
    //
    min::init_printer
        ( PAR::input_file_ref(PAR::default_parser),
	  PAR::default_parser->printer );
    mexcom::input_file =
        PAR::default_parser->input_file;

    PAR::parse();

    min::print_id_map ( PAR::default_parser->printer );
}

