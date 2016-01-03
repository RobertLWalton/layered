// Layered Languages Prefix Parser Pass
//
// File:	ll_parser_prefix.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun Jan  3 07:26:14 EST 2016
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Prefix Table Entries
//	Prefix Parser Pass
//	Prefix Parse Function
//	Prefix Pass Command Function

// Usage and Setup
// ----- --- -----

# include <ll_lexeme_standard.h>
# include <ll_parser_command.h>
# include <ll_parser_prefix.h>
# define LEX ll::lexeme
# define LEXSTD ll::lexeme::standard
# define PAR ll::parser
# define TAB ll::parser::table
# define COM ll::parser::command
# define PRE ll::parser::prefix

static min::locatable_gen prefix;
static min::locatable_gen prefix_subexpressions;

static void initialize ( void )
{
    ::prefix = min::new_str_gen ( "prefix" );
    ::prefix_subexpressions =
        min::new_lab_gen ( "prefix", "subexpressions" );

    PAR::push_new_pass ( ::prefix, PRE::new_pass );
}
static min::initializer initializer ( ::initialize );

// Prefix Table Entries
// ------ ----- -------

static min::uns32 prefix_gen_disp[] = {
    min::DISP ( & PRE::prefix_struct::label ),
    min::DISP_END };

static min::uns32 prefix_stub_disp[] = {
    min::DISP ( & PRE::prefix_struct::next ),
    min::DISP_END };

static min::packed_struct_with_base
	<PRE::prefix_struct, TAB::root_struct>
    prefix_type ( "ll::parser::prefix::prefix_type",
	        ::prefix_gen_disp,
	        ::prefix_stub_disp );
const min::uns32 & PRE::PREFIX = ::prefix_type.subtype;

void PRE::push_prefix
	( min::gen prefix_label,
	  TAB::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  TAB::new_flags new_selectors,
	  TAB::key_table prefix_table )
{
    min::locatable_var<PRE::prefix> prefix
        ( ::prefix_type.new_stub() );

    label_ref(prefix) = prefix_label;
    prefix->selectors = selectors;
    prefix->block_level = block_level;
    prefix->position = position;
    prefix->new_selectors = new_selectors;

    TAB::push ( prefix_table, (TAB::root) prefix );
}

// Prefix Parser Pass
// ------ ------ ----

static min::packed_vec< PRE::prefix_stack_struct >
    prefix_stack_type
        ( "ll::parser::prefix::prefix_stack_type" );

static min::uns32 prefix_pass_gen_disp[] =
{
    min::DISP ( & PRE::prefix_pass_struct::name ),
    min::DISP_END
};

static min::uns32 prefix_pass_stub_disp[] =
{
    min::DISP ( & PRE::prefix_pass_struct::parser ),
    min::DISP ( & PRE::prefix_pass_struct::next ),
    min::DISP ( & PRE::prefix_pass_struct
                     ::prefix_table ),
    min::DISP ( & PRE::prefix_pass_struct
                     ::prefix_stack ),
    min::DISP_END
};

static min::packed_struct_with_base
	<PRE::prefix_pass_struct, PAR::pass_struct>
    prefix_pass_type
        ( "ll::parser::prefix::prefix_pass_type",
	  ::prefix_pass_gen_disp,
	  ::prefix_pass_stub_disp );
const min::uns32 & PRE::PREFIX_PASS =
    ::prefix_pass_type.subtype;

static void prefix_pass_place
	( PAR::parser parser,
	  PAR::pass pass )
{
    PRE::prefix_pass prefix_pass =
        (PRE::prefix_pass) pass;

    int index = TAB::find_name
        ( parser->trace_flag_name_table,
	  ::prefix_subexpressions );
    MIN_REQUIRE
      ( (unsigned) index < 8 * sizeof ( TAB::flags ) );
    
    prefix_pass->trace_subexpressions =
        1ull << index;
}

static void prefix_pass_reset
	( PAR::parser parser,
	  PAR::pass pass )
{
    PRE::prefix_pass prefix_pass =
        (PRE::prefix_pass) pass;

    TAB::key_table prefix_table =
        prefix_pass->prefix_table;
    PRE::prefix_stack prefix_stack =
        prefix_pass->prefix_stack;
    min::pop ( prefix_stack, prefix_stack->length );

    min::uns64 collected_entries,
               collected_key_prefixes;

    TAB::end_block
        ( prefix_table, 0,
	  collected_key_prefixes, collected_entries );
}

static min::gen prefix_pass_end_block
	( PAR::parser parser,
	  PAR::pass pass,
	  const min::phrase_position & position,
	  min::gen name )
{
    PRE::prefix_pass prefix_pass =
        (PRE::prefix_pass) pass;
    TAB::key_table prefix_table =
        prefix_pass->prefix_table;

    min::uns64 collected_entries,
               collected_key_prefixes;

    min::uns32 block_level =
        PAR::block_level ( parser );
    MIN_REQUIRE ( block_level > 0 );
    TAB::end_block
        ( prefix_table, block_level - 1,
	  collected_key_prefixes, collected_entries );

    return min::SUCCESS();
}

static void prefix_parse ( PAR::parser parser,
		         PAR::pass pass,
		         TAB::flags selectors,
		         PAR::token & first,
		         PAR::token next );

static min::gen prefix_pass_command
	( PAR::parser parser,
	  PAR::pass pass,
	  min::obj_vec_ptr & vp,
          min::phrase_position_vec ppvec );

PAR::pass PRE::new_pass ( PAR::parser parser )
{
    min::locatable_var<PRE::prefix_pass> prefix_pass
        ( ::prefix_pass_type.new_stub() );

    PRE::name_ref ( prefix_pass ) = ::prefix;

    if ( parser->prefix_table == min::NULL_STUB )
        prefix_table_ref(parser) =
	    ( TAB::create_key_table ( 1024 ) );
    PRE::prefix_table_ref ( prefix_pass ) =
        parser->prefix_table;

    PRE::prefix_stack_ref ( prefix_pass ) =
	::prefix_stack_type.new_stub ( 100 );

    prefix_pass->parser_command = ::prefix_pass_command;
    prefix_pass->parse = ::prefix_parse;
    prefix_pass->place = ::prefix_pass_place;
    prefix_pass->reset = ::prefix_pass_reset;
    prefix_pass->end_block = ::prefix_pass_end_block;

    return (PAR::pass) prefix_pass;
}

// Prefix Parse Function
// ------ ----- --------

static void prefix_parse ( PAR::parser parser,
		         PAR::pass pass,
		         TAB::flags selectors,
		         PAR::token & first,
		         PAR::token next )
{
    PRE::prefix_pass prefix_pass =
        (PRE::prefix_pass) pass;
    PRE::prefix_stack prefix_stack =
        prefix_pass->prefix_stack;

    TAB::flags trace_flags = parser->trace_flags;
    if (   trace_flags
         & prefix_pass->trace_subexpressions )
    {
	trace_flags &=
	      PAR::TRACE_SUBEXPRESSION_ELEMENTS
	    + PAR::TRACE_SUBEXPRESSION_DETAILS
	    + PAR::TRACE_SUBEXPRESSION_LINES
	    + PAR::TRACE_KEYS;
	if ( trace_flags == 0 )
	    trace_flags =
	        PAR::TRACE_SUBEXPRESSION_ELEMENTS;
    }
    else
        trace_flags = 0;

    // We add to the stack but leave alone what is
    // already in the stack so this function can be
    // called recursively.
    //
    min::unsptr initial_length = prefix_stack->length;

    bool prefix_found = false;
    bool at_phrase_beginning = true;
    PAR::token current = first;
    while ( true )
    {

	// Find prefix if possible.
	//
	while ( current != next
		&&
		current->type != PAR::PREFIX )
	{
	    at_phrase_beginning = false;
	    current = current->next;
	}

        // Discover if prefix is in stack.  Set i to
	// stack position + 1 if yes, initial_length
	// if no.
	//
	min::unsptr i =
	    ( current != next ?
	      prefix_stack->length :
	      initial_length );
	while ( i > initial_length
	        &&
		   current->value_type
		!= (&prefix_stack[i-1])->first
		                       ->value_type )
	    -- i;
	
	// If prefix is in the stack or we are a the end
	// of the expression, close all the previous
	// subexpressions delimited by the stack entry
	// of prefix and entries closer to the top of
	// the stack, or all stack entries if we are at
	// the end of the expression.
	//
	if ( i > initial_length || current == next )
	{
	    at_phrase_beginning = true;

	    min::unsptr j = prefix_stack->length;
	    while ( j >= i && j > initial_length )
	    {
		PAR::token prefix =
		    min::pop(prefix_stack).first;
		j = prefix_stack->length;

		PAR::token first = prefix->next;

		if ( first != current )
		{
		    PAR::execute_pass_parse
			 ( parser, pass->next,
			   selectors,
			   first, current );

		    min::phrase_position position =
			{ prefix->position.begin,
			  current->previous
			         ->position.end };

		    min::obj_vec_insptr vp
		    	( prefix->value );
		    min::attr_insptr ap ( vp );

		    min::locate
		        ( ap, min::dot_position );
		    min::phrase_position_vec_insptr
		        pos = min::get ( ap );
		    pos->position = position;

		    while ( first != current )
		    {
			if (    first->string
			     != min::NULL_STUB )
			    PAR::convert_token
			        ( first );

			min::attr_push(vp) =
			    first->value;
			min::push ( pos ) =
			    first->position;

			first = first->next;
			PAR::free
			    ( PAR::remove
				  ( PAR::first_ref
				        (parser),
				    first->previous ) );
		    }
		    prefix->position = position;
		}

		prefix->type = PAR::BRACKETED;
		PAR::trace_subexpression
		    ( parser, prefix, trace_flags );
	    }
	}

	if ( current == next ) break;

	else if ( at_phrase_beginning )
	{
	    PRE::prefix_stack_struct s =
	        { current, min::MISSING() };
	    min::push(prefix_stack) = s;
	    current = current->next;
	    prefix_found = true;
	}

	else
	{
	    PAR::parse_error
	        ( parser,
		  current->position,
		  "prefix token not at beginning of"
		  " phrase; deleted and ignored" );
	    current = current->next;
	    PAR::free
		( PAR::remove
		      ( PAR::first_ref
			    (parser),
			current->previous ) );
	}
    }

    if ( ! prefix_found )
	PAR::execute_pass_parse
	    ( parser, pass->next, selectors,
	      first, current );
}


// Prefix Pass Command Function
// ------ ---- ------- --------

static min::gen prefix_pass_command
	( PAR::parser parser,
	  PAR::pass pass,
	  min::obj_vec_ptr & vp,
          min::phrase_position_vec ppvec )
{
    PRE::prefix_pass prefix_pass =
        (PRE::prefix_pass) pass;

    min::uns32 size = min::size_of ( vp );

    min::uns32 i = 1;
        // vp[i] is next lexeme or subexpression to
	// scan in the define/undefine expression.

    min::gen command = vp[i++];

    if ( command != PAR::define
         &&
	 command != PAR::undefine
         &&
	 command != PAR::print )
        return min::FAILURE();

    if ( i >= size || vp[i++] != ::prefix )
        return min::FAILURE();

    // Scan prefix type name.
    //
    min::locatable_gen name[2];
    unsigned number_of_names = 0;

    while ( true )
    {
	// Scan a name.
	//
	name[number_of_names] =
	    PAR::scan_name_string_label
	        ( vp, i, parser,

	            ( 1ull << LEXSTD::mark_t )
	          + ( 1ull << LEXSTD::separator_t )
	          + ( 1ull << LEXSTD::word_t )
	          + ( 1ull << LEXSTD::natural_t )
	          + ( 1ull << LEXSTD::numeric_t ),

		    ( 1ull << LEXSTD::
			      horizontal_space_t )
		  + ( 1ull << LEXSTD::
		              indent_before_comment_t )
		  + ( 1ull << LEXSTD::
		              indent_t )
	          + ( 1ull << LEXSTD::
		              premature_end_of_file_t )
	          + ( 1ull << LEXSTD::end_of_file_t ),

	            ( 1ull << LEXSTD::
		              premature_end_of_file_t )
	          + ( 1ull << LEXSTD::end_of_file_t ),
		  command == PAR::print );

	if ( name[number_of_names] == min::ERROR() )
	    return min::ERROR();
	else if (    name[number_of_names]
	          == min::MISSING() )
	    return PAR::parse_error
	        ( parser, ppvec[i-1],
		  "expected quoted name after" );
	else
	    ++ number_of_names;

	if ( number_of_names > 1 )
	    return PAR::parse_error
	        ( parser, ppvec->position,
		  "too many quoted names in" );

	if ( i >= size
	     ||
	     vp[i] != PAR::dotdotdot )
	    break;

	++ i;
    }

    if ( number_of_names < 1 )
	return PAR::parse_error
	    ( parser, ppvec->position,
	      "too few quoted names in" );

    if ( command == PAR::print )
    {

	COM::print_command ( vp, parser );

	parser->printer
	    << ":" << min::eol
	    << min::bom << min::no_auto_break
	    << min::set_indent ( 4 );

	int count = 0;

	{
	    TAB::key_table_iterator prefix_it
		( prefix_pass->prefix_table );
	    while ( true )
	    {
		TAB::root root = prefix_it.next();
		if ( root == min::NULL_STUB ) break;

		if ( min::is_subsequence
			 ( name[0], root->label ) < 0 )
		    continue;

		PRE::prefix prefix = (PRE::prefix) root;

		min::gen block_name =
		    PAR::block_name
			( parser,
			  prefix->block_level );
		parser->printer
		    << min::indent
		    << "block "
		    << min::pgen_name ( block_name )
		    << ": " << min::save_indent;

		parser->printer
		    << "prefix "
		    << min::pgen_quote
		           ( prefix->label );

		parser->printer
		    << " " << min::set_break;

		COM::print_flags
		    ( prefix->selectors,
		      PAR::ALL_SELECTORS,
		      parser->selector_name_table,
		      parser );

		TAB::new_flags new_selectors =
		    prefix->new_selectors;

		if ( TAB::all_flags ( new_selectors )
		     &
		     PAR::ALL_SELECTORS )
		{
		    parser->printer
		        << min::indent
			<< "with parsing"
			   " selectors ";
		    COM::print_new_flags
			( new_selectors,
			  PAR::ALL_SELECTORS,
			  parser->selector_name_table,
			  parser );
		}

		parser->printer
		    << min::restore_indent;

		++ count;
	    }
	}

	if ( count == 0 )
	    parser->printer << min::indent
	                    << "not found";
	parser->printer << min::eom;

    	return COM::PRINTED;
    }

    // Scan selectors.
    //
    TAB::flags selectors;
    min::gen sresult = COM::scan_flags
	    ( vp, i, selectors, PAR::ALL_SELECTORS,
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
    else MIN_REQUIRE ( sresult == min::SUCCESS() );

    if ( command == PAR::define )
    {
	TAB::new_flags new_selectors;
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
		min::gen result =
		    COM::scan_new_flags
			( vp, i, new_selectors,
			  PAR::ALL_SELECTORS,
			  parser->selector_name_table,
			  parser->
			    selector_group_name_table,
			  parser, true );
		if ( result == min::ERROR() )
		    return min::ERROR();
		else if ( result == min::FAILURE() )
		    return PAR::parse_error
			( parser, ppvec[i-1],
			  "expected bracketed selector"
			  " modifier list after" );
	    }
	    else
		return PAR::parse_error
		    ( parser, ppvec[i-1],
		      "expected `parsing selectors'"
		      " after" );
	}
	if ( i < size )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "expected `with' after" );

	PRE::push_prefix
	    ( name[0], selectors,
	      PAR::block_level ( parser ),
	      ppvec->position,
	      new_selectors,
	      prefix_pass->prefix_table );
    }

    else // if ( command == PAR::undefine )
    {

	if ( i < size )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "unexpected stuff after" );

	TAB::key_prefix key_prefix =
	    TAB::find_key_prefix
	        ( name[0], prefix_pass->prefix_table );

	min::uns32 count = 0;

	if ( key_prefix != min::NULL_STUB )
	for ( TAB::root root = key_prefix->first;
	      root != min::NULL_STUB;
	      root = root->next )
	{
	    if (    ( root->selectors & selectors )
		 == 0 )
		continue;

	    min::uns32 subtype =
		min::packed_subtype_of ( root );

	    if ( subtype != PRE::PREFIX )
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
