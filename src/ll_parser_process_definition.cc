// Layers Language Parser Process Definition Function
//
// File:	ll_parser_process_definition.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Dec 31 11:22:32 EST 2011
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

static min::locatable_gen define;
static min::locatable_gen undefine;
static min::locatable_gen bracket;
static min::locatable_gen indentation;
static min::locatable_gen mark;
static min::locatable_gen gluing;
static min::locatable_gen named;
static min::locatable_gen dotdotdot;
static min::locatable_gen with;

static struct initializer {
    initializer ( void )
    {
        ::define = Min::new_str_gen ( "define" );
        ::undefine = min::new_str_gen ( "undefine" );
        ::bracket = min::new_str_gen ( "bracket" );
        ::indentation = min::new_str_gen
				( "indentation" );
        ::mark = min::new_str_gen ( "mark" );
        ::gluing = min::new_str_gen ( "gluing" );
        ::named = min::new_str_gen ( "named" );
        ::dotdotdot = min::new_str_gen ( "..." );
        ::with = min::new_str_gen ( "with" );
    }
} init;

enum definition_type
    { BRACKET, INDENTATION_MARK, NAMED_BRACKET };

bool TAB::parser_execute_definition
	( ll::parser::parser parser,
	  const min::obj_vec_ptr & vp,
	  min::printer printer,
	  min::phrase_position_vec ppvec )
{
    min::uns32 size = min::size_of ( vp );
    if ( size < 2 ) return false;

    // Scan keywords before names.
    //
    bool define;
        // True if define, false if undefine.
    definition_type type;
        // Type of define or undefine.
    unsigned i = 0;
        // vp[i] is next lexeme or subexpression to
	// scan in the define/undefine expression.
    bool gluing = false;
        // True if `define/undefine gluing ...', false
	// if not.

    if ( vp[i] == ::define )
        define = true;
    else if ( vp[i] == ::undefine )
        define = false;
    else
        return false;
    ++ i;

    if ( vp[i] == ::bracket )
    {
        type = ::BRACKET;
	++ i;
    }
    else if ( vp[i] == ::indentation
              &&
	      i + 1 < size
	      &&
	      vp[i + 1] == ::mark )
    {
	type = ::INDENTATION_MARK;
	i += 2;
    }
    else if ( vp[i] == ::gluing
              &&
	      i + 2 < size
	      &&
	      vp[i + 1] == ::indentation )
	      &&
	      vp[i + 2] == ::mark )
    {
	type = ::INDENTATION_MARK;
	gluing = true;
	i += 3;
    }
    else if ( vp[i] == ::named
              &&
	      i + 1 < size
              &&
	      vp[i + 1] == ::bracket )
    {
	type = ::NAMED_BRACKET;
	i += 2;
    }
    else
        return false;

    // Scan mark names.  There can be up to MAX_NAMES.
    //
    const unsigned MAX_NAMES = 5;
    min::locatable_gen names[MAX_NAMES];
    unsigned number_of_names = 0;

    while ( i < size )
    {
	// Scan a name.
	//
	unsigned name_start = i;
	while ( i < size )
	{
	    min::gen g = vp[i];
	    if ( g == ::dotdotdot ) break

	    min::uns32 t =
	        LEXSTD::lexical_type_of ( g );

	    if ( ( t == 0 && ::is_quoted_string ( g ) )
	         ||
	         t == LEX::MARK )
	    {
	        ++ i;
		continue;
	    }
	    else
	        break;
	}
	
	if ( i == name_start )
	{
	    printer
		<< min::bom << min::set_indent ( 7 )
		<< "ERROR: empty bracket name after "
		<< min::pgen ( vp[i-1] )
		<< " in "
		<< min::pline_numbers
		       ( ppvec->file,
			 ppvec->position )  
		<< ":" << min::eom;
	    min::print_phrase_lines
		( printer,
		  ppvec->file,
		  ppvec->position );
	    return true;
	}
    }
}
