// Layers Language Parser Pass Functions
//
// File:	ll__parser.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Jan 26 06:53:50 EST 2011
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

// Usage and Setup
// ----- --- -----

# include <ll_parser.h>
# include <ll_parser_input.h>
# define LEX ll::lexeme
# define PAR ll::parser
# define TAB ll::parser::table

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
static min::static_stub<1> string_vec;
static PAR::string_insptr & free_strings =
    * ( PAR::string_insptr *) & string_vec[0];

static int number_free_strings = 0;

static int max_string_free_list_size = 100;

static int min_string_length = 80;

PAR::string_ptr PAR::new_string
	( min::uns32 n, const min::uns32 * string )
{
    string_insptr str = ::free_strings;
    if ( str == min::NULL_STUB )
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
	if ( str->length < n )
	    min::resize ( str, n );
	min::pop ( str, str->length );
    }
    str->next = min::NULL_STUB;
    min::push ( str, n, string );
    return (string_ptr) str;
}

PAR::string_ptr PAR::free_string ( string_ptr string )
{
    if ( ::max_string_free_list_size >= 0
         &&
            ::number_free_strings
	 >= ::max_string_free_list_size )
    {
        min::deallocate ( string );
	return min::NULL_STUB;
    }

    string_insptr str = (string_insptr) string;
    str->next = ::free_strings;
    ::free_strings = str;
    ++ ::number_free_strings;
    return min::NULL_STUB;
}

void PAR::set_max_string_free_list_size ( int n )
{
    ::max_string_free_list_size = n;
    if ( n >= 0 ) while ( ::number_free_strings > n )
    {
	string_insptr string = ::free_strings;
	::free_strings = string->next;
        min::deallocate ( string );
	-- ::number_free_strings;
    }
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

// Free list of tokens.
//
static min::static_stub<1> token_vec;
static PAR::token_ptr & free_tokens =
    * ( PAR::token_ptr *) & token_vec[0];

static int number_free_tokens = 0;

static int max_token_free_list_size = 1000;

PAR::token_ptr PAR::new_token ( min::uns32 type )
{
    token_ptr token = remove ( ::free_tokens );
    if ( token == min::NULL_STUB )
        token = ::token_type.new_stub();
    else
        -- ::number_free_tokens;
    token->value = min::MISSING;
    token->string = min::NULL_STUB;
    token->type = type;
    return token;
}

void PAR::free_token ( token_ptr token )
{
    if ( ::max_token_free_list_size >= 0
         &&
            ::number_free_tokens
	 >= ::max_token_free_list_size )
    {
        min::deallocate ( token );
	return;
    }

    token->value = min::MISSING;
    token->string = free_string ( token->string );
    put_at_end ( ::free_tokens, token );
    ++ ::number_free_tokens;
}

void PAR::set_max_token_free_list_size ( int n )
{
    ::max_token_free_list_size = n;
    if ( n >= 0 ) while ( ::number_free_tokens > n )
    {
        min::deallocate ( remove ( ::free_tokens ) );
	-- ::number_free_tokens;
    }
}

// Parser Closures
// ------ --------

static min::packed_struct<PAR::input_struct>
    input_type ( "ll::parser::input_type" );

void PAR::init_input
	( uns32 (*add_tokens)
	      ( parser_ptr parser, input_ptr input ),
	  void (*init)
	      ( parser_ptr parser, input_ptr input ),
	  input_ptr & input )
{
    if ( input == NULL_STUB )
        input = ::input_type.new_stub();

    input->add_tokens = add_tokens;
    input->init = init;
}

// Parser
// ------

static min::uns32 parser_stub_disp[] =
{
    min::DISP ( & PAR::parser_struct::input ),
    min::DISP ( & PAR::parser_struct::output ),
    min::DISP ( & PAR::parser_struct::pass_stack ),
    min::DISP ( & PAR::parser_struct::scanner ),
    min::DISP ( & PAR::parser_struct::input_file ),
    min::DISP ( & PAR::parser_struct::print ),
    min::DISP ( & PAR::parser_struct::hash_table ),
    min::DISP ( & PAR::parser_struct
                     ::indentation_mark_table ),
    min::DISP ( & PAR::parser_struct::first ),
    min::DISP_END
};

static min::packed_struct<PAR::parser_struct>
    parser_type ( "ll::parser::parser_type",
                  NULL, ::parser_stub_disp );

static min::static_stub<1> default_stub;
PAR::parser_ptr & PAR::default_parser =
    * (PAR::parser_ptr *) & default_stub[0];

void PAR::init_parser ( parser_ptr & parser )
{
    if ( parser == NULL_STUB )
    {
        parser = ::parser_type.new_stub();
	init_standard_input ( parser );
    }
    else if ( parser->scanner != NULL_STUB )
        LEX::init_scanner ( parser->scanner );

    parser->first = NULL_STUB;
    parser->eof = false;
    parser->finished_tokens = 0;
}

bool PAR::init_parser
	( const char * file_name,
	  parser_ptr & parser )
{
    init_parser ( parser );
    return LEX::init_file ( file_name,
                            parser->error_message,
		            parser->input_file );
}

void PAR::init_parser
	( std::istream & istream,
	  const char * file_name,
	  uns32 spool_length,
	  parser_ptr & parser )
{
    init_parser ( parser );
    LEX::init_file ( istream, file_name, spool_length,
                     parser->input_file );
}

void PAR::init_parser
	( const char * file_name,
	  const char * data,
	  parser_ptr & parser )
{
    init_parser ( parser );
    LEX::init_file ( file_name, data,
                     parser->input_file );
}

void PAR::parse ( parser_ptr parser )
{
    while ( ! parser->eof )
        parser->input->add_tokens
	    ( parser, parser->input );
}

TAB::key_prefix_ptr PAR::find
	( parser_ptr parser,
	  token_ptr first, token_ptr end,
	  TAB::table_ptr table )
{
    uns32 hash;
    uns32 table_len = table->length;
    uns32 mask = table_len - 1;
    MIN_ASSERT ( ( table_len & mask ) == 0 );
    TAB::key_prefix_ptr previous = NULL_STUB;
    while ( true )
    {
        if ( first->type != SYMBOL )
	    return previous;

	min::gen e = first->value;
	uns32 ehash;
	if ( min::is_str ( e ) )
	    ehash = min::strhash ( e );
	else if ( min::is_num ( e ) )
	{
	    int v = min::int_of ( e );
	    MIN_ASSERT ( 0 <= v && v < (1<<28) );
	    ehash = min::numhash ( e );
	}
	else
	    MIN_ABORT ( "bad key element type" );

	// Compute hash of this element's key prefix.
	//
	if ( previous == NULL_STUB )
	    hash = ehash;
	else
	    hash = min::labhash ( hash, ehash );

	// Locate key prefix.
	//
	TAB::key_prefix_ptr kprefix =
	    table[hash & mask];
	while ( kprefix != NULL_STUB )
	{
	    if ( kprefix->key_element == e
	         &&
		 kprefix->previous == previous )
	        break;
	    kprefix = kprefix->next;
	}
	if ( kprefix == NULL_STUB )
	    return previous;

	if ( previous == NULL_STUB )
	    hash = min::labhash ( 1009, hash );

	previous = kprefix;

        if ( first->next == parser->first )
	{
	    if ( end == NULL_STUB )
	    {
	        if ( parser->eof )
		    break;
		parser->input->add_tokens
		    ( parser, parser->input);
		first = first->next;
		if ( first == parser->first )
		    break;
	    }
	}
	else
	{
	    first = first->next;
	    if ( first == end ) break;
	}
    }
    return previous;
}
