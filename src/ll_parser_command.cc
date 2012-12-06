// Layers Language Parser Execute Command Function
//
// File:	ll_parser_command.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Nov 20 07:07:56 EST 2012
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents:
//
//	Usage and Setup
//	Parser Command Functions
//	Execute Selectors
//	Execute Test
//	Execute Trace
//	Execute Begin/End
//	Execute Command

// Usage and Setup
// ----- --- -----

# include <ll_lexeme_standard.h>
# include <ll_parser_command.h>
# define LEXSTD ll::lexeme::standard
# define PAR ll::parser
# define TAB ll::parser::table
# define COM ll::parser::command

static min::locatable_gen exclusive_or;

static min::locatable_gen test;
static min::locatable_gen trace;

static void initialize ( void )
{
    ::exclusive_or = min::new_str_gen ( "^" );

    ::test = min::new_str_gen ( "test" );
    ::trace = min::new_str_gen ( "trace" );
}
static min::initializer initializer ( ::initialize );

// Parser Command Functions
// ------ ------- ---------

min::gen COM::scan_simple_label
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  min::uns64 accepted_types,
	  min::gen end_value )
{
    min::uns32 j = i;
    min::uns32 s = min::size_of ( vp );
    while ( i < s )
    {
	min::uns32 t =
	    LEXSTD::lexical_type_of ( vp[i] );
	if ( ( ( 1ull << t ) & accepted_types )
	     &&
	     vp[i] != end_value )
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

// Look up a flag in a flag name table and return its
// index.  If it is not there, print an error message
// and return -1.  Position is the position of the
// flag in parser->input_file, for the error message.
//
static int lookup_flag
    ( min::gen flag,
      TAB::name_table name_table,
      PAR::parser parser,
      min::phrase_position pp )
{
    int j = TAB::get_index ( name_table, flag );

    if ( j == -1 )
        PAR::parse_error
	    ( parser, pp, "unrecognized flag name" );

    return j;
}

// Scan a `flag-name' or an `operator flag-name' for
// scan_new_flags below.  Update index i and return
// op and flag.  If no op, set op to min::MISSING().
// Return index of flag name in name_table.  If error,
// including flag name not being in table, print error
// message and return -1.  ppvec is position of vp
// object, and ppvec->file == parser->input_file is
// required.
//
static int scan_flag
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  min::phrase_position_vec ppvec,
	  min::gen & op,
	  TAB::name_table name_table,
	  PAR::parser parser,
	  bool allow_flag_list,
	  bool allow_flag_modifier_list,
	  bool is_subexpression = false )
{
    op = min::MISSING();
    min::unsptr size = min::size_of ( vp );

    assert ( ppvec->file == parser->input_file );

    if ( i >= size )
    {
	PAR::parse_error
	    ( parser,
	      i == 0 ? ppvec->position :
	               ppvec[i-1],
	      ! allow_flag_list ?
	          "expected `+', `-', `^" :
	      ! allow_flag_modifier_list ?
	          "expected file name" :
	          "expected `+', `-', `^',"
		  " or flag name",
	      i == 0 ? " at beginning of" :
	               " after" );
	return -1;
    }

    if ( vp[i] == PAR::plus )
        op = PAR::plus, ++ i;
    else if ( vp[i] == PAR::minus )
        op = PAR::minus, ++ i;
    else if ( vp[i] == ::exclusive_or )
        op = ::exclusive_or, ++ i;

    if (    ! allow_flag_modifier_list
         && op != min::MISSING() )
    {
	PAR::parse_error
	    ( parser,
	      i == 1 ? ppvec->position :
	               ppvec[i-2],
	      "expected flag name",
	      i == 1 ? " at beginning of" :
	                "after" );
	return -1;
    }
    else if (    ! allow_flag_list
              && op == min::MISSING() )
    {
	PAR::parse_error
	    ( parser,
	      i == 0 ? ppvec->position :
	               ppvec[i-1],
	      "expected `+', `-', `^",
	      i == 0 ? " at beginning of" :
	               " after" );
	return -1;
    }

    min::uns32 ibegin = i;
    min::locatable_gen flag
        ( COM::scan_simple_label
	    ( vp, i,
		( 1ull << LEXSTD::word_t )
	      + ( 1ull << LEXSTD::number_t ) ) );
    if ( flag == min::MISSING() )
    {
	PAR::parse_error
	    ( parser,
	      i == 0 ? ppvec->position :
	               ppvec[i-1],
	      "expected flag name",
	      i == 0 ? " at beginning of" :
	               " after" );
	return -1;
    }

    min::phrase_position pp;
    pp.begin = ppvec[ibegin].begin;
    pp.end = ppvec[i-1].end;

    if ( is_subexpression && i != size )
	PAR::parse_error
	    ( parser, pp,
	      "extraneous stuff after" );

    return ::lookup_flag
        ( flag, name_table, parser, pp );
}

// Like scan_new_flags but has separate indicators to
// specify whether flag lists or flag modifier lists are
// allowed.  Assumes at least one is allowed.
//
static min::gen scan_new_flags
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  TAB::new_flags & new_flags,
	  TAB::name_table name_table,
	  PAR::parser parser,
	  bool allow_flag_list,
	  bool allow_flag_modifier_list )
{
    MIN_ASSERT (    allow_flag_list
                 || allow_flag_modifier_list );

    if ( i >= min::size_of ( vp ) )
        return min::FAILURE();

    min::obj_vec_ptr subvp ( vp[i] );

    if ( subvp == min::NULL_STUB )
        return min::FAILURE();

    min::attr_ptr subap ( subvp );
    min::locate ( subap, PAR::dot_initiator );
    if ( min::get ( subap ) != PAR::left_square )
        return min::FAILURE();
    min::locate ( subap, PAR::dot_terminator );
    if ( min::get ( subap ) != PAR::right_square )
        return min::FAILURE();
    min::locate ( subap, PAR::dot_separator );
    min::gen separator = min::get ( subap );
    min::locate ( subap, PAR::dot_position );
    min::phrase_position_vec ppvec = min::get ( subap );
    assert ( ppvec != min::NULL_STUB );
    assert ( ppvec->file == parser->input_file );

    ++ i;

    // From now on FAILURE not allowed, and failures
    // must be ERRORs with printed error message.

    new_flags.or_flags = 0;
    new_flags.not_flags = 0;
    new_flags.xor_flags = 0;
    min::unsptr size = min::size_of ( subvp );
    if ( size == 0 ) /* Do nothing */;
    else if ( separator == min::NONE() )
    {
        // Bracketted list has not been parsed.  We
	// parse it ourselves by calling ::scan_flag.

	min::uns32 i = 0;
	while ( true )
	{
	    min::gen op = min::MISSING();
	    int j = ::scan_flag
	        ( subvp, i, ppvec, op, name_table,
		  parser, allow_flag_list,
		  allow_flag_modifier_list );
	    if ( j == -1 ) return min::ERROR();
	    if ( op == min::MISSING() )
	        allow_flag_modifier_list = false;
	    else
	        allow_flag_list = false;

	    if ( op == PAR::minus )
		new_flags.not_flags |=
		    (min::uns64) 1 << j;
	    else if ( op == ::exclusive_or )
		new_flags.xor_flags |=
		    (min::uns64) 1 << j;
	    else
		new_flags.or_flags |=
		    (min::uns64) 1 << j;

	    if ( i == size ) break;

	    if ( subvp[i] != PAR::comma )
		return PAR::parse_error
		    ( parser, ppvec[i-1],
		      "expected `,' after" );
	    ++ i;
	}
    }
    else if ( separator == PAR::comma )
    {
        // Bracketted list has been parsed.  We process
	// elements of the list, using ::scan_flag on
	// any that are themselves lists.
        // 
	for ( min::uns32 i = 0; i < size; ++ i )
	{
	    min::obj_vec_ptr np ( subvp[i] );
	    int j;
	    min::gen op = min::MISSING();
	    if ( np == min::NULL_STUB )
	    {
	        // Found flag name as min string.

	        min::gen flag = subvp[i];

	        MIN_ASSERT ( min::is_str ( flag ) );

	        if ( ! allow_flag_list )
		    return PAR::parse_error
			( parser, ppvec[i],
			  "expected `+', `-', or `^'"
			  " before" );
		allow_flag_modifier_list = false;

		if (    LEXSTD::lexical_type_of
		            ( flag )
		     != LEXSTD::word_t
		     &&
		        LEXSTD::lexical_type_of
		            ( flag )
		     != LEXSTD::number_t )
		    return PAR::parse_error
			( parser, ppvec[i],
			  "flag name instead of" );
		j = ::lookup_flag
		    ( flag, name_table,
		      parser, ppvec[i] );
	    }
	    else
	    {
		min::phrase_position_vec pp =
		    min::position_of ( np );
		min::uns32 k = 0;
	        j = ::scan_flag
		    ( np, k, pp, op, name_table,
		      parser, allow_flag_list,
		      allow_flag_modifier_list,
		      true );
	    }

	    if ( j == -1 ) return min::ERROR();

	    if ( op == PAR::minus )
		new_flags.not_flags |=
		    (min::uns64) 1 << j;
	    else if ( op == ::exclusive_or )
		new_flags.xor_flags |=
		    (min::uns64) 1 << j;
	    else
		new_flags.or_flags |=
		    (min::uns64) 1 << j;
	}
    }
    else
	return PAR::parse_error
	    ( parser, ppvec->position,
	      "bad separator ",
	      min::pgen ( separator,
	                  min::BRACKET_STR_FLAG ),
	      " inside" );

    // Note that an empty list that could be either a
    // flag list or a flag modifier list is treated as
    // a flag modifier list.
    //
    if ( ! allow_flag_modifier_list )
        new_flags.not_flags = ~ new_flags.or_flags;

    return min::SUCCESS();
}

min::gen COM::scan_flags
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  TAB::flags & flags,
	  TAB::name_table name_table,
	  PAR::parser parser )
{
    TAB::new_flags new_flags;
    min::gen result = ::scan_new_flags
        ( vp, i, new_flags,
	  name_table, parser, true, false );
    flags = new_flags.or_flags;
    return result;
}

min::gen COM::scan_new_flags
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  TAB::new_flags & new_flags,
	  TAB::name_table name_table,
	  PAR::parser parser,
	  bool allow_flag_list )
{
    return ::scan_new_flags
        ( vp, i, new_flags,
	  name_table, parser, allow_flag_list, true );
}

// Execute Selectors
// ------- ---------

static min::gen execute_selectors
	( min::obj_vec_ptr & vp,
          min::phrase_position_vec ppvec,
	  PAR::parser parser )
{
    min::uns32 size = min::size_of ( vp );

    bool define;
    if ( vp[1] == PAR::print )
    {

	TAB::name_table t =
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
	    PAR::parse_warn
	        ( parser, ppvec[2],
		  "misspelled; should be `selectors'" );

        if ( size > 3 )
	    return PAR::parse_error
	        ( parser, ppvec[2],
		  "extraneous stuff after" );

	return min::SUCCESS();
    }
    else if ( vp[1] == PAR::define )
        define = true;
    else if ( vp[1] == PAR::undefine )
        define = false;
    else return min::FAILURE();

    min::uns32 i = 3;
    min::uns32 begini = i;
    min::locatable_gen name;
    name = COM::scan_simple_label
	( vp, i,
	    ( 1ull << LEXSTD::word_t )
	  + ( 1ull << LEXSTD::number_t ) );
    if ( name == min::MISSING() )
	return PAR::parse_error
	    ( parser, ppvec[i-1],
	      "expected simple name after" );

    int j = TAB::get_index
	( parser->selector_name_table, name );
    if ( j < 0 )
    {
	if ( parser->selector_name_table
		   ->length >= 64 )
	{
	    min::phrase_position pp;
	    pp.begin = ppvec[begini].begin;
	    pp.end = ppvec[i-1].end;
	    return PAR::parse_error
	        ( parser, pp,
		  "too many selector names;"
		  " cannot process ",
		  min::pgen ( name,
		              min::BRACKET_STR_FLAG ),
		  " in" );
	}

	TAB::push_name
	    ( parser->selector_name_table, name );
    }

    if ( vp[2] != PAR::selector )
	PAR::parse_warn
	    ( parser, ppvec[2],
	      "misspelled; should be `selector'" );

    if ( i < size )
        return PAR::parse_error
	    ( parser, ppvec[i-1],
	      "extraneous stuff after" );

    return min::SUCCESS();
}

// Execute Test
// ------- ----

static void execute_test_scan
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
	::execute_test_scan ( subvp, printer );
    }
}

static min::gen execute_test
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
    ::execute_test_scan
        ( vp, parser->printer );
    parser->printer << "======= END TEST" << min::eol;
    return min::SUCCESS();
}

// Execute Trace
// ------- -----

static min::gen execute_trace
	( min::obj_vec_ptr & vp,
          min::phrase_position_vec ppvec,
	  PAR::parser parser )
{
    if ( vp[1] == PAR::print )
    {

	TAB::name_table t =
	    parser->trace_flag_name_table;

	parser->printer
	    << "parser print trace: [ "
	    << min::bom << min::nohbreak;

	bool first = true;
	for ( unsigned j = 0; j < t->length; ++ j )
	{
	    if ( ( parser->trace_flags & ( 1ull << j ) )
	         ==
		 0 )
	        continue;

	    if ( first ) first = false;
	    else parser->printer << ", ";

	    parser->printer
		<< min::set_break
		<< min::pgen ( t[j] );
	}
	parser->printer << " ]" << min::eom;

        if ( min::size_of ( vp ) > 3 )
	    return PAR::parse_error
	        ( parser, ppvec[2],
		  "extraneous stuff after" );

	return min::SUCCESS();
    }

    TAB::new_flags new_flags;
    min::uns32 i = 2;
    min::gen result = COM::scan_new_flags
        ( vp, i, new_flags,
	  parser->trace_flag_name_table,
	  parser, true );
    if ( result == min::ERROR() )
        return min::ERROR();
    else if ( result == min::FAILURE() )
        return PAR::parse_error
	    ( parser, ppvec[1],
	      "expected bracketed flag (modifier) list"
	      " after" );

    parser->trace_flags |= new_flags.or_flags;
    parser->trace_flags &= ~ new_flags.not_flags;
    parser->trace_flags ^= new_flags.xor_flags;
    return min::SUCCESS();
}

// Execute Begin/End
// ------- ---------

static min::gen execute_begin
	( min::obj_vec_ptr & vp,
          min::phrase_position_vec ppvec,
	  PAR::parser parser )
{
    min::uns32 i = 2;
    min::locatable_gen name
        ( COM::scan_simple_label
	    ( vp, i, 
	        ( 1ull << LEXSTD::word_t )
	      + ( 1ull << LEXSTD::number_t ) ) );
    if ( name == min::MISSING() )
	return PAR::parse_error
	    ( parser,
	      ppvec[1],
	      "expected block name after" );
    if ( i != min::size_of ( vp ) )
        return PAR::parse_error
	    ( parser,
	      ppvec[i-1],
	      "extraneous stuff after" );

    return PAR::begin_block
    		( parser, name, ppvec->position );
}

static min::gen execute_end
	( min::obj_vec_ptr & vp,
          min::phrase_position_vec ppvec,
	  PAR::parser parser )
{
    min::uns32 i = 2;
    min::locatable_gen name
        ( COM::scan_simple_label
	    ( vp, i, 
	        ( 1ull << LEXSTD::word_t )
	      + ( 1ull << LEXSTD::number_t ) ) );
    if ( name == min::MISSING() )
	return PAR::parse_error
	    ( parser,
	      ppvec[1],
	      "expected block name after" );
    min::uns32 size = min::size_of ( vp );
    if ( i != size )
        return PAR::parse_error
	    ( parser,
	      ppvec[i-1],
	      "extraneous stuff after" );

    return PAR::end_block
    		( parser, name, ppvec->position );
}

// Execute Command
// ------- -------

min::gen COM::parser_execute_command
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

    min::gen result = min::FAILURE();
    bool call_all_passes = false;

    if ( vp[1] == ::test )
        return ::execute_test
	    ( vp, ppvec, parser );
    else if ( size >= 3
              &&
	      (    vp[2] == PAR::selector
	        || vp[2] == PAR::selectors ) )
	result = ::execute_selectors
		    ( vp, ppvec, parser );
    else if ( vp[1] == ::trace
	      &&
	      ( size == 2
	        ||
	        min::is_obj ( vp[2] ) ) )
	result = ::execute_trace
		    ( vp, ppvec, parser );
    else if ( size >= 3
              &&
	      vp[1] == PAR::print
	      &&
	      vp[2] == ::trace )
	result = ::execute_trace
		    ( vp, ppvec, parser );
    else if ( vp[1] == PAR::begin )
	result = ::execute_begin
		    ( vp, ppvec, parser );
    else if ( vp[1] == PAR::end )
	result = ::execute_end
		    ( vp, ppvec, parser );

    // If call_all_passes, set result to the highest
    // priority result where the order is:
    //
    //   (lowest) FAILURE, SUCCESS, ERROR (highest)
    //
    for ( PAR::pass pass = parser->pass_stack;
          ( result == min::FAILURE() || call_all_passes)
	  &&
	  pass != NULL;
	  pass = pass->next )
    {
        min::gen saved_result = result;
        if ( pass->parser_command != NULL )
	    result = (* pass->parser_command )
	        ( parser, pass, vp, ppvec );
	if ( saved_result == min::ERROR()
	     ||
	     result == min::FAILURE() )
	    result = saved_result;
    }

    if ( result == min::SUCCESS()
         &&
	 (   parser->trace_flags
	   & PAR::TRACE_PARSER_COMMANDS ) )
	min::print_phrase_lines
	    ( parser->printer,
	      ppvec->file, ppvec->position, 0 );

    return result;
}
