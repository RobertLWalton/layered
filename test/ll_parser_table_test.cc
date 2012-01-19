// Layers Language Parser Table Test
//
// File:	ll_parser_table_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu Jan 19 03:33:04 EST 2012
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
# define PAR ll::parser
# define TAB ll::parser::table

# include <iostream>
# include <cassert>
using std::cout;
using std::endl;

unsigned failed_count = 0;

# define ASSERT(exp) \
    cout << #exp \
         << ( exp ? " is true" \
	          : ( ++ failed_count, "is false" ) ) \
	 << endl;

int main ( int argc )
{
    min::initialize();
    PAR::init ( PAR::default_parser );
    TAB::table bracket_table =
        PAR::default_parser->bracket_table;
    TAB::split_table split_table =
        PAR::default_parser->split_table;

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
    			    bracket_table )
	     == min::NULL_STUB );

    TAB::push_brackets
        ( opening_parenthesis,
	  closing_parenthesis,
	  64,
	  TAB::new_selectors(2,4,8),
	  false,
	  bracket_table );

    TAB::opening_bracket oparen = (TAB::opening_bracket)
        TAB::find ( opening_parenthesis,
	            bracket_table );
    TAB::closing_bracket cparen = (TAB::closing_bracket)
        TAB::find ( closing_parenthesis,
	            bracket_table );
    ASSERT ( oparen->closing_bracket == cparen );
    ASSERT ( cparen->opening_bracket == oparen );
    ASSERT ( oparen->selectors == 64 );
    ASSERT ( oparen->new_selectors.or_selectors == 2 );
    ASSERT ( oparen->new_selectors.not_selectors == 4 );
    ASSERT ( oparen->new_selectors.xor_selectors == 8 );

    TAB::push_indentation_mark
        ( colon, semicolon,
	  0,
	  TAB::new_selectors ( 2 ),
	  bracket_table,
	  split_table );
    TAB::indentation_mark imark = (TAB::indentation_mark)
        TAB::find ( colon, bracket_table ); 
    TAB::indentation_split isplit =
        split_table[':'];

    ASSERT ( isplit->indentation_mark == imark );
    ASSERT ( imark->indentation_split == isplit );
    ASSERT ( isplit->length == 1 );
    ASSERT ( isplit[0] == ':' );
    ASSERT ( imark->new_selectors.or_selectors == 2 );
    ASSERT ( imark->new_selectors.not_selectors == 0 );
    ASSERT ( imark->new_selectors.xor_selectors == 0 );

    if ( failed_count > 0 )
    {
        cout << failed_count << " ASSERTs are false" << endl;
	exit ( 1 );
    }
}
