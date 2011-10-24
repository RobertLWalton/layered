// Layers Language Parser Process Definition Function
//
// File:	ll_parser_process_definition.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Oct 24 07:04:25 EDT 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Parser Process Definition Function

// Usage and Setup
// ----- --- -----

# include <ll_parser_table.h>
# define PAR ll::parser
# define TAB ll::parser::table

// Parser Process Definition Function
// ------ ------- ---------- --------

bool PAR::parser_process_definition
	( PAR::parser parser,
	  PAR::token & expression )
{
    if ( expression->type != PAR::EXPRESSION )
        return false;
}
