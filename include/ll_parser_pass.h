// Layers Language Parser Pass
//
// File:	ll_parser_pass.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Jan 12 15:28:58 EST 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Strings
//	Tokens
//	Passes

// Usage and Setup
// ----- --- -----

# ifndef LL_PARSER_PASS_H
# define LL_PARSER_PASS_H

# include <min.h>
# include <ll_lexeme.h>

namespace ll { namespace parser {

using min::uns32;
using min::NULL_STUB;

} }


// Strings
// -------

namespace ll { namespace parser {

// Token character strings are optional parts of tokens.
// A token character string is allocated when its token
// is allocated and freed when its token is freed.  As
// the lifetime of a token character string may be
// short, freed token character strings are put on a
// special free list.  These token character strings
// start out being long enought to hold a typical maxi-
// mum length line (80 characters) and may grow if
// necessary to hold longer character strings.
//
// Some token character strings are kept a long time
// before they are freed.  These are resized to optimize
// memory usage.

struct string_struct;
typedef min::packed_vec_ptr<uns32,string_struct>
        string_ptr;
typedef min::packed_vec_insptr<uns32,string_struct>
        string_insptr;
struct string_struct
{
    uns32 control;
    	// Packed vector control word.
    uns32 length;
        // Length of vector.
    uns32 max_length;
        // Maximum length of vector.

    string_insptr next;
        // Pointer to next block on free list, if string
	// is on free list.  List is NULL_STUB termina-
	// ted.

    // The elements of a string are uns32 UNICODE
    // characters.
};

// Allocate a new string and return a pointer to it.
//
string_ptr new_string ( uns32 n, const uns32 * string );

// Free a string and return NULL_STUB.
//
string_ptr free_string ( string_ptr sp );

// Set the maximum number of strings on the free list.
// Set to 0 to make list empty.  Set to < 0 if there is
// no limit.  Defaults to 100.
//
void set_max_string_free_list_size ( int n );

} }

// Tokens
// ------

namespace ll { namespace parser {

struct token_struct;
typedef min::packed_struct_updptr<token_struct>
    token_ptr;
enum // Token types (see below).
{
    SYMBOL		= 0xFFFFFFFF,
    NATURAL_NUMBER	= 0xFFFFFFFE,
    LABEL		= 0xFFFFFFFD,
    EXPRESSION		= 0xFFFFFFFC
};
struct token_struct
{
    uns32 control;
    	// Packed structure control word.

    uns32 type;  // One of:
	//
        // For lexemes: the lexeme type.
	//
	// For names:
	//
    	//	SYMBOL
	//	NATURAL_NUMBER
	//	LABEL
	//
	// For expressions:
	//
    	//	EXPRESSION

    min::gen value;
        // Value for names and expressions.

    string_ptr string;
        // Character string for lexemes.

    ll::lexeme::position begin, end;
        // Position of the first character of the token.
        // and of the first character AFTER the token,
	// or the end of input.

    token_ptr next, previous;
        // Doubly linked list pointers for tokens.
};

// Allocate a new token of the given type.  Value is set
// to min:MISSING and string to NULL_STUB.
//
token_ptr new_token ( uns32 type );

// Free token.  Token is put on internal free list after
// its value is set to min:MISSING and its string to
// NULL_STUB.
//
void free_token ( token_ptr token );

// Set the maximum number of tokens on the free list.
// Set to 0 to make list empty.  Set to < 0 if there is
// no limit.  Defaults to 1000.
//
void set_max_token_free_list_size ( int n );

// Put a token just before a given token t on a list of
// tokens.
//
inline void put_before ( token_ptr t, token_ptr token )
{
    token->next = t;
    token->previous = t->previous;
    token->previous->next = token;
    token->next->previous = token;
}

// Put a token on the end of a token list with given
// first element.
//
inline void put_at_end
	( token_ptr & first, token_ptr token )
{
    if ( first == min::NULL_STUB )
    {
        first = token;
	token->previous = token->next = token;
    }
    else put_before ( first, token );
}

// Remove token from the token list with given first
// token and return the token removed.
//
inline token_ptr remove
	( token_ptr & first, token_ptr token )
{

    if ( token == first )
    {
        first = token->next;
	if ( first == token )
	{
	    first = min::NULL_STUB;
	    return token;
	}
    }
    token->previous->next = token->next;
    token->next->previous = token->previous;
    return token;
}

// Remove first token from a list of tokens with given
// first token.  Return min::NULL_STUB if list empty.
//
inline token_ptr remove ( token_ptr & first )
{
    if ( first == min::NULL_STUB )
        return min::NULL_STUB;
    else
    	remove ( first, first );
}

} }


// Passes
// ------

namespace ll { namespace parser {

// The pass struct is the base for packed structs that
// are specific kinds of passes.
//
struct pass_struct;
typedef min::packed_struct_updptr<pass_struct> pass_ptr;
struct pass_struct
{
    uns32 control;
    	// Packed structure control word.

    pass_ptr in;
        // Pass from which this pass gets its input
	// tokens.  NULL_STUB if none.

    token_ptr first;
        // First token in pass.  The pass tokens are a
	// doubly linked list.  NULL_STUB if this list
	// is empty.

    bool eop;
        // True if pass is at the end and no more tokens
	// can be obtained from the pass.

    uns32 (*get) ( pass_ptr out, pass_ptr in);
        // Function to call with this pass as the `in'
	// argument to get tokens from this pass and
	// copy them to the end the `out' pass token
	// list.  (We cannot use virtual functions in a
	// min::packed_struct.)  Returns the number of
	// tokens gotten.  Retuns 0 if at end of `in'
	// pass (in->eop must be set).

    std::ostream * trace;
        // If not NULL, the `get' function should output
	// trace messages listing the tokens it outputs
	// and other appropriate information to this
	// ostream.

    std::ostream * err;
        // If not NULL, output error messages to this
	// ostream.  Note that `trace' takes precedence
	// over `err' for printing error messages.
};

// Get tokens from `pass->in' and put them on the end of
// the `pass' token list.  Returns the number of tokens
// gotten.  Returns 0 only if `pass->in->eop' is set
// (indicating `end of pass').
//
inline uns32 get ( pass_ptr pass )
{
    return pass->in->get ( pass, pass->in );
}

// Default error and trace streams for pass creation.
// Default_err defaults to & std::cerr while default_
// trace defaults to NULL.
//
extern std::ostream * default_err;
extern std::ostream * default_trace;

// Create minimal pass that is useful as an output pass
// for testing purposes.  Tokens CANNOT be gotten FROM
// this pass, but may be gotten INTO this pass.  The
// input pass for this pass is given (and if in->trace
// is set will trace the tokens gotten into this pass).
//
pass_ptr create_output_pass ( pass_ptr in );

// Note: strings and tokens are explicitly deallocated,
// but this does not mean that their stubs will be
// garbage collected.  Passes must be protected against
// garbage collection by storing pass_ptrs in static/
// stack_stub locations.  Tokens are protected iff they
// are part of some protected pass.   Token character
// strings are protected iff they are part of a
// protected token.


} }

# endif // LL_PARSER_PASS_H
