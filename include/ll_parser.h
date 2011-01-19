// Layers Language Parsers
//
// File:	ll_parser.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Jan 19 01:17:13 EST 2011
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

// The parser performs explict subexpression recognition
// and calls the passes in the pass stack for each
// recognized subexpression.

struct input_struct;
typedef min::packed_struct_updptr<input_struct>
    input_ptr;
struct pass_struct;
typedef min::packed_struct_updptr<pass_struct>
    pass_ptr;
struct parser_struct;
typedef min::packed_struct_updptr<parser_struct>
    parser_ptr;

struct input_struct
    // Closure to call to read lexemes and convert them
    // into tokens that are put on the end of the parser
    // list of tokens.
{
    uns32 control;

    // Function to add tokens to the end of the parser
    // token list (virtual functions are not permitted
    // in packed structures).  The number of tokens
    // added is returned.  This is 0 if there are no
    // more tokens due to end of file (also, parser->eof
    // is set by this function in this case).  Tracing
    // to parser->trace is enabled by the `trace'
    // member.
    //
    uns32 (*run) ( parser_ptr parser, input_ptr input );

    bool trace;
        // Output to parser->trace a description of each
	// token added to the parser token list.  Also
	// send error messages to parser->trace instead
	// of to parser->err.
};

struct pass_struct
    // Closure to call to execute a pass on a subexpres-
    // sion.  These are organized in a list called the
    // `pass stack'.
{
    uns32 control;

    pass_ptr next;
        // Next pass in the pass stack, or NULL_STUB.
	// The parser executes passes in `next-later'
	// order on a subexpression.

    // Function to execute the pass (virtual functions
    // are not permitted in packed structures).
    //
    // The pass is run on the subexpression whose
    // first token is given.  The `end' token is the
    // token immediately after the first token.  The
    // tokens may be edited, and `first' reset.  If
    // first == end the subexpression is or became
    // empty.
    //
    uns32 (*run) ( parser_ptr parser, pass_ptr pass,
    		   token_ptr & first, token_ptr end );

    bool trace;
        // Output to parser->trace a description of each
	// token change made in the parser token list.
	// Also send error messages to parser->trace
	// instead of to parser->err.
};

struct parser_struct
{
    uns32 control;
        // Packed structure control word.

    token_ptr first;
        // First token in token list.  The tokens are a
	// doubly linked list.  NULL_STUB if this list
	// is empty.
	//
	// Tokens are added to the end of the list when
	// the input is called.   Tokens may be dele-
	// ted from the list or replaced in the list.

    input_ptr input;
        // Closure to call to get more tokens.

    pass_ptr pass_stack;
        // List of passes to call for each subexpres-
	// sion.

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
// the parser definition stack members of the parser
// must be set.
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
