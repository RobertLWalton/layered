// Layers Language Parser Table Test
//
// File:	ll_parser_table_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri Aug 31 23:48:04 EDT 2012
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

# include <ll_parser_table.h>
# include <ll_parser.h>
# include <ll_parser_bracketed.h>
# define PAR ll::parser
# define TAB ll::parser::table
# define PARBRA ll::parser::bracketed

# include <iostream>
# include <cstdlib>
# include <cassert>
using std::cout;
using std::endl;

unsigned failed_count = 0;

# define ASSERT(exp) \
    cout << #exp \
         << ( exp ? " is true" \
	          : ( ++ failed_count, "is false" ) ) \
	 << endl;

// Test
// ----

int main ( int argc, const char * argv[] )
{
    min::initialize();
    PAR::init ( PAR::default_parser );
    PARBRA::bracketed_pass bracketed_pass =
        (PARBRA::bracketed_pass)
	PAR::default_parser->pass_stack;
    TAB::table bracket_table =
        bracketed_pass->bracket_table;
    TAB::split_table split_table =
        bracketed_pass->split_table;

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

    ASSERT (    TAB::find ( opening_parenthesis,
		            TAB::ALL_FLAGS,
    			    bracket_table )
	     == min::NULL_STUB );

    min::phrase_position pp;

    TAB::push_brackets
        ( opening_parenthesis,
	  closing_parenthesis,
	  64,
	  0, pp,
	  TAB::new_flags(2,4,8),
	  false,
	  bracket_table );

    TAB::opening_bracket oparen = (TAB::opening_bracket)
        TAB::find ( opening_parenthesis,
		    TAB::ALL_FLAGS,
	            bracket_table );
    TAB::closing_bracket cparen = (TAB::closing_bracket)
        TAB::find ( closing_parenthesis,
		    TAB::ALL_FLAGS,
	            bracket_table );
    ASSERT ( oparen->closing_bracket == cparen );
    ASSERT ( cparen->opening_bracket == oparen );
    ASSERT ( oparen->selectors == 64 );
    ASSERT ( oparen->new_selectors.or_flags == 2 );
    ASSERT ( oparen->new_selectors.not_flags == 4 );
    ASSERT ( oparen->new_selectors.xor_flags == 8 );

    TAB::push_indentation_mark
        ( colon, semicolon,
	  TAB::ALL_FLAGS,
	  0, pp,
	  TAB::new_flags ( 2 ),
	  bracket_table,
	  split_table );
    TAB::indentation_mark imark =
        (TAB::indentation_mark)
        TAB::find ( colon,
		    TAB::ALL_FLAGS,
	            bracket_table ); 
    TAB::indentation_split isplit =
        split_table[':'];

    ASSERT ( isplit->indentation_mark == imark );
    ASSERT ( imark->indentation_split == isplit );
    ASSERT ( isplit->length == 1 );
    ASSERT ( isplit[0] == ':' );
    ASSERT ( imark->new_selectors.or_flags == 2 );
    ASSERT ( imark->new_selectors.not_flags == 0 );
    ASSERT ( imark->new_selectors.xor_flags == 0 );

    if ( failed_count > 0 )
    {
        cout << failed_count
	     << " ASSERTs are false" << endl;
	exit ( 1 );
    }
}
