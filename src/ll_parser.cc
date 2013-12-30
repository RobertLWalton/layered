// Layers Language Parser Functions
//
// File:	ll_parser.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Dec 30 01:11:18 EST 2013
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
//	Contexts
//	Parser
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
min::locatable_gen PAR::dot_position;
min::locatable_gen PAR::dot_initiator;
min::locatable_gen PAR::dot_terminator;
min::locatable_gen PAR::dot_separator;
min::locatable_gen PAR::dot_middle;
min::locatable_gen PAR::dot_name;
min::locatable_gen PAR::dot_arguments;
min::locatable_gen PAR::dot_keys;
min::locatable_gen PAR::dot_oper;
min::locatable_gen PAR::doublequote;
min::locatable_gen PAR::number_sign;
min::locatable_gen PAR::new_line;
min::locatable_gen PAR::semicolon;
min::locatable_gen PAR::left_square;
min::locatable_gen PAR::right_square;
min::locatable_gen PAR::comma;
min::locatable_gen PAR::parser_lexeme;
min::locatable_gen PAR::standard_lexeme;
min::locatable_gen PAR::error_operator;
min::locatable_gen PAR::error_operand;
min::locatable_gen PAR::test;
min::locatable_gen PAR::begin;
min::locatable_gen PAR::end;
min::locatable_gen PAR::define;
min::locatable_gen PAR::undefine;
min::locatable_gen PAR::print;
min::locatable_gen PAR::pass_lexeme;
min::locatable_gen PAR::selector;
min::locatable_gen PAR::selectors;
min::locatable_gen PAR::context_lexeme;
min::locatable_gen PAR::default_lexeme;
min::locatable_gen PAR::with;
min::locatable_gen PAR::parsing;
min::locatable_gen PAR::after;
min::locatable_gen PAR::before;
min::locatable_gen PAR::at;
min::locatable_gen PAR::plus;
min::locatable_gen PAR::minus;
min::locatable_gen PAR::dotdotdot;

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
	= min::new_str_gen ( "TOP-LEVEL" );
    PAR::dot_position
	= min::new_lab_gen ( ".", "position" );
    PAR::dot_initiator
	= min::new_lab_gen ( ".", "initiator" );
    PAR::dot_terminator
	= min::new_lab_gen ( ".", "terminator" );
    PAR::dot_separator
	= min::new_lab_gen ( ".", "separator" );
    PAR::dot_middle
	= min::new_lab_gen ( ".", "middle" );
    PAR::dot_name
	= min::new_lab_gen ( ".", "name" );
    PAR::dot_arguments
	= min::new_lab_gen ( ".", "arguments" );
    PAR::dot_keys
	= min::new_lab_gen ( ".", "keys" );
    PAR::dot_oper
	= min::new_lab_gen ( ".", "operator" );

    PAR::doublequote = min::new_str_gen ( "\"" );
    PAR::number_sign = min::new_str_gen ( "#" );
    PAR::new_line = min::new_str_gen ( "\n" );
    PAR::semicolon = min::new_str_gen ( ";" );
    PAR::left_square = min::new_str_gen ( "[" );
    PAR::right_square = min::new_str_gen ( "]" );
    PAR::comma = min::new_str_gen ( "," );

    PAR::parser_lexeme = min::new_str_gen ( "parser" );
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
    PAR::pass_lexeme = min::new_str_gen ( "pass" );
    PAR::selector = min::new_str_gen ( "selector" );
    PAR::selectors = min::new_str_gen ( "selectors" );
    PAR::context_lexeme =
        min::new_str_gen ( "context" );
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
	( min::uns32 n, const min::uns32 * s )
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
	MIN_ASSERT ( token != ::free_list_first );
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

static min::uns32 pass_stub_disp[] = {
    min::DISP ( & PAR::pass_struct::parser ),
    min::DISP ( & PAR::pass_struct::next ),
    min::DISP_END };

static min::packed_struct<PAR::pass_struct>
    pass_type ( "ll::parser::pass_type",
                NULL, ::pass_stub_disp );

void PAR::place_after
	( PAR::parser parser,
	  PAR::pass pass,
	  PAR::pass previous )
{
    MIN_ASSERT ( pass != previous );

    PAR::remove ( pass );

    MIN_ASSERT ( previous->parser == parser );
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
    MIN_ASSERT ( pass != next );

    PAR::remove ( pass );

    if ( parser->pass_stack == NULL_STUB )
    {
        MIN_ASSERT ( next == NULL_STUB );

	next_ref(pass) = NULL_STUB;
	pass_stack_ref(parser) = pass;
    }
    else
    {
        PAR::pass current = parser->pass_stack;
        MIN_ASSERT ( current != next );
	while ( current->next != next )
	{
	    MIN_ASSERT ( current->next != NULL_STUB );
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

    min::ref<PAR::pass> p = pass_stack_ref ( parser );
    for ( PAR::pass current = p;
	  current != min::NULL_STUB;
	  current = p )
    {
        if ( current == pass )
	{
	    p = current->next;
	    PAR::parser_ref ( pass ) = min::NULL_STUB;
	    PAR::next_ref ( pass ) = min::NULL_STUB;
	    return;
	}
	p = PAR::next_ref ( current );
    }
    MIN_ABORT ( "Could not find pass on pass->parser"
                " pass stack" );
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

// Contexts
// --------

static min::uns32 context_stub_disp[] = {
    min::DISP ( & PAR::context_struct::next ),
    min::DISP_END };

static min::packed_struct_with_base
	<PAR::context_struct, TAB::root_struct>
    context_type
	( "ll::parser::context_type",
	  TAB::root_gen_disp,
	  ::context_stub_disp );
const min::uns32 & PAR::CONTEXT = context_type.subtype;

void PAR::push_context
	( min::gen label,
	  ll::parser::table::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  const ll::parser::table::new_flags
	      & new_selectors,
	  ll::parser::table::key_table context_table )
{
    min::locatable_var<PAR::context> context
        ( ::context_type.new_stub() );

    label_ref(context) = label;
    context->selectors = selectors;
    context->block_level = block_level;
    context->position = position;
    context->new_selectors = new_selectors;

    TAB::push ( context_table, (TAB::root) context );
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
    min::DISP ( & PAR::parser_struct::context_table ),
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
	parser->max_error_count = 100;

	TAB::init_name_table
	    ( trace_flag_name_table_ref(parser) );
	assert (    PAR::TRACE_WARNINGS
	         == 1ull << TAB::push_name
	              ( parser->trace_flag_name_table,
		        ::warnings ) );
	assert (    PAR::TRACE_PARSER_INPUT
	         == 1ull << TAB::push_name
	              ( parser->trace_flag_name_table,
		        ::parser_input ) );
	assert (    PAR::TRACE_PARSER_OUTPUT
	         == 1ull << TAB::push_name
		      ( parser->trace_flag_name_table,
		        ::parser_output ) );
	assert (    PAR::TRACE_PARSER_COMMANDS
	         == 1ull << TAB::push_name
		      ( parser->trace_flag_name_table,
		        ::parser_commands ) );
	assert (    PAR::TRACE_SUBEXPRESSION_ELEMENTS
	         == 1ull << TAB::push_name
		      ( parser->trace_flag_name_table,
		        ::subexpression_elements ) );
	assert (    PAR::TRACE_SUBEXPRESSION_DETAILS
	         == 1ull << TAB::push_name
		      ( parser->trace_flag_name_table,
		        ::subexpression_details ) );
	assert (    PAR::TRACE_SUBEXPRESSION_LINES
	         == 1ull << TAB::push_name
		      ( parser->trace_flag_name_table,
		        ::subexpression_lines ) );
	assert (    PAR::TRACE_KEYS
	         == 1ull << TAB::push_name
		      ( parser->trace_flag_name_table,
		        ::keys ) );

	TAB::init_undefined_stack
	    ( undefined_stack_ref(parser) );
	TAB::init_block_stack
	    ( block_stack_ref(parser) );

	TAB::init_name_table
	    ( selector_name_table_ref(parser) );

	assert (    PAR::PARSER_SELECTOR
		 == 1ull << TAB::push_name
		      ( parser->selector_name_table,
			PAR::parser_lexeme ) );

	PAR::context_table_ref(parser) =
	    TAB::create_key_table ( 256 );

	PAR::push_context
	    ( PAR::parser_lexeme,
	      0,
	      0,
	      PAR::top_level_position,
	      TAB::new_flags
	          ( PAR::PARSER_SELECTOR,
		      TAB::ALL_FLAGS
		    - PAR::PARSER_SELECTOR,
		    0 ),
	      parser->context_table );

	min::locatable_gen parser_test
	    ( min::new_lab_gen ( "parser", "test" ) );

	PAR::push_context
	    ( parser_test,
	      0,
	      0,
	      PAR::top_level_position,
	      TAB::new_flags ( 0, 0, 0 ),
	      parser->context_table );

	BRA::bracketed_pass bracketed_pass =
	    (BRA::bracketed_pass) BRA::new_pass();
	PAR::place_before ( parser,
	                   (PAR::pass) bracketed_pass );

	top_level_indentation_mark_ref(parser) =
	    BRA::push_indentation_mark
		( PAR::top_level, PAR::semicolon,
		  0, 0, PAR::top_level_position,
		  TAB::new_flags ( 0, 0, 0 ),
		  bracketed_pass->bracket_table );

	min::locatable_gen opening_square
	    ( min::new_str_gen ( "[" ) );
	min::locatable_gen closing_square
	    ( min::new_str_gen ( "]" ) );

	BRA::push_brackets
	    ( opening_square, closing_square,
	      PAR::PARSER_SELECTOR,
	      0, PAR::top_level_position,
	      TAB::new_flags ( 0, 0, 0 ),
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

    parser->eof = false;
    parser->finished_tokens = 0;

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

    parser->error_count = 0;
    parser->max_error_count = 100;
}

void PAR::init_input_stream
	( min::ref<PAR::parser> parser,
	  std::istream & in,
	  min::uns32 print_flags,
	  min::uns32 spool_lines )
{
    init ( parser );

    min::init_input_stream
        ( input_file_ref(parser),
	  in, print_flags, spool_lines );
}

void PAR::init_input_file
	( min::ref<PAR::parser> parser,
	  min::file ifile,
	  min::uns32 print_flags,
	  min::uns32 spool_lines )
{
    init ( parser );

    min::init_input_file
        ( input_file_ref(parser),
	  ifile, print_flags, spool_lines );
}

bool PAR::init_input_named_file
	( min::ref<PAR::parser> parser,
	  min::gen file_name,
	  min::uns32 print_flags,
	  min::uns32 spool_lines )
{
    init ( parser );

    return min::init_input_named_file
        ( input_file_ref(parser),
	  file_name, print_flags, spool_lines );
}

void PAR::init_input_string
	( min::ref<PAR::parser> parser,
	  min::ptr<const char> data,
	  min::uns32 print_flags,
	  min::uns32 spool_lines )
{
    init ( parser );

    min::init_input_string
        ( input_file_ref(parser),
	  data, print_flags, spool_lines );
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

    // True if last lexeme was a line break, so an end-
    // of-file is OK.
    //
    bool eof_ok = true;

    // Go to the first non-line-break non-comment token.
    //
    parser->input->add_tokens
	( parser, parser->input );
    PAR::token current = parser->first;
    assert ( current != NULL_STUB );
    while ( current->type == LEXSTD::line_break_t
            ||
	    current->type == LEXSTD::comment_t )
    {
	eof_ok =
	    ( current->type == LEXSTD::line_break_t );

	if ( current->next == parser->first )
	{
	    parser->input->add_tokens
		( parser, parser->input );
	    assert (    current->next
		     != parser->first );
	}
	current = current->next;
	PAR::free
	    ( PAR::remove ( first_ref(parser),
			    current->previous ) );
    }

    // Top level loop.
    //
    bool first_lexeme = true;
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
	{
	    if ( ! eof_ok )
	    {
		parser->printer
		    << min::bom
		    << min::set_indent ( 7 )
		    << "ERROR: line break missing"
		       " from end of file; "
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
	    break;
	}

	// If first lexeme check its indent is 0.
	//
	if ( first_lexeme )
	{
	    first_lexeme = false;
	    if ( current->indent != 0 )
	    {
		parser->printer
		    << min::bom
		    << min::set_indent ( 7 )
		    << "ERROR: first non-comment lexeme"
		       " is indented; "
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
	}

	// Get subexpression.  First is the first token
	// of the subexpression.
	//
	PAR::token previous =
	    current == parser->first ?
	    (PAR::token) NULL_STUB :
	    current->previous;


	TAB::flags selectors = parser->selectors;

	{
	    PAR::token current_save = current;
	    TAB::key_prefix prefix =
		PAR::find_key_prefix 
		    ( parser, current,
		      parser->context_table );
	    if ( prefix != NULL_STUB )
	    {
	        current = current_save;
		PAR::context context =
		    (PAR::context) prefix->first;
		MIN_ASSERT ( context != NULL_STUB );
		selectors |=
		    context->new_selectors.or_flags;
		selectors &= ~
		    context->new_selectors.not_flags;
		selectors ^=
		    context->new_selectors.xor_flags;
	    }
	}

	bool separator_found =
	    BRA::parse_bracketed_subexpression
		( parser, selectors,
		  current,
		  0,
		  parser->top_level_indentation_mark,
		  NULL );

	PAR::token first =
	    previous == NULL_STUB ?
	    parser->first :
	    previous->next;

        // If subexpression is not empty, compact it.
	//
	if ( first != current )
	{

	    min::phrase_position position =
	        { first->position.begin,
	          current->previous->position.end };

	    min::gen terminator = PAR::new_line;
	    if ( separator_found )
	    {
	        // If subexpression ends with an inden-
		// tation separator (e.g., `;'), delete
		// the separator from the subexpression
		// and make it into the terminator.
		//
		terminator =
		    parser->top_level_indentation_mark
			     ->line_separator->label;
		PAR::remove
		    ( parser, current, terminator );
	    }

	    min::gen g = first->value;
	    bool maybe_parser_command =
	        ( g == PAR::parser_lexeme );

	    PAR::attr attributes[1] =
		{ PAR::attr ( PAR::dot_terminator,
		              terminator ) };

	    min::uns32 error_count_save =
	        parser->error_count;

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

	    PAR::compact
		( parser, parser->pass_stack->next,
	          selectors,
		  PAR::BRACKETED, trace_flags,
		  first, current, position,
		  1, attributes );

	    min::gen result = min::FAILURE();
	    if (    parser->error_count
	         != error_count_save )
		result = min::ERROR();
	    else if ( maybe_parser_command )
	    {
	        min::obj_vec_ptr vp
		    ( current->previous->value );
		if ( vp != NULL_STUB )
		    result =
		      COM::parser_execute_command
			( vp, parser );
	    }

	    if ( result == min::FAILURE() )
	    {
		++ parser->finished_tokens;
	        if ( parser->output != NULL )
		    (* parser->output->remove_tokens)
		        ( parser, parser->output );
	    }
	    else
		PAR::free
		    ( PAR::remove
			( PAR::first_ref ( parser ),
			  current->previous ) );
	}

        // As there is no bracket stack, the token after
	// the subexpression is either a line break, end
	// of file, or token after an indentation sepa-
	// rator.  In the case of a line break, it must
	// be deleted.  End of file's are OK only at the
	// beginning or after such line deleted breaks.
	//
        if ( current->type == LEXSTD::line_break_t )
	{
	    if ( current->next == parser->first )
	    {
		parser->input->add_tokens
		    ( parser, parser->input );
		assert (    current->next
			 != parser->first );
	    }
	    current = current->next;
	    PAR::free
		( PAR::remove ( first_ref(parser),
			        current->previous ) );
	    eof_ok = true;
	}
	else eof_ok = false;
    }

    for ( PAR::pass pass = parser->pass_stack;
    	  pass != min::NULL_STUB;
	  pass = pass->next )
    {
	if ( pass->end_parse != NULL )
	    ( * pass->end_parse ) ( parser, pass );
    }
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
		      parser->selectors );

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
    assert ( block_level > 0 );
    TAB::block_struct & b =
        parser->block_stack[block_level-1];

    if ( block_level == 0 )
        return PAR::parse_error
	    ( parser,
	      position,
	      "not inside a block"
	      " (no begin block to end)" );
    else if ( name != b.name )
        return PAR::parse_error
	    ( parser,
	      position,
	      "innermost block name does not match `",
	      min::name_pgen ( name ), "'" );

    while ( parser->selector_name_table->length
            >
	    b.saved_selector_name_table_length )
        TAB::pop_name ( parser->selector_name_table );

    min::uns32 length = b.saved_undefined_stack_length;
    while ( parser->undefined_stack->length > length )
    {
        TAB::undefined_struct u =
	    min::pop ( parser->undefined_stack );
	u.root->selectors = u.saved_selectors;
    }

    parser->selectors = b.saved_selectors;

    min::uns64 collected_entries,
               collected_key_prefixes;

    TAB::end_block
        ( parser->context_table, block_level - 1,
	  collected_key_prefixes, collected_entries );
        
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
    MIN_ASSERT ( ( table_len & mask ) == 0 );
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
	    MIN_ASSERT ( 0 <= v && v < (1<<28) );
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
	    assert
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
    PAR::token token = new_token ( PAR::BRACKETABLE );
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
    min::locate ( ap, PAR::dot_position );
    min::set ( ap, min::new_stub_gen ( pos ) );
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
    min::locate ( ap, PAR::dot_position );
    min::set ( ap, min::new_stub_gen ( pos ) );
}

void PAR::compact
	( PAR::parser parser,
	  PAR::pass pass,
	  PAR::table::flags selectors,
	  min::uns32 type, TAB::flags trace_flags,
	  PAR::token & first, PAR::token next,
	  min::phrase_position position,
	  min::uns32 m,
	  PAR::attr * attributes,
	  min::uns32 n )
{
    if ( pass != min::NULL_STUB )
	(* pass->parse )
	     ( parser, pass, selectors, first, next );

    if ( first->next == next
         &&
	 first->type == PAR::BRACKETABLE
	 &&
	 type == PAR::BRACKETED )
    {
	min::obj_vec_insptr vp ( first->value );
	min::attr_insptr ap ( vp );

	min::locate ( ap, PAR::dot_position );
	min::phrase_position_vec_insptr pos =
	    min::get ( ap );
	pos->position = position;

	while ( m -- )
	{
	    assert (    attributes->value
		     != min::MISSING() );
	    min::locate ( ap, attributes->name );
	    min::set ( ap, attributes->value );
	    ++ attributes;
	}
	first->type = PAR::BRACKETED;
	first->position = position;
    }
    else
    {
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
	    if ( current->value == min::MISSING() )
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
	    min::attr_push(expvp) = current->value;
	    min::push ( pos ) = current->position;

	    current = current->next;
	    PAR::free
		( PAR::remove
		      ( PAR::first_ref(parser),
			current->previous ) );
	}

	min::attr_insptr expap ( expvp );
	min::locate ( expap, PAR::dot_position );
	min::set ( expap, min::new_stub_gen ( pos ) );

	while ( m -- )
	{
	    assert (    attributes->value
	             != min::MISSING() );
	    min::locate ( expap, attributes->name );
	    min::set ( expap, attributes->value );
	    ++ attributes;
	}

	first = PAR::new_token ( type );
	PAR::put_before
	    ( first_ref(parser), next, first );

	PAR::value_ref(first) = exp;
	first->position = position;
    }

    trace_flags &= (   PAR::TRACE_SUBEXPRESSION_ELEMENTS
	             + PAR::TRACE_SUBEXPRESSION_DETAILS
	             + PAR::TRACE_SUBEXPRESSION_LINES );

    if ( trace_flags )
    {
	parser->printer
	    << ( first->type == PAR::BRACKETED ?
		 "BRACKETED EXPRESSION: " :
		 "BRACKETABLE EXPRESSION: " );

	if (   trace_flags
	     & PAR::TRACE_SUBEXPRESSION_ELEMENTS )
	    parser->printer
		<< min::indent
		<< min::bom
		<< min::indent_pgen ( first->value )
		<< min::eom;
	if (   trace_flags
	     & PAR::TRACE_SUBEXPRESSION_DETAILS )
	    parser->printer
		<< min::save_print_format
		<< min::set_context_gen_flags
			( & min::
			    no_exp_context_gen_flags )
	        << min::map_pgen ( first->value )
		<< min::eol
		<< min::restore_print_format;
	if (   trace_flags
	     & PAR::TRACE_SUBEXPRESSION_LINES )
	{
	    parser->printer
		<< min::spaces_if_before_indent
	        << min::pline_numbers
		    ( parser->input_file,
		      position )
	        << ":" << min::eol;
	    min::print_phrase_lines
		( parser->printer,
		  parser->input_file, position );
	}
    }
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

min::gen PAR::get_initiator ( min::gen v )
{
    if ( ! min::is_obj ( v ) ) return min::MISSING();
    min::obj_vec_ptr vp ( v );
    min::attr_ptr ap ( vp );
    min::locate ( ap, PAR::dot_initiator );
    min::gen result = min::get ( ap );
    if ( result == min::NONE()
         ||
	 result == min::MULTI_VALUED() )
	return min::MISSING();
    else
    	return result;
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

    if ( get_initiator ( element ) != PAR::doublequote )
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
	  parser->input_file->print_flags );

    return LEX::scan_name_string
	( name_scanner_ref ( parser ),
	  accepted_types, ignored_types, end_types,
	  empty_name_ok );
}

void PAR::convert_token ( PAR::token token )
{
    assert ( token->value == min::MISSING() );

    min::gen initiator;

    if (    token->type
	 == LEXSTD::quoted_string_t )
	initiator = PAR::doublequote;
    else
    {
	assert (    token->type
		 == LEXSTD::number_t );

	initiator = PAR::number_sign;
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
    min::locate ( elemap, PAR::dot_initiator );
    min::set ( elemap, initiator );

    token->type = PAR::BRACKETED;
}

min::gen PAR::parse_error
	( PAR::parser parser,
	  const min::phrase_position & pp,
	  const char * message1,
	  const char * message2 )
{
    parser->printer << min::bom
                    << min::set_indent ( 7 )
	            << "ERROR: in "
		    << min::pline_numbers
			   ( parser->input_file, pp )
	            << ": " << message1 << message2
		    << ":" << min::eom;
    min::print_phrase_lines
        ( parser->printer, parser->input_file, pp );
    ++ parser->error_count;
    return min::ERROR();
}

min::gen PAR::parse_error
	( PAR::parser parser,
	  const min::phrase_position & pp,
	  const char * message1,
	  const min::op & message2,
	  const char * message3 )
{
    parser->printer << min::bom
                    << min::set_indent ( 7 )
	            << "ERROR: in "
		    << min::pline_numbers
			   ( parser->input_file, pp )
	            << ": " << message1 << message2
		    << message3 << ":" << min::eom;
    min::print_phrase_lines
        ( parser->printer, parser->input_file, pp );
    ++ parser->error_count;
    return min::ERROR();
}

void PAR::parse_warn
	( PAR::parser parser,
	  const min::phrase_position & pp,
	  const char * message1,
	  const char * message2 )
{
    if ( ( parser->trace_flags & PAR::TRACE_WARNINGS )
         == 0 )
        return;

    parser->printer << min::bom
                    << min::set_indent ( 9 )
	            << "WARNING: in "
		    << min::pline_numbers
			   ( parser->input_file, pp )
	            << ": " << message1 << message2
		    << ":" << min::eom;
    min::print_phrase_lines
        ( parser->printer, parser->input_file, pp );
}

void PAR::parse_warn
	( PAR::parser parser,
	  const min::phrase_position & pp,
	  const char * message1,
	  const min::op & message2,
	  const char * message3 )
{
    if ( ( parser->trace_flags & PAR::TRACE_WARNINGS )
         == 0 )
        return;

    parser->printer << min::bom
                    << min::set_indent ( 9 )
	            << "WARNING: in "
		    << min::pline_numbers
			   ( parser->input_file, pp )
	            << ": " << message1 << message2
		    << message3 << ":" << min::eom;
    min::print_phrase_lines
        ( parser->printer, parser->input_file, pp );
}
