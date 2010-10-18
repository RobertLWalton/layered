// Layers Language Parser Stream
//
// File:	ll_parser_stream.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Oct 18 11:51:54 EDT 2010
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

namespace ll { namespace parser
    { namespace stream {

extern min::uns32 TOKEN;
struct token
{
    min::uns32 type;
    	// Packed structure type.  Is TOKEN.
    min::uns32 subtype;
        // Token type.
};

} } }

# endif // LL_PARSER_STREAM_H
