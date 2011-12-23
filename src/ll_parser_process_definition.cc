// Layers Language Parser Process Definition Function
//
// File:	ll_parser_process_definition.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri Dec 23 03:42:00 EST 2011
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

# include <ll_lexeme_standard.h>
# include <ll_parser.h>
# include <ll_parser_table.h>
# define LEXSTD ll::lexeme::standard
# define PAR ll::parser
# define TAB ll::parser::table

static min::locatable_gen DEFINE;
static min::locatable_gen UNDEFINE;
static min::locatable_gen BRACKET;
static min::locatable_gen INDENTATION;
static min::locatable_gen MARK;
static min::locatable_gen GLUING;
static min::locatable_gen NAMED;
static min::locatable_gen DOTDOTDOT;
static min::locatable_gen WITH;

static struct initializer {
    initializer ( void )
    {
        ::DEFINE = min::new_str_gen ( "define" );
        ::UNDEFINE = min::new_str_gen ( "undefine" );
        ::BRACKET = min::new_str_gen ( "bracket" );
        ::INDENTATION = min::new_str_gen
				( "indentation" );
        ::MARK = min::new_str_gen ( "mark" );
        ::GLUING = min::new_str_gen ( "gluing" );
        ::NAMED = min::new_str_gen ( "named" );
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

enum definition_type
    { BRACKET, INDENTATION_MARK, NAMED_BRACKET };

bool TAB::parser_execute_definition
	( ll::parser::parser parser,
	  min::obj_vec_ptr & vp )
{
    min::uns32 size = min::size_of ( vp );
    if ( size < 2 ) return false;

    // Scan keywords before names.
    //
    bool define;
        // True if define, false if undefine.
    definition_type type;
        // Type of define or undefine.
    unsigned offset = 1;
        // vp[offset] is next lexeme or subexpression to
	// scan in the define/undefine expression.
    bool gluing = false;
        // True if `define/undefine gluing ...', false
	// if not.

    if ( vp[0] == ::define )
        define = true;
    else if ( vp[0] == ::undefine )
        define = false;
    else
        return false;

    if ( vp[offset] == ::bracket )
    {
        type = ::BRACKET;
	++ offset;
    }
    else if ( vp[offset] == ::indentation
              &&
	      offset + 1 < size
	      &&
	      vp[offset + 1] == ::mark )
    {
	type = ::INDENTATION_MARK;
	offset += 2;
    }
    else if ( vp[offset] == ::gluing
              &&
	      offset + 2 < size
	      &&
	      vp[offset + 1] == ::indentation )
	      &&
	      vp[offset + 2] == ::mark )
    {
	type = ::INDENTATION_MARK;
	gluing = true;
	offset += 3;
    }
    else if ( vp[offset] == ::named
              &&
	      offset + 1 < size
              &&
	      vp[offset + 1] == ::bracket )
    {
	type = ::NAMED_BRACKET;
	offset += 2;
    }
    else
        return false;

    if ( offset >= size ) return false;

    // Scan mark names.  There can be up to MAX_NAMES.
    //
    const unsigned MAX_NAMES = 5;
    min::locatable_gen names[5];
    unsigned number_of_names = 0;

    while ( offset < size )
    {
	// Scan a name.
	//
	unsigned name_start = offset;
	while ( offset < size )
	{
	    min::gen g = vp[offset];
	    min::uns32 t = LEXSTD::lexical_type_of ( g );
	    if ( min::is_str ( g ) )
	}
	        &&

        if ( offset >= size
	     &&
	     vp[offset] == ::DOTDOTDOT )
	{
	    if ( offset > name_start )
	    {
	    }

	    if ( offset >= size ) break;
	    else
	    {
	    	name_start = ++ offset;
		continue;
	    }
	}
    }
}
