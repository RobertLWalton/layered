// Layers Language Parsers
//
// File:	ll_parser.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri Aug 31 23:18:04 EDT 2012
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

    using min::uns8;
    using min::uns16;
    using min::uns32;
    using min::int32;
    using min::uns64;
    using min::NULL_STUB;

    extern min::locatable_gen
        dot_position,	// . position
	dot_initiator,	// . initiator
	dot_terminator,	// . terminator
	dot_separator,	// . separator
	dot_middle,	// . middle
	dot_name,	// . name
	dot_arguments,	// . arguments
	dot_keys,	// . keys
	dot_oper,	// . operator
	doublequote,	// "
	number_sign,	// #
	new_line,	// \n
	semicolon,	// ;
	left_square,    // [
	right_square,   // ]
	comma,		// ,
	parser_lexeme,	// parser
	error_operator,	// ERROR'OPERATOR
	error_operand,	// ERROR'OPERAND
	define,		// define
	undefine,	// undefine
	begin,		// begin
	end,		// end
	print,		// print
	selector,	// selector
	selectors,	// selectors
	with;		// with

    extern min::gen_format name_format;
        // Same as min::default_gen_format except
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
    BRACKETED		= 0xFFFFFFFF,
    BRACKETABLE		= 0xFFFFFFFE,
    OPERATOR		= 0xFFFFFFFD,
    MAX_LEXEME		= 0x7FFFFFFF
};
inline bool is_lexeme ( min::uns32 token_type )
{
    return token_type <= MAX_LEXEME;
}
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
    	//	BRACKETED
	//	BRACKETABLE
	//	    Bracketed expressions have
	//	    .initiator or .terminator
	//	    attributes, and bracketable
	//	    expressions do not.  These
	//	    can be combined with each other.
	//
	// For composites:
	//
	//	OPERATOR

    const min::gen value;
        // Value for some lexeme types, for expressions,
	// and for OPERATORs.  MISSING if no value.

    const ll::parser::string string;
        // Character string for some lexeme types.

    min::phrase_position position;
        // Position of the first character of the token
        // and of the first character AFTER the token,
	// or the end of input.

    min::uns32 indent;
        // Indent of the first character of the token.
	// Used for the first token on a line by the
	// bracketed expression parser.
	//
	// Equal to LEX::AFTER_GRAPHIC if there is a
	// non-whitespace character before the token
	// in the line containing the token.  Otherwise
	// computed by assuming single spaces are 1
	// column wide, tabs are set every 8 columns,
	// and other characters (e.g., form feeds and
	// vertical tabs) take 0 columns.

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

// Pass functions: see below.
//
typedef void ( * run_pass )
        ( ll::parser::parser parser,
          ll::parser::pass pass,
	  ll::parser::table::flags selectors,
    	  ll::parser::token & first,
	  ll::parser::token next );
typedef void ( * init_pass )
        ( ll::parser::parser parser,
	  ll::parser::pass pass );
typedef min::gen ( * execute_pass_definition )
        ( min::obj_vec_ptr & vp,
	  min::phrase_position_vec ppvec,
          ll::parser::parser parser );
typedef min::gen ( * execute_block_end )
        ( min::uns32 block_level,
	  ll::parser::parser parser,
	  ll::parser::pass pass );
typedef min::gen ( * execute_undefine )
        ( min::gen label,
	  const min::phrase_position & position,
          ll::parser::parser parser,
	  ll::parser::pass pass );

struct pass_struct
    // Closure to call to execute a pass on a subexpres-
    // sion.  These closures are organized in a list
    // called the `pass stack'.
    //
    // The parser performs bracketed subexpression
    // recognition and calls the passes in the pass
    // stack for each recognized bracketed subexpres-
    // sion.  However, the first pass is always the 
    // bracketed subexpression recognition pass whose
    // run_pass function is never used (its NULL),
    // but whose other functions are used normally.
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
    // This function is NULL and unused for the first
    // pass, which is always the bracketed subexpression
    // recognition pass.
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

    // Function to execute a parser definition for the
    // pass.  Definition is the object pointed at by
    // vp, whose phrase position vector is ppvec.
    // Returns min::SUCCESS() on success, min::FAILURE()
    // if definition was not recognized, and min::
    // ERROR() if an error message was printed.  NULL if
    // missing.
    //
    ll::parser::execute_pass_definition
        execute_pass_definition;

    // Function to execute the `parser end ...' block
    // end statement.  Simply remove from any symbol
    // table stack all definitions with a block level
    // that is higher than that of the argument.  Called
    // if not NULL.
    //
    ll::parser::execute_block_end execute_block_end;

    // Function to execute the `parser undefine <label>'
    // statment.  Simply add an ll::parser::table::
    // UNDEFINED entry to every symbol table that
    // defines the label.  Use the given phrase_position
    // and parser->block_level.  Called if not NULL.
    //
    ll::parser::execute_undefine execute_undefine;

    ll::parser::table::flags trace;
        // Trace flags that output to parser->printer a
	// description of each selected change made in
	// the parser token list.

    ll::parser::table::flags selectors;
        // Pass is run only if its selectors match those
	// of the bracketed subexpression.
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

    TRACE_PARSER_DEFINITIONS = ( 1 << 2 ),
        // Print parser definitions that have no errors.

    TRACE_BRACKETED_SUBEXPRESSIONS = ( 1 << 3 ),
        // Trace bracketed subexpressions.

    TRACE_OPERATOR_SUBEXPRESSIONS = ( 1 << 4 )
        // Trace operator subexpressions.
};

extern min::locatable_var
	<ll::parser::table::name_table>
    trace_name_table;

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
        // List of passes to call for each bracketed
	// subexpression.  If NULL_STUB there are no
	// passes.  Set to NULL_STUB when parser is
	// created.

    ll::parser::table::flags trace;
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

    min::uns16 block_level;
        // Number of unclosed `parser begin' statements,
	// i.e., current block depth.  Top level is 0.

    const ll::parser::table::name_table
    	    block_name_table;
        // Names of current blocks, in order they were
	// entered.

    const ll::parser::table::name_table
    	    selector_name_table;
        // Selector name table.

    ll::parser::table::flags selectors;
        // Top level selectors.

    // Parser state:

    const ll::parser::token first;
        // First token in token list.  The tokens are a
	// doubly linked list.  NULL_STUB if this list
	// is empty.
	//
	// Tokens are added to the end of the list when
	// the input is called.   Tokens may be dele-
	// ted from the list or replaced in the list.

    uns64 error_count;
        // Number of parser error messages output so
	// far.  To determine if there is an error
	// in the parse of a given expression, check to
	// see if this is incremented.

    uns64 max_error_count;
        // Maximum allowed value of error_count.  When
	// this value is reached, the parser returns,
	// even if it has not yet reached an end of
	// file.  Default 100.

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
MIN_REF ( ll::parser::table::name_table,
		block_name_table,
          ll::parser::parser )
MIN_REF ( ll::parser::table::name_table,
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
void init_ostream
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
// Only lexeme token types with non-MISSING token value
// are considered.  If `next' is NULL_STUB, it is
// assumed that the token list finally ends with an
// end-of-file token, and this cannot be part of any
// hash table entry (because its token value is
// MISSING).
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
	  ll::parser::table::flags selectors,
	  ll::parser::table::table table,
	  ll::parser::token next = NULL_STUB );

// Locate the NEXT hash table entry in the hash table
// after the last entry found by `find_entry' or a
// previous call to this `find_next_entry' function.
//
// Only hash table entries which have a selector bit
// on are considered.  ALL_SELECTORS can be used to
// consider all entries.
//
// The entries are searched in the order longest first,
// and then in newest first order (i.e., most recently
// pushed to the hash table are the newest).
//
// If `shorten' is true, entries with labels of the same
// length as the last entry before this function was
// called are ignored, and current will always be moved
// back one or more tokens.
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
	  ll::parser::table::flags selectors,
	  ll::parser::table::root last_entry,
	  bool shorten = false );

// Put an empty expression token just before a given
// token t on a list of tokens headed by first.  The
// empty expression has no elements and only a .position
// attribute.  The position is just before t.
//
void put_empty_before
	( ll::parser::parser parser,
	  ll::parser::token t );

// Ditto but the empty expression is put just after t
// and the position is set to just after t.
//
void put_empty_after
	( ll::parser::parser parser,
	  ll::parser::token t );

// First, invokes the given pass, if that is not NULL_
// STUB, on the expression consisting of the tokens
// beginning with `first' and ending just before `next'.
//
// Then replaces the expression tokens (which may have
// been changed by the pass) by a resulting expression
// token.  Adds the m attributes whose names and values
// are given, and allows for the latter addition of n
// attributes.  Sets the position of the new expression
// token from the given argument.  The resulting
// expression token is in first == next->previous.
// Its type is given in the type argument, and must
// be either BRACKETED or BRACKETABLE.
//
// Any token in the expression being output that has a
// MISSING token value must be a non-natural number or
// quoted string.  These are replaced by a subexpression
// whose sole element is the token string of the token
// as a string general value and whose .initiator is #
// for a number or " for a quoted string.
//
// An exception to the above is made if the tokens to
// be put in the new expression consist of just a single
// expression token of BRACKETABLE type and the type
// argument is BRACKETED.  Then instead of making a new
// BRACKETED token, this function simply adds the m
// attributes to the BRACKETABLE token which is changed
// to be a BRACKETED token.  The position of this token
// is also reset.
//
// The `trace' argument should be set if the compacted
// expression is to be traced.
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
void compact
	( ll::parser::parser parser,
	  ll::parser::pass,
	  ll::parser::table::flags selectors,
	  min::uns32 type, bool trace,
	  ll::parser::token & first,
	  ll::parser::token next,
	  min::phrase_position position,
	  min::uns32 m = 0,
	  ll::parser::attr * attributes = NULL,
	  min::uns32 n = 0 );

// In a token sequence, find the next token subsequence
// that matches a given separator, or find the end of
// the token sequence.  The separator is represented as
// a vector of n min::gen values, where n == 0 if the
// separator is missing.  `first' is the first token of
// the token sequence and `next' is the next token AFTER
// the token sequence.
//
// The count of the number of tokens skipped is also
// returned.
//
ll::parser::token find_separator
	( min::uns32 & count,
	  ll::parser::token first,
	  ll::parser::token next,
	  min::gen * separator,
	  min::uns32 n );

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

// Skip n tokens.
//
inline ll::parser::token skip
	( ll::parser::token t, min::uns32 n )
{
    while ( n -- ) t = t->next;
    return t;
}

// Remove n tokens from before `next', where n is the
// number of elements of `label' (== 1 if `label' is
// a string or number).  Return the begin position of
// the last token removed.  Free the removed tokens.
//
inline min::position remove
        ( ll::parser::parser parser,
	  ll::parser::token next, min::gen label )
{
    min::position result;
    min::uns32 n = 1;
    if ( min::is_lab ( label ) )
        n = min::lablen ( label );
    while ( n -- )
    {
        result = next->previous->position.begin;
        ll::parser::free
	    ( ll::parser::remove
		  ( ll::parser::first_ref(parser),
		    next->previous ) );
    }
    return result;
}

// Remove from the parser and free the tokens from first
// through the first token before next.  Do nothing if
// first == next.  The `next' token is left untouched.
//
inline void remove
	( ll::parser::parser parser,
	  ll::parser::token first,
	  ll::parser::token next )
{
    while ( first != next )
    {
	first = first->next;
	ll::parser::free
	  ( ll::parser::remove
	      ( first_ref(parser),
		first->previous )
	  );
    }
}

// Return the n'th token before `next', where n is the
// number of elements of `label' (== 1 if `label' is
// a string or number).

inline ll::parser::token backup
        ( ll::parser::token next, min::gen label )
{
    min::uns32 n = 1;
    if ( min::is_lab ( label ) )
        n = min::lablen ( label );
    while ( n -- ) next = next->previous;
    return next;
}

// Convert a non-natural number or quoted string token
// to an expression token.  The expression has as its
// only element a min::gen string value equal to the
// translation string of the token's lexeme, and has as
// its .initiator either # for a non-natural number or
// " for a quoted string.
//
void convert_token ( ll::parser::token token );

// Prints
//
//   ERROR: in <file:line-#> <message1><message2>:
//     <file-lines-with-pp underscored>
//
// and returns min::ERROR().  Uses parser->printer
// and parser->input_file.  Increments parser->error_
// count.
//
min::gen parse_error
	( ll::parser::parser parser,
	  min::phrase_position pp,
	  const char * message1,
	  const char * message2 = "" );

// Ditto but message2 is a printer operation and it is
// followed by message3.  Message2 can be min::pgen...
// for example.
//
min::gen parse_error
	( ll::parser::parser parser,
	  min::phrase_position pp,
	  const char * message1,
	  const min::op & message2,
	  const char * message3 = "" );

// Same as above but produces a warning message and not
// an error message.  Does NOT increment parser->error_
// count.
//
min::gen parse_warn
	( ll::parser::parser parser,
	  min::phrase_position pp,
	  const char * message1,
	  const char * message2 = "" );
min::gen parse_warn
	( ll::parser::parser parser,
	  min::phrase_position pp,
	  const char * message1,
	  const min::op & message2,
	  const char * message3 = "" );

} }

# endif // LL_PARSER_H
