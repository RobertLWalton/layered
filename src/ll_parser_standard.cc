// Layered Languages Standard Parser Settings
//
// File:	ll_parser_standard.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Dec 29 19:13:08 EST 2015
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

// Standard
// --------

void PARSTD::init_block ( PAR::parser parser )
{

    min::gen result =
        PAR::begin_block
	    ( parser, PAR::standard_lexeme,
	      PAR::top_level_position );

    MIN_REQUIRE ( result == min::SUCCESS() );

    min::locatable_gen prefix_name
        ( min::new_str_gen ( "prefix" ) );
    min::locatable_gen code_name
        ( min::new_str_gen ( "code" ) );

    parser->selectors &= PAR::ALL_OPT;
    parser->selectors |=
          1ull << TAB::find_name
	      ( parser->selector_name_table,
	        prefix_name )
        | 1ull << TAB::find_name
	      ( parser->selector_name_table,
	        code_name )
	| PAR::ALWAYS_SELECTOR;
}
