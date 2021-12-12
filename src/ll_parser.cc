// Layered Languages Parser Functions
//
// File:	ll_parser.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun Dec 12 13:10:34 EST 2021
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
# define PARLEX ll::parser::lexeme
# define TAB ll::parser::table
# define BRA ll::parser::bracketed
# define COM ll::parser::command
# define PARSTD ll::parser::standard

min::locatable_gen PARLEX::NONE;
min::locatable_gen PARLEX::top_level;
min::locatable_gen PARLEX::line_level;
min::locatable_gen PARLEX::top;
min::locatable_gen PARLEX::level;
min::locatable_gen PARLEX::star_top_level_star;
min::locatable_gen PARLEX::dot_oper;
min::locatable_gen PARLEX::doublequote;
min::locatable_gen PARLEX::number_sign;
min::locatable_gen PARLEX::new_line;
min::locatable_gen PARLEX::semicolon;
min::locatable_gen PARLEX::left_parenthesis;
min::locatable_gen PARLEX::right_parenthesis;
min::locatable_gen PARLEX::left_square;
min::locatable_gen PARLEX::right_square;
min::locatable_gen PARLEX::left_curly;
min::locatable_gen PARLEX::right_curly;
min::locatable_gen PARLEX::comma;
min::locatable_gen PARLEX::colon;
min::locatable_gen PARLEX::double_colon;
min::locatable_gen PARLEX::equal;
min::locatable_gen PARLEX::vbar;
min::locatable_gen PARLEX::eaindent;
min::locatable_gen PARLEX::ealeindent;
min::locatable_gen PARLEX::ealtindent;
min::locatable_gen PARLEX::eapbreak;
min::locatable_gen PARLEX::ealsep;
min::locatable_gen PARLEX::eaoclosing;
min::locatable_gen PARLEX::eiparagraph;
min::locatable_gen PARLEX::eprefix;
min::locatable_gen PARLEX::etprefix;
min::locatable_gen PARLEX::eheader;
min::locatable_gen PARLEX::sticky;
min::locatable_gen PARLEX::reset;
min::locatable_gen PARLEX::continuing;
min::locatable_gen PARLEX::default_opt;
min::locatable_gen PARLEX::other_ea_opt;
min::locatable_gen PARLEX::default_ea_opt;
min::locatable_gen PARLEX::non_default_ea_opt;
min::locatable_gen PARLEX::other_enable_opt;
min::locatable_gen PARLEX::default_enable_opt;
min::locatable_gen PARLEX::non_default_enable_opt;
min::locatable_gen PARLEX::other_selectors;
min::locatable_gen PARLEX::parser;
min::locatable_gen PARLEX::data;
min::locatable_gen PARLEX::atom;
min::locatable_gen PARLEX::prefix;
min::locatable_gen PARLEX::header;
min::locatable_gen PARLEX::separator;
min::locatable_gen PARLEX::line;
min::locatable_gen PARLEX::paragraph;
min::locatable_gen PARLEX::standard;
min::locatable_gen PARLEX::test;
min::locatable_gen PARLEX::begin;
min::locatable_gen PARLEX::end;
min::locatable_gen PARLEX::define;
min::locatable_gen PARLEX::undefine;
min::locatable_gen PARLEX::print;
min::locatable_gen PARLEX::block;
min::locatable_gen PARLEX::pass;
min::locatable_gen PARLEX::mapped;
min::locatable_gen PARLEX::lexeme;
min::locatable_gen PARLEX::selector;
min::locatable_gen PARLEX::selectors;
min::locatable_gen PARLEX::options;
min::locatable_gen PARLEX::group;
min::locatable_gen PARLEX::lexical;
min::locatable_gen PARLEX::master;
min::locatable_gen PARLEX::implied;
min::locatable_gen PARLEX::subprefix;
min::locatable_gen PARLEX::reformatter;
min::locatable_gen PARLEX::default_lexeme;
min::locatable_gen PARLEX::with;
min::locatable_gen PARLEX::parsing;
min::locatable_gen PARLEX::after;
min::locatable_gen PARLEX::before;
min::locatable_gen PARLEX::at;
min::locatable_gen PARLEX::plus;
min::locatable_gen PARLEX::minus;
min::locatable_gen PARLEX::dotdotdot;
min::locatable_gen PARLEX::yes;
min::locatable_gen PARLEX::no;
min::locatable_gen PARLEX::keep;
min::locatable_gen PARLEX::enabled;
min::locatable_gen PARLEX::disabled;
min::locatable_gen PARLEX::star_parser;
min::locatable_gen PARLEX::parser_colon;
min::locatable_gen PARLEX::parser_test_colon;
min::locatable_gen PARLEX::ID;
min::locatable_gen PARLEX::character;

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
    PARLEX::NONE = min::new_str_gen ( "NONE" );
    PARLEX::top_level
	= min::new_lab_gen ( "TOP", "LEVEL" );
    PARLEX::line_level
	= min::new_lab_gen ( "LINE", "LEVEL" );
    PARLEX::top = min::new_str_gen ( "top" );
    PARLEX::level = min::new_str_gen ( "level" );
    PARLEX::star_top_level_star
	= min::new_lab_gen ( "*TOP*", "*LEVEL*" );
    PARLEX::dot_oper
	= min::new_str_gen ( ".operator" );

    PARLEX::doublequote = min::new_str_gen ( "\"" );
    PARLEX::number_sign = min::new_str_gen ( "#" );
    PARLEX::new_line = min::new_str_gen ( "\n" );
    PARLEX::semicolon = min::new_str_gen ( ";" );
    PARLEX::left_parenthesis = min::new_str_gen ( "(" );
    PARLEX::right_parenthesis =
        min::new_str_gen ( ")" );
    PARLEX::left_square = min::new_str_gen ( "[" );
    PARLEX::right_square = min::new_str_gen ( "]" );
    PARLEX::left_curly = min::new_str_gen ( "{" );
    PARLEX::right_curly = min::new_str_gen ( "}" );
    PARLEX::comma = min::new_str_gen ( "," );
    PARLEX::colon = min::new_str_gen ( ":" );
    PARLEX::double_colon = min::new_str_gen ( "::" );
    PARLEX::equal = min::new_str_gen ( "=" );
    PARLEX::vbar = min::new_str_gen ( "|" );

    PARLEX::eaindent =
    	min::new_lab_gen
	    ( "end", "at", "indent" );
    PARLEX::ealeindent =
    	min::new_lab_gen
	    ( "end", "at", "le", "indent" );
    PARLEX::ealtindent =
    	min::new_lab_gen
	    ( "end", "at", "lt", "indent" );
    PARLEX::eapbreak =
    	min::new_lab_gen
	    ( "end", "at", "paragraph", "break" );
    PARLEX::ealsep =
    	min::new_lab_gen
	    ( "end", "at", "line", "separator" );
    PARLEX::eaoclosing =
    	min::new_lab_gen
	    ( "end", "at", "outer", "closing" );
    PARLEX::eiparagraph =
    	min::new_lab_gen
	    ( "enable", "indented", "paragraph" );
    PARLEX::eprefix =
    	min::new_lab_gen
	    ( "enable", "prefix" );
    PARLEX::etprefix =
    	min::new_lab_gen
	    ( "enable", "table", "prefix" );
    PARLEX::eheader =
    	min::new_lab_gen ( "enable", "header" );
    PARLEX::sticky = min::new_str_gen ( "sticky" );
    PARLEX::reset = min::new_str_gen ( "reset" );
    PARLEX::continuing =
        min::new_str_gen ( "continuing" );

    PARLEX::default_opt =
        min::new_lab_gen ( "default", "options" );
    PARLEX::other_ea_opt =
        min::new_lab_gen ( "other", "end", "at",
	                   "options" );
    PARLEX::default_ea_opt =
        min::new_lab_gen
	    ( "default", "end", "at", "options" );
    PARLEX::non_default_ea_opt =
        min::new_lab_gen
	    ( "non-default", "end", "at", "options" );
    PARLEX::other_enable_opt =
        min::new_lab_gen ( "other", "enable",
	                   "options" );
    PARLEX::default_enable_opt =
        min::new_lab_gen
	    ( "default", "enable", "options" );
    PARLEX::non_default_enable_opt =
        min::new_lab_gen
	    ( "non-default", "enable", "options" );
    PARLEX::other_selectors =
        min::new_lab_gen ( "other", "selectors" );

    PARLEX::parser = min::new_str_gen ( "parser" );
    PARLEX::data = min::new_str_gen ( "data" );
    PARLEX::atom = min::new_str_gen ( "atom" );
    PARLEX::prefix = min::new_str_gen ( "prefix" );
    PARLEX::header = min::new_str_gen ( "header" );
    PARLEX::separator =
        min::new_str_gen ( "separator" );
    PARLEX::line = min::new_str_gen ( "line" );
    PARLEX::paragraph =
        min::new_str_gen ( "paragraph" );
    PARLEX::standard =
        min::new_str_gen ( "standard" );

    PARLEX::test = min::new_str_gen ( "test" );
    PARLEX::begin = min::new_str_gen ( "begin" );
    PARLEX::end = min::new_str_gen ( "end" );
    PARLEX::define = min::new_str_gen ( "define" );
    PARLEX::undefine = min::new_str_gen ( "undefine" );
    PARLEX::print = min::new_str_gen ( "print" );
    PARLEX::block = min::new_str_gen ( "block" );
    PARLEX::pass = min::new_str_gen ( "pass" );
    PARLEX::mapped = min::new_str_gen ( "mapped" );
    PARLEX::lexeme = min::new_str_gen ( "lexeme" );
    PARLEX::selector = min::new_str_gen ( "selector" );
    PARLEX::selectors =
        min::new_str_gen ( "selectors" );
    PARLEX::options = min::new_str_gen ( "options" );
    PARLEX::group = min::new_str_gen ( "group" );
    PARLEX::lexical = min::new_str_gen ( "lexical" );
    PARLEX::master = min::new_str_gen ( "master" );
    PARLEX::implied = min::new_str_gen ( "implied" );
    PARLEX::subprefix =
        min::new_str_gen ( "subprefix" );
    PARLEX::reformatter =
        min::new_str_gen ( "reformatter" );
    PARLEX::default_lexeme =
        min::new_str_gen ( "default" );
    PARLEX::with = min::new_str_gen ( "with" );
    PARLEX::parsing = min::new_str_gen ( "parsing" );
    PARLEX::after = min::new_str_gen ( "after" );
    PARLEX::before = min::new_str_gen ( "before" );
    PARLEX::at = min::new_str_gen ( "at" );
    PARLEX::plus = min::new_str_gen ( "+" );
    PARLEX::minus = min::new_str_gen ( "-" );
    PARLEX::dotdotdot = min::new_str_gen ( "..." );
    PARLEX::yes = min::new_str_gen ( "yes" );
    PARLEX::no = min::new_str_gen ( "no" );
    PARLEX::keep = min::new_str_gen ( "keep" );
    PARLEX::enabled = min::new_str_gen ( "enabled" );
    PARLEX::disabled = min::new_str_gen ( "disabled" );
    PARLEX::star_parser =
        min::new_str_gen ( "*PARSER*" );
    PARLEX::parser_colon =
        min::new_lab_gen ( "*PARSER*", ":" );
    PARLEX::parser_test_colon =
        min::new_lab_gen ( "*PARSER*", "*TEST*", ":" );
    PARLEX::ID = min::new_str_gen ( "ID" );
    PARLEX::character =
        min::new_str_gen ( "character" );

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
    token->type = type;
    return token;
}

void PAR::free ( PAR::token token )
{
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

min::uns32 PAR::find_token_type 
	( min::ref<min::gen> value_type,
	  min::gen value )
{
    value_type = min::MISSING();

    if ( min::is_obj ( value ) )
    {
	min::obj_vec_ptr vp ( value );
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
	    value_type = initiator;
	    return PAR::BRACKETED;
	}
	else if ( type != min::NONE() )
	{
	    if ( min::size_of ( vp ) == 0 )
	    {
		value_type = type;
		return PAR::PREFIX;
	    }
	    else
		return PAR::BRACKETED;
	}
	else
	    return PAR::BRACKETABLE;
    }
    else if ( value != min::NONE() )
	return PAR::DERIVED;
    else
	return 0;
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
    min::DISP ( & PAR::parser_struct::id_map ),
    min::DISP ( & PAR::parser_struct::lexeme_map ),
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

	min::init ( id_map_ref ( parser ) );
	* (min::Uchar *) & parser->id_map
	                         ->ID_character =
	    min::NO_UCHAR;

	TAB::init_undefined_stack
	    ( undefined_stack_ref(parser) );
	TAB::init_block_stack
	    ( block_stack_ref(parser) );

	TAB::init_name_table
	    ( selector_name_table_ref(parser) );

	MIN_REQUIRE
	    (    PAR::EAINDENT_OPT
	      == 1ull << TAB::push_name
		      ( parser->selector_name_table,
			PARLEX::eaindent ) );
	MIN_REQUIRE
	    (    PAR::EALEINDENT_OPT
	      == 1ull << TAB::push_name
		      ( parser->selector_name_table,
			PARLEX::ealeindent ) );
	MIN_REQUIRE
	    (    PAR::EALTINDENT_OPT
	      == 1ull << TAB::push_name
		      ( parser->selector_name_table,
			PARLEX::ealtindent ) );
	MIN_REQUIRE
	    (    PAR::EAPBREAK_OPT
	      == 1ull << TAB::push_name
		      ( parser->selector_name_table,
			PARLEX::eapbreak ) );
	MIN_REQUIRE
	    (    PAR::EALSEP_OPT
	      == 1ull << TAB::push_name
		      ( parser->selector_name_table,
			PARLEX::ealsep ) );
	MIN_REQUIRE
	    (    PAR::EAOCLOSING_OPT
	      == 1ull << TAB::push_name
		      ( parser->selector_name_table,
			PARLEX::eaoclosing ) );
	MIN_REQUIRE
	    (    PAR::EIPARAGRAPH_OPT
	      == 1ull << TAB::push_name
		      ( parser->selector_name_table,
			PARLEX::eiparagraph ) );
	MIN_REQUIRE
	    (    PAR::EPREFIX_OPT
	      == 1ull << TAB::push_name
		      ( parser->selector_name_table,
			PARLEX::eprefix ) );
	MIN_REQUIRE
	    (    PAR::ETPREFIX_OPT
	      == 1ull << TAB::push_name
		      ( parser->selector_name_table,
			PARLEX::etprefix ) );
	MIN_REQUIRE
	    (    PAR::EHEADER_OPT
	      == 1ull << TAB::push_name
		      ( parser->selector_name_table,
			PARLEX::eheader ) );
	MIN_REQUIRE
	    (    PAR::STICKY_OPT
	      == 1ull << TAB::push_name
		      ( parser->selector_name_table,
			PARLEX::sticky ) );
	MIN_REQUIRE
	    (    PAR::CONTINUING_OPT
	      == 1ull << TAB::push_name
		      ( parser->selector_name_table,
			PARLEX::continuing ) );

	while ( parser->selector_name_table->length
	        < SELECTOR_OFFSET )
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
			PARLEX::top_level ) );

	MIN_REQUIRE
	    (    PAR::LINE_LEVEL_SELECTOR
	      == 1ull << TAB::push_name
		      ( parser->selector_name_table,
			PARLEX::line_level ) );

	MIN_REQUIRE
	    (    PAR::DATA_SELECTOR
	      == 1ull << TAB::push_name
		      ( parser->selector_name_table,
			PARLEX::data ) );

	MIN_REQUIRE
	    (    PAR::ATOM_SELECTOR
	      == 1ull << TAB::push_name
		      ( parser->selector_name_table,
			PARLEX::atom ) );

	PAR::selector_group_name_table_ref(parser) =
	    TAB::create_key_table ( 32 );

	TAB::push_root
	    ( PARLEX::default_opt, PAR::DEFAULT_OPT,
	      0, PAR::top_level_position,
	      parser->selector_group_name_table );
	TAB::push_root
	    ( PARLEX::other_ea_opt, PAR::ALL_EA_OPT,
	      0, PAR::top_level_position,
	      parser->selector_group_name_table );
	TAB::push_root
	    ( PARLEX::default_ea_opt,
	      PAR::DEFAULT_EA_OPT,
	      0, PAR::top_level_position,
	      parser->selector_group_name_table );
	TAB::push_root
	    ( PARLEX::non_default_ea_opt,
	      PAR::ALL_EA_OPT - PAR::DEFAULT_EA_OPT,
	      0, PAR::top_level_position,
	      parser->selector_group_name_table );
	TAB::push_root
	    ( PARLEX::other_enable_opt,
	      PAR::ALL_ENABLE_OPT,
	      0, PAR::top_level_position,
	      parser->selector_group_name_table );
	TAB::push_root
	    ( PARLEX::default_enable_opt,
	      PAR::DEFAULT_ENABLE_OPT,
	      0, PAR::top_level_position,
	      parser->selector_group_name_table );
	TAB::push_root
	    ( PARLEX::non_default_enable_opt,
	      + PAR::ALL_ENABLE_OPT
	      - PAR::DEFAULT_ENABLE_OPT,
	      0, PAR::top_level_position,
	      parser->selector_group_name_table );
	TAB::push_root
	    ( PARLEX::other_selectors,
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
		( PARLEX::star_top_level_star,
		  min::MISSING(),
		  0, 0, PAR::top_level_position,
		  TAB::new_flags
		      ( PAR::DEFAULT_OPT,
		        BRA::INDENTATION_MARK_SELECTORS
			+ PAR::ALL_OPT
			- PAR::DEFAULT_OPT,
		        0 ),
		  min::MISSING(),
		  PAR::MISSING_MASTER,
		  PAR::MISSING_MASTER,
		  bracketed_pass->bracket_table );

	parser->selectors =
	      parser->top_level_indentation_mark
		    ->parsing_selectors.or_flags
	    | PAR::TOP_LEVEL_OFF_SELECTORS
	    | PAR::ALWAYS_SELECTOR;
	parser->lexical_master =
	    PAR::MISSING_MASTER;

	BRA::push_indentation_mark
	    ( PARLEX::parser_colon,
	      PARLEX::semicolon,
	      PAR::TOP_LEVEL_SELECTOR,
	      0, PAR::top_level_position,
	      PAR::parsing_selectors
		  ( PAR::DATA_SELECTOR ),
	      min::MISSING(),
	      PAR::MISSING_MASTER,
	      PAR::MISSING_MASTER,
	      bracketed_pass->bracket_table );

	BRA::push_indentation_mark
	    ( PARLEX::parser_test_colon,
	      PARLEX::semicolon,
	      PAR::TOP_LEVEL_SELECTOR,
	      0, PAR::top_level_position,
	      TAB::new_flags ( 0, 0, 0 ),
	      min::MISSING(),
	      PAR::MISSING_MASTER,
	      PAR::MISSING_MASTER,
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

	min::locatable_gen opening_double_quote
	    ( min::new_str_gen ( "``" ) );
	min::locatable_gen closing_double_quote
	    ( min::new_str_gen ( "''" ) );

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
	min::locatable_gen multivalue_arguments
	    ( min::new_obj_gen ( 1 ) );
	min::obj_vec_insptr mvvp
	    ( multivalue_arguments );
	min::attr_push ( mvvp ) = comma;

	BRA::push_brackets
	    ( PARLEX::left_parenthesis,
	      PARLEX::right_parenthesis,
	      PAR::DATA_SELECTOR,
	      0, PAR::top_level_position,
	      TAB::new_flags ( 0, 0, 0 ),
	      min::NULL_STUB, min::MISSING(),
	      bracketed_pass->bracket_table );

	BRA::push_brackets
	    ( PARLEX::left_square, PARLEX::right_square,
	      PAR::DATA_SELECTOR,
	      0, PAR::top_level_position,
	      TAB::new_flags ( 0, 0, 0 ),
	      min::NULL_STUB, min::MISSING(),
	      bracketed_pass->bracket_table );

	BRA::push_brackets
	    ( opening_double_brace,
	      closing_double_brace,
	      PAR::DATA_SELECTOR,
	      0, PAR::top_level_position,
	      TAB::new_flags ( 0, 0, 0 ),
	      min::NULL_STUB, min::MISSING(),
	      bracketed_pass->bracket_table );

	BRA::push_brackets
	    ( opening_quote,
	      closing_quote,
	      PAR::DATA_SELECTOR,
	      0, PAR::top_level_position,
	      TAB::new_flags
	          ( PAR::ATOM_SELECTOR,
		      PAR::COMMAND_SELECTORS
		    ^ PAR::ATOM_SELECTOR ),
	      min::NULL_STUB, min::MISSING(),
	      bracketed_pass->bracket_table );

	BRA::push_brackets
	    ( opening_double_quote,
	      closing_double_quote,
	      PAR::DATA_SELECTOR,
	      0, PAR::top_level_position,
	      TAB::new_flags ( 0, 0, 0 ),
	      min::NULL_STUB, min::MISSING(),
	      bracketed_pass->bracket_table );

	BRA::push_brackets
	    ( opening_brace_star,
	      closing_star_brace,
	      PAR::DATA_SELECTOR,
	      0, PAR::top_level_position,
	      TAB::new_flags ( 0, 0, 0 ),
	      PAR::find_reformatter
		  ( multivalue,
		    BRA::untyped_reformatter_stack ),
	      multivalue_arguments,
	      bracketed_pass->bracket_table );

	BRA::push_brackets
	    ( opening_square_angle,
	      angle_closing_square,
	      PAR::DATA_SELECTOR + PAR::ATOM_SELECTOR,
	      0, PAR::top_level_position,
	      TAB::new_flags ( PAR::ATOM_SELECTOR,
			         PAR::COMMAND_SELECTORS
			       ^ PAR::ATOM_SELECTOR ),
	      PAR::find_reformatter
		  ( label_name,
		    BRA::untyped_reformatter_stack ),
	      min::MISSING(),
	      bracketed_pass->bracket_table );

	BRA::push_brackets
	    ( opening_square_dollar,
	      dollar_closing_square,
	      PAR::DATA_SELECTOR + PAR::ATOM_SELECTOR,
	      0, PAR::top_level_position,
	      TAB::new_flags ( PAR::ATOM_SELECTOR,
			         PAR::COMMAND_SELECTORS
			       ^ PAR::ATOM_SELECTOR ),
	      PAR::find_reformatter
		  ( special_name,
		    BRA::untyped_reformatter_stack ),
	      min::MISSING(),
	      bracketed_pass->bracket_table );

	BRA::push_typed_brackets
	    ( PARLEX::left_curly,
	      PARLEX::vbar,
	      double_vbar,
	      PARLEX::right_curly,
	      PAR::DATA_SELECTOR,
	      0, PAR::top_level_position,
	      TAB::new_flags ( 0, 0, 0 ),
	      PAR::DATA_SELECTOR,
	      PARLEX::colon, PARLEX::equal,
	      PARLEX::comma, PARLEX::no,
	      PARLEX::left_square,
	      min::standard_attr_flag_parser,
	      opening_brace_star,
	      0,
	      bracketed_pass->bracket_table );

	if ( define_standard )
	{
	    PARSTD::init_input ( parser );
	    PARSTD::define_standard ( parser );
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

    min::uns64 collected_entries = 0;
    TAB::end_block ( parser->lexeme_map, -1,
                     collected_entries );

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

static void init_line_variables
        ( BRA::line_variables & line_variables,
	  PAR::parser parser,
	  PAR::token & current )
{
    if ( current->type == LEXSTD::indent_t
         &&
	 current->indent != 0 )
        PAR::parse_error
	    ( parser,
	      current->position,
	      "current indentation is not zero; top"
	      " level zero indentation assumed" );

    BRA::init_line_variables
        ( line_variables,
	  parser->top_level_indentation_mark,
	  parser,
	  parser->selectors,
	  0,
	  current );

    parser->at_paragraph_beginning = true;
}

inline bool operator ==
	( const BRA::line_data & data1,
	  const BRA::line_data & data2 )
{
    return    data1.paragraph_lexical_master
           == data2.paragraph_lexical_master
           &&
              data1.line_lexical_master
           == data2.line_lexical_master
           &&
	   data1.selectors == data2.selectors
	   &&
	   data1.implied_header == data2.implied_header
	   &&
	   data1.header_entry == data2.header_entry
	   &&
	      data1.header_selectors
	   == data2.header_selectors;
}

inline bool line_data_as_inited
        ( const BRA::line_variables & line_variables )
{
    return    line_variables.current
	   == line_variables.indentation_paragraph
	   &&
	      line_variables.paragraph
	   == line_variables.indentation_paragraph
	   &&
	      line_variables.implied_paragraph
	   == line_variables
	          .indentation_implied_paragraph;
}

inline TAB::flags output_trace_flags
	( PAR::parser parser )
{
    TAB::flags trace_flags = parser->trace_flags;
    if ( trace_flags & PAR::TRACE_PARSER_OUTPUT )
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

    return trace_flags;
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
	PAR::ensure_next ( parser, current );
	current = current->next;
	if (    current->previous->type
	     != LEXSTD::start_of_file_t )
	    PAR::free
		( PAR::remove ( first_ref(parser),
				current->previous ) );
    }

    TAB::flags trace_flags =
        ::output_trace_flags ( parser );

    // Top level loop.
    //
    BRA::line_variables line_variables;
    if ( current->type == LEXSTD::end_of_file_t )
        goto END_PARSE;
    ::init_line_variables
        ( line_variables, parser, current );

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

	PAR::token previous = current->previous;
	bool maybe_parser_command =
	    parse_paragraph_element
		( parser, current,
		  & line_variables,
		  0 );
	PAR::token output = previous->next;
	if ( output == current ) break;

	min::gen result = min::FAILURE();
	if ( maybe_parser_command )
	{
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
		         == PARLEX::parser_test_colon )
			COM::parser_test_execute_command
			    ( parser, vp[0] );
		    else if (    initiator
		              == PARLEX::parser_colon )
		    {
		        MIN_REQUIRE
			    ( ::line_data_as_inited
			          ( line_variables ) );

			COM::parser_execute_command
			    ( parser, vp[0] );

			::init_line_variables
			    ( line_variables, parser,
			      current );
			trace_flags =
			    ::output_trace_flags
			        ( parser );
		    }
		    else
			result = min::FAILURE();
		}
	    }
	}

	if ( result == min::FAILURE() )
	{
	    if ( parser->output != NULL_STUB )
	    {
		++ parser->finished_tokens;
		(* parser->output->remove_tokens)
		    ( parser, parser->output );
	    }
	    else
	    {
		trace_subexpression
		    ( parser, output, trace_flags );
		PAR::free
		    ( PAR::remove
			( PAR::first_ref ( parser ),
			  output ) );
	    }
	}
	else
	    PAR::free
		( PAR::remove
		    ( PAR::first_ref ( parser ),
		      output ) );
    }

END_PARSE:

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
      min::uns32 minimum_arguments,
      min::uns32 maximum_arguments,
      PAR::reformatter_function reformatter_function,
      min::ref<PAR::reformatter> stack )
{
    min::locatable_var<PAR::reformatter> r
        ( ::reformatter_type.new_stub() );
    PAR::next_ref(r) = stack;
    PAR::name_ref(r) = name;
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
		      parser->
		          top_level_indentation_mark,
		      parser->trace_flags,
		      parser->id_map->ID_character );

    BRA::bracketed_pass bracketed_pass =
        (BRA::bracketed_pass) parser->pass_stack;

    min::uns32 block_level =
        PAR::block_level ( parser );
    BRA::indentation_mark imark =
        parser->top_level_indentation_mark;

    top_level_indentation_mark_ref(parser) =
	BRA::push_indentation_mark
	    ( PARLEX::star_top_level_star,
	      imark->line_sep == min::NULL_STUB ?
	          min::MISSING() :
		  imark->line_sep->label,
	      0,
	      block_level, position,
	      imark->parsing_selectors,
	      imark->implied_header,
	      imark->paragraph_lexical_master,
	      imark->line_lexical_master,
	      bracketed_pass->bracket_table );

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

    if ( block_level == 0 )
        return PAR::parse_error
	    ( parser,
	      position,
	      "not inside a block"
	      " (no begin block to end)" );

    min::ref<TAB::block_struct> b =
        parser->block_stack[block_level-1];
    if ( name != (&b)->name )
        return PAR::parse_error
	    ( parser,
	      position,
	      "innermost block name does not match `",
	      min::pgen_name ( name ), "'" );

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

    min::uns64 collected_entries = 0;
    TAB::end_block ( parser->lexeme_map,
                     block_level - 1,
                     collected_entries );

    top_level_indentation_mark_ref(parser) =
        (&b)->saved_top_level_indentation_mark;
    parser->selectors =
	  parser->top_level_indentation_mark
	        ->parsing_selectors.or_flags
	| PAR::TOP_LEVEL_OFF_SELECTORS
	| PAR::ALWAYS_SELECTOR;
    parser->trace_flags = (&b)->saved_trace_flags;
    * (min::Uchar *) & parser->id_map->ID_character =
        (&b)->saved_ID_character;

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
             ! ( ( 1 << current->type )
	         & LEXSTD::symbol_mask ) )
	    break;

	min::gen e = current->value;
	uns32 hash = min::hash ( e );

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

	PAR::ensure_next ( parser, current );
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

bool PAR::set_attr_flags
	( PAR::parser parser,
	  min::attr_insptr & ap,
	  min::gen flags,
	  unsigned option,
	  const min::flag_parser * flag_parser,
	  min::packed_vec_ptr<min::ustring> flag_names )
{
    min::obj_vec_ptr fvp ( flags );
    min::attr_ptr fap ( fvp );
    min::locate ( fap, min::dot_position );
    min::phrase_position_vec pos = min::get ( fap );

    if (    option == PAR::NEW
         || option == PAR::NEW_OR_SAME )
    {
        min::attr_info info;
	min::attr_info_of ( info, ap, false );
	if ( info.flag_count > 0 )
	{
	    fvp = min::NULL_STUB;
	    if ( option == PAR::NEW )
	    {
		parse_error
		    ( parser, pos->position,
		      "",
		      min::pgen_quote ( info.name ),
		      " already has flag(s);"
		      " old flag(s) not changed" );
		return false;
	    }
	    else if ( test_attr_flags
	                  ( parser, ap, flags,
			    flag_parser,
			    flag_names ) )
	        return true;
	    else
	    {
		parse_error
		    ( parser, pos->position,
		      "",
		      min::pgen_quote ( info.name ),
		      " already has different flag(s);"
		      " old flag(s) not changed" );
		return false;
	    }
	}
    }

    for ( min::unsptr i = 0;
	  i < min::size_of ( fvp ); ++ i )
    {
	min::gen flags_text = fvp[i];
	if ( min::is_obj ( flags_text ) )
	{
	    min::obj_vec_ptr tvp ( flags_text );
	    min::attr_ptr tap ( tvp );
	    min::locate ( tap, min::dot_type );
	    min::gen type = get ( tap );
	    if ( type == min::doublequote
		 ||
		 type == min::number_sign )
		flags_text = tvp[0];
	}

	if ( min::is_str ( flags_text ) )
	{
	    min::str_ptr sp ( flags_text );
	    min::unsptr tlen = min::strlen ( sp );
	    char text_buffer[tlen+1];
	    min::strcpy ( text_buffer, sp );
	    min::uns32 flags[tlen];
	    min::uns32 flen = min::parse_flags
		( flags, text_buffer, flag_parser );
	    MIN_REQUIRE ( flen <= tlen );
	    for ( min::unsptr j = 0; j < flen; ++ j )
		min::set_flag ( ap, flags[j] );

	    if ( text_buffer[0] != 0 )
	    {
		char buffer[tlen+200];
		sprintf ( buffer,
			  "bad flag(s) `%s' in ",
			  text_buffer );
		parse_error ( parser, pos[i],
			      buffer,
			      min::pgen_quote
				  ( flags_text ),
			      "; bad flag(s) ignored" );
	    }
	}
	else
	{
	    PAR::parse_error
		( parser, pos[i],
		  "bad flags specifier `",
		   min::pgen_quote ( flags_text ),
		   "'; ignored" );
	}
    }

    return true;
}

// Compute flag name in buffer.
//
static void sflag_name
	( char buffer[100], min::uns32 flag, 
	  min::packed_vec_ptr<min::ustring>
	      flag_names )
{
    if ( flag < flag_names->length )
    {
	min::ustring s = flag_names[flag];
	MIN_ASSERT
	    ( min::ustring_length ( s ) < 100,
	      "flag name too long" );
	strcpy ( buffer, min::ustring_chars ( s ) );
    }
    else
	sprintf ( buffer, "%d", flag );
}

bool PAR::test_attr_flags
	( PAR::parser parser,
	  min::attr_insptr & ap,
	  min::gen flags,
	  const min::flag_parser * flag_parser,
	  min::packed_vec_ptr<min::ustring> flag_names,
	  min::unsptr n )
{
    bool result = true;
    min::gen zero_cc = min::new_control_code_gen ( 0 );

    min::gen f[n], d[n];
    min::unsptr nf = min::get_flags ( f, n, ap );
    if ( nf > n )
        return PAR::test_attr_flags
	    ( parser, ap, flags,
	      flag_parser, flag_names, nf );

    for ( min::unsptr i = 0; i < nf; ++ i )
        d[i] = zero_cc;
	// d records bits that are set by flags so
	// duplicates can be ignored

    min::obj_vec_ptr vp ( flags );
    min::attr_ptr avp ( vp );
    min::locate ( avp, min::dot_position );
    min::phrase_position_vec pos = min::get ( avp );

    for ( min::unsptr i = 0;
	  i < min::size_of ( vp ); ++ i )
    {
	min::gen flags_text = vp[i];
	if ( min::is_obj ( flags_text ) )
	{
	    min::obj_vec_ptr fvp ( flags_text );
	    min::attr_ptr fap ( fvp );
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
	    min::uns32 fv[len];
	    min::uns32 fvlen = min::parse_flags
		    ( fv, text_buffer, flag_parser );

	    if ( text_buffer[0] != 0 )
	    {
		char buffer[len+200];
		sprintf ( buffer,
			  "bad flag(s) `%s' in ",
			  text_buffer );
		parse_error ( parser, pos[i],
			      buffer,
			      min::pgen_quote
				  ( flags_text ),
			      "; bad flag(s) ignored" );
	    }

	    for ( min::unsptr j = 0; j < fvlen; ++ j )
	    {
	        min::unsptr k = fv[j] % min::VSIZE;
		min::unsptr i = fv[j] / min::VSIZE;
		min::unsgen dcc =
		    min::control_code_of ( d[i] );
		min::unsgen fcc =
		    min::control_code_of ( f[i] );
		if ( dcc & ( min::unsgen ( 1 ) << k ) )
		    // duplicate flag; ignore
		    continue;

		dcc |= ( min::unsgen ( 1 ) << k );
		d[i] = min::new_control_code_gen
			    ( dcc );

		if ( fcc & ( min::unsgen ( 1 ) << k ) ) 
		{
		    // Flag set correctly.  Turn off
		    // in f to indicate flag has been
		    // processed.
		    //
		    fcc &= ~ ( min::unsgen ( 1 ) << k );
		    f[i] = min::new_control_code_gen
				( fcc );
		}
		else
		{
		    // Flag off for name when it should
		    // have been on.
		    //
		    result = false;
		    min::gen name = min::name_of ( ap );

		    char buffer[100];
		    sflag_name
		        ( buffer, fv[j], flag_names );
		    parse_error ( parser, pos[i],
		    		  "flag `",
				  min::pnop,
				  buffer,
				  min::pnop,
				  "' for label ",
				  min::pgen_quote
				      ( name ),
				  " should be"
				  " previously on"
				  " but is off" );
		}
	    }
	}
	else
	{
	    PAR::parse_error
		( parser, pos[i],
		  "bad flags specifier `",
		   min::pgen_quote ( flags_text ),
		   "'; ignored" );
	}
    }

    // Check for flags that are on but should be off.
    //
    for ( min::unsptr i = 0; i < nf; ++ i )
    {
        min::unsgen cc = min::control_code_of ( f[i] );
        if ( cc == 0 ) continue;

	for ( unsigned k = 0; k < min::VSIZE; ++ k )
	{
	    if (    ( cc & ( min::unsgen ( 1 ) << k ) )
	         == 0 ) continue;

	    // Flag on for name when it should have been
	    // off.
	    //
	    result = false;
	    min::gen name = min::name_of ( ap );

	    unsigned j = i * min::VSIZE + k;
	    char buffer[100];
	    sflag_name ( buffer, j, flag_names );

	    parse_error ( parser, pos->position,
			  "flag `",
			  min::pnop,
			  buffer,
			  min::pnop,
			  "' for label ",
			  min::pgen_quote ( name ),
			  " should be previously off"
			  " but is on" );
	}
    }
    return result;
}

inline void replace_value
	( min::gen value,
	  min::file file,
	  min::phrase_position const & position )
{
    min::new_obj_gen ( value, 40, 20 );
    min::locatable_var<min::phrase_position_vec_insptr>
        pos;
    min::init ( pos, file, position, 0 );
    min::obj_vec_insptr vp ( value );
    min::attr_insptr ap ( vp );
    min::locate ( ap, min::dot_position );
    min::set ( ap, min::new_stub_gen ( pos ) );
    min::set_flag ( ap, min::standard_attr_hide_flag );
}

bool PAR::set_attr_value
	( PAR::parser parser,
	  min::attr_insptr & ap,
	  min::gen value,
	  min::phrase_position const & pos,
	  unsigned option )
{
    bool is_legal = min::is_attr_legal ( value );
    min::gen reverse_name = min::reverse_name_of ( ap );
    bool is_double_arrow =
	( reverse_name != min::NONE() );
    bool is_preallocated =
        min::is_preallocated ( value );
    if ( ! is_legal
	 ||
	 ( is_double_arrow
	   &&
	   ! is_preallocated
	   &&
	   ! min::is_obj ( value ) ) )
    {
	PAR::parse_error
	    ( parser, pos,
	      ( is_legal ?  "not an object or"
			    " preallocated" :
			    "not a legal" ),
	       min::pnop,
	       " attribute value `",
	       min::pgen_never_quote ( value ),
	       "'; ignored" );
	return false;
    }

    min::gen previous_value = min::get ( ap );
    if ( previous_value == min::NONE() )
    {
	if ( is_double_arrow && is_preallocated )
	    ::replace_value
	        ( value, parser->input_file, pos );
        min::set ( ap, value );
	return true;
    }
    if ( option == NEW )
    {
	min::gen name = min::name_of ( ap );
        parse_error ( parser, pos,
		      "",
		      min::pgen_quote ( name ),
		      is_double_arrow ?
		          " <=> " : "",
		      is_double_arrow ?
		          min::pgen_quote
			      ( reverse_name ) :
			  min::pnop,
		      " already has value(s);"
		      " old value(s) not changed" );
	return false;
    }
    if ( previous_value == value
         &&
	 option != ADD_TO_MULTISET )
        return true;
    else if ( option == NEW_OR_SAME )
    {
	min::gen name = min::name_of ( ap );
        parse_error ( parser, pos,
		      "",
		      min::pgen_quote ( name ),
		      is_double_arrow ?
		          " <=> " : "",
		      is_double_arrow ?
		          min::pgen_quote
			      ( reverse_name ) :
			  min::pnop,
		      " already has different value(s);"
		      " old value(s) not changed" );
	return false;
    }
    if ( is_double_arrow && is_preallocated )
	::replace_value
	    ( value, parser->input_file, pos );
    if ( option == ADD_TO_SET )
	min::add_to_set ( ap, value );
    else
	min::add_to_multiset ( ap, value );
    return true;
}

// Test whether value set pointed at by ap equals
// element set of vp.  Allows duplicates in both `sets';
// that is, the number of copies of a value must be
// the same in both sets.  NOT optimized for large sets.
//
static bool same_multivalue
    ( min::attr_insptr & ap, min::obj_vec_ptr & vp )
{
    min::unsptr nvp = min::size_of ( vp );
    min::gen values[nvp];
    min::unsptr nap = min::get ( values, nvp, ap );
    if ( nap != nvp ) return false;

    bool apfound[nap];
    for ( min::unsptr i = 0; i < nap; ++ i )
        apfound[i] = false;

    for ( min::unsptr i = 0; i < nvp; ++ i )
    {
        min::unsptr j = 0;
        for ( ; j < nap; ++ j )
	{
	    if ( values[j] == vp[i]
	         &&
		 ! apfound[j] )
	    {
	        apfound[j] = true;
		break;
	    }
	}
	if ( j == nap ) return false;
    }

    return true;
}

bool PAR::set_attr_multivalue
	( PAR::parser parser,
	  min::attr_insptr & ap,
	  min::gen multivalue,
	  unsigned option )
{
    min::obj_vec_ptr vp ( multivalue );
    min::attr_ptr vap ( vp );
    min::locate ( vap, min::dot_position );
    min::phrase_position_vec pos = min::get ( vap );

    switch ( option )
    {
    case NEW:
    case NEW_OR_SAME:
	if ( min::get ( ap ) != min::NONE() )
	{
	    if ( option == NEW_OR_SAME
	         &&
		 same_multivalue ( ap, vp ) )
	        return true;

	    min::gen name = min::name_of ( ap );
	    min::gen reverse_name =
	        min::reverse_name_of ( ap );
	    bool is_double_arrow =
		( reverse_name != min::NONE() );
	    parse_error ( parser, pos->position,
			  "",
			  min::pgen_quote ( name ),
			  is_double_arrow ?
			      " <=> " : "",
			  is_double_arrow ?
			      min::pgen_quote
				  ( reverse_name ) :
			      min::pnop,
			  ( option == NEW ?
			    " already has value(s);"
			    " old value(s) not"
			    " changed" :
			    " already has different"
			    " value(s); old value(s)"
			    " not changed" ) );
	    return false;
	}
	option = ADD_TO_MULTISET;
	break;
    }

    min::unsptr n = min::size_of ( vp );
    bool result = true;
    for ( min::unsptr i = 0; i < n; ++ i )
    {
        if ( ! PAR::set_attr_value
	           ( parser, ap, vp[i],
		     pos[i], option ) )
	    result = false;
    }
    return result;
}

inline void set_attributes
	( PAR::parser parser,
	  min::attr_insptr & ap,
	  const min::phrase_position & position,
	  PAR::attr * attributes,
	  min::uns32 m )
{
    while ( m -- )
    {
	min::locate ( ap, attributes->name );
	if ( attributes->flags != min::MISSING() )
	    PAR::set_attr_flags
		( parser, ap,
		  attributes->flags );

	unsigned option = PAR::NEW;
	if (    attributes->reverse_name
	     != min::MISSING() )
	{
	    min::locate_reverse
		( ap, attributes->reverse_name );
	    option = PAR::ADD_TO_SET;
	}

	if (    attributes->value
	     != min::NONE() )
	    PAR::set_attr_value
	        ( parser, ap, attributes->value,
		  attributes->value_pos.begin ?
		  attributes->value_pos :
		  position,
		  option );
	else if (    attributes->multivalue
	          != min::MISSING() )
	    PAR::set_attr_multivalue
		( parser, ap,
		  attributes->multivalue,
		  option );

	++ attributes;
    }
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

    if ( first->next == next && m == 0 )
    {
	if ( ( 1 << first->type )
	     & LEXSTD::convert_mask )
	    PAR::convert_token ( first );
        first->position = position;
    }
    else if ( first->next == next
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

	set_attributes
	    ( parser, ap, position, attributes, m );

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
	    if ( ( 1 << current->type )
	         & LEXSTD::convert_mask )
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

	set_attributes
	    ( parser, expap, position, attributes, m );

	first = PAR::new_token ( type );
	PAR::put_before
	    ( first_ref(parser), next, first );

	PAR::value_ref(first) = exp;
	first->position = position;
    }

    PAR::trace_subexpression
        ( parser, first, trace_flags );
}

void PAR::internal::trace_subexpression
	( PAR::parser parser,
	  PAR::token token,
	  min::uns32 trace_flags )
{
    parser->printer
	<< min::bol << min::save_indent
	<< min::adjust_indent ( 4 );
    if ( token->type <= LEXSTD::MAX_TYPE )
        parser->printer
	     << LEXSTD::type_names[token->type]
	     << " LEXEME: ";
    else
        parser->printer
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
    min::gen s = PAR::scan_quoted_string
        ( vp, i, parser );
    if ( s == min::MISSING() )
        return min::MISSING();
    min::str_ptr sp = s;

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
	    PAR::lexical_type_of ( vp[i] );
	if ( ( ( 1ull << t ) & accepted_types )
	     &&
	     vp[i] != end_value )
	    ++ i;
	else
	    break;
	accepted_types |= 1ull << LEXSTD::natural_t;
    }

    if ( i == j ) return min::NONE();
    else if ( i == j + 1 ) return vp[j];

    min::gen elements[i-j];
    memcpy ( elements, & vp[j], sizeof ( elements ) );
    return min::new_lab_gen ( elements, i - j );
}

// If v is a quoted string return its string value.
// otherwise return min::NONE().
//
inline min::gen quoted_string_value ( min::gen v )
{
    min::obj_vec_ptr vp ( v );
    if ( min::size_of ( vp ) != 1 ) return min::NONE();
    min::attr_ptr ap ( vp );
    min::locate ( ap, min::dot_type );
    min::gen type = min::get ( ap );
    if ( type != PARLEX::doublequote )
        return min::NONE();
    else
        return vp[0];
}

min::gen PAR::scan_label_or_value
	( PAR::parser parser,
	  min::obj_vec_ptr & vp, min::uns32 & i,
	  PAR::scan_mode mode, min::gen end_value )
{
    MIN_REQUIRE ( mode != PAR::DATA_MODE );
    min::uns32 s = min::size_of ( vp );
    min::uns64 accepted_types =
	( mode == PAR::LABEL_MODE ?
	  PAR::LABEL_HEADER_MASK :
	  PAR::VALUE_COMPONENT_MASK );
    min::gen elements[s];
    min::uns32 j = 0;
    bool errors_found = false;
    min::gen v;
    min::uns32 initial_i = i;
    while ( i < s )
    {
	if ( vp[i] == end_value )
	    break;
	if ( min::is_obj ( vp[i] ) )
	{
	    v = ::quoted_string_value ( vp[i] );
	    if ( v == min::NONE() )
	        break;
	    else
	    if ( (   accepted_types
		   & PAR::QUOTED_STRING_MASK ) == 0 )
		v = min::NONE();
	}
	else
	{
	    min::uns32 t =
		PAR::lexical_type_of ( vp[i] );
	    if ( ( 1ull << t ) & accepted_types )
	        v = vp[i];
	    else
	        v = min::NONE();
	}

	if ( v == min::NONE() )
	{
	    min::phrase_position_vec posvec =
		min::position_of ( vp );
	    PAR::parse_error
	        ( parser, posvec[i],
		  mode == PAR::VALUE_MODE ?
		  "not a legal value element `" :
		  i == initial_i ?
		  "not a legal label initial"
		  " element `" :
		  "not a legal label element `",
		  min::pgen_never_quote ( vp[i] ),
		  "'" );
	    errors_found = true;
	}
	else
	    elements[j++] = v;
	++ i;
	if ( mode == PAR::LABEL_MODE )
	    accepted_types = PAR::LABEL_COMPONENT_MASK;
    }

    if ( errors_found ) return min::NONE();
    else if ( j == 0 )
    {
	min::phrase_position_vec posvec =
	    min::position_of ( vp );
	PAR::parse_error
	    ( parser, posvec[i],
	      mode == PAR::LABEL_MODE ?
		  "empty label after" :
		  "empty value after" );
	return min::NONE();
    }
    else if ( j == 1 ) return elements[0];
    return min::new_lab_gen ( elements, j );
}

bool PAR::make_label_or_value
    ( PAR::parser parser,
      PAR::token & first,
      PAR::token next,
      PAR::scan_mode mode )
{
    min::unsptr count = 0;
    min::phrase_position position =
        { first->position.begin,
	  first->position.begin };
        // For the empty label case first == next so
	// this is the position just before next.
    min::uns64 accepted_types =
	( mode == PAR::LABEL_MODE ?
	  PAR::LABEL_HEADER_MASK :
	  PAR::VALUE_COMPONENT_MASK );

    for ( PAR::token t = first; t != next;
          ++ count, t = t->next )
    {
	position.end = t->position.end;

	if ( mode == PAR::DATA_MODE )
	{
	    min::gen v = t->value;
	    if ( ! min::is_str ( v )
	         &&
		 ! min::is_num ( v )
		 &&
		 ! min::is_lab ( v ) )
	    {
		t->type = PAR::DERIVED;
		value_ref(t) = min::NONE();
	    }
	}
	else
	if (    ( ( 1ull << t->type ) & accepted_types )
	     == 0 )
	{
	    t->type = PAR::DERIVED;
	    value_ref(t) = min::NONE();
	}

	if ( mode == PAR::LABEL_MODE )
	    accepted_types = PAR::LABEL_COMPONENT_MASK;
    }

    if ( count == 0 )
    {
	first = PAR::new_token ( PAR::DERIVED );
	first->position = position;
	put_before ( PAR::first_ref(parser),
	             next, first );
	PAR::value_ref(first) = min::empty_lab;
	if ( mode == PAR::DATA_MODE )
	    return true;
	else
	{
	    PAR::parse_error
		( parser, position,
		  mode == PAR::LABEL_MODE ?
		      "empty label" :
		      "empty value" );
	    PAR::value_ref(first) = min::NONE();
	    return false;
	}
    }

    min::gen vec[count];
    min::unsptr i = 0;
    bool error_found = false;
    for ( PAR::token t = first; t != next; t = t->next )
    {
	if ( t->value != min::NONE() )
	    vec[i++] = t->value;
	else
	{
	    PAR::parse_error
	        ( parser, t->position,
		  mode == PAR::VALUE_MODE ?
		  "not a legal value element `" :
		  t == first ?
		  "not a legal label initial"
		  " element `" :
		  "not a legal label element `",
		  min::pgen_never_quote ( t->value ),
		  "'; ignored" );
	    error_found = true;
	}
    }

    if ( i == 0 )
	PAR::value_ref(first) = min::empty_lab;
    else if ( i == 1 )
	PAR::value_ref(first) = vec[0];
    else
	PAR::value_ref(first) =
	    min::new_lab_gen ( vec, i );

    // Don't deallocate tokens until their values
    // have been put in the gc protected first->value.
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
    return ! error_found;
}

min::gen PAR::scan_value
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  min::gen end_value )
{
    min::uns32 s = min::size_of ( vp );

    // For values, handle case where value is an
    // a single expression (or number).
    //
    if ( ( i + 1 == s
	   ||
	   ( i + 1 < s
	     &&
	     vp[i+1] == end_value ) )
	 &&
	 ! min::is_str ( vp[i] ) )
	       // This excludes marks and separators.
    {
        min::gen v = quoted_string_value ( vp[i] );
	return ( v == min::NONE() ? vp[i] : v );
    }

    min::uns32 initial_i = i;
    min::gen elements[s];
    min::uns32 j = 0;

    while ( i < s )
    {
	if ( vp[i] == end_value )
	    break;
	min::gen v = ::quoted_string_value ( vp[i] );
	if ( v != min::NONE() )
	{
	    if ( (   PAR::VALUE_COMPONENT_MASK
	           & PAR::QUOTED_STRING_MASK ) == 0 )
	        break;
	    elements[j++] = v;
	}
	else
	{
	    min::uns32 t =
		PAR::lexical_type_of ( vp[i] );
	    if (   ( 1ull << t )
	         & PAR::VALUE_COMPONENT_MASK )
	        elements[j++] = vp[i];
	    else
	        break;
	}
	++ i;
    }

    if ( i == initial_i ) return min::MISSING();
    else if ( i == initial_i + 1 ) return elements[0];
    return min::new_lab_gen ( elements, j );
}

min::gen PAR::scan_label
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  min::gen end_value )
{
    min::uns32 initial_i = i;
    min::uns32 s = min::size_of ( vp );
    min::uns64 accepted_types = PAR::LABEL_HEADER_MASK;
    min::gen elements[s];
    min::uns32 j = 0;

    while ( i < s )
    {
	if ( vp[i] == end_value )
	    break;
	min::gen v = ::quoted_string_value ( vp[i] );
	if ( v != min::NONE() )
	{
	    if ( (   accepted_types
	           & PAR::QUOTED_STRING_MASK ) == 0 )
	        break;
	    elements[j++] = v;
	}
	else
	{
	    min::uns32 t =
		PAR::lexical_type_of ( vp[i] );
	    if ( ( 1ull << t ) & accepted_types )
	        elements[j++] = vp[i];
	    else
	        break;
	}
	++ i;
	accepted_types = PAR::LABEL_COMPONENT_MASK;
    }

    if ( i == initial_i ) return min::MISSING();
    else if ( i == initial_i + 1 ) return elements[0];
    return min::new_lab_gen ( elements, j );
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

    min::uns32 original_i = i;
    result = PAR::scan_simple_name ( vp, i, end_value );

    if ( result != min::NONE() ) return result;

    min::attr_ptr ap ( vp );
    min::locate ( ap, min::dot_position );
    min::phrase_position_vec ppvec = min::get ( ap );
    MIN_ASSERT ( ppvec != min::NULL_STUB,
                 ".position attribute missing" );
    MIN_ASSERT ( ppvec->file == parser->input_file,
                 ".position attribute value file is"
		 " not the same as parser input_file" );

    return PAR::parse_error
               ( parser, ppvec[original_i],
	         "expected quoted key or simple name"
		 " at" );
}

void PAR::convert_token ( PAR::token token )
{
    min::gen type;
    min::locatable_gen value ( token->value );

    if ( token->type == LEXSTD::quoted_string_t )
	type = PARLEX::doublequote;
    else
    {
	MIN_REQUIRE
	    ( token->type == LEXSTD::numeric_t );
	type = PARLEX::number_sign;
    }

    PAR::value_ref(token) = min::new_obj_gen ( 10, 1 );
    min::obj_vec_insptr elemvp ( token->value );
    min::attr_push(elemvp) = min::MISSING();

    min::attr ( elemvp, 0 ) = value;

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
