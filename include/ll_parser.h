// Layers Language Parsers
//
// File:	ll_parser.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Mar 15 19:38:39 EDT 2011
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
//	Parser Functions

// Usage and Setup
// ----- --- -----

# ifndef LL_PARSER_H
# define LL_PARSER_H

# include <ll_lexeme.h>
# include <ll_parser_table.h>

namespace ll { namespace parser {

    using min::uns32;
    using min::int32;
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
// memory usage.  Tokens need not be explicitly freed,
// as they will be garbage collected when they become
// unreferenceable.

struct string_struct;
typedef min::packed_vec_ptr<uns32,string_struct>
        string;
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
ll::parser::string new_string
	( uns32 n, const uns32 * s );

// Free a string and return NULL_STUB.
//
ll::parser::string free_string
	( ll::parser::string string );

// Set the maximum number of strings on the free list.
// Set to 0 to make list empty.  Set to < 0 if there is
// no limit.  Defaults to 100.
//
void set_max_string_free_list_size ( int n );

// Resize a string so its maximum and current length
// coincide.  This should only be done to strings
// that are going to be long-lived and not freed by
// free_string.
//
void resize ( ll::parser::string string );

} }

// Tokens
// ------

namespace ll { namespace parser {

struct token_struct;
typedef min::packed_struct_updptr<token_struct>
    token;
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

    ll::parser::string string;
        // Character string for lexemes.

    ll::lexeme::position begin, end;
        // Position of the first character of the token.
        // and of the first character AFTER the token,
	// or the end of input.

    ll::parser::token next, previous;
        // Doubly linked list pointers for tokens.
};

// Allocate a new token of the given type.  Value is set
// to min:MISSING and string to NULL_STUB.
//
ll::parser::token new_token ( uns32 type );

// Free token.  Token is put on internal free list after
// its value is set to min:MISSING and its string to
// NULL_STUB.
//
void free ( ll::parser::token token );

// Set the maximum number of tokens on the free list.
// Set to 0 to make list empty.  Set to < 0 if there is
// no limit.  Defaults to 1000.
//
void set_max_token_free_list_size ( int n );

// Put a token just before a given token t on a list of
// tokens.
//
inline void put_before
	( ll::parser::token t,
	  ll::parser::token token )
{
    min::locatable ( token, token->next ) =
        t;
    min::locatable ( token, token->previous ) =
        t->previous;
    min::locatable ( token, token->previous->next ) =
        token;
    min::locatable ( token, token->next->previous ) =
        token;
}

// Put a token just before a given token t on a list of
// tokens headed by first.
//
inline void put_before
	( ll::parser::token & first,
	  ll::parser::token t,
	  ll::parser::token token )
{
    put_before ( t, token );
    if ( first == t ) first = token;
}

// Put a token on the end of a token list with given
// first element.
//
inline void put_at_end
	( min::ptr<ll::parser::token> first,
	  ll::parser::token token )
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
inline ll::parser::token remove
	( min::ptr<ll::parser::token> first,
	  ll::parser::token token )
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
    min::locatable
        ( token->previous, token->previous->next ) =
	    token->next;
    min::locatable
        ( token->next, token->next->previous ) =
	    token->previous;
    return token;
}

// Remove first token from a list of tokens with given
// first token.  Return min::NULL_STUB if list empty.
//
inline ll::parser::token remove
	( min::ptr<ll::parser::token> first )
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
    input;

struct output_struct;
typedef min::packed_struct_updptr<output_struct>
    output;

struct pass_struct;
typedef min::packed_struct_updptr<pass_struct>
    pass;

struct parser_struct;
typedef min::packed_struct_updptr<parser_struct>
    parser;

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
    // Error messages are sent to parser->printer.
    // Tracing to parser->printer is enabled by
    // `parcer->trace' flags.
    //
    uns32 (*add_tokens)
	    ( ll::parser::parser parser,
	      ll::parser::input input );

    // Function to initialize input closure.  If not
    // NULL, called when the parser is initialized.
    //
    void (*init)
        ( ll::parser::parser parser,
	  ll::parser::input input );
};

// Set input closure functions.  If `input' is NULL_
// STUB, create closure and set `input' to a pointer
// to the created closure.  `input' must be loca-
// table by garbage collector.
//
void init
	( min::ptr<ll::parser::input> input,
	  uns32 (*add_tokens)
	      ( ll::parser::parser parser,
	        ll::parser::input input ),
	  void (*init)
	      ( ll::parser::parser parser,
	        ll::parser::input input ) = NULL );

struct output_struct
    // Closure called to process tokens the parser has
    // produced, so that output tokens can be garbage
    // collected before all input lines are processed.
    // Called whenever the parser produces a finished
    // token.
{
    uns32 control;

    // Function to remove finished tokens from the
    // parser token list.  The number of finished tokens
    // in the token list is maintained in parser->
    // finished_tokens, which may be changed by this
    // function.
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
    // Error messages are sent to parser->printer.
    // Tracing to parser->printer is enabled by
    // `parcer->trace' flags.
    //
    void (*remove_tokens)
	( ll::parser::parser parser,
	  ll::parser::output output );

    // Function to initialize output closure.  If not
    // NULL, called when the parser is initialized.
    //
    void (*init)
        ( ll::parser::parser parser,
	  ll::parser::output output );
};

// Set output closure functions.  If `output' is NULL_
// STUB, create closure and set `output' to a pointer
// to the created closure.  `output' must be loca-
// table by garbage collector.
//
void init
	( min::ptr<ll::parser::output> output,
	  void (*remove_tokens)
	      ( ll::parser::parser parser,
	        ll::parser::output output ),
	  void (*init)
	      ( ll::parser::parser parser,
	        ll::parser::output output ) = NULL );

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

    ll::parser::pass next;
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
    // Error messages are sent to parser->printer.
    // Returns true if no fatal errors, and false if
    // there is a fatal error.
    //
    bool (*run) ( ll::parser::parser parser,
                  ll::parser::pass pass,
    		  ll::parser::token & first,
		  ll::parser::token end );

    // Function to initialize pass closure.  Called if
    // not NULL when the parser is initialized.
    //
    // For normal operations there is no need to
    // init a pass, but doing so may be useful for
    // keeping statistics.
    //
    void (*init)
        ( ll::parser::parser parser,
	  ll::parser::pass pass );

    uns32 trace;
        // Trace flags that output to parser->printer a
	// description of each token change made in the
	// parser token list.
};

// Set pass closure functions.  If `pass' is NULL_
// STUB, create closure and set `pass' to a pointer
// to the created closure.  `pass' must be loca-
// table by garbage collector.
//
void init
	( min::ptr<ll::parser::pass> pass,
	  bool (*run)
	      ( ll::parser::parser parser,
	        ll::parser::pass pass,
		ll::parser::token & first,
		ll::parser::token end ),
	  void (*init)
	      ( ll::parser::parser parser,
	        ll::parser::pass pass ) = NULL );

// Place `pass' on the parser->pass_stack just after
// `previous', of if the latter is NULL_STUB, put `pass'
// at the beginning of the stack.  `pass' MUST NOT be
// on any parser->pass_stack when this function is
// called.
//
void place
	( ll::parser::parser parser,
	  ll::parser::pass pass,
	  ll::parser::pass previous = NULL_STUB );

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

    ll::parser::input input;
        // Closure to call to get more tokens.  If
	// NULL_STUB when parse function called, set
	// to ll::parser::default_input, which inputs
	// standard lexemes using the scanner as per
	// ll_parser_input.h.  Set to NULL_STUB when
	// parser is created.

    ll::parser::output output;
        // Closure to call to remove finished tokens
	// from the beginning of the token list.  May
	// be NULL_STUB if this is not to be done (the
	// finished tokens are left in the list when
	// `parse' returns).  Set to NULL_STUB when
	// parser is created.

    ll::parser::pass pass_stack;
        // List of passes to call for each subexpres-
	// sion.  If NULL_STUB there are no passes.  Set
	// to NULL_STUB when parser is created.

    uns32 trace;
        // Trace flags.  Tracing is done to parser->
	// printer.

    ll::lexeme::scanner scanner;
        // Scanner for those parser inputs that use a
	// scanner (such as default_input: see
	// ll_parser_input.h).  A scanner need NOT be
	// used by a parser input.  If parser->input
	// and parser->scanner are BOTH NULL_STUB when
	// the `parse' function is called, this is set
	// to a scanner with default parameter settings
	// and a standard lexical program as per ll_
	// parser_input.h.  If scanner->input_file is
	// NULL_STUB when the `parser' function is
	// called, it is set from parser->input_file.
	// Ditto for scanner->printer and parser->
	// printer.

    min::file input_file;
        // Input file used to print messages.  If a
	// scanner is used, this MUST be the same as
	// scanner->input_file and is also used for
	// input to the scanner.  Set to NULL_STUB
	// when the parser is created.  If NULL_STUB
	// when the `parse' function is called, this
	// is set to scanner->input_file which must
	// exist and NOT be NULL_STUB.

    min::printer printer;
        // Printer used to print messages.  If a scanner
	// is used, this MUST be the same as scanner->
	// printer.  Set to NULL_STUB when the parser is
	// created.  If NULL_STUB when the `parse' func-
	// tion is called, this is set to scanner->
	// printer which must exist and NOT be NULL_
	// STUB.

    ll::parser::table::table bracket_table;
        // Hash table for brackets and indentation
	// marks.

    ll::parser::table::split_table split_table;
        // Table for indentation splits associated with
	// indentation marks that can be split.

    int32 indent_offset;
        // Amount the indentation of a line has to be
	// offset from the indentation of an indented
	// paragraph in order to avoid error messages.
	// Must be signed integer so indentations can
	// be set to - indent_offset and indentation
	// check computations are signed, but is
	// always >= 0.  Defaults to 2.

    // Parser state:

    ll::parser::token first;
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
	// produces finished tokens and calls `output'.
};

extern min::locatable_var<ll::parser::parser>
       default_parser;

// There are several parameters that when set cause a
// parser to be (re)initialized.  These are all settable
// by init parser functions.  For these the parser is
// specified by a variable, and if this == NULL_STUB,
// a new parser is created and a pointer to it is stored
// in the variable.  This variable MUST BE locatable by
// the garbage collector.
//
// When a new parser is created, parser parameters are
// are set to defaults.  Otherwise these are left un-
// touched, and can be set either before or immediately
// after a call to init parser.  Also, several of these
// functions can be called one after the other to set
// non-conflicting parameters.

// Simply (re)initialize a parser.
//
void init
	( min::ptr<ll::parser::parser> parser );

// Reinitialize and set a parameter.  Some parameters
// are copied to both parser and any scanner that
// exists for the parser.
//
inline void init_print_flags
	( min::ptr<ll::parser::parser> parser,
	  min::uns32 print_flags )
{
    init ( parser );
    min::init_print_flags
        ( min::locatable
	      ( parser, parser->input_file ),
	  print_flags );
}
inline void init_printer
	( min::ptr<ll::parser::parser> parser,
	  min::printer printer )
{
    init ( parser );
    min::locatable ( parser, parser->printer ) =
        printer;
    if ( parser->scanner != min::NULL_STUB )
        ll::lexeme::init_printer
	    ( min::locatable
	          ( parser, parser->scanner ),
	      printer );
}

// The following initialize the parser and then call
// the corresponding min::init_... function for
// parser->input_file.
//
void init_input_stream
	( min::ptr<ll::parser::parser> parser,
	  std::istream & istream,
	  min::uns32 print_flags = 0,
	  min::uns32 spool_lines = min::ALL_LINES );
void init_input_file
	( min::ptr<ll::parser::parser> parser,
	  min::file ifile,
	  min::uns32 print_flags = 0,
	  min::uns32 spool_lines = min::ALL_LINES );
bool init_input_named_file
	( min::ptr<ll::parser::parser> parser,
	  min::gen file_name,
	  min::uns32 print_flags = 0,
	  min::uns32 spool_lines = min::ALL_LINES );
void init_input_string
	( min::ptr<ll::parser::parser> parser,
	  const char * data,
	  min::uns32 print_flags = 0,
	  min::uns32 spool_lines = min::ALL_LINES );

// The following initialize the parser and then call
// the corresponding min::init_... function for
// parser->printer.
//
void init_output_stream
	( min::ptr<ll::parser::parser> parser,
	  std::ostream & ostream );

// Run a parser.  At the end of this function each top
// level expression is one token in the parser token
// list, and parser->finished_tokens is the number of
// these tokens, unless parser->output exists and has
// modified these tokens and parser->finished_tokens.
//
void parse ( ll::parser::parser parser =
		 default_parser );

// Parser Functions
// ------ ---------

// Locate the key prefix in the hash table that
// corresponds to the longest available string of tokens
// beginning with `current'.  Tokens are added to the
// token list as necessary.  It is assumed that the
// token list finally ends with an end-of-file token,
// and this cannot be part of any hash table entry
// (because it is not a SYMBOL).
//
// Returns NULL_STUB if no such key prefix.  If a key
// prefix is found, `current' is set to the first token
// after the tokens used to find the key prefix.  If no
// case does not change `current'.
//
ll::parser::table::key_prefix find_key_prefix
	( ll::parser::parser parser,
	  ll::parser::token & current,
	  ll::parser::table::table table );

// Locate the hash table entry in the hash table that
// corresponds to the longest available string of tokens
// beginning with `current'.  Tokens are added to the
// token list as necessary.  It is assumed that the
// token list finally ends with an end-of-file token,
// and this cannot be part of any hash table entry
// (because it is not a SYMBOL).
//
// Only hash table entries which have a selector bit
// on are considered.
//
// Returns NULL_STUB if no such entry.  If an entry is
// returned, `current' is set to the token just AFTER
// the last token used to look up the entry.  If no
// entry is returned, `current' is left unchanged.
//
ll::parser::table::root find_entry
	( ll::parser::parser parser,
	  ll::parser::token & current,
	  ll::parser::table::selectors selectors,
	  ll::parser::table::table table );

} }

# endif // LL_PARSER_H
