// Layers Language Parsers
//
// File:	ll_parser.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Jan 19 11:19:47 EST 2011
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
    // Closure called to read lexemes and convert them
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
    uns32 (*add_tokens)
        ( parser_ptr parser, input_ptr input );

    // Function to initialize input closure.
    //
    uns32 (*init)
        ( parser_ptr parser, input_ptr input );

    bool trace;
        // Output to parser->trace a description of each
	// token added to the parser token list.  Also
	// send error messages to parser->trace instead
	// of to parser->err.

    ll::lexeme::scanner_ptr scanner;
        // Scanner for those parser inputs that use a
	// scanner (such as the standard_input: see
	// ll_parser_input.h).  A scanner need NOT be
	// used by a parser input.
};

struct output_struct
    // Closure called to process tokens the parser has
    // produced, so that output tokens can be garbage
    // collected before all input lines are processed.
    // Called whenever the parser produces a finished
    // token.
{
    uns32 control;

    // Function to remove finished tokens from the
    // parser token list (virtual functions are not
    // permitted in packed structures).  The number of
    // finished tokens in the token list is maintained
    // in parser->finished_tokens, which may be changed
    // by this function.  Tracing to parser->trace is
    // enabled by the `trace' member.
    //
    // This function is not required to process all or
    // even any finished tokens.  This function may
    // remove, replace, or add finished tokens, as long
    // as parser->finished_tokens and parser->first are
    // maintained and unfinished tokens are not changed.
    //
    void (*run)
	( parser_ptr parser, output_ptr output );

    bool trace;
        // Output to parser->trace a description of each
	// token removed from the parser token list.
	// Also send error messages to parser->trace
	// instead of to parser->err.
};

// Default input.
//
extern input_ptr default_input;

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
    // token immediately after the last token in the
    // subexpression.  The tokens may be edited, and
    // `first' may be reset.  If first == end the
    // subexpression is or became empty.
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

    bool eof;
        // True if `input' has delivered an end-of-file
	// token to the end of the token list, so
	// `input' should not be called again.

    uns32 finished_tokens;
        // Number of finished tokens at the beginning
	// of the input list.  The `parse' function
	// produces finished tokens and calls `output'
	// to remove them.

    input_ptr input;
        // Closure to call to get more tokens.

    output_ptr output;
        // Closure to call to remove finished tokens
	// from the beginning of the token list.  May
	// be NULL_STUB if this is not to be done (the
	// finished tokens are left in the list when
	// `parse' returns).  Defaults to NULL_STUB.

    pass_ptr pass_stack;
        // List of passes to call for each subexpres-
	// sion.  Defaults to NULL_STUB.

    std::ostream * trace;
        // Stream to which parser outputs trace
	// messages.  Defaults to std::cout.

    std::ostream * err;
        // Stream to which parser outputs error mes-
	// sages.  However, if tracing is in effect,
	// error messages are output to `trace'
	// instead.  Defaults to std::cerr.
};

extern parser_ptr default_parser;

// Create a parser.  Before using the parser the input
// must be set.  If the input variable equals NULL_STUB,
// the variable is reset to a new input closure with
// default parameters (input is from std::cin) and a
// standard lexeme program.
//
// After creating the parser the explicit parentheses
// parser definition stack members of the parser must
// be set before the parser is used.
//
parser_ptr create_parser
	( input_ptr & input = default_input );

// Run a parser.  At the end of this function each top
// level expression is one token in the parser token
// list.
//
void parse ( parser_ptr parser = default_parser );

} }

# endif // LL_PARSER_H
