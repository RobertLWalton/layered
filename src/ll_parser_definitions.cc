// Layers Language Parser Execute Definition Function
//
// File:	ll_parser_definitions.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun Apr 29 23:37:12 EDT 2012
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
static min::locatable_gen test;
static min::locatable_gen plus;
static min::locatable_gen minus;
static min::locatable_gen exclusive_or;
static min::locatable_gen parsing;
static min::locatable_gen full;
static min::locatable_gen line;

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
    ::test = min::new_str_gen ( "test" );
    ::plus = min::new_str_gen ( "+" );
    ::minus = min::new_str_gen ( "-" );
    ::exclusive_or = min::new_str_gen ( "^" );
    ::parsing = min::new_str_gen ( "parsing" );
    ::full = min::new_str_gen ( "full" );
    ::line = min::new_str_gen ( "line" );
}
static min::initializer initializer ( ::initialize );

static min::gen expected_error
	( min::printer printer,
	  min::file file,
	  min::phrase_position pp,
	  const char * what )
{
    printer << min::bom << min::set_indent ( 7 )
	    << "ERROR: in " << min::pline_numbers
			           ( file, pp )
	    << ": expected " << what << " after:"
	    << min::eom;
    min::print_phrase_lines ( printer, file, pp );
    return min::ERROR();
}

// Parser Definition Functions
// ------ ---------- ---------

min::gen PAR::scan_simple_label
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

static min::gen scan_selectors
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  TAB::selectors & selectors,
	  TAB::new_selectors & new_selectors,
	  PAR::parser parser,
	  bool scan_new_selectors )
{
    if ( i >= min::size_of ( vp ) )
        return min::MISSING();

    min::obj_vec_ptr subvp ( vp[i] );

    if ( subvp == min::NULL_STUB )
        return min::MISSING();

    min::attr_ptr subap ( subvp );
    min::locate ( subap, PAR::dot_initiator );
    if ( min::get ( subap ) != PAR::left_square )
        return min::MISSING();
    min::locate ( subap, PAR::dot_terminator );
    if ( min::get ( subap ) != PAR::right_square )
        return min::MISSING();
    min::locate ( subap, PAR::dot_separator );
    min::gen separator = min::get ( subap );
    min::locate ( subap, PAR::dot_position );
    min::phrase_position_vec ppvec = min::get ( subap );
    assert ( ppvec != min::NULL_STUB );

    ++ i;

    selectors = 0;
    new_selectors.or_selectors = 0;
    new_selectors.not_selectors = 0;
    new_selectors.xor_selectors = 0;
    min::unsptr size = min::size_of ( subvp );
    min::locatable_gen selector;
    bool op_seen = false;
    if ( separator == min::NONE() )
    {
	min::uns32 i = 0;
	while ( true )
	{
	    min::gen op = min::MISSING();
	    if ( scan_new_selectors
	         &&
		 ( subvp[i] == ::plus
	           ||
		   subvp[i] == ::minus
		   ||
		   subvp[i] == ::exclusive_or ) )
	    {
	        if ( i == 0 ) op_seen = true;
		else if ( ! op_seen )
		{
		    parser->printer
			<< min::bom
			<< min::set_indent ( 7 )
			<< "ERROR: selector operation "
			<< subvp[i]
			<< " found after selector with"
			   " NO operation in "
			<< min::pline_numbers
			       ( ppvec->file,
				 ppvec[i] )  
			<< ":" << min::eom;
		    min::print_phrase_lines
			( parser->printer,
			  ppvec->file,
			  ppvec[i] );
		    return min::ERROR();
		}
	        op = subvp[i++];
	    }
	    else
	    {
	        if ( i == 0 ) op_seen = false;
		else if ( op_seen )
		    return ::expected_error
			( parser->printer, ppvec->file,
			  ppvec[i-1],
			  "`+', `-', or `^'" );
	    }
	    min::uns32 ibegin = i;

	    selector = PAR::scan_simple_label
		( subvp, i,
	            ( 1ull << LEXSTD::word_t )
		  + ( 1ull << LEXSTD::number_t ) );
	    if ( selector == min::ERROR() )
	        return min::ERROR();
	    else if ( selector == min::MISSING() )
	    {
		min::phrase_position pp;
		if ( i == 0 )
		{
		    pp = ppvec->position;
		    pp.end = pp.begin;
		    ++ pp.end.offset;
		}
		else
		    pp = ppvec[i-1];

		return ::expected_error
		    ( parser->printer,
		      ppvec->file, pp, "name" );
	    }

	    int j = TAB::get_index
	        ( parser->selector_name_table,
		  selector );

	    if ( j >= 0 )
	    {
	        if ( op == ::plus )
		    new_selectors.or_selectors |=
		        (min::uns64) 1 << j;
	        else if ( op == ::minus )
		    new_selectors.not_selectors |=
		        (min::uns64) 1 << j;
	        else if ( op == ::exclusive_or )
		    new_selectors.xor_selectors |=
		        (min::uns64) 1 << j;
		else
		    selectors |= (min::uns64) 1 << j;
	    }
	    else
	    {
		min::phrase_position pp;
		pp.begin = ppvec[ibegin].begin;
		pp.end = ppvec[i-1].end;

		parser->printer
		    << min::bom
		    << min::set_indent ( 7 )
		    << "ERROR: unrecognized selector"
		       " name in "
		    << min::pline_numbers
			   ( ppvec->file, pp )
		    << ":"
		    << min::eom;
		min::print_phrase_lines
		    ( parser->printer,
		      ppvec->file, pp );

		return min::ERROR();
	    }

	    if ( i == size ) break;

	    if ( subvp[i] != PAR::comma)
		return ::expected_error
		    ( parser->printer, ppvec->file,
		      ppvec[i-1], "`,'" );
	    else if ( ++ i >= size )
		return ::expected_error
		    ( parser->printer, ppvec->file,
		      ppvec[i-1], "name" );
	}
    }
    else if ( separator == PAR::comma )
    {
	for ( min::uns32 i = 0; i < size; ++ i )
	{
	    min::obj_vec_ptr np ( subvp[i] );
	    if ( np == min::NULL_STUB )
	    {
	        selector = subvp[i];
		if (    LEXSTD::lexical_type_of
		            ( selector )
		     != LEXSTD::word_t )
		    selector = min::MISSING();
	    }
	    else
	    {
		min::uns32 j = 0;
		selector = PAR::scan_simple_label
		    ( np, j,
	            ( 1ull << LEXSTD::word_t )
		  + ( 1ull << LEXSTD::number_t ) );

		if ( selector == min::ERROR() )
		    return min::ERROR();
	    }

	    if ( selector == min::MISSING() )
	    {
		parser->printer
		    << min::bom
		    << min::set_indent ( 7 )
		    << "ERROR: " << subvp[i]
		    << " is not a selector name in "
		    << min::pline_numbers
			   ( ppvec->file, ppvec[i] )
		    << ":" << min::eom;
		min::print_phrase_lines
		    ( parser->printer,
		      ppvec->file,
		      ppvec[i] );

		return min::ERROR();
	    }

	    int j = TAB::get_index
	        ( parser->selector_name_table,
		  selector );

	    if ( j >= 0 )
	        selectors |= (min::uns64) 1 << j;
	    else
	    {
		parser->printer
		    << min::bom
		    << min::set_indent ( 7 )
		    << "ERROR: unrecognized selector"
		       " name in "
		    << min::pline_numbers
			   ( ppvec->file, ppvec[i] )
		    << ":"
		    << min::eom;
		min::print_phrase_lines
		    ( parser->printer,
		      ppvec->file, ppvec[i] );

		return min::ERROR();
	    }
	}
    }
    else
    {
	parser->printer
	    << min::bom << min::set_indent ( 7 )
	    << "ERROR: bad separator "
	    << min::pgen ( separator )
	    << " in "
	    << min::pline_numbers
		   ( ppvec->file,
		     ppvec->position )  
	    << ":"
	    << min::eom;
	min::print_phrase_lines
	    ( parser->printer,
	      ppvec->file,
	      ppvec->position );
	return min::ERROR();
    }

    if ( ! op_seen && scan_new_selectors )
    {
        new_selectors.or_selectors = selectors;
        new_selectors.not_selectors = ~ selectors;
    }

    return min::SUCCESS();
}

min::gen PAR::scan_selectors
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  TAB::selectors & selectors,
	  PAR::parser parser )
{
    TAB::new_selectors new_selectors;
    return ::scan_selectors
        ( vp, i, selectors, new_selectors,
	  parser, false );
}

min::gen PAR::scan_new_selectors
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  TAB::new_selectors & new_selectors,
	  PAR::parser parser )
{
    TAB::selectors selectors;
    return ::scan_selectors
        ( vp, i, selectors, new_selectors,
	  parser, true );
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

static void parser_execute_test_scan
	( min::obj_vec_ptr & vp,
	  min::printer printer )
{
    min::phrase_position_vec ppvec =
        min::position_of ( vp );
    min::unsptr size = min::size_of ( vp );
    for ( min::unsptr i = 0; i < size; ++ i )
    {
        min::obj_vec_ptr subvp ( vp[i] );
	if ( subvp == min::NULL_STUB ) continue;
	printer
	    << min::bom << min::set_indent ( 8 )
	    << "------- "
	    << min::pline_numbers
	          ( ppvec->file, ppvec[i] )
	<< ":" << min::eom;
	min::print_phrase_lines
	    ( printer, ppvec->file, ppvec[i] );
	::parser_execute_test_scan ( subvp, printer );
    }
}

static min::gen parser_execute_test
	( min::obj_vec_ptr & vp,
	  PAR::parser parser )
{
    min::phrase_position_vec ppvec =
        min::position_of ( vp );
    parser->printer
        << min::bom << min::set_indent ( 8 )
	<< "======= TEST: "
	<< min::pline_numbers
	       ( ppvec->file, ppvec->position )
	<< ":" << min::eom;
    min::print_phrase_lines
        ( parser->printer,
	  ppvec->file, ppvec->position );
    ::parser_execute_test_scan ( vp, parser->printer );
    parser->printer << "======= END TEST" << min::eol;
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
    if ( size < 2 ) return min::FAILURE();

    if ( vp[0] != ::parser ) return min::FAILURE();
    else if ( vp[1] == ::print )
        return ::parser_execute_print ( vp, parser );
    else if ( vp[1] == ::test )
        return ::parser_execute_test ( vp, parser );


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
        name[0] = PAR::scan_simple_label
	    ( vp, i,
	        ( 1ull << LEXSTD::word_t )
	      + ( 1ull << LEXSTD::number_t ) );
	if ( name[0] == min::ERROR() )
	    return min::ERROR();
	else if ( name[0] == min::MISSING() )
	    return ::expected_error
	        ( parser->printer, ppvec->file,
		  ppvec[i-1], "name" );
	else
	    ++ number_of_names;
    }
    else while ( true )
    {
	// Scan a name.
	//
	name[number_of_names] =
	    PAR::scan_name_string_label
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
	    return ::expected_error
	        ( parser->printer, ppvec->file,
		  ppvec[i-1], "quoted name" );
	else
	    ++ number_of_names;

	if ( number_of_names > max_names )
	{
	    parser->printer
		<< min::bom << min::set_indent ( 7 )
		<< "ERROR: too many quoted names"
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
    else
    {
	TAB::selectors selectors;
        min::gen sresult = PAR::scan_selectors
		( vp, i, selectors, parser );
	if ( sresult == min::ERROR() )
	    return min::ERROR();
	else if ( sresult == min::MISSING() )
	    return ::expected_error
		( parser->printer,
		  ppvec->file, ppvec[i-1],
		  "selectors" );

	switch ( type )
	{
	case ::BRACKET:
	{
	    bool full_line = false;
	    TAB::new_selectors new_selectors;
	        // Inited to zeroes.
	    while ( i < size && vp[i] == ::with )
	    {
	        ++ i;
		if ( i + 1 < size
		     &&
		     vp[i] == ::parsing
		     &&
		     vp[i+1] == ::selectors )
		{
		    i += 2;
		    min::gen result =
		        PAR::scan_new_selectors
			    ( vp, i, new_selectors,
			      parser );
		    if ( result == min::ERROR() )
		        return min::ERROR();
		    else if ( result == min::MISSING() )
		    {
		    }
		}
		else if ( i + 1 < size
		          &&
		          vp[i] == ::full
		          &&
		          vp[i+1] == ::line )
		{
		    i += 2;
		    full_line = true;
		}
		else
		    return ::expected_error
			( parser->printer, ppvec->file,
			  ppvec[i-1],
			  "`parsing selectors'"
			  " or `full line'" );
	    }
	    if ( i < size )
		return ::expected_error
		    ( parser->printer, ppvec->file,
		      ppvec[i-1], "`with'" );

	    TAB::push_brackets
	        ( name[0], name[1],
		  selectors, new_selectors, full_line,
		  parser->bracket_table );

	    break;
	}
	case ::INDENTATION_MARK:
	{
	    TAB::new_selectors new_selectors;
	        // Inited to zeroes.
	    while ( i < size && vp[i] == ::with )
	    {
	        ++ i;
		if ( i + 1 < size
		     &&
		     vp[i] == ::parsing
		     &&
		     vp[i+1] == ::selectors )
		{
		    i += 2;
		    min::gen result =
		        PAR::scan_new_selectors
			    ( vp, i, new_selectors,
			      parser );
		    if ( result == min::ERROR() )
		        return min::ERROR();
		    else if ( result == min::MISSING() )
		    {
		    }
		}
		else
		    return ::expected_error
			( parser->printer, ppvec->file,
			  ppvec[i-1],
			  "`parsing selectors'"
			  " or `full line'" );
	    }
	    if ( i < size )
		return ::expected_error
		    ( parser->printer, ppvec->file,
		      ppvec[i-1], "`with'" );

	    if ( gluing
	         &&
		    LEXSTD::lexical_type_of ( name[0] )
		 != LEXSTD::mark_t )
	    {
		parser->printer
		    << min::bom << min::set_indent ( 7 )
		    << "ERROR: gluing indentation mark"
		       " name "
		    << min::pgen ( name[0] )
		    << " is not a mark in "
		    << min::pline_numbers
			   ( ppvec->file,
			     ppvec[5] )  
		    << ":" << min::eom;
		min::print_phrase_lines
		    ( parser->printer,
		      ppvec->file,
		      ppvec[5] );
		return min::ERROR();
	    }

	    TAB::push_indentation_mark
	        ( name[0],
		  number_of_names == 2 ?
		      (min::gen) name[1] :
		      min::MISSING(),
		  selectors, new_selectors,
		  parser->bracket_table,
		  gluing ? parser->split_table :
			   (TAB::split_table)
		           min::NULL_STUB );

	    break;
	}
	case ::NAMED_BRACKET:
	{
	    if ( i < size )
		return ::expected_error
		    ( parser->printer, ppvec->file,
		      ppvec[i-1], "end of statement" );
	    bool separator_present =
	        ( number_of_names % 2 == 1 );
	    bool middle_present =
	        ( number_of_names >= 4 );

	    min::unsptr m = 1 + separator_present;
	    if (    middle_present
	         && name[m] != name[m+1] )
	    {
		min::phrase_position pp;
		pp.begin = ppvec[m].begin;
		pp.end   = ppvec[m+1].end;
		parser->printer
		    << min::bom << min::set_indent ( 7 )
		    << "ERROR: named middles "
		    << min::pgen ( name[m] )
		    << " and "
		    << min::pgen ( name[m+1] )
		    << " do not match in "
		    << min::pline_numbers
			   ( ppvec->file, pp )  
		    << ":" << min::eom;
		min::print_phrase_lines
		    ( parser->printer,
		      ppvec->file, pp );
		return min::ERROR();
	    }

	    min::gen named_opening = name[0];
	    min::gen named_separator =
	        ( separator_present ?
		  (min::gen) name[1] : min::MISSING() );
	    min::gen named_middle =
	        ( middle_present ?
		  (min::gen) name[m] : min::MISSING() );
	    min::gen named_closing =
	        name[1 + separator_present
		       + 2 * middle_present ];

	    // compute named_middle_closing if
	    // necessary.
	    //
	    min::locatable_gen named_middle_closing
	        ( min::MISSING() );;
	    min::gen middle_last = named_middle;
	    min::gen closing_first = named_closing;
	    min::unsptr middle_length = 1;
	    min::unsptr closing_length = 1;

	    min::lab_ptr middle_ptr ( named_middle );
	    min::lab_ptr closing_ptr ( named_closing );
	    if ( middle_ptr != min::NULL_STUB )
	    {
		middle_length =
		    min::length_of ( middle_ptr );
		middle_last =
		    middle_ptr[middle_length-1];
	    }
	    if ( closing_ptr != min::NULL_STUB )
	    {
		closing_length =
		    min::length_of ( closing_ptr );
		closing_first =
		    closing_ptr[closing_length-1];
	    }
	    min::uns32 middle_last_type =
	        LEXSTD::lexical_type_of
		    ( middle_last );
	    min::uns32 closing_first_type =
	        LEXSTD::lexical_type_of
		    ( closing_first );
	    if ( middle_last_type == closing_first_type
	         ||
		 middle_last_type != LEXSTD::separator_t
	       )
	    {
		min::str_ptr middle_last_ptr
		    ( middle_last );
		min::str_ptr closing_first_ptr
		    ( closing_first );
		min::unsptr middle_last_length =
		    min::strlen ( middle_last_ptr );
		min::unsptr closing_first_length =
		    min::strlen ( closing_first_ptr );
		char new_string [   middle_last_length
		                  + closing_first_length
				  + 1 ];
		strcpy
		    ( & new_string[0],
		      & middle_last_ptr[0] );
		strcpy
		    ( & new_string[middle_last_length],
		      & closing_first_ptr[0] );
		named_middle_closing =
		    min::new_str_gen ( new_string );
		if (   middle_length
		     + closing_length > 2 )
		{
		    min::gen element
			[middle_length + closing_length
				       - 1];
		    memcpy ( & element[0],
		             & middle_ptr[0],
			       ( middle_length - 1 )
			     * sizeof ( min::gen ) );
		    memcpy ( & element[middle_length],
			     & closing_ptr[0],
			       ( closing_length - 1 )
			     * sizeof ( min::gen ) );
		    element[middle_length - 1] =
			named_middle_closing;
		    named_middle_closing =
			min::new_lab_gen
			    ( element, 
				middle_last_length
			      + closing_first_length
			      - 1 );
		}
	    }

	    TAB::push_named_brackets
	        ( named_opening,
		  named_separator,
		  named_middle,
		  named_closing,
		  named_middle_closing,
		  selectors,
		  parser->bracket_table );

	    break;
	}
	default:
	    MIN_ABORT ( "bad parser (un)define type" );
	}
    }

    return min::SUCCESS();
}
