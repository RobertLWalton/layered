// Layered Languages Parser Execute Command Function
//
// File:	ll_parser_command.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri Jul 14 21:57:53 EDT 2017
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
//	Execute Top Level
//	Execute Mapped Lexeme
//	Execute ID Character
//	Execute Test
//	Execute Trace
//	Execute Block
//	Execute Command

// Usage and Setup
// ----- --- -----

# include <ll_lexeme_standard.h>
# include <ll_parser_command.h>
# define LEX ll::lexeme
# define LEXSTD ll::lexeme::standard
# define PAR ll::parser
# define PARLEX ll::parser::lexeme
# define TAB ll::parser::table
# define COM ll::parser::command

static min::locatable_gen exclusive_or;
static min::locatable_gen trace;
static min::locatable_gen top;
static min::locatable_gen token;
static min::locatable_gen value;

static void initialize ( void )
{
    ::exclusive_or = min::new_str_gen ( "^" );
    ::trace = min::new_str_gen ( "trace" );
    ::top = min::new_str_gen ( "top" );
    ::token = min::new_str_gen ( "token" );
    ::value = min::new_str_gen ( "value" );
}
static min::initializer initializer ( ::initialize );

// Parser Command Functions
// ------ ------- ---------

void COM::print_lexical_master
	( PAR::parser parser,
	  min::uns32 paragraph_master,
	  min::uns32 line_master )
{
    if ( paragraph_master != PAR::MISSING_MASTER
	 &&
	 paragraph_master == line_master )
    {
	min::locatable_gen name
	    ( PAR::get_master_name
		  ( paragraph_master, parser ) );
	parser->printer << min::indent
	    << "with lexical master ";
	if ( name != min::MISSING() )
	    parser->printer
		<< min::pgen_quote ( name );
	else
	    parser->printer << paragraph_master;
    }
    else
    {
	if ( paragraph_master != PAR::MISSING_MASTER )
	{
	    min::locatable_gen name
		( PAR::get_master_name
		      ( paragraph_master, parser ) );
	    parser->printer << min::indent
		<< "with paragraph lexical master ";
	    if ( name != min::MISSING() )
		parser->printer
		    << min::pgen_quote ( name );
	    else
		parser->printer << paragraph_master;
	}
	if ( line_master != PAR::MISSING_MASTER )
	{
	    min::locatable_gen name
		( PAR::get_master_name
		      ( line_master, parser ) );
	    parser->printer << min::indent
		<< "with line lexical master ";
	    if ( name != min::MISSING() )
		parser->printer
		    << min::pgen_quote ( name );
	    else
		parser->printer << line_master;
	}
    }
}
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
    if (    min::get ( subap )
         != PARLEX::left_parenthesis )
        return min::FAILURE();
    min::locate ( subap, min::dot_terminator );
    if (    min::get ( subap )
         != PARLEX::right_parenthesis )
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
	    if ( subvp[j] != PARLEX::comma )
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
// It is a duplication error if a named flag (not a
// named group) is in found_flags.  Unduplicated named
// flags are OR'ed to found_flags.
//
// On errors an error message is printed and false
// is returned.  Otherwise true is returned.
//
static bool scan_flag
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  min::phrase_position_vec ppvec,
	  TAB::new_flags & new_flags,
	  TAB::new_flags & group_new_flags,
	  TAB::flags & found_flags,
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

    if (    vp[i] == PARLEX::plus
         || vp[i] == PARLEX::minus
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

	if ( f & found_flags )
	{
	    PAR::parse_error
		( parser, position,
		  "flag/option name is duplicate" );
	    return false;
	}
	else
	    found_flags |= f;
    }

    if ( ( f & allowed_flags ) != f )
    {
	PAR::parse_error
	    ( parser, position,
	      "flag/option name not recognized in this"
	      " context" );
	return false;
    }

    if ( op == PARLEX::minus )
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
    if ( min::get ( subap ) != PARLEX::left_square )
        return min::FAILURE();
    min::locate ( subap, min::dot_terminator );
    if ( min::get ( subap ) != PARLEX::right_square )
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
    TAB::flags found_flags = 0;

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
			 found_flags,
			 allowed_flags,
		         allow_flag_list,
		         allow_flag_modifier_list,
		         name_table, group_name_table,
		         parser ) )
		return min::ERROR();

	    if ( i == size ) break;

	    if ( subvp[i] != PARLEX::comma )
		return PAR::parse_error
		    ( parser, ppvec[i-1],
		      "expected `,' after" );
	    ++ i;
	}
    }

    TAB::flags mask = allowed_flags & ~ found_flags;

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

min::uns32 COM::print_command
	( PAR::parser parser,
	  min::phrase_position_vec ppvec )
{
    if (    parser->message_header.begin
         != min::MISSING_POSITION )
    {
	min::print_phrase_lines
	    ( parser->printer, parser->input_file,
	      parser->message_header, 0 );
	parser->message_header.begin =
	    min::MISSING_POSITION;
    }

    return min::print_phrase_lines
	( parser->printer,
	  ppvec->file, ppvec->position, 0 );
}

// Execute Pass
// ------- ----

static min::gen execute_pass
	( min::obj_vec_ptr & vp, min::uns32 i0,
          min::phrase_position_vec ppvec,
	  PAR::parser parser )
{
    if ( vp[i0] != PARLEX::define
         &&
	 vp[i0] != PARLEX::undefine
         &&
	 vp[i0] != PARLEX::print )
	return min::FAILURE();

    min::uns32 size = min::size_of ( vp );

    min::uns32 i = i0 + 2;
    min::locatable_gen name, name2;
    PAR::new_pass new_pass = NULL;
    TAB::flags selectors;
    min::phrase_position name_pp;
    if ( vp[i0] != PARLEX::print )
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
    if ( vp[i0] == PARLEX::define )
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

	if ( vp[i] == PARLEX::after
	     ||
	     vp[i] == PARLEX::before )
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

	    if ( vp[begini-1] == PARLEX::after )
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
	else if ( vp[i] == PARLEX::at
	          &&
		  vp[i+1] == PARLEX::end )
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

    if ( vp[i0] == PARLEX::print )
    {
        min::uns32 indent =
	    COM::print_command ( parser, ppvec );
	parser->printer
	    << min::bom << min::no_auto_break
	    << min::set_indent ( indent + 4 );

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
    else if ( vp[i0] == PARLEX::define )
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
    else /* if vp[i0] == PARLEX::undefine */
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
    if ( vp[i0] != PARLEX::define
         &&
	 vp[i0] != PARLEX::print )
	return min::FAILURE();

    min::uns32 size = min::size_of ( vp );

    min::uns32 i = i0 + 2;
    min::uns32 begini = i;
    min::locatable_gen name;
    if ( vp[i0] == PARLEX::define )
    {
	name = PAR::scan_simple_name ( vp, i );
	if ( name == min::MISSING() )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "expected simple name after" );
    }
    else /* if vp[i0] == PARLEX::print */
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

    if ( vp[i0] == PARLEX::define )
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
    else /* if vp[i0] == PARLEX::print */
    {
        min::uns32 indent =
	    COM::print_command ( parser, ppvec );
	parser->printer
	    << min::bom << min::no_auto_break
	    << min::set_indent ( indent + 4 );
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
	                    << "nothing found";
	parser->printer << min::eom;

	return PAR::PRINTED;
    }

    return min::SUCCESS();
}

// Execute Top Level
// ------- --- -----

static min::gen execute_top_level
	( min::obj_vec_ptr & vp, min::uns32 i0,
          min::phrase_position_vec ppvec,
	  PAR::parser parser )
{
    min::uns32 size = min::size_of ( vp );
    MIN_REQUIRE ( size >= i0 + 3 );

    if (    vp[i0] != PARLEX::define
         && vp[i0] != PARLEX::print )
	return min::FAILURE();

    min::uns32 i = i0 + 3;

    if ( vp[i0] == PARLEX::print )
    {

	if ( i < size )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "extraneous stuff after" );

        min::uns32 indent =
	    COM::print_command ( parser, ppvec );
	parser->printer
	    << min::bom << min::no_auto_break
	    << min::set_indent ( indent + 4 );

	TAB::flags flags = parser->selectors;
	min::uns32 paragraph_lexical_master =
	    parser->paragraph_lexical_master;
	min::uns32 line_lexical_master =
	    parser->line_lexical_master;
	for ( min::uns32 i =
		  parser->block_stack->length;
	      ; -- i )
	{
	    min::gen block_name =
		( i == 0 ?
		  (min::gen) PARLEX::top_level :
		  (&parser->block_stack[i-1])
		      ->name );

	    parser->printer << min::indent
			    << "block "
			    << min::pgen_name
				 ( block_name )
			    << ": "
			    << min::save_indent;


	    parser->printer << min::indent
	                    << " selectors ";
	    COM::print_flags
		( flags,
		  PAR::COMMAND_SELECTORS,
		  parser->selector_name_table,
		  parser );

	    parser->printer << min::indent
	                    << " options ";
	    COM::print_flags
		( flags,
		  PAR::ALL_OPT,
		  parser->selector_name_table,
		  parser );

	    COM::print_lexical_master
	        ( parser,
		  paragraph_lexical_master,
		  line_lexical_master );

	    parser->printer << min::restore_indent;

	    if ( i == 0 ) break;

	    flags = (&parser->block_stack[i-1])
			->saved_selectors;
	    paragraph_lexical_master =
	        (&parser->block_stack[i-1])
		     ->saved_paragraph_lexical_master;
	    line_lexical_master =
	        (&parser->block_stack[i-1])
		     ->saved_line_lexical_master;
	}

	parser->printer << min::eom;
	return PAR::PRINTED;
    }

    TAB::new_flags new_selectors;
    TAB::new_flags new_options;
	// Inited to zeroes.

    min::uns32 paragraph_lexical_master =
        parser->paragraph_lexical_master;
    min::uns32 line_lexical_master =
        parser->line_lexical_master;

    min::uns32 saved_i = i;
    while ( i < size && vp[i] == PARLEX::with )
    {
	++ i;
	if ( i + 1 < size
	     &&
	     vp[i] == PARLEX::parsing
	     &&
	     vp[i+1] == PARLEX::selectors )
	{
	    i += 2;
	    min::gen result;
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
	     vp[i] == PARLEX::parsing
	     &&
	     vp[i+1] == PARLEX::options )
	{
	    i += 2;
	    min::gen result;
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
	if ( COM::is_lexical_master
		 ( vp, i, size ) )
	{
	    if (    COM::get_lexical_master
		       ( parser, vp, ppvec, i,
			 paragraph_lexical_master,
			 line_lexical_master )
		 == min::ERROR() )
		return min::ERROR();
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

    parser->selectors |= new_selectors.or_flags
                      |  new_options.or_flags;
    parser->selectors &= ~ (   new_selectors.not_flags
                             | new_options.not_flags );
    parser->selectors ^= new_selectors.xor_flags
                      ^  new_options.xor_flags;
    parser->selectors |= PAR::TOP_LEVEL_SELECTOR
                      |  PAR::ALWAYS_SELECTOR;
    parser->paragraph_lexical_master =
        paragraph_lexical_master;
    parser->line_lexical_master =
        line_lexical_master;

    if ( i < size )
        return PAR::parse_error
	    ( parser, ppvec[i-1],
	      "extraneous stuff after" );

    return min::SUCCESS();
}

// Execute Mapped Lexeme
// ------- ------ ------

static min::gen execute_mapped_lexeme
	( min::obj_vec_ptr & vp, min::uns32 i0,
          min::phrase_position_vec ppvec,
	  PAR::parser parser )
{
    min::uns32 size = min::size_of ( vp );
    MIN_REQUIRE ( size >= i0 + 3 );

    min::gen command = vp[i0];
    if (    command != PARLEX::define
         && command != PARLEX::undefine
         && command != PARLEX::print )
	return min::FAILURE();

    min::uns32 i = i0 + 3;

    // Scan lexeme name.
    //
    min::locatable_gen name
        ( PAR::scan_quoted_key
	    ( vp, i, parser,
	      command == PARLEX::print ) );

    if ( name == min::ERROR() )
	return min::ERROR();
    else if ( name == min::MISSING() )
	return PAR::parse_error
	    ( parser, ppvec[i-1],
	      "expected quoted name after" );

    if ( command == PARLEX::print )
    {
	if ( i < size )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "unexpected stuff after" );

	min::uns32 indent =
	    COM::print_command ( parser, ppvec );

	parser->printer
	    << min::bom << min::no_auto_break
	    << min::set_indent ( indent + 4 );

	int count = 0;

	for ( min::uns32 index = 0;
	      index < parser->lexeme_map->length;
	      ++ index )
	for ( TAB::root entry =
		  parser->lexeme_map[index];
	      entry != min::NULL_STUB;
	      entry = entry->next )
	{
	    if ( min::is_subsequence
		     ( name, entry->label ) < 0 )
		continue;

	    ++ count;

	    min::gen block_name =
		PAR::block_name
		    ( parser,
		      entry->block_level );
	    parser->printer
		<< min::indent
		<< "block "
		<< min::pgen_name ( block_name )
		<< ": " << min::save_indent;

	    parser->printer
	        << "mapped lexeme "
	    	<< min::pgen_quote ( entry->label )
		<< " ";

	    COM::print_flags
		( entry->selectors,
		  PAR::COMMAND_SELECTORS,
		  parser->selector_name_table,
		  parser );

	    TAB::lexeme_map_entry e =
	    	(TAB::lexeme_map_entry) entry;

	    min::gen token_value = e->token_value;
	    if ( token_value != min::MISSING() )
		parser->printer
		    << min::indent
		    << "with token value "
		    << min::pgen ( token_value );

	    min::uns32 master = e->lexical_master;
	    if ( master != PAR::MISSING_MASTER )
	    {
		min::locatable_gen name
		    ( PAR::get_master_name
			  ( master, parser ) );
		parser->printer
		    << min::indent
		    << "with lexical master ";
		if ( name != min::MISSING() )
		    parser->printer
			<< min::pgen_quote ( name );
		else
		    parser->printer << master;
	    }

	    parser->printer << min::restore_indent
			    << min::eol;
	}

	if ( count == 0 )
	    parser->printer << min::indent
	                    << "nothing found";
	parser->printer << min::eom;

        return PAR::PRINTED;

    }

    min::uns32 lexeme_type =
	LEX::lexeme_type ( name, parser->scanner );
    if ( lexeme_type == LEX::MISSING_TYPE )
	return PAR::parse_error
	    ( parser, ppvec[i-1], "`",
	      min::pgen_quote ( name ),
	      "' does not name a lexeme type" );

    TAB::flags selectors;
    min::gen sresult = COM::scan_flags
	    ( vp, i, selectors, PAR::COMMAND_SELECTORS,
	      parser->selector_name_table,
	      parser->selector_group_name_table,
	      parser );
    if ( sresult == min::ERROR() )
	return min::ERROR();
    else if ( sresult == min::FAILURE() )
	return PAR::parse_error
	    ( parser, ppvec[i-1],
	      "expected bracketed selector list"
	      " after" );
    else MIN_REQUIRE
             ( sresult == min::SUCCESS() );

    if ( command == PARLEX::define )
    {

	min::locatable_gen token_value ( min::NONE() );
	min::uns32 lexical_master = PAR::MISSING_MASTER;

	while ( i < size && vp[i] == PARLEX::with )
	{
	    ++ i;
	    if ( i + 1 < size
		 &&
		 vp[i] == ::token
		 &&
		 vp[i+1] == ::value )
	    {
		i += 2;
		if ( i >= size )
		    return PAR::parse_error
			( parser, ppvec[i-1],
			  "expected token value"
			  " after" );
		token_value = vp[i++];
	    }
	    else
	    if ( i + 1 < size
		 &&
		 vp[i] == PARLEX::lexical
		 &&
		 vp[i+1] == PARLEX::master )
	    {
		i += 2;
		min::phrase_position position
		    = ppvec[i];
		min::locatable_gen master_name
		    ( PAR::scan_name
			( vp, i, parser,
			  PARLEX::with ) );
		if ( master_name == min::ERROR() )
		    return min::ERROR();
		position.end = (& ppvec[i-1])->end;

		lexical_master =
		    PAR::get_lexical_master
			( master_name, parser );
		if (    lexical_master
		     == PAR::MISSING_MASTER )
		    return PAR::parse_error
			( parser, position,
			  "`",
			  min::pgen_quote
			      ( master_name ),
			  "' does NOT name a lexical"
			  " master" );
	    }
	    else
		return PAR::parse_error
		    ( parser, ppvec[i-1],
		      "expected `token value'"
		      " or `lexical master' after" );
	}

	min::uns32 token_type = 0;
	min::locatable_gen token_value_type;

	if ( min::is_obj ( token_value ) )
	{
	    token_type = PAR::BRACKETABLE;
	    min::obj_vec_ptr vp ( token_value );
	    min::attr_ptr ap ( vp );
	    min::locate ( ap, min::dot_type  );
	    min::gen type = min::get ( ap );
	    min::locate ( ap, min::dot_initiator  );
	    min::gen initiator = min::get ( ap );
	    min::locate ( ap, min::dot_terminator  );
	    min::gen terminator = min::get ( ap );

	    if ( initiator != min::NONE()
	         ||
		 terminator != min::NONE() )
	    {
	        token_type = PAR::BRACKETED;
		token_value_type = initiator;
	    }
	    else if ( type != min::NONE() )
	    {
	        if ( min::size_of ( vp ) == 0 )
		{
		    token_type = PAR::PREFIX;
		    token_value_type = type;
		}
		else
		    token_type = PAR::BRACKETED;
	    }
	    else
	        token_type = PAR::BRACKETABLE;
	}
	else if ( token_value != min::NONE() )
	    token_type = PAR::DERIVED;

	if ( i < size )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "extraneous stuff after" );
	PAR::init_lexeme_map ( parser );
	TAB::push_lexeme_map_entry
	    ( name, lexeme_type, selectors,
	      PAR::block_level ( parser ),
	      ppvec->position,
	      token_value, token_type,
	      token_value_type,
	      lexical_master,
	      parser->lexeme_map );
    }
    else
    {
        MIN_REQUIRE ( command == PARLEX::undefine );

	if ( i < size )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "unexpected stuff after" );

	min::uns32 count = 0;

	for ( TAB::root root =
	          parser->lexeme_map[lexeme_type];
	      root != min::NULL_STUB;
	      root = root->next )
	{
	    if ( root->label != name ) continue;

	    if ( ( root->selectors & selectors ) == 0 )
		continue;

	    TAB::push_undefined
		( parser->undefined_stack,
		  root, selectors );

	    ++ count;
	}

	if ( count == 0 )
	    PAR::parse_warn
		( parser, ppvec->position,
		  "undefine found no definition" );
	else if ( count > 1 )
	    PAR::parse_warn
		( parser, ppvec->position,
		  "undefine cancelled more than one"
		  " definition" );
    }

    return min::SUCCESS();
}


// Execute ID Character
// ------- -- ---------

static min::gen execute_ID_character
	( min::obj_vec_ptr & vp, min::uns32 i0,
          min::phrase_position_vec ppvec,
	  PAR::parser parser )
{
    min::uns32 size = min::size_of ( vp );
    MIN_REQUIRE ( size >= i0 + 3 );

    min::gen command = vp[i0];

    min::uns32 i = i0 + 3;

    if ( command == PARLEX::print )
    {
	if ( i < size )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "unexpected stuff after" );

	min::uns32 indent =
	    COM::print_command ( parser, ppvec );

	parser->printer
	    << min::bom << min::no_auto_break
	    << min::set_indent ( indent + 4 );

	min::Uchar ID_character = parser->ID_character;
	for ( min::uns32 i =
		  parser->block_stack->length;
	      0 <= i; -- i )
	{
	    min::gen block_name =
		( i == 0 ?
		  (min::gen) PARLEX::top_level :
		  (&parser->block_stack[i-1])
		      ->name );

	    parser->printer << min::indent
			    << "block "
			    << min::pgen_name
				 ( block_name )
			    << ": ";
	    if ( ID_character == min::NO_UCHAR )
		parser->printer << "disabled";
	    else
		parser->printer
		    << "`"
		    << min::punicode ( ID_character )
		    << "'";

	    if ( i == 0 ) break;

	    TAB::block_struct b =
		parser->block_stack[i-1];
	    ID_character = b.saved_ID_character;
	}

	parser->printer << min::eom;
	return PAR::PRINTED;
    }

    else if ( command != PARLEX::define )
	return min::FAILURE();

    else if ( i >= size )
	return PAR::parse_error
	    ( parser, ppvec[i-1],
	      "expected ID character after" );

    min::Uchar ID_character;
    if ( vp[i] == PARLEX::disabled )
	ID_character = min::NO_UCHAR;
    else
    {
	if (    min::get ( vp[i], min::dot_type )
	     != PARLEX::doublequote )
	    return PAR::parse_error
		( parser, ppvec[i],
		  "expected quoted string" );

	min::obj_vec_ptr svp = vp[i];
	if ( min::size_of ( svp ) != 1 )
	    return PAR::parse_error
		( parser, ppvec[i],
		  "expected quoted string" );

	min::str_ptr sp = svp[0];
	min::unsptr slen = min::strlen ( sp );
	if ( slen > 20 )
	    return PAR::parse_error
		( parser, ppvec[i],
		  "quoted string must have ONLY ONE"
		  " unicode character" );
	char buffer[21];
	min::strcpy ( buffer, sp );
	const char * p = buffer, * endp = buffer + slen;
	ID_character = min::utf8_to_unicode ( p, endp );
	if ( * p != 0 )
	    return PAR::parse_error
		( parser, ppvec[i],
		  "quoted string must have ONLY ONE"
		  " unicode character" );
    }

    if ( i + 1 < size )
	return PAR::parse_error
	    ( parser, ppvec[i],
	      "unexpected stuff after" );

    parser->ID_character = ID_character;

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
    if ( vp[i0] == PARLEX::print )
    {

	TAB::name_table t =
	    parser->trace_flag_name_table;

        min::uns32 indent =
	    COM::print_command ( parser, ppvec );
	parser->printer
	    << min::bom << min::no_auto_break
	    << min::set_indent ( indent + 4 )
	    << min::indent
	    << "["
	    << min::adjust_indent ( 2 )
	    << min::indent;

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

// Execute Block
// ------- -----

static min::gen execute_block
	( min::obj_vec_ptr & vp, min::uns32 i0,
          min::phrase_position_vec ppvec,
	  PAR::parser parser )
{
    if (    vp[i0] != PARLEX::begin
         && vp[i0] != PARLEX::end )
	return min::FAILURE();

    min::uns32 i = i0 + 2;
    min::locatable_gen name
        ( PAR::scan_simple_name ( vp, i ) );
    if ( name == min::MISSING() )
    {
	min::phrase_position p0 = ppvec[i0];
	min::phrase_position p1 = ppvec[i0+1];
	min::phrase_position pos = { p0.begin, p1.end };
	return PAR::parse_error
	    ( parser, pos,
	      "expected block name after" );
    }
    if ( i != min::size_of ( vp ) )
        return PAR::parse_error
	    ( parser,
	      ppvec[i-1],
	      "extraneous stuff after" );

    if ( vp[i0] == PARLEX::begin )
	return PAR::begin_block
		    ( parser, name, ppvec->position );
    else
	return PAR::end_block
		    ( parser, name, ppvec->position );
}

// Execute Command
// ------- -------

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

    parser->message_header.begin =
    parser->message_header.end =
        ipppvec->position.begin;

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
	     vp[1] == PARLEX::block )
	    result = ::execute_block
			( vp, 0, ppvec, parser );
	else if ( size >= 2
	          &&
	          vp[1] == PARLEX::pass )
	    result = ::execute_pass
			( vp, 0, ppvec, parser );
	else if ( size >= 2
		  &&
		  vp[1] == PARLEX::selector )
	    result = ::execute_selectors
			( vp, 0, ppvec, parser );
	else if ( size >= 3
		  &&
		  vp[1] == PARLEX::top
		  &&
		  vp[2] == PARLEX::level )
	    result = ::execute_top_level
			( vp, 0, ppvec, parser );
	else if ( size >= 3
		  &&
		  vp[1] == PARLEX::mapped
		  &&
		  vp[2] == PARLEX::lexeme )
	    result = ::execute_mapped_lexeme
			( vp, 0, ppvec, parser );
	else if ( size >= 3
		  &&
		  vp[1] == PARLEX::ID
		  &&
		  vp[2] == PARLEX::character )
	    result = ::execute_mapped_lexeme
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
		  vp[0] == PARLEX::print
		  &&
		  vp[1] == ::trace )
	    result = ::execute_trace
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
	    COM::print_command ( parser, ppvec );

	else if ( result == min::FAILURE() )
	    PAR::parse_error
		( parser, ppvec->position,
		  "parser command not recognized" );

    }

    parser->message_header.begin =
    parser->message_header.end =
        min::MISSING_POSITION;
}
