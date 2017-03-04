// Layered Languages Parser Functions
//
// File:	ll_parser.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Mar  4 12:11:14 EST 2017
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Strings
//	Tokens
//	Parser Closures
//	Parser
//	Reformatters
//	Parser Functions

// Usage and Setup
// ----- --- -----

# include <ll_lexeme_standard.h>
# include <ll_parser.h>
# include <ll_parser_bracketed.h>
# include <ll_parser_command.h>
# include <ll_parser_standard.h>
# define MUP min::unprotected
# define LEX ll::lexeme
# define LEXSTD ll::lexeme::standard
# define PAR ll::parser
# define TAB ll::parser::table
# define BRA ll::parser::bracketed
# define COM ll::parser::command
# define STD ll::parser::standard

min::locatable_gen PAR::top_level;
min::locatable_gen PAR::top;
min::locatable_gen PAR::level;
min::locatable_gen PAR::dot_oper;
min::locatable_gen PAR::doublequote;
min::locatable_gen PAR::number_sign;
min::locatable_gen PAR::new_line;
min::locatable_gen PAR::semicolon;
min::locatable_gen PAR::left_parenthesis;
min::locatable_gen PAR::right_parenthesis;
min::locatable_gen PAR::left_square;
min::locatable_gen PAR::right_square;
min::locatable_gen PAR::left_curly;
min::locatable_gen PAR::right_curly;
min::locatable_gen PAR::comma;
min::locatable_gen PAR::colon;
min::locatable_gen PAR::equal;
min::locatable_gen PAR::vbar;
min::locatable_gen PAR::ealbreak;
min::locatable_gen PAR::ealeindent;
min::locatable_gen PAR::ealtindent;
min::locatable_gen PAR::eapbreak;
min::locatable_gen PAR::ealsep;
min::locatable_gen PAR::eaoclosing;
min::locatable_gen PAR::sticky;
min::locatable_gen PAR::continuing;
min::locatable_gen PAR::other_ea_opt;
min::locatable_gen PAR::default_opt;
min::locatable_gen PAR::other_selectors;
min::locatable_gen PAR::parser_lexeme;
min::locatable_gen PAR::data_lexeme;
min::locatable_gen PAR::prefix_lexeme;
min::locatable_gen PAR::header_lexeme;
min::locatable_gen PAR::line_lexeme;
min::locatable_gen PAR::paragraph_lexeme;
min::locatable_gen PAR::standard_lexeme;
min::locatable_gen PAR::error_operator;
min::locatable_gen PAR::error_operand;
min::locatable_gen PAR::test;
min::locatable_gen PAR::begin;
min::locatable_gen PAR::end;
min::locatable_gen PAR::define;
min::locatable_gen PAR::undefine;
min::locatable_gen PAR::print;
min::locatable_gen PAR::block;
min::locatable_gen PAR::pass_lexeme;
min::locatable_gen PAR::selector;
min::locatable_gen PAR::selectors;
min::locatable_gen PAR::options;
min::locatable_gen PAR::group;
min::locatable_gen PAR::lexical;
min::locatable_gen PAR::master;
min::locatable_gen PAR::implied;
min::locatable_gen PAR::subprefix;
min::locatable_gen PAR::reformatter_lexeme;
min::locatable_gen PAR::default_lexeme;
min::locatable_gen PAR::with;
min::locatable_gen PAR::parsing;
min::locatable_gen PAR::after;
min::locatable_gen PAR::before;
min::locatable_gen PAR::at;
min::locatable_gen PAR::plus;
min::locatable_gen PAR::minus;
min::locatable_gen PAR::dotdotdot;
min::locatable_gen PAR::yes_lexeme;
min::locatable_gen PAR::no_lexeme;
min::locatable_gen PAR::keep_lexeme;
min::locatable_gen PAR::enabled_lexeme;
min::locatable_gen PAR::disabled_lexeme;
min::locatable_gen PAR::star_parser;
min::locatable_gen PAR::parser_colon;
min::locatable_gen PAR::parser_test_colon;

min::locatable_gen PAR::PRINTED;

static min::locatable_gen warnings;
static min::locatable_gen parser_input;
static min::locatable_gen parser_output;
static min::locatable_gen parser_commands;
static min::locatable_gen subexpression_elements;
static min::locatable_gen subexpression_details;
static min::locatable_gen subexpression_lines;
static min::locatable_gen keys;

static void initialize ( void )
{
    PAR::top_level
	= min::new_lab_gen ( "TOP", "LEVEL" );
    PAR::top = min::new_str_gen ( "top" );
    PAR::level = min::new_str_gen ( "level" );
    PAR::dot_oper
	= min::new_str_gen ( ".operator" );

    PAR::doublequote = min::new_str_gen ( "\"" );
    PAR::number_sign = min::new_str_gen ( "#" );
    PAR::new_line = min::new_str_gen ( "\n" );
    PAR::semicolon = min::new_str_gen ( ";" );
    PAR::left_parenthesis = min::new_str_gen ( "(" );
    PAR::right_parenthesis = min::new_str_gen ( ")" );
    PAR::left_square = min::new_str_gen ( "[" );
    PAR::right_square = min::new_str_gen ( "]" );
    PAR::left_curly = min::new_str_gen ( "{" );
    PAR::right_curly = min::new_str_gen ( "}" );
    PAR::comma = min::new_str_gen ( "," );
    PAR::colon = min::new_str_gen ( ":" );
    PAR::equal = min::new_str_gen ( "=" );
    PAR::vbar = min::new_str_gen ( "|" );

    PAR::ealbreak =
    	min::new_lab_gen
	    ( "end", "at", "line", "break" );
    PAR::ealeindent =
    	min::new_lab_gen
	    ( "end", "at", "le", "indent" );
    PAR::ealtindent =
    	min::new_lab_gen
	    ( "end", "at", "lt", "indent" );
    PAR::eapbreak =
    	min::new_lab_gen
	    ( "end", "at", "paragraph", "break" );
    PAR::ealsep =
    	min::new_lab_gen
	    ( "end", "at", "line", "separator" );
    PAR::eaoclosing =
    	min::new_lab_gen
	    ( "end", "at", "outer", "closing" );
    PAR::sticky = min::new_str_gen ( "sticky" );
    PAR::continuing = min::new_str_gen ( "continuing" );

    PAR::other_ea_opt =
        min::new_lab_gen ( "other", "end", "at",
	                   "options" );
    PAR::default_opt =
        min::new_lab_gen ( "default", "options" );
    PAR::other_selectors =
        min::new_lab_gen ( "other", "selectors" );

    PAR::parser_lexeme = min::new_str_gen ( "parser" );
    PAR::data_lexeme = min::new_str_gen ( "data" );
    PAR::prefix_lexeme = min::new_str_gen ( "prefix" );
    PAR::header_lexeme = min::new_str_gen ( "header" );
    PAR::line_lexeme = min::new_str_gen ( "line" );
    PAR::paragraph_lexeme =
        min::new_str_gen ( "paragraph" );
    PAR::standard_lexeme =
        min::new_str_gen ( "standard" );

    PAR::error_operator =
        min::new_str_gen ( "ERROR'OPERATOR" );
    PAR::error_operand =
        min::new_str_gen ( "ERROR'OPERAND" );

    PAR::test = min::new_str_gen ( "test" );
    PAR::begin = min::new_str_gen ( "begin" );
    PAR::end = min::new_str_gen ( "end" );
    PAR::define = min::new_str_gen ( "define" );
    PAR::undefine = min::new_str_gen ( "undefine" );
    PAR::print = min::new_str_gen ( "print" );
    PAR::block = min::new_str_gen ( "block" );
    PAR::pass_lexeme = min::new_str_gen ( "pass" );
    PAR::selector = min::new_str_gen ( "selector" );
    PAR::selectors = min::new_str_gen ( "selectors" );
    PAR::options = min::new_str_gen ( "options" );
    PAR::group = min::new_str_gen ( "group" );
    PAR::lexical = min::new_str_gen ( "lexical" );
    PAR::master = min::new_str_gen ( "master" );
    PAR::implied = min::new_str_gen ( "implied" );
    PAR::subprefix = min::new_str_gen ( "subprefix" );
    PAR::reformatter_lexeme =
        min::new_str_gen ( "reformatter" );
    PAR::default_lexeme =
        min::new_str_gen ( "default" );
    PAR::with = min::new_str_gen ( "with" );
    PAR::parsing = min::new_str_gen ( "parsing" );
    PAR::after = min::new_str_gen ( "after" );
    PAR::before = min::new_str_gen ( "before" );
    PAR::at = min::new_str_gen ( "at" );
    PAR::plus = min::new_str_gen ( "+" );
    PAR::minus = min::new_str_gen ( "-" );
    PAR::dotdotdot = min::new_str_gen ( "..." );
    PAR::yes_lexeme = min::new_str_gen ( "yes" );
    PAR::no_lexeme = min::new_str_gen ( "no" );
    PAR::keep_lexeme = min::new_str_gen ( "keep" );
    PAR::enabled_lexeme =
        min::new_str_gen ( "enabled" );
    PAR::disabled_lexeme =
        min::new_str_gen ( "disabled" );
    PAR::star_parser =
        min::new_str_gen ( "*PARSER*" );
    PAR::parser_colon =
        min::new_lab_gen ( "*PARSER*", ":" );
    PAR::parser_test_colon =
        min::new_lab_gen ( "*PARSER*", "*TEST*", ":" );

    PAR::PRINTED = min::new_special_gen ( 0 );

    ::warnings = min::new_str_gen ( "warnings" );
    ::parser_input =
        min::new_lab_gen ( "parser", "input" );
    ::parser_output =
        min::new_lab_gen ( "parser", "output" );
    ::parser_commands =
        min::new_lab_gen ( "parser", "commands" );
    ::subexpression_elements =
        min::new_lab_gen
	    ( "subexpression", "elements" );
    ::subexpression_details =
        min::new_lab_gen
	    ( "subexpression", "details" );
    ::subexpression_lines =
        min::new_lab_gen
	    ( "subexpression", "lines" );
    ::keys = min::new_str_gen ( "keys" );
}
static min::initializer initializer ( ::initialize );

// Strings
// -------

static min::uns32 string_stub_disp[] =
{
    min::DISP ( & PAR::token_struct::next ),
    min::DISP_END
};

static min::packed_vec<min::uns32,PAR::string_struct>
    string_type ( "ll::parser::string_type",
                  NULL, ::string_stub_disp );

// Free list of strings.
//
static min::locatable_var<PAR::string_insptr>
    free_strings;

static int number_free_strings = 0;

static int max_string_free_list_size = 100;

static unsigned min_string_length = 80;

PAR::string PAR::new_string
	( min::uns32 n, min::ptr<const min::uns32> s )
{
    min::locatable_var<PAR::string_insptr> str
        ( (PAR::string_insptr) ::free_strings );

    if ( str == NULL_STUB )
    {
        min::uns32 m = n;
	if ( m < ::min_string_length )
	    m = ::min_string_length;
        str = ::string_type.new_stub ( m );
    }
    else
    {
        -- ::number_free_strings;
	::free_strings = str->next;
	if ( str->max_length < n )
	    min::resize ( str, n );
	min::pop ( str, str->length );
    }
    next_ref(str) = NULL_STUB;
    min::push ( str, n, s );
    return (PAR::string) str;
}

PAR::string PAR::free_string ( PAR::string string )
{
    if ( string == NULL_STUB )
        return NULL_STUB;

    if ( ::max_string_free_list_size >= 0
         &&
            ::number_free_strings
	 >= ::max_string_free_list_size )
    {
        min::deallocate ( string );
	return NULL_STUB;
    }

    PAR::string_insptr str =
        (PAR::string_insptr) string;
    next_ref(str) = ::free_strings;
    ::free_strings = str;
    ++ ::number_free_strings;
    return NULL_STUB;
}

void PAR::set_max_string_free_list_size ( int n )
{
    ::max_string_free_list_size = n;
    if ( n >= 0 ) while ( ::number_free_strings > n )
    {
	PAR::string_insptr string = ::free_strings;
	::free_strings = string->next;
        min::deallocate ( string );
	-- ::number_free_strings;
    }
}

void PAR::resize ( PAR::string string )
{
    PAR::string_insptr s = (PAR::string_insptr) string;
    min::resize ( s, s->length );
}

// Tokens
// ------

static min::uns32 token_gen_disp[] =
{
    min::DISP ( & PAR::token_struct::value ),
    min::DISP ( & PAR::token_struct::value_type ),
    min::DISP_END
};

static min::uns32 token_stub_disp[] =
{
    min::DISP ( & PAR::token_struct::string ),
    min::DISP ( & PAR::token_struct::next ),
    min::DISP ( & PAR::token_struct::previous ),
    min::DISP_END
};

static min::packed_struct<PAR::token_struct>
    token_type ( "ll::parser::token",
                 ::token_gen_disp,
                 ::token_stub_disp );

// Free and allocated lists of tokens.
//
static min::locatable_var<PAR::token>
    free_list_first;
min::locatable_var<PAR::token>
    PAR::internal::allocated_list_first;

static min::int32 number_free_tokens = 0;
    // Does not count first element on list which is
    // never removed.

static min::int32 max_token_free_list_size = 1000;

static void token_initialize ( void )
{
    PAR::token t;
    ::free_list_first = t = ::token_type.new_stub();
    next_ref ( t ) = t;
    previous_ref ( t ) = t;
    PAR::internal::allocated_list_first = t =
        ::token_type.new_stub();
    next_ref ( t ) = t;
    previous_ref ( t ) = t;
}

static min::initializer
       token_initializer ( ::token_initialize );

PAR::token PAR::new_token ( min::uns32 type )
{
    PAR::token token =
        ::free_list_first->next;
    if ( token == token->next )
        token = ::token_type.new_stub();
    else
    {
	PAR::internal::remove ( token );
	-- number_free_tokens;
    }

    PAR::internal::put_before
        ( PAR::internal::allocated_list_first, token );

    value_ref(token) = min::MISSING();
    value_type_ref(token) = min::MISSING();
    string_ref(token) = NULL_STUB;
    token->type = type;
    return token;
}

void PAR::free ( PAR::token token )
{
    string_ref(token) = free_string ( token->string );
    value_ref(token) = min::MISSING();

    PAR::internal::remove ( token );
    if ( ::max_token_free_list_size >= 0
         &&
            ::number_free_tokens
	 >= ::max_token_free_list_size )
        min::deallocate ( token );
    else
    {
	PAR::internal::put_before
	    ( ::free_list_first, token );
	++ ::number_free_tokens;
    }
}

void PAR::set_max_token_free_list_size ( int n )
{
    ::max_token_free_list_size = n;
    if ( n >= 0 ) while ( ::number_free_tokens > n )
    {
	PAR::token token = ::free_list_first->next;
	MIN_REQUIRE ( token != ::free_list_first );
	PAR::internal::remove ( token );
        min::deallocate ( token );
	-- ::number_free_tokens;
    }
}

// Parser Closures
// ------ --------

static min::packed_struct<PAR::input_struct>
    input_type ( "ll::parser::input_type" );

void PAR::init
	( min::ref<PAR::input> input,
	  uns32 (*add_tokens)
	      ( PAR::parser parser, PAR::input input ),
	  void (*init)
	      ( PAR::parser parser, PAR::input input ) )
{
    if ( input == NULL_STUB )
        input = ::input_type.new_stub();

    input->add_tokens = add_tokens;
    input->init = init;
}

static min::packed_struct<PAR::output_struct>
    output_type ( "ll::parser::output_type" );

void PAR::init
	( min::ref<PAR::output> output,
	  void (*remove_tokens)
	      ( PAR::parser parser,
	        PAR::output output ),
	  void (*init)
	      ( PAR::parser parser,
	        PAR::output output ) )
{
    if ( output == NULL_STUB )
        output = ::output_type.new_stub();

    output->remove_tokens = remove_tokens;
    output->init = init;
}

static min::uns32 pass_gen_disp[] =
{
    min::DISP ( & PAR::pass_struct::name ),
    min::DISP_END
};

static min::uns32 pass_stub_disp[] = {
    min::DISP ( & PAR::pass_struct::parser ),
    min::DISP ( & PAR::pass_struct::next ),
    min::DISP_END };

static min::packed_struct<PAR::pass_struct>
    pass_type ( "ll::parser::pass_type",
                ::pass_gen_disp, ::pass_stub_disp );

void PAR::place_after
	( PAR::parser parser,
	  PAR::pass pass,
	  PAR::pass previous )
{
    MIN_ASSERT ( pass != previous,
                 "pass and previous arguments are"
		 " the same" );

    PAR::remove ( pass );

    MIN_ASSERT ( previous->parser == parser,
                 "previous argument parser != parser"
		 " argument" );
    next_ref(pass) = previous->next;
    next_ref(previous) = pass;

    PAR::parser_ref ( pass ) = parser;
    if ( pass->place != NULL )
	( * pass->place ) ( parser, pass );
}

void PAR::place_before
	( PAR::parser parser,
	  PAR::pass pass,
	  PAR::pass next )
{
    MIN_ASSERT ( pass != next,
                 "pass and next arguments are"
		 " the same" );

    PAR::remove ( pass );

    if ( parser->pass_stack == NULL_STUB )
    {
        MIN_ASSERT ( next == NULL_STUB,
	             "next argument is not NULL_STUB"
		     " but parser pass_stack is"
		     " emtpy" );

	next_ref(pass) = NULL_STUB;
	pass_stack_ref(parser) = pass;
    }
    else
    {
        PAR::pass current = parser->pass_stack;
        MIN_ASSERT ( current != next,
	             "next argument equals current"
		     " beginning of parser"
		     " pass_stack" );
	while ( current->next != next )
	{
	    MIN_ASSERT ( current->next != NULL_STUB,
	                 "next argument is not a pass"
			 " that is in the parser"
			 " pass_stack" );
	    current = current->next;
	}
	next_ref(pass) = current->next;
	next_ref(current) = pass;
    }

    PAR::parser_ref ( pass ) = parser;
    if ( pass->place != NULL )
	( * pass->place ) ( parser, pass );
}

void PAR::remove ( PAR::pass pass )
{
    PAR::parser parser = pass->parser;
    if ( parser == min::NULL_STUB ) return;

    min::ptr<PAR::pass> p = & pass_stack_ref ( parser );
    for ( PAR::pass current = * p;
	  current != min::NULL_STUB;
	  current = * p )
    {
        if ( current == pass )
	{
	    * p = current->next;
	    PAR::parser_ref ( pass ) = min::NULL_STUB;
	    PAR::next_ref ( pass ) = min::NULL_STUB;
	    return;
	}
	p = & PAR::next_ref ( current );
    }
    MIN_ABORT ( "Could not find pass on pass->parser"
                "_pass_stack" );
}

PAR::pass PAR::find_on_pass_stack
	( PAR::parser parser, min::gen name )
{
    PAR::pass pass = parser->pass_stack;
    while ( pass != NULL_STUB )
    {
        if ( pass->name == name ) break;
	pass = pass->next;
    }
    return pass;
}

min::locatable_var<PAR::new_pass_table_type>
    PAR::new_pass_table;

static min::uns32 new_pass_table_gen_disp[] = {
    min::DISP ( & PAR::new_pass_table_struct::name ),
    min::DISP_END };

static min::packed_vec<PAR::new_pass_table_struct>
    new_pass_table_type
        ( "ll::parser::new_pass_table_type",
	  ::new_pass_table_gen_disp );

void PAR::push_new_pass
    ( min::gen name, PAR::new_pass new_pass )
{
    if ( PAR::new_pass_table == min::NULL_STUB )
	PAR::new_pass_table =
	    ::new_pass_table_type.new_stub ( 32 );

    new_pass_table_struct e = { name, new_pass };
    min::push ( (ll::parser::new_pass_table_type)
                ll::parser::new_pass_table )
        = e;
    min::unprotected::acc_write_update
	( ll::parser::new_pass_table, name );
}

// Parser
// ------
//
min::phrase_position PAR::top_level_position =
    { { 0, 0 }, { 0, 0 } };

static min::uns32 parser_stub_disp[] =
{
    min::DISP ( & PAR::parser_struct::input ),
    min::DISP ( & PAR::parser_struct::output ),
    min::DISP ( & PAR::parser_struct::pass_stack ),
    min::DISP ( & PAR::parser_struct
                     ::trace_flag_name_table ),
    min::DISP ( & PAR::parser_struct::scanner ),
    min::DISP ( & PAR::parser_struct::input_file ),
    min::DISP ( & PAR::parser_struct::printer ),
    min::DISP ( & PAR::parser_struct::name_scanner ),
    min::DISP ( & PAR::parser_struct::undefined_stack ),
    min::DISP ( & PAR::parser_struct::block_stack ),
    min::DISP ( & PAR::parser_struct
                     ::selector_name_table ),
    min::DISP ( & PAR::parser_struct
                     ::selector_group_name_table ),
    min::DISP ( & PAR::parser_struct
                     ::top_level_indentation_mark ),
    min::DISP ( & PAR::parser_struct::first ),
    min::DISP_END
};

static min::packed_struct<PAR::parser_struct>
    parser_type ( "ll::parser::parser_type",
                  NULL, ::parser_stub_disp );

min::locatable_var<PAR::parser> PAR::default_parser;

void PAR::init ( min::ref<PAR::parser> parser,
                 bool define_standard )
{
    if ( parser == NULL_STUB )
    {
        parser = ::parser_type.new_stub();
	first_ref(parser) = min::NULL_STUB;

	parser->eof = false;
	parser->finished_tokens = 0;
	parser->at_paragraph_beginning = false;
	parser->error_count = 0;
	parser->warning_count = 0;
	parser->max_error_count = 100;
	parser->message_header.begin =
	parser->message_header.end =
	    min::MISSING_POSITION;
	parser->subexpression_gen_format =
	    min::line_gen_format;
	parser->trace_flags = PAR::TRACE_WARNINGS;
	parser->selectors = PAR::DEFAULT_OPT
	                  + PAR::TOP_LEVEL_SELECTOR
	                  + PAR::ALWAYS_SELECTOR;
	PAR::prefix_separator_ref(parser) =
	    min::MISSING();

	TAB::init_name_table
	    ( trace_flag_name_table_ref(parser) );
	MIN_REQUIRE
	    (    PAR::TRACE_WARNINGS
	      == 1ull << TAB::push_name
	              ( parser->trace_flag_name_table,
		        ::warnings ) );
	MIN_REQUIRE
	    (    PAR::TRACE_PARSER_INPUT
	      == 1ull << TAB::push_name
	              ( parser->trace_flag_name_table,
		        ::parser_input ) );
	MIN_REQUIRE
	    (    PAR::TRACE_PARSER_OUTPUT
	      == 1ull << TAB::push_name
		      ( parser->trace_flag_name_table,
		        ::parser_output ) );
	MIN_REQUIRE
	    (    PAR::TRACE_PARSER_COMMANDS
	      == 1ull << TAB::push_name
		      ( parser->trace_flag_name_table,
		        ::parser_commands ) );
	MIN_REQUIRE
	    (    PAR::TRACE_SUBEXPRESSION_ELEMENTS
	      == 1ull << TAB::push_name
		      ( parser->trace_flag_name_table,
		        ::subexpression_elements ) );
	MIN_REQUIRE
	    (    PAR::TRACE_SUBEXPRESSION_DETAILS
	      == 1ull << TAB::push_name
		      ( parser->trace_flag_name_table,
		        ::subexpression_details ) );
	MIN_REQUIRE
	    (    PAR::TRACE_SUBEXPRESSION_LINES
	      == 1ull << TAB::push_name
		      ( parser->trace_flag_name_table,
		        ::subexpression_lines ) );
	MIN_REQUIRE
	    (    PAR::TRACE_KEYS
	      == 1ull << TAB::push_name
		      ( parser->trace_flag_name_table,
		        ::keys ) );

	TAB::init_undefined_stack
	    ( undefined_stack_ref(parser) );
	TAB::init_block_stack
	    ( block_stack_ref(parser) );

	TAB::init_name_table
	    ( selector_name_table_ref(parser) );

	MIN_REQUIRE
	    (    PAR::EALBREAK_OPT
	      == 1ull << TAB::push_name
		      ( parser->selector_name_table,
			PAR::ealbreak ) );
	MIN_REQUIRE
	    (    PAR::EALEINDENT_OPT
	      == 1ull << TAB::push_name
		      ( parser->selector_name_table,
			PAR::ealeindent ) );
	MIN_REQUIRE
	    (    PAR::EALTINDENT_OPT
	      == 1ull << TAB::push_name
		      ( parser->selector_name_table,
			PAR::ealtindent ) );
	MIN_REQUIRE
	    (    PAR::EAPBREAK_OPT
	      == 1ull << TAB::push_name
		      ( parser->selector_name_table,
			PAR::eapbreak ) );
	MIN_REQUIRE
	    (    PAR::EALSEP_OPT
	      == 1ull << TAB::push_name
		      ( parser->selector_name_table,
			PAR::ealsep ) );
	MIN_REQUIRE
	    (    PAR::EAOCLOSING_OPT
	      == 1ull << TAB::push_name
		      ( parser->selector_name_table,
			PAR::eaoclosing ) );
	MIN_REQUIRE
	    (    PAR::STICKY_OPT
	      == 1ull << TAB::push_name
		      ( parser->selector_name_table,
			PAR::sticky ) );
	MIN_REQUIRE
	    (    PAR::CONTINUING_OPT
	      == 1ull << TAB::push_name
		      ( parser->selector_name_table,
			PAR::continuing ) );

	while ( parser->selector_name_table->length
	        < 16 )
	    TAB::push_name
		( parser->selector_name_table,
		  min::MISSING() );

	MIN_REQUIRE
	    (    PAR::ALWAYS_SELECTOR
	      == 1ull << TAB::push_name
		      ( parser->selector_name_table,
			min::MISSING() ) );

	MIN_REQUIRE
	    (    PAR::TOP_LEVEL_SELECTOR
	      == 1ull << TAB::push_name
		      ( parser->selector_name_table,
			PAR::top_level ) );

	MIN_REQUIRE
	    (    PAR::DATA_SELECTOR
	      == 1ull << TAB::push_name
		      ( parser->selector_name_table,
			PAR::data_lexeme ) );

	PAR::selector_group_name_table_ref(parser) =
	    TAB::create_key_table ( 32 );

	TAB::push_root
	    ( PAR::other_ea_opt, PAR::ALL_EA_OPT,
	      0, PAR::top_level_position,
	      parser->selector_group_name_table );
	TAB::push_root
	    ( PAR::default_opt, PAR::DEFAULT_OPT,
	      0, PAR::top_level_position,
	      parser->selector_group_name_table );
	TAB::push_root
	    ( PAR::other_selectors,
	      PAR::COMMAND_SELECTORS,
	      0, PAR::top_level_position,
	      parser->selector_group_name_table );

	BRA::bracketed_pass bracketed_pass =
	    (BRA::bracketed_pass)
	    BRA::new_pass ( parser );
	PAR::place_before ( parser,
	                   (PAR::pass) bracketed_pass );

	top_level_indentation_mark_ref(parser) =
	    BRA::push_indentation_mark
		( PAR::top_level, PAR::semicolon,
		  0, 0, PAR::top_level_position,
		  TAB::new_flags ( 0, 0, 0 ),
		  min::MISSING(), LEX::MISSING_MASTER,
		  bracketed_pass->bracket_table );

	top_level_indentation_mark_ref(parser) =
	    BRA::push_indentation_mark
		( PAR::parser_colon, PAR::semicolon,
	          PAR::TOP_LEVEL_SELECTOR,
		  0, PAR::top_level_position,
		  PAR::parsing_selectors
		      ( PAR::DATA_SELECTOR ),
		  min::MISSING(), LEX::MISSING_MASTER,
		  bracketed_pass->bracket_table );

	top_level_indentation_mark_ref(parser) =
	    BRA::push_indentation_mark
		( PAR::parser_test_colon,
		  PAR::semicolon,
	          PAR::TOP_LEVEL_SELECTOR,
		  0, PAR::top_level_position,
		  TAB::new_flags ( 0, 0, 0 ),
		  min::MISSING(), LEX::MISSING_MASTER,
		  bracketed_pass->bracket_table );

	min::locatable_gen label_name
	    ( min::new_str_gen ( "label" ) );
	min::locatable_gen special_name
	    ( min::new_str_gen ( "special" ) );

	min::locatable_gen opening_double_brace
	    ( min::new_lab_gen ( "{", "{" ) );
	min::locatable_gen closing_double_brace
	    ( min::new_lab_gen ( "}", "}" ) );

	min::locatable_gen opening_quote
	    ( min::new_str_gen ( "`" ) );
	min::locatable_gen closing_quote
	    ( min::new_str_gen ( "'" ) );

	min::locatable_gen opening_square_angle
	    ( min::new_lab_gen ( "[", "<" ) );
	min::locatable_gen angle_closing_square
	    ( min::new_lab_gen ( ">", "]" ) );

	min::locatable_gen opening_square_dollar
	    ( min::new_lab_gen ( "[", "$" ) );
	min::locatable_gen dollar_closing_square
	    ( min::new_lab_gen ( "$", "]" ) );

	min::locatable_gen no
	    ( min::new_str_gen ( "no" ) );

	min::locatable_gen opening_brace_star
	    ( min::new_lab_gen ( "{", "*" ) );
	min::locatable_gen closing_star_brace
	    ( min::new_lab_gen ( "*", "}" ) );

	min::locatable_gen double_vbar
	    ( min::new_str_gen ( "||" ) );

	min::locatable_gen multivalue
	    ( min::new_str_gen ( "multivalue" ) );
	min::locatable_gen comma
	    ( min::new_str_gen ( "," ) );
	min::locatable_var
		<min::packed_vec_insptr<min::gen> >
	    multivalue_arguments
		( min::gen_packed_vec_type
		      .new_stub ( 1 ) );
	min::push ( multivalue_arguments ) = comma;

	BRA::push_brackets
	    ( PAR::left_parenthesis,
	      PAR::right_parenthesis,
	      PAR::DATA_SELECTOR,
	      0, PAR::top_level_position,
	      TAB::new_flags ( 0, 0, 0 ),
	      min::NULL_STUB, min::NULL_STUB,
	      bracketed_pass->bracket_table );

	BRA::push_brackets
	    ( PAR::left_square, PAR::right_square,
	      PAR::DATA_SELECTOR,
	      0, PAR::top_level_position,
	      TAB::new_flags ( 0, 0, 0 ),
	      min::NULL_STUB, min::NULL_STUB,
	      bracketed_pass->bracket_table );

	BRA::push_brackets
	    ( opening_double_brace,
	      closing_double_brace,
	      PAR::DATA_SELECTOR,
	      0, PAR::top_level_position,
	      TAB::new_flags ( 0, 0, 0 ),
	      min::NULL_STUB, min::NULL_STUB,
	      bracketed_pass->bracket_table );

	BRA::push_brackets
	    ( opening_quote,
	      closing_quote,
	      PAR::DATA_SELECTOR,
	      0, PAR::top_level_position,
	      TAB::new_flags ( 0, 0, 0 ),
	      min::NULL_STUB, min::NULL_STUB,
	      bracketed_pass->bracket_table );

	BRA::push_brackets
	    ( opening_brace_star,
	      closing_star_brace,
	      PAR::DATA_SELECTOR,
	      0, PAR::top_level_position,
	      TAB::new_flags ( 0, 0, 0 ),
	      PAR::find_reformatter
		  ( multivalue,
		    BRA::reformatter_stack ),
	      multivalue_arguments,
	      bracketed_pass->bracket_table );

	BRA::push_brackets
	    ( opening_square_angle,
	      angle_closing_square,
	      PAR::DATA_SELECTOR,
	      0, PAR::top_level_position,
	      TAB::new_flags ( 0, 0, 0 ),
	      PAR::find_reformatter
		  ( label_name,
		    BRA::reformatter_stack ),
	      min::NULL_STUB,
	      bracketed_pass->bracket_table );

	BRA::push_brackets
	    ( opening_square_dollar,
	      dollar_closing_square,
	      PAR::DATA_SELECTOR,
	      0, PAR::top_level_position,
	      TAB::new_flags ( 0, 0, 0 ),
	      PAR::find_reformatter
		  ( special_name,
		    BRA::reformatter_stack ),
	      min::NULL_STUB,
	      bracketed_pass->bracket_table );

	BRA::push_typed_brackets
	    ( PAR::left_curly,
	      PAR::vbar,
	      double_vbar,
	      PAR::right_curly,
	      PAR::DATA_SELECTOR,
	      0, PAR::top_level_position,
	      TAB::new_flags ( 0, 0, 0 ),
	      PAR::DATA_SELECTOR,
	      PAR::colon, PAR::equal, PAR::comma, no,
	      PAR::left_square,
	      min::standard_attr_flag_parser,
	      opening_brace_star,
	      false,
	      bracketed_pass->bracket_table );

	if ( define_standard )
	{
	    STD::init_block ( parser );
	    STD::init_brackets ( parser );
	    STD::init_oper ( parser );
	}
    }
}

void PAR::reset ( min::ref<PAR::parser> parser )
{
    PAR::init ( parser );

    PAR::token token;  // WARNING:: not locatable.
    while (    ( token = PAR::remove
		     ( PAR::first_ref(parser)) )
	    != NULL_STUB )
	PAR::free ( token );

    // Restore any block_level 0 definition selectors.
    //
    while ( parser->undefined_stack->length > 0 )
    {
        TAB::undefined_struct u =
	    min::pop ( parser->undefined_stack );
	u.root->selectors = u.saved_selectors;
    }

    min::pop ( parser->block_stack,
               parser->block_stack->length );

    for ( PAR::pass pass = parser->pass_stack;
    	  pass != min::NULL_STUB;
	  pass = pass->next )
    {
	if ( pass->reset != NULL )
	    ( * pass->reset ) ( parser, pass );
    }


    parser->eof = false;
    parser->finished_tokens = 0;
    parser->at_paragraph_beginning = false;
    parser->error_count = 0;
    parser->warning_count = 0;
    parser->max_error_count = 100;
    parser->message_header.begin =
    parser->message_header.end =
	min::MISSING_POSITION;
    parser->subexpression_gen_format =
	min::line_gen_format;
    parser->trace_flags = PAR::TRACE_WARNINGS;
    parser->selectors = PAR::DEFAULT_OPT
		      + PAR::TOP_LEVEL_SELECTOR
		      + PAR::ALWAYS_SELECTOR;
    PAR::prefix_separator_ref(parser) =
	min::MISSING();
}

void PAR::init_input_stream
	( min::ref<PAR::parser> parser,
	  std::istream & in,
	  min::uns32 line_display,
	  min::uns32 spool_lines )
{
    init ( parser );

    min::init_input_stream
        ( input_file_ref(parser),
	  in, line_display, spool_lines );
}

void PAR::init_input_file
	( min::ref<PAR::parser> parser,
	  min::file ifile,
	  min::uns32 line_display,
	  min::uns32 spool_lines )
{
    init ( parser );

    min::init_input_file
        ( input_file_ref(parser),
	  ifile, line_display, spool_lines );
}

bool PAR::init_input_named_file
	( min::ref<PAR::parser> parser,
	  min::gen file_name,
	  min::uns32 line_display,
	  min::uns32 spool_lines )
{
    init ( parser );

    return min::init_input_named_file
        ( input_file_ref(parser),
	  file_name, line_display, spool_lines );
}

void PAR::init_input_string
	( min::ref<PAR::parser> parser,
	  min::ptr<const char> data,
	  min::uns32 line_display,
	  min::uns32 spool_lines )
{
    init ( parser );

    min::init_input_string
        ( input_file_ref(parser),
	  data, line_display, spool_lines );
}

void PAR::init_ostream
	( min::ref<PAR::parser> parser,
	  std::ostream & out )
{
    init ( parser );

    min::init_ostream ( printer_ref(parser), out );
}

void PAR::parse ( PAR::parser parser )
{
    // Initialize parser parameters.
    //
    if ( parser->scanner != NULL_STUB )
    {
        LEX::scanner scanner = parser->scanner;
        if ( parser->input_file != scanner->input_file )
	{
	    if ( parser->input_file == NULL_STUB )
	        input_file_ref(parser) =
			scanner->input_file;
	    else if ( scanner->input_file == NULL_STUB )
		LEX::input_file_ref(scanner) =
			parser->input_file;
	    else MIN_ABORT
	        ( "input_file of parser and"
		  " parser->scanner are not the same" );
	}
	else if ( parser->input_file == NULL_STUB )
	    MIN_ABORT
	        ( "parser->input_file not defined" );

        if ( parser->printer != scanner->printer )
	{
	    if ( parser->printer == NULL_STUB )
	        printer_ref(parser) =
			scanner->printer;
	    else if ( scanner->printer == NULL_STUB )
	        LEX::printer_ref(scanner) =
			parser->printer;
	    else MIN_ABORT
	        ( "printer of parser and"
		  " parser->scanner are not the same" );
	}
	else if ( parser->printer == NULL_STUB )
	    MIN_ABORT
	        ( "parser->printer not defined" );
    }

    if ( parser->input->init != NULL)
	( * parser->input->init )
	    ( parser, parser->input );
    if (    parser->output != NULL_STUB
         && parser->output->init != NULL)
	( * parser->input->init )
	    ( parser, parser->input );

    for ( PAR::pass pass = parser->pass_stack;
    	  pass != min::NULL_STUB;
	  pass = pass->next )
    {
	if ( pass->begin_parse != NULL )
	    ( * pass->begin_parse ) ( parser, pass );
    }

    // Go to the first indent_t or end_of_file_t token.
    // Delete skipped tokens except for start of file
    // token.
    //
    parser->input->add_tokens
	( parser, parser->input );
    PAR::token current = parser->first;
    MIN_REQUIRE ( current != NULL_STUB );
    MIN_REQUIRE
        ( current->type == LEXSTD::start_of_file_t );
    while ( current->type != LEXSTD::indent_t
            &&
	    current->type != LEXSTD::end_of_file_t )
    {
	if ( current->next == parser->first )
	{
	    parser->input->add_tokens
		( parser, parser->input );
	    MIN_REQUIRE (    current->next
		          != parser->first );
	}
	current = current->next;
	if (    current->previous->type
	     != LEXSTD::start_of_file_t )
	    PAR::free
		( PAR::remove ( first_ref(parser),
				current->previous ) );
    }

    // Top level loop.
    //
    bool first_lexeme = true;

    BRA::line_variables line_variables;
    line_variables.last_paragraph = min::NULL_STUB;
    BRA::line_data & line_data = line_variables.current;

    line_data.lexical_master =
        LEX::MISSING_MASTER;
    line_data.selectors =
        parser->selectors;
    line_data.implied_header =
        min::MISSING();
    line_data.header_entry =
        min::NULL_STUB;  // Just for safety.
    line_variables.paragraph =
    line_variables.implied_paragraph =
    line_variables.indentation_paragraph =
    line_variables.indentation_implied_paragraph =
        line_variables.current;

    parser->at_paragraph_beginning = true;
    line_variables.current.selectors =
        ~ PAR::CONTINUING_OPT;
	// line_variables.current.selectors are replaced
	// by line_variables.paragraph.selectors near
	// beginning of loop.
    while ( true )
    {
        if (   parser->error_count
	     > parser->max_error_count )
	{
	    parser->printer
		<< "PARSER MAXIMUM ALLOWED ERROR COUNT"
		<< "(" << parser->max_error_count
		<< ") EXCEEDED: PARSER QUITTING"
	        << min::eol;
	    break;
	}

        // If end of file terminate loop.
	//
        if ( current->type == LEXSTD::end_of_file_t )
	    break;

	// If indent token, then if first lexeme,
	// complain if token indent is not 0, and delete
	// it in any case.
	//
	if ( current->type == LEXSTD::indent_t )
	{
	    if ( first_lexeme && current->indent != 0 )
	    {
		min::phrase_position position =
		    current->position;
		position.begin = position.end;
		PAR::parse_error
		    ( parser, position,
		      "first non-comment lexeme"
		      " is indented" );
	    }

	    if ( current->next == parser->first )
	    {
		parser->input->add_tokens
		    ( parser, parser->input );
		MIN_REQUIRE (    current->next
			      != parser->first );
	    }

	    current = current->next;
	    PAR::free
		( PAR::remove ( first_ref(parser),
				current->previous ) );
	}
	first_lexeme = false;

	if ( parser->at_paragraph_beginning
	     &&
	     ! ( line_variables.current.selectors
		 &
		 PAR::CONTINUING_OPT ) )
	    line_variables.current =
	        line_variables.paragraph;

	// Get subexpression.  First is the first token
	// of the subexpression.

	TAB::flags selectors =
	    line_variables.current.selectors;

	line_variables.previous = current->previous;
	line_variables.at_paragraph_beginning =
	      parser->at_paragraph_beginning;
	line_variables.line_sep =
	    parser->top_level_indentation_mark
	          ->line_sep;
	bool maybe_parser_command = 
	    ( current->value == PAR::star_parser );
	min::position separator_found =
	    BRA::parse_bracketed_subexpression
		( parser, selectors,
		  current,
		  0,
		  NULL,
		  & line_variables );

	PAR::token first =
	    line_variables.previous->next;
	min::uns32 error_count_save =
	    parser->error_count;

        // If subexpression is not empty, or separator
	// found, compact it.
	//
	if ( separator_found
	     ||
	     ( first != current
	       &&
	       ( first->next != current
	         ||
		 (    first->value_type
		   != PAR::paragraph_lexeme
		   &&
		      first->value_type
		   != PAR::line_lexeme ) ) ) )
	{
	    PAR::compact_logical_line
		( parser, parser->pass_stack->next,
	          selectors,
		  first, current,
		  separator_found,
	          (TAB::root)
		  parser->top_level_indentation_mark
	                ->line_sep,
		  0 );
	}

	PAR::token output = min::NULL_STUB;

	// Compact prefix paragraph if necessary.
	//
	if (    first->value_type
	     == PAR::paragraph_lexeme )
	{
	    if ( line_variables.last_paragraph
		 != min::NULL_STUB )
	    {
		output = line_variables.last_paragraph;
		line_variables.last_paragraph
		    = min::NULL_STUB;
		PAR::compact_paragraph
		    ( parser,
		      output, first,
		      0 );
	    }

	    if ( ! parser->at_paragraph_beginning
		 ||
		 ( line_variables.current.selectors
		   &
		   PAR::CONTINUING_OPT ) )
		line_variables.last_paragraph = first;
	    else
		output = first;
	}
	else if ( parser->at_paragraph_beginning
	          &&
		     line_variables.last_paragraph
	          != min::NULL_STUB
	          &&
	          ! ( line_variables.current.selectors
		      &
		      PAR::CONTINUING_OPT ) )
	{
	    output = line_variables.last_paragraph;
	    line_variables.last_paragraph
		= min::NULL_STUB;
	    PAR::compact_paragraph
		( parser,
		  output, current,
		  0 );
	}
	else if ( first == current )
	    continue;
	else if (    line_variables.last_paragraph
	          == min::NULL_STUB )
	    output = current->previous;

	if ( output == min::NULL_STUB )
	    continue;

	min::gen result = min::FAILURE();
	if (    parser->error_count
	     != error_count_save )
	    result = min::ERROR();
	else if ( maybe_parser_command )
	{
	    line_variables.current =
	    line_variables.paragraph =
		line_variables.indentation_paragraph;
	    line_variables.implied_paragraph =
		line_variables
		    .indentation_implied_paragraph;

	    min::obj_vec_ptr vp ( output->value );
	    if ( vp != min::NULL_STUB )
	    {
	        if ( min::size_of ( vp ) == 1 )
		{
		    min::gen initiator =
		        min::get ( vp[0],
			           min::dot_initiator );
		    result = min::SUCCESS();

		    if (    initiator
		         == PAR::parser_test_colon )
			COM::parser_test_execute_command
			    ( parser, vp[0] );
		    else if (    initiator
		              == PAR::parser_colon )
			COM::parser_execute_command
			    ( parser, vp[0] );
		    else
			result = min::FAILURE();
		}
	    }
	}

	TAB::flags trace_flags =
	    parser->trace_flags;
	if ( ( parser->output == NULL_STUB )
	      &&
	      (   trace_flags
		& PAR::TRACE_PARSER_OUTPUT ) )
	{
	     trace_flags &=
		   PAR::TRACE_SUBEXPRESSION_ELEMENTS
		 + PAR::TRACE_SUBEXPRESSION_DETAILS
		 + PAR::TRACE_SUBEXPRESSION_LINES;
	    if ( trace_flags == 0 )
		trace_flags =
		  PAR::TRACE_SUBEXPRESSION_ELEMENTS;
	}
	else
	    trace_flags = 0;

	if ( result == min::FAILURE() )
	{
	    ++ parser->finished_tokens;
	    if ( parser->output != NULL_STUB )
		(* parser->output->remove_tokens)
		    ( parser, parser->output );
	    else
	        trace_subexpression
		    ( parser, output,
		      trace_flags );
	}
	else
	    PAR::free
		( PAR::remove
		    ( PAR::first_ref ( parser ),
		      output ) );
    }

    for ( PAR::pass pass = parser->pass_stack;
    	  pass != min::NULL_STUB;
	  pass = pass->next )
    {
	if ( pass->end_parse != NULL )
	    ( * pass->end_parse ) ( parser, pass );
    }
}

// Reformatters
// ------------

static min::uns32 reformatter_gen_disp[] = {
    min::DISP ( & PAR::reformatter_struct::name ),
    min::DISP_END };

static min::uns32 reformatter_stub_disp[] = {
    min::DISP ( & PAR::reformatter_struct::next ),
    min::DISP_END };

static min::packed_struct<PAR::reformatter_struct>
    reformatter_type
        ( "ll::parser::reformatter_type",
	  ::reformatter_gen_disp,
	  ::reformatter_stub_disp );
const min::uns32 & PAR::REFORMATTER =
    ::reformatter_type.subtype;

void PAR::push_reformatter
    ( min::gen name,
      min::uns32 flags,
      min::uns32 minimum_arguments,
      min::uns32 maximum_arguments,
      PAR::reformatter_function reformatter_function,
      min::ref<PAR::reformatter> stack )
{
    min::locatable_var<PAR::reformatter> r
        ( ::reformatter_type.new_stub() );
    PAR::next_ref(r) = stack;
    PAR::name_ref(r) = name;
    r->flags = flags;
    r->minimum_arguments = minimum_arguments;
    r->maximum_arguments = maximum_arguments;
    r->reformatter_function = reformatter_function;
    stack = r;
}

// Parser Functions
// ------ ---------

min::gen PAR::begin_block
	( PAR::parser parser, min::gen name,
	  const min::phrase_position & position )
{
    TAB::push_block ( parser->block_stack,
                      name,
		      parser->selector_name_table,
                      parser->undefined_stack,
		      parser->selectors,
		      parser->trace_flags );

    min::gen result = min::SUCCESS();
    for ( PAR::pass pass = parser->pass_stack;
	  pass != NULL;
	  pass = pass->next )
    {
	min::gen saved_result = result;
	if ( pass->begin_block != NULL )
	    result = (* pass->begin_block )
		( parser, pass, position, name );
	if ( saved_result == min::ERROR()
	     ||
	     result == min::FAILURE() )
	    result = saved_result;
    }

    return result;
}

min::gen PAR::end_block
	( PAR::parser parser, min::gen name,
	  const min::phrase_position & position )
{
    min::uns32 block_level =
        PAR::block_level ( parser );
    MIN_REQUIRE ( block_level > 0 );
    min::ref<TAB::block_struct> b =
        parser->block_stack[block_level-1];

    if ( block_level == 0 )
        return PAR::parse_error
	    ( parser,
	      position,
	      "not inside a block"
	      " (no begin block to end)" );
    else if ( name != (&b)->name )
        return PAR::parse_error
	    ( parser,
	      position,
	      "innermost block name does not match `",
	      min::pgen_name ( name ), "'" );

    while ( parser->selector_name_table->length
            >
	    (&b)->saved_selector_name_table_length )
        TAB::pop_name ( parser->selector_name_table );

    min::uns32 length =
        (&b)->saved_undefined_stack_length;
    while ( parser->undefined_stack->length > length )
    {
        TAB::undefined_struct u =
	    min::pop ( parser->undefined_stack );
	u.root->selectors = u.saved_selectors;
    }

    min::gen result = min::SUCCESS();
    for ( PAR::pass pass = parser->pass_stack;
	  pass != NULL;
	  pass = pass->next )
    {
	min::gen saved_result = result;
	if ( pass->end_block != NULL )
	    result = (* pass->end_block )
		( parser, pass, position, name );
	if ( saved_result == min::ERROR()
	     ||
	     result == min::FAILURE() )
	    result = saved_result;
    }

    parser->selectors = (&b)->saved_selectors;
    parser->trace_flags = (&b)->saved_trace_flags;

    min::pop ( parser->block_stack );

    return result;
}

TAB::key_prefix PAR::find_key_prefix
	( PAR::parser parser,
	  PAR::token & current,
	  TAB::key_table key_table,
	  PAR::token next )
{
    uns32 phash = min::labhash_initial;
    uns32 table_len = key_table->length;
    uns32 mask = table_len - 1;
    MIN_REQUIRE ( ( table_len & mask ) == 0 );
    TAB::key_prefix previous = NULL_STUB;
    while ( true )
    {
	if ( current == next
	     ||
             current->value == min::MISSING()
	     ||
	     ! is_lexeme ( current->type ) )
	    break;

	min::gen e = current->value;
	uns32 hash;
	if ( min::is_str ( e ) )
	    hash = min::strhash ( e );
	else if ( min::is_num ( e ) )
	{
	    int v = min::int_of ( e );
	    MIN_REQUIRE ( 0 <= v && v < (1<<28) );
	    hash = min::numhash ( e );
	}
	else
	    MIN_ABORT ( "bad key element type" );

	// Compute hash of this element's key prefix.
	//
	phash = min::labhash ( phash, hash );
	if ( previous != NULL_STUB ) hash = phash;

	// Locate key prefix.
	//
	TAB::key_prefix key_prefix =
	    key_table[hash & mask];
	while ( key_prefix != NULL_STUB )
	{
	    if ( key_prefix->key_element == e
	         &&
		 key_prefix->previous == previous )
	        break;
	    key_prefix = key_prefix->next;
	}
	if ( key_prefix == NULL_STUB ) break;

	previous = key_prefix;

        if ( current->next == parser->first )
	{
	    parser->input->add_tokens
		( parser, parser->input);
	    MIN_REQUIRE
	        ( current->next != parser->first );
	}

	current = current->next;
    }

    return previous;
}

TAB::root PAR::find_entry
	( PAR::parser parser,
	  PAR::token & current,
	  TAB::key_prefix & key_prefix,
	  TAB::flags selectors,
	  TAB::key_table key_table,
	  PAR::token next )
{
    for ( key_prefix =
	      find_key_prefix
		  ( parser, current, key_table, next );
          key_prefix != NULL_STUB;
	  key_prefix = key_prefix->previous,
	  current = current->previous )
    if ( key_prefix != NULL_STUB )
    for ( TAB::root root = key_prefix->first;
	  root != NULL_STUB;
	  root = root->next )
    {
	if ( root->selectors & selectors )
	    return root;
    }
    return NULL_STUB;
}

TAB::root PAR::find_next_entry
	( PAR::parser parser,
	  PAR::token & current,
	  TAB::key_prefix & key_prefix,
	  TAB::flags selectors,
	  TAB::root last_entry,
	  bool shorten )
{
    last_entry = shorten ? (TAB::root) NULL_STUB
                         : last_entry->next;
    while ( true )
    {
	while ( last_entry == NULL_STUB )
	{
	    key_prefix = key_prefix->previous;
	    current = current->previous;
	    if ( key_prefix == NULL_STUB )
		return NULL_STUB;
	    last_entry = key_prefix->first;
	}

	if ( last_entry->selectors & selectors )
	    return last_entry;
        last_entry = last_entry->next;
    }
}

void PAR::put_empty_before
	( ll::parser::parser parser,
	  ll::parser::token t )
{
    PAR::token token = new_token ( PAR::PURELIST );
    put_before ( PAR::first_ref(parser), t, token );

    min::phrase_position position =
        { t->position.begin, t->position.begin };
    token->position = position;
    min::locatable_var
	    <min::phrase_position_vec_insptr>
	pos;
    min::init ( pos, parser->input_file, position, 0 );

    PAR::value_ref(token) = min::new_obj_gen ( 3, 1 );
    min::obj_vec_insptr vp ( token->value );
    min::attr_insptr ap ( vp );
    min::locate ( ap, min::dot_position );
    min::set ( ap, min::new_stub_gen ( pos ) );
    min::set_flag ( ap, min::standard_attr_hide_flag );
}

void PAR::put_empty_after
	( ll::parser::parser parser,
	  ll::parser::token t )
{
    PAR::token token = new_token ( PAR::BRACKETABLE );
    put_before
        ( PAR::first_ref(parser), t->next, token );

    min::phrase_position position =
        { t->position.end, t->position.end };
    token->position = position;
    min::locatable_var
	    <min::phrase_position_vec_insptr>
	pos;
    min::init ( pos, parser->input_file, position, 0 );

    PAR::value_ref(token) = min::new_obj_gen ( 3, 1 );
    min::obj_vec_insptr vp ( token->value );
    min::attr_insptr ap ( vp );
    min::locate ( ap, min::dot_position );
    min::set ( ap, min::new_stub_gen ( pos ) );
    min::set_flag ( ap, min::standard_attr_hide_flag );
}

void PAR::put_error_operand_before
	( ll::parser::parser parser,
	  ll::parser::token t )
{
    PAR::token token = new_token ( LEXSTD::word_t );
    put_before ( PAR::first_ref(parser), t, token );
    PAR::value_ref ( token ) = PAR::error_operand;

    min::phrase_position position =
        { t->position.begin, t->position.begin };
    token->position = position;
}

void PAR::put_error_operand_after
	( ll::parser::parser parser,
	  ll::parser::token t )
{
    PAR::token token = new_token ( LEXSTD::word_t );
    put_before ( PAR::first_ref(parser), t->next,
                                         token );
    PAR::value_ref ( token ) = PAR::error_operand;

    min::phrase_position position =
        { t->position.end, t->position.end };
    token->position = position;
}

void PAR::put_error_operator_before
	( ll::parser::parser parser,
	  ll::parser::token t )
{
    PAR::token token = new_token ( PAR::OPERATOR );
    put_before ( PAR::first_ref(parser), t, token );
    PAR::value_ref ( token ) = PAR::error_operator;

    min::phrase_position position =
        { t->position.begin, t->position.begin };
    token->position = position;
}

void PAR::put_error_operator_after
	( ll::parser::parser parser,
	  ll::parser::token t )
{
    PAR::token token = new_token ( PAR::OPERATOR );
    put_before ( PAR::first_ref(parser), t->next,
                                         token );
    PAR::value_ref ( token ) = PAR::error_operator;

    min::phrase_position position =
        { t->position.end, t->position.end };
    token->position = position;
}

static void set_attr_flags
	( PAR::parser parser,
	  min::attr_insptr expap,
	  min::gen flags,
	  const min::flag_parser * flag_parser
	      = min::standard_attr_flag_parser )
{
    min::obj_vec_insptr vp ( flags );

    for ( min::unsptr i = 0;
	  i < min::size_of ( vp ); ++ i )
    {
	min::gen flags_text = vp[i];
	if ( min::is_obj ( flags_text ) )
	{
	    min::obj_vec_insptr fvp ( flags_text );
	    min::attr_insptr fap ( fvp );
	    min::locate ( fap, min::dot_type );
	    min::gen type = get ( fap );
	    if ( type == min::doublequote
		 ||
		 type == min::number_sign )
		flags_text = fvp[0];
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
		char buffer[len+200];
		sprintf ( buffer,
			  "bad flag(s) \"%s\" in ",
			  text_buffer );
		parse_error ( parser, position,
			      buffer,
			      min::pgen_quote
				  ( flags_text ) );
	    }
	}
	else
	{
	    min::attr_insptr ap ( vp );
	    min::locate ( ap, min::dot_position );
	    min::phrase_position_vec_insptr
		pos = min::get ( ap );
	    min::phrase_position position = pos[i];
	    PAR::parse_error
		( parser, position,
		  "bad flags specifier ",
		   min::pgen_quote ( flags_text ) );
	}
    }
}

static void set_attr_multivalue
	( PAR::parser parser,
	  min::attr_insptr expap,
	  min::gen multivalue )
{
    min::obj_vec_ptr vp ( multivalue );
    min::unsptr n =
	min::attr_size_of ( vp );
    min::gen values[n];
    min::unsptr m = 0;
    for ( min::unsptr i = 0; i < n; ++ i )
    {
	min::gen value = min::attr ( vp, i );
	if ( min::is_attr_legal ( value ) )
	    values[m++] = value;
	else
	{
	    min::attr_ptr ap ( vp );
	    min::locate ( ap, min::dot_position );
	    min::phrase_position_vec_insptr pos =
		min::get ( ap );
	    min::phrase_position position = pos[i];
	    PAR::parse_error
		( parser, position,
		  "not a legal attribute value `",
		   min::pgen_never_quote ( value ),
		   "'; ignored" );
	}
    }
    min::set ( expap, values, m );
}

void PAR::compact
	( PAR::parser parser,
	  PAR::pass pass,
	  PAR::table::flags selectors,
	  PAR::token & first, PAR::token next,
	  const min::phrase_position position,
	  TAB::flags trace_flags,
	  min::uns32 type,
	  min::uns32 m,
	  PAR::attr * attributes,
	  min::uns32 n )
{
    PAR::execute_pass_parse
	 ( parser, pass, selectors, first, next );

    if ( first->next == next
         &&
	 ( ( first->type == PAR::BRACKETABLE
	     &&
	     type == PAR::BRACKETING )
	   ||
	   first->type == PAR::PURELIST )
	&&
	m != 0 )
    {
	min::obj_vec_insptr vp ( first->value );
	min::attr_insptr ap ( vp );

	min::locate ( ap, min::dot_position );
	min::phrase_position_vec_insptr pos =
	    min::get ( ap );
	pos->position = position;

	while ( m -- )
	{
	    min::locate ( ap, attributes->name );
	    if (    attributes->multivalue
	         == min::MISSING() )
		min::set ( ap, attributes->value );
	    else
	        ::set_attr_multivalue
		    ( parser, ap,
		      attributes->multivalue );
	    if ( attributes->flags != min::MISSING() )
	        ::set_attr_flags ( parser, ap,
		                   attributes->flags );
	    ++ attributes;
	}
	first->type = PAR::BRACKETED;
	first->position = position;
    }
    else
    {
	if ( m == 0 )
	    type = PAR::PURELIST;
        else if ( type == PAR::BRACKETING )
	    type = PAR::BRACKETED;

	// Temporary min::gen locatable.
	//
	min::locatable_gen exp;
	min::locatable_var
		<min::phrase_position_vec_insptr>
	    pos;

	// Count tokens.  Also replace non-natural
	// numbers and quoted strings by subexpressions.
	//
	min::uns32 t = 0;
	for ( PAR::token current = first;
	      current != next;
	      ++ t, current = current->next )
	{
	    if ( current->string != min::NULL_STUB )
		PAR::convert_token ( current );
	}

	// Count the number of non-MISSING extra
	// attributes.
	//
	min::uns32 c = n + m + 1;
	    // Add 1 for position.

	// Hash table size.
	//
	min::uns32 h = c;
	if ( h == 0 ) h = 1;
	if ( h > 4 ) h = 4;

	exp = min::new_obj_gen ( 3*( c + 2 ) + t, h );
	min::obj_vec_insptr expvp ( exp );

	min::init ( pos, parser->input_file,
	            position, t );

	for ( PAR::token current = first;
	      current != next; )
	{
	    if ( min::is_attr_legal ( current->value ) )
	    {
		min::attr_push(expvp) = current->value;
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
		      ( PAR::first_ref(parser),
			current->previous ) );
	}

	min::attr_insptr expap ( expvp );
	min::locate ( expap, min::dot_position );
	min::set ( expap, min::new_stub_gen ( pos ) );
	min::set_flag
	    ( expap, min::standard_attr_hide_flag );

	while ( m -- )
	{
	    min::locate ( expap, attributes->name );
	    if (    attributes->multivalue
	         == min::MISSING() )
		min::set ( expap, attributes->value );
	    else
	        ::set_attr_multivalue
		    ( parser, expap,
		      attributes->multivalue );
	    if ( attributes->flags != min::MISSING() )
	        ::set_attr_flags ( parser, expap,
		                   attributes->flags );
	    ++ attributes;
	}

	first = PAR::new_token ( type );
	PAR::put_before
	    ( first_ref(parser), next, first );

	PAR::value_ref(first) = exp;
	first->position = position;
    }

    PAR::trace_subexpression
        ( parser, first, trace_flags );
}

void PAR::compact_logical_line
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
			PAR::new_line );

    PAR::compact
	( parser, pass, selectors,
	  first, next, position,
	  trace_flags, PAR::BRACKETING,
	  n, attributes );

    PAR::value_type_ref(first) =
        min::LOGICAL_LINE();
}

bool PAR::compact_prefix_separator
	( PAR::parser parser,
	  PAR::pass pass,
	  PAR::table::flags selectors,
	  PAR::token first,
	  PAR::token next,
          const min::position & separator_found,
	  TAB::root line_sep,
	  TAB::flags trace_flags )
{
    if ( first->next == next
         &&
	 line_sep == min::NULL_STUB )
    {
	if ( first->type == PAR::IMPLIED_PREFIX )
	{
	    PAR::free
		( PAR::remove ( first_ref(parser),
				first ) );
	    return false;
	}
    }
    else
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

	if ( first->type == PAR::IMPLIED_PREFIX )
	{
	    min::uns32 unused_size = 0;
	    for ( PAR::token t = first->next;
	          t != next; t = t->next )
	        ++ unused_size;
	    if ( separator_found )
	        unused_size += 5;
	    PAR::value_ref(first) =
	        min::copy ( vp, unused_size );
	    vp = first->value;

	    min::init ( pos, parser->input_file,
	                first->position, 0 );
	    min::attr_insptr ap ( vp );
	    min::locate ( ap, min::dot_position );
	    min::set ( ap, min::new_stub_gen ( pos ) );
	    min::set_flag
	        ( ap, min::standard_attr_hide_flag );
	    if ( line_sep != min::NULL_STUB )
	    {
		min::locate ( ap, min::dot_terminator );
		min::set ( ap, line_sep->label );
		first->position.end = separator_found;
	    }
	}
	else
	{
	    min::attr_insptr ap ( vp );
	    min::locate ( ap, min::dot_position );
	    pos = min::get ( ap );
	    if ( line_sep != min::NULL_STUB )
	    {
		min::locate ( ap, min::dot_terminator );
		min::set ( ap, line_sep->label );
		first->position.end = separator_found;
	    }
	}
	pos->position = first->position;

	while ( current != next )
	{
	    if (    current->string
		 != min::NULL_STUB )
		PAR::convert_token
		    ( current );

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
		      ( PAR::first_ref
			    (parser),
			current->previous ) );
	}
    }

    first->type = PAR::BRACKETED;

    PAR::trace_subexpression
        ( parser, first, trace_flags );

    return true;
}

void PAR::compact_paragraph
	( PAR::parser parser,
	  PAR::token & first, PAR::token next,
	  TAB::flags trace_flags )
{
    MIN_REQUIRE
        ( first->value_type == PAR::paragraph_lexeme );
    if ( first->next == next ) return;

    min::phrase_position position =
	{ first->position.begin,
	  next->previous->position.end };

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
	if ( current->value_type == PAR::line_lexeme
	     ||
	     current->value_type == min::LOGICAL_LINE()
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

void PAR::internal::trace_subexpression
	( PAR::parser parser,
	  PAR::token token,
	  min::uns32 trace_flags )
{
    parser->printer
	<< min::bol << min::save_indent
	<< min::adjust_indent ( 4 )
	<< ( token->type == PAR::BRACKETED ?
	     "BRACKETED EXPRESSION: " :
	     token->type == PAR::BRACKETABLE ?
	     "BRACKETABLE EXPRESSION: " :
	     token->type == PAR::DERIVED ?
	     "DERIVED EXPRESSION: " :
	     token->type == PAR::PREFIX ?
	     "PREFIX EXPRESSION: " :
	     token->type == PAR::PURELIST ?
	     "PURELIST EXPRESSION: " :
	     "(UNKNOWN TYPE) EXPRESSION: " );

    if (   trace_flags
	 & PAR::TRACE_SUBEXPRESSION_ELEMENTS )
	parser->printer
	    << min::indent
	    << min::bom
	    << min::adjust_indent ( 4 )
	    << min::set_gen_format
	       ( parser->subexpression_gen_format )
	    << min::pgen ( token->value )
	    << min::eom;

    if (   trace_flags
	 & PAR::TRACE_SUBEXPRESSION_DETAILS )
	print_mapped ( parser->printer, token->value );

    if (   trace_flags
	 & PAR::TRACE_SUBEXPRESSION_LINES )
    {
	parser->printer
	    << min::spaces_if_before_indent
	    << min::pline_numbers
		( parser->input_file,
		  token->position )
	    << ":" << min::eol;
	min::print_phrase_lines
	    ( parser->printer,
	      parser->input_file, token->position );
    }

    parser->printer << min::restore_indent;
}

PAR::token PAR::find_separator
	( min::uns32 & count,
	  PAR::token first,
	  PAR::token next,
	  min::gen * separator,
	  min::uns32 n )
{
    count = 0;

    for ( ; first != next;
            first = first->next, ++ count )
    {
	if ( n == 0 ) continue;

	if ( first->value != separator[0] ) continue;
	if ( n == 1 ) break;

	PAR::token t = first->next;
	min::uns32 i = 1;
	for ( ; i < n; ++ i, t = t->next )
	{
	    if ( t == next ) break;
	    if ( t->value != separator[i] ) break;
	}
	if ( i == n ) break;
    }
    return first;
}

bool PAR::is_prefix_separator ( min::gen v )
{
    if ( ! min::is_obj ( v ) ) return false;
    min::obj_vec_ptr vp ( v );
    if ( min::size_of ( vp ) != 0 ) return false;
    min::attr_ptr ap ( vp );
    min::locate ( ap, min::dot_type );
    min::gen result = min::get ( ap );
    return min::is_str ( result )
           ||
	   min::is_lab ( result );
}

min::gen PAR::scan_name_string_label
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  ll::parser::parser parser,
	  min::uns64 accepted_types,
	  min::uns64 ignored_types,
	  min::uns64 end_types,
	  bool empty_name_ok )
{
    if ( i >= min::size_of ( vp ) )
        return min::MISSING();

    min::gen element = vp[i];

    if (    min::get ( element, min::dot_type )
         != PAR::doublequote )
        return min::MISSING();

    min::obj_vec_ptr ep = element;
    if ( min::size_of ( ep ) != 1 )
        return min::MISSING();

    min::str_ptr sp = ep[0];

    ++ i;

    if ( parser->name_scanner == NULL_STUB )
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
	  min::begin_ptr_of ( sp ),
	  parser->input_file->line_display );

    return LEX::scan_name_string
	( name_scanner_ref ( parser ),
	  accepted_types, ignored_types, end_types,
	  empty_name_ok );
}

min::gen PAR::scan_simple_name
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  min::gen end_value )
{
    min::uns32 j = i;
    min::uns32 s = min::size_of ( vp );
    min::uns64 accepted_types = 1ull << LEXSTD::word_t;

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
	accepted_types |= 1ull << LEXSTD::natural_t
	               |  1ull << LEXSTD::numeric_t;
    }

    if ( i == j ) return min::MISSING();
    else if ( i == j + 1 ) return vp[j];

    min::gen elements[i-j];
    memcpy ( elements, & vp[j], sizeof ( elements ) );
    return min::new_lab_gen ( elements, i - j );
}

min::gen PAR::scan_quoted_key
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  ll::parser::parser parser,
	  bool empty_name_ok )
{
    min::gen result =
        PAR::scan_name_string_label
	    ( vp, i, parser,
	      PAR::QUOTED_KEY_SCAN_MASK,
	      PAR::IGNORED_SCAN_MASK,
	      PAR::END_SCAN_MASK,
	      empty_name_ok );

    if ( result != min::ERROR() ) return result;

    min::attr_ptr ap ( vp );
    min::locate ( ap, min::dot_position );
    min::phrase_position_vec ppvec = min::get ( ap );
    MIN_ASSERT ( ppvec != min::NULL_STUB,
                 ".position attribute missing" );
    MIN_ASSERT ( ppvec->file == parser->input_file,
                 ".position attribute value file is"
		 " not the same as parser input_file" );

    return PAR::parse_error
               ( parser, ppvec[i],
	         "badly formed quoted key" );
}

min::gen PAR::scan_name
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  ll::parser::parser parser,
	  min::gen end_value )
{
    min::gen result =
        PAR::scan_quoted_key ( vp, i, parser );
    if ( result != min::MISSING() ) return result;

    result = PAR::scan_simple_name ( vp, i, end_value );

    if ( result != min::MISSING() ) return result;

    min::attr_ptr ap ( vp );
    min::locate ( ap, min::dot_position );
    min::phrase_position_vec ppvec = min::get ( ap );
    MIN_ASSERT ( i > 0,
                 "position at beginning of vector" );
    MIN_ASSERT ( ppvec != min::NULL_STUB,
                 ".position attribute missing" );
    MIN_ASSERT ( ppvec->file == parser->input_file,
                 ".position attribute value file is"
		 " not the same as parser input_file" );

    return PAR::parse_error
               ( parser, ppvec[i-1],
	         "expected quoted key or simple name"
		 " after" );
}

void PAR::convert_token ( PAR::token token )
{
    MIN_REQUIRE ( token->string != min::NULL_STUB );

    min::gen type;

    if (    token->type
	 == LEXSTD::quoted_string_t )
	type = PAR::doublequote;
    else
    {
	MIN_REQUIRE (    token->type
		      == LEXSTD::numeric_t );

	type = PAR::number_sign;
    }


    PAR::value_ref(token)
	= min::new_obj_gen ( 10, 1 );
    min::obj_vec_insptr elemvp
	( token->value );
    min::attr_push(elemvp) = min::MISSING();

    min::attr ( elemvp, 0 ) =
                    min::new_str_gen
			( min::begin_ptr_of
			      ( token->string ),
			  token->string->length );
    PAR::string_ref(token) =
	PAR::free_string ( token->string );

    min::attr_insptr elemap ( elemvp );
    min::locate ( elemap, min::dot_type );
    min::set ( elemap, type );

    token->type = PAR::BRACKETED;
}

min::gen PAR::parse_error
	( PAR::parser parser,
	  const min::phrase_position & pp,
	  const char * message1,
	  const min::op & message2,
	  const char * message3,
	  const min::op & message4,
	  const char * message5,
	  const min::op & message6,
	  const char * message7,
	  const min::op & message8,
	  const char * message9 )
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

    parser->printer << min::bol << min::bom
                    << min::set_indent ( 7 )
	            << "ERROR: in "
		    << min::pline_numbers
			   ( parser->input_file, pp )
	            << ": "
		    << message1 << message2
		    << message3 << message4
		    << message5 << message6
		    << message7 << message8
		    << message9 << ":" << min::eom;
    min::print_phrase_lines
        ( parser->printer, parser->input_file, pp );
    ++ parser->error_count;
    return min::ERROR();
}

void PAR::parse_warn
	( PAR::parser parser,
	  const min::phrase_position & pp,
	  const char * message1,
	  const min::op & message2,
	  const char * message3,
	  const min::op & message4,
	  const char * message5,
	  const min::op & message6,
	  const char * message7,
	  const min::op & message8,
	  const char * message9 )
{
    ++ parser->warning_count;

    if ( ( parser->trace_flags & PAR::TRACE_WARNINGS )
         == 0 )
        return;

    if (    parser->message_header.begin
         != min::MISSING_POSITION )
    {
	min::print_phrase_lines
	    ( parser->printer, parser->input_file,
	      parser->message_header, 0 );
	parser->message_header.begin =
	    min::MISSING_POSITION;
    }

    parser->printer << min::bol << min::bom
                    << min::set_indent ( 9 )
	            << "WARNING: in "
		    << min::pline_numbers
			   ( parser->input_file, pp )
	            << ": "
		    << message1 << message2
		    << message3 << message4
		    << message5 << message6
		    << message7 << message8
		    << message9 << ":" << min::eom;
    min::print_phrase_lines
        ( parser->printer, parser->input_file, pp );
}
