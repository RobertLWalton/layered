// Layered Languages Parser Input Test
//
// File:	ll_parser_input_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat May 29 17:09:54 EDT 2021
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// ll_parser_input_test [-m]
//
// 	-m enables standard lexeme map

# include <ll_parser.h>
# include <ll_parser_standard.h>
# include <iostream>
# include <cstdlib>
# include <cassert>
# define PAR ll::parser
# define TAB ll::parser::table
# define PARSTD ll::parser::standard
using std::cout;
using std::endl;

int main ( int argc, const char * argv[] )
{
    min::initialize();
    PAR::init ( PAR::default_parser, false );
    PARSTD::init_input ( PAR::default_parser );
    PARSTD::define_standard
        ( PAR::default_parser, PARSTD::BLOCK );

    TAB::flags components =
        PARSTD::BLOCK + PARSTD::TOP_LEVEL;
    while ( argc > 1 )
    {
        if ( strcmp ( argv[1], "-m" ) == 0 )
	    components += PARSTD::ID + PARSTD::TABLE;
	else
	{
	    cout << "ERROR: BAD ARGUMENT " << argv[1]
	         << endl;
	    exit ( 1 );
	}
	-- argc, ++ argv;
    }

    PARSTD::define_standard
	( PAR::default_parser, components );

    PAR::init_input_stream
        ( PAR::default_parser, std::cin );
    PAR::init_ostream
        ( PAR::default_parser, std::cout );
    PAR::default_parser->printer
        << min::display_picture
	<< min::set_quoted_display_control
		( min::graphic_only_display_control );
    PAR::default_parser->trace_flags |=
        PAR::TRACE_PARSER_INPUT;
    PAR::init_line_display
        ( PAR::default_parser,
	  min::DISPLAY_PICTURE );
    PAR::parse();
}
