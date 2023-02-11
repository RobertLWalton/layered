// Layered Languages Bracketed Subexpression Parser
//
// File:	ll_parser_bracketed.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Feb 11 04:08:51 EST 2023
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
//	Bracket Type Table
//	Bracketed Subexpression Pass
//	Bracketed Subexpression Parser Functions
//	Parse Bracketed Subexpression Function
//	Bracketed Compact Functions
//	Untyped Bracketed Reformatters
//	Typed Bracketed Reformatters
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
# define PARLEX ll::parser::lexeme
# define TAB ll::parser::table
# define COM ll::parser::command
# define BRA ll::parser::bracketed

static bool initialize_called = false;
static min::locatable_gen bracket;
static min::locatable_gen type;
static min::locatable_gen indentation;
static min::locatable_gen typed;
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
static min::locatable_gen quoted_lexeme;
static min::locatable_gen string_lexeme;
static min::locatable_gen concatenator;
static min::locatable_gen middle_lexeme;
static min::locatable_gen break_lexeme;
static min::locatable_gen top;
static min::locatable_gen SPECIAL;

static void initialize ( void )
{
    if ( initialize_called ) return;
    initialize_called = true;

    ::bracket = min::new_str_gen ( "bracket" );
    ::type = min::new_str_gen ( "type" );
    ::indentation = min::new_str_gen
			    ( "indentation" );
    ::typed = min::new_str_gen ( "typed" );
    ::attributes = min::new_str_gen ( "attributes" );
    ::attribute = min::new_str_gen ( "attribute" );
    ::flags = min::new_str_gen ( "flags" );
    ::multivalue = min::new_str_gen ( "multivalue" );
    ::initiator = min::new_str_gen ( "initiator" );
    ::negator = min::new_str_gen ( "negator" );
    ::separator = min::new_str_gen ( "separator" );
    ::separators = min::new_str_gen ( "separators" );
    ::mark = min::new_str_gen ( "mark" );
    ::full = min::new_str_gen ( "full" );
    ::lines = min::new_str_gen ( "lines" );
    ::bracketed_subexpressions =
        min::new_lab_gen
	    ( "bracketed", "subexpressions" );
    ::offset = min::new_str_gen ( "offset" );
    ::quoted_lexeme = min::new_str_gen ( "quoted" );
    ::string_lexeme = min::new_str_gen ( "string" );
    ::concatenator =
        min::new_str_gen ( "concatenator" );
    ::middle_lexeme = min::new_str_gen ( "middle" );
    ::break_lexeme = min::new_str_gen ( "break" );
    ::top = min::new_str_gen ( "top" );
    ::SPECIAL = min::new_str_gen ( "SPECIAL" );
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
    min::DISP_END };

min::uns32 opening_bracket_gen_disp[] = {
    min::DISP ( & BRA::opening_bracket_struct
                     ::label ),
    min::DISP ( & BRA::opening_bracket_struct
                     ::reformatter_arguments ),
    min::DISP_END };

static min::packed_struct_with_base
	<BRA::opening_bracket_struct, TAB::root_struct>
    opening_bracket_type
	( "ll::parser::table::opening_bracket_type",
	  ::opening_bracket_gen_disp,
	  ::opening_bracket_stub_disp );
const min::uns32 & BRA::OPENING_BRACKET =
    opening_bracket_type.subtype;

static min::uns32 closing_bracket_stub_disp[] = {
    min::DISP ( & BRA::closing_bracket_struct::next ),
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
	  const TAB::new_flags & parsing_selectors,
	  PAR::reformatter reformatter,
	  min::gen reformatter_arguments,
	  TAB::key_table bracket_table )
{
    MIN_REQUIRE
        ( (   TAB::all_flags ( parsing_selectors )
	    & PAR::ALWAYS_SELECTOR ) == 0 );

    min::locatable_var<BRA::opening_bracket> opening
        ( ::opening_bracket_type.new_stub() );
    min::locatable_var<BRA::closing_bracket> closing
        ( PAR::find
	    ( closing_bracket, ::closing_bracket_type,
	      block_level, position, bracket_table ) );

    label_ref(opening) = opening_bracket;
    closing_bracket_ref(opening) = closing;
    opening->selectors = selectors;
    opening->block_level = block_level;
    opening->position = position;

    opening->parsing_selectors = parsing_selectors;
    TAB::flags off_flags =
          BRA::BRACKET_OFF_SELECTORS
	+ BRA::BRACKET_OFF_OPT;
    off_flags &= ~ opening->parsing_selectors.or_flags;
    off_flags &= ~ opening->parsing_selectors.xor_flags;
    opening->parsing_selectors.not_flags |= off_flags;

    reformatter_ref(opening) = reformatter;
    reformatter_arguments_ref(opening) =
        reformatter_arguments;

    TAB::push ( bracket_table, (TAB::root) opening );

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
    min::DISP_END };

static min::packed_struct_with_base
	<BRA::line_sep_struct, TAB::root_struct>
    line_sep_type
	( "ll::parser::table::line_sep_type",
	  TAB::root_gen_disp,
	  ::line_sep_stub_disp );
const min::uns32 & BRA::LINE_SEP =
    line_sep_type.subtype;

BRA::line_sep
    BRA::push_line_sep
	( min::gen line_sep_label,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  TAB::key_table bracket_table )
{
    min::locatable_var<BRA::line_sep>
	line_sep
	( PAR::find ( line_sep_label,
		      ::line_sep_type,
		      block_level,
		      position,
		      bracket_table ) );
    return line_sep;
}

BRA::indentation_mark
    BRA::push_indentation_mark
	( min::gen mark_label,
	  min::gen separator_label,
	  TAB::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  const TAB::new_flags & parsing_selectors,
	  min::gen implied_header,
	  min::uns32 paragraph_lexical_master,
	  min::uns32 line_lexical_master,
	  TAB::key_table bracket_table )
{
    MIN_REQUIRE
        ( (   TAB::all_flags ( parsing_selectors )
	    & PAR::ALWAYS_SELECTOR ) == 0 );

    min::locatable_var<BRA::indentation_mark> imark
        ( ::indentation_mark_type.new_stub() );
    label_ref(imark) = mark_label;
    imark->selectors = selectors;
    imark->block_level = block_level;
    imark->position = position;
    imark->parsing_selectors = parsing_selectors;
    TAB::flags off_flags =
          BRA::INDENTATION_MARK_OFF_SELECTORS;
    off_flags &= ~ imark->parsing_selectors.or_flags;
    off_flags &= ~ imark->parsing_selectors.xor_flags;
    imark->parsing_selectors.not_flags |= off_flags;
    TAB::flags on_flags =
          BRA::INDENTATION_MARK_ON_SELECTORS;
    on_flags &= ~ imark->parsing_selectors.not_flags;
    on_flags &= ~ imark->parsing_selectors.xor_flags;
    imark->parsing_selectors.or_flags |= on_flags;

    implied_header_ref(imark) = implied_header;
    implied_header_type_ref(imark) =
        min::get ( implied_header, min::dot_type );
    imark->paragraph_lexical_master =
        paragraph_lexical_master;
    imark->line_lexical_master = line_lexical_master;
    TAB::push ( bracket_table, (TAB::root) imark );

    if ( separator_label != min::MISSING() )
	line_sep_ref(imark) =
	    BRA::push_line_sep
	        ( separator_label,
		  block_level,
		  position,
		  bracket_table );
    else
        line_sep_ref(imark) = min::NULL_STUB;

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
    min::DISP ( & BRA::typed_opening_struct
                     ::reformatter_arguments ),
    min::DISP_END };

static min::uns32 typed_opening_stub_disp[] = {
    min::DISP ( & BRA::typed_opening_struct::next ),
    min::DISP ( & BRA::typed_opening_struct
                     ::closing_bracket ),
    min::DISP ( & BRA::typed_opening_struct
                     ::reformatter ),
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
	  const TAB::new_flags & parsing_selectors,
	  TAB::flags attr_selectors,
	  min::gen typed_attr_begin,
	  min::gen typed_attr_equal,
	  min::gen typed_attr_sep,
	  min::gen typed_attr_negator,
	  min::gen typed_attr_flags_initiator,
	  const min::flag_parser *
	           typed_attr_flag_parser,
	  min::gen typed_attr_multivalue_initiator,
	  TAB::flags prefix_selectors,
	  TAB::key_table bracket_table )
{
    MIN_REQUIRE
        ( (   TAB::all_flags ( parsing_selectors )
	    & PAR::ALWAYS_SELECTOR ) == 0 );

    min::locatable_var<BRA::typed_opening> opening
        ( ::typed_opening_type.new_stub() );
    min::locatable_var<BRA::closing_bracket> closing
        ( PAR::find
	    ( typed_closing, ::closing_bracket_type,
	      block_level, position, bracket_table ) );

    label_ref(opening) = typed_opening;
    opening->selectors = selectors;
    opening->block_level = block_level;
    opening->position = position;
    closing_bracket_ref(opening) = closing;

    opening->parsing_selectors = parsing_selectors;
    TAB::flags off_flags =
            BRA::BRACKET_OFF_SELECTORS
	  + BRA::BRACKET_OFF_OPT;
    off_flags &= ~ opening->parsing_selectors.or_flags;
    off_flags &= ~ opening->parsing_selectors.xor_flags;
    opening->parsing_selectors.not_flags |= off_flags;

    opening->attr_selectors = attr_selectors;
    opening->attr_selectors &= ~
        (   BRA::BRACKET_OFF_SELECTORS
	  + PAR::ALL_OPT );

    reformatter_ref(opening) = min::NULL_STUB;
    reformatter_arguments_ref(opening) = min::MISSING();

    TAB::push ( bracket_table, (TAB::root) opening );

    min::locatable_var<BRA::typed_middle> middle
        ( PAR::find
	    ( typed_middle, ::typed_middle_type,
	      block_level, position, bracket_table ) );

    typed_middle_ref(opening)  = middle;

    if ( typed_double_middle != min::MISSING() )
    {
	min::locatable_var<BRA::typed_double_middle>
		double_middle
	    ( PAR::find
		( typed_double_middle,
		  ::typed_double_middle_type,
		  block_level, position,
		  bracket_table ) );

	typed_double_middle_ref(opening)
	    = double_middle;
    }

    if ( typed_attr_begin != min::MISSING() )
    {
	min::locatable_var
		<BRA::typed_attr_begin>
	    attr_begin
		( PAR::find
		    ( typed_attr_begin,
		      ::typed_attr_begin_type,
		      block_level, position,
		      bracket_table ) );
	min::locatable_var
		<BRA::typed_attr_equal>
	    attr_equal
		( PAR::find
		    ( typed_attr_equal,
		      ::typed_attr_equal_type,
		      block_level, position,
		      bracket_table ) );
	min::locatable_var
		<BRA::typed_attr_sep>
	    attr_sep
		( PAR::find
		    ( typed_attr_sep,
		      ::typed_attr_sep_type,
		      block_level, position,
		      bracket_table ) );

	typed_attr_begin_ref(opening)
	    = attr_begin;
	typed_attr_equal_ref(opening)
	    = attr_equal;
	typed_attr_sep_ref(opening)
	    = attr_sep;
    }

    if ( typed_attr_negator != min::MISSING() )
    {
	min::locatable_var
		<BRA::typed_attr_negator>
	    attr_negator
		( PAR::find
		    ( typed_attr_negator,
		      ::typed_attr_negator_type,
		      block_level, position,
		      bracket_table ) );

	typed_attr_negator_ref(opening)
	    = attr_negator;
    }

    typed_attr_flags_initiator_ref(opening) =
        typed_attr_flags_initiator;
    opening->typed_attr_flag_parser =
    	typed_attr_flag_parser;
    typed_attr_multivalue_initiator_ref(opening) =
        typed_attr_multivalue_initiator;

    opening->prefix_selectors = prefix_selectors;

    return opening;
}

// Bracket Type Table
// ------- ---- -----

static min::uns32 bracket_type_gen_disp[] = {
    min::DISP ( & BRA::bracket_type_struct::label ),
    min::DISP ( & BRA::bracket_type_struct::group ),
    min::DISP ( & BRA::bracket_type_struct
                     ::implied_subprefix ),
    min::DISP ( & BRA::bracket_type_struct
                     ::implied_subprefix_type ),
    min::DISP ( & BRA::bracket_type_struct
                     ::reformatter_arguments ),
    min::DISP_END };

static min::uns32 bracket_type_stub_disp[] = {
    min::DISP ( & BRA::bracket_type_struct::next ),
    min::DISP ( & BRA::bracket_type_struct
                     ::reformatter ),
    min::DISP_END };

static min::packed_struct_with_base
	<BRA::bracket_type_struct, TAB::root_struct>
    bracket_type_type
        ( "ll::parser::bracketed::bracket_type_type",
	  ::bracket_type_gen_disp,
	  ::bracket_type_stub_disp );
const min::uns32 & BRA::BRACKET_TYPE =
    ::bracket_type_type.subtype;

void BRA::push_bracket_type
	( min::gen bracket_type_label,
	  TAB::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  TAB::new_flags parsing_selectors,
	  min::gen group,
	  min::gen implied_subprefix,
	  min::gen implied_subprefix_type,
	  min::uns32 line_lexical_master,
	  ll::parser::reformatter reformatter,
	  min::gen reformatter_arguments,
	  TAB::key_table bracket_type_table )
{
    MIN_REQUIRE
        ( (   TAB::all_flags ( parsing_selectors )
	    & PAR::ALWAYS_SELECTOR ) == 0 );

    min::locatable_var<BRA::bracket_type> bracket_type
        ( ::bracket_type_type.new_stub() );

    label_ref(bracket_type) = bracket_type_label;
    bracket_type->selectors = selectors;
    bracket_type->block_level = block_level;
    bracket_type->position = position;
    bracket_type->parsing_selectors = parsing_selectors;

    group_ref(bracket_type) = group;
    implied_subprefix_ref(bracket_type) =
        implied_subprefix;
    implied_subprefix_type_ref(bracket_type) =
        implied_subprefix_type;
    bracket_type->line_lexical_master =
        line_lexical_master;
    reformatter_ref(bracket_type) = reformatter;
    reformatter_arguments_ref(bracket_type) =
        reformatter_arguments;

    TAB::push ( bracket_type_table,
                (TAB::root) bracket_type );
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
                     ::bracket_type_table ),
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
    TAB::key_table bracket_type_table =
        bracketed_pass->bracket_type_table;

    min::uns64 collected_entries,
               collected_key_prefixes;

    TAB::end_block
        ( bracket_table, 0,
	  collected_key_prefixes, collected_entries );
    TAB::end_block
        ( bracket_type_table, 0,
	  collected_key_prefixes, collected_entries );
    min::pop ( bracketed_pass->block_stack,
	       bracketed_pass->block_stack->length );
    bracketed_pass->indentation_offset = 2;
    BRA::string_concatenator_ref ( bracketed_pass ) =
        min::DISABLED();
    BRA::middle_break mb = { "", "", 0, 0 };
    bracketed_pass->middle_break = mb;
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
	  bracketed_pass->string_concatenator,
	  bracketed_pass->middle_break );

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
    TAB::key_table bracket_type_table =
        bracketed_pass->bracket_type_table;

    min::uns64 collected_entries,
               collected_key_prefixes;

    min::uns32 block_level =
        PAR::block_level ( parser );
    MIN_REQUIRE ( block_level > 0 );

    TAB::end_block
        ( bracket_table, block_level - 1,
	  collected_key_prefixes, collected_entries );
    TAB::end_block
        ( bracket_type_table, block_level - 1,
	  collected_key_prefixes, collected_entries );

    BRA::block_struct b =
        min::pop ( bracketed_pass->block_stack );
    bracketed_pass->indentation_offset =
        b.indentation_offset;
    string_concatenator_ref ( bracketed_pass ) =
        b.string_concatenator;
    bracketed_pass->middle_break = b.middle_break;

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
    bracket_type_table_ref(bracketed_pass) =
	TAB::create_key_table ( 1024 );
    BRA::init_block_stack
        ( BRA::block_stack_ref(bracketed_pass), 16 );
    bracketed_pass->indentation_offset = 2;
    BRA::string_concatenator_ref ( bracketed_pass ) =
        min::DISABLED();
    BRA::middle_break mb = { "", "", 0, 0 };
    bracketed_pass->middle_break = mb;

    return (PAR::pass) bracketed_pass;
}

// Bracketed Subexpression Parser Functions
// --------- ------------- ------ ---------

const min::position BRA::ISOLATED_HEADER =
    { 0xFFFFFFFF, 1 };

void BRA::init_line_variables
	( BRA::line_variables & line_variables,
	  BRA::indentation_mark indentation_mark,
	  PAR::parser parser,
	  TAB::flags selectors,
	  min::int32 indent,
	  PAR::token & current )
{
    BRA::line_data & paragraph_data =
	line_variables
	    .indentation_paragraph;
    BRA::line_data & implied_data =
	line_variables
	    .indentation_implied_paragraph;

    paragraph_data.paragraph_lexical_master =
    implied_data.paragraph_lexical_master =
	indentation_mark->
	    paragraph_lexical_master;
    paragraph_data.line_lexical_master =
    implied_data.line_lexical_master =
	indentation_mark->line_lexical_master;
    paragraph_data.selectors =
    implied_data.selectors =
	    selectors;
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
	indentation_mark->implied_header;
    min::gen implied_header_type =
	indentation_mark->implied_header_type;
    TAB::flags header_selectors = selectors;
    bool first_time = true;
    while ( implied_header != min::MISSING() )
    {
	BRA::bracketed_pass bracketed_pass =
	    (BRA::bracketed_pass) parser->pass_stack;
	TAB::key_table bracket_type_table =
	    bracketed_pass->bracket_type_table;
	BRA::bracket_type header_entry =
	    (BRA::bracket_type)
	    TAB::find ( implied_header_type,
			BRA::BRACKET_TYPE,
			header_selectors,
			bracket_type_table );
	min::gen group = implied_header_type;
	if ( header_entry != min::NULL_STUB )
	{
	    if ( header_entry->group != min::MISSING() )
		group = header_entry->group;
	    header_selectors =
		TAB::modified_flags
		    ( header_selectors,
		      header_entry->parsing_selectors );
	}

	if ( group == PARLEX::paragraph && first_time )
	{
	    if (    header_entry->line_lexical_master
		 != PAR::MISSING_MASTER )
	    {
		min::phrase_position pos =
		    { current->position.end,
		      current->position.end };
		PAR::parse_error
		  ( parser,
		    pos,
		    "indentation mark implied"
		    " header of type `",
		    min::pgen_never_quote
		      ( implied_header_type
		      ),
		    "' cannot be isolated;"
		    " implied header ignored" );
		break;
	    }

	    paragraph_data.implied_header =
		implied_header;
	    paragraph_data.header_entry =
		header_entry;
	    paragraph_data.header_selectors
		= header_selectors;

	    first_time = false;
	    implied_header =
		header_entry->
		    implied_subprefix;
	    implied_header_type =
		header_entry->implied_subprefix_type;
	}
	else if ( group == PARLEX::line )
	{
	    implied_data.implied_header =
		    implied_header;
	    implied_data.header_entry =
		    header_entry;
	    implied_data.header_selectors =
		    header_selectors;

	    if ( first_time )
	    {
	        paragraph_data.implied_header =
		  implied_data.implied_header;
	        paragraph_data.header_entry =
		  implied_data.header_entry;
	        paragraph_data.header_selectors =
		  implied_data.header_selectors;
	    }
	    break;
	}
	else if ( first_time )
	{
	    min::phrase_position pos =
		{ current->position.end,
		  current->position.end };
	    PAR::parse_error
	      ( parser,
		pos,
		"indentation mark implied"
		" header of type `",
		min::pgen_never_quote
		  ( implied_header_type
		  ),
		"' does not have"
		" `paragraph' or `line'"
		" group; cannot begin"
		" indented lines;"
		" ignored" );
	    break;
	}
	else
	{
	    // Implied subprefix of paragraph
	    // or line header is OK.
	    // Do nothing with it.
	    //
	    break;
	}
    }

    line_variables.paragraph =
	line_variables.indentation_paragraph;
    line_variables.implied_paragraph =
	line_variables.indentation_implied_paragraph;

    line_variables.paragraph_indent = indent;
    line_variables.line_sep =
	indentation_mark->line_sep;
    line_variables.at_paragraph_end = false;
    line_variables.last_paragraph = min::NULL_STUB;
    line_variables.current.selectors &= ~
	PAR::CONTINUING_OPT;
	// line_variables.current.selectors
	// is replaced by line_variables.
	// paragraph.selectors at beginning
	// of parse_paragraph_element loop.
}

bool BRA::parse_paragraph_element
	( PAR::parser parser,
	  PAR::token & current,
	  BRA::line_variables * line_variables,
	  TAB::flags trace_flags )
{
    // Special case of line with paragraph header that
    // ended previous paragraph.
    //
    if (    line_variables->last_paragraph
         != min::NULL_STUB )
    {
	MIN_REQUIRE
	  ( current == line_variables->last_paragraph );
	MIN_REQUIRE
	  (    current->value_type
	    == PARLEX::paragraph );

	MIN_REQUIRE ( current->next != parser->first );
        current = current->next;

	if ( ( parser->at_paragraph_beginning
	       &&
	       ! ( line_variables->
			current.selectors
	           &
	           PAR::CONTINUING_OPT ) )
	      ||
	      line_variables->at_paragraph_end )
	{
	    line_variables->last_paragraph =
	        min::NULL_STUB;
	    return false;
	}
    }
    else if ( line_variables->at_paragraph_end )
        return false;
 
    BRA::bracketed_pass bracketed_pass =
        (BRA::bracketed_pass) parser->pass_stack;
    min::int32 indentation_offset =
	bracketed_pass->indentation_offset;

    while ( true )
    {
	if ( parser->at_paragraph_beginning
	     &&
	     ! ( line_variables->current.selectors
		 &
		 PAR::CONTINUING_OPT ) )
	    line_variables->current =
	        line_variables->paragraph;

	line_variables->at_paragraph_indent = false;
	if ( current->type == LEXSTD::indent_t )
	{
	    MIN_REQUIRE
	        ( current->next == parser->first );

	    min::uns32 lexical_master =
	        ( parser->at_paragraph_beginning ?
	          line_variables->
		      current.paragraph_lexical_master :
	          line_variables->
		      current.line_lexical_master );
	    if ( lexical_master != PAR::MISSING_MASTER )
		PAR::set_lexical_master
		    ( lexical_master, parser );

	    // This ensure_next MUST be after the
	    // lexical masters are reset.
	    //
	    PAR::ensure_next ( parser, current );
	    if (    current->indent
		 == line_variables->paragraph_indent )
		line_variables->at_paragraph_indent =
		    true;
	    else
	    if (   current->indent
		 > line_variables->paragraph_indent )
	    {
		min::phrase_position pos =
		    { current->next->position.begin,
		      current->next->position.begin };
		PAR::parse_warn
		    ( parser, pos,
		      "logical line begins at an indent"
		      " that is greater than the"
		      " current paragraph indent" );
	    }

	    current = current->next;
	    PAR::free
		( PAR::remove ( first_ref(parser),
				current->previous ) );
	}

	// Get subexpression.  First is the first token
	// of the subexpression.

	TAB::flags selectors =
	    line_variables->current.selectors;

	bool maybe_parser_command = 
	    ( current->value == PARLEX::star_parser
	      &&
	          line_variables->last_paragraph
	       == min::NULL_STUB );
	    // An optimization.
	min::position separator_found =
	    BRA::parse_bracketed_subexpression
		( parser, selectors,
		  current,
		  NULL,
		  line_variables );

	PAR::token first =
	    line_variables->previous->next;

	// Here to handle isolated header.
	//
	if ( separator_found == BRA::ISOLATED_HEADER )
	{
	    MIN_REQUIRE ( first->next == current );

	    if ( first->value_type == PARLEX::reset )
	    {
	        // Reset line variables as per reset
		// header.
		//
		line_variables->paragraph =
		    line_variables->
		        indentation_paragraph;
		line_variables->implied_paragraph =
		    line_variables->
			indentation_implied_paragraph;
		line_variables->current =
		    line_variables->paragraph;
	    }

	    // Remove following tokens till end of
	    // logical line, ignoring line separators.
	    // If non-comment, non-indent, non-line-
	    // break removed, announce error.
	    //
	    min::phrase_position bad_phrase =
		{ min::MISSING_POSITION,
		  min::MISSING_POSITION };
	    min::uns32 previous_t = 0;
	    TAB::flags selectors =
	        line_variables->current.selectors;
	    while ( true )
	    {
		if (    current->type
		     == LEXSTD::indent_t )
		{

		    int32 rindent =
		        PAR::relative_indent
		            ( parser,
			      indentation_offset,
			      current,
			      line_variables->
				paragraph_indent );

		    if ( rindent < 0 )
			line_variables->
			    at_paragraph_end = true;

		    if ( selectors & PAR::EAINDENT_OPT )
		        break;

		    if ( ( parser->
		               at_paragraph_beginning
			   &&
			      line_variables->previous
			                    ->next
			   != current )
			 &&
			 (   selectors
			   & PAR::EAPBREAK_OPT ) )
			    break;

	            if (     ( selectors
		               & PAR::EALEINDENT_OPT )
		          && rindent <= 0 )
		        break;

	            if (     ( selectors
		               & PAR::EALTINDENT_OPT )
		          && rindent < 0 )
		        break;
		}
		else if (    current->type
		          == LEXSTD::end_of_file_t )
		{
		    line_variables->at_paragraph_end =
		    parser->at_paragraph_beginning =
		        true;
		    break;
		}
		else if (    current->type
		          == LEXSTD::line_break_t )
		{
		    if (    previous_t
		         == LEXSTD::line_break_t )
			parser->at_paragraph_beginning =
			    true;
		}
		else if (    current->type
		          != LEXSTD::comment_t )
		{
		    parser->at_paragraph_beginning =
			false;
		    if ( ! bad_phrase.begin )
		        bad_phrase = current->position;
		    else
			bad_phrase.end =
			    current->position.end;
		}

		previous_t = current->type;
		PAR::ensure_next ( parser, current );
		current = current->next;
		PAR::free
		    ( PAR::remove
			( first_ref(parser),
			  current->previous ) );
	    }

	    if ( bad_phrase.begin )
		PAR::parse_error
		    ( parser, bad_phrase,
		      "non-comments after isolated"
		      " header; ignored" );

	    if ( first->value_type == PARLEX::reset )
	    {
		// Remove reset header.
		//
		PAR::free
		    ( PAR::remove
			  ( first_ref(parser),
			    current->previous ) );

		parser->at_paragraph_beginning = true;

		if (    line_variables->last_paragraph
		     == min::NULL_STUB )
		    continue;
		else
		{
		    BRA::compact_paragraph
			( parser,
			  line_variables->
			      last_paragraph,
			  current,
			  trace_flags );
		    line_variables->last_paragraph
			= min::NULL_STUB;
		    return false;
		}
	    }

	    // If not reset header, but instead
	    // isolated paragraph header, fall through.
	}

	line_variables->at_paragraph_end =
	    ( current->type == LEXSTD::end_of_file_t
	      ||
	      ( current->type == LEXSTD::indent_t
		&&
		  PAR::relative_indent
		      ( parser,
			indentation_offset,
			current,
			line_variables->
			    paragraph_indent )
		< 0 ) );

	// Here to handle line separator that super-
	// fluously ends a logical line and comment
	// line that becomes an empty logical line.
	//
	if ( first == current )
	{
	    if ( ! line_variables->at_paragraph_end )
	        continue;
	    else
	    if (    line_variables->last_paragraph
		 != min::NULL_STUB )
	    {
		BRA::compact_paragraph
		    ( parser,
		      line_variables->last_paragraph,
		      current,
		      trace_flags );
		line_variables->last_paragraph
		    = min::NULL_STUB;
	    }
	    return false;
	}

        // If subexpression is not a single element
	// subexpression whose one element has prefix
	// group `paragraph' or `line', compact it as
	// a logical line.
	//
	if ( first->next != current
	     ||
	     (    first->value_type
	       != PARLEX::paragraph
	       &&
		  first->value_type
	       != PARLEX::line ) )
	{
	    BRA::compact_logical_line
		( parser, parser->pass_stack->next,
	          selectors,
		  first, current,
		  separator_found,
	          (TAB::root)
		  line_variables->line_sep,
		  trace_flags );
	}

	// Compact prefix paragraph if necessary.
	//
	if (    first->value_type
	     == PARLEX::paragraph )
	{
	    if ( line_variables->last_paragraph
		 != min::NULL_STUB )
	    {
		BRA::compact_paragraph
		    ( parser,
		      line_variables->last_paragraph,
		      first,
		      trace_flags );
		MIN_REQUIRE ( first->next == current );
		current = first;
		line_variables->last_paragraph =
		    first;
		return false;
	    }

	    if ( ( parser->at_paragraph_beginning
		   &&
		   ! ( line_variables->current.selectors
		       &
		       PAR::CONTINUING_OPT ) )
		 ||
		 line_variables->at_paragraph_end )
		return false;
	    else
		line_variables->last_paragraph = first;
	}
	else if (    line_variables->last_paragraph
	          == min::NULL_STUB )
	    return maybe_parser_command;
	else if ( ( parser->at_paragraph_beginning
		    &&
		    ! ( line_variables->
		            current.selectors
		        &
		        PAR::CONTINUING_OPT ) )
		  ||
		  line_variables->at_paragraph_end )
	{
	    BRA::compact_paragraph
		( parser,
		  line_variables->last_paragraph,
		  current,
		  trace_flags );
	    line_variables->last_paragraph
		= min::NULL_STUB;
	    return false;
	}
    }
}

// Parse Bracketed Subexpression Function
// ----- --------- ------------- --------

// See ll_parser_bracketed_parse.outline for an outline
// of the parse_bracketed_subexpression function.

// The following are all static or inline; if any are
// useful elsewhere they can be put in the PAR
// namespace.

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

// Make type label.  If no label tokens, print missing
// type error message and assume label is "".  If label
// computes as min::empty_lab, print error message and
// replace label with "" (i.e., min::empty_str).
//
// If typed_data->type is MISSING, then if the new label
// is not min::empty_str, set new label token type to
// TYPE, increment typed_data->attr_count, and move TYPE
// token to before typed_data->elements if that is not
// NULL_STUB.  If on the other hand a new label token
// for type min::empty_str was produced, delete that
// token.  In any case set typed_data->type to the new
// label.
//
// If typed_data->type is NOT MISSING, check that it
// equals new label, and print error message if not.
// Then whether equal or not, delete any new label
// token.
//
inline void make_type_label
	( PAR::parser parser,
	  BRA::typed_data * typed_data,
	  PAR::token next )
{
    min::gen type = min::empty_str;
    PAR::token start = typed_data->start_previous->next;
    min::phrase_position pos = start->position;
    if ( start == next )
    {
	::missing_error
	    ( parser, next, "type; \"\" assumed" );
	start = min::NULL_STUB;
    }
    else
    {
        PAR::make_label_or_value
	    ( parser, start, next, PAR::LABEL_MODE );
	type = start->value;
	if ( type == min::NONE() )
	{
	    type = min::empty_str;
	    PAR::parse_error
		( parser, pos,
		  "bad type label; \"\" assumed" );
	}
    }

    if ( typed_data->type != min::MISSING() )
    {
        if ( typed_data->type != type )
	    PAR::parse_error
		( parser, pos,
		  "beginning type `",
		  min::pgen_never_quote
		    ( typed_data->type ),
		  "' != end type `",
		  min::pgen_never_quote
		    ( type ),
		  "'; end type ignored"
		);
    }
    else // if ( typed_data->type == min::MISSING() )
    {
	typed_data->type = type;

        if ( type != min::empty_str )
	{
	    start->type = BRA::TYPE;
	    ++ typed_data->attr_count;

	    if (    typed_data->elements
	         != min::NULL_STUB )
	    {
		typed_data->end_position =
		    next->previous->position.end;
		PAR::move_to_before
		    ( parser, typed_data->elements,
		      next->previous, next );
	    }
	    start = min::NULL_STUB;
	        // To prevent deletion of start.

	    BRA::bracketed_pass bracketed_pass =
		(BRA::bracketed_pass)
		parser->pass_stack;
	    TAB::key_table bracket_type_table =
		bracketed_pass->bracket_type_table;
	    BRA::bracket_type bracket_type =
		(BRA::bracket_type)
		TAB::find
		    ( type,
		      BRA::BRACKET_TYPE,
		      typed_data->context_selectors,
		      bracket_type_table );
	    if ( bracket_type != min::NULL_STUB )
		typed_data->element_selectors =
		    TAB::modified_flags
		        ( typed_data->context_selectors,
			  bracket_type->
			      parsing_selectors );
	}
    }

    if ( start != min::NULL_STUB )
	PAR::free
	    ( PAR::remove ( first_ref(parser),
			    start ) );
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

    PAR::make_label_or_value
        ( parser, start, next, PAR::LABEL_MODE );
    start->type = BRA::ATTR_LABEL;

    if ( typed_data->attributes == min::NULL_STUB )
	typed_data->attributes = start;

    ++ typed_data->attr_count;
}

// Finish value.  On error generate a value NONE which
// will be deleted later.
//
inline void finish_value
	( PAR::parser parser,
	  BRA::typed_data * typed_data,
	  PAR::token next )
{
    PAR::token start = typed_data->start_previous->next;
    if ( start->next == next )
    {
	min::gen initiator =
	    typed_data
		->typed_opening
		->typed_attr_multivalue_initiator;
	if ( start->value_type == initiator
	     &&
	     initiator != min::MISSING() )
	    start->type = BRA::ATTR_MULTIVALUE;
	else
	{
	    if ( ! min::is_attr_legal ( start->value ) )
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

	    start->type = BRA::ATTR_VALUE;
	}
    }
    else
    {
	PAR::make_label_or_value
	    ( parser, start, next, PAR::VALUE_MODE );
	start->type = BRA::ATTR_VALUE;
    }
}

// Finish attribute.  Uses finish_value above if
// value needs to be finished.
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
	if ( next->previous->type == BRA::ATTR_FLAGS )
	{
	    if ( subtype == BRA::TYPED_ATTR_NEGATOR )
		PAR::parse_error
		    ( parser,
		      typed_data->negator_position,
		      "negator preceding attribute"
		      " label that is followed by"
		      " flags; negator ignored" );
	}
	else
	    typed_data->start_previous->next->type =
		( subtype == BRA::TYPED_ATTR_NEGATOR ?
		  BRA::ATTR_FALSE :
		  BRA::ATTR_TRUE );
    }
    else if ( subtype == BRA::TYPED_ATTR_EQUAL )
    {
        min::uns32 t =
	    typed_data->start_previous->type;
	if ( t == BRA::ATTR_VALUE
	     ||
	     t == BRA::ATTR_MULTIVALUE )
	{
	    PAR::make_label_or_value
	        ( parser, start, next,
		  PAR::LABEL_MODE );
	    start->type = BRA::ATTR_REVERSE;
	}
	else
	    ::finish_value ( parser, typed_data, next );
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

// Concatenate the MIN strings s1 and s2, after
// removing n1 bytes from the end of s1 and n2
// bytes from the beginning of s2.
//
inline min::gen strcat
	( min::gen s1, min::gen s2,
	  min::uns32 n1 = 0, min::uns32 n2 = 0 )
{
    min::str_ptr sp1 ( s1 );
    min::str_ptr sp2 ( s2 );
    min::unsptr l1 = min::strlen ( sp1 );
    min::unsptr l2 = min::strlen ( sp2 );
    MIN_REQUIRE ( n1 <= l1 );
    MIN_REQUIRE ( n2 <= l2 );
    char s [l1 + l2 + 1];
    min::strcpy ( s, s1 );
    const char * t2 = ~ min::begin_ptr_of ( s2 );
    strcpy ( s + l1 - n1, t2 + n2 );
    return min::new_str_gen ( s );
}

// Return true if the string s is at the beginning of
// the string g, and g is longer than s.  g must a MIN
// string general value.  n is the length of s.
//
inline bool at_begin ( const char * s, min::gen g,
                       min::uns32 n )
{
    min::str_ptr sp ( g );
    const char * t = ~ min::begin_ptr_of ( sp );
    if ( strncmp ( t, s, n ) != 0 ) return false;
    return t[n] != 0;
}

// Return true if the string s is at the end of the
// string g, and g is longer than s.  g must a MIN
// string general value.  n is the length of s.
//
inline bool at_end ( const char * s, min::gen g,
                     min::uns32 n )
{
    min::str_ptr sp ( g );
    const char * t = ~ min::begin_ptr_of ( sp );
    min::uns32 m = strlen ( t );
    if ( m <= n ) return false;
    return strcmp ( t + m - n, s ) == 0;
}

// Check whether lab, which is a MIN label or a legal
// component of a MIN label, is a LL Parser label.
//
inline bool is_label ( min::gen lab )
{
    min::uns64 ok_types =
          ( 1ull << LEXSTD::word_t )
	| ( 1ull << LEXSTD::separator_t )
	| ( 1ull << LEXSTD::mark_t );
    if ( min::is_str ( lab ) )
    {
        min::uns32 type = PAR::lexical_type_of ( lab );
	return ( ( ( 1ull << type ) & ok_types ) != 0 );
    }
    min::lab_ptr labp ( lab );
    if ( labp == min::NULL_STUB ) return false;
    min::uns32 length = min::lablen ( labp );
    if ( length == 0 ) return false;
    for ( min::uns32 i = 0; i < length; ++ i )
    {
        min::uns32 type =
	    PAR::lexical_type_of ( labp[i] );
	if ( ( ( 1ull << type ) & ok_types ) == 0 )
	    return false;
	ok_types |= ( 1ull << LEXSTD::natural_t )
	          | ( 1ull << LEXSTD::number_t )
	          | ( 1ull << LEXSTD::numeric_t );
    }
    return true;
}

min::position BRA::parse_bracketed_subexpression
	( PAR::parser parser,
	  TAB::flags selectors,
	  PAR::token & current,
	  BRA::typed_data * typed_data,
	  BRA::line_variables * line_variables,
	  BRA::bracket_stack * bracket_stack_p )
{

    MIN_REQUIRE ( selectors & PAR::ALWAYS_SELECTOR );

#   define PARSE_BRA_SUBEXP \
	   BRA::parse_bracketed_subexpression
	   // To avoid a too long line

    BRA::bracketed_pass bracketed_pass =
        (BRA::bracketed_pass) parser->pass_stack;
	// First pass in pass_stack is always the
	// bracketed pass.

    min::int32 indentation_offset =
	bracketed_pass->indentation_offset;
	// Current indentation offset.

    BRA::indentation_mark indentation_found =
        min::NULL_STUB;
	// If not NULL_STUB, this is the parser table
	// entry of an indentation mark that has been
	// found, current token is the comment, line-
	// break, or end-of-file after the found
	// indentation mark, and current->previous is
	// the last token of an indentation mark.

    PAR::token start_previous = current->previous;
        // If current == start_previous->next, we are
	// at start of subexpression where prefix
	// separator is legal, etc.

    bool parsing_logical_line =
        ( bracket_stack_p == NULL );
	// True iff we are being called to parse a
	// logical line and NOT a sub-subexpression
	// inside a logical line.

    min::phrase_position bad_comment_position =
        { min::MISSING_POSITION,
	  min::MISSING_POSITION };
	// Begin is set non-MISSING if we are parsing a
	// logical line beginning at paragraph indent
	// that starts with a comment.  If it is set
	// when we get to the first non-whitespace token
	// on the logical line, a bad comment error will
	// be announced (comment lines return to caller
	// before they get to this point).  End is set
	// to end of any comment or line break found.

    if ( parsing_logical_line )
    {
	line_variables->previous = current->previous;
	line_variables->at_paragraph_beginning =
	      parser->at_paragraph_beginning;

	if ( line_variables->at_paragraph_indent
	     &&
	     current->type == LEXSTD::comment_t )
	    bad_comment_position.begin =
	        current->position.begin;
    }

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

    // For descriptions of the following variables, see
    // PREFIX_FOUND or PARSE_PREFIX_N_LIST below.

    // Variables input to both PREFIX_FOUND and
    // PARSE_PREFIX_N_LIST code blocks.
    //
    PAR::token prefix;

    // Variables input to PREFIX_FOUND code block.
    //
    min::position separator_found;
    bool premature_closing;

    // Variables input to the PARSE_PREFIX_N_LIST code
    // block.
    //
    min::gen prefix_type;
    BRA::bracket_type prefix_entry;
    min::gen prefix_group;
    TAB::flags prefix_selectors;

    if ( parsing_logical_line )
    {
	if ( ( selectors & PAR::EPREFIX_OPT ) == 0
	      &&
	      ( selectors & PAR::ETPREFIX_OPT ) == 0
	      &&
	      ( selectors & PAR::EHEADER_OPT ) == 0 )
	    goto NEXT_TOKEN;

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
	MIN_REQUIRE
	    ( prefix_group == PARLEX::paragraph
	      ||
	      prefix_group == PARLEX::line );

	if ( prefix_group == PARLEX::paragraph )
	{
	    if ( ! line_variables->
	               at_paragraph_beginning )
	    {
		min::phrase_position pp =
		    { current->position.begin,
		      current->position.begin };
		min::gen type =
		    min::get ( implied_header,
		               min::dot_type );
		PAR::parse_error
		  ( parser,
		    pp,
		    "implied paragraph header"
		    " of type `",
		    min::pgen_never_quote
			( type ),
		    " is not at beginning"
		    " of logical line in paragraph"
		    " beginning position; ignored"
		  );
		  goto NEXT_TOKEN;
	    }
	    line_variables->current =
		line_variables->implied_paragraph;
	}

	prefix = PAR::new_token
		     ( PAR::IMPLIED_HEADER );
	PAR::put_before
	    ( PAR::first_ref(parser),
	      current, prefix );
	PAR::value_ref(prefix) = implied_header;
	PAR::value_type_ref(prefix) =
	    min::new_stub_gen ( prefix_entry );
	prefix->position.begin =
	    current->position.begin;
	prefix->position.end =
	    current->position.begin;

	goto PARSE_PREFIX_N_LIST;
    }
    else
    {
	MIN_REQUIRE ( bracket_stack_p != NULL );
	if ( bracket_stack_p->prefix == min::NULL_STUB )
	    goto NEXT_TOKEN;
	BRA::bracket_type p =
	    (BRA::bracket_type)
	    min::stub_of
	        ( bracket_stack_p->prefix->value_type );
	if ( p == min::NULL_STUB )
	    goto NEXT_TOKEN;
	if ( p->implied_subprefix == min::MISSING() )
	    goto NEXT_TOKEN;

	prefix = PAR::new_token
		     ( PAR::IMPLIED_PREFIX );
	PAR::put_before
	    ( PAR::first_ref(parser),
	      current, prefix );
	PAR::value_ref(prefix) = p->implied_subprefix;
	PAR::value_type_ref(prefix) =
	      p->implied_subprefix_type;
	      // We go to PREFIX_FOUND which will
	      // convert type to prefix entry
	      // if possible.
	prefix->position.begin =
	    current->position.begin;
	prefix->position.end =
	    current->position.begin;

	if ( p->group == PARLEX::paragraph )
	{
	    // See if implied_subprefix group is
	    // `line' and change IMPLIED_PREFIX to
	    // IMPLIED_HEADER if it is.
	    //
	    TAB::key_table bracket_type_table =
		bracketed_pass->bracket_type_table;
	    BRA::bracket_type implied_subprefix_entry =
		(BRA::bracket_type)
		TAB::find
		    ( p->implied_subprefix_type,
		      BRA::BRACKET_TYPE,
		      selectors,
		      bracket_type_table );

	    if (    implied_subprefix_entry
	         != min::NULL_STUB
	         &&
	            implied_subprefix_entry->group
	         == PARLEX::line )
		prefix->type = PAR::IMPLIED_HEADER;
	}

	separator_found = min::MISSING_POSITION;
	premature_closing = false;
    }

    // Above code either jumps to NEXT_TOKEN or
    // creates implied prefix and jumps to
    // PARSE_PREFIX_N_LIST for implied prefixes created
    // by line variables or comes here for implied
    // prefixes NOT create by line variables.

PREFIX_FOUND:

    MIN_REQUIRE
        (    prefix->type == PAR::PREFIX
          || prefix->type == PAR::MAPPED_PREFIX
          || prefix->type == PAR::IMPLIED_PREFIX
	  || prefix->type == PAR::IMPLIED_HEADER );

    // Input variables:
    //
    //     prefix
    //         The prefix token.
    //         prefix->value_type must be the .type of
    //         the prefix token value at this point,
    //         and will be replaced below by the prefix
    //         entry of this .type if that exists.
    //     separator_found
    //         Specifies separator found right after
    //         prefix token, if any.
    //     premature_closing
    //         True if premature closing forced the
    //         end of the prefix token.
    //
    {
	// At this point prefix->next == current OR
	// premature_closing is true, current is
	// beyond prefix, and prefix-n-list will be
	// assumed to be empty (see FINISH_PREFIX).
	//
        MIN_REQUIRE ( prefix->next != parser->first );
        MIN_REQUIRE ( prefix->next == current
	              ||
		      premature_closing );

	PAR::token prefix_next = prefix->next;

	TAB::key_table bracket_type_table =
	    bracketed_pass->bracket_type_table;
	prefix_type = prefix->value_type;
	MIN_REQUIRE ( min::is_name ( prefix_type ) );
	prefix_entry =
	    (BRA::bracket_type)
	    TAB::find ( prefix_type,
			BRA::BRACKET_TYPE,
			selectors,
			bracket_type_table );
	prefix_group = prefix_type;
	if ( prefix_entry != min::NULL_STUB )
	{

	    if ( prefix_entry->group == PARLEX::reset )
	    {
		MIN_REQUIRE
		    ( prefix->next == current );

		PAR::token t = prefix;
		while (   line_variables->previous
		                        ->next
		        != t
			&&
			(
			      t->previous->type
			   == IMPLIED_HEADER
			   ||
			      t->previous->type
			   == IMPLIED_PREFIX ) )
		    t = t->previous;

		if (    line_variables->previous->next
		     != t
		     || ! line_variables->
		              at_paragraph_beginning )
		{
		    PAR::parse_error
		      ( parser,
			prefix->position,
			"reset header of type `",
			min::pgen_never_quote
			    ( prefix_type ),
			"' not at beginning of logical"
			" line in paragraph beginning"
			" position; ignored" 
		      );
		    prefix = min::NULL_STUB;
		    goto FINISH_PREFIX;
		}

		// If t != prefix, then we have been
		// called to scan the prefix-n-list of
		// an implied prefix.  In this case when
		// we return BRA::ISOLATED_HEADER, the
		// caller will see this and delete the
		// implied prefix or header and return
		// BRA::ISOLATED_HEADER to its caller,
		// etc.  We CANNOT delete implied pre-
		// fixes here.

		prefix->type = PAR::BRACKETED;
		PAR::value_type_ref(prefix) =
		    PARLEX::reset;
		return BRA::ISOLATED_HEADER;
	    }

	    if ( prefix_entry->group != min::MISSING() )
		prefix_group = prefix_entry->group;

	    if ( ( selectors & PAR::ETPREFIX_OPT )
	         ||
		 ( selectors & PAR::EPREFIX_OPT )
		 ||
		 ( ( prefix_group == PARLEX::paragraph
		     ||
		     prefix_group == PARLEX::line )
		   &&
		   ( selectors & PAR::EHEADER_OPT ) ) )
	    {
		PAR::value_type_ref(prefix) =
		    min::new_stub_gen ( prefix_entry );
	    }
	    else
	    if ( prefix->type == PAR::IMPLIED_PREFIX
		 ||
		 prefix->type == PAR::IMPLIED_HEADER )
	    {
	        prefix = min::NULL_STUB;
		goto FINISH_PREFIX;
	    }
	    else
	    {
		prefix->type = PAR::BRACKETED;
	        if ( premature_closing )
		    return separator_found;
		else
		    goto NEXT_TOKEN;
	    }

	}
	else
	if ( ( selectors & PAR::EPREFIX_OPT ) == 0 )
	{
	    if ( prefix->type == PAR::IMPLIED_PREFIX
		 ||
		 prefix->type == PAR::IMPLIED_HEADER )
	    {
	        prefix = min::NULL_STUB;
		goto FINISH_PREFIX;
	    }
	    else
	    {
		prefix->type = PAR::BRACKETED;
	        if ( premature_closing )
		    return separator_found;
		else
		    goto NEXT_TOKEN;
	    }
	}

	for ( BRA::bracket_stack * p =
		     bracket_stack_p;

	      p != NULL
	      &&
	      p->prefix != min::NULL_STUB;

	      p = p->previous )
	{
	    if ( p->prefix_group == prefix_group )
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
			"' has group of"
			" previous active prefix"
			" separator; not inserted"
		      );
		    prefix = min::NULL_STUB;
		    goto FINISH_PREFIX;
		}
		if ( (    prefix_group
		       == PARLEX::paragraph
		       ||
		          prefix_group
		       == PARLEX::line )
		     &&
		        prefix->type
		     != PAR::IMPLIED_HEADER )
		{
		    PAR::token t = p->prefix;
		    while (    t->type
		            == PAR::IMPLIED_HEADER
			    ||
			       t->type
			    == PAR::IMPLIED_PREFIX )
			t = t->next;
		    if ( t != prefix )
		    {
			PAR::parse_error
			  ( parser,
			    prefix->position,
			    "explicit prefix separator"
			    " of type `",
			    min::pgen_never_quote
				( prefix_type ),
			    "' has `",
			    min::pgen_never_quote
				( prefix_group ),
			    "' group but is not at the"
			    " beginning of a logical"
			    " line; ignored"
			  );
			prefix = min::NULL_STUB;
			goto FINISH_PREFIX;
		    }
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
	// prefix at the beginning of a logical line
	// in paragraph beginning position that is
	// following an implied line header that begins
	// the logical line (i.e., if it has a preceed-
	// ing implied paragraph header we would not
	// get to this point because of the above code).
	//
	// We must pretend the explicit paragraph header
	// terminates the prefix-0-list begun by the
	// implied line header.  As this list is empty,
	// the implied line header will be deleted.
	//
	// To enforce this, this code must immediately
	// follow the above code.
	//
	if ( prefix_group == PARLEX::paragraph
	     &&
	     line_variables->at_paragraph_beginning
	     &&
	     prefix->type != PAR::IMPLIED_PREFIX
	     &&
	     prefix->type != PAR::IMPLIED_HEADER
	     &&
		prefix->previous->type
	     == PAR::IMPLIED_HEADER
	     &&
	     bracket_stack_p != NULL
	     &&
		bracket_stack_p->prefix_group
	     == PARLEX::line )
	{
	    bracket_stack_p->closing_first =
		prefix;
	    bracket_stack_p->closing_next =
		prefix->next;
	    return separator_found;
	}

	if ( typed_data != NULL )
	{
	    PAR::parse_error
	      ( parser,
		prefix->position,
		"prefix separator of type `",
		min::pgen_never_quote
		    ( prefix_type ),
		"' in typed bracketed"
		" subexpression; ignored"
	      );
	    prefix = min::NULL_STUB;
	}
	else
	if ( prefix_group == PARLEX::paragraph )
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
		    "' that is not an implied header"
		    " has `paragraph' group; ignored"
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
	if ( prefix_group == PARLEX::line
	     &&
		line_variables->previous->next
	     != prefix
	     &&
	     ( bracket_stack_p == NULL
	       ||
		  bracket_stack_p->prefix_group
	       != PARLEX::paragraph ) )
	{
	    PAR::parse_error
	      ( parser,
		prefix->position,
		   prefix->type
		== PAR::IMPLIED_PREFIX ?
		    "implied prefix separator" :
		   prefix->type
		== PAR::IMPLIED_HEADER ?
		        // Probably impossible.
		    "implied header" :
		    "explicit prefix separator",
		min::pnop,
		" of type `",
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
		" subexpression and does not"
		" continue subexpression started by"
		" another prefix with the same type"
		" or group; ignored"
	      );
	    prefix = min::NULL_STUB;
	}

FINISH_PREFIX:

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

	prefix_selectors = selectors;
	if ( prefix_entry != min::NULL_STUB )
	    prefix_selectors =
	        TAB::modified_flags
		    ( prefix_selectors,
		      prefix_entry->
		          parsing_selectors );
    }

PARSE_PREFIX_N_LIST:

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
    //         The bracket_type_table entry of the
    //         prefix token .type (NULL_STUB if none).
    //     prefix_group
    //         The group of the prefix.  Equals the
    //         prefix_type if prefix_entry == NULL_STUB.
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
	cstack.prefix = prefix;
	cstack.prefix_group = prefix_group;

	while ( true )
	{
	    if (    prefix_group
		 == PARLEX::paragraph
		 &&
		    prefix->type
		 != PAR::IMPLIED_HEADER )
	    {
		// Come here if explicit paragraph
		// header found in a logical line, but
		// with possible implied line headers
		// having been previously inserted and
		// deleted, and thus possible incorrect
		// setting of selectors because of
		// a CONTINUING flag.
		//
		// Fix things up.

		MIN_REQUIRE
		    ( parsing_logical_line );
		MIN_REQUIRE
		    (    prefix->type
		      != PAR::IMPLIED_PREFIX );

		selectors =
		    line_variables->paragraph
				   .selectors;
		selectors &=
		    ~ ( PAR::STICKY_OPT
		        +
			PAR::CONTINUING_OPT );

		prefix_selectors =
		    TAB::modified_flags
		        ( selectors,
			  prefix_entry->
			     parsing_selectors );

		if ( (   prefix_selectors
		       & PAR::STICKY_OPT )
		     &&
		        prefix_entry->
			    line_lexical_master
		     != PAR::MISSING_MASTER )
		{
		    PAR::parse_error
		      ( parser,
			prefix->position,
			"isolated header of type `",
			min::pgen_never_quote
			  ( prefix_type ),
			"' cannot be sticky;"
			" sticky flag ignored" );
		    prefix_selectors &=
		        ~ PAR::STICKY_OPT;
		}

		BRA::line_data & line_data =
		    line_variables->current;
		line_data.line_lexical_master =
		    prefix_entry->line_lexical_master;
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
		    TAB::key_table bracket_type_table =
			bracketed_pass->
			    bracket_type_table;
		    BRA::bracket_type header_entry =
			(BRA::bracket_type)
			TAB::find
			    ( header_type,
			      BRA::BRACKET_TYPE,
			      prefix_selectors,
			      bracket_type_table );
		    if (    header_entry
			 != min::NULL_STUB
			 &&
			    header_entry->group
			 == PARLEX::line )
		    {
			line_data.implied_header =
			    implied_header;
			line_data.header_entry =
			    header_entry;
			line_data.header_selectors =
			    TAB::modified_flags
			        ( prefix_selectors,
			          header_entry->
				    parsing_selectors );
		    }
		}

		if (   prefix_selectors
		     & PAR::STICKY_OPT )
		{
		    line_variables->
			    implied_paragraph =
			line_variables->current;

		    line_variables->sticky_header =
		        min::copy ( prefix->value, 0 );

		     min::set ( line_variables
		                    ->sticky_header,
		                min::dot_position,
			        min::NONE() );

		    line_variables->
			paragraph.implied_header =
		      line_variables->sticky_header;
		    line_variables->
			paragraph.header_entry =
		      prefix_entry;
		    line_variables->
			paragraph.header_selectors =
		      prefix_selectors;
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

		if (    prefix_entry->
		            line_lexical_master
		     != PAR::MISSING_MASTER
		     &&
		     prefix->type == PAR::PREFIX )
		{
		    prefix->type = PAR::BRACKETED;
		    PAR::value_type_ref(prefix) =
		        PARLEX::paragraph;
		    return BRA::ISOLATED_HEADER;
		}
	    }

	    cstack.closing_first =
		min::NULL_STUB;
	    cstack.closing_next =
		min::NULL_STUB;
	    min::position separator_found =
		PARSE_BRA_SUBEXP
		  ( parser, prefix_selectors,
		    current,
		    NULL,
		    line_variables, & cstack );

	    PAR::token next =
		cstack.closing_first;
	    if ( next == min::NULL_STUB )
		next = current;

	    min::gen old_prefix_value_type =
	        prefix->value_type;

	    if (    separator_found
	         == BRA::ISOLATED_HEADER )
	    {
		if (    prefix->type
		     == PAR::IMPLIED_PREFIX
		     ||
		        prefix->type
		     == PAR::IMPLIED_HEADER )
		    PAR::free
			( PAR::remove
			      ( first_ref(parser),
				prefix ) );
	    	return separator_found;
	    }

	    if (    prefix->next == next
		 && (    prefix->type
		      == PAR::IMPLIED_PREFIX
		      ||
		         prefix->type
		      == PAR::IMPLIED_HEADER )
		 && ( prefix_group != PARLEX::line
		      ||
		      ! separator_found ) )
	    {
		PAR::free
		    ( PAR::remove ( first_ref(parser),
				    prefix ) );
		goto CONTINUE_PREFIX_N_LIST;
	    }

	    if (    prefix_group
		 == PARLEX::paragraph
		 &&
		 ( ( prefix_selectors
		     &
		     (   PAR::EAINDENT_OPT
		       + PAR::EALEINDENT_OPT
		       + PAR::EALSEP_OPT ) )
		   != 0 )
		 &&
		 prefix->next != next
		 &&
		 ( prefix->next->next != next
		   ||
		      prefix->next->value_type
		   != PARLEX::line ) )
	    {
		PAR::token first = prefix->next;
		BRA::compact_logical_line
		    ( parser, bracketed_pass->next,
		      prefix_selectors,
		      first, next,
		      separator_found,
		      (TAB::root)
		      line_variables->line_sep,
		      trace_flags );
	    }

	    if ( prefix_group == PARLEX::line
		 ||
		 prefix_group == PARLEX::paragraph )
		PAR::value_type_ref ( prefix ) =
		    prefix_group;
	    else
		PAR::value_type_ref ( prefix ) =
		    min::MISSING();

	    if ( prefix_entry != min::NULL_STUB
	         &&
		    prefix_entry->reformatter
		 != min::NULL_STUB )
	    {
		min::phrase_position position =
		    { prefix->position.begin,
		      next->previous->position.end };
		if ( separator_found )
		    position.end = separator_found;

		if ( ! ( * prefix_entry->reformatter->
		             reformatter_function )
			   ( parser,
			     (PAR::pass) bracketed_pass,
			     prefix_selectors,
			     prefix, next, position,
			     separator_found ?
				 line_variables->
				     line_sep->label :
				 min::MISSING(),
			     trace_flags,
			     (TAB::root) prefix_entry )
		   )

		    goto CONTINUE_PREFIX_N_LIST;
	     }

	     BRA::compact_prefix_list
		 ( parser, bracketed_pass->next,
		   prefix_selectors,
		   prefix, next,
		   prefix_group == PARLEX::line ?
		       separator_found :
		       min::MISSING_POSITION,
		      line_variables->line_sep
		   != min::NULL_STUB ?
		      line_variables->line_sep
				    ->label :
		      min::MISSING(),
		   trace_flags );

	CONTINUE_PREFIX_N_LIST:

	    if (    cstack.closing_first
		 == cstack.closing_next )
		return separator_found;

	    prefix = cstack.prefix
		   = cstack.closing_first;

	    if (    prefix->value_type
		 != old_prefix_value_type )
	    {
		prefix_entry =
		    (BRA::bracket_type)
		    min::stub_of ( prefix->value_type );

		prefix_selectors = selectors;

		if ( prefix_entry != min::NULL_STUB )
		{
		    prefix_selectors =
		        TAB::modified_flags
			    ( selectors,
			      prefix_entry->
			          parsing_selectors );

		    prefix_type = prefix_entry->label;

		    cstack.prefix_group =
		    prefix_group =
			(    prefix_entry->group
		          != min::MISSING() ?
			  prefix_entry->group :
			  prefix_type );
		}
		else
		    prefix_type =
		    prefix_group =
		    cstack.prefix_group =
			prefix->value_type;

	    }
	}
    }

NEXT_TOKEN:

    // Come here with `current' set to the next
    // token to process.

    // This main loop does the following:
    //
    // (1) Delete comments and line breaks.
    // (2) Handle indentation_marks already found.
    // (3) Return to caller on end-of-file or logical
    //     line ending indent token.
    // (4) Delete any continuation line indent token and
    //     loop to NEXT_TOKEN.
    // (5) Announce bad comments if any (comment lines
    //     never come here).
    // (6) Process non-indent, non-line-break, non-end-
    //     of-file, non-comment token or multi-token
    //     key.
    // (7) Iterate loop to NEXT_TOKEN.

    // Delete comments and line breaks and set
    // parser->at_paragraph_beginning if appropriate.
    //
    min::uns32 t = current->type;
    while ( t == LEXSTD::line_break_t
	    ||
	    t == LEXSTD::comment_t )
    {
	bad_comment_position.end =
	    current->position.end;
	    // Record end in case we need it.
	PAR::ensure_next ( parser, current );

	min::uns32 previous_t = t;
	current = current->next;
	t = current->type;

	if ( previous_t == LEXSTD::line_break_t
	     &&
	     t == LEXSTD::line_break_t )
	    parser->at_paragraph_beginning = true;
	else if ( t == LEXSTD::end_of_file_t )
	    parser->at_paragraph_beginning = true;

	PAR::free
	    ( PAR::remove ( first_ref(parser),
			    current->previous ) );
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

	MIN_REQUIRE ( ! bad_comment_position.begin );

	// Compute selectors for indented sub-
	// paragraph.
	//
	TAB::flags new_selectors =
	    TAB::modified_flags
	        ( selectors,
	          indentation_found->
		      parsing_selectors );

	PAR::token mark_end = current->previous;
	    // Last token of indentation mark.

	// Scan lines of paragraph.  Current will
	// become the first line break or end of
	// file after the paragraph.

	// First be sure paragraph has some
	// lines.
	//
	if ( current->type == LEXSTD::indent_t
	     &&
	     PAR::relative_indent
		 ( parser,
		   indentation_offset,
		   current,
		   line_variables->
		       paragraph_indent )
	     > 0 )
	{

	    // Initialize line_variables.
	    //
	    BRA::line_variables line_variables;
	    BRA::init_line_variables
		( line_variables,
		  indentation_found,
		  parser,
		  new_selectors,
		  current->indent,
		  current );
	    parser->at_paragraph_beginning = true;
	    while ( true )
		// Loop to parse paragraph lines.
	    {
		PAR::token previous =
		    current->previous;
		parse_paragraph_element
		    ( parser, current,
		      & line_variables,
		      trace_flags );
		PAR::token output = previous->next;
		if ( output == current ) break;
	    }
	}

	// Remove indentation mark and compact
	// indented paragraph elements into an
	// indented paragraph value.
	//
	PAR::token first = mark_end->next;
	min::phrase_position position;
	position.begin =
	    PAR::remove
		( parser, first,
		  indentation_found->label );
	position.end = current->previous
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
	      first, current, position,
	      trace_flags,
	      PAR::BRACKETING,
	      2, attributes );

	value_type_ref(first) =
	    indentation_found->label;

	// Temporary check.
	//
	MIN_REQUIRE
	  ( ! BRA::is_closed ( bracket_stack_p ) );

	// Fall through to process indent or eof at
	// current that is after indented paragraph.
	//
	indentation_found = min::NULL_STUB;
    }

    // Continuation after any indented paragraph
    // has been processed, or if there was no
    // indented paragraph.  Current is not line-break
    // or comment.
    //
    // Return if end of logical line.  Otherwise if
    // indent, delete indent (its for continuation
    // line) and loop to NEXT_TOKEN.  Otherwise fall
    // through.
    //
    if ( current->type == LEXSTD::end_of_file_t )
	return min::MISSING_POSITION;
    else if ( current->type == LEXSTD::indent_t )
    {
	if ( selectors & PAR::EAINDENT_OPT )
		return min::MISSING_POSITION;

	if ( ( parser->at_paragraph_beginning
	       &&
	       start_previous->next != current )
	     &&
	     (   selectors
	       & PAR::EAPBREAK_OPT ) )
		return min::MISSING_POSITION;

	// Truncate subexpression if current token
	// indent is at or before line_variables->
	// paragraph_indent.
	//
	if (   selectors
	     & (   PAR::EALEINDENT_OPT
		 | PAR::EALTINDENT_OPT ) )
	{
	    int32 rindent =
		PAR::relative_indent
		    ( parser,
		      bracketed_pass->
			  indentation_offset,
		      current,
		      line_variables->
			  paragraph_indent );
	    if ( rindent < 0 )
		return min::MISSING_POSITION;
	    else if ( rindent == 0
		      &&
		      (   selectors
			& PAR::EALEINDENT_OPT ) )
		return min::MISSING_POSITION;
	}

	// Next is first part of continuation line.
	// Remove indent and go to NEXT_TOKEN.
	//
	PAR::ensure_next ( parser, current );
	current = current->next;
	PAR::free
	    ( PAR::remove ( first_ref(parser),
			    current->previous ) );
	goto NEXT_TOKEN;
    }

    // Continue with non-comment, non-line-break,
    // non-eof, non-indent token.
    //
    MIN_REQUIRE
	( indentation_found == min::NULL_STUB );
    MIN_REQUIRE
	( current->type != LEXSTD::end_of_file_t
	  &&
	  current->type != LEXSTD::indent_t
	  &&
	  current->type != LEXSTD::comment_t
	  &&
	  current->type != LEXSTD::line_break_t );

    parser->at_paragraph_beginning = false;

    if ( bad_comment_position.begin )
    {
	PAR::parse_warn
	    ( parser, bad_comment_position,
	      "comment after indent that begins"
	      " non-comment logical line" );
	bad_comment_position.begin =
	    min::MISSING_POSITION;
    }

    // Process quoted strings.
    //
    if ( current->type == LEXSTD::quoted_string_t )
    {
	// Check if this quoted string is to be
	// concatenated to a previous quoted string
	// and go to NEXT_TOKEN if not.
	//
	PAR::ensure_next ( parser, current );
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
	    PAR::token t = current->previous->previous;
	    if ( t->value != concat )
		goto NEXT_TOKEN;
	    if ( start_previous->next == t )
		goto NEXT_TOKEN;

	    int number_of_concatenators = 1;
	    t = t->previous;

	    if ( t->value == concat )
	    {
		if ( start_previous->next == t )
		    goto NEXT_TOKEN;
		t = t->previous;
	        ++ number_of_concatenators;
	    }

	    if ( t->type != LEXSTD::quoted_string_t )
		goto NEXT_TOKEN;

	    // Remove string_concatenator tokens.
	    //
	    while ( number_of_concatenators -- )
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

	// Append current->previous->value to the end
	// of current->previous->previous->value, and
	// delete current->previous.
	//
	PAR::value_ref ( current->previous->previous )
	    = ::strcat ( current->previous->previous
	                        ->value,
			 current->previous->value );
	current->previous->previous
		->position.end =
	    current->previous->position.end;
	PAR::free
	    ( PAR::remove
		( first_ref(parser),
		  current->previous ) );
	goto NEXT_TOKEN;
    }

    // Process broken middle lexemes.
    //
    if ( current->type < 32
         &&
	 (   ( 1 << current->type )
           & LEXSTD::middle_mask )
	 &&
            bracketed_pass->middle_break.begin_length
	 != 0
	 &&
	 at_begin
	     ( bracketed_pass->middle_break.begin,
	       current->value,
	       bracketed_pass->middle_break
			      .begin_length )
	 &&
	 start_previous->next != current
	 &&
	 current->previous->type < 32
	 &&
	 (    ( 1 << current->previous->type )
            & LEXSTD::middle_mask )
	 &&
	 at_end ( bracketed_pass->middle_break.end,
	          current->previous->value,
		  bracketed_pass->middle_break
		                 .end_length ) )
    {
	// Append current->value to the end of current->
	// previous->value.  Then delete current and
	// move to next token, and go to NEXT_TOKEN.
	//
	PAR::value_ref ( current->previous )
	    = ::strcat ( current->previous->value,
			 current->value,
			 bracketed_pass->middle_break
			                .begin_length,
			 bracketed_pass->middle_break
			                .end_length );
	current->previous->position.end =
	    current->position.end;
	if ( current->previous->type == LEXSTD::mark_t )
	    current->previous->type = current->type;
	    // Types must be mark, numeric, or word.

	PAR::ensure_next ( parser, current );
	current = current->next;
	PAR::free
	    ( PAR::remove
		( first_ref(parser),
		  current->previous ) );
	goto NEXT_TOKEN;
    }

    if ( current->type == PAR::PREFIX
         ||
	 current->type == PAR::MAPPED_PREFIX )
    {
	prefix = current;

	PAR::ensure_next ( parser, current );
	current = current->next;

	separator_found = min::MISSING_POSITION;
	premature_closing = false;

	goto PREFIX_FOUND;
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
		 typed_data->middle_count == 0
		 &&
		 current->type == LEXSTD::mark_t )
	    {
		// Current is mark at begining of
		// typed bracketed subexpresson.
		//
		// Turn current into TYPE token.
		//
		typed_data->type = current->value;
		typed_data->has_mark_type = true;
		current->type = BRA::TYPE;
		++ typed_data->attr_count;

		// Adjust typed_data->element_selectors
		//
		TAB::key_table bracket_type_table =
		    bracketed_pass->bracket_type_table;
		BRA::bracket_type bracket_type =
		    (BRA::bracket_type)
		    TAB::find
			( typed_data->type,
			  BRA::BRACKET_TYPE,
			  typed_data->context_selectors,
			  bracket_type_table );
		if ( bracket_type != min::NULL_STUB )
		    typed_data->element_selectors =
		        TAB::modified_flags
			    ( typed_data->
			          context_selectors,
			      bracket_type->
			            parsing_selectors );

		selectors =
		    typed_data->element_selectors;
	    }

	    // Move to next token.
	    //
	    PAR::ensure_next ( parser, current );
	    current = current->next;
	    break;
	}

	min::uns32 subtype =
	    min::packed_subtype_of ( root );

	if ( trace_flags & PAR::TRACE_KEYS )
	{
	    parser->printer
		<< min::bom << min::set_indent ( 4 )
		<< "BRACKETED "
		<< "PARSER FOUND ";
	    COM::print_flags
		( selectors,
		  PAR::COMMAND_SELECTORS,
		  parser->selector_name_table,
		  parser );
	    parser->printer << " SELECTED KEY "
		<< min::pgen_quote ( root->label )
		<< min::indent << " OF SUBTYPE "
		<< min::name_of_packed_subtype
		       ( min::packed_subtype_of
			     ( root ) )
		<< min::eom;
	}

	if ( ( subtype == BRA::OPENING_BRACKET
	       ||
	       subtype == BRA::TYPED_OPENING ) )
	{
	    BRA::opening_bracket opening_bracket =
		(BRA::opening_bracket) root;
		// TYPED_OPENING is a subclass of
		// OPENING_BRACKET.

	    TAB::flags new_selectors =
	        TAB::modified_flags
		    ( selectors,
		      opening_bracket->
		          parsing_selectors );

	    BRA::bracket_stack cstack
		( bracket_stack_p );
	    cstack.opening_bracket =
		opening_bracket;

	    PAR::token previous = current->previous;
	    BRA::typed_data tdata;
	    bool is_mark_prefix = false;
	    if ( subtype == BRA::OPENING_BRACKET )
	    {
		separator_found =
		    PARSE_BRA_SUBEXP
		      ( parser, new_selectors,
			current,
			NULL,
			line_variables, & cstack );
	    }
	    else // if (    subtype
		 //      == BRA::TYPED_OPENING )
	    {
		tdata.typed_opening =
		    (BRA::typed_opening) root;
		tdata.context_selectors = selectors;
		tdata.element_selectors =
		    new_selectors;

		// Prohibit prefixes inside
		// typed-bracketed-subexpressions.
		//
		tdata.context_selectors &=
		    ~ ( PAR::EPREFIX_OPT +
		        PAR::ETPREFIX_OPT +
		        PAR::EHEADER_OPT );
		tdata.element_selectors &=
		    ~ ( PAR::EPREFIX_OPT +
		        PAR::ETPREFIX_OPT +
		        PAR::EHEADER_OPT );

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

		TAB::flags aselectors = selectors;
		aselectors &= PAR::ALL_OPT
		            - BRA::BRACKET_OFF_OPT;
		aselectors
		    |= tdata.typed_opening
			       ->attr_selectors
		    | PAR::ALWAYS_SELECTOR;
		tdata.attribute_selectors = aselectors;

		separator_found =
		    PARSE_BRA_SUBEXP
		      ( parser, aselectors,
			current,
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
	    // brackets.  This is because the new_
	    // selectors element of all opening bracket
	    // parser table entries is automatically set
	    // to remove any EALSEP_OPT flag from the
	    // separators.
	    //
	    MIN_REQUIRE ( ! separator_found );

	    PAR::token next = current;
	    min::phrase_position position;
		// Arguments for compact.

	    if (    cstack.closing_next
		 == cstack.closing_first )
	    {
		// Found a closing bracket that is
		// not ours, or found a logical line
		// end before any closing bracket.

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
		    // Likely this can never happen,
		    // but we have it here just in
		    // case.
		    //
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
			   min::MISSING(),
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
		// At this point the subexpression
		// consists of special attribute
		// tokens (with token types TYPE,
		// ATTR_{LABEL,VALUE,TRUE,FALSE,
		// FLAGS,MULTIVALUE}) followed by
		// subexpression elements (beginning
		// with typed_data.elements which is
		// NULL_STUB if there are no
		// elements).


		// Loop through special attribute
		// tokens making a list of attribu-
		// tes to be handed to the compact
		// function.
		//
		PAR::attr attributes
		    [tdata.attr_count];
		PAR::token elements =
		    tdata.elements;
		if ( elements == min::NULL_STUB )
		    elements = next;
		min::gen type = min::MISSING();
		min::unsptr i = 0;
		bool skip = false;	
		    // If true, skip to next
		    // attribute label.
		for ( PAR::token t = first;
		      t != elements; t = t->next )
		{
		    if ( t->type == BRA::TYPE )
		    {
		        if ( ! ::is_label ( t->value ) )
			{
			    PAR::parse_error
				( parser,
				  t->position,
				  "illegal type `", 
				  min::pgen_never_quote
				      ( t->value ),
				  "'; type ignored" );
			    skip = true;
			}
			else
			{
			    MIN_REQUIRE
			     ( i < tdata.attr_count );
			    attributes[i].name =
				min::dot_type;
			    attributes[i].value =
				t->value;
			    attributes[i].value_pos =
				t->position;
			    type = t->value;
			    ++ i;
			    skip = true;
			}
		    }
		    else if (    t->type
			      == BRA::ATTR_LABEL )
		    {
			if ( t->value == min::NONE() )
			{
			    PAR::parse_error
				( parser,
				  t->position,
				  "bad attribute"
				  " label; attribute"
				  " ignored" );
			    skip = true;
			}
			else
			{
			    MIN_REQUIRE
			     (   i
			       < tdata.attr_count );
			    attributes[i].name =
				t->value;
			    ++ i;
			    skip = false;
			}
		    }
		    else if (    t->type
			      == BRA::ATTR_TRUE )
		    {
			if ( t->value == min::NONE() )
			{
			    PAR::parse_error
				( parser,
				  t->position,
				  "bad attribute"
				  " label; attribute"
				  " ignored" );
			    skip = true;
			}
			else
			{
			    MIN_REQUIRE
			     (   i
			       < tdata.attr_count );
			    attributes[i].name =
				t->value;
			    attributes[i].value =
				min::TRUE;
			    attributes[i].value_pos =
			        t->position;
			    ++ i;
			    skip = false;
			}
		    }
		    else if (    t->type
			      == BRA::ATTR_FALSE )
		    {
			if ( t->value == min::NONE() )
			{
			    PAR::parse_error
				( parser,
				  t->position,
				  "bad attribute"
				  " label; attribute"
				  " ignored" );
			    skip = true;
			}
			else
			{
			    MIN_REQUIRE
			     (   i
			       < tdata.attr_count );
			    attributes[i].name =
				t->value;
			    attributes[i].value =
				min::FALSE;
			    attributes[i].value_pos =
			        t->position;
			    ++ i;
			    skip = false;
			}
		    }
		    else if ( skip )
			/* Do nothing */;
		    else if (    t->type
			      == BRA::ATTR_VALUE )
		    {
			if ( t->value == min::NONE() )
			{
			    PAR::parse_error
				( parser,
				  t->position,
				  "bad attribute"
				  " value; attribute"
				  " ignored" );
			    -- i;
			    skip = true;
			}
			else
			{
			    attributes[i-1].value =
				t->value;
			    attributes[i-1].value_pos =
			        t->position;
			}
		    }
		    else if (    t->type
			      == BRA::
				 ATTR_MULTIVALUE )
		    {
			attributes[i-1].multivalue =
			    t->value;
			attributes[i-1].value_pos =
			    t->position;
		    }
		    else if (    t->type
			      == BRA::ATTR_FLAGS )
			attributes[i-1].flags =
			    t->value;
		    else if (    t->type
			      == BRA::ATTR_REVERSE )
		    {
			if ( t->value == min::NONE() )
			{
			    PAR::parse_error
				( parser,
				  t->position,
				  "bad attribute"
				  " reverse label;"
				  " attribute"
				  " ignored" );
			    -- i;
			}
			else
			    attributes[i-1]
			        .reverse_name =
				    t->value;
		    }
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
		     ( tdata.typed_opening->
			   prefix_selectors
		       &&
		       tdata.context_selectors ) )
		    token_type = PAR::PREFIX;
		else
		    type = min::MISSING();

		bool first_equals_elements =
		    ( first == elements );
		PAR::compact
		    ( parser, bracketed_pass->next,
		      tdata.element_selectors,
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
		break;  // Goto NEXT_TOKEN
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
	    if (
		  (   selectors
		    & PAR::EIPARAGRAPH_OPT )
		  &&
		  (    current->type
		    == LEXSTD::line_break_t
		    ||
		       current->type
		    == LEXSTD::comment_t
		    ||
		       current->type
		    == LEXSTD::end_of_file_t ) )
	    {
		indentation_found =
		    (BRA::indentation_mark) root;
		break;  // Goto NEXT_TOKEN
	    }

	    // Indentation mark not at end of line
	    // or end of file; reject key.
	}
	else if ( subtype == BRA::LINE_SEP )
	{
	    if (     line_variables->line_sep
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
			 == BRA::TYPED_OPENING )
		    {
			 if (    typed_data
				     ->start_previous
				     ->next
			      != key_first )
			    ::make_type_label
				( parser, typed_data,
				  key_first );
			  else
			      typed_data->type =
			          min::empty_str;
		    }
		    else 
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
		    break;  // Goto NEXT_TOKEN
		}

		// Do the following only if subtype
		// == BRA::TYPED_MIDDLE.

		if (    typed_data->middle_count % 2
		     == 1 )
		    selectors =
			typed_data->element_selectors;
		else // if
		     // typed_data->middle_count % 2
		     // == 0
		    selectors =
			typed_data->attribute_selectors;

		break;  // Goto NEXT_TOKEN
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
		break;  // Goto NEXT_TOKEN
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
		    if ( typed_data->subtype
			 ==
			 BRA::TYPED_ATTR_NEGATOR )
			PAR::parse_error
			    ( parser,
			      typed_data->
				negator_position,
			      "negator preceding"
			      " attribute label"
			      " that is followed"
			      " by `",
			      min::pgen_never_quote
				  ( root->label ),
			      "'; negator"
			      " ignored" );
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
		else if (    typed_data->subtype
		          == BRA::TYPED_ATTR_EQUAL
			  &&
			     typed_data->
			         start_previous->type
		          != BRA::ATTR_VALUE
			  &&
			     typed_data->
			         start_previous->type
		          != BRA::ATTR_MULTIVALUE )
		{
		    ::finish_value
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
		break;  // Goto NEXT_TOKEN
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
		break;  // Goto NEXT_TOKEN
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
		    min::phrase_position & pos =
		      typed_data->negator_position;
		    pos.end = current->previous
				     ->position.end;
		    pos.begin = PAR::remove
			( parser, current,
			  root->label );
		    typed_data->subtype =
			BRA::TYPED_ATTR_NEGATOR;
		    typed_data->start_previous =
			current->previous;
		    break;  // Goto NEXT_TOKEN
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

    MIN_ABORT ( "SHOULD NOT COME HERE" );
}

// Bracketed Compact Functions
// --------- ------- ---------

void BRA::compact_logical_line
	( PAR::parser parser,
	  PAR::pass pass,
	  PAR::table::flags selectors,
	  PAR::token & first, PAR::token next,
          const min::position & separator_found,
	  TAB::root line_sep,
	  TAB::flags trace_flags )
{
    min::phrase_position position =
	{ first->position.begin,
	  next->previous->position.end };

    PAR::attr attributes[2];
    unsigned n = 0;
    attributes[n++] =
	PAR::attr ( min::dot_initiator,
		    min::LOGICAL_LINE() );

    if ( separator_found )
    {
	attributes[n++] =
	    PAR::attr ( min::dot_terminator,
			line_sep->label );
	position.end = separator_found;
    }
    else
	attributes[n++] =
	    PAR::attr ( min::dot_terminator,
			PARLEX::new_line );

    PAR::compact
	( parser, pass, selectors,
	  first, next, position,
	  trace_flags, PAR::BRACKETING,
	  n, attributes );

    PAR::value_type_ref(first) =
        min::LOGICAL_LINE();
}

void BRA::compact_paragraph
	( PAR::parser parser,
	  PAR::token & first, PAR::token next,
	  TAB::flags trace_flags )
{
    MIN_REQUIRE
        ( first->value_type == PARLEX::paragraph );
    if ( first->next == next ) return;

    min::phrase_position position =
	{ first->position.begin,
	  next->previous->position.end };

    {
        // Block that closes vp at its end so trace
	// at end of function will work.
	//
	min::obj_vec_insptr vp ( first->value);
	min::attr_ptr ap ( vp );
	min::locate ( ap, min::dot_position );
	min::phrase_position_vec_insptr ppvec =
	    (min::phrase_position_vec_insptr)
	    min::get ( ap );
	MIN_REQUIRE ( ppvec != min::NULL_STUB );

	ppvec->position = position;
	first->position = position;

	PAR::token current = first->next;
	while ( current != next )
	{
	    MIN_REQUIRE
	        ( current->type != PAR::PREFIX
		  &&
		  current->type != PAR::MAPPED_PREFIX );
	    if ( current->value_type == PARLEX::line
		 ||
		    current->value_type
		 == min::LOGICAL_LINE()
	       )
	    {
		min::attr_push(vp) = current->value;
		min::push(ppvec) = current->position;
	    }
	    else
		PAR::parse_error
		    ( parser, current->position,
		      " value; `",
		      min::pgen_never_quote
			  ( current->value ),
		      "' does not have LOGICAL_LINE"
		      " .initiator or .type with `line'"
		      " group; ignored" );

	    current = current->next;
	    PAR::free
		( PAR::remove
		      ( PAR::first_ref(parser),
			current->previous ) );
	}
    }

    PAR::trace_subexpression
        ( parser, first, trace_flags );
}

void BRA::compact_prefix_list
	( PAR::parser parser,
	  PAR::pass pass,
	  PAR::table::flags selectors,
	  PAR::token first,
	  PAR::token next,
          const min::position & separator_found,
	  min::gen separator,
	  TAB::flags trace_flags )
{
    first->position.end =
	next->previous->position.end;
    PAR::token current = first->next;

    PAR::execute_pass_parse
	 ( parser, pass, selectors,
	   current, next );

    min::obj_vec_insptr vp
	( first->value );
    min::locatable_var
	    <min::phrase_position_vec_insptr>
	pos;

    bool merge_purelist =
        ( current->next == next
	  &&
	  current->type == PAR::PURELIST );

    bool implied =
        ( first->type == PAR::IMPLIED_PREFIX
	  ||
	  first->type == PAR::IMPLIED_HEADER );

    min::uns32 size = 0;
        // Number vector elements to add to prefix.
    min::obj_vec_ptr vp2;
        // vp to purelist, or NULL.
    if ( merge_purelist )
    {
        vp2 = current->value;
	size += min::attr_size_of ( vp2 );
    }
    else for ( PAR::token t = current;
	       t != next; t = t->next )
	    ++ size;

    if ( implied )
    {
	PAR::value_ref(first) =
	    min::copy
	        ( vp,   size
		      + ( separator_found ? 5 : 0 ) );
	vp = first->value;
    }

    min::attr_insptr ap ( vp );

    if ( separator_found )
    {
	min::locate ( ap, min::dot_terminator );
	min::set ( ap, separator );
	first->position.end = separator_found;
    }

    if ( merge_purelist )
    {
	min::attr_ptr ap2 ( vp );
	min::locate ( ap, min::dot_position );
	pos = min::get ( ap );
	for ( min::uns32 i = 0;
	      i < min::attr_size_of ( vp2 ); ++ i )
	    min::attr_push(vp) = min::attr ( vp2, i );
	current = current->next;
	PAR::free
	    ( PAR::remove
		  ( PAR::first_ref (parser),
		    current->previous ) );
    }
    else
    {
	min::locate ( ap, min::dot_position );
	if ( implied )
	{
	    min::init ( pos, parser->input_file,
			first->position, size );
	    min::set ( ap, min::new_stub_gen ( pos ) );
	    min::set_flag
		( ap, min::standard_attr_hide_flag );
	}
	else
	    pos = min::get ( ap );

	while ( current != next )
	{
	    if ( LEXSTD::must_convert
	             ( current->type ) )
		PAR::convert_token ( current );

	    if ( min::is_attr_legal ( current->value ) )
	    {
		min::attr_push(vp) = current->value;
		min::push ( pos ) = current->position;
	    }
	    else
		PAR::parse_error
		    ( parser, current->position,
		      "not a legal object element"
		      " value; `",
		      min::pgen_never_quote
			  ( current->value ),
		      "'; ignored" );

	    current = current->next;
	    PAR::free
		( PAR::remove
		      ( PAR::first_ref (parser),
			current->previous ) );
	}
    }

    pos->position = first->position;
    first->type = PAR::BRACKETED;

    vp = min::NULL_STUB;
        // To allow trace to print object.
    PAR::trace_subexpression
        ( parser, first, trace_flags );
}

// Untyped Bracketed Reformatters
// ------- --------- ------------

static bool label_reformatter_function
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
    if ( ! PAR::make_label_or_value
               ( parser, first, next, PAR::DATA_MODE ) )
    {
	PAR::value_ref(first) = min::ERROR();
        PAR::parse_error
	    ( parser, position,
	      "evaluated to ERROR special value because"
	      " of error(s) in label elements" );
    }
    first->position = position;

    PAR::trace_subexpression
	( parser, first, trace_flags );

    return false;
}

// Copy of min::standard_special_names but with min::gen
// string elements instead of min::ustring elements.
//
//    if v = MUP::new_special_gen ( i )
//    then ::special_names[0x1000000 - i] is name of v
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
    min::push ( gnames ) = min::UNUSED();
        // For unames[0] which is never used.
    for ( unsigned i = 1; i < unames->length; ++ i )
        // Skip unames[0].
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
	  min::gen line_separator,
	  TAB::flags trace_flags,
	  TAB::root entry )
{
    PAR::make_label_or_value
        ( parser, first, next, PAR::LABEL_MODE );

    unsigned i;
    for ( i = 1; i < ::special_names->length; ++ i )
    {
        if ( first->value == ::special_names[i] )
	    break;
    }

    if ( i < ::special_names->length )
    {
        value_ref(first) = MUP::new_special_gen
			       ( 0x1000000 - i );
	goto DONE;
    }
    else
    {
        min::lab_ptr labp ( first->value );
	if ( labp == min::NULL_STUB
	     ||
	     min::lablen ( labp ) != 2
	     ||
	     labp[0] != ::SPECIAL
	     ||
	     ! min::is_num ( labp[1] ) )
	    goto ERROR;

	min::float64 f = min::float_of ( labp[1] );
	if (    f < -0x1000000 || f >= 0x1000000
	     || (int) f != f )
	    goto ERROR;
	int i = (int) f;
	if ( i < 0 ) i += 0x1000000;
        value_ref(first) = MUP::new_special_gen ( i );
	goto DONE;
    }
	        
ERROR:
    PAR::parse_error
	( parser, first->position,
	  "subexpression ",
	  min::pgen_quote ( first->value ),
	  " unrecognized special name;"
	  " changed to ERROR" );

    value_ref(first) = min::ERROR();

DONE:
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
	  min::gen line_separator,
	  TAB::flags trace_flags,
	  TAB::root entry )
{
    BRA::opening_bracket opening_bracket =
        (BRA::opening_bracket) entry;
    min::obj_vec_ptr args
        ( opening_bracket->reformatter_arguments );
    MIN_REQUIRE ( min::size_of ( args ) == 1 );
    min::gen separator = args[0];

    min::unsptr count = 0;
    PAR::token start = first;
    PAR::token t = first;
    while ( true )
    {
        if ( t == next || t->value == separator )
	{
	    if ( start != t )
	    {
	        if ( start->next != t )
		    PAR::make_label_or_value
		        ( parser, start, t,
			  PAR::VALUE_MODE );
	        else if ( LEXSTD::must_convert
			      ( start->type ) )
		    start->type = PAR::DERIVED;
		    // This keeps token from begin
		    // converted.
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

static bool text_reformatter_function
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
    BRA::opening_bracket opening_bracket =
	(BRA::opening_bracket) entry;
    min::obj_vec_ptr args
        ( opening_bracket->reformatter_arguments );

    bool terminator_found = false;
    min::uns32 size = 0;
    for ( PAR::token t = first; t != next; t = t->next )
    { 
	++ size;
	if (    terminator_found
	     || ! PAR::is_lexeme ( t->type ) )
	    continue;
	for ( min::unsptr i = 1;
	         ! terminator_found
	      && i < min::size_of ( args );
	      ++ i )
	    terminator_found = ( args[i] == t->value );
    }

    if ( ! terminator_found )
        return true;

    PAR::token prefix =
	PAR::new_token ( PAR::PREFIX );
    PAR::put_before
	( PAR::first_ref(parser),
	  first, prefix );
    prefix->position.begin =
	first->position.begin;
    prefix->position.end =
	first->position.begin;

    PAR::value_ref(prefix) = min::new_obj_gen ( 3, 1 );
    min::obj_vec_insptr vp ( prefix->value );
    min::attr_insptr ap ( vp );

    min::locate ( ap, min::dot_type );
    min::set ( ap, args[0] );

    min::locatable_var
	    <min::phrase_position_vec_insptr>
	pos;
    min::init ( pos, parser->input_file,
		first->position, size );
    min::locate ( ap, min::dot_position );
    min::set ( ap, min::new_stub_gen ( pos ) );
    min::set_flag ( ap, min::standard_attr_hide_flag );

    vp = min::NULL_STUB;

    BRA::bracketed_pass bracketed_pass =
	(BRA::bracketed_pass) parser->pass_stack;
    TAB::key_table bracket_type_table =
	bracketed_pass->bracket_type_table;
    BRA::bracket_type prefix_entry =
	(BRA::bracket_type)
	TAB::find
	    ( args[0],
	      BRA::BRACKET_TYPE,
	      selectors,
	      bracket_type_table );
    PAR::value_type_ref(prefix) =
	min::new_stub_gen ( prefix_entry );

    first = prefix;

    TAB::flags prefix_selectors = selectors;
    if ( prefix_entry != min::NULL_STUB )
    {
	prefix_selectors = selectors;
	    TAB::modified_flags
		( selectors,
		  prefix_entry->parsing_selectors );

	if (    prefix_entry->reformatter
	     != min::NULL_STUB )
	{
	    min::phrase_position position =
		{ prefix->position.begin,
		  next->previous->position.end };

	    if ( ! ( * prefix_entry->reformatter->
			 reformatter_function )
		       ( parser, pass,
			 selectors,
			 prefix, next, position,
			 min::MISSING(),
			 trace_flags,
			 (TAB::root) prefix_entry )
	       )

		return true;
	 }
    }

    BRA::compact_prefix_list
	( parser, pass,
	  prefix_selectors,
	  first, next,
	  min::MISSING_POSITION,
	  min::MISSING(),
	  trace_flags );

    return true;

}

min::locatable_var<PAR::reformatter>
    BRA::untyped_reformatter_stack ( min::NULL_STUB );

static void untyped_reformatter_stack_initialize
	( void )
{
    ::initialize();

    min::locatable_gen label
        ( min::new_str_gen ( "label" ) );
    PAR::push_reformatter
        ( label, 0, 0,
	  ::label_reformatter_function,
	  BRA::untyped_reformatter_stack );

    min::locatable_gen special
        ( min::new_str_gen ( "special" ) );
    PAR::push_reformatter
        ( special, 0, 0,
	  ::special_reformatter_function,
	  BRA::untyped_reformatter_stack );

    min::locatable_gen multivalue
        ( min::new_str_gen ( "multivalue" ) );
    PAR::push_reformatter
        ( multivalue, 1, 1,
	  ::multivalue_reformatter_function,
	  BRA::untyped_reformatter_stack );

    min::locatable_gen text
        ( min::new_str_gen ( "text" ) );
    PAR::push_reformatter
        ( text, 1, 1000,
	  ::text_reformatter_function,
	  BRA::untyped_reformatter_stack );
}
static min::initializer untyped_reformatter_initializer
    ( ::untyped_reformatter_stack_initialize );

// Typed Bracketed Reformatters
// ----- --------- ------------

min::locatable_var<PAR::reformatter>
    BRA::bracket_type_reformatter_stack
        ( min::NULL_STUB );

const unsigned CLEAR_SIGN = 0;
const unsigned ASSIGN_SIGN = 1;
const unsigned INITIATORS = 2;
const unsigned EQUAL_SIGN = 3;
const unsigned NEGATOR = 4;
const unsigned FLAGS_OPENING = 5;
const unsigned MULTIVALUE_OPENING = 6;
const unsigned ARGS_LENGTH = 7;

inline bool delete_subexpression
        ( PAR::parser parser,
	  PAR::token & first, PAR::token next )
{
    do {
	first = first->next;
	PAR::free
	    ( PAR::remove ( PAR::first_ref(parser),
			    first->previous ) );
    } while ( first != next );

    return false;
        
}
static bool data_reformatter_function
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
    if ( parser->id_map == min::NULL_STUB )
        return true;
    BRA::bracket_type prefix_entry =
        (BRA::bracket_type) entry;
    min::obj_vec_ptr args
        ( prefix_entry->reformatter_arguments );
    MIN_REQUIRE
        ( min::size_of ( args ) == ARGS_LENGTH );
    MIN_REQUIRE ( first != next );

    // If prefix has attributes other than .type and
    // .position, return true.
    //
    {
        min::obj_vec_ptr pvp ( first->value );
	min::attr_ptr pap ( pvp );
        min::attr_info info[2];
	min::unsptr n =
	    min::attr_info_of ( info, 2, pap );
	if ( n > 2 ) return true;
	for ( unsigned i = 0; i < n; ++ i )
	{
	    if ( info[i].name != min::dot_type
	         &&
		 info[i].name != min::dot_position )
		return true;
	    if ( info[i].reverse_attr_count > 0 )
	        return true;
        }
    }

    if ( first->next == next ) return true;
    if ( first->next->value == args[CLEAR_SIGN] )
    {
	if ( first->next->next != next ) return true;
	min::map_clear_input ( parser->id_map );
	return delete_subexpression
	    ( parser, first, next );
    }
    if ( first->next->next == next ) return true;
    if ( first->next->next->value != args[ASSIGN_SIGN] )
        return true;

    min::locatable_gen ID_gen ( first->next->value );
    if ( min::is_obj ( ID_gen ) )
    {
        min::obj_vec_ptr ID_vp ( ID_gen );
	if ( min::size_of ( ID_vp ) > 0 )
	{
	    PAR::parse_error
		( parser, position,
		  "ID references an object that already"
		  " has elements; definition ignored" );
	    return delete_subexpression
	        ( parser, first, next );
	}
	min::attr_ptr ID_ap ( ID_vp );
	min::attr_info info;
	min::unsptr A = min::attr_info_of
	    ( & info, 1, ID_ap, false );
	if ( A > 1
	     ||
	     (    A == 1
	       && info.name != min::dot_position ) )
	{
	    PAR::parse_error
		( parser, position,
		  "ID references an object that already"
		  " has single-attribute-values or"
		  " attribute-flags, other than for"
		  " .position; definition ignored" );
	    return delete_subexpression
	        ( parser, first, next );
	}
    }
    else if ( ! ( first->next->type == PAR::DERIVED
                  &&
		  min::is_preallocated ( ID_gen  ) ) )
        return true;

    PAR::token vector_end = next;
    min::gen v = next->previous->value;
    if ( min::is_obj ( v )
         &&
	    min::get ( v, min::dot_terminator )
	 == min::INDENTED_PARAGRAPH() )
    {
	min::gen initiator =
	    min::get ( v, min::dot_initiator );
	min::obj_vec_ptr ip = args[INITIATORS];
	MIN_REQUIRE ( ip != min::NULL_STUB );
	for ( min::unsptr i = 0;
	      i < min::size_of ( ip );
	      ++ i )
	{
	    if ( ip[i] == initiator )
	    {
		vector_end = next->previous;
		break;
	    }
	}
    }

    min::position end_position =
        next->previous->position.end;

    PAR::free ( PAR::remove ( first_ref(parser),
			      first->next  ) );
    PAR::free ( PAR::remove ( first_ref(parser),
			      first->next  ) );

    BRA::compact_prefix_list
        ( parser, pass, selectors, first, vector_end,
	  min::MISSING_POSITION, min::MISSING(),
	  trace_flags );

    min::obj_vec_ptr fvp ( first->value );
    min::unsptr fvpsize = min::size_of ( fvp );

    // If value has one element that is an object,
    // ID_gen is preallocated, and there are no indented
    // paragraph attributes, then replace the value by
    // its sole element and finish up.
    //
    if (    fvpsize == 1
         && vector_end == next
	 && min::is_preallocated ( ID_gen )
	 && min::is_obj ( fvp[0] ) )
    {
	PAR::value_ref(first) = fvp[0];
	fvp = min::NULL_STUB;
	PAR::trace_subexpression
	    ( parser, first, trace_flags );

	min::copy ( ID_gen, first->value, 0 );

	first = first->next;
	PAR::free ( PAR::remove ( first_ref(parser),
				  first->previous ) );
	return false;
    }

    min::attr_ptr fap ( fvp );
    locate ( fap, min::dot_position );
    min::phrase_position_vec_insptr idppvec =
	min::phrase_position_vec_insptr
            ( min::get ( fap ) );

    min::unsptr asize = 0;
    if ( vector_end != next )
    {
	idppvec->position.end = end_position;
        min::obj_vec_ptr paragraph ( vector_end->value);
	asize = min::size_of ( paragraph );
    }

    if ( min::is_preallocated ( ID_gen ) )
	min::new_obj_gen
	    ( ID_gen, 5 + fvpsize + 5 * asize,
	              4 + 2 * asize );

    min::obj_vec_insptr idvp ( ID_gen );
    for ( min::unsptr i = 0; i < fvpsize; ++ i )
	min::attr_push ( idvp ) = fvp[i];


    min::attr_insptr idap ( idvp );
    locate ( idap, min::dot_position );
    min::set ( idap, min::new_stub_gen ( idppvec ) );
    min::set_flag
        ( idap, min::standard_attr_hide_flag );

    if ( vector_end != next )
    {
        min::obj_vec_ptr paragraph
	    ( vector_end->value );
	for ( min::uns32 i = 0;
	      i < min::size_of ( paragraph ); ++ i )
        {
	    MIN_REQUIRE
	        ( min::is_obj ( paragraph[i] ) );
	    min::obj_vec_ptr line ( paragraph[i] );
	    min::phrase_position_vec lppvec =
		min::position_of ( line );
	    min::uns32 lsize = min::size_of ( line );

	    if ( lsize == 0 ) continue;
	    min::uns32 j = 0;
	    bool has_negator = false;
	    if ( line[0] == args[NEGATOR] )
	    {
	        ++ j;
		has_negator = true;
	    }
	    min::locatable_gen name
		( PAR::scan_label_or_value
		      ( parser, line, j,
		        PAR::LABEL_MODE,
		        args[EQUAL_SIGN] ) );
	    if ( name == min::NONE() )
	    {
		PAR::parse_error
		    ( parser, lppvec->position,
		      "line does not begin with a"
		      " (possibly negated)"
		      " attribute label;"
		      " line ignored" );
		continue;
	    }

	    if ( j < lsize && has_negator )
	    {
		PAR::parse_error
		    ( parser,
		      lppvec[0],
		      "negator preceeds"
		      " attribute label"
		      " which is followed"
		      " by flags or value;"
		      " line ignored" );
		continue;
	    }

	    min::gen flags = min::MISSING();

	    const char * message =
	        "after attribute label `";
	    if ( j < lsize && min::is_obj ( line[j] ) )
	    {
	        min::obj_vec_ptr flagsvp ( line[j] );
		min::attr_ptr flagsap ( flagsvp );
		min::locate
		    ( flagsap, min::dot_initiator );
		if (    min::get ( flagsap )
		     == args[FLAGS_OPENING] )
		{
		    flags = line[j++];
		    message =
			"after attribute label flags `";
		}
	    }

	    if (    j < lsize
	         && line[j] != args[EQUAL_SIGN] )
	    {
		PAR::parse_error
		    ( parser, lppvec[j],
		      message,
		      min::pgen_never_quote
			  ( args[EQUAL_SIGN] ),
		      "' was expected but not"
		      " found; line ignored" );
		continue;
	    }

	    // Now if j < lsize, then
	    // line[j] == args[EQUAL_SIGN];
	    // j == lsize is also possible.
	    //
	    if ( j + 1 == lsize )
	    {
		PAR::parse_error
		    ( parser, lppvec[j],
		      "after `",
		      min::pgen_never_quote
			  ( args[EQUAL_SIGN] ),
		      "' argument value was expected"
		      " but not found; line ignored" );
		continue;
	    }

	    min::locatable_gen value
	        ( has_negator ? min::FALSE
		              : min::TRUE );
	    bool has_value = false;
	    bool is_multivalue = false;
	    min::locatable_gen reverse_name
		( min::MISSING() );
	    min::phrase_position pos;
	    if ( j + 2 == lsize
	         ||
	    	 (    j + 2 < lsize
	           && line[j+2] == args[EQUAL_SIGN] ) )
	    {
	        // Single token value, including
		// possible multi-value bracketed
		// list.  Double arrow attribute
		// is possible.
		//
		has_value = true;
		pos = lppvec[++j];
		value = line[j++];
		is_multivalue =
		    (    min::get ( value,
		                    min::dot_initiator )
		      == args[MULTIVALUE_OPENING] );

		min::gen v =
		    PAR::quoted_string_value ( value );
		if ( v != min::NONE() ) value = v;

		if ( j < lsize )
		{
		    // Double arrow attribute.  Syntax:
		    //   name = value(s) = reverse_name
		    // line[j] == args[EQUAL_SIGN].
		    //
		    MIN_REQUIRE
		      ( line[j] == args[EQUAL_SIGN] );
		    min::uns32 saved_j = j ++;
		    reverse_name =
			( PAR::scan_label_or_value
			      ( parser, line, j,
			        PAR::LABEL_MODE ) );
		    if (    reverse_name
		         == min::NONE() )
		    {
			PAR::parse_error
			    ( parser, lppvec[saved_j],
			      "reverse attribute label"
			      " after second `",
			      min::pgen_never_quote
				  ( args[EQUAL_SIGN] ),
			      "' was expected but not"
			      " found; line ignored" );
			continue;
		    }
		}
	    }
	    else if ( j + 2 < lsize )
	    {
	        // Multi-token value, i.e., a MIN label.
		//
		has_value = true;
		min::uns32 saved_j = j ++;
		value = PAR::scan_label_or_value
		    ( parser, line, j,
		      PAR::VALUE_MODE );
		if ( value == min::NONE() )
		{
		    PAR::parse_error
			( parser, lppvec[saved_j],
			  "after `",
			  min::pgen_never_quote
			      ( args[EQUAL_SIGN] ),
			  "' attribute value (label or"
			  " single bracketed"
			  " subexpression) was"
			  " expected but none found;"
			  " line ignored" );
		    continue;
		}
		pos = lppvec[saved_j];
		pos.end = (~lppvec[j-1]).end;
	    }
	    else if ( j == lsize
	              &&
		      flags == min::MISSING() )
	    {
	        has_value = true;
	        pos = lppvec->position;
	    }

	    if ( j < lsize )
	    {
		min::phrase_position pos =
		    { (&lppvec[j])->begin,
		      (&lppvec[lsize-1])->end };
		PAR::parse_error
		    ( parser, pos,
		      "extra stuff at end of line;"
		      " line ignored" );
		continue;
	    }

	    min::locate ( idap, name );

	    if ( flags != min::MISSING() )
		PAR::set_attr_flags
		    ( parser, idap, flags,
		      PAR::NEW_OR_SAME );

	    if ( ! has_value ) continue;

	    unsigned option;
	    if ( reverse_name != min::MISSING() )
	    {
	        min::locate_reverse
		    ( idap, reverse_name );
		option = PAR::ADD_TO_SET;
	    }
	    else
	    {
		// M is flag 37 + 13 = 50
		// S is flag 37 + 19 = 56
		#define MI ( 50 / min::VSIZE )
		#define MO ( 50 % min::VSIZE )
		#define SI ( 56 / min::VSIZE )
		#define SO ( 56 % min::VSIZE )
		min::gen flag_vector[3];
		unsigned c = min::get_flags
		    ( flag_vector, 3, idap );
		if ( c > MI
		     &&
		     (   MUP::value_of
		             ( flag_vector[MI] )
		       & ( min::unsgen ( 1 ) << MO ) ) )
		    option = PAR::ADD_TO_MULTISET;
		else if ( c > SI
		          &&
		          (   MUP::value_of
			          ( flag_vector[MI] )
			    & (    min::unsgen ( 1 )
			        << SO ) ) )
		    option = PAR::ADD_TO_SET;
		else
		    option = PAR::NEW_OR_SAME;
	    }

	    if ( ! is_multivalue )    
		PAR::set_attr_value
		    ( parser, idap, value, pos,
		      option );
	    else
		PAR::set_attr_multivalue
		    ( parser, idap, value,
		      PAR::ADD_TO_MULTISET );
	}
	
	PAR::free
	    ( PAR::remove ( first_ref(parser),
			    next->previous ) );
    }

    PAR::value_ref(first) = ID_gen;
    first->position.end = end_position;
    idvp = min::NULL_STUB;
    PAR::trace_subexpression
	( parser, first, trace_flags );

    return delete_subexpression ( parser, first, next );
}

static bool sentence_reformatter_function
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
    BRA::bracket_type prefix_entry =
        (BRA::bracket_type) entry;
    min::obj_vec_ptr args
        ( prefix_entry->reformatter_arguments );
    MIN_REQUIRE ( args != min::NULL_STUB );

    bool terminator_found = false;
    for ( PAR::token t = first;
          ! terminator_found && t != next;
	  t = t->next )
    {
	if ( ! PAR::is_lexeme ( t->type ) )
	    continue;
	for ( min::unsptr i = 0;
	         ! terminator_found
	      && i < min::size_of ( args );
	      ++ i )
	    terminator_found = ( args[i] == t->value );
    }

    if ( ! terminator_found )
        return true;

    for ( PAR::token t = first->next; t != next;
                                      t = t->next )
    {
	if ( ! PAR::is_lexeme ( t->type ) )
	    continue;

	min::unsptr i = 0;
	while (    i < min::size_of ( args )
	        && args[i] != t->value )
	    ++ i;
	if ( i >= min::size_of ( args ) ) continue;

	min::position separator_found =
	    t->position.end;
	PAR::token next_first = min::NULL_STUB;
	if ( t->next != next )
	{
	    next_first = t;
	    next_first->position.begin =
	    next_first->position.end =
		separator_found;
	    PAR::value_ref(next_first) =
		first->value;
	    PAR::value_type_ref(next_first) =
		first->value_type;
	    next_first->type = first->type;
	    first->type = PAR::IMPLIED_PREFIX;
	}

	BRA::compact_prefix_list
	    ( parser, pass, selectors, first, t,
	      separator_found, args[i],
	      trace_flags );

	first = next_first;
    }
    if ( first == min::NULL_STUB )
	PAR::free
	    ( PAR::remove ( first_ref(parser),
			    next->previous ) );
    else
    {
	BRA::compact_prefix_list
	    ( parser, pass, selectors, first, next,
	      min::MISSING_POSITION, min::MISSING(),
	      trace_flags );
    }
    return false;
}

static void bracket_type_reformatter_stack_initialize
	( void )
{
    ::initialize();

    min::locatable_gen data_name
        ( min::new_str_gen ( "data" ) );
    PAR::push_reformatter
        ( data_name, 7, 7,
	  ::data_reformatter_function,
	  BRA::bracket_type_reformatter_stack );

    min::locatable_gen sentence_name
        ( min::new_str_gen ( "sentence" ) );
    PAR::push_reformatter
        ( sentence_name, 0, 1000,
	  ::sentence_reformatter_function,
	  BRA::bracket_type_reformatter_stack );
}
static min::initializer
	bracket_type_reformatter_initializer
    ( ::bracket_type_reformatter_stack_initialize );


// Bracketed Pass Command Function
// --------- ---- ------- --------

enum definition_type
    { BRACKET,
      INDENTATION_MARK,
      TYPED_BRACKET,
      BRACKET_TYPE };

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

    if ( command != PARLEX::define
         &&
	 command != PARLEX::undefine
         &&
	 command != PARLEX::print )
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
        if ( command == PARLEX::print )
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
				<< ": " << offset;

		if ( i == 0 ) break;

		BRA::block_struct b =
		    bracketed_pass->block_stack[i-1];
		offset = b.indentation_offset;
	    }

	    parser->printer << min::eom;
	    return PAR::PRINTED;
	}
	else if ( command != PARLEX::define )
	    return min::FAILURE();

	else if ( i + 2 >= size )
	    return PAR::parse_error
		( parser, ppvec[i+1],
		  "expected integer after" );
	else
	{
	    if ( ! min::is_num ( vp[i+2] ) )
		return PAR::parse_error
		    ( parser, ppvec[i+2],
		      "expected integer" );
	    min::float64 offset =
	        min::float_of ( vp[i+2] );
	    if ( ! std::isfinite ( offset )
	         ||
		 offset < 1
		 ||
		 offset > 100
		 ||
		 (min::int32) offset != offset )
		return PAR::parse_error
		    ( parser, ppvec[i+2],
		      "expected integer in range"
		      " [1,100]" );
	    if ( i + 3 < size )
		return PAR::parse_error
		    ( parser, ppvec[i+2],
		      "unexpected stuff after" );
	    bracketed_pass->indentation_offset =
	        (min::int32) offset;
	}

	return min::SUCCESS();
    }

    if ( vp[i] == ::quoted_lexeme
         &&
	 i + 2 < size
	 &&
	 vp[i+1] == ::string_lexeme
	 &&
	 vp[i+2] == ::concatenator )
    {
        if ( command == PARLEX::print )
	{
	    if ( i + 3 < size )
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
	else if ( command != PARLEX::define )
	    return min::FAILURE();

	else if ( i + 3 >= size )
	    return PAR::parse_error
		( parser, ppvec[i+1],
		  "expected string concatenator"
		  " after" );

	i += 3;
	min::locatable_gen concat = vp[i];
	if ( concat == PARLEX::enabled )
	    concat = min::ENABLED();
	else if ( concat == PARLEX::disabled )
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

    if ( vp[i] == ::middle_lexeme
         &&
	 i + 1 < size
	 &&
	 vp[i+1] == ::break_lexeme )
    {
        if ( command == PARLEX::print )
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

	    BRA::middle_break middle_break =
	        bracketed_pass->middle_break; 
	    for ( min::uns32 i =
	              bracketed_pass->
		          block_stack->length;
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
				<< ": ";
		if ( middle_break.begin_length == 0 )
		    parser->printer << "disabled";
		else
		{
		    min::print_cstring
		        ( parser->printer,
			  middle_break.begin,
			  min::quote_all_str_format );
		    parser->printer << " ... ";
		    min::print_cstring
		        ( parser->printer,
			  middle_break.end,
			  min::quote_all_str_format );
		}

		if ( i == 0 ) break;

		BRA::block_struct b =
		    bracketed_pass->block_stack[i-1];
		middle_break = b.middle_break;
	    }

	    parser->printer << min::eom;
	    return PAR::PRINTED;
	}
	else if ( command != PARLEX::define )
	    return min::FAILURE();

	else if ( i + 2 >= size )
	    return PAR::parse_error
		( parser, ppvec[i+1],
		  "expected middle break name"
		  " after" );

	i += 2;

	BRA::middle_break middle_break;

	if ( vp[i] == PARLEX::disabled )
	{
	    middle_break.begin_length =
	    middle_break.end_length = 0;
	}
	else
	{
	    min::gen begin_name =
	        PAR::scan_quoted_string ( vp, i );
	    if ( begin_name == min::MISSING()
	         ||
		 PAR::lexical_type_of ( begin_name )
		 != LEXSTD::mark_t )
		return PAR::parse_error
		    ( parser, ppvec[i],
		      "expected quoted mark" );
	    middle_break.begin_length =
	        min::strlen ( begin_name );
	    if (   middle_break.begin_length
	         > sizeof ( middle_break.begin ) - 1 )
		return PAR::parse_error
		    ( parser, ppvec[i],
		      "too many characters in" );
	    min::strcpy ( middle_break.begin,
	                  begin_name );

	    if ( i >= size
		 ||
		 vp[i] != PARLEX::dotdotdot )
		return PAR::parse_error
		    ( parser, ppvec[i],
		      "expected ... after" );
	    ++ i;
	    min::gen end_name =
	        PAR::scan_quoted_string ( vp, i );
	    if ( end_name == min::MISSING()
	         ||
		 PAR::lexical_type_of ( end_name )
		 != LEXSTD::mark_t )
		return PAR::parse_error
		    ( parser, ppvec[i],
		      "expected quoted mark" );
	    middle_break.end_length =
	        min::strlen ( end_name );
	    if (   middle_break.end_length
	         > sizeof ( middle_break.end ) - 1 )
		return PAR::parse_error
		    ( parser, ppvec[i],
		      "too many characters in" );
	    min::strcpy ( middle_break.end,
	                  end_name );
	}

	if ( i + 1 < size )
	    return PAR::parse_error
		( parser, ppvec[i],
		  "unexpected stuff after" );

	bracketed_pass->middle_break = middle_break;

	return min::SUCCESS();
    }

    if ( vp[i] == ::bracket )
    {
	if ( i + 1 < size && vp[i + 1] == ::type )
	{
	    type = ::BRACKET_TYPE;
	    min_names = 1;
	    max_names = 1;
	    i += 2;
	}
        else
	{
	    type = ::BRACKET;
	    min_names = max_names =
		( command == PARLEX::print ? 1 : 2 );
	    ++ i;
	}
    }
    else if ( vp[i] == ::indentation
              &&
	      i + 1 < size
	      &&
	      vp[i + 1] == ::mark )
    {
	type = ::INDENTATION_MARK;
	min_names = 1;
	max_names =
	    ( command == PARLEX::print ? 1 : 2 );
	i += 2;
    }
    else if ( vp[i] == ::typed
              &&
	      i + 1 < size
	      &&
	      vp[i + 1] == ::bracket )
    {
	type = ::TYPED_BRACKET;
	if ( command == PARLEX::print )
	    min_names = max_names = 1;
	else
	    min_names = 2, max_names = 4;
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
	SCAN_NAME ( name[number_of_names],
	            command == PARLEX::print );

	++ number_of_names;

	if ( number_of_names > max_names )
	    return PAR::parse_error
	        ( parser, ppvec->position,
		  "too many quoted names in" );

	if ( i >= size
	     ||
	     vp[i] != PARLEX::dotdotdot )
	    break;

	++ i;
    }

    if ( number_of_names < min_names )
	return PAR::parse_error
	    ( parser, ppvec->position,
	      "too few quoted names in" );

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

	TAB::key_table_iterator it
	    ( type == ::BRACKET_TYPE ?
	      bracketed_pass->bracket_type_table :
	      bracketed_pass->bracket_table );
	while ( true )
	{
	    TAB::root root = it.next();
	    if ( root == min::NULL_STUB ) break;

	    if ( min::is_subsequence
		     ( name[0], root->label ) < 0 )
		continue;

	    min::uns32 subtype =
		min::packed_subtype_of ( root );

	    if ( (    type == ::BRACKET
	           && subtype != BRA::OPENING_BRACKET )
		 ||
		 (    type == ::TYPED_BRACKET
		   && subtype != BRA::TYPED_OPENING )
		 ||
		 (    type == ::INDENTATION_MARK
		   && subtype != BRA::INDENTATION_MARK )
		 ||
		 (    type == ::BRACKET_TYPE
		   && subtype != BRA::BRACKET_TYPE ) )
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
		    << ( type == ::BRACKET ?
		         "bracket " : "typed bracket " )
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

		TAB::new_flags parsing_selectors =
		    opening_bracket->parsing_selectors;

		TAB::flags all_flags =
		    TAB::all_flags
		         ( parsing_selectors,
			   0,
			   BRA::BRACKET_OFF_SELECTORS
			   +
			   BRA::BRACKET_OFF_OPT );

		if ( all_flags
		     &
		     BRA::BRACKET_SELECTORS )
		{
		    parser->printer
			<< min::indent
			<< "with parsing"
			   " selectors ";
		    COM::print_new_flags
			( parsing_selectors,
			  BRA::BRACKET_SELECTORS,
			  0, BRA::BRACKET_OFF_SELECTORS,
			  parser->selector_name_table,
			  parser, true );
		}

		if ( all_flags
		     &
		     BRA::BRACKET_OPT )
		{
		    parser->printer
			<< min::indent
			<< "with parsing"
			   " options ";
		    COM::print_new_flags
			( parsing_selectors,
			  BRA::BRACKET_OPT,
			  0, BRA::BRACKET_OFF_OPT,
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

		    min::obj_vec_ptr args
			( opening_bracket->
			      reformatter_arguments );
		    if ( args != min::NULL_STUB )
		    {
			parser->printer
			    << " ( " << min::set_break;
			for ( min::uns32 i = 0;
			      i < min::size_of ( args );
			      ++ i )
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
		    if (    typed_opening->
		                prefix_selectors
		         != PAR::ALL_SELECTORS )
		    {
			parser->printer
			    << min::indent
			    << "with prefix"
			       " selectors ";
			COM::print_flags
			    ( typed_opening->
				  prefix_selectors,
			      PAR::COMMAND_SELECTORS,
			      parser->
			          selector_name_table,
			      parser );
		    }
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

		TAB::new_flags parsing_selectors =
		    indentation_mark->parsing_selectors;

		TAB::flags all_flags =
		  TAB::all_flags
		    ( parsing_selectors,
		      BRA::
		       INDENTATION_MARK_ON_SELECTORS,
		      BRA::
		       INDENTATION_MARK_OFF_SELECTORS );

		if ( all_flags
		     &
		     BRA::INDENTATION_MARK_SELECTORS )
		{
		    parser->printer
			<< min::indent
			<< "with parsing"
			   " selectors ";
		    COM::print_new_flags
		      ( parsing_selectors,
			BRA::
			  INDENTATION_MARK_SELECTORS,
			BRA::
			  INDENTATION_MARK_ON_SELECTORS,
			BRA::
			 INDENTATION_MARK_OFF_SELECTORS,
			parser->
			  selector_name_table,
			parser, true );
		}

		if ( all_flags
		     &
		     BRA::INDENTATION_MARK_OPT )
		{
		    parser->printer
			<< min::indent
			<< "with parsing"
			   " options ";
		    COM::print_new_flags
			( parsing_selectors,
			  BRA::INDENTATION_MARK_OPT,
			  0,0,
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

		COM::print_lexical_master
		    ( parser,
		      indentation_mark->
		          paragraph_lexical_master,
		      indentation_mark->
		          line_lexical_master );
	    }
	    else if ( subtype == BRA::BRACKET_TYPE )
	    {
		BRA::bracket_type bracket_type =
		    (BRA::bracket_type) root;

		parser->printer
		    << "bracket type "
		    << min::pgen_quote
		           ( bracket_type->label );

		parser->printer
		    << " " << min::set_break;

		COM::print_flags
		    ( bracket_type->selectors,
		      PAR::COMMAND_SELECTORS,
		      parser->selector_name_table,
		      parser );

		TAB::new_flags parsing_selectors =
		    bracket_type->parsing_selectors;

		TAB::flags all_flags =
		  TAB::all_flags
		    ( parsing_selectors,
		      0,
		      BRA::BRACKET_OFF_SELECTORS );

		if ( all_flags
		     &
		     BRA::BRACKET_SELECTORS )
		{
		    parser->printer
		        << min::indent
			<< "with parsing"
			   " selectors ";
		    COM::print_new_flags
		      ( parsing_selectors,
			BRA::BRACKET_SELECTORS,
			0,
			BRA::BRACKET_OFF_SELECTORS,
			parser->selector_name_table,
			parser, true );
		}

		if ( all_flags
		     &
		     BRA::BRACKET_TYPE_OPT )
		{
		    parser->printer
			<< min::indent
			<< "with parsing"
			   " options ";
		    COM::print_new_flags
			( parsing_selectors,
			  BRA::BRACKET_TYPE_OPT,
			  0, 0,
			  parser->selector_name_table,
			  parser, true );
		}

		min::gen group = bracket_type->group;
		if ( group != min::MISSING() )
		    parser->printer
			<< min::indent
			<< "with group "
			<< min::pgen ( group );

		min::gen subprefix =
		    bracket_type->implied_subprefix;
		if ( subprefix != min::MISSING() )
		    parser->printer
			<< min::indent
			<< "with implied subprefix "
			<< min::pgen ( subprefix );

		COM::print_lexical_master
		    ( parser,
		      PAR::MISSING_MASTER,
		      bracket_type->
		          line_lexical_master );

		if (    bracket_type->reformatter
		     != min::NULL_STUB )
		{
		    parser->printer
			<< min::indent
			<< "with "
			<< min::pgen_name
			       ( bracket_type->
			             reformatter->name )
			<< " reformatter";

		    min::obj_vec_ptr args
			( bracket_type->
			      reformatter_arguments );
		    if ( args != min::NULL_STUB )
		    {
			parser->printer
			    << " ( " << min::set_break;
			for ( min::uns32 i = 0;
			      i < min::size_of ( args );
			      ++ i )
			{
			    if ( i != 0 )
				parser->printer
				    << ", "
				    << min::set_break;
			    if (    args[i]
			         != min::MISSING() )
				parser->printer
				    << min::pgen_quote
					   ( args[i] );
			}
			parser->printer << " )";
		    }
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
	                    << "nothing found";
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

    if (     type == ::INDENTATION_MARK
         &&  name[0] == PARLEX::star_top_level_star )
	return PAR::parse_error
	    ( parser, ppvec->position,
	      "(un)defining indentation mark named"
	      " *TOP* *LEVEL* is not permitted"
	      " (name is reserved to system)" );

    if ( command == PARLEX::define ) switch ( type )
    {
    case ::BRACKET:
    {
	TAB::new_flags new_selectors;
	TAB::new_flags new_options;
	    // Inited to zeroes.
	PAR::reformatter reformatter = min::NULL_STUB;
	min::locatable_gen reformatter_arguments;
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
		min::gen result =
		    COM::scan_new_flags
			( vp, i, new_selectors,
			  BRA::BRACKET_SELECTORS,
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
		 vp[i] == PARLEX::parsing
		 &&
		 vp[i+1] == PARLEX::options )
	    {
		i += 2;
		min::gen result =
		    COM::scan_new_flags
			( vp, i, new_options,
			  BRA::BRACKET_OPT,
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
			  PARLEX::reformatter ) );
		if (    name != min::NONE()
		     && j < size
		     &&    vp[j]
		        == PARLEX::reformatter )
		{
		    min::phrase_position position =
			{ (&ppvec[i])->begin,
			  (&ppvec[j])->end };
		    PAR::reformatter reformatter_stack =
			BRA::untyped_reformatter_stack;
		    reformatter =
			PAR::find_reformatter
			    ( name, reformatter_stack );
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

			min::unsptr s =
			    min::size_of ( args );
			if ( s < reformatter->
				     minimum_arguments )
			    return PAR::parse_error
				    ( parser, position,
				      "too few"
				      " reformatter"
				      " arguments" );
			if ( s > reformatter->
				     maximum_arguments )
			    return PAR::parse_error
				    ( parser, position,
				      "too many"
				      " reformatter"
				      " arguments" );
		    }
		}
		else
		    return PAR::parse_error
			( parser, ppvec[i-1],
			  "expected"
			  " `parsing selectors',"
			  " `parsing options',"
			  " or `... reformatter ...'"
			  " after" );
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
	min::uns32 paragraph_lexical_master =
	    PAR::MISSING_MASTER;
	min::uns32 line_lexical_master =
	    PAR::MISSING_MASTER;
	min::locatable_gen implied_header
	    ( min::MISSING() );
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
		min::gen result =
		    COM::scan_new_flags
			( vp, i, new_selectors,
			  BRA::
			    INDENTATION_MARK_SELECTORS,
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
		 vp[i] == PARLEX::parsing
		 &&
		 vp[i+1] == PARLEX::options )
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
	    if ( i + 1 < size
		 &&
		 vp[i] == PARLEX::implied
		 &&
		 vp[i+1] == PARLEX::header )
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
		      " header', or"
		      " `... lexical master'"
		      " after" );
	}
	if ( i < size )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "expected `with' after" );

	if (    paragraph_lexical_master
	     == PAR::MISSING_MASTER
	     &&
	        line_lexical_master
	     != PAR::MISSING_MASTER )
	{
	    PAR::parse_warn
		( parser, ppvec->position,
		  "line lexical master set but"
		  " paragraph lexical master is"
		  " missing; paragraph lexical master"
		  " set equal to line lexical master" );
	    paragraph_lexical_master =
	        line_lexical_master;
	}

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
	      paragraph_lexical_master,
	      line_lexical_master,
	      bracketed_pass->bracket_table );

	break;
    }
    case ::TYPED_BRACKET:
    {
	// Name errors are checked above.

	bool has_middle = ( number_of_names == 4 );

	TAB::new_flags new_parsing_selectors;
	TAB::flags new_attribute_selectors;
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
	TAB::flags new_prefix_selectors =
	    PAR::ALL_SELECTORS;

	while ( i < size && vp[i] == PARLEX::with )
	{
	    ++ i;
	    if ( i + 1 < size
		 &&
		 ( vp[i] == PARLEX::parsing
		   ||
		   vp[i] == ::attribute
		   ||
		   vp[i] == PARLEX::prefix )
		 &&
		 vp[i+1] == PARLEX::selectors )
	    {
		min::gen kind = vp[i];
		i += 2;
		min::gen result;
		if ( kind == PARLEX::parsing )
		    result = COM::scan_new_flags
			( vp, i, new_parsing_selectors,
			  BRA::BRACKET_SELECTORS,
	                  parser->selector_name_table,
			  parser->
			    selector_group_name_table,
			  parser, true );
		else
		    result = COM::scan_flags
			( vp, i,
			  kind == ::attribute ?
			  new_attribute_selectors :
			  new_prefix_selectors,
			  PAR::COMMAND_SELECTORS,
	                  parser->selector_name_table,
			  parser->
			    selector_group_name_table,
			  parser );
		if ( result == min::ERROR() )
		    return min::ERROR();
		else if ( result == min::FAILURE() )
		    return PAR::parse_error
			( parser, ppvec[i-1],
			  kind == PARLEX::parsing ?
			  "expected bracketed selector"
			  " modifier list after" :
			  "expected bracketed selector"
			  " list after" );
	    }
	    else
	    if ( i + 1 < size
		 &&
		 vp[i] == PARLEX::parsing
		 &&
		 vp[i+1] == PARLEX::options )
	    {
		i += 2;
		min::gen result =
		    COM::scan_new_flags
			( vp, i, new_options,
			  BRA::BRACKET_OPT,
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
		     vp[i] != PARLEX::dotdotdot )
		    return PAR::parse_error
			( parser, ppvec[i-1],
			  "expected ... after" );
		i += 1;

		SCAN_NAME ( attribute_equal, false );

		if ( i >= size
		     ||
		     vp[i] != PARLEX::dotdotdot )
		    return PAR::parse_error
			( parser, ppvec[i-1],
			  "expected ... after" );
		i += 1;

		SCAN_NAME
		    ( attribute_separator, false );
	    }
	    else return PAR::parse_error
		( parser, ppvec[i-1],
		  "expected `attributes',"
		  " or `parsing selectors',"
		  " or `attribute selectors',"
		  " or `prefix selectors',"
		  " or `parsing options',"
		  " or `attribute flags initiator',"
		  " or `attribute multivalue"
		       " initiator',"
		  " or `attribute negator'"
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

	new_parsing_selectors.or_flags |=
	    new_options.or_flags;
	new_parsing_selectors.not_flags |=
	    new_options.not_flags;
	new_parsing_selectors.xor_flags |=
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
	      new_parsing_selectors,
	      new_attribute_selectors,
	      attribute_begin,
	      attribute_equal,
	      attribute_separator,
	      attribute_negator,
	      attribute_flags_initiator,
	      min::standard_attr_flag_parser,
	      attribute_multivalue_initiator,
	      new_prefix_selectors,
	      bracketed_pass->bracket_table );

	break;
    }
    case ::BRACKET_TYPE:
    {
	TAB::new_flags new_selectors;
	TAB::new_flags new_options;
	min::locatable_gen group ( min::MISSING() );
	min::locatable_gen implied_subprefix
	    ( min::MISSING() );
	min::locatable_gen implied_subprefix_type
	    ( min::MISSING() );
	min::uns32 paragraph_lexical_master =
	    PAR::MISSING_MASTER;
	min::uns32 line_lexical_master =
	    PAR::MISSING_MASTER;
	PAR::reformatter reformatter = min::NULL_STUB;
	min::locatable_gen reformatter_arguments;

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
		 vp[i] == PARLEX::parsing
		 &&
		 vp[i+1] == PARLEX::options )
	    {
		i += 2;
		min::gen result =
		    COM::scan_new_flags
			( vp, i, new_options,
			  BRA::BRACKET_TYPE_OPT,
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
	    if ( vp[i] == PARLEX::group )
	    {
		++ i;
		min::phrase_position position
		    = ppvec[i];
		group = PAR::scan_name
		          ( vp, i, parser,
			    PARLEX::with );
		if ( group == min::ERROR() )
		    return min::ERROR();
		position.end = (& ppvec[i-1])->end;
	    }
	    else
	    if ( i + 1 < size
		 &&
		 vp[i] == PARLEX::implied
		 &&
		 vp[i+1] == PARLEX::subprefix )
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
	    if ( COM::is_lexical_master
	             ( vp, i, size ) )
	    {
		int j = i;
	        if (    COM::get_lexical_master
			   ( parser, vp, ppvec, i,
			     paragraph_lexical_master,
			     line_lexical_master )
		     == min::ERROR() )
		    return min::ERROR();
		if (    paragraph_lexical_master
		     != PAR::MISSING_MASTER )
		{
		    min::phrase_position pos =
		        { (& ppvec[j])->begin,
			  (& ppvec[i-1])->end };
		    return PAR::parse_error
			( parser, pos,
			  "paragraph lexical master"
			  " may NOT be given" );
		}
	    }
	    else if ( i < size )
	    {
		min::uns32 j = i;
		min::locatable_gen name
		  ( PAR::scan_simple_name
			( vp, j,
			  PARLEX::reformatter ) );
		if (    name != min::NONE()
		     && j < size
		     &&    vp[j]
		        == PARLEX::reformatter )
		{
		    min::phrase_position position =
			{ (&ppvec[i])->begin,
			  (&ppvec[j])->end };
		    PAR::reformatter reformatter_stack =
			BRA::
			bracket_type_reformatter_stack;
		    reformatter =
			PAR::find_reformatter
			    ( name, reformatter_stack );
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

			min::unsptr s =
			    min::size_of ( args );
			if ( s < reformatter->
				     minimum_arguments )
			    return PAR::parse_error
				    ( parser, position,
				      "too few"
				      " reformatter"
				      " arguments" );
			if ( s > reformatter->
				     maximum_arguments )
			    return PAR::parse_error
				    ( parser, position,
				      "too many"
				      " reformatter"
				      " arguments" );
		    }
		}
		else
		    return PAR::parse_error
			( parser, ppvec[i-1],
			  "expected"
			  " `prefix selectors',"
			  " `parsing selectors',"
			  " `parsing options',"
			  " `group',"
			  " `implied subprefix',"
			  " or `... lexical master'"
			  " or `... reformatter ...'"
			  " after" );
	    }
	    else
		return PAR::parse_error
		    ( parser, ppvec[i-1],
		      "expected `prefix selectors',"
		      " `parsing selectors',"
		      " `parsing options',"
		      " `group',"
		      " `implied subprefix',"
		      " or `... lexical master'"
		      " or `... reformatter ...'"
		      " after" );
	}
	if ( i < size )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "unexpected stuff after" );

	if (    TAB::all_flags ( new_selectors ) != 0
	     && group == PARLEX::reset )
	{
	    PAR::parse_error
		( parser, ppvec->position,
		  "`with parsing selectors' not allowed"
		  " for bracket type with group"
		  " `reset'; selectors ignored" );
	    new_selectors = TAB::new_flags ( 0, 0, 0 );
	}

	if (    TAB::all_flags ( new_options ) != 0
	     && group != PARLEX::paragraph )
	{
	    PAR::parse_error
		( parser, ppvec->position,
		  "`with parsing options' not allowed"
		  " for bracket type unless group is"
		  " `paragraph'; options ignored" );
	    new_options = TAB::new_flags ( 0, 0, 0 );
	}

	if (    implied_subprefix != min::MISSING()
	     && group == PARLEX::reset )
	{
	    PAR::parse_error
		( parser, ppvec->position,
		  "`with implied subprefix' not allowed"
		  " for bracket type with group"
		  " `reset'; implied subprefix ignored"
		);
	    implied_subprefix = min::MISSING();
	}

	if (    reformatter != min::NULL_STUB
	     && group == PARLEX::reset )
	{
	    PAR::parse_error
		( parser, ppvec->position,
		  "`with ... reformatter' not allowed"
		  " for bracket type with group"
		  " `reset'; reformatter ignored"
		);
	    reformatter = min::NULL_STUB;
	}

	if ( line_lexical_master != PAR::MISSING_MASTER
	     &&
	     group != PARLEX::paragraph )
	{
	    PAR::parse_error
		( parser, ppvec->position,
		  "`with line lexical master' not"
		  " allowed for bracket type unless"
		  " group is `paragraph'; line lexical"
		  " master ignored" );
	    line_lexical_master = PAR::MISSING_MASTER;
	}

	new_selectors.or_flags |=
	    new_options.or_flags;
	new_selectors.not_flags |=
	    new_options.not_flags;
	new_selectors.xor_flags |=
	    new_options.xor_flags;

	BRA::push_bracket_type
	    ( name[0], selectors,
	      PAR::block_level ( parser ),
	      ppvec->position,
	      new_selectors,
	      group,
	      implied_subprefix,
	      implied_subprefix_type,
	      line_lexical_master,
	      reformatter,
	      reformatter_arguments,
	      bracketed_pass->bracket_type_table );
	break;
    }
    default:
	MIN_ABORT ( "bad parser define type" );
    }
    else /* if command == PARLEX::undefine */
    {
	if ( i < size )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "unexpected stuff after" );

	TAB::key_prefix key_prefix =
	    TAB::find_key_prefix
	        ( name[0],
		     type == ::BRACKET_TYPE ? 
		  bracketed_pass->bracket_type_table :
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
	    case ::BRACKET_TYPE:
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
