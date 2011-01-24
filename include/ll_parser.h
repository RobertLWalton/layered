// Layers Language Parsers
//
// File:	ll_parser.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Jan 24 10:13:02 EST 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Strings
//	Tokens
//	Parser Closures
//	Parser

// Usage and Setup
// ----- --- -----

# ifndef LL_PARSER_H
# define LL_PARSER_H

# include <ll_lexeme.h>
# include <ll_parser_table.h>

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
    // Error messages are sent to parser->print->err.
    // Tracing to parser->print->trace is enabled by
    // `parcer->trace' flags.
    //
    uns32 (*add_tokens)
        ( parser_ptr parser, input_ptr input );

    // Function to initialize input closure.
    //
    void (*init)
        ( parser_ptr parser, input_ptr input );
};

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
	  input_ptr & input );

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
    // by this function.
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
    // Error messages are sent to parser->print->err.
    // Tracing to parser->print->trace is enabled by
    // `parcer->trace' flags.
    //
    void (*remove_tokens)
	( parser_ptr parser, output_ptr output );

    // Function to initialize output closure.
    //
    void (*init)
        ( parser_ptr parser, input_ptr input );
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
    // Error messages are sent to parser->print->err.
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

    uns32 trace;
        // Trace flags that output to parser->print->
	// trace a description of each token change made
	// in the parser token list.
};

} }

// Parser
// ------

namespace ll { namespace parser {

// Parser Trace Flags:
//
enum {

    TRACE_INPUT		= ( 1 << 0 ),
        // Trace each token input by the parser->input
	// closure (e.g. from the lexical scanner).

    TRACE_OUTPUT	= ( 1 << 1 )
        // Trace processing by the parser->output
	// closure.
};

struct parser_struct
{
    uns32 control;
        // Packed structure control word.

    // Parser parameters:

    input_ptr input;
        // Closure to call to get more tokens.  Defaults
	// to an input that inputs standard lexemes
	// using the default scanner: see ll_parser_
	// input.h.

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

    uns32 trace;
        // Trace flags.  Tracing is done to print->
	// trace.

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

    ll::lexeme::print_ptr print;
        // Print parameters used in conjunction with
	// input_file to print messages.  If a scanner
	// is used, this is the same as scanner->print.

    ll::parser::table::table_ptr hash_table;
        // Hash table for brackets and indentation
	// marks.

    ll::parser::table::table_ptr indentation_mark_table;
        // Table for indentation marks that can be
	// split.  Has 256 elements, and the entry for
	// an indentation mark whose list uns8 byte is b
	// is recorded in the indentation_mark_table[b]
	// list.

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

    char error_message[512];
        // Buffer for error messages for fatal errors.
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

// Set the parser input_file to equal the contents of
// the named file.  Return true if no error and false
// if error.  If there is an error, an error message is
// put in parser->error_message.  (Re)initialize parser.
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
