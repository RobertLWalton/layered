// Layered Languages Name String Test
//
// File:	ll_lexeme_name_string_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Dec  4 00:03:47 EST 2021
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

# include <ll_lexeme.h>
# include <ll_lexeme_standard.h>
# include <iostream>
# include <cassert>
# define LEX ll::lexeme
# define LEXSTD ll::lexeme::standard

const min::uns64 accepted_types =
      ( 1ull << LEXSTD::mark_t )
    + ( 1ull << LEXSTD::separator_t );
const min::uns64 ignored_types =
      ( 1ull << LEXSTD::horizontal_space_t )
    + ( 1ull << LEXSTD::indent_t )
    + ( 1ull << LEXSTD::start_of_file_t )
    + ( 1ull << LEXSTD::end_of_file_t )
    + ( 1ull << LEXSTD::premature_end_of_file_t );
const min::uns64 end_types =
      ( 1ull << LEXSTD::end_of_file_t )
    + ( 1ull << LEXSTD::premature_end_of_file_t );

static void test_string ( const char * s )
{
    min::printer printer =
        LEX::default_scanner->printer;
    printer
        << "TESTING \""
        << min::bom
	<< min::ascii << min::graphic_only
	<< s << "\"" << min::eom;
    LEX::init_input_string
    	( LEX::default_scanner,
	  min::new_ptr ( s ),
	  min::DISPLAY_NON_GRAPHIC );
    min::gen value = LEX::scan_name_string
    	( LEX::default_scanner,
	  ::accepted_types,
	  ::ignored_types,
	  ::end_types );
    printer << "    VALUE: "
            << min::pgen
	           ( value,
		     min::always_quote_gen_format )
            << min::eol;
}

int main ( int argc, const char * argv[] )
{
    min::initialize();
    LEX::init_ostream
	    ( LEX::default_scanner, std::cout )
        << min::ascii;
    LEXSTD::init_standard_program();
    LEX::init_program ( LEX::default_scanner,
    			LEXSTD::default_program );

    if ( argc > 1 )
        LEX::default_scanner->trace = LEX::TRACE;

    ::test_string ( "(" );
    ::test_string ( "(|" );
    ::test_string ( "<+" );
    ::test_string ( "<+ -" );
    ::test_string ( "  <+  +> " );
    ::test_string ( "  \t  " );
    ::test_string ( "<+ foo +>" );
    ::test_string ( "<+ \n +>" );
    ::test_string ( "<+ \001 +>" );
    ::test_string ( "\"<FOO>" );
}
