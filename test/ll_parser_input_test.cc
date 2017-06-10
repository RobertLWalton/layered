// Layered Languages Parser Input Test
//
// File:	ll_parser_input_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Jun 10 14:55:07 EDT 2017
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// ll_parser_input_test [-m] [-p]
//
// 	-m enables standard lexeme map
// 	-p enables standard prefixes

# include <ll_parser.h>
# include <ll_parser_standard.h>
# include <iostream>
# include <cstdlib>
# include <cassert>
# define PAR ll::parser
# define PARSTD ll::parser::standard
using std::cout;
using std::endl;

int main ( int argc, const char * argv[] )
{
    min::initialize();
    PAR::init ( PAR::default_parser, false );
    PARSTD::init_input ( PAR::default_parser );
    PARSTD::init_block ( PAR::default_parser );

    while ( argc > 1 )
    {
        if ( strcmp ( argv[1], "-m" ) == 0 )
	    PARSTD::init_lexeme_map
		( PAR::default_parser );
        else if ( strcmp ( argv[1], "-p" ) == 0 )
	    PARSTD::init_prefixes
		( PAR::default_parser );
	else
	{
	    cout << "ERROR: BAD ARGUMENT " << argv[1]
	         << endl;
	    exit ( 1 );
	}
	-- argc, ++ argv;
    }

    PAR::init_input_stream
        ( PAR::default_parser, std::cin );
    PAR::init_ostream
        ( PAR::default_parser, std::cout );
    PAR::default_parser->printer
        << min::display_picture;
    PAR::default_parser->trace_flags |=
        PAR::TRACE_PARSER_INPUT;
    PAR::init_line_display
        ( PAR::default_parser,
	  min::DISPLAY_PICTURE );
    PAR::parse();
}
