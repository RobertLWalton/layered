// Layered Languages Standard Primary Parser
//
// File:	ll_parser_standard_standard.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri Nov  3 01:47:36 EDT 2023
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Define Standard Primary

// Usage and Setup
// ----- --- -----

# include <ll_parser.h>
# include <ll_parser_table.h>
# include <ll_parser_standard.h>
# include <ll_parser_primary.h>
# define PAR ll::parser
# define TAB ll::parser::table
# define PARSTD ll::parser::standard
# define PRIM ll::parser::primary

// Define Standard Primary
// ------ -------- -------

void PARSTD::define_primary
	( PAR::parser parser, TAB::flags components )
{
    if ( components & PARSTD::PRIMARY_PARSING )
	PRIM::init_primary ( parser );
}

