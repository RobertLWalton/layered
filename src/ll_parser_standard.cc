// Layers Language Standard Parser Settings
//
// File:	ll_parser_standard.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Dec  1 01:12:34 EST 2012
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Standard

// Usage and Setup
// ----- --- -----

# include <ll_parser_table.h>
# include <ll_parser.h>
# include <ll_parser_standard.h>
# define PAR ll::parser
# define TAB ll::parser::table
# define PARSTD ll::parser::standard

// Standard Brackets
// -------- --------

void PARSTD::init_block ( PAR::parser parser )
{

    min::gen result =
        PAR::begin_block
	    ( parser, PAR::standard_lexeme,
	      PAR::top_level_position );

    MIN_ASSERT ( result == min::SUCCESS() );

    min::locatable_gen code_name
        ( min::new_str_gen ( "code" ) );

    parser->selectors =
        1ull << TAB::find_name
	    ( parser->selector_name_table, code_name );
}
