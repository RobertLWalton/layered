// Layers Language Parsers
//
// File:	ll_parser.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Jan 18 19:01:04 EST 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Parser

// Usage and Setup
// ----- --- -----

# ifndef LL_PARSER_H
# define LL_PARSER_H

# include <ll_lexeme.h>
# include <ll_parser_token.h>
# include <ll_parser_table.h>

namespace ll { namespace parser {

    using min::uns32;
    using min::NULL_STUB;

} }


// Parser
// ------

namespace ll { namespace parser {

struct parser_struct;
typedef min::packed_struct_updptr<parser_struct>
    parser_ptr;
struct parser_struct
{
    uns32 control;
        // Packed structure control word.

    LEX::scanner_ptr scanner;
	// Lexical scanner with standard program.

    token_ptr first;
        // First token in token list.  The tokens are a
	// doubly linked list.  NULL_STUB if this list
	// is empty.
	//
	// Tokens are added to the end of the list when
	// the scanner is called.   Tokens may be dele-
	// ted from the list or replaced in the list.

    bool eof;
        // True if scanner has delivered an end-of-file
	// token to the end of the token list, and so
	// the scanner should not be called again.

    std::ostream * trace;
        // If not NULL, the parser should output trace
	// messages listing the tokens it creates,
	// deletes, or replaces, and other appropriate
	// information, to this ostream.

    std::ostream * err;
        // If not NULL, output error messages to this
	// ostream.  However `trace' takes precedence
	// over `err' for printing error messages.
};

extern parser_ptr default_parser;

// Create a parser.  Before using the parser the scanner
// program must be set (either before or just after
// creating the parser), and after creating the parser
// the parser definition stack must be set.
//
parser_ptr create_parser
	( ll::lexeme::scanner_ptr scanner =
	      ll::lexeme::default_scannter,
	  std::ostream & trace =
	      * (std::ostream *) NULL,
	  std::ostream & err = std::cerr );

// Run a parser.  At the end of this function each top
// level expression is one token in the parser token
// list.
//
void parse ( parser_ptr parser = default_parser );

} }

# endif // LL_PARSER_H
