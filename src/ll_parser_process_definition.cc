// Layers Language Parser Process Definition Function
//
// File:	ll_parser_process_definition.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Oct 25 05:36:50 EDT 2011
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

# include <ll_parser.h>
# include <ll_parser_table.h>
# define PAR ll::parser
# define TAB ll::parser::table

min::locatable_gen DEFINE;

struct initializer {
    initializer ( void )
    {
        ::DEFINE = min::new_str_gen ( "define" );
    }
} init;

// Parser Process Definition Function
// ------ ------- ---------- --------

bool PAR::parser_process_definition
	( PAR::parser parser,
	  PAR::token & expression )
{
    if ( expression->type != PAR::EXPRESSION )
        return false;
    min::gen expr = expression->value;
    if ( ! min::is_obj ( expr ) ) return false;
    min::obj_vec_ptr vp ( expr );
    min::unsptr length = size_of ( vp );
    if ( length < 2 ) return false;
    if ( vp[0] != DEFINE ) return false;
}
