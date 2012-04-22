// Layers Language Parsers
//
// File:	ll_parser.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun Apr 22 07:55:04 EDT 2012
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

    extern min::locatable_gen
        position,	// . position
	initiator,	// . initiator
	terminator,	// . terminator
	separator,	// . separator
	middle,		// . middle
	name,		// . name
	arguments,	// . arguments
	keys,		// . keys
	doublequote,	// "
	number_sign,	// #
	new_line,	// \n
	semicolon,	// ;
	left_square,    // [
	right_square,   // ]
	comma,		// ,
	parser_lexeme,	// parser
	error_operator,	// ERROR'OPERATOR
	error_operand;	// ERROR'OPERAND

    extern min::printer_format name_format;
        // Same as min::default_printer_format except
	// that {str,lab}_{pre,post}fix are all "".

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
// start out being long enough to hold a typical maxi-
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

    const ll::parser::string_insptr next;
        // Pointer to next block on free list, if string
	// is on free list.  List is NULL_STUB termina-
	// ted.

    // The elements of a string are uns32 UNICODE
    // characters.
};

MIN_REF ( ll::parser::string_insptr, next,
          ll::parser::string_insptr )

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
    EXPRESSION		= 0xFFFFFFFF,
    OPERATOR		= 0xFFFFFFFE
};
struct token_struct
{
    uns32 control;
    	// Packed structure control word.

    uns32 type;  // One of:
	//
        // For lexemes: the lexeme type.
	//
	// For expressions:
	//
    	//	EXPRESSION
	//
	// For composites:
	//
	//	OPERATOR

    const min::gen value;
        // Value for some lexeme types and for
	// expressions.

    const ll::parser::string string;
        // Character string for some lexeme types.

    min::phrase_position position;
        // Position of the first character of the token
        // and of the first character AFTER the token,
	// or the end of input.

    min::uns32 indent;
        // Indent of the first character of the token.
	// Computed by assuming tabs are set every 8
	// columns and form feeds and vertical tabs
	// to not print.  Equal to LEX::AFTER_GRAPHIC
	// if there is a non-whitespace character before
	// the token in the line containing the token.

    const ll::parser::token next, previous;
        // Doubly linked list pointers for tokens.
};

MIN_REF ( ll::parser::string, string,
          ll::parser::token )
MIN_REF ( min::gen, value,
          ll::parser::token )
MIN_REF ( ll::parser::token, next,
          ll::parser::token )
MIN_REF ( ll::parser::token, previous,
          ll::parser::token )

// Allocate a new token of the given type.  Value is set
// to min::MISSING() and string to min::NULL_STUB.
//
ll::parser::token new_token ( uns32 type );

// Free token and return NULL_STUB.  Token is put on
// internal free list after its value is set to MISSING
// and its string is freed and set to NULL_STUB.
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
    next_ref(token) = t;
    previous_ref(token) = t->previous;
    next_ref(token->previous) = token;
    previous_ref(token->next) = token;
}

// Put a token just before a given token t on a list of
// tokens headed by first.
//
inline void put_before
	( min::ref<ll::parser::token> first,
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
	( min::ref<ll::parser::token> first,
	  ll::parser::token token )
{
    if ( first == min::NULL_STUB )
    {
        first = token;
	previous_ref(token) = next_ref(token) = token;
    }
    else put_before ( first, token );
}

// Remove token from the token list with given first
// token and return the token removed.  Note that the
// removed token is NOT freed.
//
inline ll::parser::token remove
	( min::ref<ll::parser::token> first,
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
    next_ref(token->previous) = token->next;
    previous_ref(token->next) = token->previous;
    return token;
}

// Remove first token from a list of tokens with given
// first token.  Return min::NULL_STUB if list empty.
// Note that the removed token is NOT freed.
//
inline ll::parser::token remove
	( min::ref<ll::parser::token> first )
{
    if ( first == min::NULL_STUB )
        return min::NULL_STUB;
    else
    	return remove ( first, first );
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
	( min::ref<ll::parser::input> input,
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
	( min::ref<ll::parser::output> output,
	  void (*remove_tokens)
	      ( ll::parser::parser parser,
	        ll::parser::output output ),
	  void (*init)
	      ( ll::parser::parser parser,
	        ll::parser::output output ) = NULL );

typedef bool ( * run_pass )
        ( ll::parser::parser parser,
          ll::parser::pass pass,
	  ll::parser::table::selectors selectors,
    	  ll::parser::token & first,
	  ll::parser::token next );
typedef void ( * init_pass )
        ( ll::parser::parser parser,
	  ll::parser::pass pass );

struct pass_struct
    // Closure to call to execute a pass on a subexpres-
    // sion.  These closures are organized in a list
    // called the `pass stack'.
    //
    // The parser performs explict subexpression recog-
    // nition and calls the passes in the pass stack for
    // each recognized explicit subexpression.
{
    uns32 control;

    const ll::parser::pass next;
        // Next pass in the pass stack, or NULL_STUB.
	// The parser executes passes in `next-later'
	// order on a subexpression.

    // Function to execute the pass.
    //
    // The pass is run on the subexpression whose
    // first token is given.  The `next' token is the
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
    ll::parser::run_pass run_pass;

    // Function to initialize pass closure.  Called if
    // not NULL when the parser is initialized.
    //
    // For normal operations there is no need to
    // init a pass, but doing so may be useful for
    // keeping statistics.
    //
    ll::parser::init_pass init_pass;

    uns32 trace;
        // Trace flags that output to parser->printer a
	// description of each selected change made in
	// the parser token list.

    ll::parser::table::selectors selectors;
        // Pass is run only if its selectors match those
	// of the explicit subexpression.
};

MIN_REF ( ll::parser::pass, next, ll::parser::pass )

// Set pass closure functions.  If `pass' is NULL_
// STUB, create closure and set `pass' to a pointer
// to the created closure.  `pass' must be loca-
// table by garbage collector.
//
void init
	( min::ref<ll::parser::pass> pass,
	  ll::parser::run_pass run_pass,
	  ll::parser::init_pass init_pass = NULL );

// Place `pass' on the parser->pass_stack just before
// `next', or if the latter is NULL_STUB, put `pass'
// at the end of the stack.  `pass' MUST NOT be on
// any parser->pass_stack when this function is called.
//
void place
	( ll::parser::parser parser,
	  ll::parser::pass pass,
	  ll::parser::pass next = NULL_STUB );

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

    TRACE_OUTPUT	= ( 1 << 1 ),
        // Trace processing by the parser->output
	// closure.

    TRACE_EXPLICIT_SUBEXPRESSIONS = ( 1 << 2 )
        // Trace explicit subexpressions.
};

struct parser_struct
{
    uns32 control;
        // Packed structure control word.

    // Parser parameters:

    const ll::parser::input input;
        // Closure to call to get more tokens.  Must
	// be set to a value other than NULL_STUB before
	// the `parse' function is called.  Set to
	// NULL_STUB when the parser is created.

    const ll::parser::output output;
        // Closure to call to remove finished tokens
	// from the beginning of the token list.  May
	// be NULL_STUB if this is not to be done (the
	// finished tokens are left in the list when
	// `parse' returns).  Set to NULL_STUB when
	// parser is created.

    const ll::parser::pass pass_stack;
        // List of passes to call for each explicit
	// subexpression.  If NULL_STUB there are no
	// passes.  Set to NULL_STUB when parser is
	// created.

    uns32 trace;
        // Parser trace flags: see above.  Tracing is
	// done to parser->printer.

    const ll::lexeme::scanner scanner;
        // Scanner for those parser `input' closures
	// that use a scanner (such as ll::parser::
	// standard::input: see ll_parser_standard.h).
	//
	// A scanner need NOT be used by a parser input.
	// If parser->scanner is NOT NULL_STUB when the
	// `parse' function is called, parser->scanner->
	// input_file is set from parser->input_file if
	// the former is NULL_STUB and the latter is not
	// NULL_STUB, and ditto for parser->scanner->
	// printer and parser->printer.

    const min::file input_file;
        // Input file used to print messages.  If a
	// scanner is used, this MUST be the same as
	// scanner->input_file and is also used for
	// input to the scanner.  Set to NULL_STUB
	// when the parser is created.  If NULL_STUB
	// when the `parse' function is called, this
	// is set to scanner->input_file which must
	// exist and NOT be NULL_STUB.

    const min::printer printer;
        // Printer used to print messages.  If a scanner
	// is used, this MUST be the same as scanner->
	// printer.  Set to NULL_STUB when the parser is
	// created.  If NULL_STUB when the `parse' func-
	// tion is called, this is set to scanner->
	// printer which must exist and NOT be NULL_
	// STUB.

    const ll::lexeme::scanner name_scanner;
        // Scanner used to scan name strings, e.g., by
	// parser definitions.  Defaults to a scanner
	// with the same program and printer as the main
	// parser scanner.

    const ll::parser::table::selector_name_table
    	    selector_name_table;
        // Selector name table.

    const ll::parser::table::table bracket_table;
        // Hash table for brackets and indentation
	// marks.

    const ll::parser::table::split_table split_table;
        // Table for indentation splits associated with
	// indentation marks that can be split.

    ll::parser::table::selectors selectors;
        // Top level selectors.

    int32 indent_offset;
        // Amount the indentation of a line has to be
	// offset from the indentation of an indented
	// paragraph in order to avoid error messages.
	// Must be signed integer so indentations can
	// be set to - indent_offset and indentation
	// check computations are signed, but is
	// always >= 0.  Defaults to 2.

    // Parser state:

    const ll::parser::token first;
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
	// of the token list.  The `parse' function
	// produces finished tokens and calls `output'.
};

MIN_REF ( ll::parser::input, input,
          ll::parser::parser )
MIN_REF ( ll::parser::output, output,
          ll::parser::parser )
MIN_REF ( ll::parser::pass, pass_stack,
          ll::parser::parser )
MIN_REF ( ll::lexeme::scanner, scanner,
          ll::parser::parser )
MIN_REF ( min::file, input_file,
          ll::parser::parser )
MIN_REF ( min::printer, printer,
          ll::parser::parser )
MIN_REF ( ll::lexeme::scanner, name_scanner,
          ll::parser::parser )
MIN_REF ( ll::parser::table::table, bracket_table,
          ll::parser::parser )
MIN_REF ( ll::parser::table::split_table, split_table,
          ll::parser::parser )
MIN_REF ( ll::parser::table::selector_name_table,
		selector_name_table,
          ll::parser::parser )
MIN_REF ( ll::parser::token, first,
          ll::parser::parser )

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
	( min::ref<ll::parser::parser> parser );

// Reinitialize and set a parameter.  Some parameters
// are copied to both parser and any scanner that
// exists for the parser.
//
inline void init_print_flags
	( min::ref<ll::parser::parser> parser,
	  min::uns32 print_flags )
{
    init ( parser );
    min::init_print_flags
        ( input_file_ref(parser),
	  print_flags );
}
inline void init_printer
	( min::ref<ll::parser::parser> parser,
	  min::printer printer )
{
    init ( parser );
    printer_ref(parser) = printer;
    if ( parser->scanner != min::NULL_STUB )
        ll::lexeme::init_printer
	    ( scanner_ref(parser),
	      printer );
}

// The following initialize the parser and then call
// the corresponding min::init_... function for
// parser->input_file.
//
void init_input_stream
	( min::ref<ll::parser::parser> parser,
	  std::istream & istream,
	  min::uns32 print_flags = 0,
	  min::uns32 spool_lines = min::ALL_LINES );
void init_input_file
	( min::ref<ll::parser::parser> parser,
	  min::file ifile,
	  min::uns32 print_flags = 0,
	  min::uns32 spool_lines = min::ALL_LINES );
bool init_input_named_file
	( min::ref<ll::parser::parser> parser,
	  min::gen file_name,
	  min::uns32 print_flags = 0,
	  min::uns32 spool_lines = min::ALL_LINES );
void init_input_string
	( min::ref<ll::parser::parser> parser,
	  min::ptr<const char> data,
	  min::uns32 print_flags = 0,
	  min::uns32 spool_lines = min::ALL_LINES );

// The following initialize the parser and then call
// the corresponding min::init_... function for
// parser->printer.
//
void init_output_stream
	( min::ref<ll::parser::parser> parser,
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
// beginning with `current'.  If `next' is NULL_STUB,
// tokens are added to the token list as necessary, but
// otherwise only tokens before `next' are considered.
// Only tokens with a non-MISSING token value are
// considered.  If `next' is NULL_STUB, it is assumed
// that the token list finally ends with an end-of-file
// token, and this cannot be part of any hash table
// entry (because its token value is MISSING).
//
// Returns NULL_STUB if no such key prefix.  If a key
// prefix is found, `current' is set to the first token
// after the tokens used to find the key prefix.  If no
// key prefix is found, `current' is not changed.
//
ll::parser::table::key_prefix find_key_prefix
	( ll::parser::parser parser,
	  ll::parser::token & current,
	  ll::parser::table::table table,
	  ll::parser::token next = NULL_STUB );

// Locate the hash table entry in the hash table that
// corresponds to the longest available string of tokens
// beginning with `current'.  If `next' is NULL_STUB,
// tokens are added to the token list as necessary, but
// otherwise only tokens before `next' are considered.
// Only tokens with a non-MISSING token value are
// considered.  If `next' is NULL_STUB, it is assumed
// that the token list finally ends with an end-of-file
// token, and this cannot be part of any hash table
// entry (because its token value is MISSING).
//
// Only hash table entries which have a selector bit
// on are considered.
//
// Returns NULL_STUB if no such entry.  If an entry is
// returned, `current' is set to the token just AFTER
// the last token used to look up the entry.  If no
// entry is returned, `current' is left unchanged.
//
// The key_prefix used to find the entry is also
// returned if there is an entry, so that find_next_
// entry below can be used.
//
ll::parser::table::root find_entry
	( ll::parser::parser parser,
	  ll::parser::token & current,
	  ll::parser::table::key_prefix & key_prefix,
	  ll::parser::table::selectors selectors,
	  ll::parser::table::table table,
	  ll::parser::token next = NULL_STUB );

// Locate the NEXT hash table entry in the hash table
// after the last entry found by `find_entry' or a
// previous call to this `find_next_entry' function.
//
// Only hash table entries which have a selector bit
// on are considered.
//
// The entries are searched in the order longest first,
// and then in newest first order (i.e., most recently
// pushed to the hash table are the newest).
//
// This function is for use when in addition to selector
// bits the entry to be found must meet other criteria
// not easily expressed by giving arguments to a func-
// tion.
//
ll::parser::table::root find_next_entry
	( ll::parser::parser parser,
	  ll::parser::token & current,
	  ll::parser::table::key_prefix & key_prefix,
	  ll::parser::table::selectors selectors,
	  ll::parser::table::root last_entry );

// First, invokes the given pass, if that is not NULL_
// STUB, on the expression consisting of the tokens
// beginning with `first' and ending just before `next'.
// If the pass returns false, this function returns
// false immediately.  Otherwise this function returns
// true.
//
// Then replaces the expression tokens (which may have
// been changed by the pass) by a resulting EXPRESSION
// token.  Adds the m attributes whose names and values
// are given, and allows for the latter addition of n
// attributes.  Sets the position of the new EXPRESSION
// token from the given argument.  The resulting
// EXPRESSION token is in first == next->previous.
//
// Any token in the expression being output that has a
// MISSING token value must be a non-natural number or
// quoted string.  These are replaced by a subexpression
// whose sole element is the token string of the token
// as a string general value and whose .initiator is #
// for a number or " for a quoted string.
//
struct attr
{
    min::gen name;
    min::gen value;
    attr ( min::gen name, min::gen value )
        : name ( name ), value ( value ) {}
    attr ( void )
        : name ( min::MISSING() ),
	  value ( min::MISSING() ) {}
};
bool compact
	( ll::parser::parser parser,
	  ll::parser::pass,
	  ll::parser::table::selectors selectors,
	  ll::parser::token & first,
	  ll::parser::token next,
	  min::phrase_position position,
	  min::uns32 m = 0,
	  ll::parser::attr * attributes = NULL,
	  min::uns32 n = 0 );

// Given a min::gen value, return the .initiator
// attribute of that value if it is an object with a
// single .initiator attribute value, or return
// min::MISSING() otherwise.
//
min::gen get_initiator ( min::gen v );

// Given an object vector pointer vp pointing at an
// expression, and an index i of an element in the
// object attribute vector, then if the element is
// a quoted string, increment i by one and call
// ll::lexeme::scan_name_string with the value of
// the quoted string, parser->name_scanner, and the
// other arguments passed to this function, to make
// the quoted string into a label, returning the value
// returned by scan_name_string.  Note that this value
// will be min::ERROR() if an error message is printed
// to parser->name_scanner->printer.
//
// If the element does not exist (i >= size_of ( vp)) or
// is not a quoted string, this function just returns
// min::MISSING().
//
// If parser->name_scanner is NULL_STUB, this function
// initializes it to its default.
//
min::gen scan_name_string_label
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  ll::parser::parser parser,
	  min::uns64 accepted_types,
	  min::uns64 ignored_types,
	  min::uns64 end_types );

// Given a vector pointer vp to an expression, test if
// the expression is a parser definition.  Do nothing
// but return min::FAILURE() if no.  If yes, process
// the definition, and if there is no error return
// min::SUCCESS(), but if there is an error, print an
// error message too parser->printer and return min::
// ERROR().  Note that only expressions that begin with
// `parser' can be parser definitions.
//
min::gen parser_execute_definition
	( min::obj_vec_ptr & vp,
	  ll::parser::parser parser );

} }

# endif // LL_PARSER_H
