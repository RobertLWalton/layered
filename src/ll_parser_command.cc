// Layers Language Parser Execute Command Function
//
// File:	ll_parser_command.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Dec 31 03:54:19 EST 2013
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents:
//
//	Usage and Setup
//	Parser Command Functions
//	Execute Pass
//	Execute Selectors
//	Execute Context
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
static min::locatable_gen trace;
min::locatable_gen COM::PRINTED;

static void initialize ( void )
{
    ::exclusive_or = min::new_str_gen ( "^" );
    ::trace = min::new_str_gen ( "trace" );
    COM::PRINTED = min::new_special_gen ( 0 );
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

void COM::print_new_flags
	( const TAB::new_flags & new_flags,
	  TAB::name_table name_table,
	  PAR::parser parser,
	  bool allow_flag_list )
{
	min::uns64 all_flags = new_flags.or_flags
	                     | new_flags.not_flags
	                     | new_flags.xor_flags;

	if ( all_flags == 0 )
	{
	    parser->printer << "none";
	    return;
	}

	parser->printer << "["
	                << min::suppressible_space
	                << min::save_indent
	                << min::nohbreak;

	bool is_flag_list =
	    allow_flag_list
	    &&
	    new_flags.xor_flags == 0
	    &&
	    new_flags.or_flags == ~ new_flags.not_flags;
	min::uns64 suppress =
	    ( is_flag_list ?
	        new_flags.not_flags :
	        TAB::ALL_FLAGS & ~ all_flags );

	bool first = true;
	for ( min::unsptr i = 0;
	      i < name_table->length; ++ i )
	{
	    min::uns64 mask = 1ull << i;
	    if ( mask & suppress ) continue;

	    if ( ! first )
	        parser->printer << ", "
		                << min::set_break;
	    else
	        first = false;

	    if ( ! is_flag_list )
	    {
	        if ( mask & new_flags.or_flags )
		    parser->printer << "+";
	        if ( mask & new_flags.not_flags )
		    parser->printer << "-";
	        if ( mask & new_flags.xor_flags )
		    parser->printer << "^";
	        parser->printer << " ";
	    }

	    parser->printer << min::name_pgen
	                           ( name_table[i] );
	}

	parser->printer << min::suppressible_space
	                << "]"
	                << min::restore_indent;
}

void COM::print_flags
	( TAB::flags flags,
	  TAB::name_table name_table,
	  PAR::parser parser )
{
    TAB::new_flags new_flags;
    new_flags.or_flags = flags;
    new_flags.not_flags = ~ flags;
    new_flags.xor_flags = 0;
    COM::print_new_flags
        ( new_flags, name_table, parser, true );
} 

void COM::print_command
	( min::obj_vec_ptr & vp,
	  PAR::parser parser )
{

    min::uns32 size = min::size_of ( vp );

    for ( min::uns32 i = 0; i < size; ++ i )
    {
	parser->printer
	    << ( i == 0 ? "" : " " )
	    << min::pgen ( vp[i],
			   min::OBJ_EXP_FLAG );
    }
}

// Execute Pass
// ------- ----

static min::gen execute_pass
	( min::obj_vec_ptr & vp,
          min::phrase_position_vec ppvec,
	  PAR::parser parser )
{
    if ( vp[1] != PAR::define
         &&
	 vp[1] != PAR::undefine
         &&
	 vp[1] != PAR::print )
	return min::FAILURE();

    min::uns32 size = min::size_of ( vp );

    min::uns32 i = 3;
    min::locatable_gen name, name2;
    PAR::new_pass new_pass = NULL;
    TAB::flags selectors;
    min::phrase_position name_pp;
    if ( vp[1] != PAR::print )
    {
	min::uns32 begini = i;
	name = COM::scan_simple_label
	    ( vp, i,
		( 1ull << LEXSTD::word_t )
	      + ( 1ull << LEXSTD::number_t ) );
	if ( name == min::MISSING() )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "expected simple name after" );
	name_pp.begin = ppvec[begini].begin;
	name_pp.end = ppvec[i-1].end;

	new_pass = PAR::find_new_pass ( name );
	if ( new_pass == NULL )
	    return PAR::parse_error
		( parser, name_pp,
		  "is not a pass name" );
    }

    PAR::pass previous = min::NULL_STUB;
    PAR::pass next = min::NULL_STUB;
    if ( vp[1] == PAR::define )
    {
	min::gen result = COM::scan_flags
		    ( vp, i, selectors,
		      parser->selector_name_table,
		      parser );
	if ( result == min::ERROR() )
	    return result;
	else if ( result == min::FAILURE() )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "expected parsing selectors after" );

	if ( vp[i] == PAR::after
	     ||
	     vp[i] == PAR::before )
	{
	    ++ i;
	    min::uns32 begini = i;
	    name2 = COM::scan_simple_label
		( vp, i,
		    ( 1ull << LEXSTD::word_t )
		  + ( 1ull << LEXSTD::number_t ) );
	    if ( name2 == min::MISSING() )
		return PAR::parse_error
		    ( parser, ppvec[i-1],
		      "expected simple name after" );
	    min::phrase_position name2_pp;
	    name2_pp.begin = ppvec[begini].begin;
	    name2_pp.end = ppvec[i-1].end;

	    if ( PAR::find_new_pass ( name2 ) == NULL )
		return PAR::parse_error
		    ( parser, name2_pp,
		      "is not a pass name" );
	    PAR::pass pass2 =
		PAR::find_on_pass_stack
		    ( parser, name2 );
	    if ( pass2 == min::NULL_STUB )
		return PAR::parse_error
		    ( parser, name2_pp,
		      "is not on the pass stack" );

	    if ( vp[begini-1] == PAR::after )
	        previous = pass2;
	    else
	    {
	        if ( pass2 == parser->pass_stack )
		    return PAR::parse_error
			( parser, name2_pp,
			  "cannot put pass before"
			  " `top' pass" );
		next = pass2;
	    }
	}
	else if ( vp[i] == PAR::at
	          &&
		  vp[i+1] == PAR::end )
	    i += 2;  // previous == next == NULL_STUB
	else
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "expected `after', `before', or"
		  " `at end' after" );
    }

    if ( i < size )
        return PAR::parse_error
	    ( parser, ppvec[i-1],
	      "extraneous stuff after" );

    if ( vp[1] == PAR::print )
    {
        COM::print_command ( vp, parser );
	parser->printer
	    << ":" << min::eol
	    << min::bom << min::nohbreak
	    << min::set_indent ( 4 );

	PAR::pass pass = parser->pass_stack;
	if ( pass == min::NULL_STUB )
	    parser->printer << min::indent
	                    << "parser stack is empty";
	else while ( pass != min::NULL_STUB )
	{
	    parser->printer << min::indent
			    << min::name_pgen
				 ( pass->name );
	    pass = pass->next;
	}
	parser->printer << min::eom;

	return COM::PRINTED;
    }
    else if ( vp[1] == PAR::define )
    {
        PAR::pass pass =
	    PAR::find_on_pass_stack ( parser, name );
	if ( pass == parser->pass_stack )
	    return PAR::parse_error
		( parser, name_pp,
		  "cannot define `top' pass" );
	else if ( pass == min::NULL_STUB )
	    pass = (* new_pass)();
	else
	    PAR::remove ( pass );
	pass->selectors = selectors;
	if ( previous != min::NULL_STUB )
	    PAR::place_after ( parser, pass, previous );
	else
	    PAR::place_before ( parser, pass, next );
    }
    else /* if vp[1] == PAR::undefine */
    {
        PAR::pass pass =
	    PAR::find_on_pass_stack ( parser, name );
	if ( pass == parser->pass_stack )
	    return PAR::parse_error
		( parser, name_pp,
		  "cannot undefine `top' pass" );
	else if ( pass != min::NULL_STUB )
	    PAR::remove ( pass );
    }

    return min::SUCCESS();
}

// Execute Selectors
// ------- ---------

static min::gen execute_selectors
	( min::obj_vec_ptr & vp,
          min::phrase_position_vec ppvec,
	  PAR::parser parser )
{
    if ( vp[1] != PAR::define
         &&
	 vp[1] != PAR::print )
	return min::FAILURE();

    min::uns32 size = min::size_of ( vp );

    min::uns32 i = 3;
    min::uns32 begini = i;
    min::locatable_gen name;
    if ( vp[1] == PAR::define )
    {
	name = COM::scan_simple_label
	    ( vp, i,
		( 1ull << LEXSTD::word_t )
	      + ( 1ull << LEXSTD::number_t ) );
	if ( name == min::MISSING() )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "expected simple name after" );
    }
    else /* if vp[1] == PAR::print */
    {
	name = PAR::scan_name_string_label
	    ( vp, i, parser,

		( 1ull << LEXSTD::mark_t )
	      + ( 1ull << LEXSTD::separator_t )
	      + ( 1ull << LEXSTD::word_t )
	      + ( 1ull << LEXSTD::number_t ),

		( 1ull << LEXSTD::
			      horizontal_space_t )
	      + ( 1ull << LEXSTD::end_of_file_t ),

		( 1ull << LEXSTD::end_of_file_t ),
	      true /* empty name ok */ );

	if ( name == min::ERROR() )
	    return min::ERROR();
	else if ( name == min::MISSING() )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "expected quoted name after" );
    }

    if ( i < size )
        return PAR::parse_error
	    ( parser, ppvec[i-1],
	      "extraneous stuff after" );

    if ( vp[1] == PAR::define )
    {
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
		      " cannot process \"",
		      min::name_pgen ( name ),
		      "\" in" );
	    }

	    TAB::push_name
		( parser->selector_name_table, name );
	}
    }
    else /* if vp[1] == PAR::print */
    {
        COM::print_command ( vp, parser );
	parser->printer
	    << ":" << min::eol
	    << min::bom << min::nohbreak
	    << min::set_indent ( 4 );
	int count = 0;

	TAB::name_table t =
	    parser->selector_name_table;

	min::uns32 block_level =
	    PAR::block_level ( parser );
	for ( min::uns32 j = t->length; 0 < j --; )
	{
	    if (   min::is_subsequence ( name, t[j] )
	         < 0 )
	        continue;

	    while ( block_level > 0
	            &&
		    parser->block_stack[block_level-1]
		      .saved_selector_name_table_length
		    >= j )
	        -- block_level;

	    min::gen block_name =
	        PAR::block_name ( parser, block_level );

	    parser->printer << min::indent
			    << "block "
			    << min::name_pgen
				 ( block_name )
			    << ": "
		            << min::name_pgen ( t[j] );

	    ++ count;
	}

	if ( count == 0 )
	    parser->printer << min::indent
	                    << "not found";
	parser->printer << min::eom;

	return COM::PRINTED;
    }

    return min::SUCCESS();
}

// Execute Context
// ------- -------

static min::gen execute_context
	( min::obj_vec_ptr & vp,
          min::phrase_position_vec ppvec,
	  PAR::parser parser )
{
    min::uns32 size = min::size_of ( vp );
    assert ( size >= 3 );

    if (    vp[1] != PAR::define
         && vp[1] != PAR::print )
	return min::FAILURE();

    min::uns32 i = 3;
    min::locatable_gen name
	( PAR::scan_name_string_label
	    ( vp, i, parser,

		( 1ull << LEXSTD::mark_t )
	      + ( 1ull << LEXSTD::separator_t )
	      + ( 1ull << LEXSTD::word_t )
	      + ( 1ull << LEXSTD::number_t ),

		( 1ull << LEXSTD::
			      horizontal_space_t )
	      + ( 1ull << LEXSTD::end_of_file_t ),

		( 1ull << LEXSTD::end_of_file_t ),
		vp[1] == PAR::print ) );

    if ( name == min::ERROR() )
	return min::ERROR();
    else if ( name == min::MISSING() )
	return PAR::parse_error
	    ( parser, ppvec[i-1],
	      "expected quoted name after" );

    if ( vp[1] == PAR::print )
    {

	if ( i < size )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "extraneous stuff after" );

        COM::print_command ( vp, parser );
	parser->printer
	    << ":" << min::eol
	    << min::bom << min::nohbreak
	    << min::set_indent ( 4 );

	int count = 0;

	if ( min::is_subsequence
		 ( name, PAR::default_lexeme ) >= 0 )
	{
	    TAB::flags flags = parser->selectors;
	    for ( min::uns32 i =
	              parser->block_stack->length;
		  0 <= i; -- i )
	    {
	        min::gen block_name =
		    ( i == 0 ?
		      (min::gen) PAR::top_level :
		      parser->block_stack[i-1].name );

	        parser->printer << min::indent
		                << "block "
				<< min::name_pgen
				     ( block_name )
				<< ": \"default\" ";
		COM::print_flags
		    ( flags,
		      parser->selector_name_table,
		      parser );

		++ count;

		if ( i == 0 ) break;

		flags = parser->block_stack[i-1]
			    .saved_selectors;
	    }
	}

	{
	    TAB::key_table_iterator it
	        ( parser->context_table );
	    while ( true )
	    {
	        TAB::root root = it.next();
		if ( root == min::NULL_STUB ) break;
		PAR::context context =
		    (PAR::context) root;
		MIN_ASSERT
		    ( context != min::NULL_STUB );

		if ( min::is_subsequence
		         ( name, context->label ) < 0 )
		    continue;

		min::gen block_name =
		    PAR::block_name
			( parser,
			  context->block_level );
		parser->printer
		    << min::indent
		    << "block "
		    << min::name_pgen ( block_name )
		    << ": \""
		    << min::name_pgen ( context->label )
		    << "\" ";
		COM::print_new_flags
		    ( context->new_selectors,
		      parser->selector_name_table,
		      parser, true );

		++ count;
	    }
	}

	if ( count == 0 )
	    parser->printer << min::indent
			    << "not found";

	parser->printer << min::eom;
	return COM::PRINTED;
    }

    TAB::flags selectors;
    TAB::new_flags new_flags;
    if ( size >= i + 4
         &&
	 vp[i] == PAR::with
	 &&
	 vp[i+1] == PAR::parsing
	 &&
	 vp[i+2] == PAR::selectors )
    {
	i += 3;
	min::gen result;
	if ( name == PAR::default_lexeme )
	    result = COM::scan_flags
		( vp, i, selectors,
		  parser->selector_name_table,
		  parser );
	else
	    result = COM::scan_new_flags
		( vp, i, new_flags,
		  parser->selector_name_table,
		  parser, true );
	if ( result == min::ERROR() )
	    return min::ERROR();
	else if ( result == min::FAILURE() )
	    return PAR::parse_error
		( parser, ppvec[1],
		  "expected bracketed flag (modifier)"
		  " list after" );
    }
    else if ( name != PAR::default_lexeme )
    {
        new_flags.or_flags = 0;
        new_flags.not_flags = 0;
        new_flags.xor_flags = 0;
    }
    else
	return PAR::parse_error
	    ( parser, ppvec[i-1],
	      "expected `with parsing selectors'"
	      " after" );

    if ( name == PAR::default_lexeme )
        parser->selectors = selectors;
    else
    {
	min::phrase_position pp;
	pp.begin = ppvec[0].begin;
	pp.end = ppvec[i-1].end;

	PAR::push_context
	    ( name, 0, PAR::block_level ( parser ), pp,
	      new_flags, parser->context_table );
    }

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
    const min::uns64 TRACE_ANY =
	  PAR::TRACE_SUBEXPRESSION_ELEMENTS
	+ PAR::TRACE_SUBEXPRESSION_DETAILS
	+ PAR::TRACE_SUBEXPRESSION_LINES;
    const min::uns64 TRACE_E_OR_D =
	  PAR::TRACE_SUBEXPRESSION_ELEMENTS
	+ PAR::TRACE_SUBEXPRESSION_DETAILS;

    TAB::flags flags = parser->trace_flags;
    flags &= TRACE_ANY;
    if ( flags == 0 )
	flags = PAR::TRACE_SUBEXPRESSION_ELEMENTS;

    if ( flags & TRACE_E_OR_D )
    {
	min::gen obj =
	    min::new_stub_gen
	        ( (const min::stub *) vp );
	vp = min::NULL_STUB;
	    // Close vp so pgen can print obj.

	parser->printer
	    << min::save_print_format << min::nohbreak
	    << "======= TEST: "
	    << min::restore_print_format
	    << min::bom;

	if ( flags & PAR::TRACE_SUBEXPRESSION_ELEMENTS )
	    parser->printer
		<< min::indent_pgen ( obj )
		<< min::eom
		<< min::flush_id_map;

	if ( ( flags & TRACE_E_OR_D ) == TRACE_E_OR_D)
	    parser->printer
		<< min::indent
		<< min::bom;

	if ( flags & PAR::TRACE_SUBEXPRESSION_DETAILS )
	{
	    parser->printer
	        << min::set_context_gen_flags
			( & min::
			    no_exp_context_gen_flags )
		<< min::indent_pgen ( obj )
		<< min::eol
		<< min::flush_id_map
		<< min::eom;
	}

	vp = obj;  // Reopen vp.
    }

    if ( flags & PAR::TRACE_SUBEXPRESSION_LINES )
    {
        if ( ( flags & TRACE_E_OR_D ) == 0 )
	    parser->printer
	        << "======= TEST:" << min::eol;

	parser->printer
	    << "------- "
	    << min::bom
	    << min::pline_numbers
		   ( ppvec->file, ppvec->position )
	    << ":" << min::eom;

	min::print_phrase_lines
	    ( parser->printer,
	      ppvec->file, ppvec->position );
	::execute_test_scan
	    ( vp, parser->printer );

	parser->printer
	    << "======= END TEST" << min::eol;
    }

    return COM::PRINTED;
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
	    << "parser print trace:" << min::eol
	    << min::bom << min::nohbreak
	    << min::set_indent ( 4 ) << min::indent
	    << "["
	    << min::set_indent ( 6 ) << min::indent;

	TAB::flags trace_flags = parser->trace_flags;
	for ( unsigned j = 0; j < t->length; ++ j )
	{
	    if ( j > 0 ) parser->printer << ", ";

	    parser->printer
		<< min::set_break
	        << ( ( trace_flags & ( 1ull << j ) ) ?
		   	"+ " : "- " )
		<< min::name_pgen ( t[j] );
	}
	parser->printer << " ]" << min::eom;

        if ( min::size_of ( vp ) > 3 )
	    return PAR::parse_error
	        ( parser, ppvec[2],
		  "extraneous stuff after" );

	return COM::PRINTED;
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

    if ( vp[1] == PAR::test )
        result = ::execute_test
	    ( vp, ppvec, parser );
    else if ( size >= 3
              &&
	      vp[2] == PAR::pass_lexeme )
	result = ::execute_pass
		    ( vp, ppvec, parser );
    else if ( size >= 3
              &&
	      vp[2] == PAR::selector )
	result = ::execute_selectors
		    ( vp, ppvec, parser );
    else if ( size >= 3
              &&
	      vp[2] == PAR::context_lexeme )
	result = ::execute_context
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

    // As long as command is unrecognized (i.e.,
    // result == FAILURE) call pass parser_command
    // functions.
    //
    for ( PAR::pass pass = parser->pass_stack;
          result == min::FAILURE()
	  &&
	  pass != NULL;
	  pass = pass->next )
    {
        if ( pass->parser_command != NULL )
	    result = (* pass->parser_command )
	        ( parser, pass, vp, ppvec );
    }

    if ( result == min::SUCCESS()
         &&
	 (   parser->trace_flags
	   & PAR::TRACE_PARSER_COMMANDS ) )
	min::print_phrase_lines
	    ( parser->printer,
	      ppvec->file, ppvec->position, 0 );
    else if ( result == COM::PRINTED )
        result = min::SUCCESS();

    return result;
}
