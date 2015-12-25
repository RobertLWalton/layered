// Layered Languages Prefix Parser Pass
//
// File:	ll_parser_prefix.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Dec 23 18:34:13 EST 2015
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
    min::DISP ( & OP::prefix_struct::label ),
    min::DISP_END };

static min::uns32 prefix_stub_disp[] = {
    min::DISP ( & OP::prefix_struct::next ),
    min::DISP_END };

static min::packed_struct_with_base
	<OP::prefix_struct, TAB::root_struct>
    prefix_type ( "ll::parser::prefix::prefix_type",
	        ::prefix_gen_disp,
	        ::prefix_stub_disp );
const min::uns32 & PRE::PREFIX = ::prefix_type.subtype;

void PRE::push_prefix
	( min::gen prefix_label,
	  TAB::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  TAB::new_flags new_flags,
	  TAB::key_table prefix_table )
{
    min::locatable_var<PRE::prefix> prefix
        ( ::prefix_type.new_stub() );

    label_ref(prefix) = prefix_label;
    prefix->selectors = selectors;
    prefix->block_level = block_level;
    prefix->position = position;
    prefix->new_flags = new_flags;

    TAB::push ( prefix_table, (TAB::root) prefix );
}

// Prefix Parser Pass
// ------ ------ ----

static min::packed_vec< PRE::prefix_stack_struct >
    prefix_stack_type
        ( "(ll_parser_prefix.cc)::prefix_stack_type" );

static min::uns32 prefix_pass_stub_disp[] =
{
    min::DISP ( & PRE::prefix_pass_struct::parser ),
    min::DISP ( & PRE::prefix_pass_struct::next ),
    min::DISP ( & PRE::prefix_pass_struct::prefix_table ),
    min::DISP ( & PRE::prefix_pass_struct::prefix_stack ),
    min::DISP_END
};

static min::packed_struct_with_base
	<PRE::prefix_pass_struct, PAR::pass_struct>
    prefix_pass_type
        ( "ll::parser::prefix::prefix_pass_type",
	  NULL,
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
    PRE::prefix_pass prefix_pass = (PRE::prefix_pass) pass;

    TAB::key_table prefix_table = prefix_pass->prefix_table;
    PRE::prefix_stack prefix_stack = prefix_pass->prefix_stack;
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
    PRE::prefix_pass prefix_pass = (PRE::prefix_pass) pass;
    TAB::key_table prefix_table = prefix_pass->prefix_table;

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

PAR::pass PRE::new_pass ( void )
{
    min::locatable_var<PRE::prefix_pass> prefix_pass
        ( ::prefix_pass_type.new_stub() );

    PRE::name_ref ( prefix_pass ) = ::prefix;

    PRE::prefix_table_ref ( prefix_pass ) =
        TAB::create_key_table ( 1024 );

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
    PRE::prefix_pass prefix_pass = (PRE::prefix_pass) pass;
    PRE::prefix_stack prefix_stack = prefix_pass->prefix_stack;

    TAB::flags trace_flags = parser->trace_flags;
    if ( trace_flags & prefix_pass->trace_subexpressions )
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

    bool at_phrase_beginning = true;
    PAR::token current = first;
    while ( true )
    {

	// Find prefix if possible.
	//
	while ( current != next
		&&
		current->type != BRA::PREFIX )
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
		!= prefix_stack[i-1].first->value_type )
	    -- i;
	
	// If prefix is in the stack or we are a the end
	// of the expression, close all the previous
	// subexpressions delimited by the stack entry of
	// prefix and entries closer to the top of the
	// stack, or all stack entries if we are at the
	// end of the expression.
	//
	if ( i > initial_length || current == next )
	{
	    at_phrase_beginning = true;

	    min::unsptr j = prefix_stack->length;
	    while ( j >= i && j > original_length )
	    {
		PAR::token first =
		    min::pop(prefix_stack).first;
		j = prefix_stack->length;

		PAR::execute_pass_parse
		     ( parser, pass->next,
		       selectors,
		       first->next, current );

		if ( first->next != current )
		{
		    min::phrase_position position =
			{ first->position.begin,
			  current->previous
			         ->position.end };

		    min::obj_vec_insptr vp
		    	( first->value );
		    min::attr_insptr ap ( vp );

		    min::locate
		        ( ap, min::dot_position );
		    min::phrase_position_vec_insptr
		        pos = min::get ( ap );
		    pos->position = position;

		    PAR::token next = first->next;
		    while ( next != current )
		    {
			if (    next->string
			     != min::NULL_STUB )
			    PAR::convert_token ( next );

			min::attr_push(vp) =
			    next->value;
			min::push ( pos ) =
			    next->position;

			next = next->next;
			PAR::free
			    ( PAR::remove
				  ( PAR::first_ref
				        (parser),
				    next->previous ) );
		    }
		}

		PAR::trace_subexpression
		    ( parser, first, trace_flags );
	    }
	}

	if ( current == next ) break;

	else if ( at_phrase_beginning )
	{
	    PRE::prefix_stack_struct s =
	        { current, min::MISSING() };
	    min::push(prefix_stack) = s;
	    current = current->next;
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
}


TBD

// Operator Pass Command Function
// -------- ---- ------- --------

static min::gen oper_pass_command
	( PAR::parser parser,
	  PAR::pass pass,
	  min::obj_vec_ptr & vp,
          min::phrase_position_vec ppvec )
{
    OP::oper_pass oper_pass = (OP::oper_pass) pass;

    min::uns32 size = min::size_of ( vp );

    // Scan keywords before names.
    //
    bool bracket = false;
        // True if bracket, false if not.
    bool indentation_mark = false;
        // True if indentation mark, false if not.

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

    if ( i >= size || vp[i++] != ::oper )
        return min::FAILURE();

    if ( i >= size || command == PAR::print )
        /* Do nothing. */;
    else if ( vp[i] == ::bracket )
    {
	++ i;
	bracket = true;
    }
    else if ( vp[i] == ::indentation
              &&
	      i + 1 < size
	      &&
	      vp[i+1] == ::mark )
    {
        i += 2;
	indentation_mark = true;
    }

    // Scan operator names.
    //
    min::locatable_gen name[3];
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

	if ( number_of_names > 2 )
	    return PAR::parse_error
	        ( parser, ppvec->position,
		  "too many quoted names in" );

	if ( i >= size
	     ||
	     vp[i] != PAR::dotdotdot )
	    break;

	++ i;
    }

    if ( number_of_names < ( bracket ? 2 : 1 ) )
	return PAR::parse_error
	    ( parser, ppvec->position,
	      "too few quoted names in" );
    else if ( ! bracket && number_of_names > 1 )
	return PAR::parse_error
	    ( parser, ppvec->position,
	      "too many quoted names in" );
    else if ( ! bracket )
        name[1] = min::MISSING();

    if ( command == PAR::print )
    {

	COM::print_command ( vp, parser );

	parser->printer
	    << ":" << min::eol
	    << min::bom << min::no_auto_break
	    << min::set_indent ( 4 );

	int count = 0;

	{
	    TAB::key_table_iterator oper_it
		( oper_pass->oper_table );
	    while ( true )
	    {
		TAB::root root = oper_it.next();
		if ( root == min::NULL_STUB ) break;

		if ( min::is_subsequence
			 ( name[0], root->label ) < 0 )
		    continue;

		::print_op ( (OP::oper) root,
		             ::OPERATOR,
		             parser );

		++ count;
	    }

	    TAB::key_table_iterator bracket_it
		( oper_pass->oper_bracket_table );
	    while ( true )
	    {
		TAB::root root = bracket_it.next();
		if ( root == min::NULL_STUB ) break;

		if ( min::is_subsequence
			 ( name[0], root->label ) < 0 )
		    continue;

		::print_op ( (OP::oper) root,
		             ::BRACKET,
		             parser );

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

    // Scan operator flags.
    //
    min::uns32 oper_flags = 0;

    min::phrase_position oper_flags_position;
    oper_flags_position.begin = (&ppvec[i])->begin;

    while ( i < size )
    {
	min::uns32 new_oper_flag;
        if ( vp[i] == OP::prefix )
	    new_oper_flag = OP::PREFIX;
        else if ( vp[i] == OP::infix )
	    new_oper_flag = OP::INFIX;
        else if ( vp[i] == OP::postfix )
	    new_oper_flag = OP::POSTFIX;
        else if ( vp[i] == OP::afix )
	    new_oper_flag = OP::AFIX;
        else if ( vp[i] == OP::nofix )
	    new_oper_flag = OP::NOFIX;
	else break;

	if ( oper_flags & new_oper_flag )
	    return PAR::parse_error
		( parser, ppvec[i],
		  "operator flag ",
		  min::pgen_quote ( vp[i] ),
		  " appears twice" );

	oper_flags |= new_oper_flag;
	++ i;
    }

    if ( oper_flags == 0 )
	return PAR::parse_error
	    ( parser, ppvec[i-1],
	      "expected operator flags after" );

    oper_flags_position.end = (&ppvec[i-1])->end;

    if ( ( oper_flags & OP::NOFIX )
          &&
	 ( oper_flags & OP::PREFIX ) )
	return PAR::parse_error
	    ( parser, oper_flags_position,
	      "operator flags nofix and prefix"
	      " are incompatible" );
    if ( ( oper_flags & OP::NOFIX )
          &&
	 ( oper_flags & OP::INFIX ) )
	return PAR::parse_error
	    ( parser, oper_flags_position,
	      "operator flags nofix and infix"
	      " are incompatible" );
    if ( ( oper_flags & OP::NOFIX )
          &&
	 ( oper_flags & OP::POSTFIX ) )
	return PAR::parse_error
	    ( parser, oper_flags_position,
	      "operator flags nofix and postfix"
	      " are incompatible" );
    if ( ( oper_flags & OP::INFIX )
          &&
	 ( oper_flags & OP::POSTFIX ) )
	return PAR::parse_error
	    ( parser, oper_flags_position,
	      "operator flags infix and postfix"
	      " are incompatible" );

    min::int32 precedence;
    bool precedence_found = false;
    PAR::reformatter reformatter = min::NULL_STUB;
    min::locatable_var
    	    < PAR::reformatter_arguments >
        reformatter_arguments ( min::NULL_STUB );
    while ( i < size && vp[i] == PAR::with )
    {
	++ i;
	if ( i < size
	     &&
	     vp[i] == ::precedence )
	{
	    ++ i;
	    int sign = +1;
	    if ( i >= size )
		/* do nothing */;
	    else if ( vp[i] == PAR::plus )
		sign = +1, ++ i;
	    else if ( vp[i] == PAR::minus )
		sign = -1, ++ i;
	    min::gen pg = min::MISSING();
	    if ( i < size )
	    {
		if (    PAR::get_attribute
		            ( vp[i], min::dot_type )
		     == PAR::number_sign )
		{
		    min::obj_vec_ptr pvp = vp[i];
		    if ( min::size_of ( pvp ) == 1 )
		        pg = pvp[0];
		}
		else
		    pg = vp[i];
	    }
	        
	    if ( ! min::strto ( precedence, pg, 10 ) )
		return PAR::parse_error
		    ( parser, ppvec[i-1],
		      "expected precedence integer"
		      " after" );
	    precedence *= sign;
	    precedence_found = true;
	    ++ i;
	    continue;

	}
	else if ( i < size )
	{
	    min::uns32 j = i;
	    min::locatable_gen name
	      ( COM::scan_simple_name
	          ( vp, j, PAR::reformatter_lexeme ) );
	    if (    j < size
		 && vp[j] == PAR::reformatter_lexeme )
	    {
		min::phrase_position position =
		    { (&ppvec[i])->begin,
		      (&ppvec[j])->end };
		reformatter =
		    PAR::find_reformatter
		        ( name,
			  OP::reformatter_stack );
		if ( reformatter == min::NULL_STUB )
		{
		    return PAR::parse_error
			( parser, position,
			  "undefined reformatter"
			  " name" );
		}

		min::uns32 illegal_flags =
		        oper_flags
		    & ~ reformatter->flags;
		if ( illegal_flags != 0 )
		{
		    char buffer[200];
		    char * s = buffer;
		    s += sprintf
		        ( s, " reformatter"
			     " incompatible with" );
		    if ( illegal_flags & OP::PREFIX )
		        s += sprintf ( s, " prefix" );
		    if ( illegal_flags & OP::INFIX )
		        s += sprintf ( s, " infix" );
		    if ( illegal_flags & OP::POSTFIX )
		        s += sprintf ( s, " postfix" );
		    if ( illegal_flags & OP::NOFIX )
		        s += sprintf ( s, " nofix" );
		    if ( illegal_flags & OP::AFIX )
		        s += sprintf ( s, " afix" );
		    s += sprintf
		        ( s, " operator flag(s)" );
		    return PAR::parse_error
			    ( parser, ppvec->position,
			      "",
			      min::pgen_quote ( name ),
			      buffer );
		}

		i = j + 1;

		name = COM::scan_names
		    ( vp, i, reformatter_arguments,
		          parser );
		if ( name == min::ERROR() )
		    return min::ERROR();
		if (    reformatter_arguments
		     == min::NULL_STUB )
		{
		    if ( reformatter->minimum_arguments
		         > 0 )
			return PAR::parse_error
				( parser, position,
				  "reformatter"
				  " arguments"
				  " missing" );
		}
		else
		{
		    position.end = (&ppvec[i-1])->end;

		    if (   reformatter_arguments->length
			 < reformatter->
			       minimum_arguments )
			return PAR::parse_error
				( parser, position,
				  "too few reformatter"
				  " arguments" );
		    if (   reformatter_arguments->length
			 > reformatter->
			       maximum_arguments )
			return PAR::parse_error
				( parser, position,
				  "too many reformatter"
				  " arguments" );
		}

		continue;
	    }
	}

	return PAR::parse_error
	    ( parser, ppvec[i-1],
	      command == PAR::define ?
	      "expected `precedence ...' or"
	      " `... reformatter' after" :
	      "expected `precedence ...' after" );

    }
    if ( i < size )
	return PAR::parse_error
	    ( parser, ppvec[i-1],
	      "expected `with' after" );
    if ( ! precedence_found )
	return PAR::parse_error
	    ( parser, ppvec[i-1],
	      "expected `with precedence ...'"
	      " after" );

    if ( command == PAR::define )
    {
	OP::push_oper
	    ( name[0], name[1],
	      selectors,
	      PAR::block_level ( parser ),
	      ppvec->position,
	      oper_flags, precedence,
	      reformatter, reformatter_arguments,
	      bracket || indentation_mark ?
	          oper_pass->oper_bracket_table :
		  oper_pass->oper_table );
    }

    else // if ( command == PAR::undefine )
    {
	if ( reformatter != min::NULL_STUB )
	    return PAR::parse_error
		( parser, ppvec->position,
		  "did NOT expect"
		  " `with ... reformatter'" );

	TAB::key_prefix key_prefix =
	    TAB::find_key_prefix
	        ( name[0],
	          bracket || indentation_mark ?
		      oper_pass->oper_bracket_table :
		      oper_pass->oper_table );

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

	    if ( subtype != OP::OPER )
		continue;

	    OP::oper oper = (OP::oper) root;
	    if ( oper->precedence != precedence )
	        continue;
	    if ( oper->flags != oper_flags )
	        continue;
	    if ( oper->terminator != name[1] )
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

