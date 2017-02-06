// Layered Languages Parser Execute Command Function
//
// File:	ll_parser_command.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Feb  6 06:37:34 EST 2017
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
static min::locatable_gen top;

static void initialize ( void )
{
    ::exclusive_or = min::new_str_gen ( "^" );
    ::trace = min::new_str_gen ( "trace" );
    ::top = min::new_str_gen ( "top" );
}
static min::initializer initializer ( ::initialize );

// Parser Command Functions
// ------ ------- ---------

min::gen COM::scan_args
	( min::obj_vec_ptr & vp, min::uns32 & i,
          min::ref< min::packed_vec_ptr<min::gen> >
	      arg_vec,
	  ll::parser::parser parser )
{
    if ( i >= min::size_of ( vp ) )
        return min::FAILURE();

    min::obj_vec_ptr subvp ( vp[i] );

    if ( subvp == min::NULL_STUB )
        return min::FAILURE();

    min::attr_ptr subap ( subvp );
    min::locate ( subap, min::dot_initiator );
    if ( min::get ( subap ) != PAR::left_parenthesis )
        return min::FAILURE();
    min::locate ( subap, min::dot_terminator );
    if ( min::get ( subap ) != PAR::right_parenthesis )
        return min::FAILURE();
    min::locate ( subap, min::dot_position );
    min::phrase_position_vec ppvec = min::get ( subap );
    MIN_ASSERT ( ppvec != min::NULL_STUB,
                 ".position attribute missing" );
    MIN_ASSERT ( ppvec->file == parser->input_file,
                 ".position attribute value file is"
		 " not the same as parser input_file" );

    ++ i;

    min::packed_vec_insptr<min::gen> names =
        (const min::stub *)
	(min::packed_vec_ptr<min::gen>) arg_vec;
    if ( names == min::NULL_STUB )
    {
	names = 
	    min::gen_packed_vec_type.new_stub ( 10 );
	arg_vec = names;
    }
    else
        min::pop ( names, arg_vec->length );
    
    min::uns32 s = min::size_of ( subvp );
    min::uns32 j = 0;
    min::locatable_gen name;
    while ( j < s )
    {
        name = PAR::scan_name ( subvp, j, parser );

	if ( name == min::ERROR() ) return min::ERROR();

	min::push ( names ) = name;

	if ( j < s )
	{
	    if ( subvp[j] != PAR::comma )
		return PAR::parse_error
		    ( parser, ppvec[j-1],
		      "expected `,' after" );
	    ++ j;
	}
    }

    return min::SUCCESS();
}

// Scan a `flag-name' or an `operator flag-name' for
// scan_new_flags below.  Update index i, new_flags,
// group_new_flags, allow_flag_list, and allow_flag_
// modifier_list, and return true if no error.  If
// error, including flag name not being in tables,
// print error message and return false.
//
// ppvec is position of vp object, and ppvec->file
// == parser->input_file is required.
//
// If the flag name is found in the group_name_table,
// the associated flag set is OR'ed into group_new_
// flags.  If flag name is not found in the group_name_
// table, but is found in the name_table, the flag is
// OR'ed into new_flags.  The group_name_table may be
// NULL_STUB, indicating that it is empty.
//
// If the flag is preceded by an operator, +, -, or ^,
// then if allow_flag_modifier_list is true, the
// found flag set is OR'ed into the associated component
// of new_flags or group_new_flags, i.e., into the
// or_flags, not_flags, or xor_flags component, and
// allow_flag_list is set to false.  It is an error if
// there is an operator and allow_flag_modifier_list
// is false.
//
// If the flag is NOT preceeded by an operator, then if
// allows_flag_list is true, the found flag set is
// OR'ed into the or_flags component, and allow_flag_
// modifier_list is set to false.  Is it an error if
// there is NO operator and allows_flag_list is false.
//
// It is an error if a named flag is not in allowed_
// flags, but flags in named flag group need not be
// in allowed flags.
//
// On errors an error message is printed and false
// is returned.  Otherwise true is returned.
//
static bool scan_flag
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  min::phrase_position_vec ppvec,
	  TAB::new_flags & new_flags,
	  TAB::new_flags & group_new_flags,
	  TAB::flags allowed_flags,
	  bool & allow_flag_list,
	  bool & allow_flag_modifier_list,
	  TAB::name_table name_table,
	  TAB::key_table group_name_table,
	  PAR::parser parser )
{
    min::gen op = min::MISSING();
    min::unsptr size = min::size_of ( vp );

    MIN_REQUIRE ( ppvec->file == parser->input_file );

    if ( i >= size )
    {
	PAR::parse_error
	    ( parser,
	      i == 0 ? ppvec->position :
	               ppvec[i-1],
	      ! allow_flag_list ?
	          "expected `+', `-', `^" :
	      ! allow_flag_modifier_list ?
	          "expected flag/option name" :
	          "expected `+', `-', `^',"
		  " or flag/option name",
	      min::pnop,
	      i == 0 ? " at beginning of" :
	               " after" );
	return false;
    }

    if (    vp[i] == PAR::plus
         || vp[i] == PAR::minus
	 || vp[i] == ::exclusive_or )
        op = vp[i], ++ i;

    if (    ! allow_flag_modifier_list
         && op != min::MISSING() )
    {
	PAR::parse_error
	    ( parser,
	      i == 1 ? ppvec->position :
	               ppvec[i-2],
	      "expected flag/option name",
	      min::pnop,
	      i == 1 ? " at beginning of" :
	                "after" );
	return false;
    }
    else if (    ! allow_flag_list
              && op == min::MISSING() )
    {
	PAR::parse_error
	    ( parser,
	      i == 0 ? ppvec->position :
	               ppvec[i-1],
	      "expected `+', `-', `^",
	      min::pnop,
	      i == 0 ? " at beginning of" :
	               " after" );
	return false;
    }
    else if ( op == min::MISSING() )
        allow_flag_modifier_list = false;
    else
        allow_flag_list = false;

    min::uns32 ibegin = i;
    min::locatable_gen flag_name
        ( PAR::scan_simple_name ( vp, i ) );
    if ( flag_name == min::MISSING() )
    {
	PAR::parse_error
	    ( parser,
	      i == 0 ? ppvec->position :
	               ppvec[i-1],
	      "expected flag/option name",
	      min::pnop,
	      i == 0 ? " at beginning of" :
	               " after" );
	return false;
    }

    min::phrase_position position;
    position.begin = (&ppvec[ibegin])->begin;
    position.end = (&ppvec[i-1])->end;

    TAB::new_flags * p = NULL;
    TAB::flags f;
    if ( group_name_table != min::NULL_STUB )
    {
        TAB::key_prefix key_prefix =
	    TAB::find_key_prefix
	        ( flag_name, group_name_table );
        if ( key_prefix != min::NULL_STUB
	     &&
	     key_prefix->first != min::NULL_STUB )
	{
	    f = key_prefix->first->selectors;
	    p = & group_new_flags;
	}
    }

    if ( p == NULL )
    {
	int j = TAB::get_index
	            ( name_table, flag_name );

	if ( j == -1 )
	{
	    PAR::parse_error
		( parser, position,
		  "unrecognized flag/option name" );
	    return false;
	}
	f = 1ull << j;
	p = & new_flags;
    }

    if ( ( f & allowed_flags ) != f )
    {
	PAR::parse_error
	    ( parser, position,
	      "flag/option name not recognized in this"
	      " context" );
	return false;
    }

    if ( op == PAR::minus )
        p->not_flags |= f;
    else if ( op == ::exclusive_or )
        p->xor_flags |= f;
    else
        p->or_flags |= f;

    return true;
}

// Like scan_new_flags but has separate indicators to
// specify whether flag lists or flag modifier lists are
// allowed.  Assumes at least one is allowed.
//
// Also, if flag list is given, sets not_flags =
// ~ or_flags & allowed_flags.
//
static min::gen scan_new_flags
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  TAB::new_flags & new_flags,
	  TAB::flags allowed_flags,
	  TAB::name_table name_table,
	  TAB::key_table group_name_table,
	  PAR::parser parser,
	  bool allow_flag_list,
	  bool allow_flag_modifier_list )
{
    MIN_ASSERT (    allow_flag_list
                 || allow_flag_modifier_list,
		 "allow_flag_list and allow_flag_"
		 "modifier_list arguments both false" );

    if ( i >= min::size_of ( vp ) )
        return min::FAILURE();

    min::obj_vec_ptr subvp ( vp[i] );

    if ( subvp == min::NULL_STUB )
        return min::FAILURE();

    min::attr_ptr subap ( subvp );
    min::locate ( subap, min::dot_initiator );
    if ( min::get ( subap ) != PAR::left_square )
        return min::FAILURE();
    min::locate ( subap, min::dot_terminator );
    if ( min::get ( subap ) != PAR::right_square )
        return min::FAILURE();
    min::locate ( subap, min::dot_position );
    min::phrase_position_vec ppvec = min::get ( subap );
    MIN_ASSERT ( ppvec != min::NULL_STUB,
                 ".position attribute missing" );
    MIN_ASSERT ( ppvec->file == parser->input_file,
                 ".position attribute value file is"
		 " not the same as parser input_file" );

    ++ i;

    // From now on FAILURE not allowed, and failures
    // must be ERRORs with printed error message.

    new_flags.or_flags = 0;
    new_flags.not_flags = 0;
    new_flags.xor_flags = 0;
    TAB::new_flags new_group_flags;

    min::unsptr size = min::size_of ( subvp );
    if ( size != 0 )
    {
        // Bracketted list which not been parsed.  We
	// parse it ourselves by calling ::scan_flag.

	min::uns32 i = 0;
	while ( true )
	{
	    if ( ! ::scan_flag
		       ( subvp, i, ppvec,
		         new_flags, new_group_flags,
			 allowed_flags,
		         allow_flag_list,
		         allow_flag_modifier_list,
		         name_table, group_name_table,
		         parser ) )
		return min::ERROR();

	    if ( i == size ) break;

	    if ( subvp[i] != PAR::comma )
		return PAR::parse_error
		    ( parser, ppvec[i-1],
		      "expected `,' after" );
	    ++ i;
	}
    }

    TAB::flags mask =
    	  new_flags.or_flags
	| new_flags.not_flags
	| new_flags.xor_flags;

    mask = ~ mask & allowed_flags;

    new_flags.or_flags |=
        new_group_flags.or_flags & mask;
    new_flags.not_flags |=
        new_group_flags.not_flags & mask;
    new_flags.xor_flags |=
        new_group_flags.xor_flags & mask;

    // Note that an empty list that could be either a
    // flag list or a flag modifier list is treated as
    // a flag list.
    //
    if ( allow_flag_list )
        new_flags.not_flags =
	   (  ~ new_flags.or_flags & allowed_flags );

    return min::SUCCESS();
}

min::gen COM::scan_flags
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  TAB::flags & flags,
	  TAB::flags allowed_flags,
	  TAB::name_table name_table,
	  TAB::key_table group_name_table,
	  PAR::parser parser )
{
    TAB::new_flags new_flags;
    min::gen result = ::scan_new_flags
        ( vp, i, new_flags, allowed_flags,
	  name_table, group_name_table,
	  parser, true, false );
    flags = new_flags.or_flags;
    return result;
}

min::gen COM::scan_new_flags
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  TAB::new_flags & new_flags,
	  TAB::flags allowed_flags,
	  TAB::name_table name_table,
	  TAB::key_table group_name_table,
	  PAR::parser parser,
	  bool allow_flag_list )
{
    return ::scan_new_flags
        ( vp, i, new_flags, allowed_flags,
	  name_table, group_name_table,
	  parser, allow_flag_list, true );
}

void COM::print_new_flags
	( const TAB::new_flags & new_flags,
	  TAB::flags allowed_flags,
	  TAB::name_table name_table,
	  PAR::parser parser,
	  bool allow_flag_list )
{
	TAB::new_flags nf;
	nf.or_flags =
	    new_flags.or_flags & allowed_flags;
	nf.not_flags =
	    new_flags.not_flags & allowed_flags;
	nf.xor_flags =
	    new_flags.xor_flags & allowed_flags;

	TAB::flags all_flags = nf.or_flags
	                     | nf.not_flags
	                     | nf.xor_flags;

	parser->printer << "["
	                << min::save_indent
	                << min::no_auto_break;

	bool is_flag_list =
	    allow_flag_list
	    &&
	    nf.xor_flags == 0
	    &&
	       nf.or_flags
	    == ( ~ nf.not_flags & allowed_flags );
	TAB::flags suppress =
	    ( is_flag_list ?
	        nf.not_flags | ~ allowed_flags :
	        ~ all_flags );

	bool first = true;
	for ( min::unsptr i = 0;
	      i < name_table->length; ++ i )
	{
	    if ( name_table[i] == min::MISSING() )
	        continue;

	    TAB::flags mask = 1ull << i;
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

	    parser->printer << min::pgen_name
	                           ( name_table[i] );
	}

	parser->printer << "]" << min::restore_indent;
}

void COM::print_flags
	( TAB::flags flags,
	  TAB::flags allowed_flags,
	  TAB::name_table name_table,
	  PAR::parser parser )
{
    TAB::new_flags new_flags;
    new_flags.or_flags = flags;
    new_flags.not_flags = ~ flags;
    new_flags.xor_flags = 0;
    COM::print_new_flags
        ( new_flags, allowed_flags,
	  name_table, parser, true );
} 

void COM::print_command
	( min::obj_vec_ptr & vp,
	  PAR::parser parser )
{

    min::uns32 size = min::size_of ( vp );

    parser->printer << min::indent;
    for ( min::uns32 i = 0; i < size; ++ i )
    {
	parser->printer
	    << ( i == 0 ? "" : " " )
	    << min::pgen ( vp[i] );
    }
}

// Execute Pass
// ------- ----

static min::gen execute_pass
	( min::obj_vec_ptr & vp, min::uns32 i0,
          min::phrase_position_vec ppvec,
	  PAR::parser parser )
{
    if ( vp[i0] != PAR::define
         &&
	 vp[i0] != PAR::undefine
         &&
	 vp[i0] != PAR::print )
	return min::FAILURE();

    min::uns32 size = min::size_of ( vp );

    min::uns32 i = i0 + 2;
    min::locatable_gen name, name2;
    PAR::new_pass new_pass = NULL;
    TAB::flags selectors;
    min::phrase_position name_pp;
    if ( vp[i0] != PAR::print )
    {
	min::uns32 begini = i;
	name = PAR::scan_simple_name ( vp, i );
	if ( name == min::MISSING() )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "expected simple name after" );
	name_pp.begin = (&ppvec[begini])->begin;
	name_pp.end = (&ppvec[i-1])->end;

	new_pass = PAR::find_new_pass ( name );
	if ( new_pass == NULL && name != ::top )
	    return PAR::parse_error
		( parser, name_pp,
		  "is not a pass name" );
    }

    PAR::pass previous = min::NULL_STUB;
    PAR::pass next = min::NULL_STUB;
    if ( vp[i0] == PAR::define )
    {
	min::gen result = COM::scan_flags
		    ( vp, i, selectors,
		      PAR::COMMAND_SELECTORS,
		      parser->selector_name_table,
		      parser->selector_group_name_table,
		      parser );
	if ( result == min::ERROR() )
	    return result;
	else if ( result == min::FAILURE() )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "expected parsing selectors after" );
	else MIN_REQUIRE ( result == min::SUCCESS() );

	if ( vp[i] == PAR::after
	     ||
	     vp[i] == PAR::before )
	{
	    ++ i;
	    min::uns32 begini = i;
	    name2 = PAR::scan_simple_name ( vp, i );
	    if ( name2 == min::MISSING() )
		return PAR::parse_error
		    ( parser, ppvec[i-1],
		      "expected simple name after" );
	    min::phrase_position name2_pp;
	    name2_pp.begin = (&ppvec[begini])->begin;
	    name2_pp.end = (&ppvec[i-1])->end;

	    if (    PAR::find_new_pass ( name2 ) == NULL
	         && name2 != ::top )
		return PAR::parse_error
		    ( parser, name2_pp,
		      "is not a pass name" );
	    if ( name2 == name )
		return PAR::parse_error
		    ( parser, name2_pp,
		      "a pass cannot be installed"
		      " before or after itself" );
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

    if ( vp[i0] == PAR::print )
    {
        COM::print_command ( vp, parser );
	parser->printer
	    << ":" << min::indent
	    << min::bom << min::no_auto_break
	    << min::adjust_indent ( 4 );

	PAR::pass pass = parser->pass_stack;
	if ( pass == min::NULL_STUB )
	    parser->printer << min::indent
	                    << "parser stack is empty";
	else while ( pass != min::NULL_STUB )
	{
	    parser->printer << min::indent
			    << min::pgen_name
				 ( pass->name );

	    if ( pass != parser->pass_stack )
	    {
		parser->printer << " ";
		COM::print_flags
		    ( pass->selectors,
		      PAR::COMMAND_SELECTORS,
		      parser->selector_name_table,
		      parser );
	    }

	    pass = pass->next;
	}
	parser->printer << min::eom;

	return PAR::PRINTED;
    }
    else if ( vp[i0] == PAR::define )
    {
        PAR::pass pass =
	    PAR::find_on_pass_stack ( parser, name );
	if ( pass == parser->pass_stack )
	    return PAR::parse_error
		( parser, name_pp,
		  "cannot define `top' pass" );
	else if ( pass == min::NULL_STUB )
	    pass = (* new_pass) ( parser );
	else
	    PAR::remove ( pass );
	pass->selectors = selectors;
	if ( previous != min::NULL_STUB )
	    PAR::place_after ( parser, pass, previous );
	else
	    PAR::place_before ( parser, pass, next );
    }
    else /* if vp[i0] == PAR::undefine */
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
	( min::obj_vec_ptr & vp, min::uns32 i0,
          min::phrase_position_vec ppvec,
	  PAR::parser parser )
{
    if ( vp[i0] != PAR::define
         &&
	 vp[i0] != PAR::print )
	return min::FAILURE();

    min::uns32 size = min::size_of ( vp );

    min::uns32 i = i0 + 2;
    min::uns32 begini = i;
    min::locatable_gen name;
    if ( vp[i0] == PAR::define )
    {
	name = PAR::scan_simple_name ( vp, i );
	if ( name == min::MISSING() )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "expected simple name after" );
    }
    else /* if vp[i0] == PAR::print */
    {
	name = PAR::scan_quoted_key
		   ( vp, i, parser,
		     true /* empty name OK */ );

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

    if ( vp[i0] == PAR::define )
    {
	int j = TAB::get_index
	    ( parser->selector_name_table, name );
	if ( j < 0 )
	{
	    if ( parser->selector_name_table
		       ->length >= 64 )
	    {
		min::phrase_position pp;
		pp.begin = (&ppvec[begini])->begin;
		pp.end = (&ppvec[i-1])->end;
		return PAR::parse_error
		    ( parser, pp,
		      "too many selector names;"
		      " cannot process ",
		      min::pgen_quote ( name ),
		      " in" );
	    }

	    TAB::push_name
		( parser->selector_name_table, name );
	}
    }
    else /* if vp[i0] == PAR::print */
    {
        COM::print_command ( vp, parser );
	parser->printer
	    << ":" << min::indent
	    << min::bom << min::no_auto_break
	    << min::adjust_indent ( 4 );
	int count = 0;

	TAB::name_table t =
	    parser->selector_name_table;

	min::uns32 block_level =
	    PAR::block_level ( parser );
	for ( min::uns32 j = t->length; 17 < j --; )
	    // Ignore 1ull << 16 == ALWAYS_SELECTOR
	{
	    if (   min::is_subsequence ( name, t[j] )
	         < 0 )
	        continue;

	    while ( block_level > 0
	            &&
		    (&parser->block_stack
		          [block_level-1])
		      ->saved_selector_name_table_length
		    > j )
	        -- block_level;

	    min::gen block_name =
	        PAR::block_name ( parser, block_level );

	    parser->printer << min::indent
			    << "block "
			    << min::pgen_name
				 ( block_name )
			    << ": "
		            << min::pgen_name ( t[j] );

	    ++ count;
	}

	if ( count == 0 )
	    parser->printer << min::indent
	                    << "not found";
	parser->printer << min::eom;

	return PAR::PRINTED;
    }

    return min::SUCCESS();
}

// Execute Context
// ------- -------

static min::gen execute_context
	( min::obj_vec_ptr & vp, min::uns32 i0,
          min::phrase_position_vec ppvec,
	  PAR::parser parser )
{
    min::uns32 size = min::size_of ( vp );
    MIN_REQUIRE ( size >= i0 + 2 );

    if (    vp[i0] != PAR::define
         && vp[i0] != PAR::print )
	return min::FAILURE();

    min::uns32 i = i0 + 2;
    min::locatable_gen name
	( PAR::scan_quoted_key
	    ( vp, i, parser, vp[i0] == PAR::print ) );

    if ( name == min::ERROR() )
	return min::ERROR();
    else if ( name == min::MISSING() )
	return PAR::parse_error
	    ( parser, ppvec[i-1],
	      "expected quoted name after" );

    if ( vp[i0] == PAR::print )
    {

	if ( i < size )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "extraneous stuff after" );

        COM::print_command ( vp, parser );
	parser->printer
	    << ":" << min::indent
	    << min::bom << min::no_auto_break
	    << min::adjust_indent ( 4 );

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
		      (&parser->block_stack[i-1])
		          ->name );

	        parser->printer << min::indent
		                << "block "
				<< min::pgen_name
				     ( block_name )
				<< ": default "
				<< min::save_indent;


		COM::print_flags
		    ( flags,
		      PAR::COMMAND_SELECTORS,
		      parser->selector_name_table,
		      parser );

		parser->printer
		    << " "
		    << min::set_break
		    << "options ";
		COM::print_flags
		    ( flags,
		      PAR::ALL_OPT,
		      parser->selector_name_table,
		      parser );

		parser->printer << min::restore_indent;

		++ count;

		if ( i == 0 ) break;

		flags = (&parser->block_stack[i-1])
			    ->saved_selectors;
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
		MIN_REQUIRE
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
		    << min::pgen_name ( block_name )
		    << ": "
		    << min::pgen_name ( context->label )
		    << " "
		    << min::save_indent;

		COM::print_new_flags
		    ( context->new_selectors,
		      PAR::COMMAND_SELECTORS,
		      parser->selector_name_table,
		      parser, true );

		if ( TAB::all_flags
		         ( context->new_selectors )
		     &
		     PAR::ALL_OPT )
		{
		    parser->printer
		        << " "
			<< min::set_break
		        << "options ";
		    COM::print_new_flags
			( context->new_selectors,
			  PAR::ALL_OPT,
			  parser->selector_name_table,
			  parser, true );
		}
		parser->printer << min::restore_indent;

		++ count;
	    }
	}

	if ( count == 0 )
	    parser->printer << min::indent
			    << "not found";

	parser->printer << min::eom;
	return PAR::PRINTED;
    }

    TAB::new_flags new_selectors;
    TAB::new_flags new_options;
	// Inited to zeroes.
    TAB::flags selectors = 0;
    TAB::flags options = 0;
    min::uns32 saved_i = i;
    while ( i < size && vp[i] == PAR::with )
    {
	++ i;
	if ( i + 1 < size
	     &&
	     vp[i] == PAR::parsing
	     &&
	     vp[i+1] == PAR::selectors )
	{
	    i += 2;
	    min::gen result;
	    if ( name == PAR::default_lexeme )
		result = COM::scan_flags
		    ( vp, i, selectors,
		      PAR::COMMAND_SELECTORS,
		      parser->selector_name_table,
		      parser->selector_group_name_table,
		      parser );
	    else
		result = COM::scan_new_flags
		    ( vp, i, new_selectors,
		      PAR::COMMAND_SELECTORS,
		      parser->selector_name_table,
		      parser->selector_group_name_table,
		      parser, true );
	    if ( result == min::ERROR() )
		return min::ERROR();
	    else if ( result == min::FAILURE() )
		return PAR::parse_error
		    ( parser, ppvec[i-1],
		      "expected bracketed selector"
		      " (modifier) list after" );
	    else MIN_REQUIRE
	             ( result == min::SUCCESS() );
	}
	else
	if ( i + 1 < size
	     &&
	     vp[i] == PAR::parsing
	     &&
	     vp[i+1] == PAR::options )
	{
	    i += 2;
	    min::gen result;
	    if ( name == PAR::default_lexeme )
		result = COM::scan_flags
		    ( vp, i, options,
		      PAR::ALL_OPT,
		      parser->selector_name_table,
		      parser->selector_group_name_table,
		      parser );
	    else
		result = COM::scan_new_flags
		    ( vp, i, new_options,
		      PAR::ALL_OPT,
		      parser->selector_name_table,
		      parser->selector_group_name_table,
		      parser, true );
	    if ( result == min::ERROR() )
		return min::ERROR();
	    else if ( result == min::FAILURE() )
		return PAR::parse_error
		    ( parser, ppvec[i-1],
		      "expected bracketed options"
		      " (modifier) list after" );
	    else MIN_REQUIRE
	    	     ( result == min::SUCCESS() );
	}
	else
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "expected `parsing selectors'"
		  " or `parsing options' after" );
    }

    if ( i == saved_i )
	return PAR::parse_error
	    ( parser, ppvec[i-1],
	      "expected `with' after" );

    if ( name == PAR::default_lexeme )
        parser->selectors = selectors | options;
    else
    {
	min::phrase_position pp;
	pp.begin = (&ppvec[0])->begin;
	pp.end = (&ppvec[i-1])->end;
	new_selectors.or_flags |=
	    new_options.or_flags;
	new_selectors.not_flags |=
	    new_options.not_flags;
	new_selectors.xor_flags |=
	    new_options.xor_flags;

	PAR::push_context
	    ( name, 0, PAR::block_level ( parser ), pp,
	      new_selectors, parser->context_table );
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
	    << min::bom
	    << min::adjust_indent ( 8 )
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

    if ( flags & TRACE_ANY )
	parser->printer
	    << min::bol
	    << min::save_print_format
	    << min::no_auto_break
	    << "======= TEST: "
	    << min::restore_print_format;

    if ( flags & TRACE_E_OR_D )
    {
	min::gen obj =
	    min::new_stub_gen
	        ( (const min::stub *) vp );
	vp = min::NULL_STUB;
	    // Close vp so pgen can print obj.

	if ( flags & PAR::TRACE_SUBEXPRESSION_ELEMENTS )
	    parser->printer
	        << min::bom
		<< min::adjust_indent ( 4 )
		<< min::set_gen_format
		   ( parser->subexpression_gen_format )
		<< min::pgen ( obj )
		<< min::eom
		<< min::flush_id_map;

	if ( flags & PAR::TRACE_SUBEXPRESSION_DETAILS )
	    min::print_mapped ( parser->printer,  obj );

	vp = obj;  // Reopen vp.
    }

    if ( flags & PAR::TRACE_SUBEXPRESSION_LINES )
    {
	parser->printer
	    << min::bol
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
    }

    if ( flags & TRACE_ANY )
	parser->printer
	    << "======= END TEST" << min::eol;
    return PAR::PRINTED;
}

void COM::parser_test_execute_command
	( PAR::parser parser,
	  min::gen indented_paragraph )
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

    min::obj_vec_ptr vp ( indented_paragraph );
    min::phrase_position_vec ppvec =
        min::position_of ( vp );
    MIN_ASSERT ( ppvec != min::NULL_STUB,
                 "missing .position attribute" );

    min::uns32 size = min::size_of ( vp );
    if ( size == 0 ) return;

    for ( min::uns32 i = 0; i < size; ++ i )
    {

	if ( ( flags & TRACE_E_OR_D ) || i == 0 )
	    parser->printer
		<< min::bol
		<< min::save_print_format
		<< min::no_auto_break
		<< "======= PARSER TEST: "
		<< min::restore_print_format;

	if ( flags & PAR::TRACE_SUBEXPRESSION_ELEMENTS )
	    parser->printer
		<< min::bom
		<< min::adjust_indent ( 4 )
		<< min::set_gen_format
		   ( parser->subexpression_gen_format )
		<< min::pgen ( vp[i] )
		<< min::eom
		<< min::flush_id_map;

	if (   flags
	     & PAR::TRACE_SUBEXPRESSION_DETAILS )
	    min::print_mapped
		( parser->printer,  vp[i] );

	if ( flags & PAR::TRACE_SUBEXPRESSION_LINES )
	{
	    min::obj_vec_ptr subvp ( vp[i] );
	    if ( subvp == min::NULL_STUB ) continue;

	    parser->printer
		<< min::bol
		<< "======= "
		<< min::bom
		<< min::pline_numbers
		       ( ppvec->file, ppvec[i] )
		<< ":" << min::eom;

	    min::print_phrase_lines
		( parser->printer,
		  ppvec->file, ppvec[i] );

	    ::execute_test_scan
		( subvp, parser->printer );
	}
    }

    parser->printer
	<< "======= END PARSER TEST(S)" << min::eol;
}

// Execute Trace
// ------- -----

static min::gen execute_trace
	( min::obj_vec_ptr & vp, min::uns32 i0,
          min::phrase_position_vec ppvec,
	  PAR::parser parser )
{
    if ( vp[i0] == PAR::print )
    {

	TAB::name_table t =
	    parser->trace_flag_name_table;

	parser->printer
	    << "parser print trace:" << min::indent
	    << min::bom << min::no_auto_break
	    << min::adjust_indent ( 4 ) << min::indent
	    << "["
	    << min::adjust_indent ( 2 ) << min::indent;

	TAB::flags trace_flags = parser->trace_flags;
	for ( unsigned j = 0; j < t->length; ++ j )
	{
	    if ( j > 0 ) parser->printer << ", ";

	    parser->printer
		<< min::set_break
	        << ( ( trace_flags & ( 1ull << j ) ) ?
		   	"+ " : "- " )
		<< min::pgen_name ( t[j] );
	}
	parser->printer << " ]" << min::eom;

        if ( min::size_of ( vp ) > i0 + 2 )
	    return PAR::parse_error
	        ( parser, ppvec[i0 + 1],
		  "extraneous stuff after" );

	return PAR::PRINTED;
    }

    TAB::new_flags new_flags;
    min::uns32 i = i0 + 1;
    min::gen result = COM::scan_new_flags
        ( vp, i, new_flags, TAB::ALL_FLAGS,
	  parser->trace_flag_name_table,
	  min::NULL_STUB,
	  parser, true );
    if ( result == min::ERROR() )
        return min::ERROR();
    else if ( result == min::FAILURE() )
        return PAR::parse_error
	    ( parser, ppvec[i0],
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
	( min::obj_vec_ptr & vp, min::uns32 i0,
          min::phrase_position_vec ppvec,
	  PAR::parser parser )
{
    min::uns32 i = i0 + 1;
    min::locatable_gen name
        ( PAR::scan_simple_name ( vp, i ) );
    if ( name == min::MISSING() )
	return PAR::parse_error
	    ( parser,
	      ppvec[i0],
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
	( min::obj_vec_ptr & vp, min::uns32 i0,
          min::phrase_position_vec ppvec,
	  PAR::parser parser )
{
    min::uns32 i = i0 + 1;
    min::locatable_gen name
        ( PAR::scan_simple_name ( vp, i ) );
    if ( name == min::MISSING() )
	return PAR::parse_error
	    ( parser,
	      ppvec[i0],
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
	( min::obj_vec_ptr & vp, min::uns32 i0,
	  PAR::parser parser )
{
    min::uns32 size = min::size_of ( vp );
    if ( size < i0 + 1 ) return min::FAILURE();

    min::phrase_position_vec ppvec =
        min::position_of ( vp );
    MIN_ASSERT ( ppvec != min::NULL_STUB,
                 "missing .position attribute" );

    min::gen result = min::FAILURE();

    parser->printer << min::bol << min::bom;
    if ( vp[i0] == PAR::test )
        result = ::execute_test
	    ( vp, ppvec, parser );
    else if ( size >= i0 + 2
              &&
	      vp[i0+1] == PAR::pass_lexeme )
	result = ::execute_pass
		    ( vp, i0, ppvec, parser );
    else if ( size >= i0 + 2
              &&
	      vp[i0+1] == PAR::selector )
	result = ::execute_selectors
		    ( vp, i0, ppvec, parser );
    else if ( size >= i0 + 2
              &&
	      vp[i0+1] == PAR::context_lexeme )
	result = ::execute_context
		    ( vp, i0, ppvec, parser );
    else if ( vp[i0] == ::trace
	      &&
	      ( size == i0 + 1
	        ||
	        min::is_obj ( vp[i0+1] ) ) )
	result = ::execute_trace
		    ( vp, i0, ppvec, parser );
    else if ( size >= i0 + 2
              &&
	      vp[i0] == PAR::print
	      &&
	      vp[i0+1] == ::trace )
	result = ::execute_trace
		    ( vp, i0, ppvec, parser );
    else if ( vp[i0] == PAR::begin )
	result = ::execute_begin
		    ( vp, i0, ppvec, parser );
    else if ( vp[i0] == PAR::end )
	result = ::execute_end
		    ( vp, i0, ppvec, parser );

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
	        ( parser, pass, vp, i0, ppvec );
    }

    if ( result == min::SUCCESS()
         &&
	 (   parser->trace_flags
	   & PAR::TRACE_PARSER_COMMANDS ) )
	min::print_phrase_lines
	    ( parser->printer,
	      ppvec->file, ppvec->position, 0 );
    else if ( result == PAR::PRINTED )
        result = min::SUCCESS();

    else if ( result == min::FAILURE() )
	return PAR::parse_error
	    ( parser, ppvec->position,
	      "parser command not recognized" );

    parser->printer << min::eom;
    return result;
}

void COM::parser_execute_command
	( PAR::parser parser,
	  min::gen indented_paragraph )
{
    min::obj_vec_ptr ipvp ( indented_paragraph );
    min::phrase_position_vec ipppvec =
        min::position_of ( ipvp );
    MIN_ASSERT ( ipppvec != min::NULL_STUB,
                 "missing .position attribute" );

    min::uns32 ipsize = min::size_of ( ipvp );
    if ( ipsize == 0 ) return;

    parser->printer << min::bol << min::bom
                    << min::adjust_indent ( 4 )
                    << "PARSER: "
		    << min::indent;

    for ( min::uns32 i = 0; i < ipsize; ++ i )
    {
	min::obj_vec_ptr vp ( ipvp[i] );
	if ( vp == min::NULL_STUB )
	{
	    PAR::parse_error
		( parser, ipppvec[i],
		  "parser command not recognized" );
	    continue;
	}
	min::phrase_position_vec ppvec =
	    min::position_of ( vp );
	MIN_ASSERT ( ppvec != min::NULL_STUB,
		     "missing .position attribute" );

	min::uns32 size = min::size_of ( vp );
	if ( size == 0 )
	{
	    PAR::parse_error
		( parser, ipppvec[i],
		  "empty parser command" );
	    continue;
	}

	min::gen result = min::FAILURE();

	if ( size >= 2
	     &&
	     vp[1] == PAR::pass_lexeme )
	    result = ::execute_pass
			( vp, 0, ppvec, parser );
	else if ( size >= 2
		  &&
		  vp[1] == PAR::selector )
	    result = ::execute_selectors
			( vp, 0, ppvec, parser );
	else if ( size >= 2
		  &&
		  vp[1] == PAR::context_lexeme )
	    result = ::execute_context
			( vp, 0, ppvec, parser );
	else if ( vp[0] == ::trace
		  &&
		  ( size == 1
		    ||
		    min::is_obj ( vp[1] ) ) )
	    result = ::execute_trace
			( vp, 0, ppvec, parser );
	else if ( size >= 2
		  &&
		  vp[0] == PAR::print
		  &&
		  vp[1] == ::trace )
	    result = ::execute_trace
			( vp, 0, ppvec, parser );
	else if ( vp[0] == PAR::begin )
	    result = ::execute_begin
			( vp, 0, ppvec, parser );
	else if ( vp[0] == PAR::end )
	    result = ::execute_end
			( vp, 0, ppvec, parser );

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
		    ( parser, pass, vp, 0, ppvec );
	}

	if ( result == min::SUCCESS()
	     &&
	     (   parser->trace_flags
	       & PAR::TRACE_PARSER_COMMANDS ) )
	    min::print_phrase_lines
		( parser->printer,
		  ppvec->file, ppvec->position, 0 );

	else if ( result == min::FAILURE() )
	    PAR::parse_error
		( parser, ppvec->position,
		  "parser command not recognized" );

    }

    parser->printer << min::eom;
}
