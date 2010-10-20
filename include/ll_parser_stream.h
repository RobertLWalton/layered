// Layers Language Parser Stream
//
// File:	ll_parser_stream.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Oct 20 00:54:58 EDT 2010
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

# ifndef LL_PARSER_STREAM_H
# define LL_PARSER_STREAM_H

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

exterm min::uns32 STRING_BLOCK;
struct string_block
{
    min::uns32 type;
    	// Packed vector type.  Is STRING_BLOCK.
    min::uns32 length;
        // Length of vector.  For string blocks,
	// always equals the max_length.
    min::uns32 max_length;
        // Maximum length of vector.

    string_block ** previous;
    string_block ** next;
        // Pointers to maintain circular list of
	// string blocks.

    // The elements of a string block are min::uns32
    // values.
};

extern string_block * current_string_block;
extern min:uns32 current_string_block_offset;
    // Pointer to next point in circular memory composed
    // of string blocks to allocate a new string.

extern min::uns32 TOKEN;
struct token
{
    min::uns32 type;
    	// Packed structure type.  Is TOKEN.
    min::uns32 token_type;  // One of:
    enum
    {
    }

    min::uns32 begin_line;
    min::uns32 begin_index;
    min::uns32 begin_column;
        // Position of the first character of the token.

    min::uns32 end_line;
    min::uns32 end_index;
    min::uns32 end_column;
        // Position of the first character AFTER the
	// token, or the end of input.
};

} }

# endif // LL_PARSER_STREAM_H
