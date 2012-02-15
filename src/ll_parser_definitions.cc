// Layers Language Parser Execute Definition Function
//
// File:	ll_parser_definitions.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Feb 15 06:52:02 EST 2012
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents:
//
//	Usage and Setup
//	Parser Definition Functions
//	Execute Parser Print Function
//	Execute Definition Function

// Usage and Setup
// ----- --- -----

# include <ll_lexeme_standard.h>
# include <ll_parser_definitions.h>
# define LEXSTD ll::lexeme::standard
# define PAR ll::parser
# define TAB ll::parser::table

static min::locatable_gen parser;
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
static min::locatable_gen print;
static min::locatable_gen selectors;

static void initialize ( void )
{
    ::parser = min::new_str_gen ( "parser" );
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
    ::print = min::new_str_gen ( "print" );
    ::selectors = min::new_str_gen ( "selectors" );
}
static min::initializer initializer ( ::initialize );

// Parser Definition Functions
// ------ ---------- ---------

min::gen PAR::make_simple_label
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  min::uns64 accepted_types )
{
    min::uns32 j = i;
    min::uns32 s = min::size_of ( vp );
    while ( i < s )
    {
	min::uns32 t =
	    LEXSTD::lexical_type_of ( vp[i] );
	if ( ( 1ull << t ) & accepted_types )
	    ++ i;
	else
	    break;
    }

    if ( i == j ) return min::MISSING();
    else if ( i == j + 1 ) return vp[j];

    min::gen elements[i-j];
    memcpy ( elements, & vp[j], sizeof ( elements ) );
    return min::new_lab_gen ( elements, i - j );
}

// Execute Parser Print Function
// ------- ------ ----- --------

static min::gen parser_execute_print
	( min::obj_vec_ptr & vp,
	  PAR::parser parser )
{
    min::uns32 size = min::size_of ( vp );
    if ( size < 3 ) return min::FAILURE();

    int i = 2;
    TAB::selector_name_table t =
        parser->selector_name_table;

    if ( vp[i] == ::selectors )
    {
        parser->printer
	    << "parser print selectors:" << min::eol
	    << min::bom << min::nohbreak
	    << min::set_indent ( 8 ) << min::indent
	    << "["
	    << min::set_indent ( 10 ) << min::indent;

	for ( unsigned j = 0; j < t->length; ++ j )
	{
	    if ( j > 0 ) parser->printer << ", ";
	    parser->printer
		<< min::setbreak
		<< min::pgen
		       ( t[j], & PAR::name_format );
	}
	parser->printer << " ]" << min::eom;
    }
    return min::SUCCESS();
}


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
    if ( size < 1 ) return min::FAILURE();

    if ( vp[0] != ::parser ) return min::FAILURE();


    min::phrase_position_vec ppvec =
        min::position_of ( vp );
    assert ( ppvec != min::NULL_STUB );

    // Scan keywords before names.
    //
    bool define;
        // True if define, false if undefine.
    definition_type type;
        // Type of define or undefine.
    unsigned i = 1;
        // vp[i] is next lexeme or subexpression to
	// scan in the define/undefine expression.
    bool gluing = false;
        // True if `define/undefine gluing ...', false
	// if not.
    unsigned min_names, max_names;
        // Minimum and maximum number of names allowed.

    if ( size < 2 ) return min::FAILURE();
    else if ( vp[i] == ::print )
        return ::parser_execute_print ( vp, parser );
    else if ( vp[i] == ::define )
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

    if ( type == ::SELECTOR )
    {
        if ( i < size )
	{
	    parser->printer
		<< min::bom << min::set_indent ( 7 )
		<< "ERROR: extraneous stuff after"
		   " selector name in "
		<< min::pline_numbers
		       ( ppvec->file,
			 ppvec->position )  
		<< ":" << min::eom;
	    min::print_phrase_lines
		( parser->printer,
		  ppvec->file,
		  ppvec[i] );
	    return min::ERROR();
	}

	int i = TAB::get_index
	    ( parser->selector_name_table, name[0] );
	if ( i < 0 )
	{
	    if ( parser->selector_name_table
	               ->length >= 64 )
	    {
		parser->printer
		    << min::bom << min::set_indent ( 7 )
		    << "ERROR: too many selector names;"
		       " table overflow in "
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

	    min::push
	        ( (TAB::selector_name_table_insptr)
	          parser->selector_name_table ) =
		    name[0];
	}
    }

    // TBD

    return min::SUCCESS();
}
