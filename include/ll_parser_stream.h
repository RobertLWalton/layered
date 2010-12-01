// Layers Language Parser Pass
//
// File:	ll_parser_pass.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Dec  1 03:11:24 EST 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	LL Parser Tokens
//	LL Parser Streams

// Usage and Setup
// ----- --- -----

# ifndef LL_PARSER_PASS_H
# define LL_PARSER_PASS_H

# include <min.h>

// LL Parser Tokens
// -- ------ ------

namespace ll { namespace parser {

// Token character strings are optional parts of tokens.
// A token character string is allocated when its token
// is allocated and freed when its token is freed.
// As token allocation/freeing has a FIFO aspect, so
// does token character string allocating/freeing.
//
// Small enough token character strings are allocated
// to `token circular string memory' which is composed
// of string_blocks.  There is one pointer into this
// memory that points at the next location at which a
// string is to be allocated.  If there is not enough
// room, a new string block is allocated.  The string
// blocks in the token circular string memory are all
// the same size.
//
// Large token character strings are allocated to a
// string block by themselves, and this block is freed
// when the token character string is freed.

min::uns32 large_string_length;
    // Strings of this length or larger are NOT
    // allocated to circular string memory, but instead
    // are allocated to their own private string block.

// Strings in a circular string memory consist of uns32
// values.  The first contains the length of the string,
// and the rest are the uns32 characters of the string.
// When freed the high order bit of the length uns32 is
// set.  As the length cannot be >= large_string_length,
// and this is < 2**31, there is no ambiguity.
//
const min::uns32 FREE_FLAG = 1 << 31;

struct string_block;
typedef min::packed_vec_ptr<string_block,min::uns32>
        string_block_ptr;
struct string_block
{
    min::uns32 type;
    	// Packed vector type.
    min::uns32 length;
        // Length of vector.  For string blocks,
	// always equals the max_length.
    min::uns32 max_length;
        // Maximum length of vector.

    string_block_ptr previous;
    string_block_ptr next;
        // Pointers to maintain circular list of
	// string blocks.

    // The elements of a string block are min::uns32
    // values.
};

// Pointer to a string in circular string memory.
//
struct string_pointer
{
    string_block_ptr block;
    min::uns32 offset;
        // block[offset] is the length of the string and
	// the uns32 characters of the string follow the
	// length in memory.
};

// Allocate a new string and return a pointer to it.
//
string_ptr new_string
	( min::uns32 n, uns32 * string );

// Free a string.
//
void free_string ( const string_ptr & sp );

struct token;
typedef min::packed_struct_ptr<token> token_ptr;
struct token
{
    min::uns32 type;
    	// Packed structure type.

    min::uns32 kind;  // One of:
    enum
    {
    	LEXEME		= 1,
	NAME		= 2,
	EXPRESSION	= 3
    };

    min::uns32 type;  // One of:
    enum
    {
        // For lexemes: the type.

	// For names:
	//
    	SYMBOL		= 0xFFFFFFFF,
	NATURAL_NUMBER	= 0xFFFFFFFE,
	LABEL		= 0xFFFFFFFD
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
        // Doubly linked list pointers for passes,
	// which are circular lists of tokens.
};

// The pass struct is the base for packed structs that
// are specific kinds of passes.
//
struct pass;
typedef min::packed_struct_ptr<pass> pass_ptr;
struct pass
{
    min::uns32 type;
    	// Packed structure type.

    token_ptr first;
    token_ptr_last;
        // First and last token in pass.  NULL if pass
	// empty.

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
