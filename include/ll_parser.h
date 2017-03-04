// Layered Languages Parsers
//
// File:	ll_parser.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Mar  4 18:25:27 EST 2017
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
//	Reformatters
//	Parser Functions

// Usage and Setup
// ----- --- -----

# ifndef LL_PARSER_H
# define LL_PARSER_H

# include <ll_lexeme.h>
# include <ll_lexeme_standard.h>
# include <ll_parser_table.h>

namespace ll { namespace parser {

    using min::uns8;
    using min::uns16;
    using min::uns32;
    using min::int32;
    using min::uns64;
    using min::NULL_STUB;

    extern min::locatable_gen
        top_level,	// TOP LEVEL
	top,		// top
	level,		// level
	dot_oper,	// .operator
	doublequote,	// "
	number_sign,	// #
	new_line,	// \n
	semicolon,	// ;
	left_parenthesis,
			// (
	right_parenthesis,
			// )
	left_square,    // [
	right_square,   // ]
	left_curly,     // }
	right_curly,    // {
	comma,		// ,
	colon,		// :
	equal,		// =
	vbar,		// |
	ealbreak,	// end at line break
	ealeindent,	// end at le indent
	ealtindent,	// end at lt indent
	eapbreak,	// end at paragraph break
	ealsep,		// end at line separator
	eaoclosing,	// end at outer closings
	sticky,		// sticky
	continuing,	// continuing
	other_ea_opt,	// other end at options
	default_opt,	// default options
	other_selectors, // other selectors
	parser_lexeme,	// parser
	data_lexeme,	// data
	prefix_lexeme,	// prefix
	header_lexeme,	// header
	line_lexeme,	// line
	paragraph_lexeme,
			// paragraph
	standard_lexeme,// standard
	error_operator,	// ERROR'OPERATOR
	error_operand,	// ERROR'OPERAND
	define,		// define
	undefine,	// undefine
	test,		// test
	begin,		// begin
	end,		// end
	print,		// print
	block,		// block
	pass_lexeme,	// pass
	selector,	// selector
	selectors,	// selectors
	options,	// options
	group,		// group
	lexical,	// lexical
	master,		// master
	implied,	// implied
	subprefix,	// subprefix
	reformatter_lexeme,
			// reformatter
	default_lexeme,	// default
	with,		// with
	parsing,	// parsing
	after,		// after
	before,		// before
	at,		// at
	plus,		// +
	minus,		// -
	dotdotdot,	// ...
	yes_lexeme,	// yes
	no_lexeme,	// no
	keep_lexeme,	// keep
	enabled_lexeme,	// enabled
	disabled_lexeme,// disabled
	star_parser,    // *PARSER*
	parser_colon,   // *PARSER*:
	parser_test_colon;
			// *PARSER* *TEST*:

    extern min::gen_format name_format;
        // Same as min::default_gen_format except
	// that {str,lab}_{pre,post}fix are all "".

namespace bracketed {

    // We this small piece of ll_parser_bracketed.h.
    //
    struct indentation_mark_struct;
    typedef min::packed_struct_updptr
		<indentation_mark_struct>
	    indentation_mark;
}

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
	( uns32 n, min::ptr<const min::uns32> s );

inline ll::parser::string new_string
	( uns32 n, const uns32 * s )
{
    return ll::parser::new_string
        ( n, min::new_ptr ( s ) );
}

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
    BRACKETING		= 0xFFFFFFFE,
      // Not an actual token type: see `compact'.
    BRACKETABLE		= 0xFFFFFFFD,
    PURELIST		= 0xFFFFFFFC,
    PREFIX		= 0xFFFFFFFB,
    IMPLIED_PREFIX	= 0xFFFFFFFA,
    OPERATOR		= 0xFFFFFFF9,
    DERIVED		= 0xFFFFFFF8,

    TEMPORARY_TT	= 0xFFFFF000,
      // TEMPORARY_TT + n for 0 <= n < 63 may be used
      // by reformatters for temporary token types.
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
	//	BRACKETING
	//	BRACKETABLE
	//	    BRACKETABLE tokens have a MIN object
	//	    value without a .type, .initiator,
	//	    or .terminator.  BRACKETED tokens
	//	    have MIN object values with any
	//	    attributes.  BRACKETING is a pseudo-
	//	    token type passed to the `compact'
	//	    function to indicate that token is
	//	    to be BRACKETED but it has no attri-
	//	    butes but .type, .initiator, and/or
	//	    .terminator, so if it has a single
	//	    BRACETABLE element, it can be merged
	//	    into the value of that element
	//	    instead of becoming a separate
	//	    token.
	//
	//	PURELIST
	//	    PURELIST tokens have no attributes.
	//	    If `compact' is asked to compact a
	//	    list whose only element is a PURE-
	//	    LIST token, it will add attributes
	//	    to that token instead of making
	//	    a new containing list.
	//
	//	PREFIX
	//	    PREFIX tokens are suitable for use
	//	    as prefix separators; their value
	//	    is a MIN object with a .type, no
	//	    .initiator, no .terminator, and
	//	    no elements.  The MIN value may have
	//	    other properties.
	//
	//	IMPLIED_PREFIX
	//	    For use as implied subprefix separa-
	//	    tors.  These are the same as PREFIX
	//	    tokens except: before elements are
	//	    added the token must be upgraded to
	//	    PREFIX by replacing its value by a
	//	    copy of the value; and if there are
	//	    NO elements, the token is to be
	//	    completely deleted as if it never
	//	    existed.
	//
	// For recognized operators:
	//
	//	OPERATOR
	//	    Non-bracketed operators have as
	//	    value the label of operator, and
	//	    bracketed operators have as value
	//	    the MIN object whose .initiator
	//	    indicates it is an operator.
	//
	// For tokens derived from syntax, such as the
	// label value derived from [< x y >]:
	//
	// 	DERIVED
	// 	    The value is derived but is not
	// 	    a MIN object.

    const min::gen value;
        // Value for some lexeme types, and MISSING for
	// others that have strings instead.  See the
	// type above for non-lexeme type values.

    const min::gen value_type;
        // For a BRACKETED token, the .initiator of its
	// MIN value, if any, or the prefix group of its
	// .type, if that is `line' or `paragraph'.  For
	// a PREFIX or IMPLIED_PREFIX token, the .type
	// of its MIN value, if any.  Otherwise MISSING.

    const ll::parser::string string;
        // Character string for lexeme types that have
	// no value.

    min::phrase_position position;
        // Position of the first character of the token
        // and of the first character AFTER the token,
	// or the end of input.

    min::uns32 indent;
        // Indent of the first character AFTER the
	// token, or of the end of input.  Used in the
	// indent_t and indent_before_comment_t tokens
	// to determine line indent.  See ll_lexeme.h
	// for details of how character and end of input
	// indent is computed.

    const ll::parser::token next, previous;
        // Doubly linked list pointers for tokens.
};

MIN_REF ( ll::parser::string, string,
          ll::parser::token )
MIN_REF ( min::gen, value,
          ll::parser::token )
MIN_REF ( min::gen, value_type,
          ll::parser::token )
MIN_REF ( ll::parser::token, next,
          ll::parser::token )
MIN_REF ( ll::parser::token, previous,
          ll::parser::token )

// To avoid garbage collection, tokens are always on one
// of the following lists: free list, allocated list, or
// one of many working lists (e.g., the list of all
// tokens input from a file).  The free and allocated
// lists are hidden, and the working lists are defined
// by the user using a variable of the form
//
//   min::locatable_var<ll::parser::token>
//       working_list_first;
//
// or equivalent pointing to the first element of the
// working list.
//
// Allocation moves a token from the free list to the
// allocated list.  Putting a token on an working list
// moves the token from the allocated list to the
// working list.  Removing a token moves the token from
// a working list to the allocated list.  Freeing the
// token moves the token from the allocated list to the
// free list.  If the free list gets too long, tokens on
// it are removed and forgotten, allowing them to be
// garbage collected.  If the free list gets to short
// the regular memory allocator is called to create a
// token.
//
// To make inline code more efficient by eliminating
// internal checks, the free and allocated lists are
// initialized to non-empty and the first token on
// these lists is never changed or removed.

// Allocate a new token of the given type by moving the
// token from the free list to the allocated list and
// setting its type.  Its value is set to min::MISSING()
// and its string to min::NULL_STUB.
//
ll::parser::token new_token ( uns32 type );

// Free token and return NULL_STUB.  Token is moved from
// the allocated list to the free list after its value
// is set to MISSING and its string is freed and set to
// NULL_STUB.
//
void free ( ll::parser::token token );

// Set the maximum number of tokens on the free list.
// Set to 0 to make list empty.  Set to < 0 if there is
// no limit.  Defaults to 1000.
//
void set_max_token_free_list_size ( int n );

namespace internal {

    // Put a token just before a given token t on a list
    // of tokens.
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

    // Remove a token from the list it is on.
    //
    inline void remove ( ll::parser::token token )
    {
        next_ref(token->previous) = token->next;
	previous_ref(token->next) = token->previous;
    }

    extern min::locatable_var<ll::parser::token>
	allocated_list_first;
}

// Take token from the allocated list and put it just
// before a given token t that is on the working list
// headed by first.  If t is the first token on the
// working list, then `token' becomes the first token
// on the working list.
//
inline void put_before
	( min::ref<ll::parser::token> first,
	  ll::parser::token t,
	  ll::parser::token token )
{
    internal::remove ( token );
    internal::put_before ( t, token );
    if ( first == t ) first = token;
}

// Take token from the allocated list and put it at the
// end of the working list with given first element.  If
// the working list was empty, the token becomes the
// only element of the working list.
//
inline void put_at_end
	( min::ref<ll::parser::token> first,
	  ll::parser::token token )
{
    internal::remove ( token );
    if ( first == min::NULL_STUB )
    {
        first = token;
	previous_ref(token) = next_ref(token) = token;
    }
    else internal::put_before ( first, token );
}

// Remove token from the working list with given first
// token and put the token on the allocated list.
// Return the token removed.
//
inline ll::parser::token remove
	( min::ref<ll::parser::token> first,
	  ll::parser::token token )
{
    if ( token == first )
        first = ( token == token->next ?
	          (ll::parser::token) min::NULL_STUB :
		  token->next );
    internal::remove ( token );
    internal::put_before
        ( internal::allocated_list_first, token );
    return token;
}

// Remove first token from the working list of tokens
// with the given first token, and put the token on
// the allocated list.  Returned the removed token
// (now on the allocated list).  But return min::NULL_
// STUB instead if working list was empty.
//
inline ll::parser::token remove
	( min::ref<ll::parser::token> first )
{
    if ( first == min::NULL_STUB )
        return min::NULL_STUB;
    else
    {
	ll::parser::token token = first;
	first = ( token == token->next ?
		  (ll::parser::token) min::NULL_STUB :
		  token->next );
	ll::parser::internal::remove ( token );
	ll::parser::internal::put_before
	    ( internal::allocated_list_first, token );
	return token;
    }
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
    // parser->trace_flags.
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
    // parser->trace_flags.
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

extern min::locatable_gen PRINTED;
    // Return value for parser_command functions:
    // see below.

namespace pass_function {

    // Function called (if not NULL) when the pass is
    // put on a parser stack.  This function may do
    // things like look up the trace flag of the pass
    // in the parser.
    //
    typedef void ( * place )
	    ( ll::parser::parser parser,
	      ll::parser::pass pass );

    // Function called (if not NULL) when the parser
    // is reset via the `ll::parser::reset' function.
    // This function should reset parameters to default
    // values and remove all symbol table entries with
    // block_level > 0.
    //
    typedef void ( * reset )
	    ( ll::parser::parser parser,
	      ll::parser::pass pass );

    // Function called (if not NULL) at the beginning
    // of a parse function execution.  This function
    // typically does nothing but initialize statistics.
    //
    typedef min::gen ( * begin_parse )
	    ( ll::parser::parser parser,
	      ll::parser::pass pass );

    // Function called to execute the pass to parse a
    // subexpression.
    //
    // The pass is run on the subexpression whose
    // first token is given.  The `next' token is the
    // token immediately after the last token in the
    // subexpression.  The tokens within the subexpres-
    // sion may be edited, and `first' may be reset.
    // If first == end the subexpression is or became
    // empty.
    //
    // Error and warning messages are sent to parser->
    // printer.  Warnings are errors that likely do not
    // affect correctness very much, such as failure to
    // end a file with a line break.  Warnings only
    // print messages if TRACE_WARNINGS is on, which it
    // is by default.  Errors are counted in parser->
    // error_count and warnings in parser->warning_
    // count.  Returns true if no fatal errors, and
    // false if there is a fatal error.
    //
    // After the pass runs on the subexpression, it must
    // call the next pass in the parser pass stack
    // before returning.  This is often done by calling
    // the ll::parser::compact function with pass->next
    // to call the next pass and compact the subexpres-
    // sion.
    //
    // This function is NULL and unused for the first
    // pass, which is always the bracketed subexpression
    // recognition pass.
    //
    typedef void ( * parse )
	    ( ll::parser::parser parser,
	      ll::parser::pass pass,
	      ll::parser::table::flags selectors,
	      ll::parser::token & first,
	      ll::parser::token next );

    // Function called (if not NULL) when at the end of
    // a parse function execution.  This function
    // typically does nothing but print statistics.
    //
    typedef min::gen ( * end_parse )
	    ( ll::parser::parser parser,
	      ll::parser::pass pass );

    // Function called (if not NULL) to execute a parser
    // command, other than `parser begin/end block ...',
    // that may require action by a pass.  Examples are
    // `parser define/undefine ...'.  Vp[i0 ..] contains
    // the command, vp[0 .. i0-1] is ignored, and ppvec
    // is the phrase position vector of vp.
    //
    // Return min::SUCCESS() on success if the command
    // was not printed, ll::parser::PRINTED on success
    // if the command printed itself (along with other
    // things), min::FAILURE() if command was not
    // recognized, and min::ERROR() if an error message
    // (including the command) was printed.  Increment
    // parser->warning/error_count on printing an error/
    // warning message.
    //
    typedef min::gen ( * parser_command )
	    ( ll::parser::parser parser,
	      ll::parser::pass pass,
	      min::obj_vec_ptr & vp, min::uns32 i0,
	      min::phrase_position_vec ppvec );

    // Function called (if not NULL) by `parser begin
    // <name>' AFTER parser block_level has been
    // incremented.  Normally there is nothing to do.
    // Position is the position of the parser command,
    // and name is the block name.
    //
    typedef min::gen ( * begin_block )
	    ( ll::parser::parser parser,
	      ll::parser::pass pass,
	      const min::phrase_position & position,
	      min::gen name );

    // Ditto but called by `parser end <name>' BEFORE
    // parser block_level has been decremented.  This
    // function should remove all pass symbol table
    // entries with block level == parser block_level.
    //
    typedef min::gen ( * end_block )
	    ( ll::parser::parser parser,
	      ll::parser::pass pass,
	      const min::phrase_position & position,
	      min::gen name );
}

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

    const min::gen name;
        // Pass name as per new pass table.

    const ll::parser::pass next;
        // Next pass in the pass stack, or NULL_STUB.
	// The parser executes passes in `next-later'
	// order on a subexpression.

    const ll::parser::parser parser;
        // Parser whose pass stack this pass is on,
	// or NULL_STUB if the pass is not on any
	// pass stack.

    ll::parser::table::flags selectors;
        // Pass is run only if its selectors match those
	// of the bracketed subexpression.

    ll::parser::pass_function::place place;
    ll::parser::pass_function::reset reset;
    ll::parser::pass_function::begin_parse begin_parse;
    ll::parser::pass_function::parse parse;
    ll::parser::pass_function::end_parse end_parse;
    ll::parser::pass_function::parser_command
        parser_command;
    ll::parser::pass_function::begin_block begin_block;
    ll::parser::pass_function::end_block end_block;
};

MIN_REF ( min::gen, name, ll::parser::pass )
MIN_REF ( ll::parser::parser, parser, ll::parser::pass )
MIN_REF ( ll::parser::pass, next, ll::parser::pass )

// Execute pass->parse if pass != NULL_STUB and pass->
// selectors & selectors is not 0, and otherwise if
// pass != NULL_STUB repeat with pass = pass->next.
//
inline void execute_pass_parse
	( ll::parser::parser parser,
	  ll::parser::pass pass,
	  ll::parser::table::flags selectors,
	  ll::parser::token & first,
	  ll::parser::token next )
{
    while ( pass != min::NULL_STUB )
    {
        if ( ( pass->selectors & selectors ) != 0 )
	{
	    (* pass->parse )
		 ( parser, pass, selectors,
		           first, next );
	    break;
	}
	pass = pass->next;
    }
}

// Place `pass' on the parser->pass_stack just after
// `previous', which must be a pass on parser->pass_
// stack.  `pass' is first removed from any parser pass
// stack it was previously on.
//
void place_after
	( ll::parser::parser parser,
	  ll::parser::pass pass,
	  ll::parser::pass previous );

// Place `pass' on the parser->pass_stack just before
// `next', or if the latter is NULL_STUB, put `pass'
// at the end of the stack.  `pass' is first removed
// from any parser pass stack it was previously on.
// `next' cannot be the top of the stack.
//
void place_before
	( ll::parser::parser parser,
	  ll::parser::pass pass,
	  ll::parser::pass next = NULL_STUB );

// Remove `pass' from the pass stack of `pass->parser'
// if the latter is not NULL_STUB.
//
void remove ( ll::parser::pass pass );

// Return the pass with the given name on the parser's
// pass stack.  Return NULL_STUB if none.
//
ll::parser::pass find_on_pass_stack
	( ll::parser::parser parser, min::gen name );

// There is a single parser new pass table (not the same
// as a parser pass stack) set up by program initializa-
// tion.  This maps parser pass names to `new_pass'
// functions that create passes.

typedef ll::parser::pass ( * new_pass )
	( ll::parser::parser parser );

struct new_pass_table_struct
{
    min::gen name;
    ll::parser::new_pass new_pass;
};

struct new_pass_table_struct;
typedef min::packed_vec_insptr
	    <new_pass_table_struct>
	new_pass_table_type;
extern const uns32 & NEW_PASS_TABLE_TYPE;
    // Subtype of min::packed_vec
    //                <new_pass_table_struct>.

// The one and only parser new pass table.
//
extern min::locatable_var
	<ll::parser::new_pass_table_type>
    new_pass_table;

// Look up parser pass name in parser new pass table,
// and return parser new_pass function found, or NULL
// if none found.
//
inline ll::parser::new_pass find_new_pass
	( min::gen name )
{
    ll::parser::new_pass_table_type t =
        ll::parser::new_pass_table;
    for ( min::uns32 i = 0; i < t->length; ++ i )
    {
        if ( (&t[i])->name == name )
	    return (&t[i])->new_pass;
    }
    return NULL;
}

// Push parser pass name and parser new_pass function
// into parser pass table.
//
void push_new_pass ( min::gen name,
                     ll::parser::new_pass new_pass );

} }

// Parser
// ------

namespace ll { namespace parser {

enum {
    // Built in trace flags that have compiled offsets
    // in parser->trace_flags.  Other trace flags are
    // assigned offsets dynamically.

    // Global flags.
    //
    TRACE_WARNINGS			= 1ull << 0,
        // Print warning messages.

    TRACE_PARSER_INPUT			= 1ull << 1,
        // Trace flag with name `parser input' that
	// traces each token input by the parser->input
	// closure (e.g. from the lexical scanner).  

    TRACE_PARSER_OUTPUT			= 1ull << 2,
        // Trace flag with the name `parser output' that
	// traces processing by the parser->output
	// closure.

    TRACE_PARSER_COMMANDS		= 1ull << 3,
        // Trace flag with the name `parser commands'
	// that traces all parser commands that have no
	// errors.

    // Flags that modify the tracing of passes.
    //
    TRACE_SUBEXPRESSION_ELEMENTS	= 1ull << 4,
	// Trace flag with name `subexpression elements'
	// that causes subexpression values to be print-
	// ed with parser->subexpression_gen_format.
    TRACE_SUBEXPRESSION_DETAILS		= 1ull << 5,
	// Trace flag with name `subexpression details'
	// that causes subexpression values to be
	// printed with min::print_mapped.
    TRACE_SUBEXPRESSION_LINES		= 1ull << 6,
	// Trace flag with name `subexpression lines'
	// that causes subexpression values to be
	// printed with min::print_phrase_lines.  If
	// no TRACE_SUBEXPRESSION_XXX flag is on, this
	// is assumed as a default.
    TRACE_KEYS				= 1ull << 7,
	// Trace flag with name `keys' that causes
	// the finding or rejecting of sequences of
	// tokens (called keys) in the symbol tables
	// to be printed.

    TRACE_FLAGS				= 8
        // Total number of built in trace flags.
};

enum {
    // Built in selectors that have compiled values.
    // Other selectors are assigned values dynamically.
    //
    // Values are hidden and may be changed.

    // Options:
    //
    ALL_OPT       			= 0xFFFFull,

    EALBREAK_OPT			= 1ull << 0,
    EALEINDENT_OPT			= 1ull << 1,
    EALTINDENT_OPT			= 1ull << 2,
    EAPBREAK_OPT			= 1ull << 3,
    EALSEP_OPT				= 1ull << 4,
    EAOCLOSING_OPT			= 1ull << 5,
    STICKY_OPT				= 1ull << 6,
    CONTINUING_OPT			= 1ull << 7,

    ALL_EA_OPT    = EALBREAK_OPT
	          + EALEINDENT_OPT
	          + EALTINDENT_OPT
	          + EAPBREAK_OPT
	          + EALSEP_OPT
	          + EAOCLOSING_OPT,

    ALL_BRACKET_OPT = ALL_EA_OPT
		    - EALSEP_OPT,

    ALL_PREFIX_OPT = ALL_EA_OPT
                   + STICKY_OPT
		   + CONTINUING_OPT,

    DEFAULT_EA_OPT = EALEINDENT_OPT
	           + EALSEP_OPT
	           + EAOCLOSING_OPT,

    DEFAULT_OPT = DEFAULT_EA_OPT,

    // Selectors:
    //
    ALL_SELECTORS 			= -1ull << 16,

    ALWAYS_SELECTOR			= 1ull << 16,
        // Hidden selector that is always on.

    // Builtin selectors:
    //
    TOP_LEVEL_SELECTOR			= 1ull << 17,
    DATA_SELECTOR			= 1ull << 18,

    COMMAND_SELECTORS = ALL_SELECTORS
                      - ALWAYS_SELECTOR
};

const uns32 NO_LINE_INDENT = 0xFFFFFFFF;
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
	//
	// Also see TRACE_PARSER_INPUT trace flag.

    const ll::parser::output output;
        // Closure to call to remove finished tokens
	// from the beginning of the token list.  May
	// be NULL_STUB if this is not to be done (the
	// finished tokens are left in the list when
	// `parse' returns).  Set to NULL_STUB when
	// parser is created.
	//
	// Also see TRACE_PARSER_OUTPUT trace flag.

    const ll::parser::pass pass_stack;
        // List of passes to call for each bracketed
	// subexpression.  If NULL_STUB there are no
	// passes.  Set to NULL_STUB when parser is
	// created.

    ll::parser::table::flags trace_flags;
        // Parser trace flags: see above.  Tracing is
	// done to parser->printer.

    const min::gen_format * subexpression_gen_format;
        // min::gen_format used to print subexpressions
	// when the TRACE_SUBEXPRESSIONS_ELEMENTS flag
	// is in trace_flags.  Defaults to min::line_
	// gen_format.

    const ll::parser::table::name_table
    	    trace_flag_name_table;
        // Trace flag names.

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
	// parser commands.  Defaults to a scanner
	// with the same program and printer as the main
	// parser scanner.

    const ll::parser::table::undefined_stack
    	    undefined_stack;
	// Undefined records that record ll::parser::
	// table::root selectors replaced by undefine
	// commands.

    const ll::parser::table::block_stack
    	    block_stack;
        // Information about blocks.  length is the
	// parser `block_level', i.e., the level of
	// the current block.

    const ll::parser::table::name_table
    	    selector_name_table;
    const ll::parser::table::key_table
            selector_group_name_table;
        // Selector name and group tables.

    ll::parser::table::flags selectors;
    const min::gen prefix_separator;
        // Top level selectors and prefix_separator.

    const ll::parser::bracketed::indentation_mark
	    top_level_indentation_mark;
	// Top level indentation mark for parse
	// function.

    // Parser state:

    const ll::parser::token first;
        // First token in token list.  The tokens are a
	// doubly linked list.  NULL_STUB if this list
	// is empty.
	//
	// Tokens are added to the end of the list when
	// the input is called.   Tokens may be dele-
	// ted from the list or replaced in the list.

    bool at_paragraph_beginning;
        // Set true when parser initialized, when
	// a blank line is encountered, or at the
	// beginning of an indented paragraph.  Set
	// false by the first token of a non-comment,
	// non-blank line.  Unchanged by comment lines.

    uns64 error_count;
    uns64 warning_count;
        // Number of parser error/warning messages
	// output so far.  To determine if there is an
	// error/warning in the parse of a given expres-
	// sion, check to see if error/warning_count is
	// incremented.  Warning_count is incremented
	// even if TRACE_WARNING is off so that warning
	// messages are not printed.

    uns64 max_error_count;
        // Maximum allowed value of error_count.  When
	// this value is reached, the parser returns,
	// even if it has not yet reached an end of
	// file.  Default 100.

    min::phrase_position message_header;
        // Position in input_file of lines that are to
	// be printed using min::print_phrase_lines
	// before any error or warning message, unless
	// message_header.begin equals MISSING_POSITION.
	// When these lines are printed, message_header
	// .begin is set to MISSING_POSITION.

    bool eof;
        // True if `input' has delivered an end-of-file
	// token to the end of the token list, so
	// `input' should not be called again.

    uns32 finished_tokens;
        // Number of finished tokens at the beginning
	// of the token list.  The `parse' function
	// produces finished tokens and calls `output'.
};

inline min::uns32 block_level
	( ll::parser::parser parser )
{
    return parser->block_stack->length;
}
inline min::gen block_name
	( ll::parser::parser parser,
	  min::uns32 block_level )
{
    MIN_ASSERT
        ( block_level <= parser->block_stack->length,
	  "block_level argument too large" );
    return block_level == 0 ?
           (min::gen) ll::parser::top_level :
	   (&parser->block_stack[block_level-1])->name;
}

MIN_REF ( ll::parser::input, input,
          ll::parser::parser )
MIN_REF ( ll::parser::output, output,
          ll::parser::parser )
MIN_REF ( ll::parser::pass, pass_stack,
          ll::parser::parser )
MIN_REF ( ll::parser::table::name_table,
		trace_flag_name_table,
          ll::parser::parser )
MIN_REF ( ll::lexeme::scanner, scanner,
          ll::parser::parser )
MIN_REF ( min::file, input_file,
          ll::parser::parser )
MIN_REF ( min::printer, printer,
          ll::parser::parser )
MIN_REF ( ll::lexeme::scanner, name_scanner,
          ll::parser::parser )
MIN_REF ( ll::parser::table::undefined_stack,
		undefined_stack,
          ll::parser::parser )
MIN_REF ( ll::parser::table::block_stack,
		block_stack,
          ll::parser::parser )
MIN_REF ( ll::parser::table::name_table,
		selector_name_table,
          ll::parser::parser )
MIN_REF ( ll::parser::table::key_table,
		selector_group_name_table,
          ll::parser::parser )
MIN_REF ( min::gen, prefix_separator,
          ll::parser::parser )
MIN_REF ( ll::parser::bracketed::indentation_mark,
		top_level_indentation_mark,
          ll::parser::parser )
MIN_REF ( ll::parser::token, first,
          ll::parser::parser )

extern min::locatable_var<ll::parser::parser>
       default_parser;

extern min::phrase_position top_level_position;

// This `init' function creates a parser and stores a
// pointer to it in the argument variable, if the
// variable's previous value is NULL_STUB.  The variable
// MUST BE locatable by the garbage collector.
//
// When a parser is created, bracketed and prefix passes
// are created for the parser, with the prefix pass
// being placed immediately after the bracketed pass
// in the parser pass stack.
//
// When the parser is created, if define_standard is
// true, a `standard' parser block is begun with the
// standard brackets, operators, etc.  Setting define_
// standard to false is used only for testing.
//
void init ( min::ref<ll::parser::parser> parser,
            bool define_standard = true );

// This `reset' function restores parser parameters to
// the values they had when the parser was first
// created.
//
void reset ( min::ref<ll::parser::parser> parser );

// Initialize parser if necessary and set a parameter.
// Some parameters are copied to both parser and any
// scanner that exists for the parser.
//
inline void init_line_display
	( min::ref<ll::parser::parser> parser,
	  min::uns32 line_display )
{
    init ( parser );
    min::init_line_display
        ( input_file_ref(parser),
	  line_display );
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
	  min::uns32 line_display = 0,
	  min::uns32 spool_lines = min::ALL_LINES );
void init_input_file
	( min::ref<ll::parser::parser> parser,
	  min::file ifile,
	  min::uns32 line_display = 0,
	  min::uns32 spool_lines = min::ALL_LINES );
bool init_input_named_file
	( min::ref<ll::parser::parser> parser,
	  min::gen file_name,
	  min::uns32 line_display = 0,
	  min::uns32 spool_lines = min::ALL_LINES );
void init_input_string
	( min::ref<ll::parser::parser> parser,
	  min::ptr<const char> data,
	  min::uns32 line_display = 0,
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

// Reformatters
// ------------

// There are several reformatter stacks set up by pro-
// gram initialization which list all the reformatters.
// For example, there is one stack for operator refor-
// matters and one for bracket reformatters.
//
// A reformatter and a vector of reformatter arguments
// may be associated with an operator, bracket pair,
// or other syntactic construct.

typedef bool ( * reformatter_function )
        ( ll::parser::parser parser,
	  ll::parser::pass pass,
	  ll::parser::table::flags selectors,
	  ll::parser::token & first,
	  ll::parser::token next,
	  const min::phrase_position & position,
	  ll::parser::table::flags trace_flags,
	  ll::parser::table::root entry );
    //
    // A reformatter_function reformats the tokens from
    // first to next->previous.  Trace_flags are passed
    // to `compact', if the function calls that.
    // `position' is the position of the tokens.  Pass
    // is the reformatting pass (pass->next is passed
    // to `compact' if that is called), and parser and
    // selectors are the current parser and selectors.
    // Entry is the key table entry that contained the
    // reformatter being called.  This entry contains
    // various kinds of reformatter arguments.
    //
    // The function may change `first'.  Note that if
    // this is done, a recalculated position would be
    // incorrect, which is why `position' is calculated
    // before the function is called.
    //
    // If true is returned, the caller of the function
    // will immediately call ll:: parser::compact (even
    // if first->next == next).  For the operator pass,
    // this call makes a BRACKETABLE token with no
    // syntax attributes, but with the given `position'
    // as the .position attribute and with the value
    // of the original first operator token as the
    // .operator attribute.  For the bracketed pass,
    // this call is a BRACKETING call that makes a
    // BRACKETED token with the brackets as .initiator
    // and .terminator, but with NO .type attribute.  In
    // all cases parser, pass->next, selectors, first,
    // next, position, and trace_flags are passed to the
    // `compact' function.
    //
    // For the bracketed pass, the list of tokens passed
    // to the reformatted does NOT include the brackets,
    // but the position does include them.

typedef min::packed_vec_ptr<min::gen>
	reformatter_arguments;
    // Argument list for a reformatter.  Contained in
    // the key table entry that specifies the
    // reformatter.

struct reformatter_struct;
typedef min::packed_struct_updptr<reformatter_struct>
        reformatter;
extern const uns32 & REFORMATTER;
    // Subtype of
    // min::packed_struct<reformatter_struct>.
struct reformatter_struct
{
    min::uns32 control;

    ll::parser::reformatter next;
    min::gen name;
    min::uns32 flags;
        // Used by some passes to check the legality
	// of associating a reformatter with a bracket
	// or operator or whatever.  For the operator
	// pass, these are operator flags (PREFIX,
	// INFIX, etc.) such that an operator may not
	// associate with a reformatter that does not
	// have one of the operator's flags.
    min::uns32 minimum_arguments, maximum_arguments;
        // Minimum and maximum number of arguments.
    ll::parser::reformatter_function
        reformatter_function;
};
MIN_REF ( ll::parser::reformatter, next,
          ll::parser::reformatter )
MIN_REF ( min::gen, name,
          ll::parser::reformatter )

// Look up reformatter name in reformatter stack, and
// return reformatter if found, or NULL_STUB if not
// found.
//
inline ll::parser::reformatter find_reformatter
	( min::gen name,
	  ll::parser::reformatter stack )
{
    ll::parser::reformatter r = stack;
    while ( r != min::NULL_STUB )
    {
        if ( r->name == name ) return r;
	r = r->next;
    }
    return min::NULL_STUB;
}

// Push a new reformatter into the reformatter stack.
//
void push_reformatter
    ( min::gen name,
      min::uns32 flags,
      min::uns32 minimum_arguments, 
      min::uns32 maximum_arguments,
      ll::parser::reformatter_function
          reformatter_function,
      min::ref<ll::parser::reformatter> stack );

// Parser Functions
// ------ ---------

// Compute new_flags that set the selectors to the
// selectors argument, turning on these and the ALWAYS_
// SELECTOR and turning off all other selectors.
//
inline ll::parser::table::new_flags parsing_selectors
        ( ll::parser::table::flags selectors )
{
    return ll::parser::table::new_flags
        (   selectors
	  | ll::parser::ALWAYS_SELECTOR,
	    ll::parser::COMMAND_SELECTORS
	  & ~ selectors, 0 );
}

// Ditto for options.
//
inline ll::parser::table::new_flags parsing_options
        ( ll::parser::table::flags options )
{
    return ll::parser::table::new_flags
        ( options, ll::parser::ALL_OPT & ~ options, 0 );
}

// Begin/end a parser block with the given name.  Return
// min::SUCCESS() on success and min::ERROR() if an
// error message was written to parser->printer (see
// ll::parser::parse_error below).  The position should
// be the position of the entire parser command.  End_
// block checks that `name' matches the name of the
// block being ended.
//
min::gen begin_block
    ( ll::parser::parser parser, min::gen name,
      const min::phrase_position & position );
min::gen end_block
    ( ll::parser::parser parser, min::gen name,
      const min::phrase_position & position );

// Locate the key prefix in the key table that
// corresponds to the longest available string of tokens
// beginning with `current'.  If `next' is NULL_STUB,
// tokens are added to the token list as necessary, but
// otherwise only tokens before `next' are considered.
// Only lexeme token types with non-MISSING token value
// are considered.  If `next' is NULL_STUB, it is
// assumed that the token list finally ends with an
// end-of-file token, and this cannot be part of any
// key table entry (because its token value is
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
	  ll::parser::table::key_table key_table,
	  ll::parser::token next = NULL_STUB );

// Locate the key table entry in the key table that
// corresponds to the longest available string of tokens
// beginning with `current'.  If `next' is NULL_STUB,
// tokens are added to the token list as necessary, but
// otherwise only tokens before `next' are considered.
// Only tokens with a non-MISSING token value are
// considered.  If `next' is NULL_STUB, it is assumed
// that the token list finally ends with an end-of-file
// token, and this cannot be part of any key table
// entry (because its token value is MISSING).
//
// Only key table hash entries which have a selector bit
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
	  ll::parser::table::key_table key_table,
	  ll::parser::token next = NULL_STUB );

// Locate the NEXT hash entry in the key table after the
// last entry found by `find_entry' or a previous call
// to this `find_next_entry' function.
//
// Only hash entries which have a selector bit on are
// considered.  ALWAYS_SELECTOR can be used to consider
// all entries.
//
// The entries are searched in the order longest first,
// and then in newest first order (i.e., most recently
// pushed to the key table are the newest).
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
// attribute, and is PURELIST.  The position is just
// before t.
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

// Ditto but put ERROR'OPERAND or ERROR'OPERATOR before
// or after.
//
void put_error_operand_before
	( ll::parser::parser parser,
	  ll::parser::token t );
void put_error_operand_after
	( ll::parser::parser parser,
	  ll::parser::token t );
void put_error_operator_before
	( ll::parser::parser parser,
	  ll::parser::token t );
void put_error_operator_after
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
// be either BRACKETED, or BRACKETABLE.  The type argu-
// ment can also be BRACKETING as a special case: see
// below.  Note that the type argument should never be
// PURELIST.
//
// If the type argument is BRACKETABLE, the m attributes
// MUST NOT include any .type, .initiator, or .termina-
// tor attributes.
//
// Any token in the expression being output that has a
// MISSING token value must be a non-natural number or
// quoted string.  These are replaced by a subexpression
// whose sole element is the token string of the token
// as a string general value and whose .initiator is #
// for a number or " for a quoted string.
//
// Space is allocated in the new object for the m
// attributes, 1 .position attribute, and n attributes
// to be added later.
//
// The type argument is changed to PURELIST if m == 0
// and otherwise to BRACKETED if the type argument is
// as BRACKETING.
//
// An exception to the above is made if the tokens to
// be put in the new expression consist of just a single
// token, m != 0, and either the single token is of
// PURELIST type, or the single token is of BRACKETABLE
// type and the type argument is BRACKETING.  Then
// instead of making a new token, this function simply
// adds the m attributes to the PURELIST or BRACKETABLE
// token, whose type is changed to BRACKETED.  The
// positions of this token and of its MIN object value
// are also reset to the position argument.
//
// If the type argument is BRACKETING, the m attributes
// can only include .type, .initiator, and/or .termina-
// tor attributes.
//
// The trace_subexpression function is called with the
// final output subexpression token to process any
// TRACE_SUBEXPRESSION_... trace_flags.
//
struct attr
{
    min::gen name;
    min::gen value;
    min::gen multivalue;
    min::gen flags;
    attr ( min::gen name, min::gen value )
        : name ( name ), value ( value ),
	  multivalue ( min::MISSING() ),
	  flags ( min::MISSING() ) {}
    attr ( void )
        : name ( min::MISSING() ),
	  value ( min::MISSING() ),
	  multivalue ( min::MISSING() ),
	  flags ( min::MISSING() ) {}
};
void compact
	( ll::parser::parser parser,
	  ll::parser::pass,
	  ll::parser::table::flags selectors,
	  ll::parser::token & first,
	  ll::parser::token next,
	  const min::phrase_position position,
	  ll::parser::table::flags trace_flags,
	  min::uns32 type,
	  min::uns32 m = 0,
	  ll::parser::attr * attributes = NULL,
	  min::uns32 n = 0 );

// Compact logical line, giving it the min::LOGICAL_LINE
// .initiator and either the "<NL>" .terminator if
// separator_found is false, or the line_sep->label
// .terminator otherwise.
//
void compact_logical_line
	( ll::parser::parser parser,
	  ll::parser::pass,
	  ll::parser::table::flags selectors,
	  ll::parser::token & first,
	  ll::parser::token next,
	  const min::position & separator_found,
	  ll::parser::table::root line_sep,
	  ll::parser::table::flags trace_flags );

// Given an expression beginning with first and ending
// just before next, in which the first token is a
// PREFIX token, add the non-first tokens to the first
// token value as elements.  Execute pass->next on
// the list of non-first elements before doing this, and
// then convert any non-first element tokens with
// strings to tokens with values of .type <Q> or #.
// The end position of the expanded PREFIX token is
// updated to equal the end position of next->previous,
// and the type of the PREFIX token is changed to
// BRACKETED.  Lastly non-first element tokens are
// removed, but both the first and the next tokens are
// not.
//
// If on the other hand the first token is an IMPLIED_
// PREFIX token, then if there are some elements or
// separator_found is true, the value of this token is
// replaced by a copy before it is used, but if there
// are no elements and separator_found is false, the
// first token is deleted and nothing else is done.
//
// If separator_found is true, set the .terminator
// attribute of the expanded PREFIX to line_sep->label
// and update the end position of the expanded PREFIX
// token to separator_found.
//
// Returns false if first token was deleted and true if
// first token was remade into a BRACKETED token.
//
bool compact_prefix_separator
	( ll::parser::parser parser,
	  ll::parser::pass pass,
	  ll::parser::table::flags selectors,
	  ll::parser::token first,
	  ll::parser::token next,
	  const min::position & separator_found,
	  ll::parser::table::root line_sep,
	  ll::parser::table::flags trace_flags );

// Compact a paragraph after each line has been parsed.
// The first token value has a .type with `paragraph'
// group, and 0 or 1 elements.  The element, if it
// exists, and the other tokens, have .type with `line'
// group or are logical lines with LOGICAL_LINE
// .initiator.  All token values but the first are
// appended to the vector of elements of the first
// token.
//
void compact_paragraph
	( ll::parser::parser parser,
	  ll::parser::token & first,
	  ll::parser::token next,
	  ll::parser::table::flags trace_flags );

// The following function prints TRACE_SUBEXPRESSION_...
// tracing information for a token that has just been
// created by the `compact' function or a reformatter
// or is about to be output.
//
namespace internal {
    void trace_subexpression
	( ll::parser::parser parser,
	  ll::parser::token token,
	  min::uns32 trace_flags );
}
inline void trace_subexpression
	( ll::parser::parser parser,
	  ll::parser::token token,
	  min::uns32 trace_flags )
{
    trace_flags &=
        (   ll::parser::TRACE_SUBEXPRESSION_ELEMENTS
          + ll::parser::TRACE_SUBEXPRESSION_DETAILS
          + ll::parser::TRACE_SUBEXPRESSION_LINES );

    if ( trace_flags )
        internal::trace_subexpression
	    ( parser, token, trace_flags );
}

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

// Return true iff v is an object with no elements and
// a .type attribute that is a min string or label.
//
bool is_prefix_separator ( min::gen v );

// Scan type masks:
//
const min::uns64 QUOTED_KEY_SCAN_MASK =
      ( 1ull << ll::lexeme::standard::mark_t )
    + ( 1ull << ll::lexeme::standard::separator_t )
    + ( 1ull << ll::lexeme::standard::word_t )
    + ( 1ull << ll::lexeme::standard::natural_t );
const min::uns64 IGNORED_SCAN_MASK =
      ( 1ull << ll::lexeme::standard
                          ::horizontal_space_t )
    + ( 1ull << ll::lexeme::standard
                          ::indent_before_comment_t )
    + ( 1ull << ll::lexeme::standard::indent_t )
    + ( 1ull << ll::lexeme::standard
                          ::premature_end_of_file_t )
    + ( 1ull << ll::lexeme::standard::start_of_file_t )
    + ( 1ull << ll::lexeme::standard::end_of_file_t );
const min::uns64 END_SCAN_MASK =
      ( 1ull << ll::lexeme::standard
                          ::premature_end_of_file_t )
    + ( 1ull << ll::lexeme::standard::end_of_file_t );

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
      min::uns64 end_types,
      bool empty_name_ok = false );

// Ditto with scan types set for quoted key.  In
// addition, prints `badly formed quoted key' error
// message when returning min::ERROR().  Returns
// min::MISSING() if quoted key missing but not in
// error.
//
min::gen scan_quoted_key
    ( min::obj_vec_ptr & vp, min::uns32 & i,
      ll::parser::parser parser,
      bool empty_name_ok = false );

// Given an object vector pointer vp pointing at an
// expression, and an index i of an element in the
// vector, then if vp[i] is a word not equal to
// end_value, increment i until i >= size of vp or vp[i]
// is not a word or number or is equal to end_value.
//
// If i has been incremented at least once, make and
// return a label from the elements scanned over.  If
// there is only 1 element, return just that element.
// If there is more than one, return the MIN label
// containing the elements.  If there are no elements,
// return min::MISSING().
//
min::gen scan_simple_name
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  min::gen end_value = min::MISSING() );

// If vp[i] is a quoted string, scan it as per scan_
// quoted_key and increment i by 1.  Otherwise call
// scan_simple_name as above.  Returns min::ERROR()
// if error message printed.  It is an error if
// vp[i] is not a quoted string and does not begin
// a simple name.  min::MISSING() is NEVER returned.
//
min::gen scan_name
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  ll::parser::parser parser,
	  min::gen end_value = min::MISSING() );

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

// Move the tokens from `first' through the first token
// before `next' to just before `after'.  Do nothing if
// first == next.  The `after' token is left untouched.
//
// If after == parser->first, the latter is changed to
// equal `first' (if first != next).
//
// It is required, but not checked, that `after' NOT be
// one of the tokens being moved.  It is also required
// but not checked that parser->first NOT be one of
// the tokens being moved.
//
inline void move_to_before
	( ll::parser::parser parser,
	  ll::parser::token after,
	  ll::parser::token first,
	  ll::parser::token next )
{
    if ( first == next ) return;

    // The following protects the tokens being moved
    // from the garbage collector and remembers
    // the last token being moved.
    //
    min::locatable_var<ll::parser::token> last
    	( next->previous );

    next_ref(first->previous) = next;
    previous_ref(next) = first->previous;

    previous_ref(first) = after->previous;
    next_ref(after->previous) = first;

    next_ref(last) = after;
    previous_ref(after) = last;

    MIN_REQUIRE ( parser->first != first );
    if ( after == parser->first )
        ll::parser::first_ref(parser) = first;
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
// its .type either # for a non-natural number or " for
// a quoted string.
//
void convert_token ( ll::parser::token token );

// First prints the lines delimited by parser->message_
// header if parser->message_header.begin is not
// MISSING_POSITION, and if it prints these lines, sets
// parser->message_header.begin to MISSING_POSITION.
//
// Then prints
//
//   ERROR: in <file:line-#> <message1><message2>...:
//     <file-lines-with-pp underscored>
//
// and returns min::ERROR().
//
// Increments parser->error_count.
//
// Printing lines is done with min::print_phrase_lines,
// parser->printer and parser->input_file.
//
min::gen parse_error
	( ll::parser::parser parser,
	  const min::phrase_position & pp,
	  const char * message1,
	  const min::op & message2 = min::pnop,
	  const char * message3 = "",
	  const min::op & message4 = min::pnop,
	  const char * message5 = "",
	  const min::op & message6 = min::pnop,
	  const char * message7 = "",
	  const min::op & message8 = min::pnop,
	  const char * message9 = "" );

// Same as above but produces a warning message and not
// an error message, but only if parser TRACE_WARNINGS
// flag is on.  Increments parser->warning_count evern
// if TRACE_WARNINGS is off.
//
void parse_warn
	( ll::parser::parser parser,
	  const min::phrase_position & pp,
	  const char * message1,
	  const min::op & message2 = min::pnop,
	  const char * message3 = "",
	  const min::op & message4 = min::pnop,
	  const char * message5 = "",
	  const min::op & message6 = min::pnop,
	  const char * message7 = "",
	  const min::op & message8 = min::pnop,
	  const char * message9 = "" );

} }

# endif // LL_PARSER_H
