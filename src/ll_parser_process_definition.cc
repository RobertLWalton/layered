// Layers Language Parser Process Definition Function
//
// File:	ll_parser_process_definition.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Oct 26 01:35:21 EDT 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Parser Definition Functions
//	Parser Definition Executors
//	Parser Process Definition Function

// Usage and Setup
// ----- --- -----

# include <ll_parser.h>
# include <ll_parser_table.h>
# define PAR ll::parser
# define TAB ll::parser::table

min::locatable_gen DEFINE;
min::locatable_gen BRACKET;
min::locatable_gen DOTDOTDOT;
min::locatable_gen WITH;

struct initializer {
    initializer ( void )
    {
        ::DEFINE = min::new_str_gen ( "define" );
        ::BRACKET = min::new_str_gen ( "bracket" );
        ::DOTDOTDOT = min::new_str_gen ( "..." );
        ::WITH = min::new_str_gen ( "with" );
    }
} init;

// Parser Definition Functions
// ------ ---------- ---------

// Parse a definition name.  vp points at the expres-
// sion, i is the index of the current element,
// n is the max number of names in the definition
// name, and `names' is a vector of n min::gen elements
// into which the names in the definition name are
// copied.  These names are represented by strings or
// labels.  The number of names actually found (<= n) is
// returned if there is no error.
//
// A definition name is terminated by a separator, mark,
// or the word `with'.  Quoted strings in names are
// converted to strings.
//
// Errors: TBD
//
static int parse_definition_name
	( min::obj_vec_ptr & vp, min::unsptr & i,
	  min::unsptr n, min::gen * names,
	  PAR::parser parser )
{
    min::unsptr ncount = 0;
    min::unsptr size = min::size_of ( vp );
    while ( true )
    {
        min::unsptr ibegin = i;
    }
}


// Parser Definition Executors
// ------ ---------- ---------

static bool define_bracket
	( min::obj_vec_ptr & vp,
	  PAR::parser parser )
{
    min::gen names[2];
    min::unsptr i = 2;
    min::unsptr ncount = ::parse_definition_name
			    ( vp, i, 2, names, parser );
}

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
    if ( vp[0] == DEFINE )
    {
        if ( vp[1] == BRACKET )
	    return ::define_bracket ( vp, parser );
    }
    else return false;
}
