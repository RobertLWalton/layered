// Layers Language Parser Execute Definition Function
//
// File:	ll_parser_execute_definition.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu Jan 19 03:29:33 EST 2012
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents:
//
//	Usage and Setup
//	Execute Definition Function

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

static void initialize ( void )
{
    ::define = min::new_str_gen ( "define" );
    ::undefine =
	min::new_str_gen ( "undefine" );
    ::bracket = min::new_str_gen ( "bracket" );
    ::indentation = min::new_str_gen
			    ( "indentation" );
    ::mark = min::new_str_gen ( "mark" );
    ::gluing = min::new_str_gen ( "gluing" );
    ::named = min::new_str_gen ( "named" );
    ::dotdotdot = min::new_str_gen ( "..." );
    ::with = min::new_str_gen ( "with" );
}
static min::initializer initializer ( ::initialize );

// Execute Definition Function
// ------- ---------- --------

enum definition_type
    { BRACKET, INDENTATION_MARK, NAMED_BRACKET };

bool TAB::parser_execute_definition
	( min::obj_vec_ptr & vp,
	  min::printer printer,
	  TAB::selector_name_table selector_name_table,
	  TAB::table bracket_table,
	  TAB::split_table split_table )
{
    min::uns32 size = min::size_of ( vp );
    min::phrase_position_vec ppvec =
        min::position_of ( vp );
    assert ( ppvec != min::NULL_STUB );

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
    unsigned min_names, max_names;
        // Minimum and maximum number of names allowed.

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
	min_names = 2;
	max_names = 2;
	++ i;
    }
    else if ( vp[i] == ::indentation
              &&
	      i + 1 < size
	      &&
	      vp[i + 1] == ::mark )
    {
	type = ::INDENTATION_MARK;
	min_names = 1;
	max_names = 2;
	i += 2;
    }
    else if ( vp[i] == ::gluing
              &&
	      i + 2 < size
	      &&
	      vp[i + 1] == ::indentation
	      &&
	      vp[i + 2] == ::mark )
    {
	type = ::INDENTATION_MARK;
	min_names = 1;
	max_names = 2;
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
	min_names = 2;
	max_names = 6;
	i += 2;
    }
    else
        return false;

    // Scan mark names.
    //
    min::locatable_gen name[max_names];
    unsigned number_of_names = 0;

    while ( i < size )
    {
	// Scan a name.
	//
	unsigned name_start = i;
	while ( i < size )
	{
	    min::gen g = vp[i];
	    if ( g == ::dotdotdot ) break;

	    min::uns32 t =
	        LEXSTD::lexical_type_of ( g );

	    if ( ( t == 0
	           &&
		      PAR::get_initiator ( g )
		   == PAR::doublequote )
	         ||
	         t == LEXSTD::mark_t )
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
		<< "ERROR: empty bracket name in "
		<< min::pline_numbers
		       ( ppvec->file,
			 ppvec->position )  
		<< ":" << min::eom;
	    min::print_phrase_lines
		( printer,
		  ppvec->file,
		  ppvec->position );
	    printer << "       just after:" << min::eol;
	    min::print_phrase_lines
		( printer,
		  ppvec->file,
		  ppvec[i-1] );
	    return true;
	}
	if ( number_of_names >= max_names )
	{
	    printer
		<< min::bom << min::set_indent ( 7 )
		<< "ERROR: too many mark-names in "
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
	name[number_of_names++] =
	    PAR::make_label ( vp, name_start, i );
    }
    if ( number_of_names < min_names )
    {
	printer
	    << min::bom << min::set_indent ( 7 )
	    << "ERROR: too few mark-names in "
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

    // TBD

    return true;
}
