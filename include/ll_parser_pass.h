// Layers Language Parser Pass
//
// File:	ll_parser_pass.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Dec 20 13:56:47 EST 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Parser Tokens
//	Parser Passes

// Usage and Setup
// ----- --- -----

# ifndef LL_PARSER_PASS_H
# define LL_PARSER_PASS_H

# include <min.h>

// Parser Tokens
// ------ ------

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
typedef min::packed_vec_ptr<string_struct,min::uns32>
        string_ptr;
struct string_struct
{
    min::uns32 type;
    	// Packed vector type.
    min::uns32 length;
        // Length of vector.
    min::uns32 max_length;
        // Maximum length of vector.

    string_ptr next;
        // Pointer to next block on free list, if string
	// is on free list.  List is NULL_STUB termina-
	// ted.

    // The elements of a string are min::uns32 UNICODE
    // characters.
};

// Allocate a new string and return a pointer to it.
//
string_ptr new_string
	( min::uns32 n, min::uns32 * string );

// Free a string and return NULL_STUB.
//
string_ptr free_string ( string_ptr sp );

struct token_struct;
typedef min::packed_struct_ptr<token_struct> token_ptr;
struct token_struct
{
    min::uns32 type;
    	// Packed structure type.

    min::uns32 kind;  // One of:
    enum
    {
        // For lexemes: the lexeme type.

	// For names:
	//
    	SYMBOL		= 0xFFFFFFFF,
	NATURAL_NUMBER	= 0xFFFFFFFE,
	LABEL		= 0xFFFFFFFD,

	// For expressions:
	//
    	EXPRESSION	= 0xFFFFFFFC
    };

    min::gen value;
        // Value for names and expressions.

    string_ptr string;
        // Character string for lexemes.

    min::uns32 begin_line;
    min::uns32 begin_index;
    min::uns32 begin_column;
        // Position of the first character of the token.

    min::uns32 end_line;
    min::uns32 end_index;
    min::uns32 end_column;
        // Position of the first character AFTER the
	// token, or the end of input.

    token_ptr next, previous;
        // Doubly linked list pointers for tokens.
};

// Allocate a new token of the given kind.  Value is set
// to min:MISSING and string to NULL_STUB.
//
token_ptr new_token ( min::uns32 kind );

// Free token.  Token is put on internal free list after
// its value is set to min:MISSING and its string to
// NULL_STUB.
//
void free_token ( token_ptr token );

} }


// Parser Passes
// ------ ------

namespace ll { namespace parser {

// The pass struct is the base for packed structs that
// are specific kinds of passes.
//
struct pass_struct;
typedef min::packed_struct_ptr<pass_struct> pass_ptr;
struct pass_struct
{
    min::uns32 type;
    	// Packed structure type.

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

    min::uns32 (*get) ( pass_ptr out, pass_ptr in);
        // Function to call with this pass as the `in'
	// argument to get tokens from this pass and
	// copy them to the end the `out' pass token
	// list.  (We cannot use virtual functions in a
	// min::packed_struct.)  Returns the number of
	// tokens gotten.  Retuns 0 if at end of `in'
	// pass.
};

inline min::uns32 get ( pass_ptr out, pass_ptr in)
{
    return in->get ( out, in );
}

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
