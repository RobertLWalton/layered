// Layers Language Parser Pass Functions
//
// File:	ll__parser_pass.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Jan 11 06:56:04 EST 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Lines beginning with `    /// ' are the same as in
// the Standard Lexical Program appendix of ../doc/
// layered-introduction.tex.  As a consequence, this
// file uses 80 columns instead of the system default 56
// columns.

// Table of Contents
//
//	Usage and Setup
//	Strings
//	Tokens
//	Passes

// Usage and Setup
// ----- --- -----

# include <ll_parser_pass.h>
# define PAR ll::parser

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

PAR::token_ptr PAR::new_token ( min::uns32 kind )
{
    token_ptr token = remove ( ::free_tokens );
    if ( token == min::NULL_STUB )
        token = ::token_type.new_stub();
    else
        -- ::number_free_tokens;
    token->value = min::MISSING;
    token->string = min::NULL_STUB;
    token->kind = kind;
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

// Passes
// ------

std::ostream * PAR::default_err = & std::cerr;
std::ostream * PAR::default_trace = NULL;
