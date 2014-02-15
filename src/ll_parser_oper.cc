// Layers Language Operator Parser Pass
//
// File:	ll_parser_oper.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Feb 15 15:06:56 EST 2014
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Operator Table Entries
//	Operator Parser Pass
//	Operator Parse Function
//	Operator Reformatters
//	Operator Pass Command Function

// Usage and Setup
// ----- --- -----

# include <ll_lexeme_standard.h>
# include <ll_parser_command.h>
# include <ll_parser_oper.h>
# include <cstdio>
# define LEX ll::lexeme
# define LEXSTD ll::lexeme::standard
# define PAR ll::parser
# define TAB ll::parser::table
# define COM ll::parser::command
# define OP ll::parser::oper

min::locatable_gen OP::dollar;
min::locatable_gen OP::AND;
min::locatable_gen OP::prefix;
min::locatable_gen OP::infix;
min::locatable_gen OP::postfix;
min::locatable_gen OP::afix;
min::locatable_gen OP::nofix;
static min::locatable_gen operator_subexpressions;
static min::locatable_gen oper;
static min::locatable_gen bracket;
static min::locatable_gen indentation;
static min::locatable_gen mark;
static min::locatable_gen precedence;
static min::locatable_gen reformatter;

static void initialize ( void )
{
    OP::dollar  = min::new_str_gen ( "$" );
    OP::AND     = min::new_str_gen ( "AND" );
    OP::prefix  = min::new_str_gen ( "prefix" );
    OP::infix   = min::new_str_gen ( "infix" );
    OP::postfix = min::new_str_gen ( "postfix" );
    OP::afix    = min::new_str_gen ( "afix" );
    OP::nofix   = min::new_str_gen ( "nofix" );

    ::operator_subexpressions =
        min::new_lab_gen
	    ( "operator", "subexpressions" );
    ::oper = min::new_str_gen ( "operator" );
    ::bracket = min::new_str_gen ( "bracket" );
    ::indentation = min::new_str_gen ( "indentation" );
    ::mark = min::new_str_gen ( "mark" );
    ::precedence = min::new_str_gen ( "precedence" );
    ::reformatter = min::new_str_gen ( "reformatter" );

    PAR::push_new_pass ( ::oper, OP::new_pass );
}
static min::initializer initializer ( ::initialize );

// Operator Table Entries
// -------- ----- -------

static min::uns32 oper_gen_disp[] = {
    min::DISP ( & OP::oper_struct::label ),
    min::DISP ( & OP::oper_struct::terminator ),
    min::DISP_END };

static min::uns32 oper_stub_disp[] = {
    min::DISP ( & OP::oper_struct::next ),
    min::DISP ( & OP::oper_struct
                    ::reformatter_arguments ),
    min::DISP_END };

static min::packed_struct_with_base
	<OP::oper_struct, TAB::root_struct>
    oper_type ( "ll::parser::oper::oper_type",
	        ::oper_gen_disp,
	        ::oper_stub_disp );
const min::uns32 & OP::OPER = oper_type.subtype;

void OP::push_oper
	( min::gen oper_label,
	  min::gen terminator,
	  TAB::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  min::uns32 flags,
	  min::int32 precedence,
	  OP::reformatter reformatter,
	  min::packed_vec_ptr<min::gen>
	      reformatter_arguments,
	  TAB::key_table oper_table )
{
    min::locatable_var<OP::oper> oper
        ( ::oper_type.new_stub() );

    label_ref(oper) = oper_label;
    terminator_ref(oper) = terminator;
    oper->selectors = selectors;
    oper->block_level = block_level;
    oper->position = position;
    oper->flags = flags;
    oper->precedence = precedence;
    oper->reformatter = reformatter;
    reformatter_arguments_ref(oper) =
	reformatter_arguments;

    TAB::push ( oper_table, (TAB::root) oper );
}

// Operator Parser Pass
// -------- ------ ----

static min::packed_vec< OP::oper_stack_struct >
    oper_stack_type
        ( "(ll_parser_oper.cc)::oper_stack_type" );

static min::uns32 oper_pass_stub_disp[] =
{
    min::DISP ( & OP::oper_pass_struct::parser ),
    min::DISP ( & OP::oper_pass_struct::next ),
    min::DISP ( & OP::oper_pass_struct::oper_table ),
    min::DISP ( & OP::oper_pass_struct
                    ::oper_bracket_table ),
    min::DISP ( & OP::oper_pass_struct::oper_stack ),
    min::DISP_END
};

static min::packed_struct_with_base
	<OP::oper_pass_struct, PAR::pass_struct>
    oper_pass_type
        ( "ll::parser::oper::oper_pass_type",
	  NULL,
	  ::oper_pass_stub_disp );
const min::uns32 & OP::OPER_PASS =
    ::oper_pass_type.subtype;

static void oper_pass_place
	( PAR::parser parser,
	  PAR::pass pass )
{
    OP::oper_pass oper_pass = (OP::oper_pass) pass;

    int index = TAB::find_name
        ( parser->trace_flag_name_table,
	  ::operator_subexpressions );
    assert
      ( (unsigned) index < 8 * sizeof ( TAB::flags ) );
    
    oper_pass->trace_subexpressions =
        1ull << index;
}

static void oper_pass_reset
	( PAR::parser parser,
	  PAR::pass pass )
{
    OP::oper_pass oper_pass = (OP::oper_pass) pass;

    TAB::key_table oper_table = oper_pass->oper_table;
    TAB::key_table oper_bracket_table =
        oper_pass->oper_bracket_table;
    OP::oper_stack oper_stack = oper_pass->oper_stack;
    min::pop ( oper_stack, oper_stack->length );

    min::uns64 collected_entries,
               collected_key_prefixes;

    TAB::end_block
        ( oper_table, 0,
	  collected_key_prefixes, collected_entries );
    TAB::end_block
        ( oper_bracket_table, 0,
	  collected_key_prefixes, collected_entries );

    oper_pass->temporary_count = 0;
}

static min::gen oper_pass_end_block
	( PAR::parser parser,
	  PAR::pass pass,
	  const min::phrase_position & position,
	  min::gen name )
{
    OP::oper_pass oper_pass = (OP::oper_pass) pass;
    TAB::key_table oper_table = oper_pass->oper_table;
    TAB::key_table oper_bracket_table =
        oper_pass->oper_bracket_table;

    min::uns64 collected_entries,
               collected_key_prefixes;

    min::uns32 block_level =
        PAR::block_level ( parser );
    assert ( block_level > 0 );
    TAB::end_block
        ( oper_table, block_level - 1,
	  collected_key_prefixes, collected_entries );
    TAB::end_block
        ( oper_bracket_table, block_level - 1,
	  collected_key_prefixes, collected_entries );

    return min::SUCCESS();
}

static void oper_parse ( PAR::parser parser,
		         PAR::pass pass,
		         TAB::flags selectors,
		         PAR::token & first,
		         PAR::token next );

static min::gen oper_pass_command
	( PAR::parser parser,
	  PAR::pass pass,
	  min::obj_vec_ptr & vp,
          min::phrase_position_vec ppvec );

PAR::pass OP::new_pass ( void )
{
    min::locatable_var<OP::oper_pass> oper_pass
        ( ::oper_pass_type.new_stub() );

    OP::name_ref ( oper_pass ) = ::oper;

    OP::oper_table_ref ( oper_pass ) =
        TAB::create_key_table ( 1024 );

    OP::oper_bracket_table_ref ( oper_pass ) =
        TAB::create_key_table ( 256 );

    OP::oper_stack_ref ( oper_pass ) =
	::oper_stack_type.new_stub ( 100 );

    oper_pass->parser_command = ::oper_pass_command;
    oper_pass->parse = ::oper_parse;
    oper_pass->place = ::oper_pass_place;
    oper_pass->reset = ::oper_pass_reset;
    oper_pass->end_block = ::oper_pass_end_block;

    return (PAR::pass) oper_pass;
}

// Operator Parse Function
// -------- ----- --------

// Return true iff the argument is a precedence in the
// oper_stack.
//
inline bool check_precedence
	( min::int32 precedence,
	 OP::oper_stack oper_stack )
{
    for ( min::uns32 i = 0; i < oper_stack->length;
    			    ++ i )
    {
        if ( oper_stack[i].precedence == precedence )
	    return true;
    }
    return false;
}

static void oper_parse ( PAR::parser parser,
		         PAR::pass pass,
		         TAB::flags selectors,
		         PAR::token & first,
		         PAR::token next )
{
    OP::oper_pass oper_pass = (OP::oper_pass) pass;
    OP::oper_stack oper_stack = oper_pass->oper_stack;

    TAB::flags trace_flags = parser->trace_flags;
    if ( trace_flags & oper_pass->trace_subexpressions )
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
    min::unsptr initial_length = oper_stack->length;

    // Data that is pushed to oper_stack.  D is in
    // effect the top of the stack.
    //
    OP::oper_stack_struct D;
    D.first = first;
    D.precedence = OP::NO_PRECEDENCE;
    D.first_oper = min::NULL_STUB;

    min::uns32 last_oper_flags = 0;
        // Flags of the last operator seen in the
	// expression.

    PAR::token current = D.first;
    while ( true )
    {

	// Find operator if possible.
	//
	PAR::token next_current = current;
	OP::oper oper = min::NULL_STUB;

	if ( current != next )
	{
	    bool bracketed =
	        ( current->type == PAR::BRACKETED );

	    TAB::key_prefix key_prefix;
	    TAB::root root = min::NULL_STUB;
	    min::gen initiator, terminator;
	    
	    if ( bracketed )
	    {
	        {
		    min::obj_vec_ptr vp
		        ( current->value );
		    min::attr_ptr ap ( vp );

		    min::locate
		        ( ap, PAR::dot_initiator );
		    initiator = min::get ( ap );
		    min::locate
		        ( ap, PAR::dot_terminator );
		    terminator = min::get ( ap );

		    // Destroy object pointers to make
		    // object current->value non-
		    // OBJ_PRIVATE.
		}

		if ( initiator == min::NONE() )
		    root = min::NULL_STUB;
		else
		    root = TAB::find
			( initiator, selectors,
			  oper_pass->
			      oper_bracket_table );
		if ( terminator == min::NONE() )
		    terminator = min::MISSING();
		oper = (OP::oper) root;
		while ( oper != min::NULL_STUB
		        &&
			(    (   oper->selectors
			       & selectors )
			  == 0
			  ||
			     oper->terminator
			  != terminator ) )
		{
		    root = root->next;
		    oper = (OP::oper) root;
		}
	    }
	    else
	    {
	        root = PAR::find_entry
		    ( parser, next_current, key_prefix,
		      selectors, oper_pass->oper_table,
		      next );
		oper = (OP::oper) root;
	    }
	    while ( root != min::NULL_STUB
		    &&
		    ( oper == min::NULL_STUB
		      ||	     
		      ( oper->flags & OP::PREFIX
			&&
			( current != D.first
			  ||
			    oper->precedence
			  < D.precedence
			  ||
			  (    oper->precedence
			    == D.precedence
			    &&
			       (   last_oper_flags
				 & OP::PREFIX )
			    == 0 )
			)
		      )
		      ||
		      ( oper->flags & OP::INFIX
			&&
			current == D.first
			&&
			oper->precedence >= D.precedence
		      )
		      ||
		      ( oper->flags & OP::POSTFIX
			&&
			current == D.first
			&&
			(   oper->precedence
			  > D.precedence
			  ||
			  (    oper->precedence
			    == D.precedence
			    &&
			       (   last_oper_flags
				 & OP::POSTFIX )
			    == 0 )
			)
		      )
		      ||
		      ( oper->flags & OP::AFIX
			&&
			oper->precedence != D.precedence
			&&
			! check_precedence
			      ( oper->precedence,
				oper_stack ) )
		      )
		  )
	    {
	        if ( bracketed )
		{
		    do
		    {
			root = root->next;
			oper = (OP::oper) root;
		    }
		    while ( oper != min::NULL_STUB
			    &&
			    (    (   oper->selectors
				   & selectors )
			      == 0
			      ||
			         oper->terminator
			      != terminator ) );
		}
		else
		{
		    root = PAR::find_next_entry
			( parser, next_current,
			          key_prefix,
			          selectors, root );
		    oper = (OP::oper) root;
		}
	    }

	    // Make OPERATOR token if an operator was
	    // found.  Note that next_current ends up
	    // pointing after the OPERATOR token and
	    // current is left intact and points at the
	    // new OPERATOR token.  If no operator was
	    // found, current == next_current.
	    //
	    if ( oper != min::NULL_STUB )
	    {
	        if ( bracketed )
		{
		    current->type = PAR::OPERATOR;
		    next_current = current->next;
		}
		else
		{
		    current->position.end =
			next_current->previous
				    ->position.end;
		    while (    current
			    != next_current->previous )
			PAR::free
			    ( PAR::remove
				  ( PAR::first_ref
				             (parser),
				    next_current->
					previous ) );
		    current->type = PAR::OPERATOR;
		    PAR::value_ref ( current ) =
			oper->label;
		    MIN_ASSERT
		      (    current->string
		        == min::NULL_STUB );
		}

		if ( trace_flags & PAR::TRACE_KEYS )
		{
		    parser->printer
			<< min::bom
			<< min::set_indent ( 7 )
			<< "OPERATOR `"
			<< min::name_pgen
			       ( current->value )
			<< "' found; "
			<< min::pline_numbers
			       ( parser->input_file,
				 current->position )
			<< ":" << min::eom;
		    min::print_phrase_lines
			( parser->printer,
			  parser->input_file,
			  current->position );
		}
	    }
	}

	min::int32 oper_precedence = OP::NO_PRECEDENCE;
	    // Effective operator precedence.

	// Insert ERROR'OPERATOR token just before
	// current position if bad token (e.g.,
	// operand or operator with too high a
	// precedence) found after a postfix operator.
	//
	// Also compute oper_precedence.
	//
	if ( current == D.first
	     &&
	     last_oper_flags & OP::POSTFIX
	     &&
	     current != next
	     &&
	     ( oper == min::NULL_STUB
	       ||
	       oper->precedence > D.precedence
	       ||
	       ( oper->precedence == D.precedence
	         &&
		    ( oper->flags & OP::POSTFIX )
		 == 0 ) ) )
	{
	    next_current = current;
	    PAR::put_error_operator_before
	        ( parser, next_current );
	    current = next_current->previous;
	    oper_precedence = D.precedence - 1;
	    D.first = current;

	    parser->printer
		<< min::bom
		<< min::set_indent ( 7 )
		<< "ERROR: missing"
		   " operator of precedence "
		<< oper_precedence
		<< " inserted; "
		<< min::pline_numbers
		       ( parser->input_file,
			 current->position )
		<< ":" << min::eom;
	    min::print_phrase_lines
		( parser->printer,
		  parser->input_file,
		  current->position );
	    ++ parser->error_count;
	}
	else if ( oper != min::NULL_STUB )
	    oper_precedence = oper->precedence;

	// If no operator found and not at end of
	// expression, move to next token and loop.
	//
	if ( current == next_current
	     &&
	     current != next )
	{
	    current = current->next;
	    continue;
	}

	// Insert ERROR'OPERAND token just before
	// current if bad token (e.g., operator with too
	// low a precedence) found after infix or prefix
	// operator.
	//
	if ( current == D.first
	     &&
	     ( ( last_oper_flags & OP::INFIX
	         &&
		 ( current == next
		   ||
		   oper_precedence <= D.precedence )
	       )
	       ||
	       ( last_oper_flags & OP::PREFIX
	         &&
		 ( current == next
		   ||
		   oper_precedence < D.precedence
		   ||
	           ( oper->precedence == D.precedence
	             &&
		        ( oper->flags & OP::PREFIX )
		     == 0 )
		 )
	       )
	     )
	   )
	{
	    PAR::put_error_operand_after
	        ( parser, current->previous );
	    D.first = current->previous;

	    parser->printer
		<< min::bom
		<< min::set_indent ( 7 )
		<< "ERROR: missing operand inserted; "
		<< min::pline_numbers
		       ( parser->input_file,
			 D.first->position )
		<< ":" << min::eom;
	    min::print_phrase_lines
		( parser->printer,
		  parser->input_file,
		  D.first->position );
	    ++ parser->error_count;
	}

	// Close previous subexpressions until
	// D.precedence < oper_precedence or
	// D.precedence == oper_precedence and last
	// operator is postfix or current == next.
	// If current == next and oper_stack->length
	// == initial_length return to caller.
	//
	OP::oper first_oper = min::NULL_STUB;
	while ( true )
	{
	    if ( current != D.first )
	    {
		min::phrase_position position;
		position.begin =
		    D.first->position.begin;
		position.end =
		    current->previous->position.end;

		TAB::flags reformatter_trace_flags =
		       oper_stack->length
		    != initial_length ?
		    trace_flags : 0;

		if ( first_oper != min::NULL_STUB )
		{
		    PAR::attr attr
			( PAR::dot_oper,
			  first_oper->label );

		    if (    first_oper->reformatter
		         == NULL
			 ||
			 ( * first_oper->reformatter )
			     ( parser, pass, selectors,
			       D.first, current,
			       reformatter_trace_flags,
			       first_oper,
			       position ) )
			PAR::compact
			    ( parser, pass->next,
			      selectors,
			      PAR::BRACKETABLE,
			      reformatter_trace_flags,
			      D.first, current,
			      position,
			      1, & attr );
		}
		else
		{
		    PAR::execute_pass_parse
			 ( parser, pass->next,
			   selectors,
			   D.first, current );

		    if ( D.first->next != current )
		    {
			min::phrase_position position;
			position.begin =
			    D.first->position.begin;
			position.end =
			    current->previous
			           ->position.end;
			PAR::compact
			    ( parser, pass->next,
			      selectors,
			      PAR::BRACKETABLE,
			      reformatter_trace_flags,
			      D.first, current,
			      position );
		    }
		}
	    }

	    if ( current == next )
	    {
		if (    oper_stack->length
		     == initial_length )
		{
		    first = D.first;
		    return;
		}
		first_oper = D.first_oper;
		D = min::pop ( oper_stack );
	    }
	    else if ( oper_precedence < D.precedence
	              ||
		      ( oper_precedence == D.precedence
		        &&
		        last_oper_flags & OP::POSTFIX )
		    )

	    {
		first_oper = D.first_oper;
	        D = min::pop ( oper_stack );
	    }
	    else break;
	}

	// Start new subexpression if oper_precedence
	// > D.precedence or oper_precedence ==
	// D.precedence, last operator was prefix,
	// and last operator is previous token.
	//
	if ( oper_precedence > D.precedence
	     ||
	     ( oper_precedence == D.precedence
	       &&
	       last_oper_flags & OP::PREFIX
	       &&
	          current->previous->type
	       == PAR::OPERATOR ) )
	{
	    min::push ( oper_stack ) = D;
	    D.precedence = oper_precedence;
	    D.first_oper = oper;
	}

	D.first = next_current;
	last_oper_flags = oper->flags;
	current = next_current;
    }
}

// Operator Reformatters
// -------- ------------

// Given a sequence of tokens starting at `first' and
// ending just BEFORE `next', return the token_count of
// the number of tokens in the sequence, the oper_count
// of the number of operators in the sequence, and if
// there are operators, the position i of the first
// operator (i == 0 for `first'), and the value of
// that operator's token.
//
static void count_operators
	( PAR::token first, PAR::token next,
	  min::uns32 & token_count,
	  min::uns32 & oper_count,
	  min::uns32 & i,
	  min::gen & value )
{
    token_count = 0;
    oper_count = 0;
    for ( ; first != next;
          ++ token_count, first = first->next )
    {
        if ( first->type != PAR::OPERATOR )
	    continue;
	if ( oper_count == 0 )
	{
	    i = token_count;
	    value = first->value;
	}
	++ oper_count;
    }
}

static bool separator_reformatter
        ( PAR::parser parser,
	  PAR::pass pass,
	  TAB::flags selectors,
	  PAR::token & first,
	  PAR::token next,
	  TAB::flags trace_flags,
	  OP::oper first_oper,
	  min::phrase_position & position )
{
    MIN_ASSERT ( first != next );

    min::gen separator = first_oper->label;
    bool separator_should_be_next = false;
        // Equivalent meaning: the last token was an
	// operand.
    for ( PAR::token t = first; t != next; )
    {
        if ( t->type == PAR::OPERATOR )
	{
	    if ( separator != t->value )
	    {
		parser->printer
		    << min::bom
		    << min::set_indent ( 7 )
		    << "ERROR: wrong separator `"
		    << min::name_pgen ( t->value )
		    << "' changed to `"
		    << min::name_pgen ( separator )
		    << "'; "
		    << min::pline_numbers
			   ( parser->input_file,
			     t->position )
		    << ":" << min::eom;
		min::print_phrase_lines
		    ( parser->printer,
		      parser->input_file,
		      t->position );
		++ parser->error_count;
	    }

	    if ( ! separator_should_be_next )
	    {
	        PAR::put_empty_before ( parser, t );
		if ( t == first ) first = t->previous;
	    }
	    else separator_should_be_next = false;

	    if ( t->next == next )
	    {
	        // We need to do this before removing
		// operator as we need operator
		// position.
		//
		PAR::put_empty_after ( parser, t );
	    }
	    t = t->next;
	    PAR::free
		( PAR::remove
		      ( PAR::first_ref(parser),
			t->previous ) );
	}
	else
	{
	    MIN_ASSERT ( ! separator_should_be_next );
	        // Two operands should never be in
		// next to each other.
	    separator_should_be_next = true;
	    t = t->next;
	}
    }

    PAR::attr separator_attr
        ( PAR::dot_separator, separator );

    PAR::compact
        ( parser, pass->next, selectors,
	  PAR::BRACKETABLE, trace_flags,
	  first, next, position,
	  1, & separator_attr );

    return false;
}

static bool declare_reformatter
        ( PAR::parser parser,
	  PAR::pass pass,
	  TAB::flags selectors,
	  PAR::token & first,
	  PAR::token next,
	  TAB::flags trace_flags,
	  OP::oper first_oper,
	  min::phrase_position & position )
{
    MIN_ASSERT ( first != next );

    // We need to be careful to insert empty operands
    // using put_empty_before/after the operator so
    // the positions of the empty operands are correctly
    // set to be just before or after the operator.

    PAR::token t = first;

    // Ensure first element is operand.
    //
    if ( t->type == PAR::OPERATOR )
    {
	PAR::put_empty_before ( parser, t );
	first = t->previous;
    }
    else
        t = t->next;

    // Second element must be operator.
    //
    MIN_ASSERT
	( t != next && t->type == PAR::OPERATOR );

    t = t->next;

    // Ensure third element is operand.
    //
    if ( t == next || t->type == PAR::OPERATOR )
	PAR::put_empty_after ( parser, t->previous );
    else
        t = t->next;

    // Move second element to head of list.
    //
    PAR::token oper =
	PAR::remove ( PAR::first_ref ( parser ),
		      first->next );
    PAR::put_before ( PAR::first_ref ( parser ),
		      first, oper );
    first = oper;

    // Check that remaining elements (other than first
    // three) are bracketted operators and convert them
    // to operands.
    //
    while ( t != next )
    {
        if ( t->type != PAR::OPERATOR
	     ||
	     ! min::is_obj ( t->value ) )
	{
	    parser->printer
		<< min::bom
		<< min::set_indent ( 7 )
		<< "ERROR: expected bracketed"
		   " expression and got `"
		<< min::name_pgen ( t->value )
		<< "'; deleted; "
		<< min::pline_numbers
		       ( parser->input_file,
			 t->position )
		<< ":" << min::eom;
	    min::print_phrase_lines
		( parser->printer,
		  parser->input_file,
		  t->position );
	    ++ parser->error_count;

	    t = t->next;
	    PAR::free
	        ( PAR::remove
		      ( PAR::first_ref ( parser ),
		        t->previous ) );
	}
	else
	{
	    t->type = PAR::BRACKETED;
	    t = t->next;
	}
    }

    return true;
}

static bool right_associative_reformatter
        ( PAR::parser parser,
	  PAR::pass pass,
	  TAB::flags selectors,
	  PAR::token & first,
	  PAR::token next,
	  TAB::flags trace_flags,
	  OP::oper first_oper,
	  min::phrase_position & position )
{
    MIN_ASSERT ( first != next );

    // As operators must be infix, operands and
    // operators must alternate with operands first and
    // last.

    // Work from end to beginning taking 3 tokens at a
    // time and rewriting them into a subexpression.
    //
    while ( first->next != next )
    {
        PAR::token t = next->previous;
	MIN_ASSERT ( t->type != PAR::OPERATOR );
	MIN_ASSERT ( t != first );
        t = t->previous;
	MIN_ASSERT ( t->type == PAR::OPERATOR );
	MIN_ASSERT ( t != first );
        t = t->previous;
	MIN_ASSERT ( t->type != PAR::OPERATOR );

        min::phrase_position position =
	    { t->position.begin,
	      t->next->next->position.end };

	// Switch operator and first operand.
	//
	PAR::token oper =
	    PAR::remove ( PAR::first_ref ( parser ),
	                  t->next );
	PAR::put_before ( PAR::first_ref ( parser ),
	                  t, oper );
	bool t_is_first = ( t == first );
	t = t->previous;

	PAR::attr oper_attr
	    ( PAR::dot_oper, oper->value );
	PAR::compact
	    ( parser, pass->next, selectors,
	      PAR::BRACKETABLE, trace_flags,
	      t, next, position,
	      1, & oper_attr );

	if ( t_is_first ) first = t;
    }

    return false;
}

static bool unary_reformatter
        ( PAR::parser parser,
	  PAR::pass pass,
	  TAB::flags selectors,
	  PAR::token & first,
	  PAR::token next,
	  TAB::flags trace_flags,
	  OP::oper first_oper,
	  min::phrase_position & position )
{

    while ( first->type != PAR::OPERATOR )
    {
	MIN_ASSERT ( first != next );

	parser->printer
	    << min::bom
	    << min::set_indent ( 7 )
	    << "ERROR: expected an operator and got `"
	    << min::name_pgen ( first->value )
	    << "'; deleted; "
	    << min::pline_numbers
		   ( parser->input_file,
		     first->position )
	    << ":" << min::eom;
	min::print_phrase_lines
	    ( parser->printer,
	      parser->input_file,
	      first->position );
	++ parser->error_count;

	first = first->next;
	PAR::free
	    ( PAR::remove
		  ( PAR::first_ref ( parser ),
		    first->previous ) );
    }

    PAR::token t = first->next;

    while ( t != next && t->type == PAR::OPERATOR )
    {
	parser->printer
	    << min::bom
	    << min::set_indent ( 7 )
	    << "ERROR: expected operand and got `"
	    << min::name_pgen ( t->value )
	    << "'; deleted; "
	    << min::pline_numbers
		   ( parser->input_file,
		     t->position )
	    << ":" << min::eom;
	min::print_phrase_lines
	    ( parser->printer,
	      parser->input_file,
	      t->position );
	++ parser->error_count;

	t = t->next;
	PAR::free
	    ( PAR::remove
		  ( PAR::first_ref ( parser ),
		    t->previous ) );
    }

    if ( t == next )
    {
	t = t->previous;

	parser->printer
	    << min::bom
	    << min::set_indent ( 7 )
	    << "ERROR: expected operand after `"
	    << min::name_pgen ( t->value )
	    << "'; inserted ERROR'OPERAND; "
	    << min::pline_numbers
		   ( parser->input_file,
		     t->position )
	    << ":" << min::eom;
	min::print_phrase_lines
	    ( parser->printer,
	      parser->input_file,
	      t->position );
	++ parser->error_count;

	PAR::put_error_operand_after ( parser, t );
	t = t->next;
    }

    // Delete extra stuff from end of list.
    //
    t = t->next;
    if ( t != next )
    {
        min::phrase_position position =
	    { t->position.begin,
	      next->previous->position.end };
	parser->printer
	    << min::bom
	    << min::set_indent ( 7 )
	    << "ERROR: extra stuff at end of unary"
	       " expression; deleted; "
	    << min::pline_numbers
		   ( parser->input_file,
		     position )
	    << ":" << min::eom;
	min::print_phrase_lines
	    ( parser->printer,
	      parser->input_file,
	      position );
	++ parser->error_count;

	while ( t != next )
	{
	    t = t->next;
	    PAR::free
		( PAR::remove
		      ( PAR::first_ref ( parser ),
			t->previous ) );
	}
    }

    return true;
}

static bool binary_reformatter
        ( PAR::parser parser,
	  PAR::pass pass,
	  TAB::flags selectors,
	  PAR::token & first,
	  PAR::token next,
	  TAB::flags trace_flags,
	  OP::oper first_oper,
	  min::phrase_position & position )
{
    MIN_ASSERT ( first != next );

    // We need to be careful to insert error operands
    // using put_error_operand_before/after the operator
    // so the positions of the error operands are
    // correctly set to be just before or after the
    // operator.

    PAR::token t = first;

    // Ensure first element is operand.
    //
    if ( t->type == PAR::OPERATOR )
    {
	parser->printer
	    << min::bom
	    << min::set_indent ( 7 )
	    << "ERROR: expected operand before `"
	    << min::name_pgen ( t->value )
	    << "'; inserted ERROR'OPERAND; "
	    << min::pline_numbers
		   ( parser->input_file,
		     t->position )
	    << ":" << min::eom;
	min::print_phrase_lines
	    ( parser->printer,
	      parser->input_file,
	      t->position );
	++ parser->error_count;

	PAR::put_error_operand_before ( parser, t );
	first = t->previous;
    }
    else
        t = t->next;

    // Second element must be operator.
    //
    MIN_ASSERT
	( t != next && t->type == PAR::OPERATOR );

    t = t->next;

    // Ensure third element is operand.
    //
    if ( t == next || t->type == PAR::OPERATOR )
    {
	t = t->previous;

	parser->printer
	    << min::bom
	    << min::set_indent ( 7 )
	    << "ERROR: expected operand after `"
	    << min::name_pgen ( t->value )
	    << "'; inserted ERROR'OPERAND; "
	    << min::pline_numbers
		   ( parser->input_file,
		     t->position )
	    << ":" << min::eom;
	min::print_phrase_lines
	    ( parser->printer,
	      parser->input_file,
	      t->position );
	++ parser->error_count;

	PAR::put_error_operand_after ( parser, t );
	t = t->next;
    }

    // Move second element to head of list.
    //
    PAR::token oper =
	PAR::remove ( PAR::first_ref ( parser ),
		      first->next );
    PAR::put_before ( PAR::first_ref ( parser ),
		      first, oper );
    first = oper;

    // Delete extra stuff from end of list.
    //
    t = t->next;
    if ( t != next )
    {
        min::phrase_position position =
	    { t->position.begin,
	      next->previous->position.end };
	parser->printer
	    << min::bom
	    << min::set_indent ( 7 )
	    << "ERROR: extra stuff at end of binary"
	       " expression; deleted; "
	    << min::pline_numbers
		   ( parser->input_file,
		     position )
	    << ":" << min::eom;
	min::print_phrase_lines
	    ( parser->printer,
	      parser->input_file,
	      position );
	++ parser->error_count;

	while ( t != next )
	{
	    t = t->next;
	    PAR::free
		( PAR::remove
		      ( PAR::first_ref ( parser ),
			t->previous ) );
	}
    }

    return true;
}

static bool infix_reformatter
        ( PAR::parser parser,
	  PAR::pass pass,
	  TAB::flags selectors,
	  PAR::token & first,
	  PAR::token next,
	  TAB::flags trace_flags,
	  OP::oper first_oper,
	  min::phrase_position & position )
{
    MIN_ASSERT ( first != next );

    // As operators must be infix, operands and
    // operators must alternate with operands first and
    // last.  All operators must be the same; the first
    // is moved to the front and the others deleted.

    // Remove all operators but first, and check that
    // they are the same as first operator.
    //
    MIN_ASSERT ( first->next->type == PAR::OPERATOR );
    for ( PAR::token t = first->next->next;
          t->next != next; t = t->next )
    {
        MIN_ASSERT ( t->next->type == PAR::OPERATOR );
	if ( t->next->value != first->next->value )
	{
	    parser->printer
		<< min::bom
		<< min::set_indent ( 7 )
		<< "ERROR: wrong operator `"
		<< min::name_pgen ( t->next->value )
		<< "' changed to `"
		<< min::name_pgen ( first->next->value )
		<< "'; all operators"
		   " must be the same in this"
		   " subexpression; "
		<< min::pline_numbers
		       ( parser->input_file,
			 position )
		<< ":" << min::eom;
	    min::print_phrase_lines
		( parser->printer,
		  parser->input_file,
		  position );
	    ++ parser->error_count;
	}
        PAR::free
	    ( PAR::remove
		  ( PAR::first_ref(parser), t->next ) );
    }

    // Move first operator (second element) to head of
    // list.
    //
    PAR::put_before
	( PAR::first_ref(parser), first,
	  PAR::remove
	      ( PAR::first_ref(parser), first->next ) );
    first = first->previous;

    return true;
}

static bool infix_and_reformatter
        ( PAR::parser parser,
	  PAR::pass pass,
	  TAB::flags selectors,
	  PAR::token & first,
	  PAR::token next,
	  TAB::flags trace_flags,
	  OP::oper first_oper,
	  min::phrase_position & position )
{
    MIN_ASSERT ( first != next );

    OP::oper_pass oper_pass = (OP::oper_pass) pass;

    // As operators must be infix, operands and
    // operators must alternate with operands first and
    // last.

    // Work from beginning to end replacing
    //
    //	   operand1 operator operand2
    //
    // by
    //	   (operator operand1 operand2) next-operand1
    //
    // For the last operator, next-operand1 is omitted
    // and operand2 is not assigned a temporary.  For
    // all other cases final operand2 has the form
    // ($ T operand) and next-operand1 has the form
    // ($ T) where T is the next temporary variable
    // number.
    //
    for ( PAR::token operand1 = first;
          operand1 != next ; )
    {
	MIN_ASSERT ( operand1->type != PAR::OPERATOR );
        PAR::token op = operand1->next;
	MIN_ASSERT ( op != next );
	MIN_ASSERT ( op->type == PAR::OPERATOR );
        PAR::token operand2 = op->next;
	MIN_ASSERT ( operand2 != next );
	MIN_ASSERT ( operand2->type != PAR::OPERATOR );

        min::phrase_position position =
	    { operand1->position.begin,
	      operand2->position.end };

	// If not last operator, replace operand2 by
	// ($ T operand2 ), compute next-operand1 to be
	// ( $ T ), and insert it after operand2.
	//
	if ( operand2->next != next )
	{
	    // Insert tokens for $ and T before
	    // operand2.
	    //
	    min::phrase_position position2 =
	        operand2->position;
	    min::phrase_position before_position2 =
	        { operand2->position.begin,
	          operand2->position.begin };
	    PAR::token t =
	        PAR::new_token ( LEXSTD::word_t  );
	    PAR::value_ref ( t ) = OP::dollar;
	    t->position = before_position2;
	    PAR::put_before
		( first_ref(parser), operand2, t );

	    t = PAR::new_token ( LEXSTD::number_t  );
	    PAR::value_ref ( t ) =
	        min::new_num_gen
		    ( oper_pass->temporary_count ++ );
	    t->position = before_position2;
	    PAR::put_before
		( first_ref(parser), operand2, t );

	    // Copy tokens for $ and T after operand2.
	    //
	    PAR::token t2 = operand2->previous;
	    t = PAR::new_token ( t2->type );
	    PAR::value_ref ( t ) = t2->value;
	    t->position = before_position2;
	    PAR::put_before
		( first_ref(parser),
		  operand2->next, t );

	    t2 = operand2->previous->previous;
	    t = PAR::new_token ( t2->type );
	    PAR::value_ref ( t ) = t2->value;
	    t->position = before_position2;
	    PAR::put_before
		( first_ref(parser),
		  operand2->next, t );

	    // Compact new operand2 = ( $ T operand2 ).
	    //
	    PAR::attr oper_attr
		( PAR::dot_oper, OP::dollar );
	    t = operand2->next;
	    operand2 = operand2->previous->previous;
	    PAR::compact
		( parser, pass->next, selectors,
		  PAR::BRACKETABLE, trace_flags,
		  operand2, t, position2,
		  1, & oper_attr );

	    // Compact next-operand1 = ( $ T )
	    //
	    PAR::compact
		( parser, pass->next, selectors,
		  PAR::BRACKETABLE, trace_flags,
		  t, t->next->next, position2,
		  1, & oper_attr );
	}

	bool is_first = ( operand1 == first );
	PAR::token next_operand1 = operand2->next;

	// Switch operator and first operand.
	//
	PAR::remove ( PAR::first_ref ( parser ), op );
	PAR::put_before ( PAR::first_ref ( parser ),
	                  operand1, op );

	// Compact ( op operand1 operand2 )
	//
	PAR::attr oper_attr
	    ( PAR::dot_oper, op->value );
	PAR::compact
	    ( parser, pass->next, selectors,
	      PAR::BRACKETABLE, trace_flags,
	      op, next_operand1, position,
	      1, & oper_attr );
	if ( is_first ) first = op;

	operand1 = next_operand1;
    }

    if ( first->next != next )
    {
        // More than one operator.  Insert and_op.
	//
	min::gen and_op =
	    first_oper->reformatter_arguments[0];
	PAR::token t =
	    PAR::new_token ( PAR::OPERATOR  );
	PAR::value_ref ( t ) = and_op;
	t->position.begin = first->position.begin;
	t->position.end = first->position.begin;
	PAR::put_before ( first_ref(parser), first, t );
	first = t;

	// Compact.
	//
        min::phrase_position position =
	    { first->position.begin,
	      next->previous->position.end };
	PAR::attr oper_attr
	    ( PAR::dot_oper, and_op );
	PAR::compact
	    ( parser, pass->next, selectors,
	      PAR::BRACKETABLE, trace_flags,
	      first, next, position,
	      1, & oper_attr );
    }

    return false;
}

static bool sum_reformatter
        ( PAR::parser parser,
	  PAR::pass pass,
	  TAB::flags selectors,
	  PAR::token & first,
	  PAR::token next,
	  TAB::flags trace_flags,
	  OP::oper first_oper,
	  min::phrase_position & position )
{
    MIN_ASSERT ( first != next );

    // As operators must be infix, operands and
    // operators must alternate with operands first and
    // last.  The operators must be `+' and `-'.  The
    // `+'s are deleted and a `+' is inserted at the
    // beginning.  Any `- x' is made into a
    // subexpression with prefix `-'.

    // Remove all operators and replace operands x
    // preceeded by `-' with `- x' subexpressions.
    //
    for ( PAR::token t = first; t->next != next; )
    {
        MIN_ASSERT ( t->next->type == PAR::OPERATOR );
	min::gen op = t->next->value;
	MIN_ASSERT
	    ( op == PAR::plus || op == PAR::minus );
	if ( op == PAR::plus )
	{
	    PAR::free
		( PAR::remove
		      ( PAR::first_ref(parser),
			t->next ) );
	    t = t->next;
	}
	else
	{
	    t = t->next;
	    min::phrase_position position =
		{ t->position.begin,
		  t->next->position.end };
	    PAR::attr oper_attr
		( PAR::dot_oper, PAR::minus );
	    PAR::compact
		( parser, pass->next, selectors,
		  PAR::BRACKETABLE, trace_flags,
		  t, t->next->next, position,
		  1, & oper_attr );
	}
    }

    PAR::token new_first =
	PAR::new_token ( PAR::OPERATOR );
    new_first->position.begin = first->position.begin;
    new_first->position.end   = first->position.begin;
    PAR::value_ref ( new_first ) = PAR::plus;
    PAR::put_before
	( first_ref(parser), first, new_first );
    first = new_first;

    return true;
}

min::locatable_var<OP::reformatter_table_type>
    OP::reformatter_table;

static min::uns32 reformatter_table_gen_disp[] = {
    min::DISP ( & OP::reformatter_table_struct::name ),
    min::DISP_END };

static min::packed_vec<OP::reformatter_table_struct>
    reformatter_table_type
        ( "ll::parser::oper::reformatter_table_type",
	  ::reformatter_table_gen_disp );

static void reformatter_table_initialize ( void )
{
    OP::reformatter_table =
        ::reformatter_table_type.new_stub ( 32 );

    min::locatable_gen separator
        ( min::new_str_gen ( "separator" ) );
    OP::push_reformatter
        ( separator, OP::NOFIX,
	  ::separator_reformatter );
    min::locatable_gen declare
        ( min::new_str_gen ( "declare" ) );
    OP::push_reformatter
        ( declare, OP::NOFIX + OP::PREFIX + OP::INFIX,
	  ::declare_reformatter );

    min::locatable_gen right_associative
        ( min::new_lab_gen ( "right", "associative" ) );
    OP::push_reformatter
        ( right_associative, OP::INFIX,
	  ::right_associative_reformatter );

    min::locatable_gen prefix
        ( min::new_str_gen ( "unary" ) );
    OP::push_reformatter
        ( prefix, OP::PREFIX + OP::NOFIX,
	  ::unary_reformatter );

    min::locatable_gen binary
        ( min::new_str_gen ( "binary" ) );
    OP::push_reformatter
        ( binary, OP::INFIX + OP::NOFIX,
	  ::binary_reformatter );

    min::locatable_gen infix
        ( min::new_str_gen ( "infix" ) );
    OP::push_reformatter
        ( infix, OP::INFIX,
	  ::infix_reformatter );

    min::locatable_gen infix_and
        ( min::new_lab_gen ( "infix", "and" ) );
    OP::push_reformatter
        ( infix_and, OP::INFIX,
	  ::infix_and_reformatter );

    min::locatable_gen sum
        ( min::new_str_gen ( "sum" ) );
    OP::push_reformatter
        ( sum, OP::INFIX,
	  ::sum_reformatter );
}
static min::initializer reformatter_initializer
    ( ::reformatter_table_initialize );

// Operator Pass Command Function
// -------- ---- ------- --------

// Print op subroutine for print command.
//
enum table_type { OPERATOR, BRACKET, NAME };
void static print_op
	( OP::oper op,
	  ::table_type table_type,
	  PAR::parser parser )
{
    MIN_ASSERT ( op != min::NULL_STUB );

    min::gen block_name =
	PAR::block_name
	    ( parser,
	      op->block_level );
    parser->printer
	<< min::indent
	<< "block "
	<< min::name_pgen ( block_name )
	<< ": " << min::save_indent;

    if ( table_type == ::OPERATOR )
	parser->printer
	    << "operator \""
	    << min::name_pgen ( op->label );
    else if ( table_type == ::BRACKET )
	parser->printer
	    << "bracket \""
	    << min::name_pgen ( op->label )
	    << "\" ... \""
	    << min::name_pgen ( op->terminator );

    parser->printer
	<< "\" " << min::set_break;

    COM::print_flags
	( op->selectors,
	  parser->selector_name_table,
	  parser );

    parser->printer << min::indent;
    if ( op->flags & OP::PREFIX )
	parser->printer << "prefix";
    if ( op->flags & OP::INFIX )
	parser->printer
	    << min::space_if_after_indent
	    << "infix";
    if ( op->flags & OP::POSTFIX )
	parser->printer
	    << min::space_if_after_indent
	    << "postfix";
    if ( op->flags & OP::NOFIX )
	parser->printer
	    << min::space_if_after_indent
	    << "nofix";
    if ( op->flags & OP::AFIX )
	parser->printer
	    << min::space_if_after_indent
	    << "afix";

    parser->printer
	<< min::indent
	<< "with precedence "
	<< op->precedence;

    if ( op->reformatter != NULL )
    {
	min::gen name = OP::find_name
	    ( op->reformatter );
	parser->printer
	    << min::indent
	    << "with reformatter "
	    << min::name_pgen ( name );

	min::packed_vec_ptr<min::gen> args =
	    op->reformatter_arguments;
        if ( args != min::NULL_STUB )
	{
	    parser->printer << " ( " << min::set_break;
	    for ( min::uns32 i = 0; i < args->length;
	                            ++ i )
	    {
		if ( i != 0 )
		    parser->printer << ", "
		                    << min::set_break;
	        parser->printer
		    << '"' << min::name_pgen ( args[i] )
		    << '"';
	    }
	    parser->printer << " )";
	}
    }

    parser->printer
	<< min::restore_indent;
}

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
	          + ( 1ull << LEXSTD::number_t ),

	            ( 1ull << LEXSTD::
		                  horizontal_space_t )
	          + ( 1ull << LEXSTD::end_of_file_t ),

	            ( 1ull << LEXSTD::end_of_file_t ),
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
	    << min::bom << min::nohbreak
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
	    ( vp, i, selectors,
	      parser->selector_name_table, parser );
    if ( sresult == min::ERROR() )
	return min::ERROR();
    else if ( sresult == min::MISSING() )
	return PAR::parse_error
	    ( parser, ppvec[i-1],
	      "expected bracketed selector list"
	      " after" );

    // Scan operator flags.
    //
    min::uns32 oper_flags = 0;

    min::phrase_position oper_flags_position;
    oper_flags_position.begin = ppvec[i].begin;

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
		  "operator flag `",
		  min::name_pgen ( vp[i] ),
		  "' appears twice" );

	oper_flags |= new_oper_flag;
	++ i;
    }

    if ( oper_flags == 0 )
	return PAR::parse_error
	    ( parser, ppvec[i-1],
	      "expected operator flags after" );

    oper_flags_position.end = ppvec[i-1].end;

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
    OP::reformatter reformatter = NULL;
    min::locatable_var
    	    < min::packed_vec_ptr<min::gen> >
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
	    if ( i >= size
		 ||
		 ! min::strto ( precedence,
				vp[i], 10 ) )
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
	            ( vp, j, ::reformatter ) );
	    if (    j < size
		 && vp[j] == ::reformatter )
	    {
		min::uns32 allowed_oper_flags;
		reformatter =
		    OP::find_reformatter
		        ( name, allowed_oper_flags );
		if ( reformatter == NULL )
		{
		    min::phrase_position position =
			{ ppvec[i].begin,
			  ppvec[j-1].end };
		    return PAR::parse_error
			( parser, position,
			  "undefined reformatter"
			  " name" );
		}

		min::uns32 illegal_flags =
		    oper_flags & ~ allowed_oper_flags;
		if ( illegal_flags != 0 )
		{
		    char buffer[200];
		    char * s = buffer;
		    s += sprintf
		        ( s, "' reformatter"
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
			      "`",
			      min::name_pgen ( name ),
			      buffer );
		}

		i = j + 1;

		name = COM::scan_names
		    ( vp, i, reformatter_arguments,
		          parser );
		if ( name == min::ERROR() )
		    return min::ERROR();

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
	OP::push_oper
	    ( name[0], name[1],
	      selectors,
	      PAR::block_level ( parser ),
	      ppvec->position,
	      oper_flags, precedence, reformatter,
	      reformatter_arguments,
	      bracket || indentation_mark ?
	          oper_pass->oper_bracket_table :
		  oper_pass->oper_table );

    else // if ( command == PAR::undefine )
    {
	if ( reformatter != NULL )
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
