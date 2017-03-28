// Layered Languages Bracketed Subexpression Parser
//
// File:	ll_parser_bracketed.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Mar 28 07:15:18 EDT 2017
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
//	Prefix Table
//	Bracketed Subexpression Pass
//	Bracketed Subexpression Parser Functions
//	Parse Bracketed Subexpression Function
//	Bracketed Reformatters
//	Bracketed Pass Command Function

// Usage and Setup
// ----- --- -----

# include <ll_lexeme_standard.h>
# include <ll_lexeme_program_data.h>
# include <ll_parser.h>
# include <ll_parser_command.h>
# include <ll_parser_bracketed.h>
# define MUP min::unprotected
# define LEX ll::lexeme
# define LEXSTD ll::lexeme::standard
# define LEXDATA ll::lexeme::program_data
# define PAR ll::parser
# define TAB ll::parser::table
# define COM ll::parser::command
# define BRA ll::parser::bracketed

const min::position BRA::PARAGRAPH_END =
    { 0xFFFFFFFF, 0xFFFFFFFE };

static bool initialize_called = false;
static min::locatable_gen bracket;
static min::locatable_gen indentation;
static min::locatable_gen typed;
static min::locatable_gen element;
static min::locatable_gen attributes;
static min::locatable_gen attribute;
static min::locatable_gen flags;
static min::locatable_gen multivalue;
static min::locatable_gen initiator;
static min::locatable_gen negator;
static min::locatable_gen separator;
static min::locatable_gen separators;
static min::locatable_gen allowed;
static min::locatable_gen mark;
static min::locatable_gen full;
static min::locatable_gen lines;
static min::locatable_gen bracketed_subexpressions;
static min::locatable_gen offset;
static min::locatable_gen string_lexeme;
static min::locatable_gen concatenator;
static min::locatable_gen top;

static void initialize ( void )
{
    if ( initialize_called ) return;
    initialize_called = true;

    ::bracket = min::new_str_gen ( "bracket" );
    ::indentation = min::new_str_gen
			    ( "indentation" );
    ::typed = min::new_str_gen ( "typed" );
    ::element = min::new_str_gen ( "element" );
    ::attributes = min::new_str_gen ( "attributes" );
    ::attribute = min::new_str_gen ( "attribute" );
    ::flags = min::new_str_gen ( "flags" );
    ::multivalue = min::new_str_gen ( "multivalue" );
    ::initiator = min::new_str_gen ( "initiator" );
    ::negator = min::new_str_gen ( "negator" );
    ::separator = min::new_str_gen ( "separator" );
    ::separators = min::new_str_gen ( "separators" );
    ::allowed = min::new_str_gen ( "allowed" );
    ::mark = min::new_str_gen ( "mark" );
    ::full = min::new_str_gen ( "full" );
    ::lines = min::new_str_gen ( "lines" );
    ::bracketed_subexpressions =
        min::new_lab_gen
	    ( "bracketed", "subexpressions" );
    ::offset = min::new_str_gen ( "offset" );
    ::string_lexeme = min::new_str_gen ( "string" );
    ::concatenator =
        min::new_str_gen ( "concatenator" );
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
    opening->new_selectors.or_flags &= ~
        ( PAR::TOP_LEVEL_SELECTOR + PAR::EALSEP_OPT );
    opening->new_selectors.not_flags |=
        ( PAR::TOP_LEVEL_SELECTOR + PAR::EALSEP_OPT );
    opening->new_selectors.xor_flags &= ~
        ( PAR::TOP_LEVEL_SELECTOR + PAR::EALSEP_OPT );
    reformatter_ref(opening) = reformatter;
    reformatter_arguments_ref(opening) =
        reformatter_arguments;

    TAB::push ( bracket_table, (TAB::root) opening );
    TAB::push ( bracket_table, (TAB::root) closing );

    return opening;
}

// Indentation Marks
// ----------- -----

static min::uns32 indentation_mark_gen_disp[] = {
    min::DISP ( & BRA::indentation_mark_struct::label ),
    min::DISP ( & BRA::indentation_mark_struct
                     ::implied_header ),
    min::DISP ( & BRA::indentation_mark_struct
                     ::implied_header_type ),
    min::DISP_END };
static min::uns32 indentation_mark_stub_disp[] = {
    min::DISP ( & BRA::indentation_mark_struct::next ),
    min::DISP ( & BRA::indentation_mark_struct
                     ::line_sep ),
    min::DISP_END };

static min::packed_struct_with_base
	<BRA::indentation_mark_struct, TAB::root_struct>
    indentation_mark_type
	( "ll::parser::table::indentation_mark_type",
	  ::indentation_mark_gen_disp,
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
	  min::gen implied_header,
	  min::uns32 lexical_master,
	  TAB::key_table bracket_table )
{
    min::locatable_var<BRA::indentation_mark> imark
        ( ::indentation_mark_type.new_stub() );
    label_ref(imark) = mark_label;
    imark->selectors = selectors;
    imark->block_level = block_level;
    imark->position = position;
    imark->new_selectors = new_selectors;
    imark->new_selectors.or_flags &= ~
        PAR::TOP_LEVEL_SELECTOR;
    imark->new_selectors.not_flags |=
        PAR::TOP_LEVEL_SELECTOR;
    imark->new_selectors.xor_flags &= ~
        PAR::TOP_LEVEL_SELECTOR;
    implied_header_ref(imark) = implied_header;
    implied_header_type_ref(imark) =
        min::get ( implied_header, min::dot_type );
    imark->lexical_master = lexical_master;
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
                     ::typed_middle ),
    min::DISP ( & BRA::typed_opening_struct
                     ::typed_double_middle ),
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

static min::uns32 typed_double_middle_stub_disp[] = {
    min::DISP ( & BRA::typed_double_middle_struct
                     ::next ),
    min::DISP ( & BRA::typed_double_middle_struct
                     ::typed_opening ),
    min::DISP_END };

static min::packed_struct_with_base
	<BRA::typed_double_middle_struct,
	 TAB::root_struct>
    typed_double_middle_type
	( "ll::parser::table::typed_double_middle_type",
	  TAB::root_gen_disp,
	  ::typed_double_middle_stub_disp );
const min::uns32 & BRA::TYPED_DOUBLE_MIDDLE =
    typed_double_middle_type.subtype;

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
	  min::gen typed_double_middle,
	  min::gen typed_closing,
	  TAB::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  const TAB::new_flags & element_selectors,
	  TAB::flags attr_selectors,
	  min::gen typed_attr_begin,
	  min::gen typed_attr_equal,
	  min::gen typed_attr_sep,
	  min::gen typed_attr_negator,
	  min::gen typed_attr_flags_initiator,
	  const min::flag_parser *
	           typed_attr_flag_parser,
	  min::gen typed_attr_multivalue_initiator,
	  bool prefix_separators_allowed,
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

    opening->selectors = selectors;
    closing->selectors = PAR::ALWAYS_SELECTOR;

    opening->block_level = block_level;
    closing->block_level = block_level;

    opening->position = position;
    closing->position = position;

    opening->new_selectors = element_selectors;
    opening->new_selectors.or_flags &= ~
        ( PAR::TOP_LEVEL_SELECTOR + PAR::EALSEP_OPT );
    opening->new_selectors.not_flags |=
        ( PAR::TOP_LEVEL_SELECTOR + PAR::EALSEP_OPT );
    opening->new_selectors.xor_flags &= ~
        ( PAR::TOP_LEVEL_SELECTOR + PAR::EALSEP_OPT );
    opening->attr_selectors = attr_selectors;

    reformatter_ref(opening) = min::NULL_STUB;
    reformatter_arguments_ref(opening) =
        min::NULL_STUB;

    TAB::push ( bracket_table, (TAB::root) opening );
    TAB::push ( bracket_table, (TAB::root) closing );

    min::locatable_var<BRA::typed_middle> middle
        ( ::typed_middle_type.new_stub() );
    label_ref(middle)  = typed_middle;

    typed_middle_ref(opening)  = middle;
    typed_opening_ref(middle)  = opening;

    middle->selectors  = PAR::ALWAYS_SELECTOR;
    middle->block_level  = block_level;
    middle->position  = position;

    TAB::push ( bracket_table, (TAB::root) middle );

    if ( typed_double_middle != min::MISSING() )
    {
	min::locatable_var<BRA::typed_double_middle>
		double_middle
	    ( ::typed_double_middle_type.new_stub() );
	label_ref(double_middle)
	    = typed_double_middle;

	typed_double_middle_ref(opening)
	    = double_middle;
	typed_opening_ref(double_middle)
	    = opening;

	double_middle->selectors = PAR::ALWAYS_SELECTOR;
	double_middle->block_level = block_level;
	double_middle->position = position;

	TAB::push ( bracket_table,
	            (TAB::root) double_middle );
    }

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

	attr_begin->selectors    = PAR::ALWAYS_SELECTOR;
	attr_equal->selectors    = PAR::ALWAYS_SELECTOR;
	attr_sep->selectors 	 = PAR::ALWAYS_SELECTOR;

	attr_begin->block_level  = block_level;
	attr_equal->block_level  = block_level;
	attr_sep->block_level 	 = block_level;

	attr_begin->position     = position;
	attr_equal->position     = position;
	attr_sep->position 	 = position;

	TAB::push ( bracket_table,
	           (TAB::root) attr_begin );
	TAB::push ( bracket_table,
	           (TAB::root) attr_equal );
	TAB::push ( bracket_table,
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

	attr_negator->selectors = PAR::ALWAYS_SELECTOR;

	attr_negator->block_level = block_level;

	attr_negator->position    = position;

	TAB::push ( bracket_table,
	           (TAB::root) attr_negator );
    }

    typed_attr_flags_initiator_ref(opening) =
        typed_attr_flags_initiator;
    opening->typed_attr_flag_parser =
    	typed_attr_flag_parser;
    typed_attr_multivalue_initiator_ref(opening) =
        typed_attr_multivalue_initiator;

    opening->prefix_separators_allowed =
        prefix_separators_allowed;

    return opening;
}

// Prefix Table
// ------ -----

static min::uns32 prefix_gen_disp[] = {
    min::DISP ( & BRA::prefix_struct::label ),
    min::DISP ( & BRA::prefix_struct::group ),
    min::DISP ( & BRA::prefix_struct
                     ::implied_subprefix ),
    min::DISP ( & BRA::prefix_struct
                     ::implied_subprefix_type ),
    min::DISP_END };

static min::uns32 prefix_stub_disp[] = {
    min::DISP ( & BRA::prefix_struct::next ),
    min::DISP_END };

static min::packed_struct_with_base
	<BRA::prefix_struct, TAB::root_struct>
    prefix_type ( "ll::parser::bracketed::prefix_type",
	        ::prefix_gen_disp,
	        ::prefix_stub_disp );
const min::uns32 & BRA::PREFIX = ::prefix_type.subtype;

void BRA::push_prefix
	( min::gen prefix_label,
	  TAB::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  TAB::new_flags new_selectors,
	  min::gen group,
	  min::gen implied_subprefix,
	  min::gen implied_subprefix_type,
	  min::uns32 lexical_master,
	  TAB::key_table prefix_table )
{
    min::locatable_var<BRA::prefix> prefix
        ( ::prefix_type.new_stub() );

    label_ref(prefix) = prefix_label;
    prefix->selectors = selectors;
    prefix->block_level = block_level;
    prefix->position = position;
    prefix->new_selectors = new_selectors;

    prefix->new_selectors.or_flags &= ~
        PAR::TOP_LEVEL_SELECTOR;
    prefix->new_selectors.not_flags |=
        PAR::TOP_LEVEL_SELECTOR;
    prefix->new_selectors.xor_flags &= ~
        PAR::TOP_LEVEL_SELECTOR;

    group_ref(prefix) = group;
    implied_subprefix_ref(prefix) = implied_subprefix;
    implied_subprefix_type_ref(prefix) =
        implied_subprefix_type;
    prefix->lexical_master = lexical_master;

    TAB::push ( prefix_table, (TAB::root) prefix );
}

// Bracketed Subexpression Pass
// --------- ------------- ----

static min::uns32 block_struct_gen_disp[] =
{
    min::DISP ( & BRA::block_struct
                     ::string_concatenator ),
    min::DISP_END
};

static min::packed_vec<BRA::block_struct>
    block_stack_type
        ( "ll::parser::bracketed::block_stack_type",
	  ::block_struct_gen_disp );

void BRA::init_block_stack
	( min::ref<BRA::block_stack> block_stack,
	  min::uns32 max_length )
{
    if ( block_stack == min::NULL_STUB )
        block_stack =
	     ::block_stack_type
	         .new_stub ( max_length );
}

static min::uns32 bracketed_pass_gen_disp[] =
{
    min::DISP ( & BRA::bracketed_pass_struct
                     ::name ),
    min::DISP_END
};

static min::uns32 bracketed_pass_stub_disp[] =
{
    min::DISP ( & BRA::bracketed_pass_struct
                     ::parser ),
    min::DISP ( & BRA::bracketed_pass_struct
                     ::next ),
    min::DISP ( & BRA::bracketed_pass_struct
                     ::bracket_table ),
    min::DISP ( & BRA::bracketed_pass_struct
                     ::prefix_table ),
    min::DISP_END
};

static min::packed_struct_with_base
	<BRA::bracketed_pass_struct,
	 PAR::pass_struct>
    bracketed_pass_type
        ( "ll::parser::bracketed::bracketed_pass_type",
	  ::bracketed_pass_gen_disp,
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
    TAB::key_table prefix_table =
        bracketed_pass->prefix_table;

    min::uns64 collected_entries,
               collected_key_prefixes;

    TAB::end_block
        ( bracket_table, 0,
	  collected_key_prefixes, collected_entries );
    TAB::end_block
        ( prefix_table, 0,
	  collected_key_prefixes, collected_entries );
    min::pop ( bracketed_pass->block_stack,
	       bracketed_pass->block_stack->length );
    bracketed_pass->indentation_offset = 2;
    BRA::string_concatenator_ref ( bracketed_pass ) =
        PAR::number_sign;
}

static min::gen bracketed_pass_begin_block
	( PAR::parser parser,
	  PAR::pass pass,
	  const min::phrase_position & position,
	  min::gen name )
{
    BRA::bracketed_pass bracketed_pass =
        (BRA::bracketed_pass) pass;

    BRA::push_block
	( bracketed_pass->block_stack,
	  bracketed_pass->indentation_offset,
	  bracketed_pass->string_concatenator );

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
    TAB::key_table prefix_table =
        bracketed_pass->prefix_table;

    min::uns64 collected_entries,
               collected_key_prefixes;

    min::uns32 block_level =
        PAR::block_level ( parser );
    MIN_REQUIRE ( block_level > 0 );

    TAB::end_block
        ( bracket_table, block_level - 1,
	  collected_key_prefixes, collected_entries );
    TAB::end_block
        ( prefix_table, block_level - 1,
	  collected_key_prefixes, collected_entries );

    BRA::block_struct b =
        min::pop ( bracketed_pass->block_stack );
    bracketed_pass->indentation_offset =
        b.indentation_offset;
    string_concatenator_ref ( bracketed_pass ) =
        b.string_concatenator;

    return min::SUCCESS();
}

static min::gen bracketed_pass_command
	( PAR::parser parser,
	  PAR::pass pass,
	  min::obj_vec_ptr & vp, min::uns32 i0,
          min::phrase_position_vec ppvec );

PAR::pass BRA::new_pass ( PAR::parser parser )
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
    bracket_table_ref(bracketed_pass) =
	TAB::create_key_table ( 1024 );
    prefix_table_ref(bracketed_pass) =
	TAB::create_key_table ( 1024 );
    BRA::init_block_stack
        ( BRA::block_stack_ref(bracketed_pass), 16 );
    bracketed_pass->indentation_offset = 2;
    BRA::string_concatenator_ref ( bracketed_pass ) =
        PAR::number_sign;

    return (PAR::pass) bracketed_pass;
}

// Bracketed Subexpression Parser Functions
// --------- ------------- ------ ---------

// These are all static or inline; if any are useful
// elsewhere they can be put in the PAR namespace.

// Given the token sequence from `first' to `next', make
// a label containing these tokens.  Convert quoted
// string and numeric tokens to strings.  Announce
// non-name components as errors and ignore them.  If
// allow_sublabels is false, also treat components
// that are themselves labels as errors.
//
// If there are 0 non-erroneous label components, make
// an empty label, and if there is exactly 1 component,
// use the value of that component as the label, rather
// than making a label.
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
// at the new token, and the new token position is set
// to the empty string just before `next'.  This is the
// only case where `first' is updated.
//
static void make_label
    ( PAR::parser parser,
      PAR::token & first,
      PAR::token next,
      bool allow_sublabels = false )
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

	switch ( t->type )
	{
	case LEXSTD::mark_t:
	case LEXSTD::separator_t:
	    PAR::parse_warn
	        ( parser, t->position,
		  "name element ",
		  min::pgen_quote ( t->value ),
		  " is a mark or separator and"
		  " therefore should be quoted" );

	case LEXSTD::word_t:
	case LEXSTD::natural_t:
	    break;

	case LEXSTD::quoted_string_t:
	case LEXSTD::numeric_t:
	    t->type = PAR::DERIVED;
	    PAR::value_ref(t) = min::new_str_gen
			( min::begin_ptr_of
			      ( t->string ),
			  t->string->length );
	    PAR::string_ref(t) =
		PAR::free_string ( t->string );
	    break;

	case PAR::DERIVED:
	    if ( min::is_lab ( t->value ) 
	         &&
		 allow_sublabels )
	        break;

	default:
	    PAR::parse_error
	        ( parser, t->position,
		  "not a legal label element `",
		  min::pgen_never_quote ( t->value ),
		  "'; ignored" );

	    t->type = PAR::DERIVED;
	    value_ref(t) = min::NONE();
	}
    }

    if ( count == 0 )
    {
	first = PAR::new_token ( PAR::DERIVED );
	first->position = position;
	put_before ( PAR::first_ref(parser),
	             next, first );
	PAR::value_ref(first) = min::empty_lab;
	return;
    }

    min::gen vec[count];
    min::unsptr i = 0;
    for ( PAR::token t = first; t != next; t = t->next )
    {
	if ( t->value != min::NONE() )
	    vec[i++] = t->value;
    }
    if ( i == 1 )
	PAR::value_ref(first) = vec[0];
    else
	PAR::value_ref(first) =
	    min::new_lab_gen ( vec, i );

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

    first->type = PAR::DERIVED;
    first->position = position;
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
    MIN_REQUIRE ( token->type == LEXSTD::indent_t );

    int relative_indent =
        (min::int32) token->indent - indent;
    if (    relative_indent != 0
	 && relative_indent < indentation_offset 
	 && relative_indent > - indentation_offset )
    {
	char buffer[200];
	sprintf ( buffer, "lexeme indent %d too near"
			  " paragraph indent %d",
			  token->indent, indent );
	min::phrase_position position = token->position;
	position.begin = position.end;
	PAR::parse_warn ( parser, position, buffer );
    }
    return relative_indent;
}

// Parse Bracketed Subexpression Function
// ----- --------- ------------- --------

// See ll_parser_bracketed_parse.outline for an outline
// of the parse_bracketed_subexpression function.

// Ensure there is a next token.
//
inline void ensure_next
	( PAR::parser parser, PAR::token current )
{
    if ( current->next == parser->first
         &&
	 current->type != LEXSTD::end_of_file_t )
    {
	parser->input->add_tokens
	    ( parser, parser->input );
	MIN_REQUIRE
	    ( current->next != parser->first );
    }
}

// Announce `missing <message>' error with position
// just before current.
//
static void missing_error
        ( PAR::parser parser,
	  PAR::token current,
	  const char * message1,
	  const min::op & message2 = min::pnop,
	  const char * message3 = "" )
{
    min::phrase_position position =
        { current->position.begin,
	  current->position.begin };

    PAR::parse_error
        ( parser, position,
	  "missing ", min::pnop, message1,
	  message2, message3 );
}

static void punctuation_error
	( PAR::parser parser,
	  PAR::token key_first,
	  PAR::token next,
	  min::gen label )
{
	MIN_REQUIRE ( key_first != next );

	min::phrase_position position =
	    { key_first->position.begin,
	      next->previous->position.end };

	PAR::parse_error
	    ( parser, position,
	      "unexpected punctuation ",
	      min::pgen_quote ( label ),
	      " found; deleted and ignored" );

	PAR::remove
	    ( parser, next, label );
}

// Make type label.  If no label tokens, just print
// missing type error message.  Otherwise if typed_data
// ->type is MISSING, set new label token type to
// TYPE, increment typed_data->attr_count, and move
// token to before typed_data->elements if that is not
// NULL_STUB. 
//
// If typed_data->type is NOT MISSING, check that it
// equals new label, and print error message if not.
// Then in any case delete the new label token.
//
inline void make_type_label
	( PAR::parser parser,
	  BRA::typed_data * typed_data,
	  PAR::token next )
{
    PAR::token start = typed_data->start_previous->next;
    if ( start == next )
    {
	::missing_error ( parser, next, "type" );
	return;
    }

    ::make_label ( parser, start, next );

    if ( typed_data->type == min::MISSING() )
    {
	start->type = BRA::TYPE;
	typed_data->type = start->value;
	++ typed_data->attr_count;

	if ( typed_data->elements != min::NULL_STUB )
	{
	    typed_data->end_position =
		next->previous->position.end;
	    PAR::move_to_before
		( parser, typed_data->elements,
		  next->previous, next );
	}
    }
    else
    {
	if ( typed_data->type != start->value )
	    PAR::parse_error
		( parser, start->position,
		  "beginning type `",
		  min::pgen_never_quote
		    ( typed_data->type ),
		  "' != end type `",
		  min::pgen_never_quote
		    ( start->value ),
		  "'; end type ignored"
		);

	PAR::free
	    ( PAR::remove ( first_ref(parser),
			    start ) );
    }
}

// Make attribute label.
//
// Label may be followed by flags which are in bracketed
// subexpression token just before `next'.  This token
// is marked ATTR_FLAGS.  Attribute label token is
// marked ATT_LABEL.
//
// Set typed_data->attributes if that is NULL_STUB and
// increment typed_data->attr_count.
//
inline void make_attribute_label
	( PAR::parser parser,
	  BRA::typed_data * typed_data,
	  PAR::token next )
{
    PAR::token start = typed_data->start_previous->next;

    min::gen initiator =
        typed_data->typed_opening
	          ->typed_attr_flags_initiator;

    if ( next != start
         &&
	 next->previous != start
         &&
	 next->previous->value_type == initiator
	 &&
	 initiator != min::MISSING() )
    {
	next = next->previous;
	next->type = BRA::ATTR_FLAGS;
    }

    ::make_label ( parser, start, next );
    start->type = BRA::ATTR_LABEL;

    if ( typed_data->attributes == min::NULL_STUB )
	typed_data->attributes = start;

    ++ typed_data->attr_count;
}

// Finish attribute.  On error either generate an attri-
// bute with value NONE which will be deleted later.
//
// If typed_data indicates there are no attributes
// (e.g., we are immediately after a typed middle)
// do nothing.
//
inline void finish_attribute
	( PAR::parser parser,
	  BRA::typed_data * typed_data,
	  PAR::token next )
{
    min::uns32 subtype = typed_data->subtype;
    PAR::token start = typed_data->start_previous->next;

    if ( subtype == BRA::TYPED_OPENING
         ||
	 subtype == BRA::TYPED_ATTR_SEP
         ||
	 subtype == BRA::TYPED_ATTR_NEGATOR
	 ||
	 ( subtype == BRA::TYPED_MIDDLE
	   &&
	   start != next ) )
    {
        ::make_attribute_label
	           ( parser, typed_data, next );

	typed_data->start_previous->next->type =
	    ( subtype == BRA::TYPED_ATTR_NEGATOR ?
	      BRA::ATTR_FALSE :
	      BRA::ATTR_TRUE );
    }
    else if ( subtype == BRA::TYPED_ATTR_EQUAL )
    {
	if ( start == next )
	{
	    missing_error ( parser, next,
	                    "attribute value;"
			    " attribute ignored" );
	    start = PAR::new_token ( BRA::ATTR_VALUE );
	    put_before ( PAR::first_ref(parser),
			 next, start );
	    // Position needed to determine end of
	    // typed bracketed expression.
	    min::phrase_position pos =
	        { typed_data->end_position,
	          typed_data->end_position };
		// Position recorded by ATTR_EQUAL.
	    start->position = pos;
	    PAR::value_ref(start) = min::NONE();
	}
	else if ( start->next == next
	          &&
		  start->string == min::NULL_STUB )
	{
	    min::gen initiator =
		typed_data
		    ->typed_opening
		    ->typed_attr_multivalue_initiator;
	    start->type =
	        ( start->value_type == initiator
		  &&
		  initiator != min::MISSING() ?
		  BRA::ATTR_MULTIVALUE :
		  BRA::ATTR_VALUE );

	    if ( start->type == BRA::ATTR_VALUE
	         &&
		 ! min::is_attr_legal ( start->value ) )
	    {
		PAR::parse_error
		  ( parser,
		    start->position,
		    "not a legal attribute value `",
		    min::pgen_never_quote
		        ( start->value ),
		    "'; attribute ignored" );
		PAR::value_ref(start) = min::NONE();
	    }
	}
	else
	{
	    ::make_label
	        ( parser, start, next );
	    start->type = BRA::ATTR_VALUE;
	}
    }
    else MIN_ASSERT ( subtype == BRA::TYPED_MIDDLE,
                      "unrecognized subtype in"
		      " finish_attribute" );
}

// If end of non-first attibute group, move it to
// end of previous attibutes.
//
inline void move_attributes
	( PAR::parser parser,
	  BRA::typed_data * typed_data,
	  PAR::token next )
{
    if (    typed_data->middle_count
	 != 0
	 &&
	    typed_data->elements
	 != min::NULL_STUB
	 &&
	    typed_data->attributes
	 != min::NULL_STUB )
    {
	typed_data->end_position =
	    next->previous->position.end;
	PAR::move_to_before
	    ( parser,
	      typed_data->elements,
	      typed_data->attributes,
	      next );
    }

    typed_data->attributes = min::NULL_STUB;
}

min::position BRA::parse_bracketed_subexpression
	( PAR::parser parser,
	  TAB::flags selectors,
	  PAR::token & current,
	  min::int32 indent,
	  BRA::typed_data * typed_data,
	  BRA::line_variables * line_variables,
	  BRA::bracket_stack * bracket_stack_p )
{

#   define PARSE_BRA_SUBEXP \
	   BRA::parse_bracketed_subexpression
	   // To avoid a too long line

    // Note: if typed_data != NULL, selectors are those
    // for scanning type and attributes, and selectors
    // for scanning elements are in typed_data->saved_
    // selectors.

    BRA::bracketed_pass bracketed_pass =
        (BRA::bracketed_pass) parser->pass_stack;
	// First pass in pass_stack is always the
	// bracketed pass.

    BRA::indentation_mark indentation_found =
        min::NULL_STUB;
	// If not NULL_STUB, current token is an end-of-
	// line and current->previous token is the last
	// token of an indentation mark.

    PAR::token start_previous = current->previous;
        // If current == start_previous->next, we are
	// at start of subexpression where prefix
	// separator is legal, etc.

    bool parsing_logical_line =
        ( line_variables->previous == start_previous );
	// True iff we are parsing a logical line and
	// NOT a bracketed (e.g.with `( )') subexpres-
	// sion inside a logical line.  (bracketed_
	// stack_p may still be non-NULL if the logical
	// line is itself inside a bracketed subexpres-
	// sion.)

    TAB::flags trace_flags = parser->trace_flags;
    if (   trace_flags
         & bracketed_pass->trace_subexpressions )
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

	// Variables input to both PREFIX_FOUND and
	// PREFIX_PARSE code blocks.
	//
	PAR::token prefix;

	// Variables input to PREFIX_FOUND code block.
	//
	min::position separator_found;
	bool premature_closing;

	// Variables input to the PREFIX_PARSE code
	// block.
	//
	min::gen prefix_type;
	BRA::prefix prefix_entry;
	min::gen prefix_group;
	TAB::flags prefix_selectors;

	if ( parsing_logical_line )
	{
	    BRA::line_data & line_data =
	        line_variables->current;
	    min::gen implied_header =
	        line_data.implied_header;
	    if ( implied_header == min::MISSING() )
	        goto NEXT_TOKEN;

	    prefix_entry =
	        line_data.header_entry;
	    prefix_selectors =
	        line_data.header_selectors;
	    MIN_REQUIRE
	        ( prefix_entry != min::NULL_STUB );
	    prefix_group = prefix_entry->group;

	    prefix = PAR::new_token
	                 ( PAR::IMPLIED_PREFIX );
	    PAR::put_before
	        ( PAR::first_ref(parser),
		  current, prefix );
	    PAR::value_ref(prefix) = implied_header;
	    PAR::value_type_ref(prefix) =
	        prefix_entry->label;
	    prefix->position.begin =
	        current->position.begin;
	    prefix->position.end =
	        current->position.begin;

	    if ( prefix_group == PAR::paragraph_lexeme )
		line_variables->current =
		    line_variables->implied_paragraph;

	    goto PREFIX_PARSE;
	}
	else
	{
	    if ( bracket_stack_p == NULL )
	        goto NEXT_TOKEN;
	    BRA::prefix p =
	        bracket_stack_p->prefix_entry;
	    if ( p == min::NULL_STUB )
	        goto NEXT_TOKEN;
	    min::gen implied_subprefix =
	        p->implied_subprefix;
	    if ( implied_subprefix == min::MISSING() )
	        goto NEXT_TOKEN;

	    prefix = PAR::new_token
	                 ( PAR::IMPLIED_PREFIX );
	    PAR::put_before
	        ( PAR::first_ref(parser),
		  current, prefix );
	    PAR::value_ref(prefix) = implied_subprefix;
	    PAR::value_type_ref(prefix) =
	        p->implied_subprefix_type;
	    prefix->position.begin =
	        current->position.begin;
	    prefix->position.end =
	        current->position.begin;

	    separator_found = min::MISSING_POSITION;
	    premature_closing = false;
	}

	// Code above either jumps to NEXT_TOKEN or
	// creates implied prefix and jumps to
	// PREFIX_PARSE for implied prefixes created
	// by line variables or comes here for implied
	// prefixes NOT create by line variables.

PREFIX_FOUND:

	// Come here when PREFIX token has been created.
	// (This is like a nested inline function, but
	// C++ does not support NESTED inlines.)
	//
	// Input variables:
	//
	//     prefix
	//         The prefix token.
	//     separator_found
	//         Specifies separator found right after
	//         prefix token, if any.
	//     premature_closing
	//         True if premature closing forced the
	//         end of the prefix token.
	//
	{
	    TAB::key_table prefix_table =
		bracketed_pass->prefix_table;
	    prefix_type = prefix->value_type;
	    prefix_entry =
		(BRA::prefix)
		TAB::find ( prefix_type, selectors,
			    prefix_table );
	    prefix_group =
		( prefix_entry != min::NULL_STUB ?
		  prefix_entry->group :
		  min::MISSING() );

	    if ( ( prefix_group != PAR::paragraph_lexeme
	           &&
		   prefix_group != PAR::line_lexeme )
		 ||
		     prefix->previous->type
		 == PAR::IMPLIED_PREFIX )
	    for ( BRA::bracket_stack * p =
	                 bracket_stack_p;

		  p != NULL
		  &&
		  p->prefix_type != min::MISSING()
		  &&
		  p->line_variables == line_variables;

		  p = p->previous )
	    {
		if ( p->prefix_type == prefix_type
		     ||
		     ( p->prefix_entry != min::NULL_STUB
		       &&
		          p->prefix_entry->group
		       == prefix_group
		       &&
		          min::MISSING()
		       != prefix_group ) )
		{
		    if (    prefix->type
			 == PAR::IMPLIED_PREFIX )
		    {
			PAR::parse_error
			  ( parser,
			    prefix->position,
			    "implied prefix separator"
			    " of type `",
			    min::pgen_never_quote
				( prefix_type ),
			    "' has type or group of"
			    " previous active prefix"
			    " separator; not inserted"
			  );
			MIN_REQUIRE
			    ( ! premature_closing );
			PAR::token next = prefix->next;
			PAR::free
			    ( PAR::remove
				  ( first_ref(parser),
				    next->previous ) );
			goto NEXT_TOKEN;
		    }
		    p->closing_first = prefix;
		    p->closing_next = prefix->next;

		    for ( BRA::bracket_stack
			      * q = bracket_stack_p;
			  q != p;
			  q = q->previous )
			q->closing_first =
			    q->closing_next = prefix;

		    return separator_found;
		}
	    }

	    // Special case to handle explicit paragraph
	    // prefix following an implied line prefix,
	    // provided the implied line prefix does not
	    // follow any paragraph prefix.  To enforce
	    // this last, this code must follow the code
	    // immediately above.
	    //
	    if ( prefix_group == PAR::paragraph_lexeme
	         &&
		 line_variables->at_paragraph_beginning
	         &&
		 prefix->type != PAR::IMPLIED_PREFIX
		 &&
		    prefix->previous->type
		 == PAR::IMPLIED_PREFIX
		 &&
		 bracket_stack_p != NULL
		 &&
		    bracket_stack_p->prefix_entry
		 != min::NULL_STUB
		 &&
		    bracket_stack_p->prefix_entry
		                   ->group
		 == PAR::line_lexeme )
	    {
		bracket_stack_p->closing_first =
		    prefix;
		bracket_stack_p->closing_next =
		    prefix->next;
		return separator_found;
	    }

	    PAR::token prefix_next = prefix->next;
	    if ( prefix_group == PAR::paragraph_lexeme )
	    {
	        if (    prefix->type
		     == PAR::IMPLIED_PREFIX )
	        {
		    MIN_REQUIRE
		        ( bracket_stack_p != NULL );
		    PAR::parse_error
		      ( parser,
			prefix->position,
			"implied prefix separator of"
			" type `",
			min::pgen_never_quote
			    ( prefix_type ),
			"' IMPLIED by prefix separator"
			" of type `",
			min::pgen_never_quote
			  ( bracket_stack_p->
			        prefix_type ),
			"' has `paragraph' group;"
			" ignored"
		      );
		    prefix = min::NULL_STUB;
		}
		else
		if (    line_variables->previous->next
		     != prefix
		     ||
		     ! line_variables->
		           at_paragraph_beginning )
	        {
		    PAR::parse_error
		      ( parser,
			prefix->position,
			"explicit prefix separator of"
			" type `",
			min::pgen_never_quote
			    ( prefix_type ),
			"' has `paragraph' group but is"
			" not at beginning of a logical"
			" line that is in paragraph"
			" beginning position; ignored"
		      );
		    prefix = min::NULL_STUB;
		}

	    }
	    else
	    if ( prefix_group == PAR::line_lexeme
	         &&
	            line_variables->previous->next
		 != prefix
		 &&
		 ( bracket_stack_p == NULL
		   ||
		      bracket_stack_p->prefix_entry
		   == min::NULL_STUB
		   ||
		      bracket_stack_p->prefix_entry
		                     ->group
		   != PAR::paragraph_lexeme ) )
	    {
		PAR::parse_error
		  ( parser,
		    prefix->position,
		       prefix->type
		    == PAR::IMPLIED_PREFIX ?
		        "implied " : "explicit ",
		    min::pnop,
		    "prefix separator of type `",
		    min::pgen_never_quote
			( prefix_type ),
		    "' has `line' group but is not at"
		    " beginning of logical line or"
		    " immediately after a prefix"
		    " separator of `paragraph' group;"
		    " ignored"
		  );
		prefix = min::NULL_STUB;
	    }
	    else
	    if ( start_previous->next != prefix )
	    {
		PAR::parse_error
		  ( parser,
		    prefix->position,
		    "prefix separator of type `",
		    min::pgen_never_quote
		        ( prefix_type ),
		    "' not at beginning of"
		    " subexpression; ignored"
		  );
		prefix = min::NULL_STUB;
	    }

	    if ( prefix == min::NULL_STUB )
		PAR::free
		    ( PAR::remove
			( first_ref(parser),
			  prefix_next->previous ) );

	    if ( premature_closing )
	    {
		// Found a closing bracket
		// that is not ours or
		// logical line end.
		// Kick to caller.
		//
		if ( prefix != min::NULL_STUB )
		    prefix->type = PAR::BRACKETED;
		return separator_found;
	    }

	    if ( prefix == min::NULL_STUB )
	        goto NEXT_TOKEN;
	}

	prefix_selectors = selectors;
	if ( prefix_entry != min::NULL_STUB )
	{
	    prefix_selectors |=
	      prefix_entry->
		  new_selectors.or_flags;
	    prefix_selectors &= ~
	      prefix_entry->
		  new_selectors.not_flags;
	    prefix_selectors ^=
	      prefix_entry->
		  new_selectors.xor_flags;
	    prefix_selectors |=
	      PAR::ALWAYS_SELECTOR;
	}

PREFIX_PARSE:

	// Come here when ready to parse prefix-n-list
	// that is headed by the prefix.
	//
	// Input variables:
	//
	//     prefix
	//         The prefix token.
	//     prefix_type
	//         The .type of the prefix token.
	//     prefix_entry
	//         The prefix_table entry of the prefix
	//         token .type (NULL_STUB if none).
	//     prefix_group
	//         The group of the prefix_entry, or
	//         MISSING if prefix_entry is NULL_STUB.
	//     prefix_selectors
	//         The selectors to be used in parsing
	//         the prefix-n-list headed by the
	//         prefix token.
	//
	{
	    MIN_REQUIRE ( prefix->next == current );

	    // Start new subexpression
	    // that begins with a prefix
	    // separator.
	    //
	    BRA::bracket_stack cstack
		( bracket_stack_p );
	    cstack.prefix_type = prefix_type;
	    cstack.prefix_entry = prefix_entry;
	    cstack.line_variables = line_variables;

	    while ( true )
	    {
	        if (    prefix_group
		     == PAR::paragraph_lexeme
		     &&
		        prefix->type
		     != PAR::IMPLIED_PREFIX )
		{
		    // Come here if explicit paragraph
		    // header found in a logical line,
		    // but with possible implied line
		    // headers having been previously
		    // inserted and deleted, possible
		    // incorrect setting of selectors
		    // to something other than
		    // paragraph.selectors because of
		    // a CONTINUING flag, and possible
		    // incorrect prefix_entry because
		    // of incorrect selectors.
		    //
		    // Fix things up.

		    MIN_REQUIRE
		        ( parsing_logical_line );

		    selectors =
			line_variables->paragraph
			               .selectors;
		    TAB::key_table prefix_table =
			bracketed_pass->prefix_table;
		    BRA::prefix new_prefix_entry =
			(BRA::prefix)
			TAB::find
			    ( prefix_type,
			      selectors,
			      prefix_table );
		    if (    new_prefix_entry
			 == min::NULL_STUB
			 ||
			    new_prefix_entry->group
			 != PAR::paragraph_lexeme )
			PAR::parse_error
			  ( parser,
			    prefix->position,
			    "prefix separator of"
			    " type `",
			    min::pgen_never_quote
				( prefix_type ),
			    "' has `paragraph' group"
			    " after selectors modified"
			    " by implied headers, but"
			    " does not have `paragraph'"
			    " group before this"
			    " modification; continuing"
			    " with prefix definition"
			    " located by INCORRECT"
			    " modified selectors"
			  );
		    else if (    new_prefix_entry
			      != prefix_entry )
		    {
			prefix_entry =
			    new_prefix_entry;
			cstack.prefix_entry =
			    prefix_entry;
		    }
		    prefix_selectors = selectors;
		    prefix_selectors |=
		        prefix_entry->
			    new_selectors.or_flags;
		    prefix_selectors &= ~
		        prefix_entry->
			    new_selectors.not_flags;
		    prefix_selectors ^=
		        prefix_entry->
			    new_selectors.xor_flags;
		    prefix_selectors |=
		        PAR::ALWAYS_SELECTOR;

		    BRA::line_data & line_data =
		        line_variables->current;
		    line_data.lexical_master =
		        prefix_entry->lexical_master;
		    line_data.selectors =
		        prefix_selectors;
		    line_data.implied_header =
		        min::MISSING();
		    line_data.header_entry =
		        min::NULL_STUB;
			// Just for safety.

		    min::gen implied_header =
		        prefix_entry->implied_subprefix;
		    if (     implied_header
		          != min::MISSING() )
		    {
			min::gen header_type =
			    prefix_entry->
				implied_subprefix_type;
			TAB::key_table prefix_table =
			    bracketed_pass->
			        prefix_table;
			BRA::prefix header_entry =
			    (BRA::prefix)
			    TAB::find
				( header_type,
				  prefix_selectors,
				  prefix_table );
			if (    header_entry
			     != min::NULL_STUB
			     &&
			        header_entry->group
			     == PAR::line_lexeme )
			{
			    TAB::flags
			            header_selectors =
				prefix_selectors;
			    header_selectors |=
				header_entry->
				    new_selectors
				        .or_flags;
			    header_selectors &= ~
				header_entry->
				    new_selectors
				        .not_flags;
			    header_selectors ^=
				header_entry->
				    new_selectors
				        .xor_flags;
			    header_selectors |=
				PAR::ALWAYS_SELECTOR;
			    line_data.implied_header =
				implied_header;
			    line_data.header_entry =
				header_entry;
			    line_data.header_selectors =
				header_selectors;
			}
		    }

		    if (   line_variables->current
		                          .selectors
		         & PAR::STICKY_OPT )
		    {
			line_variables->
			        implied_paragraph =
			    line_variables->current;

			line_variables->
			    paragraph.implied_header =
			  prefix->value;
			line_variables->
			    paragraph.header_entry =
			  prefix_entry;
			line_variables->
			    paragraph.header_selectors =
			  prefix_selectors;

			// The following causes compact_
			// prefix_separator to make a
			// copy of prefix->value, so
			// paragraph.implied_header
			// remains unchanged.
			//
			// Changing prefix->type to
			// IMPLIED_PREFIX does no harm
			// as at this point prefix is
			// the first thing in the
			// logical line and we can
			// pretend it was implied.
			//
			MIN_REQUIRE
			    (    start_previous->next
			      == prefix );
			prefix->type =
			    PAR::IMPLIED_PREFIX;
		    }
		    else
		    {
			line_variables->paragraph =
			    line_variables->
			        indentation_paragraph;
			line_variables->
				implied_paragraph =
			    line_variables->
			  indentation_implied_paragraph;
		    }
		}

		cstack.closing_first =
		    min::NULL_STUB;
		cstack.closing_next =
		    min::NULL_STUB;
		min::position separator_found =
		    PARSE_BRA_SUBEXP
		      ( parser, prefix_selectors,
			current, indent,
			NULL,
			line_variables, & cstack );

		PAR::token next =
		    cstack.closing_first;
		if ( next == min::NULL_STUB )
		    next = current;

			(TAB::root) min::NULL_STUB;

		if (    prefix_group
		     == PAR::paragraph_lexeme
		     &&
		     ( ( prefix_selectors
		         &
			 (   PAR::EALBREAK_OPT
			   + PAR::EALEINDENT_OPT
			   + PAR::EALTINDENT_OPT
			   + PAR::EALSEP_OPT ) )
		       != 0 )
		     &&
		     prefix->next != next
		     &&
		     ( prefix->next->next != next
		       ||
		          prefix->next->value_type
		       != PAR::line_lexeme ) )
		{
		    PAR::token first = prefix->next;
		    PAR::compact_logical_line
		        ( parser, bracketed_pass->next,
		          prefix_selectors,
		          first, next,
			  separator_found,
			  (TAB::root)
			  line_variables->line_sep,
			  trace_flags );
		}

		if ( compact_prefix_separator
		         ( parser, bracketed_pass->next,
		           prefix_selectors,
		           prefix, next,
			      prefix_group
			   == PAR::line_lexeme ?
			       separator_found :
			       min::MISSING_POSITION,
			   (TAB::root)
			   line_variables->line_sep,
			   trace_flags ) )
		{
		    if (    prefix_group
		         == PAR::line_lexeme
			 ||
		            prefix_group
		         == PAR::paragraph_lexeme )
			PAR::value_type_ref ( prefix ) =
			    prefix_group;
		    else
			PAR::value_type_ref ( prefix ) =
			    min::MISSING();
		}

		if (    cstack.closing_first
		     == cstack.closing_next )
		    return separator_found;

		prefix = cstack.closing_first;

		if (    prefix->value_type
		     != prefix_type )
		{
		    TAB::key_table prefix_table =
			bracketed_pass->prefix_table;
		    prefix_entry = (BRA::prefix)
			TAB::find
			    ( prefix->value_type,
			      selectors,
			      prefix_table );
		    MIN_REQUIRE
			(    prefix_entry
			  != min::NULL_STUB );
		    if (    prefix_entry->group
			 != prefix_group )
		    {
		        // Special case where line
			// IMPLIED_PREFIX is immediately
			// followed by paragraph
			// explicit prefix.
			//
		        MIN_REQUIRE
			    (    prefix_group
			      == PAR::line_lexeme );
		        MIN_REQUIRE
			    (    prefix_entry->group
			      == PAR::paragraph_lexeme
			    );
		        MIN_REQUIRE
			    (    line_variables->
			             previous
			      == start_previous );
			prefix_group =
			    prefix_entry->group;
			selectors =
			    line_variables->paragraph
			                   .selectors;
		    }
		    cstack.prefix_type =
		    prefix_type =
			prefix->value_type;
		    cstack.prefix_entry =
			prefix_entry;
		    prefix_selectors = selectors;
		    prefix_selectors |=
			prefix_entry->
			    new_selectors.or_flags;
		    prefix_selectors &= ~
			prefix_entry->
			    new_selectors.not_flags;
		    prefix_selectors ^=
			prefix_entry->
			    new_selectors.xor_flags;
		    prefix_selectors |=
			PAR::ALWAYS_SELECTOR;
		}
	    }
	}

NEXT_TOKEN:

        // Skip comments, line breaks, and indent before
	// comments, so that either nothing is skipped
	// OR current is an indent or end-of-file.
	//
	// Comments skipped that are indented less than
	// the indent of any following indent token are
	// errors.  These errors are ignored but cause
	// warning messages.
	//
        PAR::token first = current;
	min::uns32 t = current->type;
	while ( t == LEXSTD::line_break_t
	        ||
		t == LEXSTD::comment_t
		||
	           t
		== LEXSTD::indent_before_comment_t )
	{
	    min::uns32 previous_t = t;

	    ensure_next ( parser, current );
	    current = current->next;
	    t = current->type;

	    if ( previous_t == LEXSTD::line_break_t
	         &&
		 t == LEXSTD::line_break_t )
		parser->at_paragraph_beginning = true;
	    else if ( t == LEXSTD::end_of_file_t )
		parser->at_paragraph_beginning = true;
	}

	// Delete what has been skipped.
	//
	// We might issue warning for end of file not
	// immediately preceeded by a line break, but
	// an error message has already been issued by
	// add_tokens.
	//
	if ( first != current )
	{
	    // If we skipped, we stop at an indent
	    // or end of file.
	    //
	    MIN_REQUIRE
	      ( t == LEXSTD::indent_t
		||
	        t == LEXSTD::end_of_file_t
	      );

	    min::uns32 current_indent =
	        t == LEXSTD::indent_t ?
		current->indent : 0;

	    // Delete tokens and find the bounds of any
	    // comments that are not indented as much
	    // as current_indent.
	    //
	    bool iic_exists = false;
	    min::phrase_position iic_position;
		// Data on insufficiently indented
		// comments.  Includes begin of first
		// such and end of last such.

	    while ( first != current )
	    {
		if (    first->type
		     == LEXSTD::indent_before_comment_t
		     &&
		     first->indent < current_indent )
		{
		    MIN_REQUIRE ( first->next->type
		                  ==
			          LEXSTD::comment_t );
		    if ( ! iic_exists )
		    {
		        iic_exists = true;
			iic_position.begin =
			    first->next->position.begin;
		    }
		    iic_position.end =
		        first->next->position.end;
		}
		first = first->next;
		PAR::free
		    ( PAR::remove ( first_ref(parser),
				    first->previous ) );
	    }

	    // Issue warning for any insufficiently
	    // indented comments.
	    //
	    if ( iic_exists )
	        PAR::parse_warn
		    ( parser, iic_position,
		      "comments NOT indented"
		       " as much as following line" );
	}

	if ( indentation_found != min::NULL_STUB )
	{
	    // We come here to process an indented para-
	    // graph.  The indentation mark was found
	    // below but processing was deferred until
	    // after skip above could be done to move
	    // past the line break that followed the
	    // indentation mark.
	    //
	    MIN_REQUIRE
	      ( current->type == LEXSTD::indent_t
		||
	        current->type == LEXSTD::end_of_file_t
	      );

	    // Compute selectors for indented sub-
	    // paragraph.
	    //
	    TAB::flags new_selectors = selectors;
	    new_selectors |=
		indentation_found->new_selectors
				 .or_flags;
	    new_selectors &= ~
		indentation_found->new_selectors
				 .not_flags;
	    new_selectors ^=
		indentation_found->new_selectors
				 .xor_flags;
	    new_selectors |= PAR::ALWAYS_SELECTOR;

	    PAR::token mark_end = current->previous;
	        // Last token of indentation mark.
	    min::int32 indentation_offset =
		bracketed_pass->indentation_offset;

	    // Scan lines of paragraph.  Current will
	    // become the first line break or end of
	    // file after the paragraph.
	    //
	    BRA::line_variables line_variables;
	    line_variables.last_paragraph =
	        min::NULL_STUB;
		// Only component used after paragraph
		// ends.

	    // First be sure paragraph has some
	    // lines.
	    //
	    if ( current->type == LEXSTD::indent_t
		 &&
		 relative_indent
		     ( parser,
		       indentation_offset,
		       current, indent )
		 > 0 )
	    {

		// Compute paragraph_indent for indented
		// subparagraph.
		//
		min::int32 paragraph_indent =
		    current->indent;

		// Move current to beginning of line.
		//
		ensure_next ( parser, current );
		current = current->next;
		PAR::free
		    ( PAR::remove
			( first_ref(parser),
			  current->previous ) );

		// Loop to parse paragraph lines.
		//
		BRA::line_data & paragraph_data =
		    line_variables
		        .indentation_paragraph;
		BRA::line_data & implied_data =
		    line_variables
		        .indentation_implied_paragraph;

		paragraph_data.lexical_master =
		implied_data.lexical_master =
		    indentation_found->lexical_master;
		paragraph_data.selectors =
		implied_data.selectors =
		        new_selectors;
		paragraph_data.implied_header =
		implied_data.implied_header =
		        min::MISSING();
		paragraph_data.header_entry =
		implied_data.header_entry =
		        min::NULL_STUB;
			// Just for safety.

		// If H = indentation_mark->
		//            implied_header,
		// we go through the following loop:
		//     0 times if H is MISSING
		//     1 times if H is paragraph header
		//                with MISSING
		//                implied_subprefix
		//     1 times if H is line header
		//     2 times if H is paragraph header
		//                with non-MISSING
		//                implied_subprefix
		//                (should be a `line'
		//                header)
		//
		min::gen implied_header =
		    indentation_found->implied_header;
		min::gen implied_header_type =
		    indentation_found->
		        implied_header_type;
		TAB::flags header_selectors =
		    new_selectors;
		while (    implied_header
		        != min::MISSING() )
		{
		    TAB::key_table prefix_table =
			bracketed_pass->prefix_table;
		    BRA::prefix header_entry =
			(BRA::prefix)
			TAB::find ( implied_header_type,
			            header_selectors,
				    prefix_table );
		    min::gen group = min::MISSING();
		    if (    header_entry
		         != min::NULL_STUB )
		    {
		        group = header_entry->group;
			header_selectors |=
			    header_entry->
			        new_selectors.or_flags;
			header_selectors &= ~
			    header_entry->
			        new_selectors.not_flags;
			header_selectors ^=
			    header_entry->
			        new_selectors.xor_flags;
		        header_selectors |=
			    PAR::ALWAYS_SELECTOR;
		    }

		    if (    group
		         == PAR::paragraph_lexeme
			 &&
			    paragraph_data
			        .implied_header
			 == min::MISSING() )
		    {
			paragraph_data.implied_header =
			    implied_header;
			paragraph_data.header_entry =
			    header_entry;
			paragraph_data.header_selectors
			    = header_selectors;

			if (    header_entry->
			            lexical_master
			     != PAR::MISSING_MASTER )
			    implied_data.lexical_master
			        = header_entry->
				      lexical_master;

			implied_header =
			    header_entry->
			        implied_subprefix;
			implied_header_type =
			    header_entry->
			        implied_subprefix_type;
		    }
		    else if (    group
		              == PAR::line_lexeme )
		    {
			implied_data.implied_header =
			        implied_header;
			implied_data.header_entry =
			        header_entry;
			implied_data.header_selectors =
			        header_selectors;

			if (    paragraph_data
			            .implied_header
			     == min::MISSING() )
			{
			  paragraph_data
			   .implied_header =
			      implied_data
			        .implied_header;
			  paragraph_data
			   .header_entry =
			      implied_data
			        .header_entry;
			  paragraph_data
			   .header_selectors =
			      implied_data
			        .header_selectors;
			}
			break;
		    }
		    else
		    if (    paragraph_data
			        .implied_header
			 == min::MISSING() )
		    {
			min::phrase_position pos =
			    { current->position.begin,
			      current->position.begin };
			PAR::parse_error
			  ( parser,
			    pos,
			    "implied header of type"
			    " `",
			    min::pgen_never_quote
			      ( implied_header_type
			      ),
			    "' does not have"
			    " `paragraph' or `line'"
			    " group; cannot begin lines"
			    " with indent" );
			break;
		    }
		    else
		        break;
		}

		line_variables.paragraph =
		    line_variables
		        .indentation_paragraph;
		line_variables.implied_paragraph =
		    line_variables
		        .indentation_implied_paragraph;

		line_variables.line_sep =
		    indentation_found->line_sep;
		parser->at_paragraph_beginning = true;
		line_variables.current.selectors = ~
		    PAR::CONTINUING_OPT;
		    // line_variables.current.selectors
		    // is replaced by line_variables.
		    // paragraph.selectors at beginning
		    // of loop.
		while ( true )
		{
		    if ( parser->at_paragraph_beginning
		         &&
		         ! ( line_variables.current
			                   .selectors
			     &
			     PAR::CONTINUING_OPT ) )
			line_variables.current =
			    line_variables.paragraph;

		    line_variables.previous =
		        current->previous;
		    line_variables
		           .at_paragraph_beginning =
			parser->at_paragraph_beginning;
		    separator_found =
		      BRA::
		       parse_bracketed_subexpression
			    ( parser,
			      line_variables.current
			                    .selectors,
			      current,
			      paragraph_indent,
			      NULL,
			      & line_variables,
			      bracket_stack_p );
		    PAR::token first =
		        line_variables.previous->next;
		    PAR::token next = current;

		    // A logical line cannot be closed
		    // by a closing bracket or prefix.
		    //
		    MIN_REQUIRE
		        ( ! BRA::is_closed
			        ( bracket_stack_p ) );

		    // Compact line subsubexp if it
		    // is not empty and not a single
		    // element subsubexp whose element
		    // has prefix group `paragraph' or
		    // `line'.
		    //
		    if ( first != next
			 &&
			 ( first->next != next
			   ||
			   (    first->value_type
			     != PAR::paragraph_lexeme
			     &&
			        first->value_type
			     != PAR::line_lexeme
			   )
			 )
		       )
			PAR::compact_logical_line
			    ( parser,
			      bracketed_pass->next,
			      line_variables.current
			                    .selectors,
			      first, next,
			      separator_found,
			      (TAB::root)
			      indentation_found->
			          line_sep,
			      trace_flags );

		    // Compact prefix paragraph if
		    // necessary.
		    //
		    if (    first->value_type
		         == PAR::paragraph_lexeme )
		    {
		        if ( line_variables
			         .last_paragraph
		             != min::NULL_STUB )
			    PAR::compact_paragraph
				( parser,
				  line_variables
				      .last_paragraph,
				  first,
				  trace_flags );

			if ( parser->
			         at_paragraph_beginning
			     &&
			     ! ( line_variables
			             .current.selectors
			         &
			         PAR::CONTINUING_OPT ) )
			    line_variables
			            .last_paragraph
				= min::NULL_STUB;
			else
			    line_variables
			            .last_paragraph
				= first;
		    }
		    else
		    if ( parser->at_paragraph_beginning
		         &&
		            line_variables
			        .last_paragraph
		         != min::NULL_STUB
			 &&
			 ! ( line_variables.current
			                   .selectors
			     &
			     PAR::CONTINUING_OPT ) )
		    {
		        PAR::compact_paragraph
			    ( parser,
			      line_variables
			          .last_paragraph,
			      next,
			      trace_flags );
			line_variables.last_paragraph
			    = min::NULL_STUB;
		    }

		    // See if there are more lines
		    // in the indented paragraph.
		    //
		    if ( separator_found )
			continue;
		    else if (    current->type
			      == LEXSTD::end_of_file_t )
			break;

		    MIN_REQUIRE
		        (    current->type
			  == LEXSTD::indent_t );

		    if (   relative_indent
		               ( parser,
			         indentation_offset,
			         current,
			         paragraph_indent )
		         < 0 )
			break;

		    // Loop to get next line.
		    //
		    ensure_next ( parser, current );
		    current = current->next;
		    PAR::free
			( PAR::remove
			    ( first_ref(parser),
			      current->previous ) );
		}
	    }

	    // Compact prefix paragraph at end of
	    // indented paragraph.
	    //
	    if (    line_variables.last_paragraph
		 != min::NULL_STUB )
	    {
		PAR::compact_paragraph
		    ( parser,
		      line_variables
			  .last_paragraph,
		      current,
		      trace_flags );
		line_variables.last_paragraph =
		    min::NULL_STUB;
	    }

	    PAR::token next = current;
	    if ( BRA::is_closed
		     ( bracket_stack_p ) )
	    {
		// Paragraph was terminated by outer
		// closing bracket.  Set paragraph end
		// to beginning of that bracket.
		//
		next = bracket_stack_p
			  ->closing_first;
	    }

	    // Compact paragraph lines into a
	    // paragraph.
	    //
	    PAR::token first = mark_end->next;
	    min::phrase_position position;
	    position.begin =
		PAR::remove
		    ( parser, first,
		      indentation_found->label );
	    position.end = next->previous
			       ->position.end;

	    PAR::attr attributes[2] =
		{ PAR::attr
		      ( min::dot_initiator,
			indentation_found->
			    label ),
		  PAR::attr
		      ( min::dot_terminator,
		        min::INDENTED_PARAGRAPH() ) };

	    PAR::compact
		( parser, bracketed_pass->next,
		  new_selectors,
		  first, next, position,
		  trace_flags,
		  PAR::BRACKETING,
		  2, attributes );

	    value_type_ref(first) =
		indentation_found->label;

	    // Terminate subexpression if closing
	    // bracket was found during indentation
	    // processing.
	    // 
	    if ( BRA::is_closed ( bracket_stack_p ) )
		return min::MISSING_POSITION;

	    // Otherwise fall through to process
	    // indent or eof at current that is after
	    // indented lines.
	    //
	    indentation_found = min::NULL_STUB;
	}

	// Continuation after any indented paragraph
	// has been processed, or if there was no
	// indented paragraph.
	//
	if ( current->type == LEXSTD::end_of_file_t )
	    return selectors & PAR::EAPBREAK_OPT ?
		   BRA::PARAGRAPH_END :
		   min::MISSING_POSITION;
	else if ( current->type == LEXSTD::indent_t )
	{
	    if ( ( parser->at_paragraph_beginning
	           &&
		   start_previous->next != current )
	         &&
		 (   selectors
		   & PAR::EAPBREAK_OPT ) )
		    return BRA::PARAGRAPH_END;

	    // Truncate subexpression if current token
	    // indent is at or before indent argument.
	    //
	    if (   selectors
	         & (   PAR::EALEINDENT_OPT
	             | PAR::EALTINDENT_OPT ) )
	    {
		int32 rindent =
		    relative_indent
			( parser,
			  bracketed_pass->
			      indentation_offset,
			  current, indent );
		if ( rindent < 0 )
		    return min::MISSING_POSITION;
		else if ( rindent <= 0
		          &&
		          (   selectors
		            & PAR::EALEINDENT_OPT ) )
		    return min::MISSING_POSITION;
	    }

	    // Next is first part of continution line.
	    // Remove indent and continue with next
	    // token that is not indent, comment,
	    // indent-before-comment, line-break,
	    // or end-of-file.
	    //
	    ensure_next ( parser, current );
	    current = current->next;

	    PAR::free
	        ( PAR::remove ( first_ref(parser),
		                current->previous ) );
	}

	// Continue with non-comment, non-line-break,
	// non-eof, non-indent token.
	//
	MIN_REQUIRE
	    ( indentation_found == min::NULL_STUB );
	MIN_REQUIRE
	    ( current->type != LEXSTD::end_of_file_t
	      &&
	         current->type
	      != LEXSTD::indent_before_comment_t
	      &&
	      current->type != LEXSTD::indent_t
	      &&
	      current->type != LEXSTD::comment_t
	      &&
	      current->type != LEXSTD::line_break_t );

	parser->at_paragraph_beginning = false;

	// Process quoted strings.
	//
	if ( current->type == LEXSTD::quoted_string_t )
	{
	    ensure_next ( parser, current );
	    current = current->next;
	    min::gen concat =
	        bracketed_pass->string_concatenator;

	    if ( concat == min::DISABLED() )
	        goto NEXT_TOKEN;

	    if (    start_previous->next
	         == current->previous )
	        goto NEXT_TOKEN;

	    if ( concat != min::ENABLED() )
	    {
	        if (    current->previous->previous
		               ->value
		     != concat )
		    goto NEXT_TOKEN;
		if (    start_previous->next
		     == current->previous->previous )
		    goto NEXT_TOKEN;
		if (    current->previous->previous
		               ->previous->type
		     != LEXSTD::quoted_string_t )
		    goto NEXT_TOKEN;

		// Remove string_concatenator token.
		//
		PAR::free
		    ( PAR::remove
			( first_ref(parser),
			  current->previous
			         ->previous ) );
	    }
	    else if (    current->previous->previous
	                        ->type
		      != LEXSTD::quoted_string_t )
	        goto NEXT_TOKEN;

	    // Merge current->previous into current->
	    // previous->previous, and delete current->
	    // previous.
	    //
	    min::push
		( (PAR::string_insptr)
		      current->previous->previous
				       ->string,
		  current->previous->string->length,
		  min::begin_ptr_of
		      ( current->previous->string ) );
	    current->previous->previous
		    ->position.end =
		current->previous->position.end;
	    PAR::free
		( PAR::remove
		    ( first_ref(parser),
		      current->previous ) );
	    goto NEXT_TOKEN;
	}

	// If lookup key in bracket table.
	//
	PAR::token key_first = current;
	    // First token of key, or == current if
	    // there is no key.
	TAB::key_prefix key_prefix;
	TAB::root root =
	    find_entry ( parser, current, key_prefix,
			 selectors,
			 bracketed_pass->
			     bracket_table );
	while ( true )
	{
	    // Each iteration of this loop examines the
	    // found entry to see if it is a selected
	    // opening bracket or indentation mark, or a
	    // not necessarily selected closing bracket,
	    // line separator, or typed bracketed punc-
	    // tuation mark that matches a symbol active
	    // because of the bracket_stack or line_sep
	    // arguments.
	    //
	    // Note closing brackets, line selectors,
	    // and typed bracketed punctuation marks
	    // have all selectors on and selectors has
	    // ALWAYS_SELECTOR on.
	    //
	    if ( root == min::NULL_STUB )
	    {
	        // No active bracket table entry found.

		if ( start_previous->next == current
		     &&
		     typed_data != NULL
		     &&
		     current->type == LEXSTD::mark_t )
		{
		    // Current is mark at begining of
		    // typed bracketed subexpresson.
		    //
		    // Turn current into TYPE token.
		    //
		    selectors =
		        typed_data->saved_selectors;
		    typed_data->type = current->value;
		    typed_data->has_mark_type = true;
		    current->type = BRA::TYPE;
		    ++ typed_data->attr_count;
		}

		// Move to next token.
		//
		current = key_first->next;
		break;
	    }

	    min::uns32 subtype =
		min::packed_subtype_of ( root );

	    if ( trace_flags & PAR::TRACE_KEYS )
	        parser->printer
		    << "BRACKETED SUBEXPRESSION PARSER"
		       " FOUND SELECTED KEY "
		    << min::pgen_quote ( root->label )
		    << min::indent << " OF SUBTYPE "
		    << min::name_of_packed_subtype
		           ( min::packed_subtype_of
			         ( root ) )
		    << min::eol;

	    if ( ( subtype == BRA::OPENING_BRACKET
	           ||
		   subtype == BRA::TYPED_OPENING ) )
	    {
		BRA::opening_bracket opening_bracket =
		    (BRA::opening_bracket) root;

		TAB::flags new_selectors = selectors;
		new_selectors |=
		    opening_bracket->new_selectors
				    .or_flags;
		new_selectors &= ~
		    opening_bracket->new_selectors
				    .not_flags;
		new_selectors ^=
		    opening_bracket->new_selectors
				    .xor_flags;
		new_selectors |= PAR::ALWAYS_SELECTOR;

		BRA::bracket_stack cstack
		    ( bracket_stack_p );
		cstack.opening_bracket =
		    opening_bracket;
		cstack.line_variables =
		    line_variables;

		PAR::token previous = current->previous;
		BRA::typed_data tdata;
		bool is_mark_prefix = false;
		if ( subtype == BRA::OPENING_BRACKET )
		{
		    separator_found =
			PARSE_BRA_SUBEXP
			  ( parser, new_selectors,
			    current, indent,
			    NULL,
			    line_variables, & cstack );
		}
		else // if (    subtype
		     //      == BRA::TYPED_OPENING )
		{
		    tdata.typed_opening =
			(BRA::typed_opening) root;
		    tdata.saved_selectors =
		        new_selectors;
		    tdata.middle_count = 0;
		    tdata.attr_count = 0;
		    tdata.start_previous =
		        current->previous;
		    tdata.elements = min::NULL_STUB;
		    tdata.attributes = min::NULL_STUB;
		    tdata.end_position =
			min::MISSING_POSITION;
		    tdata.subtype = BRA::TYPED_OPENING;
		    tdata.type = min::MISSING();
		    tdata.has_mark_type = false;

		    TAB::flags tselectors =
		        new_selectors;
		    tselectors &= PAR::ALL_OPT;
		    tselectors
		        |= tdata.typed_opening
				   ->attr_selectors
		        | PAR::ALWAYS_SELECTOR;

		    separator_found =
			PARSE_BRA_SUBEXP
			  ( parser, tselectors,
			    current, indent,
			    & tdata,
			    line_variables, & cstack );

		    // We do typed bracketed subexpres-
		    // sion finishing here that is
		    // logically done by TYPED_CLOSING,
		    // but is instead done here because
		    // the TYPED_CLOSING may be missing
		    // and thus be inserted below to
		    // correct its being missing.

		    PAR::token next =
		        cstack.closing_first;
		    if ( next == min::NULL_STUB )
		        next = current;

		    if ( tdata.has_mark_type )
		    {
			PAR::token type_token =
			    previous->next;
			MIN_REQUIRE
			    (    type_token->type
			      == BRA::TYPE );
			MIN_REQUIRE
			    (    type_token->value
			      == tdata.type );
			if (    type_token
			     == next->previous )
			    is_mark_prefix = true;
			else if (    next->previous
			                 ->type
			          != LEXSTD::mark_t )
			    ::missing_error
			        ( parser, next, "`",
				  min::pgen_never_quote
				    ( tdata.type ),
				  "' at end of"
				  " typed bracketed"
				  " expression;"
				  " inserted before" );
			else
			{
			    if (    next->previous
			                ->value
				 != tdata.type )
			    {
				min::gen v[2] =
				    { tdata.type,
				      next->previous
					  ->value };
				PAR::value_ref
					(type_token) =
				    min::new_lab_gen
					 ( v, 2 );
			    }
			    MIN_REQUIRE
			        (    type_token
				  != next->previous );
			    PAR::free
			        ( PAR::remove
				    ( first_ref(parser),
				      next->previous )
				);
			}
			if (    type_token
			     != next->previous )
			    tdata.elements =
				type_token->next;
		    }
		    else
		    if ( tdata.middle_count % 2 == 1 )
		    {
		        MIN_REQUIRE
			    (    tdata.subtype
			      == BRA::TYPED_MIDDLE );
			missing_error
			    ( parser, next, "`",
			      min::pgen_never_quote
			          ( tdata.typed_opening
				      ->typed_middle
				      ->label ),
			      "'; inserted before" );
			if (    tdata.elements
			     == min::NULL_STUB
			     &&
			        tdata.start_previous
				    ->next
			     != next )
			    tdata.elements =
			        tdata.start_previous
				    ->next;
		    }
		    else
		    {
	                // Note TYPED_ATTR_BEGIN can
			// only happen here if typed
			// attribute beginning was AFTER
			// an attribute, as if it is
			// after a beginning type,
			// tdata.subtype is set to
			// TYPED_ATTR_SEP instead.

		        if ( (    tdata.subtype
			       == BRA::TYPED_OPENING
			       ||
			          tdata.subtype
			       == BRA::TYPED_MIDDLE )
			     &&
			        tdata.start_previous
				          ->next
			     != next )
			{
			    ::make_type_label
			        ( parser, & tdata,
				  next );
			}
		        else if (    tdata.subtype
			          == BRA::
				     TYPED_ATTR_BEGIN )
			{
			    ::make_type_label
			        ( parser, & tdata,
				  next );
			}
		        else if (    tdata.subtype
			          != BRA::TYPED_OPENING
				)
			{
			    ::finish_attribute
				( parser, & tdata,
				  next );
			    ::move_attributes
				( parser, & tdata,
				  next );
			}
		    }
		}

		// Separator cannot be found inside
		// brackets.
		//
		MIN_REQUIRE ( ! separator_found );

		PAR::token next = current;
		min::phrase_position position;
		    // Arguments for compact.

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
		    if (   cstack.closing_first
			 != min::NULL_STUB )
			next = cstack.closing_first;
		    position.end =
			  next->previous->position.end;

		    // Because tokens in a typed
		    // bracketed subexpression have been
		    // rearranged, the original next->
		    // previous may have been moved or
		    // removed, and so cannot be used
		    // for the end position.  Tdata.end_
		    // position records the true end of
		    // the subexpression in this case.
		    // 
		    if ( subtype == BRA::TYPED_OPENING
		         &&
			 tdata.end_position
			 &&
			   tdata.end_position
			 > position.end )
		        position.end =
			    tdata.end_position;

		    if (   cstack.closing_first
			 == min::NULL_STUB )
		    {
			min::phrase_position pos =
			    position;
			pos.begin = position.end;
		        PAR::parse_error
			    ( parser, pos,
			      "missing closing"
			      " bracket `",
			      min::pgen_never_quote
			          ( opening_bracket->
			            closing_bracket->
				        label ),
			      "' inserted at end of"
			      " logical line that"
			      " ends here" );
		    }
		    else if ( min::is_name
		                  ( next->value ) )
		        PAR::parse_error
			    ( parser, next->position,
			      "missing closing"
			      " bracket `",
			      min::pgen_never_quote
			          ( opening_bracket->
			            closing_bracket->
				        label ),
			      "' inserted before `",
			      min::pgen_never_quote
			          ( next->value ),
			      "'" );
		    else
		    {
			min::phrase_position position =
			    next->position;
			position.begin = position.end;
		        PAR::parse_error
			    ( parser, position,
			      "missing closing"
			      " bracket `",
			      min::pgen_never_quote
			          ( opening_bracket->
			            closing_bracket->
				        label ),
			      "' inserted just before"
			      " here" );
		    }
		}
		else
		{
		    MIN_REQUIRE (    cstack.closing_next
		                  == current );
		    position.end =
			current->previous->position.end;

		    PAR::remove ( parser, current,
			          cstack.opening_bracket
			              ->closing_bracket
				      ->label );
		}

		PAR::token first = previous->next;
		position.begin =
		    PAR::remove
			( parser, first,
			  opening_bracket->label );
		    // At this point previous is no
		    // longer valid.

		if ( subtype == BRA::OPENING_BRACKET )
		{
		    if (    opening_bracket
		                 ->reformatter
		         == min::NULL_STUB
		         ||
		         ( * opening_bracket->
			       reformatter->
			       reformatter_function )
			     ( parser,
			       (PAR::pass)
			           bracketed_pass,
			       new_selectors,
			       first, next, position,
			       trace_flags,
			       (TAB::root)
			           opening_bracket )
		       )
		    {
			// Untyped bracketed subexpres-
			// sion without reformatter, or
			// with reformatter requesting
			// compaction.

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
			    ( parser,
			      bracketed_pass->next,
			      new_selectors,
			      first, next, position,
			      trace_flags,
			      PAR::BRACKETING,
			      2, attributes, 1 );

			value_type_ref(first) =
			    opening_bracket->label;
		    }
		}
		else // if (    subtype
		     //      == BRA::TYPED_OPENING )
		{
		    PAR::attr attributes
			[tdata.attr_count];
		    PAR::token elements =
		        tdata.elements;
		    if ( elements == min::NULL_STUB )
		        elements = next;
		    min::gen type = min::MISSING();
		    min::unsptr i = 0;
		    bool skip = false;	
		    for ( PAR::token t = first;
		          t != elements; t = t->next )
		    {
		        if ( t->type == BRA::TYPE
			     &&
			        t->value
			     != min::empty_str )
			{
			    MIN_REQUIRE
			      ( i < tdata.attr_count );
			    if (    t->value
			         == min::empty_lab )
			    {
			        PAR::parse_error
				    ( parser,
				      t->position,
				      "empty type"
				      " label;"
				      " ignored" );
			        skip = true;
			    }
			    else
			    {
				attributes[i].name =
				    min::dot_type;
				attributes[i].value =
				    t->value;
				type = t->value;
				++ i;
				skip = false;
			    }
			}
		        else if (    t->type
			          == BRA::ATTR_LABEL )
			{
			    MIN_REQUIRE
			      ( i < tdata.attr_count );
			    if (    t->value
			         == min::empty_lab )
			    {
			        PAR::parse_error
				    ( parser,
				      t->position,
				      "empty attribute"
				      " label;"
				      " attribute"
				      " ignored" );
			        skip = true;
			    }
			    else
			    {
				attributes[i].name =
				    t->value;
				++ i;
				skip = false;
			    }
			}
		        else if (    t->type
			          == BRA::ATTR_TRUE )
			{
			    MIN_REQUIRE
			      ( i < tdata.attr_count );
			    if (    t->value
			         == min::empty_lab )
			    {
			        PAR::parse_error
				    ( parser,
				      t->position,
				      "empty attribute"
				      " label;"
				      " attribute"
				      " ignored" );
			        skip = true;
			    }
			    else
			    {
				attributes[i].name =
				    t->value;
				attributes[i].value =
				    min::TRUE;
				++ i;
				skip = false;
			    }
			}
		        else if (    t->type
			          == BRA::ATTR_FALSE )
			{
			    MIN_REQUIRE
			      ( i < tdata.attr_count );
			    if (    t->value
			         == min::empty_lab )
			    {
			        PAR::parse_error
				    ( parser,
				      t->position,
				      "empty attribute"
				      " label;"
				      " attribute"
				      " ignored" );
			        skip = true;
			    }
			    else
			    {
				attributes[i].name =
				    t->value;
				attributes[i].value =
				    min::FALSE;
				++ i;
				skip = false;
			    }
			}
			else if ( skip )
			    /* Do nothing */;
		        else if (    t->type
			          == BRA::ATTR_VALUE )
			{
			    // min::is_attr_legal
			    //     ( t->value )
			    // checked when token made
			    // and if false token value
			    // replaced by NONE.
			    //
			    if (    t->value
			         == min::NONE() )
			    {
			        -- i;
				skip = true;
			    }
			    else
				attributes[i-1].value =
				    t->value;
			}
		        else if (    t->type
			          == BRA::
				     ATTR_MULTIVALUE )
			    attributes[i-1].multivalue =
			        t->value;
		        else if (    t->type
			          == BRA::ATTR_FLAGS )
			    attributes[i-1].flags =
			        t->value;
		    }

		    min::uns32 token_type =
		        i == 0 ?
			    PAR::BRACKETING :
			i == 1
			&&
			   attributes[0].name
			== min::dot_type ?
			    PAR::BRACKETING :
			    PAR::BRACKETED;
		    if ( type != min::MISSING()
		         &&
			 tdata.middle_count == 0
		         &&
			 ( ! tdata.has_mark_type 
			   ||
			   is_mark_prefix )
			 &&
			 tdata.typed_opening->
			     prefix_separators_allowed )
		        token_type = PAR::PREFIX;
		    else
		        type = min::MISSING();

		    bool first_equals_elements =
		        ( first == elements );
		    PAR::compact
			( parser, bracketed_pass->next,
			  new_selectors,
			  elements, next, position,
			  trace_flags,
			  token_type,
			  i, attributes, 1 );

		    value_type_ref(elements) = type;

		    // We delay deleting tokens until
		    // their values are protected from
		    // the garbage collector by compact.
		    //
		    // Compact deletes the elements and
		    // inserts a new token before `next'
		    // containing the compacted sub-
		    // expression, setting `elements'
		    // to point at that token.
		    //
		    if ( ! first_equals_elements )
		    do
		    {
		        first = first->next;
			PAR::free
			    ( PAR::remove
				( first_ref(parser),
				  first->previous ) );
		    }
		    while ( first != elements );

		    if ( elements->type == PAR::PREFIX )
		    {
			prefix = elements;
			premature_closing =
			    (    cstack.closing_next
		              == cstack.closing_first );
			goto PREFIX_FOUND;
		    }
		}

		// Come here after compacting.

		if (    cstack.closing_next
		     == cstack.closing_first )
		{
		    // Found a closing bracket that is
		    // not ours or logical line end.
		    // Kick to caller.
		    //
		    return separator_found;
		}
		else
		    break;
	    }
	    else if ( subtype == BRA::CLOSING_BRACKET )
	    {
		BRA::closing_bracket closing_bracket =
		    (BRA::closing_bracket) root;

		for ( BRA::bracket_stack * p =
			  bracket_stack_p;
		      p != NULL
		      &&
		         p->line_variables
		      == line_variables;
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

			return min::MISSING_POSITION;
		    }
		    else if ( ! (   selectors
		                  & PAR::EAOCLOSING_OPT
				) )
		        break;
		}

		// Closing bracket does not match a
		// bracket stack entry; reject key.
	    }
	    else if ( subtype == BRA::INDENTATION_MARK )
	    {
                if (    current->type
		     == LEXSTD::line_break_t
		     ||
		        current->type
		     == LEXSTD::comment_t
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
	    else if ( subtype == BRA::LINE_SEP
	              &&
                         line_variables->line_sep
		      == (BRA::line_sep) root
		      &&
		      (   selectors
		        & PAR::EALSEP_OPT ) )
	    {
		separator_found =
		    current->previous->position.end;
		PAR::remove
		    ( parser, current, root->label );
		return separator_found ;
	    }
	    else if ( subtype == BRA::TYPED_MIDDLE
		      ||
		         subtype
		      == BRA::TYPED_DOUBLE_MIDDLE )
	    {
		if ( typed_data != NULL
		     &&
		        root
		     == (    subtype
			   == BRA::TYPED_MIDDLE ?
		          (TAB::root)
		          typed_data->typed_opening
			            ->typed_middle :
		          (TAB::root)
		          typed_data->
			      typed_opening->
			      typed_double_middle )
		     &&
		     ! typed_data->has_mark_type )
		{
		    typed_data->end_position =
		        current->previous->position.end;

		    if (    typed_data->middle_count % 2
		         == 0 )
		    {
		        if (    typed_data->subtype
			     == BRA::TYPED_OPENING
			     &&
			        typed_data
				    ->start_previous
				    ->next
			     != key_first )
			    ::make_type_label
			        ( parser, typed_data,
				  key_first );
		        else if (    typed_data->subtype
			          != BRA::TYPED_OPENING
				)
			{
			    ::finish_attribute
				( parser, typed_data,
				  key_first );
			    ::move_attributes
				( parser, typed_data,
				  key_first );
			}
		    }
		    else // if
		         // typed_data->middle_count % 2
			 // == 1
		    {
			if (    typed_data->elements
			     == min::NULL_STUB
			     &&
			        typed_data
				    ->start_previous
				    ->next
			     != key_first )
			    typed_data->elements =
			        typed_data
				    ->start_previous
				    ->next;
		    }

		    PAR::remove ( parser, current,
			          root->label );
		    typed_data->subtype =
			BRA::TYPED_MIDDLE;
		    typed_data->start_previous =
		        current->previous;
		    ++ typed_data->middle_count;

		    if (    subtype
		         == BRA::TYPED_DOUBLE_MIDDLE )
		    {
			++ typed_data->middle_count;
			break;
		    }

		    // Do the following only if subtype
		    // == BRA::TYPED_MIDDLE.

		    if (    typed_data->middle_count % 2
		         == 0 )
			selectors =
			    typed_data->saved_selectors;
		    else // if
		         // typed_data->middle_count % 2
			 // == 1
		    {
			selectors &= PAR::ALL_OPT;
			selectors |=
			      typed_data
			          ->typed_opening
			          ->attr_selectors
			    | PAR::ALWAYS_SELECTOR;
		    }

		    break;
		}

		// Typed middle does not match typed_
		// opening; reject key.
	    }
	    else if (    subtype
	              == BRA::TYPED_ATTR_BEGIN )
	    {
		if ( typed_data != NULL
		     &&
		        typed_data->typed_opening
			          ->typed_attr_begin
		     == (BRA::typed_attr_begin) root
		     &&
		     typed_data->middle_count % 2 == 0
		     &&
		     ! typed_data->has_mark_type )
		{
		    typed_data->end_position =
		        current->previous->position.end;
		        
		    if (    typed_data->subtype
		         == BRA::TYPED_OPENING )
		    {
			::make_type_label
			    ( parser, typed_data,
			      key_first );
			PAR::remove
			    ( parser, current,
			      root->label );
			typed_data->subtype =
			    BRA::TYPED_ATTR_SEP;
			typed_data->start_previous =
			    current->previous;
		    }
		    else if (   typed_data->middle_count
			      > 0
			      &&
				 key_first
			      != typed_data
			             ->start_previous
				     ->next )
		    {
			::finish_attribute
			    ( parser, typed_data,
			      key_first );
			::move_attributes
			    ( parser, typed_data,
			      key_first );
			PAR::remove
			    ( parser, current,
			      root->label );
			typed_data->subtype =
			    BRA::TYPED_ATTR_BEGIN;
			typed_data->start_previous =
			    current->previous;
		    }
		    else
		        ::punctuation_error
			    ( parser, key_first,
			      current, root->label );
		    break;
		}

		// Typed_attr_begin does not match
		// typed_opening; reject key.
	    }
	    else if (    subtype
	              == BRA::TYPED_ATTR_EQUAL )
	    {
		if ( typed_data != NULL
		     &&
		        typed_data->typed_opening
			          ->typed_attr_equal
		     == (BRA::typed_attr_equal) root
		     &&
		     typed_data->middle_count % 2 == 0
		     &&
		     ! typed_data->has_mark_type )
		{
		    typed_data->end_position =
		        current->previous->position.end;
		        
		    if (    typed_data->subtype
		         == BRA::TYPED_OPENING
			 ||
		            typed_data->subtype
		         == BRA::TYPED_MIDDLE
			 ||
		            typed_data->subtype
		         == BRA::TYPED_ATTR_SEP
			 ||
		            typed_data->subtype
		         == BRA::TYPED_ATTR_NEGATOR )
		    {
			::make_attribute_label
			    ( parser, typed_data,
			      key_first );
			PAR::remove
			    ( parser, current,
			      root->label );
			typed_data->subtype =
			    BRA::TYPED_ATTR_EQUAL;
			typed_data->start_previous =
			    current->previous;
		    }
		    else
		        ::punctuation_error
			    ( parser, key_first,
			      current, root->label );
		    break;
		}

		// Typed_attr_equal does not match
		// typed_opening; reject key.
	    }
	    else if (    subtype
	              == BRA::TYPED_ATTR_SEP )
	    {
		if ( typed_data != NULL
		     &&
		        typed_data->typed_opening
			          ->typed_attr_sep
		     == (BRA::typed_attr_sep) root
		     &&
		     typed_data->middle_count % 2 == 0
		     &&
		     ! typed_data->has_mark_type )
		{
		    typed_data->end_position =
		        current->previous->position.end;
		        
		    if (    typed_data->subtype
		         == BRA::TYPED_OPENING
			 ||
		            typed_data->subtype
		         == BRA::TYPED_MIDDLE
			 ||
		            typed_data->subtype
		         == BRA::TYPED_ATTR_SEP
			 ||
		            typed_data->subtype
		         == BRA::TYPED_ATTR_EQUAL
			 ||
		            typed_data->subtype
		         == BRA::TYPED_ATTR_NEGATOR )
		    {
			::finish_attribute
			    ( parser, typed_data,
			      key_first );
			PAR::remove
			    ( parser, current,
			      root->label );
			typed_data->subtype =
			    BRA::TYPED_ATTR_SEP;
			typed_data->start_previous =
			    current->previous;
		    }
		    else
		        ::punctuation_error
			    ( parser, key_first,
			      current, root->label );
		    break;
		}

		// Typed_attr_sep does not match
		// typed_opening; reject key.
	    }
	    else if (    subtype
	              == BRA::TYPED_ATTR_NEGATOR )
	    {
		if ( typed_data != NULL
		     &&
		        typed_data->typed_opening
			          ->typed_attr_negator
		     == (BRA::typed_attr_negator) root
		     &&
		     typed_data->middle_count % 2 == 0
		     &&
		     ! typed_data->has_mark_type )
		{
		    typed_data->end_position =
		        current->previous->position.end;
		        
		    if ( (    typed_data->subtype
			   == BRA::TYPED_OPENING
			   ||
			      typed_data->subtype
		           == BRA::TYPED_ATTR_SEP
			   ||
			      typed_data->subtype
			   == BRA::TYPED_MIDDLE
			 )
			 &&
			    key_first
			 == typed_data->start_previous
			              ->next )
		    {
			PAR::remove
			    ( parser, current,
			      root->label );
			typed_data->subtype =
			    BRA::TYPED_ATTR_NEGATOR;
			typed_data->start_previous =
			    current->previous;
			break;
		    }
		}

		// Typed_attr_negator does not match
		// typed_opening and appear in attribute
		// label beginning context; reject key.
	    }

	    if ( trace_flags & PAR::TRACE_KEYS )
	        parser->printer
		    << "BRACKETED SUBEXPRESSION PARSER"
		       " REJECTED KEY "
		    << min::pgen_quote ( root->label )
		    << min::eol;

	    root = PAR::find_next_entry
	               ( parser, current, key_prefix,
			 selectors, root );
	}

	// Loop to next token.
	//
	goto NEXT_TOKEN;
    }

    MIN_ABORT ( "SHOULD NOT COME HERE" );
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
    ::make_label ( parser, first, next, true );
    first->position = position;

    PAR::trace_subexpression
	( parser, first, trace_flags );

    return false;
}

// Copy of min::standard_special_names but with min::gen
// string elements instead of min::ustring elements.
//
//    if v = MUP::new_special_gen ( i )
//    then ::special_names[0xFFFFFF - i] is name of v
//
static min::locatable_var
           <min::packed_vec_ptr<min::gen> >
    special_names;

static void special_names_initialize ( void )
{
    min::packed_vec_ptr<min::ustring>
        unames = min::standard_special_names;
    min::locatable_var
	       <min::packed_vec_insptr<min::gen> >
	gnames
	( min::gen_packed_vec_type.new_stub ( 10 ) );
    ::special_names = gnames;
    for ( unsigned i = 0; i < unames->length; ++ i )
    {
	min::locatable_gen n =
	    min::new_str_gen
	        ( min::ustring_chars ( unames[i] ) );
        min::push ( gnames ) = n;
    }
}

static min::initializer special_names_initializer
	    ( ::special_names_initialize );

static bool special_reformatter_function
        ( PAR::parser parser,
	  PAR::pass pass,
	  TAB::flags selectors,
	  PAR::token & first,
	  PAR::token next,
	  const min::phrase_position & position,
	  TAB::flags trace_flags,
	  TAB::root entry )
{
    ::make_label ( parser, first, next );

    unsigned i;
    for ( i = 0; i < ::special_names->length; ++ i )
    {
        if ( first->value == ::special_names[i] )
	    break;
    }

    if ( i < ::special_names->length )
        value_ref(first) = MUP::new_special_gen
			       ( 0xFFFFFF - i );
    else
    {
	PAR::parse_error
	    ( parser, first->position,
	      "subexpression ",
	      min::pgen_quote ( first->value ),
	      " unrecognized special name;"
	      " changed to ERROR" );

	value_ref(first) = min::ERROR();
    }

    first->position = position;

    PAR::trace_subexpression
	( parser, first, trace_flags );

    return false;
}

static bool multivalue_reformatter_function
        ( PAR::parser parser,
	  PAR::pass pass,
	  TAB::flags selectors,
	  PAR::token & first,
	  PAR::token next,
	  const min::phrase_position & position,
	  TAB::flags trace_flags,
	  TAB::root entry )
{
    BRA::opening_bracket opening_bracket =
        (BRA::opening_bracket) entry;
    min::gen separator =
        opening_bracket->reformatter_arguments[0];

    min::unsptr count = 0;
    PAR::token start = first;
    PAR::token t = first;
    while ( true )
    {
        if ( t == next || t->value == separator )
	{
	    if ( start != t )
	    {
	        if ( start->next != t
		     ||
		     start->string != min::NULL_STUB )
		    ::make_label ( parser, start, t );
		++ count;
	    }

	    if ( t == next ) break;
	    t = t->next;
	    if ( t->previous == first )
	        first = t;
	    PAR::free
		( PAR::remove
		    ( first_ref(parser),
		      t->previous ) );
	    start = t;
	}
	else t = t->next;
    }
        
    PAR::attr separator_attr
        ( min::dot_separator, separator );

    PAR::compact
        ( parser, min::NULL_STUB, selectors,
	  first, next, position,
	  trace_flags, PAR::BRACKETABLE,
	  1, & separator_attr );

    return true;
}

min::locatable_var<PAR::reformatter>
    BRA::reformatter_stack ( min::NULL_STUB );

static void reformatter_stack_initialize ( void )
{
    ::initialize();

    min::locatable_gen label
        ( min::new_str_gen ( "label" ) );
    PAR::push_reformatter
        ( label, 0, 0, 0,
	  ::label_reformatter_function,
	  BRA::reformatter_stack );

    min::locatable_gen special
        ( min::new_str_gen ( "special" ) );
    PAR::push_reformatter
        ( special, 0, 0, 0,
	  ::special_reformatter_function,
	  BRA::reformatter_stack );

    min::locatable_gen multivalue
        ( min::new_str_gen ( "multivalue" ) );
    PAR::push_reformatter
        ( multivalue, 0, 1, 1,
	  ::multivalue_reformatter_function,
	  BRA::reformatter_stack );
}
static min::initializer reformatter_initializer
    ( ::reformatter_stack_initialize );

// Bracketed Pass Command Function
// --------- ---- ------- --------

enum definition_type
    { BRACKET,
      INDENTATION_MARK,
      TYPED_BRACKET,
      PREFIX };

static min::gen bracketed_pass_command
	( PAR::parser parser,
	  PAR::pass pass,
	  min::obj_vec_ptr & vp, min::uns32 i0,
          min::phrase_position_vec ppvec )
{
    BRA::bracketed_pass bracketed_pass =
        (BRA::bracketed_pass) pass;

    min::uns32 size = min::size_of ( vp );

    // Scan keywords before names.
    //
    unsigned i = i0;
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
    if ( i >= size )
        return min::FAILURE();

#   define SCAN_NAME(variable, partial_name_ok) \
        variable = \
	  PAR::scan_quoted_key \
	    ( vp, i, parser, partial_name_ok ); \
	\
	if ( variable == min::ERROR() ) \
	    return min::ERROR(); \
	else if ( variable == min::MISSING() ) \
	    return PAR::parse_error \
	        ( parser, ppvec[i-1], \
		  "expected quoted name after" );

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

	    min::uns32 indent =
		COM::print_command ( parser, ppvec );
	    parser->printer
		<< min::bom << min::no_auto_break
		<< min::set_indent ( indent + 4 );

	    min::int32 offset =
	        bracketed_pass->indentation_offset; 
	    for ( min::uns32 i =
	              bracketed_pass->
		          block_stack->length;
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

		BRA::block_struct b =
		    bracketed_pass->block_stack[i-1];
		offset = b.indentation_offset;
	    }

	    parser->printer << min::eom;
	    return PAR::PRINTED;
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

    if ( vp[i] == ::string_lexeme
         &&
	 i + 1 < size
	 &&
	 vp[i+1] == ::concatenator )
    {
        if ( command == PAR::print )
	{
	    if ( i + 2 < size )
		return PAR::parse_error
		    ( parser, ppvec[i+1],
		      "unexpected stuff after" );

	    min::uns32 indent =
		COM::print_command ( parser, ppvec );
	    parser->printer
		<< min::bom << min::no_auto_break
		<< min::set_indent ( indent + 4 );

	    min::gen concat =
	        bracketed_pass->string_concatenator; 
	    for ( min::uns32 i =
	              bracketed_pass->
		          block_stack->length;
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
				<< ": ";
		if ( concat == min::ENABLED() )
		    parser->printer << "enabled";
		else if ( concat == min::DISABLED() )
		    parser->printer << "disabled";
		else
		    parser->printer
			<< min::pgen_quote ( concat );

		if ( i == 0 ) break;

		BRA::block_struct b =
		    bracketed_pass->block_stack[i-1];
		concat = b.string_concatenator;
	    }

	    parser->printer << min::eom;
	    return PAR::PRINTED;
	}
	else if ( command != PAR::define )
	    return min::FAILURE();

	else if ( i + 2 >= size )
	    return PAR::parse_error
		( parser, ppvec[i+1],
		  "expected string concatenator"
		  " after" );

	i += 2;
	min::locatable_gen concat = vp[i];
	if ( concat == PAR::enabled_lexeme )
	    concat = min::ENABLED();
	else if ( concat == PAR::disabled_lexeme )
	    concat = min::DISABLED();
	else
	{
	    SCAN_NAME ( concat, false );
	    if ( min::is_lab ( concat ) )
		return PAR::parse_error
		    ( parser, ppvec[i+2],
		      "too many components" );
	}

	if ( i + 1 < size )
	    return PAR::parse_error
		( parser, ppvec[i],
		  "unexpected stuff after" );

	BRA::string_concatenator_ref
	    ( bracketed_pass ) = concat;

	return min::SUCCESS();
    }

    if ( command == PAR::print )
    {
        if ( vp[i] == ::bracket )
	    type = ::BRACKET;
	else if ( vp[i] == PAR::prefix_lexeme )
	    type = ::PREFIX;
	else
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
    else if ( vp[i] == ::typed
              &&
	      i + 1 < size
	      &&
	      vp[i + 1] == ::bracket )
    {
	type = ::TYPED_BRACKET;
	min_names = 2;
	max_names = 4;
	i += 2;
    }
    else if ( vp[i] == PAR::prefix_lexeme )
    {
	type = ::PREFIX;
	min_names = 1;
	max_names = 1;
	++ i;
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
	SCAN_NAME ( name[number_of_names],
	            command == PAR::print );

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

	min::uns32 indent =
	    COM::print_command ( parser, ppvec );

	parser->printer
	    << min::bom << min::no_auto_break
	    << min::set_indent ( indent + 4 );

	int count = 0;

	TAB::key_table_iterator it
	    ( type == ::BRACKET ?
	      bracketed_pass->bracket_table :
	      bracketed_pass->prefix_table );
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
		 subtype != BRA::TYPED_OPENING
		 &&
		 subtype != BRA::INDENTATION_MARK
		 &&
		 subtype != BRA::PREFIX )
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

	    if ( subtype == BRA::OPENING_BRACKET
		 ||
		 subtype == BRA::TYPED_OPENING )
	    {
		BRA::opening_bracket opening_bracket =
		    (BRA::opening_bracket) root;
		BRA::closing_bracket closing_bracket =
		    opening_bracket->closing_bracket;

		BRA::typed_opening typed_opening =
		    (BRA::typed_opening) root;

		parser->printer
		    << "bracket "
		    << min::pgen_quote
			   ( opening_bracket->label )
		    << " ... ";

		BRA::typed_double_middle double_middle =
		    min::NULL_STUB;
		if ( typed_opening != min::NULL_STUB )
		{
		    BRA::typed_middle middle =
			typed_opening->typed_middle;
		    if ( middle != min::NULL_STUB )
		    {
			parser->printer
			    << min::pgen_quote
				   ( middle->label )
			    << " ... "
			    << min::pgen_quote
				   ( middle->label )
			    << " ... ";
			double_middle =
			    typed_opening->
				typed_double_middle;
		    }
		}

		parser->printer
		    << min::pgen_quote
			   ( closing_bracket->label )
		    << " " << min::set_break;

		if ( double_middle != min::NULL_STUB )
		{
		    parser->printer
			<< "// "
			<< min::pgen_quote
			       ( double_middle->label )
			<< " is also allowed."
			<< min::indent;
		}

		COM::print_flags
		    ( root->selectors,
		      PAR::COMMAND_SELECTORS,
		      parser->selector_name_table,
		      parser );

		TAB::new_flags new_selectors =
		    opening_bracket->new_selectors;

		if ( TAB::all_flags ( new_selectors )
		     &
		     PAR::COMMAND_SELECTORS )
		{
		    parser->printer
			<< min::indent
			<< "with parsing"
			   " selectors ";
		    COM::print_new_flags
			( new_selectors,
			  PAR::COMMAND_SELECTORS,
			  parser->selector_name_table,
			  parser, true );
		}

		if ( TAB::all_flags ( new_selectors )
		     &
		     PAR::ALL_BRACKET_OPT )
		{
		    parser->printer
			<< min::indent
			<< "with parsing"
			   " options ";
		    COM::print_new_flags
			( new_selectors,
			  PAR::ALL_BRACKET_OPT,
			  parser->selector_name_table,
			  parser, true );
		}

		if ( typed_opening == min::NULL_STUB
		     &&
			opening_bracket->reformatter
		     != min::NULL_STUB )
		{
		    parser->printer
			<< min::indent
			<< "with "
			<< min::pgen_name
			       ( opening_bracket->
				     reformatter->name )
			<< " reformatter";

		    min::packed_vec_ptr<min::gen> args =
			opening_bracket->
			    reformatter_arguments;
		    if ( args != min::NULL_STUB )
		    {
			parser->printer
			    << " ( " << min::set_break;
			for ( min::uns32 i = 0;
			      i < args->length; ++ i )
			{
			    if ( i != 0 )
				parser->printer
				    << ", "
				    << min::set_break;
			    parser->printer
				<< min::pgen_quote
				       ( args[i] );
			}
			parser->printer << " )";
		    }
		}

#		define TOATTR(x) \
		    typed_opening->typed_attr_ ## x
#		define PQ(x) min::pgen_quote ( x )

		if ( typed_opening != min::NULL_STUB )
		{
		    parser->printer
			<< min::indent
			<< "with attribute"
			   " selectors ";
		    COM::print_flags
			( typed_opening->attr_selectors,
			  PAR::COMMAND_SELECTORS,
			  parser->selector_name_table,
			  parser );

		    if (    TOATTR(begin)
			 != min::NULL_STUB )
			parser->printer
			    << min::indent
			    << "with attributes "
			    << PQ ( TOATTR
				      (begin->label) )
			    << " ... "
			    << PQ ( TOATTR
				      (equal->label) )
			    << " ... "
			    << PQ ( TOATTR
				      (sep->label) );

		    if (    TOATTR(negator)
			 != min::NULL_STUB )
			parser->printer
			    << min::indent
			    << "with attribute negator "
			    << PQ ( TOATTR
				      (negator->label)
				  );

		    if (    TOATTR(flags_initiator)
			 != min::MISSING() )
			parser->printer
			    << min::indent
			    << "with attribute flags"
			    << " initiator "
			    << PQ ( TOATTR
				      (flags_initiator)
				  );

		    if (    TOATTR(multivalue_initiator)
			 != min::MISSING() )
			parser->printer
			  << min::indent
			  << "with attribute"
			     " multivalue initiator "
			  << PQ
			     ( TOATTR
				 (multivalue_initiator)
			     );
		    if ( typed_opening->
			     prefix_separators_allowed )
			parser->printer
			  << min::indent
			  << "with prefix"
			     " separators allowed";
		}

#		undef PQ
#		undef TOATTR
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
		      PAR::COMMAND_SELECTORS,
		      parser->selector_name_table,
		      parser );

		TAB::new_flags new_selectors =
		    indentation_mark->new_selectors;

		if ( TAB::all_flags ( new_selectors )
		     &
		     PAR::COMMAND_SELECTORS )
		{
		    parser->printer
			<< min::indent
			<< "with parsing"
			   " selectors ";
		    COM::print_new_flags
			( new_selectors,
			  PAR::COMMAND_SELECTORS,
			  parser->
			      selector_name_table,
			  parser, true );
		}

		if ( TAB::all_flags ( new_selectors )
		     &
		     PAR::ALL_EA_OPT )
		{
		    parser->printer
			<< min::indent
			<< "with parsing"
			   " options ";
		    COM::print_new_flags
			( new_selectors,
			  PAR::ALL_EA_OPT,
			  parser->
			      selector_name_table,
			  parser, true );
		}

		min::gen header =
		    indentation_mark->
		        implied_header;
		if ( header != min::MISSING() )
		    parser->printer
			<< min::indent
			<< "with implied header "
			<< min::pgen ( header );

		min::uns32 master =
		    indentation_mark->
		        lexical_master;
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
	    }
	    else if ( subtype == BRA::PREFIX )
	    {
		BRA::prefix prefix = (BRA::prefix) root;

		parser->printer
		    << "prefix "
		    << min::pgen_quote
		           ( prefix->label );

		parser->printer
		    << " " << min::set_break;

		COM::print_flags
		    ( prefix->selectors,
		      PAR::COMMAND_SELECTORS,
		      parser->selector_name_table,
		      parser );

		TAB::new_flags new_selectors =
		    prefix->new_selectors;

		if ( TAB::all_flags ( new_selectors )
		     &
		     PAR::COMMAND_SELECTORS )
		{
		    parser->printer
		        << min::indent
			<< "with parsing"
			   " selectors ";
		    COM::print_new_flags
			( new_selectors,
			  PAR::COMMAND_SELECTORS,
			  parser->selector_name_table,
			  parser, true );
		}

		min::gen group = prefix->group;
		if ( group != min::MISSING() )
		    parser->printer
			<< min::indent
			<< "with group "
			<< min::pgen ( group );

		min::gen subprefix =
		    prefix->implied_subprefix;
		if ( subprefix != min::MISSING() )
		    parser->printer
			<< min::indent
			<< "with implied subprefix "
			<< min::pgen ( subprefix );

		min::uns32 master =
		    prefix->lexical_master;
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

		if ( TAB::all_flags ( new_selectors )
		     &
		     PAR::ALL_PREFIX_OPT )
		{
		    parser->printer
			<< min::indent
			<< "with parsing"
			   " options ";
		    COM::print_new_flags
			( new_selectors,
			  PAR::ALL_PREFIX_OPT,
			  parser->selector_name_table,
			  parser, true );
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

        return PAR::PRINTED;
    }

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

    if ( type == ::TYPED_BRACKET )
    {
	if ( number_of_names != 2
	     &&
	     number_of_names != 4 )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "expected 2 or 4 but not 3 names"
		  " ending with" );
	if ( number_of_names == 4
	     &&
	     name[1] != name[2] )
	{
	    // ppvec[..] is a min::ref and must be
	    // copied into a local variable before
	    // its .begin or .end can be extracted.
	    //
	    min::phrase_position pos1 = ppvec[i-3];
	    min::phrase_position pos2 = ppvec[i-2];
	    min::phrase_position pos =
	        { pos1.begin, pos2.end };
	    return PAR::parse_error
		( parser, pos,
		  "two typed middle names are not"
		  " equal" );
	}
    }

    if ( command == PAR::define ) switch ( type )
    {
    case ::BRACKET:
    {
	TAB::new_flags new_selectors;
	TAB::new_flags new_options;
	    // Inited to zeroes.
	PAR::reformatter reformatter = min::NULL_STUB;
	min::locatable_var
		< PAR::reformatter_arguments >
	    reformatter_arguments ( min::NULL_STUB );
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
			  PAR::COMMAND_SELECTORS,
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
	    if ( i + 1 < size
		 &&
		 vp[i] == PAR::parsing
		 &&
		 vp[i+1] == PAR::options )
	    {
		i += 2;
		min::gen result =
		    COM::scan_new_flags
			( vp, i, new_options,
			  PAR::ALL_BRACKET_OPT,
	                  parser->selector_name_table,
			  parser->
			    selector_group_name_table,
			  parser, true );
		if ( result == min::ERROR() )
		    return min::ERROR();
		else if ( result == min::FAILURE() )
		    return PAR::parse_error
			( parser, ppvec[i-1],
			  "expected bracketed options"
			  " (modifier) list after" );
	    }
	    else if ( i < size )
	    {
		min::uns32 j = i;
		min::locatable_gen name
		  ( PAR::scan_simple_name
			( vp, j,
			  PAR::reformatter_lexeme ) );
		if (    j < size
		     &&    vp[j]
		        == PAR::reformatter_lexeme )
		{
		    min::phrase_position position =
			{ (&ppvec[i])->begin,
			  (&ppvec[j])->end };
		    reformatter =
			PAR::find_reformatter
			    ( name,
			      BRA::reformatter_stack );
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
		    if (    reformatter_arguments
			 == min::NULL_STUB )
		    {
			if (   reformatter->
			           minimum_arguments
			     > 0 )
			    return PAR::parse_error
				    ( parser, position,
				      "reformatter"
				      " arguments"
				      " missing" );
		    }
		    else
		    {
			position.end =
			    (&ppvec[i-1])->end;

			if (   reformatter_arguments->
			           length
			     < reformatter->
				   minimum_arguments )
			    return PAR::parse_error
				    ( parser, position,
				      "too few"
				      " reformatter"
				      " arguments" );
			if (   reformatter_arguments->
			           length
			     > reformatter->
				   maximum_arguments )
			    return PAR::parse_error
				    ( parser, position,
				      "too many"
				      " reformatter"
				      " arguments" );
		    }

		    continue;
		}
	    }
	    else
		return PAR::parse_error
		    ( parser, ppvec[i-1],
		      "expected `parsing selectors',"
		      " `parsing options',"
		      " or `... reformatter ...'"
		      " after" );
	}
	if ( i < size )
	    return PAR::parse_error
		( parser, ppvec[i],
		  "expected `with' before" );

	new_selectors.or_flags |=
	    new_options.or_flags;
	new_selectors.not_flags |=
	    new_options.not_flags;
	new_selectors.xor_flags |=
	    new_options.xor_flags;

	BRA::push_brackets
	    ( name[0], name[1],
	      selectors,
	      PAR::block_level ( parser ),
	      ppvec->position,
	      new_selectors,
	      reformatter,
	      reformatter_arguments,
	      bracketed_pass->bracket_table );

	break;
    }
    case ::INDENTATION_MARK:
    {
	TAB::new_flags new_selectors;
	TAB::new_flags new_options;
	    // Inited to zeroes.
	min::uns32 lexical_master = PAR::MISSING_MASTER;
	min::locatable_gen implied_header
	    ( min::MISSING() );
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
			  PAR::COMMAND_SELECTORS,
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
	    if ( i + 1 < size
		 &&
		 vp[i] == PAR::parsing
		 &&
		 vp[i+1] == PAR::options )
	    {
		i += 2;
		min::gen result =
		    COM::scan_new_flags
			( vp, i, new_options,
			  PAR::ALL_EA_OPT,
	                  parser->selector_name_table,
			  parser->
			    selector_group_name_table,
			  parser, true );
		if ( result == min::ERROR() )
		    return min::ERROR();
		else if ( result == min::FAILURE() )
		    return PAR::parse_error
			( parser, ppvec[i-1],
			  "expected bracketed options"
			  " (modifier) list after" );
	    }
	    else
	    if ( i + 1 < size
		 &&
		 vp[i] == PAR::lexical
		 &&
		 vp[i+1] == PAR::master )
	    {
		i += 2;
		min::phrase_position position
		    = ppvec[i];
		min::locatable_gen master_name
		    ( PAR::scan_name
		        ( vp, i, parser, PAR::with ) );
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
	    if ( i + 1 < size
		 &&
		 vp[i] == PAR::implied
		 &&
		 vp[i+1] == PAR::header_lexeme )
	    {
		i += 2;
		if ( i >= size
		     ||
		     !  PAR::is_prefix_separator
		            ( vp[i] ) )
		    return PAR::parse_error
			( parser, ppvec[i-1],
			  "expected prefix separator"
			  " after" );
		implied_header = vp[i];
		++ i;
	    }
	    else
		return PAR::parse_error
		    ( parser, ppvec[i-1],
		      "expected `parsing selectors',"
		      " `parsing options', `implied"
		      " header', or `lexical master'"
		      " after" );
	}
	if ( i < size )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "expected `with' after" );

	if ( TAB::all_flags ( new_options ) == 0 )
	{
	    new_options.or_flags = PAR::DEFAULT_EA_OPT;
	    new_options.not_flags =
	            PAR::ALL_EA_OPT
		& ~ new_options.or_flags;
	}

	new_selectors.or_flags |=
	    new_options.or_flags;
	new_selectors.not_flags |=
	    new_options.not_flags;
	new_selectors.xor_flags |=
	    new_options.xor_flags;

	BRA::push_indentation_mark
	    ( name[0],
	      number_of_names == 2 ?
		  (min::gen) name[1] :
		  min::MISSING(),
	      selectors,
	      PAR::block_level ( parser ),
	      ppvec->position,
	      new_selectors,
	      implied_header,
	      lexical_master,
	      bracketed_pass->bracket_table );

	break;
    }
    case ::TYPED_BRACKET:
    {
	// Name errors are checked above.

	bool has_middle = ( number_of_names == 4 );

	TAB::new_flags new_element_selectors;
	TAB::new_flags new_attribute_selectors;
	TAB::new_flags new_options;
	    // Inited to zeroes.
	min::locatable_gen
	    attribute_begin ( min::MISSING() ),
	    attribute_equal ( min::MISSING() ),
	    attribute_separator ( min::MISSING() ),
	    attribute_negator ( min::MISSING() ),
	    attribute_flags_initiator
	    	( min::MISSING() ),
	    attribute_multivalue_initiator
	        ( min::MISSING() );
	bool prefix_separators_allowed = false;

	while ( i < size && vp[i] == PAR::with )
	{
	    ++ i;
	    if ( i + 1 < size
		 &&
		 ( vp[i] == ::element
		   ||
		   vp[i] == ::attribute )
		 &&
		 vp[i+1] == PAR::selectors )
	    {
		bool is_element =
		    ( vp[i] == ::element );
	        TAB::new_flags & new_selectors =
		    ( is_element ?
		      new_element_selectors :
		      new_attribute_selectors );
	        
		i += 2;
		min::gen result =
		    COM::scan_new_flags
			( vp, i, new_selectors,
			  PAR::COMMAND_SELECTORS,
	                  parser->selector_name_table,
			  parser->
			    selector_group_name_table,
			  parser, true );
		if ( result == min::ERROR() )
		    return min::ERROR();
		else if ( result == min::FAILURE() )
		    return PAR::parse_error
			( parser, ppvec[i-1],
			  is_element ?
			  "expected bracketed selector"
			  " modifier list after" :
			  "expected bracketed selector"
			  " list after" );
	    }
	    else
	    if ( i + 1 < size
		 &&
		 vp[i] == PAR::parsing
		 &&
		 vp[i+1] == PAR::options )
	    {
		i += 2;
		min::gen result =
		    COM::scan_new_flags
			( vp, i, new_options,
			  PAR::ALL_BRACKET_OPT,
	                  parser->selector_name_table,
			  parser->
			    selector_group_name_table,
			  parser, true );
		if ( result == min::ERROR() )
		    return min::ERROR();
		else if ( result == min::FAILURE() )
		    return PAR::parse_error
			( parser, ppvec[i-1],
			  "expected bracketed options"
			  " (modifier) list after" );
	    }
	    else
	    if ( i + 2 < size
		 &&
		 vp[i] == ::attribute
		 &&
		 ( vp[i+1] == ::flags
		   ||
		   vp[i+1] == ::multivalue )
		 &&
		 vp[i+2] == ::initiator )
	    {
		min::locatable_gen & v =
		    ( vp[i+1] == ::flags ?
		      attribute_flags_initiator :
		      attribute_multivalue_initiator );
		i += 3;
		SCAN_NAME ( v, false );
	    }
	    else
	    if ( i + 1 < size
		 &&
		 vp[i] == ::attribute
		 &&
		 vp[i+1] == ::negator )
	    {
		i += 2;

		SCAN_NAME ( attribute_negator, false );
	    }
	    else
	    if ( i < size
		 &&
		 vp[i] == ::attributes )
	    {
		i += 1;

		SCAN_NAME ( attribute_begin, false );

		if ( i >= size
		     ||
		     vp[i] != PAR::dotdotdot )
		    return PAR::parse_error
			( parser, ppvec[i-1],
			  "expected ... after" );
		i += 1;

		SCAN_NAME ( attribute_equal, false );

		if ( i >= size
		     ||
		     vp[i] != PAR::dotdotdot )
		    return PAR::parse_error
			( parser, ppvec[i-1],
			  "expected ... after" );
		i += 1;

		SCAN_NAME
		    ( attribute_separator, false );
	    }
	    else
	    if ( i + 2 < size
		 &&
		 vp[i] == PAR::prefix_lexeme
		 &&
		 vp[i+1] == ::separators
		 &&
		 vp[i+2] == ::allowed )
	    {
		i += 3;
		prefix_separators_allowed = true;
	    }
	    else return PAR::parse_error
		( parser, ppvec[i-1],
		  "expected `attributes',"
		  " or `attribute negator',"
		  " or `attribute flags initiator',"
		  " or `attribute multivalue"
		       " initiator',"
		  " or `element selectors',"
		  " or `attribute selectors',"
		  " or `parsing options',"
		  " or `prefix separators allowed'"
		  " after" );
	}
	if ( i < size )
	    return PAR::parse_error
		( parser, ppvec[i],
		  "expected `with' before" );

	if (    TAB::all_flags
	            ( new_attribute_selectors )
	     == 0 )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "expected `with attribute selectors'"
		  " after" );

	new_element_selectors.or_flags |=
	    new_options.or_flags;
	new_element_selectors.not_flags |=
	    new_options.not_flags;
	new_element_selectors.xor_flags |=
	    new_options.xor_flags;

	min::locatable_gen
	    middle ( min::MISSING() ),
	    double_middle ( min::MISSING() );

	if ( has_middle ) middle = name[1];

	if ( min::is_str ( middle ) )
	{
	    min::str_ptr middlep ( middle );
	    min::unsptr middlel =
	        min::strlen ( middlep );
	    char buffer[2*middlel+1];
	    min::strcpy ( buffer, middlep );
	    min::strcpy ( buffer + middlel, middlep );

	    if (    parser->name_scanner
	         == min::NULL_STUB )
	    {
		 LEX::init_program
		     ( name_scanner_ref ( parser ),
		       parser->scanner->program );
		 LEX::init_printer
		     ( name_scanner_ref ( parser ),
		       parser->scanner->printer );
	    }

	    LEX::init_input_string
		( name_scanner_ref ( parser ),
		  min::new_ptr ( buffer ),
		  parser->input_file->line_display );

	    double_middle =
		( LEX::scan_name_string
		    ( name_scanner_ref ( parser ),
		      PAR::QUOTED_KEY_SCAN_MASK,
		      PAR::IGNORED_SCAN_MASK,
		      PAR::END_SCAN_MASK,
		      false ) );

	    if ( ! min::is_str ( double_middle ) )
	        double_middle = min::MISSING();
	}

	BRA::push_typed_brackets
	    ( name[0],
	      middle,
	      double_middle,
	      name[1+2*has_middle],
	      selectors,
	      PAR::block_level ( parser ),
	      ppvec->position,
	      new_element_selectors,
	      new_attribute_selectors.or_flags,
	      attribute_begin,
	      attribute_equal,
	      attribute_separator,
	      attribute_negator,
	      attribute_flags_initiator,
	      min::standard_attr_flag_parser,
	      attribute_multivalue_initiator,
	      prefix_separators_allowed,
	      bracketed_pass->bracket_table );

	break;
    }
    case ::PREFIX:
    {
	TAB::new_flags new_selectors;
	TAB::new_flags new_options;
	min::locatable_gen group ( min::MISSING() );
	min::locatable_gen implied_subprefix
	    ( min::MISSING() );
	min::locatable_gen implied_subprefix_type
	    ( min::MISSING() );
	min::uns32 lexical_master = PAR::MISSING_MASTER;

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
			  PAR::COMMAND_SELECTORS,
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
	    if ( i + 1 < size
		 &&
		 vp[i] == PAR::parsing
		 &&
		 vp[i+1] == PAR::options )
	    {
		i += 2;
		min::gen result =
		    COM::scan_new_flags
			( vp, i, new_options,
			  PAR::ALL_PREFIX_OPT,
	                  parser->selector_name_table,
			  parser->
			    selector_group_name_table,
			  parser, true );
		if ( result == min::ERROR() )
		    return min::ERROR();
		else if ( result == min::FAILURE() )
		    return PAR::parse_error
			( parser, ppvec[i-1],
			  "expected bracketed options"
			  " (modifier) list after" );
	    }
	    else
	    if ( vp[i] == PAR::group )
	    {
		++ i;
		min::phrase_position position
		    = ppvec[i];
		group = PAR::scan_name
		          ( vp, i, parser, PAR::with );
		if ( group == min::ERROR() )
		    return min::ERROR();
		position.end = (& ppvec[i-1])->end;
	    }
	    else
	    if ( i + 1 < size
		 &&
		 vp[i] == PAR::implied
		 &&
		 vp[i+1] == PAR::subprefix )
	    {
		i += 2;
		if ( i >= size
		     ||
		     !  PAR::is_prefix_separator
		            ( vp[i] ) )
		    return PAR::parse_error
			( parser, ppvec[i-1],
			  "expected prefix separator"
			  " after" );
		implied_subprefix = vp[i];
		if ( ! min::is_obj
		           ( implied_subprefix ) )
		    return PAR::parse_error
			( parser, ppvec[i],
			  "implied subprefix should be"
			  " prefix separator" );
		implied_subprefix_type =
		    min::get ( implied_subprefix,
			       min::dot_type );
		if ( ! min::is_name
		           ( implied_subprefix_type ) )
		    return PAR::parse_error
			( parser, ppvec[i],
			  "implied subprefix should be"
			  " prefix separator" );
		++ i;
	    }
	    else
	    if ( i + 1 < size
		 &&
		 vp[i] == PAR::lexical
		 &&
		 vp[i+1] == PAR::master )
	    {
		i += 2;
		min::phrase_position position
		    = ppvec[i];
		min::locatable_gen master_name
		    ( PAR::scan_name
		        ( vp, i, parser, PAR::with ) );
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
		      "expected `parsing selectors',"
		      " `parsing options',"
		      " `parsing options',"
		      " `group',"
		      " `implied subprefix',"
		      " or `lexical master' after" );
	}
	if ( i < size )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "unexpected stuff after" );

	if ( group == PAR::paragraph_lexeme )
	{
	    new_selectors.or_flags |=
		new_options.or_flags;
	    new_selectors.not_flags |=
		new_options.not_flags;
	    new_selectors.xor_flags |=
		new_options.xor_flags;
	}
	else if ( TAB::all_flags ( new_options ) != 0 )
	    PAR::parse_error
		( parser, ppvec->position,
		  "`with options' not allowed for"
		  " prefix unless prefix group is"
		  " `paragraph'; options ignored" );

	BRA::push_prefix
	    ( name[0], selectors,
	      PAR::block_level ( parser ),
	      ppvec->position,
	      new_selectors,
	      group,
	      implied_subprefix,
	      implied_subprefix_type,
	      lexical_master,
	      bracketed_pass->prefix_table );
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
		     type == ::PREFIX ? 
		  bracketed_pass->prefix_table :
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
	    case ::TYPED_BRACKET:
	    {
		if ( subtype != BRA::TYPED_OPENING )
		    continue;

		BRA::typed_opening typed_opening =
		    (BRA::typed_opening) root;
		BRA::typed_middle typed_middle =
		    typed_opening->typed_middle;
		BRA::closing_bracket typed_closing =
		    typed_opening->closing_bracket;

		if ( number_of_names == 4 )
		{
		    if ( typed_middle->label != name[1]
		         ||
			 typed_closing->label != name[3]
		       )
		        continue;
		}
		else
		{
		    if ( typed_middle != min::NULL_STUB
		         ||
			 typed_closing->label != name[1]
		       )
		        continue;
		}

		break;
	    }
	    case ::PREFIX:
	    {
		// Here to suppress warning message.
		//
	        break;
	    }
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
