// Layered Languages Bracketed Subexpression Parser
//
// File:	ll_parser_bracketed.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Aug 17 16:12:01 EDT 2015
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Untyped Brackets
//	Indentation Marks
//	Typed Brackets
//	Bracketed Subexpression Pass
//	Bracketed Subexpression Parser Functions
//	Parse Bracketed Subexpression Function
//	Bracketed Reformatters
//	Bracketed Pass Command Function

// Usage and Setup
// ----- --- -----

# include <ll_lexeme_standard.h>
# include <ll_parser.h>
# include <ll_parser_command.h>
# include <ll_parser_bracketed.h>
# define LEX ll::lexeme
# define LEXSTD ll::lexeme::standard
# define PAR ll::parser
# define TAB ll::parser::table
# define COM ll::parser::command
# define BRA ll::parser::bracketed

static min::locatable_gen bracket;
static min::locatable_gen indentation;
static min::locatable_gen mark;
static min::locatable_gen full;
static min::locatable_gen lines;
static min::locatable_gen bracketed_subexpressions;
static min::locatable_gen offset;
static min::locatable_gen top;

static void initialize ( void )
{
    ::bracket = min::new_str_gen ( "bracket" );
    ::indentation = min::new_str_gen
			    ( "indentation" );
    ::mark = min::new_str_gen ( "mark" );
    ::full = min::new_str_gen ( "full" );
    ::lines = min::new_str_gen ( "lines" );
    ::bracketed_subexpressions =
        min::new_lab_gen
	    ( "bracketed", "subexpressions" );
    ::offset = min::new_str_gen ( "offset" );
    ::top = min::new_str_gen ( "top" );
}
static min::initializer initializer ( ::initialize );

// Untyped Brackets
// ------- --------

static min::uns32 opening_bracket_stub_disp[] = {
    min::DISP ( & BRA::opening_bracket_struct::next ),
    min::DISP ( & BRA::opening_bracket_struct
                     ::closing_bracket ),
    min::DISP ( & BRA::opening_bracket_struct
                     ::reformatter ),
    min::DISP ( & BRA::opening_bracket_struct
                     ::reformatter_arguments ),
    min::DISP_END };

static min::packed_struct_with_base
	<BRA::opening_bracket_struct, TAB::root_struct>
    opening_bracket_type
	( "ll::parser::table::opening_bracket_type",
	  TAB::root_gen_disp,
	  ::opening_bracket_stub_disp );
const min::uns32 & BRA::OPENING_BRACKET =
    opening_bracket_type.subtype;

static min::uns32 closing_bracket_stub_disp[] = {
    min::DISP ( & BRA::closing_bracket_struct::next ),
    min::DISP ( & BRA::closing_bracket_struct
                     ::opening_bracket ),
    min::DISP_END };

static min::packed_struct_with_base
	<BRA::closing_bracket_struct, TAB::root_struct>
    closing_bracket_type
	( "ll::parser::table::closing_bracket_type",
	  TAB::root_gen_disp,
	  ::closing_bracket_stub_disp );
const min::uns32 & BRA::CLOSING_BRACKET =
    closing_bracket_type.subtype;

BRA::opening_bracket
    BRA::push_brackets
	( min::gen opening_bracket,
	  min::gen closing_bracket,
	  TAB::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  const TAB::new_flags & new_selectors,
	  PAR::reformatter reformatter,
	  PAR::reformatter_arguments
	      reformatter_arguments,
	  min::uns32 options,
	  TAB::key_table bracket_table )
{
    min::locatable_var<BRA::opening_bracket> opening
        ( ::opening_bracket_type.new_stub() );
    min::locatable_var<BRA::closing_bracket> closing
        ( ::closing_bracket_type.new_stub() );

    label_ref(opening) = opening_bracket;
    label_ref(closing) = closing_bracket;

    closing_bracket_ref(opening) = closing;
    opening_bracket_ref(closing) = opening;

    opening->selectors = selectors;
    closing->selectors = PAR::ALWAYS_SELECTOR;

    opening->block_level = block_level;
    closing->block_level = block_level;

    opening->position = position;
    closing->position = position;

    opening->new_selectors = new_selectors;
    reformatter_ref(opening) = reformatter;
    reformatter_arguments_ref(opening) =
        reformatter_arguments;
    opening->options = options;

    TAB::push ( bracket_table, (TAB::root) opening );
    TAB::push ( bracket_table, (TAB::root) closing );

    return opening;
}

// Indentation Marks
// ----------- -----

static min::uns32 indentation_mark_stub_disp[] = {
    min::DISP ( & BRA::indentation_mark_struct::next ),
    min::DISP ( & BRA::indentation_mark_struct
                     ::line_sep ),
    min::DISP_END };

static min::packed_struct_with_base
	<BRA::indentation_mark_struct, TAB::root_struct>
    indentation_mark_type
	( "ll::parser::table::indentation_mark_type",
	  TAB::root_gen_disp,
	  ::indentation_mark_stub_disp );
const min::uns32 & BRA::INDENTATION_MARK =
    indentation_mark_type.subtype;

static min::uns32 line_sep_stub_disp[] = {
    min::DISP ( & BRA::line_sep_struct::next ),
    min::DISP ( & BRA::line_sep_struct
                     ::indentation_mark ),
    min::DISP_END };

static min::packed_struct_with_base
	<BRA::line_sep_struct, TAB::root_struct>
    line_sep_type
	( "ll::parser::table::line_sep_type",
	  TAB::root_gen_disp,
	  ::line_sep_stub_disp );
const min::uns32 & BRA::LINE_SEP =
    line_sep_type.subtype;

BRA::indentation_mark
    BRA::push_indentation_mark
	( min::gen mark_label,
	  min::gen separator_label,
	  TAB::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  const TAB::new_flags & new_selectors,
	  TAB::key_table bracket_table )
{
    min::locatable_var<BRA::indentation_mark> imark
        ( ::indentation_mark_type.new_stub() );
    label_ref(imark) = mark_label;
    imark->selectors = selectors;
    imark->block_level = block_level;
    imark->position = position;
    imark->new_selectors = new_selectors;
    TAB::push ( bracket_table, (TAB::root) imark );

    if ( separator_label != min::MISSING() )
    {
	min::locatable_var<BRA::line_sep>
	    separator
	    ( ::line_sep_type.new_stub() );
	label_ref(separator) = separator_label;
	separator->selectors = PAR::ALWAYS_SELECTOR;
	separator->block_level = block_level;
	separator->position = position;
	indentation_mark_ref(separator) = imark;
	line_sep_ref(imark) = separator;
	TAB::push ( bracket_table,
	            (TAB::root) separator );
    }

    return imark;
}

// Typed Brackets
// ----- --------

static min::uns32 typed_opening_gen_disp[] = {
    min::DISP ( & BRA::typed_opening_struct::label ),
    min::DISP ( & BRA::typed_opening_struct
                     ::typed_attr_flags_initiator ),
    min::DISP ( & BRA::typed_opening_struct
                     ::typed_attr_multivalue_initiator
	      ),
    min::DISP_END };

static min::uns32 typed_opening_stub_disp[] = {
    min::DISP ( & BRA::typed_opening_struct::next ),
    min::DISP ( & BRA::typed_opening_struct
                     ::closing_bracket ),
    min::DISP ( & BRA::typed_opening_struct
                     ::reformatter ),
    min::DISP ( & BRA::typed_opening_struct
                     ::reformatter_arguments ),
    min::DISP ( & BRA::typed_opening_struct
                     ::key_table ),
    min::DISP ( & BRA::typed_opening_struct
                     ::typed_middle ),
    min::DISP ( & BRA::typed_opening_struct
                     ::closing_bracket ),
    min::DISP ( & BRA::typed_opening_struct
                     ::typed_attr_begin ),
    min::DISP ( & BRA::typed_opening_struct
                     ::typed_attr_equal ),
    min::DISP ( & BRA::typed_opening_struct
                     ::typed_attr_sep ),
    min::DISP ( & BRA::typed_opening_struct
                     ::typed_attr_negator ),
    min::DISP_END };

static min::packed_struct_with_base
	<BRA::typed_opening_struct,
	 BRA::opening_bracket_struct>
    typed_opening_type
	( "ll::parser::table::typed_opening_type",
	  ::typed_opening_gen_disp,
	  ::typed_opening_stub_disp );
const min::uns32 & BRA::TYPED_OPENING =
    typed_opening_type.subtype;

static min::uns32 typed_middle_stub_disp[] = {
    min::DISP ( & BRA::typed_middle_struct::next ),
    min::DISP ( & BRA::typed_middle_struct
                     ::typed_opening ),
    min::DISP_END };

static min::packed_struct_with_base
	<BRA::typed_middle_struct, TAB::root_struct>
    typed_middle_type
	( "ll::parser::table::typed_middle_type",
	  TAB::root_gen_disp,
	  ::typed_middle_stub_disp );
const min::uns32 & BRA::TYPED_MIDDLE =
    typed_middle_type.subtype;

static min::uns32 typed_attr_begin_stub_disp[] = {
    min::DISP ( & BRA::typed_attr_begin_struct
                     ::next ),
    min::DISP ( & BRA::typed_attr_begin_struct
                     ::typed_opening ),
    min::DISP_END };

static min::packed_struct_with_base
	<BRA::typed_attr_begin_struct,
	 TAB::root_struct>
    typed_attr_begin_type
	( "ll::parser"
	    "::table::typed_attr_begin_type",
	  TAB::root_gen_disp,
	  ::typed_attr_begin_stub_disp );
const min::uns32 & BRA::TYPED_ATTR_BEGIN =
    typed_attr_begin_type.subtype;

static min::uns32 typed_attr_equal_stub_disp[] = {
    min::DISP ( & BRA::typed_attr_equal_struct
                     ::next ),
    min::DISP ( & BRA::typed_attr_equal_struct
                     ::typed_opening ),
    min::DISP_END };

static min::packed_struct_with_base
	<BRA::typed_attr_equal_struct,
	 TAB::root_struct>
    typed_attr_equal_type
	( "ll::parser"
	    "::table::typed_attr_equal_type",
	  TAB::root_gen_disp,
	  ::typed_attr_equal_stub_disp );
const min::uns32 & BRA::TYPED_ATTR_EQUAL =
    typed_attr_equal_type.subtype;

static min::uns32
	typed_attr_sep_stub_disp[] = {
    min::DISP ( & BRA::typed_attr_sep_struct
                     ::next ),
    min::DISP ( & BRA::typed_attr_sep_struct
                     ::typed_opening ),
    min::DISP_END };

static min::packed_struct_with_base
	<BRA::typed_attr_sep_struct,
	 TAB::root_struct>
    typed_attr_sep_type
	( "ll::parser"
	    "::table::typed_attr_sep_type",
	  TAB::root_gen_disp,
	  ::typed_attr_sep_stub_disp );
const min::uns32 & BRA::TYPED_ATTR_SEP =
    typed_attr_sep_type.subtype;

static min::uns32
	typed_attr_negator_stub_disp[] = {
    min::DISP ( & BRA::typed_attr_negator_struct
                     ::next ),
    min::DISP ( & BRA::typed_attr_negator_struct
                     ::typed_opening ),
    min::DISP_END };

static min::packed_struct_with_base
	<BRA::typed_attr_negator_struct,
	 TAB::root_struct>
    typed_attr_negator_type
	( "ll::parser"
	    "::table::typed_attr_negator_type",
	  TAB::root_gen_disp,
	  ::typed_attr_negator_stub_disp );
const min::uns32 & BRA::TYPED_ATTR_NEGATOR =
    typed_attr_negator_type.subtype;


BRA::typed_opening
    BRA::push_typed_brackets
	( min::gen typed_opening,
	  min::gen typed_middle,
	  min::gen typed_closing,
	  TAB::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  const TAB::new_flags & element_selectors,
	  TAB::flags attr_selectors,
	  PAR::reformatter reformatter,
	  PAR::reformatter_arguments
	      reformatter_arguments,
	  min::uns32 options,
	  min::gen typed_attr_begin,
	  min::gen typed_attr_equal,
	  min::gen typed_attr_sep,
	  min::gen typed_attr_negator,
	  min::gen typed_attr_flags_initiator,
	  const min::flag_parser *
	           typed_attr_flag_parser,
	  min::gen typed_attr_multivalue_initiator,
	  TAB::key_table bracket_table )
{
    min::locatable_var<BRA::typed_opening> opening
        ( ::typed_opening_type.new_stub() );
    min::locatable_var<BRA::closing_bracket> closing
        ( ::closing_bracket_type.new_stub() );

    label_ref(opening) = typed_opening;
    label_ref(closing) = typed_closing;

    closing_bracket_ref(opening) = closing;
    opening_bracket_ref(closing) =
        (BRA::opening_bracket) opening;

    min::locatable_var<TAB::key_table> key_table
        ( TAB::create_key_table ( 64 ) );
    key_table_ref(opening) = key_table;

    opening->selectors = selectors;
    closing->selectors = PAR::ALWAYS_SELECTOR;

    opening->block_level = block_level;
    closing->block_level = block_level;

    opening->position = position;
    closing->position = position;

    opening->new_selectors = element_selectors;
    opening->attr_selectors = attr_selectors;

    reformatter_ref(opening) = reformatter;
    reformatter_arguments_ref(opening) =
        reformatter_arguments;

    opening->options = options;

    TAB::push ( bracket_table, (TAB::root) opening );
    TAB::push ( bracket_table, (TAB::root) closing );

    min::locatable_var<BRA::typed_middle> middle1
        ( ::typed_middle_type.new_stub() );
    min::locatable_var<BRA::typed_middle> middle2
        ( ::typed_middle_type.new_stub() );
    label_ref(middle1)  = typed_middle;
    label_ref(middle2)  = typed_middle;

    typed_middle_ref(opening)  = middle1;
    typed_opening_ref(middle1)  = opening;

    middle1->selectors  = PAR::ALWAYS_SELECTOR;
    middle2->selectors  = BRA::MIDDLE_SELECTOR;
    middle1->block_level  = block_level;
    middle2->block_level  = block_level;
    middle1->position  = position;
    middle2->position  = position;

    TAB::push ( bracket_table, (TAB::root) middle1 );
    TAB::push ( key_table, (TAB::root) middle2 );

    if ( typed_attr_begin != min::MISSING() )
    {
	min::locatable_var
		<BRA::typed_attr_begin>
	    attr_begin
		( ::typed_attr_begin_type
		    .new_stub() );
	min::locatable_var
		<BRA::typed_attr_equal>
	    attr_equal
		( ::typed_attr_equal_type
		    .new_stub() );
	min::locatable_var
		<BRA::typed_attr_sep>
	    attr_sep
		( ::typed_attr_sep_type
		    .new_stub() );

	label_ref(attr_begin)
	    = typed_attr_begin;
	label_ref(attr_equal)
	    = typed_attr_equal;
	label_ref(attr_sep)
	    = typed_attr_sep;

	typed_attr_begin_ref(opening)
	    = attr_begin;
	typed_attr_equal_ref(opening)
	    = attr_equal;
	typed_attr_sep_ref(opening)
	    = attr_sep;
	typed_opening_ref(attr_begin)
	    = opening;
	typed_opening_ref(attr_equal)
	    = opening;
	typed_opening_ref(attr_sep)
	    = opening;

	attr_begin->selectors    = BRA::ATTR_SELECTOR;
	attr_equal->selectors    = BRA::ATTR_SELECTOR;
	attr_sep->selectors 	 = BRA::ATTR_SELECTOR;

	attr_begin->block_level  = block_level;
	attr_equal->block_level  = block_level;
	attr_sep->block_level 	 = block_level;

	attr_begin->position     = position;
	attr_equal->position     = position;
	attr_sep->position 	 = position;

	TAB::push ( key_table,
	           (TAB::root) attr_begin );
	TAB::push ( key_table,
	           (TAB::root) attr_equal );
	TAB::push ( key_table,
	           (TAB::root) attr_sep );
    }

    if ( typed_attr_negator != min::MISSING() )
    {
	min::locatable_var
		<BRA::typed_attr_negator>
	    attr_negator
		( ::typed_attr_negator_type
		    .new_stub() );

	label_ref(attr_negator)
	    = typed_attr_negator;

	typed_attr_negator_ref(opening)
	    = attr_negator;
	typed_opening_ref(attr_negator)
	    = opening;

	attr_negator->selectors = BRA::NEGATOR_SELECTOR;

	attr_negator->block_level = block_level;

	attr_negator->position    = position;

	TAB::push ( key_table,
	           (TAB::root) attr_negator );
    }

    typed_attr_flags_initiator_ref(opening) =
        typed_attr_flags_initiator;
    opening->typed_attr_flag_parser =
    	typed_attr_flag_parser;
    typed_attr_multivalue_initiator_ref(opening) =
        typed_attr_multivalue_initiator;

    return opening;
}

// Bracketed Subexpression Pass
// --------- ------------- ----

static min::packed_vec<min::int32>
    indentation_offset_stack_type
        ( "ll::parser::bracketed"
	    "::indentation_offset_stack_type" );

static min::uns32 bracketed_pass_stub_disp[] =
{
    min::DISP ( & BRA::bracketed_pass_struct
                     ::parser ),
    min::DISP ( & BRA::bracketed_pass_struct
                     ::next ),
    min::DISP ( & BRA::bracketed_pass_struct
                     ::bracket_table ),
    min::DISP_END
};

static min::packed_struct_with_base
	<BRA::bracketed_pass_struct,
	 PAR::pass_struct>
    bracketed_pass_type
        ( "ll::parser::bracketed::bracketed_pass_type",
	  NULL,
	  ::bracketed_pass_stub_disp );
const min::uns32 & BRA::BRACKETED_PASS =
    ::bracketed_pass_type.subtype;

static void bracketed_pass_place
	( PAR::parser parser,
	  PAR::pass pass )
{
    // Bracketed pass must be at top of stack.
    //
    MIN_ASSERT ( parser->pass_stack == pass,
                 "bracketed pass is not at top of"
		 " parser pass_stack ");

    BRA::bracketed_pass bracketed_pass =
        (BRA::bracketed_pass) pass;

    int index = TAB::find_name
        ( parser->trace_flag_name_table,
	  ::bracketed_subexpressions );
    MIN_REQUIRE
      ( (unsigned) index < 8 * sizeof ( TAB::flags ) );
    bracketed_pass->trace_subexpressions =
        1ull << index;
}

static void bracketed_pass_reset
	( PAR::parser parser,
	  PAR::pass pass )
{
    BRA::bracketed_pass bracketed_pass =
        (BRA::bracketed_pass) pass;
    TAB::key_table bracket_table =
        bracketed_pass->bracket_table;

    min::uns64 collected_entries,
               collected_key_prefixes;

    TAB::end_block
        ( bracket_table, 0,
	  collected_key_prefixes, collected_entries );
    bracketed_pass->indentation_offset = 2;
}

static min::gen bracketed_pass_begin_block
	( PAR::parser parser,
	  PAR::pass pass,
	  const min::phrase_position & position,
	  min::gen name )
{
    BRA::bracketed_pass bracketed_pass =
        (BRA::bracketed_pass) pass;

    min::push ( bracketed_pass->
                    indentation_offset_stack ) =
	bracketed_pass->indentation_offset;

    return min::SUCCESS();
}


static min::gen bracketed_pass_end_block
	( PAR::parser parser,
	  PAR::pass pass,
	  const min::phrase_position & position,
	  min::gen name )
{
    BRA::bracketed_pass bracketed_pass =
        (BRA::bracketed_pass) pass;
    TAB::key_table bracket_table =
        bracketed_pass->bracket_table;

    min::uns64 collected_entries,
               collected_key_prefixes;

    min::uns32 block_level =
        PAR::block_level ( parser );
    MIN_REQUIRE ( block_level > 0 );

    TAB::end_block
        ( bracket_table, block_level - 1,
	  collected_key_prefixes, collected_entries );

    bracketed_pass->indentation_offset =
        min::pop
	    ( bracketed_pass->
	          indentation_offset_stack );

    return min::SUCCESS();
}

static min::gen bracketed_pass_command
	( PAR::parser parser,
	  PAR::pass pass,
	  min::obj_vec_ptr & vp,
          min::phrase_position_vec ppvec );

PAR::pass BRA::new_pass ( void )
{
    min::locatable_var<BRA::bracketed_pass>
	bracketed_pass
	    ( ::bracketed_pass_type.new_stub() );

    BRA::name_ref ( bracketed_pass ) = ::top;

    bracketed_pass->place =
        ::bracketed_pass_place;
    bracketed_pass->reset =
        ::bracketed_pass_reset;
    bracketed_pass->begin_block =
        ::bracketed_pass_begin_block;
    bracketed_pass->end_block =
        ::bracketed_pass_end_block;
    bracketed_pass->parser_command =
        ::bracketed_pass_command;
    bracketed_pass->indentation_offset = 2;
    bracket_table_ref(bracketed_pass) =
	TAB::create_key_table ( 256 );
    indentation_offset_stack_ref(bracketed_pass) =
        ::indentation_offset_stack_type.new_stub ( 16 );

    return (PAR::pass) bracketed_pass;
}

// Bracketed Subexpression Parser Functions
// --------- ------------- ------ ---------

// These are all static or inline; if any are useful
// elsewhere they can be put in the PAR namespace.

// Given the token sequence from `first' to `next', make
// a label containing these tokens.  Convert quoted
// string and numeric tokens to strings.  Announce
// non-name components as errors and replace their
// values with "ERRONEOUS-LABEL-COMPONENT".
//
// If there are 0 label components, make an empty label,
// and if there is exactly 1 component, use the value of
// that component as the label, rather than making a
// label.
//
// All tokens but the first are removed, and the type
// of the first token is set to DERIVED, the label is
// returned as the value of the first token, and the
// position of this first token is set to include all
// the tokens that made the label.
//
// If `first' == `next' (and therefore the label is
// empty), a new first token is made and inserted before
// `next', and the `first' argument is updated to point
// at the new token, and the position is set to the
// empty string just before `next'.  This is the only
// case where `first' is updated.
//
static void make_label
    ( PAR::parser parser,
      PAR::token & first,
      PAR::token next )
{
    min::unsptr count = 0;
    min::phrase_position position =
        { first->position.begin,
	  first->position.begin };
        // For the empty label case first == next so
	// this is the position just before next.

    for ( PAR::token t = first; t != next;
          ++ count, t = t->next )
    {
	position.end = t->position.end;

	if ( min::is_name ( t->value ) ) continue;
        else if ( t->type == LEXSTD::quoted_string_t
	          ||
	          t->type == LEXSTD::numeric_t )
	{
	    t->type = PAR::DERIVED;
	    PAR::value_ref(t) = min::new_str_gen
			( min::begin_ptr_of
			      ( t->string ),
			  t->string->length );
	    PAR::string_ref(t) =
		PAR::free_string ( t->string );
	}
	else
	{
	    parser->printer
		<< min::bom << min::set_indent ( 7 )
		<< "ERROR: subexpression "
		<< min::pgen_quote ( t->value )
		<< " illegal for label element"
		   " - changed to"
		   " ERRONEOUS-LABEL-COMPONENT; "
		<< min::pline_numbers
		       ( parser->input_file,
			 t->position )
		<< ":" << min::eom;
	    min::print_phrase_lines
		( parser->printer,
		  parser->input_file,
		  t->position );
	    ++ parser->error_count;

	    t->type = PAR::DERIVED;
	    value_ref(t) = min::new_str_gen
	        ( "ERRONEOUS-LABEL-COMPONENT" );
	}
    }

    if ( count == 0 )
    {
	first = new_token ( PAR::DERIVED );
	put_before ( PAR::first_ref(parser),
	             next, first );
	PAR::value_ref(first) = min::empty_lab;
    }
    else if ( count != 1 )
    {
	min::gen vec[count];
	min::unsptr i = 0;
	for ( PAR::token t = first; t != next;
			 t = t->next )
	    vec[i++] = t->value;
	MIN_REQUIRE ( i == count );
	PAR::value_ref(first) =
	    min::new_lab_gen ( vec, count );

	// Don't deallocate tokens until their values
	// have been put in gc protected label.
	//
	for ( PAR::token t = first->next; t != next; )
	{
	    t = t->next;
	    PAR::free
		( PAR::remove
		    ( first_ref(parser),
		      t->previous ) );
	}
    }

    first->type = PAR::DERIVED;
    first->position = position;
}

// Complain that token indent is too near indent.
//
static void complain_near_indent
	( PAR::parser parser,
	  PAR::token token,
	  min::int32 indent )
{
    parser->printer
	<< min::bom << min::set_indent ( 7 )
	<< "ERROR: lexeme indent "
	<< token->indent
	<< " too near paragraph indent "
	<< indent
	<< "; "
	<< min::pline_numbers
	       ( parser->input_file,
		 token->position )
	<< ":" << min::eom;
    min::print_phrase_lines
	( parser->printer,
	  parser->input_file,
	  token->position );
    ++ parser->error_count;
}

// Return token indent - indent and complain if token
// indent is too near indent.  Assert token has an
// indent.
//
inline min::int32 relative_indent
	( PAR::parser parser,
	  min::int32 indentation_offset,
	  PAR::token token,
	  min::int32 indent )
{
    MIN_REQUIRE ( token->indent != LEX::AFTER_GRAPHIC );

    int relative_indent =
        (min::int32) token->indent - indent;
    if (    relative_indent != 0
	 && relative_indent < indentation_offset 
	 && relative_indent > - indentation_offset )
        ::complain_near_indent
	    ( parser, token, indent );
    return relative_indent;
}

// Parse Bracketed Subexpression Function
// ----- --------- ------------- --------

// Outline of parse_bracketed_subexpression:
//
//   indentation_mark indentation_found = NONE
//   loop:
//     if current->type is end-of-file:
//       return false
//     if current->type is line-break:
//       remove following line-breaks and full
//              line comments, issuing warning
//              messages for insufficiently
//              indented comments
//       if indentation_found != NONE:
//         if paragraph has some lines:
//           compute new_selectors from
//                   current selectors and
//                   indentation_found
//           delete current line-break	            
//           paragraph_indent = current->indent
//
//           loop to parse paragraph lines:
//               parse_bracketed_subexpression with
//                   paragraph_indent, indentation_
//                   found, new_selectors, bracket
//                   stack; moves current to end of
//                   line; remember line ended by
//                   a separator
//               if bracket stack top closed:
//                   require that there was no line
//                           ending separator
//               compact line found with .type = <LF>
//                       and .terminator = any line
//                       ending separator found;
//                       if line ending separator
//                       found remove it first
//               end loop if top of bracket stack
//                   closed, or current->type is end-of-
//                   file, or current->next has indent
//                   less than paragraph_indent
//               delete current line-break and continue
//                      loop
//
//           remove indentation mark
//           compact paragraph lines with .type =
//                   indentation_found label
//	     if bracket stack top closed or current->
//	        type is end-of-file:
//	          return false
//	     indentation_found = NONE
// 	 
// 	 // Continue line-break processing
// 	 // current->type is line-break and next
// 	 // token is not a line-break or comment
// 	 //
// 	 if current->next->type is end-of-file:
// 	    return false
// 	 if current->next does not begin an indented
// 	    continuation line:
// 	      return false
// 	 remove current line-break
// 	 iterate top level loop; going to continuation
// 	         line
//
//    // continue top level loop if no line-break
//    //          found
//    //
//    if current->type is a comment, delete comment
//       and iterate top level loop
//    if current->type is quoted string:
//      if current->previous->type is quoted string:
//         merge quoted strings
//      current = current->next
//      iterate top level loop
//    
//    lookup key in bracket pass bracket table
//    loop to refine key until it is selected:
//
//      if key == NONE:
//        current = current->next
//        iterate top level loop
//
//      if key is selected opening bracket or typed
//         opening:
//        compute new_selectors from existing selectors
//                and key
//        create new bracket stack entry with opening
//               bracket = key
//        if key has full lines option, create new
//           bracket stack with new entry as its only
//           entry
//        else push new entry into existing stack
//        parse_bracketed_subexpression with
//          new_selectors, no indentaton_mark,
//          new bracket stack, and indent = indent
//          argument to top level, unless full lines
//          option given, in which case indent = -2;
//          closing found is recorded in closing
//          stack
//        if closing was found that did not match
//           top of closing stack, print error
//           message
//        call opening bracket reformatter if any,
//             and if none or if requested by
//             reformatter, compact with .initiator
//             and .terminator being the opening
//             and closing bracket (the latter
//             taken from the symbol table and
//             not the input)
//        if closing found was line break before
//           insuffiently indented line:
//          iterate top level loop
//        else if closing bracket was found that did not
//           match top of closing stack:
//          return false
//        else
//          iterate top level loop
//
//      if key is closing bracket in bracket stack:
//        close bracket stack entry that matches key
//        close all higher stack entries indication
//          their closing brackets were not found
//        return false
//
//      if key is selected indentation mark and
//         current->type is end-of-file or line break:
//        indentation_found = key
//        iterate top level loop
//
//      if key is line separator matching indentation_
//         mark argument:
//        return true
//
//      reject key
//      iterate loop to refine key

bool BRA::parse_bracketed_subexpression
	( PAR::parser parser,
	  TAB::flags selectors,
	  PAR::token & current,
	  min::int32 indent,
	  BRA::indentation_mark indentation_mark,
	  BRA::bracket_stack * bracket_stack_p )
{
    BRA::bracketed_pass pass =
        (BRA::bracketed_pass) parser->pass_stack;

    BRA::indentation_mark indentation_found =
        min::NULL_STUB;
	// If not NULL_STUB, current token is an end-of-
	// line and current->previous token is the last
	// token of an indentation mark.

    // Discover if the subexpression we are scanning is
    // a typed bracketed subexpression, and if yes:
    //
    //     save selectors
    //     recompute selectors
    //     use the fact that saved_selectors is not 0
    //         to indicate we are not in element-list
    //
    BRA::typed_opening typed_opening = min::NULL_STUB;
    TAB::flags saved_selectors;
    if ( indentation_mark == min::NULL_STUB
         &&
	 bracket_stack_p != NULL )
    {
        typed_opening =
	    (BRA::typed_opening)
	    bracket_stack_p->opening_bracket;
	if ( typed_opening != min::NULL_STUB )
	{
	    saved_selectors = selectors;
	    selectors = typed_opening->attr_selectors
	              | PAR::ALWAYS_SELECTOR;
	}
    }

    TAB::flags trace_flags = parser->trace_flags;
    if ( trace_flags & pass->trace_subexpressions )
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

    while ( true )
    {
        // Truncate if end of file.
	//
	if ( current->type == LEXSTD::end_of_file_t )
	    return false;

	// Ensure there is a next token.
	//
	if ( current->next == parser->first )
	{
	    parser->input->add_tokens
		( parser, parser->input );
	    MIN_REQUIRE
		( current->next != parser->first );
	}

	// Process line breaks.
	//
	if ( current->type == LEXSTD::line_break_t )
	{
	    // Move forward to next token that is not
	    // a line break or full line comment.
	    //
	    PAR::token next = current->next;
	    while ( true )
	    {
		if ( next->type != LEXSTD::line_break_t
		     &&
		     next->type != LEXSTD::comment_t )
		    break;

		if ( next->next == parser->first )
		{
		    parser->input->add_tokens
			( parser, parser->input );
		    MIN_REQUIRE
		        (    next->next
			  != parser->first );
		}
		next = next->next;
	    }

	    min::uns32 next_indent =
	        next->type == LEXSTD::end_of_file_t ?
		0 :
		next->indent;

	    // Delete the line breaks and full line
	    // comments skipped (keeping the line break
	    // at `current') and find the bounds of any
	    // comments that are not indented as much
	    // as the indent of next.
	    //
	    bool iic_exists = false;
	    min::phrase_position iic_position;
		// Data on insufficiently indented
		// comments.  Includes begin of first
		// such and end of last such.

	    while ( current->next != next )
	    {
		if (    current->next->type
		     == LEXSTD::comment_t
		     &&
		       current->next->indent
		     < next_indent )
		{
		    if ( ! iic_exists )
		    {
		        iic_exists = true;
			iic_position.begin =
			    current->next
			           ->position.begin;
		    }
		    iic_position.end =
		        current->next->position.end;
		}
		    
		PAR::free
		    ( PAR::remove ( first_ref(parser),
				    current->next ) );
	    }

	    // Issue warning for any insufficiently
	    // indented comments.
	    //
	    if ( iic_exists )
	    {
		parser->printer
		    << min::bom
		    << min::set_indent ( 9 )
		    << "WARNING: comments NOT indented"
		       " as much as following line; "
		    << min::pline_numbers
			   ( parser->input_file,
			     iic_position )
		    << ":" << min::eom;
		min::print_phrase_lines
		    ( parser->printer,
		      parser->input_file,
		      iic_position );
		++ parser->error_count;
	    }

	    if ( indentation_found != min::NULL_STUB )
	    {
		// Tokens that bracket lines to be
		// scanned.  mark_end is the last token
		// of the indentation mark, and next is
		// the first token after the paragraph.
		//
		PAR::token mark_end = current->previous;
		PAR::token next = current;

		// Scan lines of paragraph.
		//
		// First be sure paragraph has some
		// lines.
		//
		if (    current->next->type
		     != LEXSTD::end_of_file_t
		     &&
		     relative_indent
		         ( parser,
			   pass->indentation_offset,
			   current->next, indent )
			 > 0 )
		{
		    // Compute selectors and paragraph
		    // indent for indented subparagraph.
		    //
		    TAB::flags new_selectors =
			selectors;
		    new_selectors |=
			indentation_found->new_selectors
					 .or_flags;
		    new_selectors &= ~
			indentation_found->new_selectors
					 .not_flags;
		    new_selectors ^=
			indentation_found->new_selectors
					 .xor_flags;
		    new_selectors |=
		        PAR::ALWAYS_SELECTOR;

		    min::int32 paragraph_indent =
		        current->next->indent;

		    MIN_REQUIRE
		        (    (unsigned) paragraph_indent
			  != LEX::AFTER_GRAPHIC );

		    // Delete line break.
		    //
		    current = current->next;
		    PAR::free
			( PAR::remove
			    ( first_ref(parser),
			      current->previous ) );

		    while ( true )
		    {
			// Find a paragraph line.
			//
			PAR::token previous =
			    current->previous;
			bool separator_found =
			  BRA::
			   parse_bracketed_subexpression
				( parser, new_selectors,
				  current,
				  paragraph_indent,
				  indentation_found,
				  bracket_stack_p );
			PAR::token first =
			    previous->next;
			next = current;
			if ( BRA::is_closed
			         ( bracket_stack_p ) )
			{
			    MIN_REQUIRE
			        ( ! separator_found );
			    next = bracket_stack_p
			              ->closing_first;
			}

			// Compact line subsubexp.
			//
			if ( first != next )
			{
			    min::phrase_position
			        position;
			    position.begin =
			        first->position.begin;
			    position.end =
			        next->previous
				    ->position.end;

			    PAR::attr attributes[2];
			    unsigned n = 0;
			    attributes[n++] =
			        PAR::attr
			          ( min::dot_type,
				    PAR::new_line );

			    if ( separator_found )
			    {
			        min::gen terminator =
				  indentation_found
				  ->line_sep->label;

			        PAR::remove
				    ( parser,
				      next,
				      terminator );
				first = previous->next;
				    // In case first was
				    // removed.
				attributes[n++] =
				    PAR::attr
				      ( min::
				        dot_terminator,
					terminator );
			    }

			    PAR::compact
				( parser,
				  pass->next,
				  selectors,
				  first, next,
				  position,
				  trace_flags,
				  PAR::BRACKETING,
				  n, attributes );

			    value_type_ref(first) =
			        PAR::new_line;
			}

			// See if there are more lines
			// in the paragraph.
			//
			if ( separator_found )
			    continue;
			else if
			    ( BRA::is_closed
			          ( bracket_stack_p )
			      ||
			         current->type
			      == LEXSTD::end_of_file_t
			      ||
			         current->next->type
			      == LEXSTD::end_of_file_t 
			      ||
			      ( current->next->indent
			        !=
				LEX::AFTER_GRAPHIC
				&&
			          (int)
				  current->next->indent
			        < paragraph_indent ) )
			    break;

			// Delete line break.
			//
			current = current->next;
			PAR::free
			    ( PAR::remove
				( first_ref(parser),
				  current->previous ) );
		    }

		    MIN_REQUIRE ( next == current );
		}

		PAR::token first = mark_end->next;
		min::phrase_position position;
		position.begin =
		    PAR::remove
			( parser, first,
			  indentation_found->label );
		position.end = next->previous
		                   ->position.end;

		PAR::attr attributes[1] =
		    { PAR::attr
		          ( min::dot_type,
			    indentation_found->
			        label ) };

		PAR::compact
		    ( parser, pass->next,
		      selectors,
		      first, next, position,
		      trace_flags,
		      PAR::BRACKETING,
		      1, attributes );

		value_type_ref(first) =
		    indentation_found->label;

		// Terminate subexpression if closing
		// bracket was found during indentation
		// processing, or if current token is an
		// end of file.
		// 
		if ( BRA::is_closed ( bracket_stack_p )
		     ||
		        current->type
		     == LEXSTD::end_of_file_t )
		    return false;

		// Otherwise fall through to process
		// line break at current that is after
		// indented lines.
		//
		indentation_found = min::NULL_STUB;
	    }

	    // If indentation was found, current may
	    // have changed.  In any case, it is a
	    // line break followed by a token that is
	    // not a line break or comment.
	    //
	    MIN_REQUIRE (    current->type
	                  == LEXSTD::line_break_t );
	    next = current->next;
	    MIN_REQUIRE (    next->type
	                  != LEXSTD::line_break_t
		          &&
		             next->type
		          != LEXSTD::comment_t );

	    // Truncate expression if line break is
	    // followed by an end of file.
	    //
	    if ( next->type == LEXSTD::end_of_file_t )
		return false;

	    // Now next is neither a line break or end
	    // of file or comment.

	    // Truncate subexpression if next token
	    // indent is at or before indent argument.
	    //
	    if ( next->indent != LEX::AFTER_GRAPHIC
	         &&
		    relative_indent
		          ( parser,
			    pass->indentation_offset,
			    next, indent )
		 <= 0 )
		return false;

	    // Remove line break and move to next token.
	    //
	    PAR::free ( PAR::remove ( first_ref(parser),
		                      current ) );
	    current = next;
	    continue;
	}
	else if ( current->type == LEXSTD::comment_t )
	{
	    // Comment that follows non-comment tokens
	    // on a line.  Remove and continue.

	    current = current->next;
	    PAR::free
		( PAR::remove ( first_ref(parser),
		                current->previous ) );
	    continue;
	}

	MIN_REQUIRE
	    ( indentation_found == min::NULL_STUB );
	MIN_REQUIRE
	    ( current->type != LEXSTD::end_of_file_t );
	MIN_REQUIRE
	    ( current->type != LEXSTD::line_break_t );
	MIN_REQUIRE
	    ( current->type != LEXSTD::comment_t );

	// Process quoted strings.
	//
	if ( current->type == LEXSTD::quoted_string_t )
	{
	    if ( current != parser->first
	         &&
		    current->previous->type
		 == LEXSTD::quoted_string_t )
	    {
	        // Merge current and current->previous,
		// which are both quoted strings.
		//
	        min::push
		    ( (PAR::string_insptr)
		          current->previous->string,
		      current->string->length,
		      min::begin_ptr_of
		          ( current->string ) );
		current->previous->position.end =
		    current->position.end;
		current = current->next;
		PAR::free
		    ( PAR::remove
			( first_ref(parser),
			  current->previous ) );
	    }
	    else
		current = current->next;

	    continue;
	}

	// If lookup key in bracket table.
	//
	PAR::token key_first = current;
	    // First token of key, or == current if
	    // there is no key.
	TAB::key_prefix key_prefix;
	TAB::root root =
	    find_entry ( parser, current, key_prefix,
			 TAB::ALL_FLAGS,
			 pass->bracket_table );
	while ( true )
	{
	    // Each iteration of this loop examines the
	    // found entry to see if it is a selected
	    // opening bracket or indentation mark, or a
	    // not necessarily selected closing bracket
	    // or line separator that matches a symbol
	    // active because of the bracket_stack or
	    // indentation_mark arguments.
	    //
	    if ( root == min::NULL_STUB )
	    {
	        // No active bracket table entry found.

		current = key_first->next;
		break;
	    }

	    min::uns32 subtype =
		min::packed_subtype_of ( root );

	    if ( trace_flags & PAR::TRACE_KEYS )
	        parser->printer
		    << "BRACKETED SUBEXPRESSION PARSER"
		       " FOUND "
		    << ( root->selectors & selectors ?
		         "SELECTED KEY " :
			 "UNSELECTED KEY " )
		    << min::pgen_quote ( root->label )
		    << min::indent << " OF SUBTYPE "
		    << min::name_of_packed_subtype
		           ( min::packed_subtype_of
			         ( root ) )
		    << min::eol;

	    if ( ( subtype == BRA::OPENING_BRACKET
	           ||
		   subtype == BRA::TYPED_OPENING )
	         &&
		 ( selectors & root->selectors ) != 0 )
	    {
		BRA::opening_bracket opening_bracket =
		    (BRA::opening_bracket) root;

		TAB::flags new_selectors =
		    selectors;
		new_selectors |=
		    opening_bracket->new_selectors
				    .or_flags;
		new_selectors &= ~
		    opening_bracket->new_selectors
				    .not_flags;
		new_selectors ^=
		    opening_bracket->new_selectors
				    .xor_flags;
		new_selectors |=
		    PAR::ALWAYS_SELECTOR;

		bool full_lines =
		    (   opening_bracket->options
		      & BRA::FULL_LINES );

		BRA::bracket_stack cstack
		    ( full_lines ? NULL :
		                   bracket_stack_p );
		cstack.opening_bracket =
		    opening_bracket;

		PAR::token previous = current->previous;
		BRA::parse_bracketed_subexpression
		    ( parser, new_selectors,
		      current,
		      full_lines ?
			  - pass->indentation_offset :
			  indent,
		      min::NULL_STUB,
		      & cstack );
		PAR::token first = previous->next;

		if (    cstack.closing_next
		     == cstack.closing_first )
		{
		    // Found a closing bracket that is
		    // not ours, or found a line break
		    // or end of file that terminates a
		    // paragraph with the closing
		    // bracket missing.

		    // Compute location `next' just
		    // before which closing bracket
		    // should be inserted.
		    //
		    PAR::token next =
			(    cstack.closing_next
			  == min::NULL_STUB ?
			  current :
			  cstack.closing_next );

		    parser->printer
			<< min::bom
			<< min::set_indent ( 7 )
			<< "ERROR: missing"
			   " closing bracket "
			<< min::pgen_quote
			     ( opening_bracket->
			       closing_bracket->
				   label )
			<< " inserted before ";

		    if ( next->value != min::MISSING() )
		        parser->printer
			    << min::pgen_quote
			      ( next->value )
			    << "; ";
		    else if (    next->type
		              == LEXSTD::line_break_t )
		        parser->printer
			    << "end of line; ";
		    else if (    next->type
		              == LEXSTD::end_of_file_t )
		        parser->printer
			    << "end of file; ";
		    else if
		        (    next->type
		          == LEXSTD::quoted_string_t )
		        parser->printer
			    << "\"... \"; ";

		    parser->printer
			<< min::pline_numbers
			       ( parser->input_file,
				 next->position )
			<< ":" << min::eom;
		    min::print_phrase_lines
			( parser->printer,
			  parser->input_file,
			  next->position );
		    ++ parser->error_count;

		    min::phrase_position position;
		    position.end =
			next->previous->position.end;
		    position.begin =
			PAR::remove
			    ( parser, first,
			      opening_bracket->label );

		    if (    opening_bracket->reformatter
		         == min::NULL_STUB
			 ||
			 ( * opening_bracket->
			       reformatter->
			       reformatter_function )
		             ( parser, (PAR::pass) pass,
			       selectors,
			       first, current, position,
			       trace_flags,
			       (TAB::root)
			           opening_bracket )
		       )
		    {
			PAR::attr attributes[2] =
			    { PAR::attr
				  ( min::dot_initiator,
				    opening_bracket->
					label ),
			      PAR::attr
				  ( min::dot_terminator,
				    opening_bracket->
				      closing_bracket->
					      label ) };

			PAR::compact
			    ( parser, pass->next,
			      selectors,
			      first, next, position,
			      trace_flags,
			      PAR::BRACKETING,
			      2, attributes, 1 );

			value_type_ref(first) =
		    	    opening_bracket->label;
		    }

		    if (    cstack.closing_next
			 == min::NULL_STUB )
		    {
			// Found a line break before
			// non-indented line or an end
			// of file when a closing
			// bracket was expected.  Go
			// to appropriate code above
			// to process.
			//
			break;
		    }

		    // Found a closing bracket that is
		    // not ours.  It must be in the
		    // bracket_stack and so needs to
		    // be kicked to our caller.
		    //
		    return false;
		}
		else
		{
		    MIN_REQUIRE (    cstack.closing_next
		                  == current );

		    min::phrase_position position;
		    position.end =
			current->previous->position.end;
		    PAR::remove ( parser, current,
			          cstack.opening_bracket
			              ->closing_bracket
				      ->label );
		    position.begin =
			PAR::remove
			    ( parser, first,
			      opening_bracket->label );

		    if (    opening_bracket->reformatter
		         == min::NULL_STUB
			 ||
			 ( * opening_bracket->
			       reformatter->
			       reformatter_function )
		             ( parser, (PAR::pass) pass,
			       selectors,
			       first, current,
			       position,
			       trace_flags,
			       (TAB::root)
				   opening_bracket )
		       )
		    {
			PAR::attr attributes[2] =
			    { PAR::attr
				  ( min::dot_initiator,
				    opening_bracket->
					label ),
			      PAR::attr
				  ( min::dot_terminator,
				    opening_bracket->
				      closing_bracket->
					    label ) };

			PAR::compact
			    ( parser, pass->next,
			      selectors,
			      first, current, position,
			      trace_flags,
			      PAR::BRACKETING,
			      2, attributes, 1 );

			value_type_ref(first) =
		    	    opening_bracket->label;
		    }
		    break;
		}
	    }

	    else if ( subtype == BRA::CLOSING_BRACKET )
	    {
		BRA::closing_bracket closing_bracket =
		    (BRA::closing_bracket) root;

		for ( BRA::bracket_stack * p =
			  bracket_stack_p;
		      p != NULL;
		      p = p->previous )
		{
		    if (    p->opening_bracket
		         != min::NULL_STUB
			 &&
		            p->opening_bracket
		             ->closing_bracket
			 == closing_bracket )
		    {
		        p->closing_first = key_first;
			p->closing_next = current;

			for ( BRA::bracket_stack *
			          q = bracket_stack_p;
			      q != p;
			      q = q->previous )
			    q->closing_first =
			        q->closing_next =
				    key_first;

			return false;
		    }
		}

		// Closing bracket does not match any
		// bracket stack entry; reject key.
	    }
	    else if (    subtype
	              == BRA::TYPED_MIDDLE )
	    {
		if ( typed_opening != min::NULL_STUB
		     &&
		        typed_opening->typed_middle
		     == (BRA::typed_middle) root )
		{
		    if ( saved_selectors != 0 )
		    {
			// Beginning of element list
			//
			selectors = saved_selectors;
			saved_selectors = 0;
		    }
		    else
		    {
			// End of element list
			//
			saved_selectors = selectors;
			selectors =
			      typed_opening->
			          attr_selectors
			    | PAR::ALWAYS_SELECTOR;
		    }

		    break;
		}
	    }
	    else if ( subtype == BRA::INDENTATION_MARK
	              &&
		         ( selectors & root->selectors )
		      != 0 )
	    {
                if (    current->type
		     == LEXSTD::line_break_t
		     ||
		        current->type
		     == LEXSTD::end_of_file_t )
		{
		    indentation_found =
			(BRA::indentation_mark) root;
		    break;
		}

		// Indentation mark not at end of line
		// or end of file; reject key.
	    }
	    else if (    subtype
	              == BRA::LINE_SEP )
	    {
		BRA::line_sep line_sep =
			(BRA::line_sep) root;
                if (    line_sep->indentation_mark
		     == indentation_mark )
		    return true;

		// Indentation separator does not match
		// indentation_mark argument; reject
		// key.
	    }

	    if ( trace_flags & PAR::TRACE_KEYS )
	        parser->printer
		    << "BRACKETED SUBEXPRESSION PARSER"
		       " REJECTED KEY "
		    << min::pgen_quote ( root->label )
		    << min::eol;

	    root = PAR::find_next_entry
	               ( parser, current, key_prefix,
			 TAB::ALL_FLAGS, root );
	}
    }

    return false;
}

// Bracketed Reformatters
// --------- ------------

static bool label_reformatter_function
        ( PAR::parser parser,
	  PAR::pass pass,
	  TAB::flags selectors,
	  PAR::token & first,
	  PAR::token next,
	  const min::phrase_position & position,
	  TAB::flags trace_flags,
	  TAB::root entry )
{
    make_label ( parser, first, next );
    first->position = position;

    PAR::trace_subexpression
	( parser, first, trace_flags );

    return false;
}

// Skip to next key (punctuation mark).  Return token
// after key in `current', first token of key in
// `key_first', count of number of tokens skipped
// NOT counting key tokens in `skip_count'.  `next' is
// next token after tokens that may be skipped or part
// of key (i.e., end of token sequence marker).
//
// If key found, return its subtype, and return its
// label in `key_label'.  But if key not found (so
// key_first and current are set == next), return 0.
//
// Key_table contains the keys with various selectors.
// Only keys with a selector that is also in the
// `selectors' argument are recognized.  If a key
// is not initially recognized, selectors is reset
// to skip_selectors with skip_count is incremented.
//
inline min::uns32 get_next_key
        ( PAR::parser parser,
	  PAR::token & key_first,
	  PAR::token & current,
	  PAR::token next,
	  TAB::flags selectors,
	  TAB::flags skip_selectors,
	  min::unsptr & skip_count,
	  min::gen & key_label,
	  TAB::key_table key_table )
{
    key_first = current;
    skip_count = 0;

    TAB::root root = min::NULL_STUB;
    TAB::key_prefix key_prefix;
    while ( current != next )
    {
	root =
	    find_entry ( parser, current, key_prefix,
			 selectors,
			 key_table, next );
	if ( root == min::NULL_STUB )
	{
	    key_first = current = key_first->next;
	    ++ skip_count;
	    selectors = skip_selectors;
	}
	else
	    break;
    }

    if ( root == min::NULL_STUB )
        return 0;
    else
    {
        key_label = root->label;
	return min::packed_subtype_of ( root );
    }
}

// Delete the punctuation just found.  Update first,
// start, and key_first as necessary.
//
inline void remove ( PAR::parser parser, 
		     PAR::token & first,
		     PAR::token & start,
                     PAR::token & key_first,
		     PAR::token current )
{
    if ( key_first == first ) first = current;
    if ( key_first == start ) start = current;

    while ( key_first != current )
    {
        key_first = key_first->next;
	PAR::free
	    ( PAR::remove
		( first_ref(parser),
		  key_first->previous ) );
    }
}

// Turn the sequence of tokens from first through
// next->previous into a label valued token (of type to
// be set by caller) and a sequence of flags_type tokens
// containing flags for the label.  The flags tokens are
// identified by being BRACKETED tokens with .initiator
// == flags_initiator that are after the label token.
// However, the `first' token is assumed to NOT be a
// flags token.  Flags token types are changed to flags_
// type.
//
inline void make_label_with_flags
	( PAR::parser parser,
	  PAR::token & first,
	  PAR::token next,
	  min::gen flags_initiator,
	  min::uns32 flags_type )
{
    while ( first != next
            &&
	    first->next != next
	    &&
	    next->previous->type == PAR::BRACKETED )
    {
	min::obj_vec_insptr vp
		( next->previous->value );
	min::attr_insptr ap ( vp );
	min::locate ( ap, min::dot_initiator );
	if ( min::get ( ap ) != flags_initiator )
	    break;
	next = next->previous;
	next->type = flags_type;
    }
    make_label ( parser, first, next );
}

// Announce the punctuation just found is illegal and
// will be ignored, and delete it.   Update first,
// start, and key_first as necessary.
//
inline void punctuation_error
        ( PAR::parser parser,
	  PAR::token & first,
	  PAR::token & start,
	  PAR::token & key_first,
	  PAR::token current,
	  min::gen key_label )
{
    MIN_REQUIRE ( key_first != current );

    min::phrase_position position =
        { key_first->position.begin,
	  current->previous->position.end };

    parser->printer
	<< min::bom << min::set_indent ( 7 )
	<< "ERROR: unexpected punctuation "
	<< min::pgen_quote ( key_label )
	<< " - deleted and ignored; "
	<< min::pline_numbers
	       ( parser->input_file,
		 position )
	<< ":" << min::eom;
    min::print_phrase_lines
	( parser->printer,
	  parser->input_file,
	  position );
    ++ parser->error_count;

    ::remove ( parser, first, start,
               key_first, current );
}

// Announce `missing <message>' error with position
// just before current.
//
static void missing_error
        ( PAR::parser parser,
	  PAR::token current,
	  const char * message )
{
    min::phrase_position position =
        { current->position.begin,
	  current->position.begin };

    parser->printer
	<< min::bom << min::set_indent ( 7 )
	<< "ERROR: missing " << message
	<< "; "
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

static void set_attr_flags
	( PAR::parser parser,
	  PAR::token current,
	  PAR::token next,
	  min::attr_insptr expap,
	  min::gen label,
	  min::uns32 attr_flags_type,
	  const min::flag_parser * flag_parser )
{
    min::locate ( expap, label );
    while ( current->next->type == attr_flags_type )
    {
        MIN_REQUIRE ( current->next != next );
	min::obj_vec_insptr vp ( current->next->value );

	for ( min::unsptr i = 0;
	      i < min::size_of ( vp ); ++ i )
	{
	    min::gen flags_text = vp[i];
	    if ( min::is_obj ( flags_text ) )
	    {
		min::obj_vec_insptr vp ( flags_text );
		min::attr_insptr ap ( vp );
		min::locate ( ap, min::dot_type );
		min::gen type = get ( ap );
		if ( type == min::doublequote
		     ||
		     type == min::number_sign )
		    flags_text = vp[0];
	    }

	    if ( min::is_str ( flags_text ) )
	    {
	        min::str_ptr sp ( flags_text );
	        min::unsptr len = min::strlen ( sp );
	        char text_buffer[len+1];
	        min::strcpy ( text_buffer, sp );
	        min::uns32 flags[len];
	        len = min::parse_flags
	            ( flags, text_buffer, flag_parser );
	        for ( min::unsptr j = 0; j < len; ++ j )
	            min::set_flag ( expap, flags[j] );
		
		if ( text_buffer[0] != 0 )
		{
		    min::attr_insptr ap ( vp );
		    min::locate
		        ( ap, min::dot_position );
		    min::phrase_position_vec_insptr
		        pos = min::get ( ap );
		    min::phrase_position position =
		        pos[i];
		    parser->printer
			<< min::bom
			<< min::set_indent ( 7 )
			<< "ERROR: bad flag(s) "
			<< text_buffer
			<< " in "
			<< min::pgen_quote
			       ( flags_text )
			<< "; "
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
	    }
	    else
	    {
		min::attr_insptr ap ( vp );
		min::locate ( ap, min::dot_position );
		min::phrase_position_vec_insptr
		    pos = min::get ( ap );
		min::phrase_position position = pos[i];
		parser->printer
		    << min::bom
		    << min::set_indent ( 7 )
		    << "ERROR: bad flags specifier "
		    << min::pgen_quote
			   ( flags_text )
		    << "; "
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
	}

	PAR::free
	    ( PAR::remove
		( first_ref(parser),
		  current->next ) );
    }
}

static bool typed_bracketed_reformatter_function
        ( PAR::parser parser,
	  PAR::pass pass,
	  TAB::flags selectors,
	  PAR::token & first,
	  PAR::token next,
	  const min::phrase_position & position,
	  TAB::flags trace_flags,
	  TAB::root entry )
{
    // Types of tokens after 1st pass.
    //
    const min::uns32 TYPE =       PAR::TEMPORARY_TT + 0;
        // .type value
    const min::uns32 ATTR_LABEL = PAR::TEMPORARY_TT + 1;
    const min::uns32 ATTR_FLAGS = PAR::TEMPORARY_TT + 2;
    const min::uns32 ATTR_VALUE = PAR::TEMPORARY_TT + 3;
        // Attribute label, flags, and value.  Must be
	// consecutive tokens in that order.  There is
	// only 1 label token, 0 or more flags tokens,
	// and 1 value token.
    const min::uns32 ATTR_TRUE =  PAR::TEMPORARY_TT + 4;
    const min::uns32 ATTR_FALSE = PAR::TEMPORARY_TT + 5;
        // Attribute label for attribute with TRUE or
	// FALSE value implied.  May be followed by
	// ATTR_FLAGS tokens.
    //
    // Other token types are object elements after 1st
    // pass.
    //
    // Each ATTR_LABEL token is followed by a corres-
    // ponding ATTR_VALUE token.  ATTR_TRUE and ATTR_
    // FALSE tokens are attribute labels that have NO
    // following ATTR_VALUE token; their values are
    // implied and are either TRUE or FALSE.
    //
    // ATTR_FALSE token values are attribute labels that
    // do NOT include the attribute negator, but the
    // position of these tokens DOES include the
    // attribute negator.
    //
    // There can be a type at the beginning, the `begin
    // type', and also at the end, the `end type'.  But
    // there can be only one TYPE token.  If both types
    // are given, only the `begin type' generates a TYPE
    // token, and error messages concerning type mis-
    // matches are generated when end type is found.
    //
    // If there are two ATTR_LABEL/TRUE/FALSE tokens
    // with the same attribute label, it is an error
    // detected during the second pass.

    min::unsptr attr_count = 0;
        // Count of attribute labels.
    min::unsptr element_count = 0;
        // Count of object elements.
    bool after_elements = false;
        // True if after TYPED_MIDDLE.
    bool after_attribute = false;
        // True if attribute has been found since
	// after_elements was set true.
    PAR::token after_negator;
        // If attr negator found, first token after
	// negator.
    PAR::token type_token = min::NULL_STUB;
        // TYPE token.  First type encountered.
	// See above.
    TAB::flags next_selectors;
        // Selectors in particular contexts.

    BRA::typed_opening typed_opening =
        (BRA::typed_opening) entry;
    TAB::key_table key_table = typed_opening->key_table;
    min::gen flags_initiator =
        typed_opening->typed_attr_flags_initiator;
    const min::flag_parser * attr_flag_parser =
        typed_opening->typed_attr_flag_parser;

    // Data for macros below:
    //
    PAR::token start;
    	// == current at start of scan; NOT set by NEXT!
    min::unsptr skip_count;
        // Count of tokens skipped by NEXT; except does
	// NOT include key.
    PAR::token key_first;
        // First token of key found by NEXT, or ==
	// current if no key found.
    PAR::token current = first;
        // Next token to be scanned.
    min::uns32 key_subtype;
	// Subtype of key found by NEXT, or 0 if no key
	// found; i.e., end of subexpression found
	// instead.
    min::gen key_label;
        // Label of key found by NEXT, or unused if no
	// key found.

    if ( first == next ) goto DONE;
        // Shortcut for {}
	// Cannot put this before variable declarations
	// above.

#   define NEXT(selectors) \
	key_subtype = \
	    get_next_key ( parser, key_first, \
	                   current, next, \
			   selectors, selectors, \
		           skip_count, key_label, \
			   key_table )
#   define NEXT2(selectors,skip_selectors) \
	key_subtype = \
	    get_next_key ( parser, key_first, \
	                   current, next, \
			   selectors, skip_selectors, \
		           skip_count, key_label, \
			   key_table )
#   define LABEL(t) \
	{ \
	    MIN_REQUIRE ( start != key_first ); \
	    make_label ( parser, \
	                 start, key_first ); \
	    start->type = t; \
	}
#   define LABEL_WITH_FLAGS(t) \
	{ \
	    MIN_REQUIRE ( start != key_first ); \
	    make_label_with_flags \
	        ( parser, start, key_first, \
		  flags_initiator, \
		  ATTR_FLAGS ); \
	    start->type = t; \
	}
#   define PUNCTUATION_ERROR \
	::punctuation_error \
	    ( parser, first, start, key_first, \
	      current, key_label )
#   define REMOVE \
	if ( key_subtype != 0 ) \
	    ::remove ( parser, first, start, \
	               key_first, current )

    // In syntax below
    //
    //     {	denotes	    typed_opening
    //     :	denotes	    typed_attr_begin
    //     =	denotes	    typed_attr_equal
    //     ,	denotes	    typed_attr_sep
    //     |	denotes	    typed_middle
    //     }	denotes	    closing_bracket
    //
    // Start Pass 1 at beginning of subexpression.
    //
    // {}		goto DONE
    // {|		goto ELEMENTS
    // {<type>}		type_token = ...;
    // 			goto DONE
    // {<type>|		type_token = ...;
    // 			goto ELEMENTS
    // {<type>:		type_token = ...;
    // 			goto ATTRIBUTES
    start = current;
    while ( true )
    {
	NEXT (   BRA::MIDDLE_SELECTOR
	       + BRA::ATTR_SELECTOR );

	if ( key_subtype == 0
	     ||
	     key_subtype == BRA::TYPED_ATTR_BEGIN
	     ||
	     key_subtype == BRA::TYPED_MIDDLE )
	{
	    REMOVE;

	    if ( start != key_first )
	    {
		LABEL(TYPE);
		type_token = start;
	    }

	    if ( key_subtype == BRA::TYPED_MIDDLE )
	        goto ELEMENTS;
	    else if (    key_subtype
	              == BRA::TYPED_ATTR_BEGIN )
	        goto ATTRIBUTES;
	    else
		goto DONE;
	}
	else PUNCTUATION_ERROR;
    }

ATTRIBUTES:

    // Come here if:
    //     Before elements and after : or ,
    //     After elements and after | or ,
    //
    // <attr-negator>? <attr-label> <attr-flags>* ,
    // 			goto ATTRIBUTES
    // <attr-label> <attr-flags>* =
    // 			goto ATTRIBUTE_VALUE
    //
    // If ! after_elements:
    //
    // <attr-negator>? <attr-label> <attr-flags>* |
    // 			goto ELEMENTS
    // <attr-negator>? <attr-label> <attr-flags>* }
    // 			goto DONE
    // |
    //                  MISSING ATTR ERROR
    //                  goto ELEMENTS
    // }
    //                  MISSING ATTR ERROR
    // 			goto DONE
    //
    // If after_elements:
    //
    //     <attr-negator>? <attr-label> <attr-flags>* :
    //	 		goto END_TYPE
    //     :
    //                  MISSING ATTR ERROR
    //	 		goto END_TYPE
    //
    // If after_elements and after_attribute:
    //
    //     <attr-negator>? <attr-label> <attr-flags>* }
    //                  MISSING END TYPE ERROR
    // 			goto DONE
    //     }
    //                  MISSING ATTR ERROR
    //                  MISSING END TYPE ERROR
    // 			goto DONE
    //
    // If after_elements and ! after_attribute:
    //
    //     <type> } 	goto END_TYPE_FOUND
    //
    //     } 		goto DONE
    //
    after_negator = min::NULL_STUB;
    start = current;
    next_selectors = BRA::MIDDLE_SELECTOR
	           + BRA::ATTR_SELECTOR
	           + BRA::NEGATOR_SELECTOR;
    while ( true )
    {
	NEXT2 ( next_selectors,
		  BRA::MIDDLE_SELECTOR
	        + BRA::ATTR_SELECTOR );
	next_selectors = BRA::MIDDLE_SELECTOR
	               + BRA::ATTR_SELECTOR;
	if ( key_subtype == BRA::TYPED_ATTR_NEGATOR )
	    after_negator = current;
	else if  ( (    key_subtype
	             == BRA::TYPED_ATTR_BEGIN
	             &&
		     after_elements )
	           ||
	           ( key_subtype == BRA::TYPED_MIDDLE
	             &&
		     ! after_elements )
	           ||
	           key_subtype == BRA::TYPED_ATTR_SEP
	           ||
	           ( key_subtype == 0
		     &&
		     ( ! after_elements
		       ||
		       after_attribute ) ) )
	{
	    min::uns32 type = ATTR_TRUE;
	    min::phrase_position position =
		{ start->position.begin,
		  key_first->previous->position.end };
	    if ( after_negator != min::NULL_STUB
	         &&
		 after_negator != current )
	    {
		PAR::token negator_current = start;
		::remove ( parser, first, start,
	                   negator_current,
			   after_negator );
		    // 3rd and 4th argument must be
		    // separate memory variables

		type = ATTR_FALSE;
	    }
	    else if ( start == key_first )
	    {
	        if (    key_subtype
		     == BRA::TYPED_ATTR_SEP )
		{
		    PUNCTUATION_ERROR;
		    continue;
		}
		missing_error
		    ( parser, key_first,
		         key_subtype == 0
		      && after_elements ?
		      "attribute and end type" :
		      "attribute" );

		if (    key_subtype 
		          == BRA::TYPED_MIDDLE )
		    goto ELEMENTS;
		else if (    key_subtype 
		          == BRA::TYPED_ATTR_BEGIN )
		    goto END_TYPE;
		else
		    goto DONE;
	    }
	    REMOVE;
	    LABEL_WITH_FLAGS(type);
	    start->position = position;

	    ++ attr_count;
	    after_attribute = after_elements;

	    if ( key_subtype == BRA::TYPED_ATTR_SEP )
	        goto ATTRIBUTES;
	    else if (    key_subtype
	              == BRA::TYPED_ATTR_BEGIN )
	        goto END_TYPE;
	    else if ( key_subtype == BRA::TYPED_MIDDLE )
	        goto ELEMENTS;
	    else if ( after_elements )
	    {
		MIN_REQUIRE ( key_subtype == 0 );
		MIN_REQUIRE ( after_attribute );

		missing_error
		    ( parser, current, "end type" );

		goto DONE;
	    }
	    else
	        goto DONE;
	}
	else if ( key_subtype == BRA::TYPED_ATTR_EQUAL )
	{
	    if ( start == key_first )
	    {
	        PUNCTUATION_ERROR;
		continue;
	    }
	    REMOVE;
	    LABEL_WITH_FLAGS(ATTR_LABEL);
	    ++ attr_count;
	    after_attribute = after_elements;
	    goto ATTRIBUTE_VALUE;
	}
	else if ( key_subtype == 0 )
	{
	    if ( start != key_first )
	        goto END_TYPE_FOUND;

	    goto DONE;
	}
	else PUNCTUATION_ERROR;
    }

ATTRIBUTE_VALUE:

    // Come here if after =
    //
    // <value> ,	goto ATTRIBUTES
    //
    // If ! after_elements:
    //
    //     <value> |	goto ELEMENTS
    //     <value> }	goto DONE
    //
    // If after_elements:
    //
    //     <value> :	goto END_TYPE
    //     <value> }	MISSING END TYPE ERROR
    //     		goto DONE
    //
    // If <value> is empty add
    // 	    MISSING ATTRIBUTE VALUE ERROR; FALSE ASSUMED
    //
    start = current;
    while ( true )
    {
	NEXT (   BRA::MIDDLE_SELECTOR
	       + BRA::ATTR_SELECTOR );
	if ( key_subtype == BRA::TYPED_ATTR_SEP
	     ||
	     ( key_subtype == BRA::TYPED_MIDDLE
	       &&
	       ! after_elements )
	     ||
	     ( key_subtype == BRA::TYPED_ATTR_BEGIN
	       &&
	       after_elements )
	     ||
	     key_subtype == 0 )
	{
	    REMOVE;
	    if ( start == current )
	    {
		missing_error
		    ( parser, current,
		      "attribute value;"
		      " FALSE assumed" );

		MIN_REQUIRE
		    (    start->previous->type
		      == ATTR_LABEL );
		start->previous->type = ATTR_FALSE;
	    }
	    else if ( start->next == current
	              &&
		      start->value != min::MISSING() )
	        start->type = ATTR_VALUE;
	    else
		LABEL ( ATTR_VALUE );

	    if ( key_subtype == BRA::TYPED_ATTR_BEGIN )
	        goto END_TYPE;
	    else if (    key_subtype
	              == BRA::TYPED_ATTR_SEP )
	        goto ATTRIBUTES;
	    else if ( key_subtype == BRA::TYPED_MIDDLE )
	        goto ELEMENTS;
	    else
	    {
		MIN_REQUIRE ( key_subtype == 0 );
		if ( after_elements )
		    missing_error
			( parser, current, "end type" );
		goto DONE;
	    }
	}
	else PUNCTUATION_ERROR;
    }

ELEMENTS:

    // Come here if after | and before elements
    //
    // <element>* |	goto ATTRIBUTES
    // <element>* }	MISSING | ERROR
    // 			goto DONE
    //
    after_elements = true;
    start = current;

    NEXT ( BRA::MIDDLE_SELECTOR );

    MIN_REQUIRE ( key_subtype == BRA::TYPED_MIDDLE
	          ||
	          key_subtype == 0 );

    element_count = skip_count;
    REMOVE;
    if ( key_subtype == BRA::TYPED_MIDDLE )
	goto ATTRIBUTES;
    else
    {
	min::phrase_position position =
	    { current->position.begin,
	      current->position.begin };
	key_label = typed_opening->typed_middle
				 ->label;

	parser->printer
	    << min::bom
	    << min::set_indent ( 7 )
	    << "ERROR: premature end of typed"
	       " bracketed subexpression; "
	    << min::pgen_quote ( key_label )
	    << " inserted; "
	    << min::pline_numbers
		   ( parser->input_file,
		     position )
	    << ":" << min::eom;
	min::print_phrase_lines
	    ( parser->printer,
	      parser->input_file,
	      position );
	++ parser->error_count;

	goto DONE;
    }

END_TYPE:

    // Come here if after : and after elements
    //
    // <type>}		goto DONE
    // 
    start = current;
    while ( true )
    {
	NEXT (   BRA::MIDDLE_SELECTOR
	       + BRA::ATTR_SELECTOR );
	if ( key_subtype == 0 )
	{
	    if ( start == key_first )
	    {
		missing_error
		    ( parser, current, "end type" );
		goto DONE;
	    }
	    else
	        goto END_TYPE_FOUND;
	}
	else PUNCTUATION_ERROR;
    }

END_TYPE_FOUND:

    // Come here when end type found, with start set
    // to first token and current to token after type.
    //
    LABEL(TYPE);

    if ( type_token == min::NULL_STUB )
	type_token = start;
    else
    {
	if (    type_token->value
	     != start->value )
	{
	    parser->printer
		<< min::bom
		<< min::set_indent ( 7 )
		<< "ERROR: end type "
		<< min::pgen_quote
		     ( start->value )
		<< " != begin type "
		<< min::pgen_quote
		     ( type_token->value )
		<< "; end type ignored; "
		<< min::pline_numbers
		       ( parser->input_file,
			 start->position )
		<< ":" << min::eom;
	    min::print_phrase_lines
		( parser->printer,
		  parser->input_file,
		  start->position );
	    ++ parser->error_count;
	}

	PAR::free
	    ( PAR::remove
		( first_ref(parser),
		  start ) );
    }

    goto DONE;

#   undef NEXT
#   undef LABEL
#   undef PUNCTUATION_ERROR
#   undef REMOVE

DONE:

    // Second Pass
    //
    min::locatable_gen exp;
    min::locatable_var
	    <min::phrase_position_vec_insptr>
	pos;

    ++ attr_count;	// For .position
    if ( type_token != min::NULL_STUB )
	++ attr_count;	// For .type

    // Hash table size.
    //
    min::uns32 h = attr_count;
    if ( h > 128 ) h = 128;

    exp = min::new_obj_gen (   3*( attr_count + 2 )
                             + element_count, h );

    min::init ( pos, parser->input_file,
		position, element_count );

    min::obj_vec_insptr expvp ( exp );
    min::attr_insptr expap ( expvp );
    min::locate ( expap, min::dot_position );
    min::set ( expap, min::new_stub_gen ( pos ) );
    min::set_flag
	( expap, min::standard_attr_hide_flag );

#   define SET_ATTR_FLAGS \
	if ( current->next->type == ATTR_FLAGS ) \
	    ::set_attr_flags \
	        ( parser, current, next, \
		  expap, label, ATTR_FLAGS, \
		  attr_flag_parser )

    min::locatable_gen label;
    min::locatable_gen value;
    min::gen old_value;
    min::gen value_type = min::MISSING();
    for ( PAR::token current = first;
	  current != next; )
    {
        if ( current->type == TYPE )
	{
	    label = min::dot_type;
	    value = current->value;
	    value_type = current->value;
	}
	else if ( current->type == ATTR_LABEL )
	{
	    MIN_REQUIRE ( current != next );
	    label = current->value;
	    SET_ATTR_FLAGS;
	    current = current->next;
	    PAR::free
		( PAR::remove
		      ( PAR::first_ref(parser),
			current->previous ) );
	    MIN_REQUIRE ( current->type == ATTR_VALUE );
	    value = current->value;
	}
        else if ( current->type == ATTR_FALSE )
	{
	    label = current->value;
	    value = min::FALSE;
	    SET_ATTR_FLAGS;
	}
        else if ( current->type == ATTR_TRUE )
	{
	    label = current->value;
	    value = min::TRUE;
	    SET_ATTR_FLAGS;
	}
	else
	{
	    // current is vector element
	    //
	    if ( current->value == min::MISSING() )
	    {
		current->type = PAR::DERIVED;
		PAR::value_ref(current) =
		    min::new_str_gen
			( min::begin_ptr_of
			      ( current->string ),
			  current->string->length );
		PAR::string_ref(current) =
		    PAR::free_string
		        ( current->string );
	    }
	    min::attr_push(expvp) = current->value;
	    min::push ( pos ) = current->position;
	    goto NEXT_ITEM;
	}

	min::locate ( expap, label );
	if ( min::get ( expap ) == min::NONE() )
	    min::set ( expap, value );
	else
	{
	    parser->printer
		<< min::bom
		<< min::set_indent ( 7 )
		<< "ERROR: attribute "
		<< min::pgen_quote ( label )
		<< " appears more than once;"
		   " later value "
		<< min::pgen_quote ( value )
		<< " ignored; "
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

NEXT_ITEM:

	current = current->next;
	PAR::free
	    ( PAR::remove
		  ( PAR::first_ref(parser),
		    current->previous ) );
    }

    expvp = min::NULL_STUB;
        // Necessary so trace_subexpression can open
	// pointer to object.

    first = PAR::new_token ( after_elements ?
                             PAR::BRACKETED :
			     PAR::PREFIX );
    PAR::put_before
	( first_ref(parser), next, first );

    PAR::value_ref(first) = exp;
    first->position = position;
    PAR::value_type_ref(first) = value_type;

    PAR::trace_subexpression
	( parser, first, trace_flags );

    return false;
}

min::locatable_var<PAR::reformatter>
    BRA::reformatter_stack ( min::NULL_STUB );

static void reformatter_stack_initialize ( void )
{
    min::locatable_gen label
        ( min::new_str_gen ( "label" ) );
    PAR::push_reformatter
        ( label, 0, 0, 0,
	  ::label_reformatter_function,
	  BRA::reformatter_stack );

    min::locatable_gen typed_bracketed
        ( min::new_lab_gen ( "typed", "bracketed" ) );
    PAR::push_reformatter
        ( typed_bracketed, 0, 0, 0,
	  ::typed_bracketed_reformatter_function,
	  BRA::reformatter_stack );
}
static min::initializer reformatter_initializer
    ( ::reformatter_stack_initialize );

// Bracketed Pass Command Function
// --------- ---- ------- --------

enum definition_type
    { BRACKET,
      INDENTATION_MARK };

static min::gen bracketed_pass_command
	( PAR::parser parser,
	  PAR::pass pass,
	  min::obj_vec_ptr & vp,
          min::phrase_position_vec ppvec )
{
    BRA::bracketed_pass bracketed_pass =
        (BRA::bracketed_pass) pass;

    min::uns32 size = min::size_of ( vp );

    // Scan keywords before names.
    //
    unsigned i = 1;
        // vp[i] is next lexeme or subexpression to
	// scan in the define/undefine expression.
    min::gen command = vp[i];
        // define, undefine, or print.
    definition_type type;
        // Type of command (see above).
    unsigned min_names, max_names;
        // Minimum and maximum number of names allowed.

    if ( command != PAR::define
         &&
	 command != PAR::undefine
         &&
	 command != PAR::print )
        return min::FAILURE();
    ++ i;

    if ( vp[i] == ::indentation
         &&
	 i + 1 < size
	 &&
	 vp[i+1] == ::offset )
    {
        if ( command == PAR::print )
	{
	    if ( i + 2 < size )
		return PAR::parse_error
		    ( parser, ppvec[i+1],
		      "unexpected stuff after" );

	    COM::print_command ( vp, parser );
	    parser->printer
		<< ":" << min::eol
		<< min::bom << min::no_auto_break
		<< min::set_indent ( 4 );

	    min::int32 offset =
	        bracketed_pass->indentation_offset; 
	    for ( min::uns32 i =
	              bracketed_pass->
		          indentation_offset_stack->
			      length;
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
				<< ": " << offset;

		if ( i == 0 ) break;

		offset =
		    bracketed_pass->
		        indentation_offset_stack[i-1];
	    }

	    parser->printer << min::eom;
	    return COM::PRINTED;
	}
	else if ( command != PAR::define )
	    return min::FAILURE();

	else if ( i + 2 >= size
	          ||
		  ! min::strto
	                ( bracketed_pass->
			      indentation_offset,
		          vp[i+2], 10 ) )
	    return PAR::parse_error
		( parser, ppvec[i+1],
		  "expected reasonable sized integer"
		  " after" );

	else if ( i + 3 < size )
	    return PAR::parse_error
		( parser, ppvec[i+2],
		  "unexpected stuff after" );

	return min::SUCCESS();
    }

    if ( command == PAR::print )
    {
        if ( vp[i] != ::bracket )
	    return min::FAILURE();
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
    else
        return min::FAILURE();

    // Scan mark names.
    //
    min::locatable_gen name[max_names+1];
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

	if ( number_of_names > max_names )
	    return PAR::parse_error
	        ( parser, ppvec->position,
		  "too many quoted names in" );

	if ( i >= size
	     ||
	     vp[i] != PAR::dotdotdot )
	    break;

	++ i;
    }

    if ( number_of_names < min_names )
	return PAR::parse_error
	    ( parser, ppvec->position,
	      "too few quoted names in" );

    if ( command == PAR::print )
    {
	if ( i < size )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "unexpected stuff after" );

	COM::print_command ( vp, parser );

	parser->printer
	    << ":" << min::eol
	    << min::bom << min::no_auto_break
	    << min::set_indent ( 4 );

	int count = 0;
	TAB::key_table_iterator it
	    ( bracketed_pass->bracket_table );
	while ( true )
	{
	    TAB::root root = it.next();
	    if ( root == min::NULL_STUB ) break;

	    if ( min::is_subsequence
		     ( name[0], root->label ) < 0 )
		continue;

	    min::uns32 subtype =
		min::packed_subtype_of ( root );

	    if ( subtype != BRA::OPENING_BRACKET
	         &&
		 subtype != BRA::INDENTATION_MARK )
	        continue;

	    ++ count;

	    min::gen block_name =
		PAR::block_name
		    ( parser,
		      root->block_level );
	    parser->printer
		<< min::indent
		<< "block "
		<< min::pgen_name ( block_name )
		<< ": " << min::save_indent;

	    if ( subtype == BRA::OPENING_BRACKET )
	    {
		BRA::opening_bracket opening_bracket =
		    (BRA::opening_bracket) root;
		BRA::closing_bracket closing_bracket =
		    opening_bracket->closing_bracket;

		parser->printer
		    << "bracket "
		    << min::pgen_quote
		           ( opening_bracket->label )
		    << " ... "
		    << min::pgen_quote
		           ( closing_bracket->label )
		    << " " << min::set_break;
		COM::print_flags
		    ( root->selectors,
		      parser->selector_name_table,
		      parser );

		TAB::new_flags new_selectors =
		    opening_bracket->new_selectors;

		if ( ! TAB::is_empty
			   ( new_selectors ) )
		{
		    parser->printer
		        << min::indent
			<< "with parsing"
			   " selectors ";
		    COM::print_new_flags
			( new_selectors,
			  parser->
			      selector_name_table,
			  parser );
		}

		if (   opening_bracket->options
		     & BRA::FULL_LINES )
		    parser->printer
			<< min::indent
			<< "with full lines";
	    }
	    else if ( subtype == BRA::INDENTATION_MARK )
	    {
		BRA::indentation_mark indentation_mark =
		    (BRA::indentation_mark) root;
		BRA::line_sep line_sep =
		    indentation_mark->line_sep;

		parser->printer
		    << "indentation mark "
		    << min::pgen_quote
		        ( indentation_mark->label );
		if ( line_sep != min::NULL_STUB )
		    parser->printer
		        << " ... "
			<< min::pgen_quote
			    ( line_sep->label );
		parser->printer
		    << " " << min::set_break;
		COM::print_flags
		    ( root->selectors,
		      parser->selector_name_table,
		      parser );

		TAB::new_flags new_selectors =
		    indentation_mark->new_selectors;

		if ( ! TAB::is_empty
			   ( new_selectors ) )
		{
		    parser->printer
			<< min::indent
			<< "with parsing"
			   " selectors ";
		    COM::print_new_flags
			( new_selectors,
			  parser->
			      selector_name_table,
			  parser );
		}
	    }
	    else
	    {
		MIN_ABORT
		    ( "bad parser print type" );
	    }

	    parser->printer << min::restore_indent
			    << min::eol;
	}

	if ( count == 0 )
	    parser->printer << min::indent
	                    << "not found";
	parser->printer << min::eom;

        return COM::PRINTED;
    }

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

    if ( command == PAR::define ) switch ( type )
    {
    case ::BRACKET:
    {
	min::uns32 options = 0;
	TAB::new_flags new_selectors;
	    // Inited to zeroes.
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
	                  parser->selector_name_table,
			  parser );
		if ( result == min::ERROR() )
		    return min::ERROR();
		else if ( result == min::FAILURE() )
		    return PAR::parse_error
			( parser, ppvec[i-1],
			  "expected bracketed selector"
			  " modifier list after" );
	    }
	    else if ( i + 1 < size
		      &&
		      vp[i] == ::full
		      &&
		      vp[i+1] == ::lines )
	    {
		i += 2;
		options = BRA::FULL_LINES;
	    }
	    else
		return PAR::parse_error
		    ( parser, ppvec[i-1],
		      "expected `parsing selectors'"
		      " or `full lines' after" );
	}
	if ( i < size )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "expected `with' after" );

	BRA::push_brackets
	    ( name[0], name[1],
	      selectors,
	      PAR::block_level ( parser ),
	      ppvec->position,
	      new_selectors,
	      min::NULL_STUB,
	      min::NULL_STUB,
	      options,
	      bracketed_pass->bracket_table );

	break;
    }
    case ::INDENTATION_MARK:
    {
	TAB::new_flags new_selectors;
	    // Inited to zeroes.
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
	                  parser->selector_name_table,
			  parser );
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
		      " or `full lines' after" );
	}
	if ( i < size )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "expected `with' after" );

	BRA::push_indentation_mark
	    ( name[0],
	      number_of_names == 2 ?
		  (min::gen) name[1] :
		  min::MISSING(),
	      selectors,
	      PAR::block_level ( parser ),
	      ppvec->position,
	      new_selectors,
	      bracketed_pass->bracket_table );

	break;
    }
    default:
	MIN_ABORT ( "bad parser define type" );
    }
    else /* if command == PAR::undefine */
    {
	if ( i < size )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "unexpected stuff after" );

	TAB::key_prefix key_prefix =
	    TAB::find_key_prefix
	        ( name[0],
		  bracketed_pass->bracket_table );

	min::uns32 count = 0;

	if ( key_prefix != min::NULL_STUB )
	for ( TAB::root root = key_prefix->first;
	      root != min::NULL_STUB;
	      root = root->next )
	{
	    if ( ( root->selectors & selectors ) == 0 )
		continue;

	    min::uns32 subtype =
		min::packed_subtype_of ( root );

	    switch ( type )
	    {
	    case ::BRACKET:
	    {
		if ( subtype != BRA::OPENING_BRACKET )
		    continue;

		BRA::opening_bracket opening_bracket =
		    (BRA::opening_bracket) root;
		BRA::closing_bracket closing_bracket =
		    opening_bracket->closing_bracket;

		if ( closing_bracket->label != name[1] )
		    continue;

		break;
	    }

	    case ::INDENTATION_MARK:
	    {
		if ( subtype != BRA::INDENTATION_MARK )
		    continue;

		BRA::indentation_mark indentation_mark =
		    (BRA::indentation_mark) root;
		BRA::line_sep line_sep =
		    indentation_mark->line_sep;
		if ( line_sep == min::NULL_STUB
		     &&
		     number_of_names == 2 )
		    continue;
		if ( line_sep != min::NULL_STUB
		     &&
		     number_of_names == 1 )
		    continue;
		if ( line_sep != min::NULL_STUB
		     &&
		     line_sep->label != name[1] )
		    continue;

		break;
	    }
	    default:
		MIN_ABORT
		    ( "bad parser undefine type" );
	    }

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
