// Layered Languages Operator Parser Pass
//
// File:	ll_parser_oper.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri Dec  3 23:55:28 EST 2021
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Operator Table Entries
//	Operator Parser Pass
//	Operator Parsing Functions
//	Operator Parse Function
//	Operator Reformatters
//	Operator Pass Command Function

// Usage and Setup
// ----- --- -----

# include <ll_lexeme_standard.h>
# include <ll_parser_standard.h>
# include <ll_parser_command.h>
# include <ll_parser_oper.h>
# include <cstdio>
# define LEX ll::lexeme
# define LEXSTD ll::lexeme::standard
# define PAR ll::parser
# define PARLEX ll::parser::lexeme
# define PARSTD ll::parser::standard
# define TAB ll::parser::table
# define COM ll::parser::command
# define OP ll::parser::oper
# define OPLEX ll::parser::oper::lexeme

min::locatable_gen OPLEX::dollar;
min::locatable_gen OPLEX::AND;
min::locatable_gen OPLEX::initial;
min::locatable_gen OPLEX::left;
min::locatable_gen OPLEX::right;
min::locatable_gen OPLEX::final;
min::locatable_gen OPLEX::afix;
min::locatable_gen OPLEX::line;
min::locatable_gen OPLEX::prefix;
min::locatable_gen OPLEX::infix;
min::locatable_gen OPLEX::postfix;
min::locatable_gen OPLEX::nofix;
min::locatable_gen OPLEX::end_operator;
min::locatable_gen OPLEX::error_operator;
min::locatable_gen OPLEX::error_operand;
min::locatable_gen OPLEX::error_separator;
min::locatable_gen OPLEX::operator_subexpressions;
min::locatable_gen OPLEX::oper;
min::locatable_gen OPLEX::bracket;
min::locatable_gen OPLEX::indentation;
min::locatable_gen OPLEX::mark;
min::locatable_gen OPLEX::precedence;
min::locatable_gen OPLEX::operators;
min::locatable_gen OPLEX::has_condition;
min::locatable_gen OPLEX::control;

static void init_end_oper ( void );
static void init_error_oper ( void );
static void initialize ( void )
{
    OPLEX::dollar  = min::new_str_gen ( "$" );
    OPLEX::AND     = min::new_str_gen ( "AND" );
    OPLEX::initial  = min::new_str_gen ( "initial" );
    OPLEX::left  = min::new_str_gen ( "left" );
    OPLEX::right  = min::new_str_gen ( "right" );
    OPLEX::final  = min::new_str_gen ( "final" );
    OPLEX::afix    = min::new_str_gen ( "afix" );
    OPLEX::line    = min::new_str_gen ( "line" );
    OPLEX::prefix  = min::new_str_gen ( "prefix" );
    OPLEX::infix   = min::new_str_gen ( "infix" );
    OPLEX::postfix = min::new_str_gen ( "postfix" );
    OPLEX::nofix   = min::new_str_gen ( "nofix" );
    OPLEX::end_operator =
        min::new_str_gen ( "END'OPERATOR" );
    OPLEX::error_operator =
        min::new_str_gen ( "ERROR'OPERATOR" );
    OPLEX::error_operand =
        min::new_str_gen ( "ERROR'OPERAND" );
    OPLEX::error_separator =
        min::new_str_gen ( "ERROR'SEPARATOR" );

    OPLEX::operator_subexpressions =
        min::new_lab_gen
	    ( "operator", "subexpressions" );
    OPLEX::oper = min::new_str_gen ( "operator" );
    OPLEX::bracket = min::new_str_gen ( "bracket" );
    OPLEX::indentation =
        min::new_str_gen ( "indentation" );
    OPLEX::mark = min::new_str_gen ( "mark" );
    OPLEX::precedence =
        min::new_str_gen ( "precedence" );
    OPLEX::operators = min::new_str_gen ( "operators" );
    OPLEX::has_condition =
        min::new_lab_gen ( "has", "condition" );
    OPLEX::control = min::new_str_gen ( "control" );

    ::init_end_oper();
    ::init_error_oper();

    PAR::push_new_pass ( OPLEX::oper, OP::new_pass );
}
static min::initializer initializer ( ::initialize );

// Operator Table Entries
// -------- ----- -------

static min::uns32 oper_gen_disp[] = {
    min::DISP ( & OP::oper_struct::label ),
    min::DISP ( & OP::oper_struct::terminator ),
    min::DISP ( & OP::oper_struct
                    ::reformatter_arguments ),
    min::DISP_END };

static min::uns32 oper_stub_disp[] = {
    min::DISP ( & OP::oper_struct::next ),
    min::DISP ( & OP::oper_struct::reformatter ),
    min::DISP_END };

static min::packed_struct_with_base
	<OP::oper_struct, TAB::root_struct>
    oper_type ( "ll::parser::oper::oper_type",
	        ::oper_gen_disp,
	        ::oper_stub_disp );
const min::uns32 & OP::OPER = ::oper_type.subtype;

min::locatable_var<OP::oper> OP::end_oper;
min::locatable_var<OP::oper> OP::error_oper;

void OP::push_oper
	( min::gen oper_label,
	  min::gen terminator,
	  TAB::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  min::uns32 flags,
	  min::int32 precedence,
	  PAR::reformatter reformatter,
	  min::gen reformatter_arguments,
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
    reformatter_ref(oper) = reformatter;
    reformatter_arguments_ref(oper) =
	reformatter_arguments;

    TAB::push ( oper_table, (TAB::root) oper );
}

static void init_end_oper ( void )
{
    if ( OP::end_oper != min::NULL_STUB ) return;
    OP::end_oper = ::oper_type.new_stub();
    label_ref(OP::end_oper) = OPLEX::end_operator;
    terminator_ref(OP::end_oper) = min::MISSING();
    OP::end_oper->selectors = 0;
    OP::end_oper->block_level = 0;
    OP::end_oper->position = PAR::top_level_position;
    OP::end_oper->flags = OP::NOFIX;
    OP::end_oper->precedence =
        OP::low_precedence - 2;
    reformatter_ref(OP::end_oper) = min::NULL_STUB;
    reformatter_arguments_ref(OP::end_oper) =
        min::MISSING();
}

static void init_error_oper ( void )
{
    if ( OP::error_oper != min::NULL_STUB ) return;
    OP::error_oper = ::oper_type.new_stub();
    label_ref(OP::error_oper) = OPLEX::error_operator;
    terminator_ref(OP::error_oper) = min::MISSING();
    OP::error_oper->selectors = 0;
    OP::error_oper->block_level = 0;
    OP::error_oper->position = PAR::top_level_position;
    OP::error_oper->flags = OP::NOFIX;
    OP::error_oper->precedence =
        OP::low_precedence - 1;
    reformatter_ref(OP::error_oper) = min::NULL_STUB;
    reformatter_arguments_ref(OP::error_oper) =
        min::MISSING();
}

// Operator Parser Pass
// -------- ------ ----

static min::packed_vec< OP::oper_stack_struct >
    oper_stack_type
        ( "ll::parser::oper::oper_stack_type" );

static min::packed_vec< OP::oper_vec_struct >
    oper_vec_type
        ( "ll::parser::oper::oper_vec_type" );

static min::uns32 oper_pass_gen_disp[] =
{
    min::DISP ( & OP::oper_pass_struct::name ),
    min::DISP_END
};

static min::uns32 oper_pass_stub_disp[] =
{
    min::DISP ( & OP::oper_pass_struct::parser ),
    min::DISP ( & OP::oper_pass_struct::next ),
    min::DISP ( & OP::oper_pass_struct::oper_table ),
    min::DISP ( & OP::oper_pass_struct
                    ::oper_bracket_table ),
    min::DISP ( & OP::oper_pass_struct::oper_stack ),
    min::DISP ( & OP::oper_pass_struct::oper_vec ),
    min::DISP_END
};

static min::packed_struct_with_base
	<OP::oper_pass_struct, PAR::pass_struct>
    oper_pass_type
        ( "ll::parser::oper::oper_pass_type",
	  ::oper_pass_gen_disp,
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
	  OPLEX::operator_subexpressions );
    MIN_REQUIRE
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
    MIN_REQUIRE ( block_level > 0 );
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
	  min::obj_vec_ptr & vp, min::uns32 i0,
          min::phrase_position_vec ppvec );

PAR::pass OP::new_pass ( PAR::parser parser )
{
    min::locatable_var<OP::oper_pass> oper_pass
        ( ::oper_pass_type.new_stub() );

    OP::name_ref ( oper_pass ) = OPLEX::oper;

    OP::oper_table_ref ( oper_pass ) =
        TAB::create_key_table ( 1024 );

    OP::oper_bracket_table_ref ( oper_pass ) =
        TAB::create_key_table ( 256 );

    OP::oper_vec_ref ( oper_pass ) =
	::oper_vec_type.new_stub ( 100 );

    OP::oper_stack_ref ( oper_pass ) =
	::oper_stack_type.new_stub ( 100 );

    oper_pass->parser_command = ::oper_pass_command;
    oper_pass->parse = ::oper_parse;
    oper_pass->place = ::oper_pass_place;
    oper_pass->reset = ::oper_pass_reset;
    oper_pass->end_block = ::oper_pass_end_block;

    return (PAR::pass) oper_pass;
}

OP::oper_pass OP::init_oper
	( PAR::parser parser,
	  PAR::pass next )
{
    PAR::pass pass =
        PAR::find_on_pass_stack ( parser, OPLEX::oper );
    if ( pass != min::NULL_STUB )
        return (OP::oper_pass) pass;

    min::locatable_var<OP::oper_pass> oper_pass
        ( (OP::oper_pass) OP::new_pass ( parser ) );
    PAR::place_before
        ( parser, (PAR::pass) oper_pass, next );
    return oper_pass;
}

// Operator Parsing Functions
// -------- ------- ---------

bool afix_OK ( OP::oper_vec v,
	       min::int32 precedence )
{
    for ( min::uns32 i = v->length; i > 0; )
    {
	OP::oper_vec_struct last = v[--i];
	if (    last.op != min::NULL_STUB )
	{
	    if ( precedence == last.precedence )
	        return true;
	    if ( precedence > last.precedence )
	        return false;
	}
    }
    MIN_ABORT ( "afix_OK did not find end_oper" );
}
        
bool OP::flags_OK ( OP::oper_vec v,
	            min::uns32 flags,
		    min::int32 precedence,
		    OP::oper op )
{
    min::uns32 length = v->length;
    MIN_REQUIRE ( length >= 1 );
    OP::oper_vec_struct last = v[length-1];

    if ( last.op == min::NULL_STUB )
    {
	if ( op == min::NULL_STUB ) return false;
	else if ( flags & OP::INITIAL ) return false;
	goto OK;
    }
    else if ( last.flags & OP::RIGHT )
    {
        if ( op == min::NULL_STUB ) goto OK;
	else if ( flags & OP::LEFT ) return false;
	else if ( flags & OP::INITIAL )
	{
	    if ( last.precedence > precedence )
	        return false;
	    else if ( last.precedence < precedence )
	        goto OK;
	    else if ( last.flags & flags & OP::INITIAL )
	        goto OK;
	    else return false;
	}
	else
	{
	    if ( last.precedence >= precedence )
	        return false;
	    else
	        goto OK;
	}
    }
    else if ( last.flags & OP::FINAL )
    {
        if ( op == min::NULL_STUB ) return false;
	else if ( flags & OP::INITIAL ) return false;
	else if ( flags & OP::LEFT )
	{
	    if ( last.precedence > precedence )
	        goto OK;
	    else if ( last.precedence < precedence )
	        return false;
	    else if ( last.flags & flags & OP::FINAL )
	        goto OK;
	    else return false;
	}
	else
	{
	    if ( last.precedence <= precedence )
	        return false;
	    else
	        goto OK;
	}
    }
    else // last.flags is nofix
    {
        if ( op == min::NULL_STUB ) goto OK;
	else if ( flags & OP::INITIAL )
	{
	    if ( last.precedence < precedence )
	        goto OK;
	    else return false;
	}
	else if ( flags & OP::LEFT )
	{
	    if ( last.precedence > precedence )
	        goto OK;
	    else return false;
	}
	else goto OK;
    }

OK:
    // Come here when we are going to return true if
    // AFIX flag allows.
    //
    if (    ( flags & OP::AFIX )
         && ! afix_OK ( v, precedence ) )
	return false;
    OP::oper_vec_struct next =
        { flags, precedence, op };
    min::push ( v ) = next;

    return true;
}


// Operator Parse Function
// -------- ----- --------

min::int32 OP::high_precedence = +1e6;
min::int32 OP::low_precedence  = -1e6;
min::int32 OP::prefix_precedence  = +1e6 - 1;
min::int32 OP::postfix_precedence  = +1e6;

static void put_error_operator_before
	( ll::parser::parser parser,
	  ll::parser::token t,
          OP::oper_vec vec )
{
    PAR::token token = new_token ( PAR::OPERATOR );
    put_before ( PAR::first_ref(parser), t, token );
    PAR::value_ref ( token ) = OPLEX::error_operator;

    min::phrase_position position =
        { t->position.begin, t->position.begin };
    token->position = position;

    PAR::parse_error
	( parser, t->position,
	  "",
	  min::printf_op<200>
	      ( "missing operator; nofix error operator"
		" of precedence %d inserted",
	        OP::low_precedence - 1 ) );

    bool OK = OP::flags_OK
	( vec, OP::NOFIX,
	       OP::low_precedence - 1,
	       OP::error_oper );

    MIN_REQUIRE ( OK );

}

void OP::put_error_operand_before
	( ll::parser::parser parser,
	  ll::parser::token t )
{
    PAR::token token = new_token ( LEXSTD::word_t );
    put_before ( PAR::first_ref(parser), t, token );
    PAR::value_ref ( token ) = OPLEX::error_operand;

    min::phrase_position position =
        { t->position.begin, t->position.begin };
    token->position = position;

    PAR::parse_error
	( parser, token->position,
	  "missing operand; error operand inserted" );
}

void OP::put_error_operand_after
	( ll::parser::parser parser,
	  ll::parser::token t )
{
    PAR::token token = new_token ( LEXSTD::word_t );
    PAR::put_before
        ( PAR::first_ref(parser), t->next, token );
    PAR::value_ref ( token ) = OPLEX::error_operand;

    min::phrase_position position =
        { t->position.end, t->position.end };
    token->position = position;

    PAR::parse_error
	( parser, token->position,
	  "missing operand; error operand inserted" );
}

PAR::token OP::delete_bad_token
	( ll::parser::parser parser,
	  ll::parser::token t,
	  const char * message )
{
    PAR::parse_error
	( parser, t->position, message,
	  min::pgen_quote ( t->value ),
	  "; deleted" );

    t = t->next;
    PAR::free
	( PAR::remove
	      ( PAR::first_ref ( parser ),
		t->previous ) );
    return t;
}

PAR::token OP::delete_extra_stuff
	( ll::parser::parser parser,
	  ll::parser::token t,
	  ll::parser::token next )
{
    min::phrase_position position =
	{ t->position.begin,
	  next->previous->position.end };

    PAR::parse_error
	( parser, position,
	  "extra stuff at end of"
	   " expression; deleted" );

    while ( t != next )
    {
	t = t->next;
	PAR::free
	    ( PAR::remove
		  ( PAR::first_ref ( parser ),
		    t->previous ) );
    }

    return next;
}

min::printer operator <<
	( min::printer p, OP::oper op )
{
    if ( op == min::NULL_STUB )
        return p << "NULL_STUB";
    p << min::pgen_quote ( op->label )
      << "[" << op->precedence;
    if ( op->flags & OP::INITIAL )
        p << ", INITIAL";
    if ( op->flags & OP::LEFT )
        p << ", LEFT";
    if ( op->flags & OP::RIGHT )
        p << ", RIGHT";
    if ( op->flags & OP::FINAL )
        p << ", FINAL";
    if ( op->flags & OP::AFIX )
        p << ", AFIX";
    if ( op->flags & OP::LINE )
        p << ", LINE";
    return p << "]";
}

void trace_operator ( PAR::parser parser,
		      const min::phrase_position & pos,
		      OP::oper op, bool accepted )
{
    parser->printer
	<< min::bom
	<< min::adjust_indent ( 7 )
	<< ( accepted ? "ACCEPTED " : "REJECTED " )
	<< "OPERATOR " << op << "; "
	<< min::pline_numbers
	       ( parser->input_file, pos )
	<< ":" << min::eom;
    min::print_phrase_lines
	( parser->printer,
	  parser->input_file,
	  pos );
}

// Returns true if operator found, false otherwise.
//
static bool oper_parse_pass_1 ( PAR::parser parser,
		                OP::oper_pass oper_pass,
		                TAB::flags selectors,
		                PAR::token & first,
		                PAR::token next,
				OP::oper_vec vec,
				TAB::flags trace_flags )
{
    OP::oper_vec_struct end_op =
        { OP::NOFIX, OP::low_precedence - 2,
	             OP::end_oper };
    min::push ( vec ) = end_op;
    PAR::token current = first;
    PAR::token non_op_first = min::NULL_STUB;
    bool is_line =
        ( selectors & PAR::LINE_LEVEL_SELECTOR );
    while ( current != next )
    {
	// Find operator if possible.
	//
	TAB::root root = min::NULL_STUB;

        // If root != NULL_STUB, oper equals root,
	// flags and precedence are oper's parameters,
	// OK is the result of checking LINE flag and
	// calling flags_OK for oper, and current points
	// at the token that is the OPERATOR (after its
	// type is changed to OPERATOR).
	//
	OP::oper oper;
	min::uns32 flags;
	min::int32 precedence;
	bool OK;

	bool bracketed =
	    ( current->type == PAR::BRACKETED );
	    
	if ( bracketed )
	{
	    if ( min::is_name
		     ( current->value_type ) )
		root = TAB::find
		    ( current->value_type,
		      OP::OPER,
		      selectors,
		      oper_pass->
			  oper_bracket_table );

	    while ( root != min::NULL_STUB )
	    {
		if ( root->selectors & selectors )
		{
		    oper = (OP::oper) root;
		    flags = oper->flags;
		    precedence = oper->precedence;
		    if (    ( flags & OP::LINE )
		         && ! is_line )
		        OK = false;
		    else
			OK = OP::flags_OK
			    ( vec, flags, precedence,
				   oper );
		    if ( trace_flags & PAR::TRACE_KEYS )
			trace_operator
			    ( parser, current->position,
			      oper, OK );
		    if ( OK ) break;
		}
		root = root->next;
	    }
	}
	else
	{
	    TAB::key_prefix key_prefix;
	    PAR::token next_current = current;
	    root = PAR::find_entry
		( parser, next_current, key_prefix,
		  selectors, oper_pass->oper_table,
		  next );

	    min::phrase_position pos =
		{ current->position.begin,
		  next_current->previous
		              ->position.end };

	    while ( root != min::NULL_STUB )
	    {
		oper = (OP::oper) root;
		flags = oper->flags;
		precedence = oper->precedence;
		if (    ( flags & OP::LINE )
		     && ! is_line )
		    OK = false;
		else
		    OK = OP::flags_OK
			( vec, flags, precedence,
			       oper );
		if ( trace_flags & PAR::TRACE_KEYS )
		    trace_operator
		        ( parser, pos, oper, OK );
		if ( OK ) break;
		root = PAR::find_next_entry
		    ( parser, next_current,
			      key_prefix,
			      selectors, root );
		pos.end = next_current->previous
		                      ->position.end;
	    }

	    if ( root != min::NULL_STUB )
	    {
		// Delete tokens in operator except
		// for the first, which is at current,
		// and may == first.
		//
	        while ( current->next != next_current )
		    PAR::free
			( PAR::remove
			      ( PAR::first_ref (parser),
				next_current->
				    previous ) );
		value_ref ( current ) = root->label;
	    }
	}

	if ( root == min::NULL_STUB )
	{
	    if ( non_op_first == min::NULL_STUB )
	    {
	        non_op_first = current;
		OK = OP::flags_OK ( vec );
		if ( ! OK )
		{
		    ::put_error_operator_before
		        ( parser, current, vec );
		    OK = OP::flags_OK ( vec );
		    MIN_REQUIRE ( OK );
		}
	    }
	    current = current->next;
	    if ( current != next )
		continue;
	    else if ( non_op_first == first )
	        return false;
	}

	if ( non_op_first != min::NULL_STUB )
	{
	    min::phrase_position position =
		{ non_op_first->position.begin,
		  current->previous->position.end };

	    bool is_first = ( non_op_first == first );

	    PAR::compact ( parser, oper_pass->next,
			   selectors,
			   non_op_first, current,
			   position,
			   trace_flags,
			   PAR::BRACKETABLE );

	    if ( is_first ) first = non_op_first;
	    non_op_first = min::NULL_STUB;
	}

	if ( root == min::NULL_STUB ) continue;

	MIN_REQUIRE ( OK );

	current->type = PAR::OPERATOR;

	current = current->next;
    }

    bool OK = OP::flags_OK
		  ( vec, OP::NOFIX,
		    OP::low_precedence - 2,
		    OP::end_oper );
    if ( ! OK )
    {
	OP::put_error_operand_before
	    ( parser, current );
	OK = OP::flags_OK ( vec );
	MIN_REQUIRE ( OK );
        OK = OP::flags_OK
	      ( vec, OP::NOFIX,
		OP::low_precedence - 2,
		OP::end_oper );
	MIN_REQUIRE ( OK );
    }

    return true;
}

void compact_expression ( PAR::parser parser,
			  OP::oper_pass oper_pass,
			  TAB::flags selectors,
			  PAR::token & first,
			  PAR::token next,
			  OP::oper oper,
			  TAB::flags trace_flags )
{
    min::phrase_position position =
        { first->position.begin,
	  next->previous->position.end };

    if ( oper->reformatter == min::NULL_STUB
	 ||
	 ( * oper
	       ->reformatter
	       ->reformatter_function )
	     ( parser, (PAR::pass) oper_pass, selectors,
	       first, next,
	       position, min::MISSING(),
	       trace_flags,
	       (TAB::root) oper )
       )
	PAR::compact
	    ( parser, min::NULL_STUB,
	      selectors,
	      first, next,
	      position,
	      trace_flags,
	      PAR::BRACKETABLE );
}

static void oper_parse_pass_2 ( PAR::parser parser,
		                OP::oper_pass oper_pass,
		                TAB::flags selectors,
		                PAR::token & first,
		                PAR::token next,
				OP::oper_vec vec,
				min::unsptr vec_origin,
				TAB::flags trace_flags )
{

    // We add to the stack but leave alone what is
    // already in the stack so this function can be
    // called recursively.
    //
    OP::oper_stack oper_stack = oper_pass->oper_stack;
    min::unsptr stack_origin = oper_stack->length;

    // Data that is pushed to oper_stack.  D is in
    // effect the top of the stack.
    //
    OP::oper_stack_struct D;
    D.first = first;
    D.precedence = OP::low_precedence - 2;
    D.first_oper = min::NULL_STUB;

    PAR::token current = D.first;
    min::uns32 index = vec_origin + 1;
    while ( true )
    {
        OP::oper_vec_struct v;
	v = vec[index++];

	if ( v.op == min::NULL_STUB )
	{
	    current = current->next;
	    continue;
	}

	// Close previous subexpressions as long as
	// D.precedence > v.precedence.
	//
	while ( D.precedence > v.precedence )
	{
	    OP::oper oper = D.first_oper;
	    D = min::pop ( oper_stack );
	    TAB::flags compact_trace_flags =
	        D.first != first || current != next ?
		trace_flags : 0;
	    compact_expression
	        ( parser, oper_pass, selectors,
		  D.first, current,
		  oper, compact_trace_flags );
	}

	if ( v.flags & OP::FINAL )
	{
	    current = current->next;
	    OP::oper oper = v.op;
	    if ( D.precedence == v.precedence )
	    {
	        oper = D.first_oper;
		D = min::pop ( oper_stack );
	    }
	    TAB::flags compact_trace_flags =
	        D.first != first || current != next ?
		trace_flags : 0;
	    compact_expression
	        ( parser, oper_pass, selectors,
		  D.first, current,
		  oper, compact_trace_flags );
	    continue;
	}

	if ( current == next ) break;

	if ( D.precedence < v.precedence
	     ||
	     v.flags & OP::INITIAL )
	{
	    min::push ( oper_stack ) = D;
	    D.precedence = v.precedence;
	    D.first_oper = v.op;
	}
	current = current->next;
	D.first = current;
    }

    MIN_REQUIRE (stack_origin == oper_stack->length );

    first = D.first;
}

static void oper_parse ( PAR::parser parser,
		         PAR::pass pass,
		         TAB::flags selectors,
		         PAR::token & first,
		         PAR::token next )
{
    OP::oper_pass oper_pass = (OP::oper_pass) pass;

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

    if ( trace_flags & PAR::TRACE_KEYS )
    {
	min::phrase_position pos =
	    { first->position.begin,
	      next->previous->position.end };

	parser->printer
	    << min::bom
	    << min::adjust_indent ( 4 )
	    << "OPERATOR PASS ";
        
	COM::print_flags
	    ( selectors, PAR::COMMAND_SELECTORS,
	      parser->selector_name_table,
	      parser );

	parser->printer
	    << " "
	    << min::pline_numbers
		   ( parser->input_file, pos )
	    << ":" << min::eom;
	min::print_phrase_lines
	    ( parser->printer,
	      parser->input_file,
	      pos );

    }

    // We add to the vector but leave alone what is
    // already in the vector so this function can be
    // called recursively.
    //
    OP::oper_vec vec = oper_pass->oper_vec;
    min::unsptr vec_origin = vec->length;

    bool op_found = oper_parse_pass_1
	( parser, oper_pass, selectors, first, next,
	  vec, trace_flags );
    if ( op_found )
	oper_parse_pass_2
	    ( parser, oper_pass, selectors, first, next,
	      vec, vec_origin, trace_flags );

    min::pop ( vec, vec->length - vec_origin );

}



// Operator Reformatters
// -------- ------------

static bool control_reformatter_function
        ( PAR::parser parser,
	  PAR::pass pass,
	  TAB::flags selectors,
	  PAR::token & first,
	  PAR::token next,
	  const min::phrase_position & position,
	  min::gen line_separator,
	  TAB::flags trace_flags,
	  TAB::root entry )
{
    OP::oper op = (OP::oper) entry;
    min::obj_vec_ptr args ( op->reformatter_arguments );
    MIN_REQUIRE ( min::size_of ( args ) >= 1 );

    while (    first != next
            && first->type != PAR::OPERATOR )
        first = OP::delete_bad_token
	    ( parser, first, "expected an operator"
			     " and found an operand " );
    MIN_ASSERT ( first != next,
		 "expression must have an operator" );

    PAR::token t = first->next;

    if ( t == next )
    {
	PAR::parse_error
	    ( parser, t->previous->position,
	      "expression ends prematurely after" );
	return true;
    }

    if (    min::size_of ( args ) >= 2
         && args[1] == OPLEX::has_condition )
    {
	if ( t->type == PAR::OPERATOR )
	    OP::put_error_operand_before ( parser, t );
	else
	{
	    t = t->next;
	    if ( t == next )
	    {
		PAR::parse_error
		    ( parser, t->previous->position,
		      "expression ends prematurely"
		      " after" );
		return true;
	    }
	    MIN_ASSERT ( t->type == PAR::OPERATOR,
	                 "expected an operator and"
			 " found an operand" );
	}
    }
    else if ( t->type != PAR::OPERATOR )
    {
        t = OP::delete_bad_token
	    ( parser, t, "expected an operator"
			     " and found an operand " );
	if ( t == next )
	{
	    PAR::parse_error
		( parser, t->previous->position,
		  "expression ends prematurely"
		  " after" );
	    return true;
	}
	MIN_ASSERT ( t->type == PAR::OPERATOR,
		     "expected an operator and"
		     " found an operand" );
    }

    if ( ! min::is_obj ( t->value )
         &&
	 t->value != args[0] )
    {
        PAR::parse_error
	    ( parser, t->previous->position,
	      "expected `",
	      min::pgen_never_quote ( args[0] ),
	      "' operator but found `",
	      min::pgen_never_quote ( t->value ),
	      "' operator; operator changed to `",
	      min::pgen_never_quote ( args[0] ),
	      "' operator" );
	PAR::value_ref ( t ) = args[0];
    }

    if ( t->value == args[0] )
    {
	t = t->next;
	if ( t == next )
	{
	    PAR::parse_error
		( parser, t->previous->position,
		  "expected statement after" );
	    OP::put_error_operand_before ( parser, t );
	    return true;
	}
	else if ( t->type == PAR::OPERATOR )
	{
	    PAR::parse_error
		( parser, t->position,
		  "expected an operand and found an"
		  " operator" );
            t = OP::delete_extra_stuff
	        ( parser, t, next );
	    return true;
	}
    }
    else if (    min::get ( t->value,
		            min::dot_terminator )
	      != min::INDENTED_PARAGRAPH()
	      ||
                 min::get ( t->value,
		            min::dot_initiator )
	      != args[0] )
    {
	PAR::parse_error
	    ( parser, t->position,
	      "expected `",
	      min::pgen_never_quote ( args[0] ),
	      "' operator or indented paragraph but"
	      " found different operator" );
	t = OP::delete_extra_stuff ( parser, t, next );
	return true;
    }

    t = t->next;

    // Delete extra stuff from end of list.
    //
    if ( t != next )
        t = OP::delete_extra_stuff ( parser, t, next );

    return true;
}

static bool declare_reformatter_function
        ( PAR::parser parser,
	  PAR::pass pass,
	  TAB::flags selectors,
	  PAR::token & first,
	  PAR::token next,
	  const min::phrase_position & position,
	  min::gen line_separator,
	  TAB::flags trace_flags,
	  TAB::root entry )
{
    MIN_REQUIRE ( first != next );

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
	( t != next && t->type == PAR::OPERATOR,
	  "second element is missing or not operator" );

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
	     min::is_name ( t->value ) )
	{
	    PAR::parse_error
	        ( parser, t->position,
		  "expected bracketed expression and"
		  " got ",
		  min::pgen_quote ( t->value ),
		  "; deleted" );

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

static bool separator_reformatter_function
        ( PAR::parser parser,
	  PAR::pass pass,
	  TAB::flags selectors,
	  PAR::token & first,
	  PAR::token next,
	  const min::phrase_position & position,
	  min::gen line_separator,
	  TAB::flags trace_flags,
	  TAB::root entry )
{
    MIN_REQUIRE ( first != next );

    bool separator_should_be_next = false;
        // Equivalent meaning: the last token was an
	// operand.
    min::gen separator = min::NONE();
    min::phrase_position separator_position;
    for ( PAR::token t = first; t != next; )
    {
        if ( t->type == PAR::OPERATOR )
	{
	    if ( separator == min::NONE() )
	    {
	        separator = t->value;
		separator_position = t->position;

		if ( ! min::is_name ( separator ) )
		{
		    PAR::parse_error
			( parser, separator_position,
			  "separator ",
			  min::pgen_quote ( separator ),
			  " is not a name; changed to ",
			  min::pgen_quote
			      ( OPLEX::error_separator )
			);
		    separator = OPLEX::error_separator;
		}
	    }
	    else if ( separator != t->value )
	        PAR::parse_error
		    ( parser, t->position,
		      "wrong separator ",
		      min::pgen_quote ( t->value ),
		      " changed to ",
		      min::pgen_quote ( separator ) );

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
		// position, which will be copied to
		// the empty token.
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
	    MIN_ASSERT ( ! separator_should_be_next,
	                 "separator expected but"
			 " operand found" );
	        // Two operands should never be next to
		// each other.
	    separator_should_be_next = true;
	    t = t->next;
	}
    }

    PAR::attr separator_attr
        ( min::dot_separator,
	  separator );

    PAR::compact
        ( parser, pass->next, selectors,
	  first, next, position,
	  trace_flags, PAR::BRACKETABLE,
	  1, & separator_attr );

    return false;
}

static bool selector_reformatter_function
        ( PAR::parser parser,
	  PAR::pass pass,
	  TAB::flags selectors,
	  PAR::token & first,
	  PAR::token next,
	  const min::phrase_position & position,
	  min::gen line_separator,
	  TAB::flags trace_flags,
	  TAB::root entry )
{
    MIN_REQUIRE ( first != next );

    OP::oper op = (OP::oper) entry;
    min::obj_vec_ptr args ( op->reformatter_arguments );

    min::gen first_op = args[0];
    min::gen second_op = args[1];

    // Operators and operands must alternate with
    // operands at the ends.  The operators must
    // alternate between the first and second operator,
    // with there being an even number of operators.
    
    PAR::token t = first;
    bool even = true;
    while ( true )
    {
	MIN_REQUIRE ( t != next );
	if ( t->type == PAR::OPERATOR )
	{
	    OP::put_error_operand_before
	        ( parser, t );
	    if ( first == t ) first = t->previous;
	}
	else
	{
	    t = t->next;
	    if ( t == next ) break;
	}

        MIN_REQUIRE ( t->type == PAR::OPERATOR);

	min::gen op = t->value;
	min::gen desired_op =
	    even ? first_op : second_op;
	if ( op != desired_op )
	{
	    PAR::parse_error
		( parser, t->position,
		  "wrong operator ",
		  min::pgen_quote ( op ),
		  " changed to ",
		  min::pgen_quote ( desired_op ) );
	    PAR::value_ref ( t ) = desired_op;
	}

	t = t->next;
	if ( t == next )
	{
	    OP::put_error_operand_after
	        ( parser, t->previous );
	    break;
	}
	even = ! even;
    }

    if ( ! even )
    {
	PAR::token token = new_token ( PAR::OPERATOR );
	put_before ( PAR::first_ref(parser), t, token );
	PAR::value_ref ( token ) = second_op;

	min::phrase_position position =
	    { t->position.begin, t->position.begin };
	token->position = position;

	PAR::parse_error
	    ( parser, position,
	      "missing operator ",
	      min::pgen_quote ( second_op ),
	      " inserted" );

	OP::put_error_operand_before ( parser, t );
    }

    return true;
}

// Do checking for infix, right associative, and left
// associative reformatters.
//
static void infix_check
        ( PAR::parser parser,
	  PAR::token first,
	  PAR::token next,
	  TAB::root entry )
{
    MIN_REQUIRE ( first != next );

    OP::oper oper = (OP::oper) entry;
    min::obj_vec_ptr args
        ( oper->reformatter_arguments );
    min::uns32 length =
        ( args == min::NULL_STUB ? 0 :
	  min::size_of ( args ) );
    
    PAR::token t = first;
    while ( true )
    {
        if ( t == next )
	{
	    PAR::parse_error
		( parser, t->previous->position,
		  "operator should be infix" );
	    OP::put_error_operand_before ( parser, t );
	    return;
	}
	else if ( t->type == PAR::OPERATOR )
	{
	    PAR::parse_error
		( parser, t->position,
		  "operator should be infix" );
	    OP::put_error_operand_before ( parser, t );
	}
	else
	{
	    t = t->next;
	    if ( t == next ) return;
	    MIN_ASSERT ( t->type == PAR::OPERATOR,
			 "operator expected but operand"
			 " found" );
	}

	t = t->next;

	if ( length == 0 ) continue;
	bool found = false;
	min::gen op = t->previous->value;
	for ( min::uns32 i = 0; ! found && i < length;
	                        ++ i )
	    found = ( args[i] == op );
	if ( ! found )
	{
	    PAR::parse_error
		( parser, t->previous->position,
		  "illegal operator ",
		  min::pgen_quote ( op ),
		  " in expression beginnning with ",
		  min::pgen_quote ( oper->label ),
		  "; changed to ",
		  min::pgen_quote ( oper->label ) );
	    PAR::value_ref ( t->previous ) =
	        oper->label;
	}
    }
}

static void associate_left
        ( PAR::parser parser,
	  PAR::pass pass,
	  TAB::flags selectors,
	  PAR::token & first,
	  PAR::token next,
	  const min::phrase_position & position,
	  TAB::flags trace_flags )
{
    MIN_REQUIRE ( first != next );

    // As operators must be infix, operands and opera-
    // tors must alternate with operands first and last.

    // Work from beginning to end taking 3 tokens at a
    // time and rewriting them into a subexpression.
    //
    while ( first->next != next )
    {
        PAR::token t = first;
	MIN_ASSERT ( t->type != PAR::OPERATOR,
	             "operand expected but operator"
		     " found" );
        t = t->next;
	MIN_ASSERT ( t != next,
	             "premature expression end" );
	MIN_ASSERT ( t->type == PAR::OPERATOR,
	             "operator expected but operand"
		     " found" );
        t = t->next;
	MIN_ASSERT ( t != next,
	             "premature expression end" );
	MIN_ASSERT ( t->type != PAR::OPERATOR,
	             "operand expected but operator"
		     " found" );

        min::phrase_position subposition =
	    { first->position.begin,
	      t->position.end };

	if ( t->next == next ) subposition = position;

	PAR::compact
	    ( parser, pass->next, selectors,
	      first, t->next, subposition,
	      trace_flags, PAR::BRACKETABLE );
    }
}

static void associate_right
        ( PAR::parser parser,
	  PAR::pass pass,
	  TAB::flags selectors,
	  PAR::token & first,
	  PAR::token next,
	  const min::phrase_position & position,
	  TAB::flags trace_flags )
{
    MIN_REQUIRE ( first != next );

    // As operators must be infix, operands and opera-
    // tors must alternate with operands first and last.

    // Work from end to beginning taking 3 tokens at a
    // time and rewriting them into a subexpression.
    //
    while ( first->next != next )
    {
        PAR::token t = next->previous;
	MIN_ASSERT ( t->type != PAR::OPERATOR,
	             "operand expected but operator"
		     " found" );
	MIN_ASSERT ( t != first,
	             "premature expression end" );
        t = t->previous;
	MIN_ASSERT ( t->type == PAR::OPERATOR,
	             "operator expected but operand"
		     " found" );
	MIN_ASSERT ( t != first,
	             "premature expression end" );
        t = t->previous;
	MIN_ASSERT ( t->type != PAR::OPERATOR,
	             "operand expected but operator"
		     " found" );

        min::phrase_position subposition =
	    { t->position.begin,
	      t->next->next->position.end };

	bool t_is_first = ( t == first );
	if ( t_is_first ) subposition = position;

	PAR::compact
	    ( parser, pass->next, selectors,
	      t, next, subposition,
	      trace_flags, PAR::BRACKETABLE );

	if ( t_is_first ) first = t;
    }
}

static bool infix_reformatter_function
        ( PAR::parser parser,
	  PAR::pass pass,
	  TAB::flags selectors,
	  PAR::token & first,
	  PAR::token next,
	  const min::phrase_position & position,
	  min::gen line_separator,
	  TAB::flags trace_flags,
	  TAB::root entry )
{
    infix_check ( parser, first, next, entry );
    return true;
}

static bool right_associative_reformatter_function
        ( PAR::parser parser,
	  PAR::pass pass,
	  TAB::flags selectors,
	  PAR::token & first,
	  PAR::token next,
	  const min::phrase_position & position,
	  min::gen line_separator,
	  TAB::flags trace_flags,
	  TAB::root entry )
{

    infix_check ( parser, first, next, entry );
    associate_right
        ( parser, pass, selectors,
	  first, next,
	  position, trace_flags );

    return false;
}

static bool left_associative_reformatter_function
        ( PAR::parser parser,
	  PAR::pass pass,
	  TAB::flags selectors,
	  PAR::token & first,
	  PAR::token next,
	  const min::phrase_position & position,
	  min::gen line_separator,
	  TAB::flags trace_flags,
	  TAB::root entry )
{

    infix_check ( parser, first, next, entry );
    associate_left
        ( parser, pass, selectors,
	  first, next,
	  position, trace_flags );

    return false;
}

static bool unary_reformatter_function
        ( PAR::parser parser,
	  PAR::pass pass,
	  TAB::flags selectors,
	  PAR::token & first,
	  PAR::token next,
	  const min::phrase_position & position,
	  min::gen line_separator,
	  TAB::flags trace_flags,
	  TAB::root entry )
{

    while (    first != next
            && first->type != PAR::OPERATOR )
        first = OP::delete_bad_token
	    ( parser, first, "expected an operator"
			     " and found an operand " );
    MIN_ASSERT ( first != next,
		 "expression must have an operator" );

    PAR::token t = first->next;

    while ( t != next && t->type == PAR::OPERATOR )
        t = OP::delete_bad_token
	        ( parser, t,
		  "expected an operand and found an"
		  " operator " );

    if ( t == next )
    {
	t = t->previous;

	OP::put_error_operand_after ( parser, t );
	t = t->next;
    }
    t = t->next;

    // Delete extra stuff from end of list.
    //
    if ( t != next )
        t = OP::delete_extra_stuff ( parser, t, next );

    return true;
}

static bool binary_reformatter_function
        ( PAR::parser parser,
	  PAR::pass pass,
	  TAB::flags selectors,
	  PAR::token & first,
	  PAR::token next,
	  const min::phrase_position & position,
	  min::gen line_separator,
	  TAB::flags trace_flags,
	  TAB::root entry )
{
    MIN_REQUIRE ( first != next );

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
	OP::put_error_operand_before ( parser, t );
	first = t->previous;
    }
    else
        t = t->next;

    // Ensure that next element is an operator.
    //
    while ( t != next && t->type != PAR::OPERATOR )
        t = OP::delete_bad_token
	        ( parser, t, "expected an operator"
		             " and found an operand " );

    MIN_ASSERT
	( t != next,
	  "expression must have an operator" );
    t = t->next;

    // Ensure that next element is an operand.
    //
    while ( t != next && t->type == PAR::OPERATOR )
        t = OP::delete_bad_token
	        ( parser, t,
		  "expected an operand and found an"
		  " operator " );

    if ( t == next )
    {
	t = t->previous;

	OP::put_error_operand_after ( parser, t );
	t = t->next;
    }
    t = t->next;

    // We should be at end of expression.
    //
    if ( t != next )
        t = OP::delete_extra_stuff ( parser, t, next );

    return true;
}

min::locatable_var<PAR::reformatter>
    OP::reformatter_stack ( min::NULL_STUB );

static void reformatter_stack_initialize ( void )
{
    min::locatable_gen control
        ( min::new_str_gen ( "control" ) );
    PAR::push_reformatter
        ( control, 1, 2,
	  ::control_reformatter_function,
	  OP::reformatter_stack );

    min::locatable_gen declare
        ( min::new_str_gen ( "declare" ) );
    PAR::push_reformatter
        ( declare, 0, 0,
	  ::declare_reformatter_function,
	  OP::reformatter_stack );

    min::locatable_gen separator
        ( min::new_str_gen ( "separator" ) );
    PAR::push_reformatter
        ( separator, 0, 0,
	  ::separator_reformatter_function,
	  OP::reformatter_stack );

    min::locatable_gen selector
        ( min::new_str_gen ( "selector" ) );
    PAR::push_reformatter
        ( selector, 2, 2,
	  ::selector_reformatter_function,
	  OP::reformatter_stack );

    min::locatable_gen infix
        ( min::new_str_gen ( "infix" ) );
    PAR::push_reformatter
        ( infix, 0, 1000,
	  ::infix_reformatter_function,
	  OP::reformatter_stack );

    min::locatable_gen right_associative
        ( min::new_lab_gen ( "right", "associative" ) );
    PAR::push_reformatter
        ( right_associative, 0, 1000,
	  ::right_associative_reformatter_function,
	  OP::reformatter_stack );

    min::locatable_gen left_associative
        ( min::new_lab_gen ( "left", "associative" ) );
    PAR::push_reformatter
        ( left_associative, 0, 1000,
	  ::left_associative_reformatter_function,
	  OP::reformatter_stack );

    min::locatable_gen prefix
        ( min::new_str_gen ( "unary" ) );
    PAR::push_reformatter
        ( prefix, 0, 0,
	  ::unary_reformatter_function,
	  OP::reformatter_stack );

    min::locatable_gen binary
        ( min::new_str_gen ( "binary" ) );
    PAR::push_reformatter
        ( binary, 0, 0,
	  ::binary_reformatter_function,
	  OP::reformatter_stack );
}
static min::initializer reformatter_initializer
    ( ::reformatter_stack_initialize );

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
    MIN_REQUIRE ( op != min::NULL_STUB );

    min::gen block_name =
	PAR::block_name
	    ( parser,
	      op->block_level );
    parser->printer
	<< min::indent
	<< "block "
	<< min::pgen_name ( block_name )
	<< ": " << min::save_indent;

    if ( table_type == ::OPERATOR )
	parser->printer
	    << "operator "
	    << min::pgen_quote ( op->label );
    else if ( table_type == ::BRACKET )
    {
	parser->printer
	    << "bracket "
	    << min::pgen_quote ( op->label );
	if ( op->terminator != min::MISSING() )
	    parser->printer
		<< " ... "
		<< min::pgen_quote ( op->terminator );
    }

    parser->printer
	<< " " << min::set_break;

    COM::print_flags
	( op->selectors, PAR::COMMAND_SELECTORS,
	  parser->selector_name_table,
	  parser );

    parser->printer << min::indent;
    min::uns32 flags = op->flags &
        ( OP::INITIAL + OP::LEFT + OP::RIGHT
	              + OP::FINAL );
    if ( flags == OP::PREFIX )
	parser->printer << "prefix";
    else if ( flags == OP::INFIX )
	parser->printer << "infix";
    else if ( flags == OP::POSTFIX )
	parser->printer << "postfix";
    else if ( flags == OP::NOFIX )
	parser->printer << "nofix";
    else
    {
	if ( op->flags & OP::INITIAL )
	    parser->printer << "initial";
	if ( op->flags & OP::LEFT )
	    parser->printer
		<< min::space_if_after_indent
		<< "left";
	if ( op->flags & OP::RIGHT )
	    parser->printer
		<< min::space_if_after_indent
		<< "right";
	if ( op->flags & OP::FINAL )
	    parser->printer
		<< min::space_if_after_indent
		<< "final";
    }
    if ( op->flags & OP::AFIX )
	parser->printer
	    << min::space_if_after_indent
	    << "afix";
    if ( op->flags & OP::LINE )
	parser->printer
	    << min::space_if_after_indent
	    << "line";

    parser->printer
	<< min::indent
	<< "with precedence "
	<< op->precedence;

    if ( op->reformatter != min::NULL_STUB )
    {
	parser->printer
	    << min::indent
	    << "with "
	    << min::pgen_name
	           ( op->reformatter->name )
	    << " reformatter";

	min::obj_vec_ptr args
	    ( op->reformatter_arguments );
        if ( args != min::NULL_STUB )
	{
	    parser->printer << " ( " << min::set_break;
	    for ( min::uns32 i = 0;
	          i < min::size_of ( args ); ++ i )
	    {
		if ( i != 0 )
		    parser->printer << ", "
		                    << min::set_break;
	        parser->printer
		    << min::pgen_quote ( args[i] );
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
	  min::obj_vec_ptr & vp, min::uns32 i0,
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

    min::uns32 i = i0;
        // vp[i] is next lexeme or subexpression to
	// scan in the define/undefine expression.

    min::gen command = vp[i++];

    if ( command != PARLEX::define
         &&
	 command != PARLEX::undefine
         &&
	 command != PARLEX::print )
        return min::FAILURE();

    if ( i >= size || vp[i++] != OPLEX::oper )
        return min::FAILURE();

    if ( i >= size || command == PARLEX::print )
        /* Do nothing. */;
    else if ( vp[i] == OPLEX::bracket )
    {
	++ i;
	bracket = true;
    }
    else if ( vp[i] == OPLEX::indentation
              &&
	      i + 1 < size
	      &&
	      vp[i+1] == OPLEX::mark )
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
	    PAR::scan_quoted_key
	        ( vp, i, parser,
		  command == PARLEX::print );

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
	     vp[i] != PARLEX::dotdotdot )
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

    if ( command == PARLEX::print )
    {

	min::uns32 indent =
	    COM::print_command ( parser, ppvec );

	parser->printer
	    << min::bom << min::no_auto_break
	    << min::set_indent ( indent + 4 );

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
	                    << "nothing found";
	parser->printer << min::eom;

    	return PAR::PRINTED;
    }

    // Scan selectors.
    //
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
    else MIN_REQUIRE ( sresult == min::SUCCESS() );

    // Scan operator flags.
    //
    min::uns32 oper_flags = 0;

    min::phrase_position oper_flags_position;
    oper_flags_position.begin = (&ppvec[i])->begin;

    while ( i < size )
    {
	min::uns32 new_oper_flag;
        if ( vp[i] == OPLEX::initial )
	    new_oper_flag = OP::INITIAL;
        else if ( vp[i] == OPLEX::left )
	    new_oper_flag = OP::LEFT;
        else if ( vp[i] == OPLEX::right )
	    new_oper_flag = OP::RIGHT;
        else if ( vp[i] == OPLEX::final )
	    new_oper_flag = OP::FINAL;
        else if ( vp[i] == OPLEX::afix )
	    new_oper_flag = OP::AFIX;
        else if ( vp[i] == OPLEX::line )
	    new_oper_flag = OP::LINE;
        else if ( vp[i] == OPLEX::prefix )
	    new_oper_flag = OP::PREFIX;
        else if ( vp[i] == OPLEX::infix )
	    new_oper_flag = OP::INFIX;
        else if ( vp[i] == OPLEX::postfix )
	    new_oper_flag = OP::POSTFIX;
        else if ( vp[i] == OPLEX::nofix )
	    new_oper_flag = OP::NOFIX;
	else break;

	oper_flags |= new_oper_flag;
	++ i;
    }

    oper_flags_position.end = (&ppvec[i-1])->end;

    if ( ( oper_flags & OP::INITIAL )
          &&
	 ( oper_flags & OP::LEFT ) )
	return PAR::parse_error
	    ( parser, oper_flags_position,
	      "operator flags initial and left"
	      " are incompatible" );
    if ( ( oper_flags & OP::RIGHT )
          &&
	 ( oper_flags & OP::FINAL ) )
	return PAR::parse_error
	    ( parser, oper_flags_position,
	      "operator flags right and final"
	      " are incompatible" );
    if ( ( oper_flags & OP::INITIAL )
          &&
	 ( oper_flags & OP::AFIX ) )
	return PAR::parse_error
	    ( parser, oper_flags_position,
	      "operator flags initial and afix"
	      " are incompatible" );

    min::int32 precedence;
    bool precedence_found = false;
    PAR::reformatter reformatter = min::NULL_STUB;
    min::locatable_gen reformatter_arguments;
    while ( i < size && vp[i] == PARLEX::with )
    {
	++ i;
	if ( i < size
	     &&
	     vp[i] == OPLEX::precedence )
	{
	    ++ i;
	    if ( i >= size )
		return PAR::parse_error
		    ( parser, ppvec[i-1],
		      "expected precedence integer"
		      " after" );
	    min::int64 p;
	    if ( ! min::strto ( p, vp[i], 0 ) )
		return PAR::parse_error
		    ( parser, ppvec[i],
		      "precedence is not an integer" );
	    if ( p < OP::low_precedence
		 ||
		 p > OP::high_precedence )
		return PAR::parse_error
		    ( parser, ppvec[i],
		      "precedence out of range" );
	    precedence = (min::int32) p;
	    precedence_found = true;
	    ++ i;
	    continue;
	}
	else if ( i < size )
	{
	    min::uns32 j = i;
	    min::locatable_gen name
	      ( PAR::scan_simple_name
	          ( vp, j, PARLEX::reformatter ) );
	    if (    j < size
		 && vp[j] == PARLEX::reformatter )
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

		i = j + 1;

		name = COM::scan_args
		    ( vp, i, reformatter_arguments,
		          parser );
		if ( name == min::ERROR() )
		    return min::ERROR();
		min::obj_vec_ptr args
		    ( reformatter_arguments );
		if ( args == min::NULL_STUB )
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

		    min::unsptr s =
		        min::size_of ( args );

		    if ( s < reformatter->
			         minimum_arguments )
			return PAR::parse_error
				( parser, position,
				  "too few reformatter"
				  " arguments" );
		    if ( s > reformatter->
			         maximum_arguments )
			return PAR::parse_error
				( parser, position,
				  "too many reformatter"
				  " arguments" );
		}

		continue;
	    }
	    else
		return PAR::parse_error
		    ( parser, ppvec[i-1],
		      command == PARLEX::define ?
		      "expected `precedence ...' or"
		      " `... reformatter' after" :
		      "expected `precedence ...'"
		      " after" );
	}

	return PAR::parse_error
	    ( parser, ppvec[i-1],
	      command == PARLEX::define ?
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

    if ( command == PARLEX::define )
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

    else // if ( command == PARLEX::undefine )
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
