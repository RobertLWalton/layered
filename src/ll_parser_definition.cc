// Layers Language Parser Execute Definition Function
//
// File:	ll_parser_definition.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun Aug 26 03:47:22 EDT 2012
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents:
//
//	Usage and Setup
//	Parser Definition Functions
//	Execute Selector Definition Function
//	Parser Execute Test Function
//	Execute Definition Function

// Usage and Setup
// ----- --- -----

# include <ll_lexeme_standard.h>
# include <ll_parser_definition.h>
# define LEXSTD ll::lexeme::standard
# define PAR ll::parser
# define TAB ll::parser::table
# define PARDEF ll::parser::definition

static min::locatable_gen test;
static min::locatable_gen plus;
static min::locatable_gen minus;
static min::locatable_gen exclusive_or;

static void initialize ( void )
{
    ::test = min::new_str_gen ( "test" );
    ::plus = min::new_str_gen ( "+" );
    ::minus = min::new_str_gen ( "-" );
    ::exclusive_or = min::new_str_gen ( "^" );
}
static min::initializer initializer ( ::initialize );

// Parser Definition Functions
// ------ ---------- ---------

min::gen PARDEF::expected_error
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

min::gen PARDEF::extra_stuff_after_error
	( min::printer printer,
	  min::file file,
	  min::phrase_position pp )
{
    printer << min::bom << min::set_indent ( 7 )
	    << "ERROR: in " << min::pline_numbers
			           ( file, pp )
	    << ": extraneous stuff after:"
	    << min::eom;
    min::print_phrase_lines ( printer, file, pp );
    return min::ERROR();
}

void PARDEF::misspell_warning
	( min::printer printer,
	  min::file file,
	  min::phrase_position pp,
	  min::gen what_is,
	  min::gen should_be )
{
    printer << min::bom << min::set_indent ( 7 )
	    << "WARNING: in " << min::pline_numbers
			           ( file, pp )
	    << ": " << what_is
	    << " should be " << should_be << ":"
	    << min::eom;
    min::print_phrase_lines ( printer, file, pp );
}

min::gen PARDEF::scan_simple_label
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

static min::gen scan_flags
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  TAB::flags & flags,
	  TAB::new_flags & new_flags,
	  PAR::parser parser,
	  bool scan_new_flags )
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

    flags = 0;
    new_flags.or_flags = 0;
    new_flags.not_flags = 0;
    new_flags.xor_flags = 0;
    min::unsptr size = min::size_of ( subvp );
    min::locatable_gen flag;
    bool op_seen = false;
    if ( separator == min::NONE() )
    {
	min::uns32 i = 0;
	while ( true )
	{
	    min::gen op = min::MISSING();
	    if ( scan_new_flags
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
			<< "ERROR: flag operation "
			<< subvp[i]
			<< " found after flag with"
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
		    return PARDEF::expected_error
			( parser->printer, ppvec->file,
			  ppvec[i-1],
			  "`+', `-', or `^'" );
	    }
	    min::uns32 ibegin = i;

	    flag = PARDEF::scan_simple_label
		( subvp, i,
	            ( 1ull << LEXSTD::word_t )
		  + ( 1ull << LEXSTD::number_t ) );
	    if ( flag == min::ERROR() )
	        return min::ERROR();
	    else if ( flag == min::MISSING() )
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

		return PARDEF::expected_error
		    ( parser->printer,
		      ppvec->file, pp, "name" );
	    }

	    int j = TAB::get_index
	        ( parser->selector_name_table,
		  flag );

	    if ( j >= 0 )
	    {
	        if ( op == ::plus )
		    new_flags.or_flags |=
		        (min::uns64) 1 << j;
	        else if ( op == ::minus )
		    new_flags.not_flags |=
		        (min::uns64) 1 << j;
	        else if ( op == ::exclusive_or )
		    new_flags.xor_flags |=
		        (min::uns64) 1 << j;
		else
		    flags |= (min::uns64) 1 << j;
	    }
	    else
	    {
		min::phrase_position pp;
		pp.begin = ppvec[ibegin].begin;
		pp.end = ppvec[i-1].end;

		parser->printer
		    << min::bom
		    << min::set_indent ( 7 )
		    << "ERROR: unrecognized flag"
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
		return PARDEF::expected_error
		    ( parser->printer, ppvec->file,
		      ppvec[i-1], "`,'" );
	    else if ( ++ i >= size )
		return PARDEF::expected_error
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
	        flag = subvp[i];
		if (    LEXSTD::lexical_type_of
		            ( flag )
		     != LEXSTD::word_t )
		    flag = min::MISSING();
	    }
	    else
	    {
		min::uns32 j = 0;
		flag = PARDEF::scan_simple_label
		    ( np, j,
	            ( 1ull << LEXSTD::word_t )
		  + ( 1ull << LEXSTD::number_t ) );

		if ( flag == min::ERROR() )
		    return min::ERROR();
	    }

	    if ( flag == min::MISSING() )
	    {
		parser->printer
		    << min::bom
		    << min::set_indent ( 7 )
		    << "ERROR: " << subvp[i]
		    << " is not a flag name in "
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
		  flag );

	    if ( j >= 0 )
	        flags |= (min::uns64) 1 << j;
	    else
	    {
		parser->printer
		    << min::bom
		    << min::set_indent ( 7 )
		    << "ERROR: unrecognized flag"
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

    if ( ! op_seen && scan_new_flags )
    {
        new_flags.or_flags = flags;
        new_flags.not_flags = ~ flags;
    }

    return min::SUCCESS();
}

min::gen PARDEF::scan_flags
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  TAB::flags & flags,
	  PAR::parser parser )
{
    TAB::new_flags new_flags;
    return ::scan_flags
        ( vp, i, flags, new_flags,
	  parser, false );
}

min::gen PARDEF::scan_new_flags
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  TAB::new_flags & new_flags,
	  PAR::parser parser )
{
    TAB::flags flags;
    return ::scan_flags
        ( vp, i, flags, new_flags,
	  parser, true );
}

// Execute Selector Definition Function
// ------- -------- ---------- --------

static min::gen parser_execute_selector_definition
	( min::obj_vec_ptr & vp,
          min::phrase_position_vec ppvec,
	  PAR::parser parser )
{
    min::uns32 size = min::size_of ( vp );

    bool define;
    if ( vp[1] == PAR::print )
    {

	TAB::flag_name_table t =
	    parser->selector_name_table;

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
		<< min::set_break
		<< min::pgen ( t[j] );
	}
	parser->printer << " ]" << min::eom;

	if ( vp[2] != PAR::selectors )
	    PARDEF::misspell_warning
	        ( parser->printer,
		  ppvec->file,
		  ppvec[2], vp[2], PAR::selectors );

        if ( size > 3 )
	    return PARDEF::extra_stuff_after_error
	        ( parser->printer,
		  ppvec->file,
		  ppvec[2] );

	return min::SUCCESS();
    }
    else if ( vp[1] == PAR::define )
        define = true;
    else if ( vp[1] == PAR::undefine )
        define = false;
    else return min::FAILURE();

    unsigned i = 3;
    min::locatable_gen name;
    name = PARDEF::scan_simple_label
	( vp, i,
	    ( 1ull << LEXSTD::word_t )
	  + ( 1ull << LEXSTD::number_t ) );
    if ( name == min::ERROR() )
	return min::ERROR();
    else if ( name == min::MISSING() )
	return PARDEF::expected_error
	    ( parser->printer, ppvec->file,
	      ppvec[i-1], "name" );

    int j = TAB::get_index
	( parser->selector_name_table, name );
    if ( j < 0 )
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
	    ( (TAB::flag_name_table_insptr)
	      parser->selector_name_table ) = name;
    }

    if ( vp[2] != PAR::selector )
	PARDEF::misspell_warning
	    ( parser->printer,
	      ppvec->file,
	      ppvec[2], vp[2], PAR::selector );

    if ( i < size )
        return PARDEF::extra_stuff_after_error
	    ( parser->printer,
	      ppvec->file, ppvec[i-1] );

    return min::SUCCESS();
}

// Parser Execute Test Function
// ------ ------- ---- --------

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
	          ( ppvec->file, ppvec[i])
	<< ":" << min::eom;
	min::print_phrase_lines
	    ( printer, ppvec->file, ppvec[i] );
	::parser_execute_test_scan ( subvp, printer );
    }
}

static min::gen parser_execute_test
	( min::obj_vec_ptr & vp,
          min::phrase_position_vec ppvec,
	  PAR::parser parser )
{
    min::gen obj =
        min::new_stub_gen ( (const min::stub *) vp );
    vp = min::NULL_STUB;
        // Close vp so pgen can print obj.
    parser->printer
	<< "======= TEST: "
	<< min::place_indent ( 0 )
	<< min::bom
	<< min::flush_pgen ( obj )
	<< min::eom
	<< min::flush_id_map;
    vp = obj;  // Reopen vp.
    parser->printer
	<< min::bom << min::set_indent ( 8 )
	<< "------- "
	<< min::pline_numbers
	       ( ppvec->file, ppvec->position )
	<< ":" << min::eom;
    min::print_phrase_lines
        ( parser->printer,
	  ppvec->file, ppvec->position );
    ::parser_execute_test_scan
        ( vp, parser->printer );
    parser->printer << "======= END TEST" << min::eol;
    return min::SUCCESS();
}

// Execute Definition Function
// ------- ---------- --------

min::gen PARDEF::parser_execute_definition
	( min::obj_vec_ptr & vp,
	  PAR::parser parser )
{
    min::uns32 size = min::size_of ( vp );
    if ( size < 2 ) return min::FAILURE();
    if ( vp[0] != PAR::parser_lexeme )
        return min::FAILURE();

    min::phrase_position_vec ppvec =
        min::position_of ( vp );
    assert ( ppvec != min::NULL_STUB );

    min::locatable_gen result;

    if ( vp[1] == ::test )
        return ::parser_execute_test
			( vp, ppvec, parser );
    else if ( size >= 3
              &&
	      (    vp[2] == PAR::selector
	        || vp[2] == PAR::selectors ) )
	result = ::parser_execute_selector_definition
		    ( vp, ppvec, parser );
    else
	result =
	    PARDEF::parser_execute_bracket_definition
		( vp, ppvec, parser );



    for ( PAR::pass pass = parser->pass_stack;
          result == min::FAILURE() && pass != NULL;
	  pass = pass->next )
    {
        if ( pass->execute_pass_definition != NULL )
	    result = (* pass->execute_pass_definition )
	        ( vp, ppvec, parser );
    }

    if ( result == min::SUCCESS()
         &&
	 parser->trace & PAR::TRACE_PARSER_DEFINITIONS )
	min::print_phrase_lines
	    ( parser->printer,
	      ppvec->file, ppvec->position, 0 );

    return result;
}
