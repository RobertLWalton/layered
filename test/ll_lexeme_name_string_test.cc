// Layered Languages Name String Test
//
// File:	ll_lexeme_name_string_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun Aug 27 00:43:02 EDT 2023
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

static min::line_format marked_line_format =
    * min::marked_line_format;
    // Gets DISPLAY_NON_GRAPHIC below in main.

static void test_string ( const char * s )
{
    min::printer printer =
        LEX::default_scanner->printer;
    printer
        << min::bom
        << "TESTING \""
	<< min::place_indent ( 0 ) 
	<< min::ascii << min::graphic_only
	<< s << "\"" << min::eom;
    LEX::init_input_string
    	( LEX::default_scanner,
	  min::new_ptr ( s ),
	  & ::marked_line_format );
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

    ::marked_line_format.op_flags |=
        min::DISPLAY_NON_GRAPHIC;

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
