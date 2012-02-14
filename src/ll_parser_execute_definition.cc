// Layers Language Parser Execute Definition Function
//
// File:	ll_parser_execute_definition.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Feb 13 20:16:37 EST 2012
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
static min::locatable_gen selector;
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
    ::selector = min::new_str_gen ( "selector" );
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
    { SELECTOR,
      BRACKET,
      INDENTATION_MARK,
      NAMED_BRACKET };

min::gen PAR::parser_execute_definition
	( min::obj_vec_ptr & vp,
	  PAR::parser parser )
{
    min::uns32 size = min::size_of ( vp );
    min::phrase_position_vec ppvec =
        min::position_of ( vp );
    assert ( ppvec != min::NULL_STUB );

    if ( size < 2 ) return min::FAILURE();

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
        return min::FAILURE();
    ++ i;

    if ( vp[i] == ::selector )
    {
        type = ::SELECTOR;
	min_names = 1;
	max_names = 1;
	++ i;
    }
    else if ( vp[i] == ::bracket )
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
        return min::FAILURE();

    // Scan mark names.
    //
    min::locatable_gen name[max_names+1];
    unsigned number_of_names = 0;

    if ( type == ::SELECTOR )
    {
        name[0] = PAR::make_simple_label
	    ( vp, i,
	        ( 1ull << LEXSTD::word_t )
	      + ( 1ull << LEXSTD::number_t ) );
	if ( name[0] == min::ERROR() )
	    return min::ERROR();
	else if ( name[0] == min::MISSING() )
	{
	    parser->printer
		<< min::bom << min::set_indent ( 7 )
		<< "ERROR: in "
		<< min::pline_numbers
		       ( ppvec->file,
			 ppvec[i-1] )  
		<< " expected name after :"
		<< min::eom;
	    min::print_phrase_lines
		( parser->printer,
		  ppvec->file,
		  ppvec[i-1] );
	    return min::ERROR();
	}
	else
	    ++ number_of_names;
    }
    else while ( true )
    {
	// Scan a name.
	//
	name[number_of_names] =
	    PAR::make_name_string_label
	        ( vp, i, parser,

	            ( 1ull << LEXSTD::mark_t )
	          + ( 1ull << LEXSTD::separator_t )
	          + ( 1ull << LEXSTD::word_t )
	          + ( 1ull << LEXSTD::number_t ),

	            ( 1ull << LEXSTD::
		                  horizontal_space_t )
	          + ( 1ull << LEXSTD::end_of_file_t ),

	            ( 1ull << LEXSTD::end_of_file_t ) );

	if ( name[number_of_names] == min::ERROR() )
	    return min::ERROR();
	else if (    name[number_of_names]
	          == min::MISSING() )
	{
	    parser->printer
		<< min::bom << min::set_indent ( 7 )
		<< "ERROR: in "
		<< min::pline_numbers
		       ( ppvec->file,
			 ppvec[i-1] )  
		<< " expected quoted string name"
		   " after :"
		<< min::eom;
	    min::print_phrase_lines
		( parser->printer,
		  ppvec->file,
		  ppvec[i-1] );
	    return min::ERROR();
	}
	else
	    ++ number_of_names;

	if ( number_of_names > max_names )
	{
	    parser->printer
		<< min::bom << min::set_indent ( 7 )
		<< "ERROR: too many quoted string names"
		   " in "
		<< min::pline_numbers
		       ( ppvec->file,
			 ppvec->position )  
		<< ":" << min::eom;
	    min::print_phrase_lines
		( parser->printer,
		  ppvec->file,
		  ppvec->position );
	    return min::ERROR();
	}

	if ( i >= size
	     ||
	     vp[i] != ::dotdotdot )
	    break;

	++ i;
    }

    if ( number_of_names < min_names )
    {
	parser->printer
	    << min::bom << min::set_indent ( 7 )
	    << "ERROR: too few defined names in "
	    << min::pline_numbers
		   ( ppvec->file,
		     ppvec->position )  
	    << ":" << min::eom;
	min::print_phrase_lines
	    ( parser->printer,
	      ppvec->file,
	      ppvec->position );
	return min::ERROR();
    }

    // TBD

    return min::SUCCESS();
}
