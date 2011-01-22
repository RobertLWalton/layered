// Layers Language Parsers
//
// File:	ll_parser.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Jan 22 03:09:50 EST 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Parser Closures
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


// Parser Closures
// ------ --------

namespace ll { namespace parser {

// Pointers to packed structs defined below.

struct input_struct;
typedef min::packed_struct_updptr<input_struct>
    input_ptr;

struct output_struct;
typedef min::packed_struct_updptr<output_struct>
    output_ptr;

struct pass_struct;
typedef min::packed_struct_updptr<pass_struct>
    pass_ptr;

struct parser_struct;
typedef min::packed_struct_updptr<parser_struct>
    parser_ptr;

// Note: virtual functions are NOT permitted in packed
// structs.

struct input_struct
    // Closure called to read lexemes and convert them
    // into tokens that are put on the end of the parser
    // list of tokens.
{
    uns32 control;

    // Function to add tokens to the end of the parser
    // token list.  The number of tokens added is
    // returned.  This is 0 if parser->eof is set, which
    // means that there are no more tokens due to end of
    // file.  This function sets parser->eof when it
    // returns the last token (typically and end of file
    // token).
    //
    // Error messages are sent to parser->err or
    // parser->trace (see `trace' member below). Trac-
    // ing to parser->trace is enabled by the `trace'
    // member.
    //
    uns32 (*add_tokens)
        ( parser_ptr parser, input_ptr input );

    // Function to initialize input closure.
    //
    void (*init)
        ( parser_ptr parser, input_ptr input );

    bool trace;
        // Output to parser->trace a description of each
	// token added to the parser token list.  Also
	// send input error messages to parser->trace
	// instead of to parser->err.
};

extern input_ptr & default_input;
    // Default value for parser->input.  This variable
    // is not set until the first parser is created
    // (first call to init_parser).
    //
    // This input expects parser->scanner to have a
    // standard lexeme program (see ll_lexeme_standard)
    // and returns lexemes produced by that scanner as
    // per ll_parser_input.h.

// Set input closure functions.  If `input' is NULL_
// STUB, create closure and set `input' to a pointer
// to the created closure.  `input' must be loca-
// table by garbage collector.
//
void init_input
	( uns32 (*add_tokens)
	      ( parser_ptr parser, input_ptr input ),
	  void (*init)
	      ( parser_ptr parser, input_ptr input ),
	  input_ptr & input = default_input );

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
    // This function is called whenever parser->
    // finished_tokens is incremented by +1.
    //
    // This function is not required to process all or
    // even any finished tokens.  This function may
    // remove, replace, or add finished tokens, as long
    // as parser->finished_tokens and parser->first are
    // maintained and unfinished tokens are not changed.
    //
    void (*remove_tokens)
	( parser_ptr parser, output_ptr output );

    // Function to initialize output closure.
    //
    void (*init)
        ( parser_ptr parser, input_ptr input );

    bool trace;
        // Output to parser->trace a description of each
	// token removed from the parser token list.
	// Also send error messages to parser->trace
	// instead of to parser->err.
};

struct pass_struct
    // Closure to call to execute a pass on a subexpres-
    // sion.  These closures are organized in a list
    // called the `pass stack'.
    //
    // The parser performs explict subexpression recog-
    // nition and calls the passes in the pass stack for
    // each recognized subexpression.
{
    uns32 control;

    pass_ptr next;
        // Next pass in the pass stack, or NULL_STUB.
	// The parser executes passes in `next-later'
	// order on a subexpression.

    // Function to execute the pass.
    //
    // The pass is run on the subexpression whose
    // first token is given.  The `end' token is the
    // token immediately after the last token in the
    // subexpression.  The tokens within the subexpres-
    // sion may be edited, and `first' may be reset.
    // If first == end the subexpression is or became
    // empty.
    //
    uns32 (*run) ( parser_ptr parser, pass_ptr pass,
    		   token_ptr & first, token_ptr end );

    // Function to initialize pass closure.
    //
    // For normal operations there is no need to
    // init a pass, but doing so may be useful for
    // keeping statistics.
    //
    void (*init)
        ( parser_ptr parser, pass_ptr pass );

    bool trace;
        // Output to parser->trace a description of each
	// token change made in the parser token list.
	// Also send error messages to parser->trace
	// instead of to parser->err.
};

} }

// Parser
// ------

namespace ll { namespace parser {

struct parser_struct
{
    uns32 control;
        // Packed structure control word.

    // Parser parameters:

    input_ptr input;
        // Closure to call to get more tokens.  Defaults
	// to ll::parser::default_input.

    output_ptr output;
        // Closure to call to remove finished tokens
	// from the beginning of the token list.  May
	// be NULL_STUB if this is not to be done (the
	// finished tokens are left in the list when
	// `parse' returns).  Defaults to NULL_STUB.

    pass_ptr pass_stack;
        // List of passes to call for each subexpres-
	// sion.  Defaults to NULL_STUB.  Should be
	// set after parser is created to the desired
	// pass stack.

    std::ostream * trace;
        // Stream to which parser outputs trace
	// messages.  Defaults to & std::cout.

    std::ostream * err;
        // Stream to which parser outputs error mes-
	// sages.  However, if tracing is in effect,
	// error messages are output to `trace'
	// instead.  Defaults to & std::cerr.

    ll::lexeme::scanner_ptr scanner;
        // Scanner for those parser inputs that use a
	// scanner (such as the standard_input: see
	// ll_parser_input.h).  A scanner need NOT be
	// used by a parser input.  Defaults to a
	// scanner with default parameter settings and
	// a standard lexical program (see ll_lexeme_
	// standard.h and ll_parser_input.h).

    ll::lexeme::file_ptr input_file;
        // Input file used to print messages.  If a
	// scanner is used, this is the same as
	// scanner->input_file.

    // Parser state:

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
};

extern parser_ptr & default_parser;

// There are several parameters that when set cause a
// parser to be (re)initialized.  These are all settable
// by init_parser functions.  For these the parser is
// specified by a variable, and if this == NULL_STUB,
// a new parser is created and a pointer to it is stored
// in the variable.  This variable MUST BE locatable by
// the garbage collector.
//
// When a new parser is created, parser parameters are
// are set to defaults.  Otherwise these are left un-
// touched, and can be set either before or immediately
// after a call to init_parser.

// Simply (re)initialize a parser.
//
void init_parser
	( parser_ptr & parser = default_parser );

// Set the parser input_file to equal the contents
// of the named file.  Return true if no error and
// false if error.  If there is an error, an error
// message is put in parser->scanner->error_message.
// (Re)initialize parser.
//
// See init_file in ll_lexeme.h for more details.
//
bool init_parser
	( const char * file_name,
	  parser_ptr & parser = default_parser );

// Ditto but initialize input_file to input from an
// istream and have the given spool_length.  See
// init_file in ll_lexeme.h for details.
//
void init_parser
	( std::istream & istream,
	  const char * file_name,
	  uns32 spool_length,
	  parser_ptr & parser = default_parser );

// Ditto but initialize input_file to the contents of
// a data string.  See init_file in ll_lexeme.h for
// details.
//
void init_parser
	( const char * file_name,
	  const char * data,
	  parser_ptr & parser = default_parser );

// Run a parser.  At the end of this function each top
// level expression is one token in the parser token
// list, and parser->finished_tokens is the number of
// these tokens, unless parser->output exists and has
// modified these tokens and parser->finished_tokens.
//
void parse ( parser_ptr parser = default_parser );

} }

# endif // LL_PARSER_H
