// Layered Languages Parser Table Test
//
// File:	ll_parser_table_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Nov  8 04:42:20 EST 2016
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Test

// Usage and Setup
// ----- --- -----

# include <ll_lexeme.h>
# include <ll_parser_table.h>
# include <ll_parser.h>
# include <ll_parser_bracketed.h>
# define LEX ll::lexeme
# define PAR ll::parser
# define TAB ll::parser::table
# define BRA ll::parser::bracketed

# include <iostream>
# include <cstdlib>
# include <cassert>
using std::cout;
using std::endl;

unsigned failed_count = 0;

# define ASSERT(exp) \
    cout << #exp \
         << ( exp ? " is true" \
	          : ( ++ failed_count, " is false" ) ) \
	 << endl;

// Test
// ----

int main ( int argc, const char * argv[] )
{
    min::initialize();
    PAR::init ( PAR::default_parser, false );
    BRA::bracketed_pass bracketed_pass =
        (BRA::bracketed_pass)
	PAR::default_parser->pass_stack;
    TAB::key_table bracket_table =
        bracketed_pass->bracket_table;

    min::locatable_gen opening_parenthesis;
    opening_parenthesis = min::new_str_gen ( "(" );
    min::locatable_gen closing_parenthesis;
    closing_parenthesis = min::new_str_gen ( ")" );

    min::locatable_gen opening_brace;
    opening_brace = min::new_str_gen ( "{" );
    min::locatable_gen closing_brace;
    closing_brace = min::new_str_gen ( "}" );

    min::locatable_gen colon;
    colon = min::new_str_gen ( ":" );
    min::locatable_gen semicolon;
    semicolon = min::new_str_gen ( ";" );

    ASSERT (    TAB::find ( opening_brace,
		            TAB::ALL_FLAGS,
    			    bracket_table )
	     == min::NULL_STUB );

    min::phrase_position pp;

    BRA::push_brackets
        ( opening_parenthesis,
	  closing_parenthesis,
	  64,
	  0, pp,
	  TAB::new_flags(2,4,8),
	  min::NULL_STUB, min::NULL_STUB,
	  bracket_table );

    BRA::opening_bracket oparen = (BRA::opening_bracket)
        TAB::find ( opening_parenthesis,
		    TAB::ALL_FLAGS,
	            bracket_table );
    BRA::closing_bracket cparen = (BRA::closing_bracket)
        TAB::find ( closing_parenthesis,
		    TAB::ALL_FLAGS,
	            bracket_table );
    ASSERT ( oparen->closing_bracket == cparen );
    ASSERT ( cparen->opening_bracket == oparen );
    ASSERT ( oparen->selectors == 64 );
    ASSERT ( oparen->new_selectors.or_flags == 2 );
    ASSERT ( oparen->new_selectors.not_flags == 4 );
    ASSERT ( oparen->new_selectors.xor_flags == 8 );

    BRA::push_indentation_mark
        ( colon, semicolon,
	  TAB::ALL_FLAGS,
	  0, pp,
	  TAB::new_flags ( 2 ),
	  min::MISSING(), LEX::MISSING,
	  bracket_table );
    BRA::indentation_mark imark =
        (BRA::indentation_mark)
        TAB::find ( colon,
		    TAB::ALL_FLAGS,
	            bracket_table ); 

    ASSERT ( imark->new_selectors.or_flags == 2 );
    ASSERT ( imark->new_selectors.not_flags == 0 );
    ASSERT ( imark->new_selectors.xor_flags == 0 );

    if ( failed_count > 0 )
    {
        cout << failed_count
	     << " ASSERTs are false" << endl;
	return 1;
    }
    
    return 0;
}
