// Layered Languages Parsers
//
// File:	ll_parser.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu Jul 28 17:17:00 EDT 2022
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
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
# include <cmath>

namespace ll { namespace parser {

    using min::uns8;
    using min::uns16;
    using min::uns32;
    using min::int32;
    using min::uns64;
    using min::NULL_STUB;
    using min::Uchar;

    namespace lexeme {

	extern min::locatable_gen
	    NONE,		// NONE
	    top_level,		// TOP LEVEL
	    line_level,		// LINE LEVEL
	    top,		// top
	    level,		// level
	    star_top_level_star, // *TOP* *LEVEL*
	    dot_oper,		// .operator
	    doublequote,	// "
	    number_sign,	// #
	    new_line,		// \n
	    semicolon,		// ;
	    left_parenthesis,	// (
	    right_parenthesis,	// )
	    left_square,    	// [
	    right_square,   	// ]
	    left_curly,     	// }
	    right_curly,    	// {
	    comma,		// ,
	    colon,		// :
	    double_colon,	// ::
	    equal,		// =
	    vbar,		// |
	    eaindent,		// end at indent
	    ealeindent,		// end at le indent
	    ealtindent,		// end at lt indent
	    eapbreak,		// end at paragraph
	    			//     break
	    ealsep,		// end at line separator
	    eaoclosing,		// end at outer closings
	    eiparagraph,	// enable indented
	    			//        paragraph
	    eprefix,		// enable prefix
	    etprefix,		// enable table prefix
	    eheader,		// enable header
	    sticky,		// sticky
	    reset,		// reset
	    continuing,		// continuing
	    default_opt,	// default options
	    other_ea_opt,	// other end at options
	    default_ea_opt,	// default end at
	    			//         options
	    non_default_ea_opt,	// non-default end at
	    			//             options
	    other_enable_opt,	// other enable options
	    default_enable_opt,	// default enable
	    			//         options
	    non_default_enable_opt,
	    			// non-default enable
	    			//             options
	    other_selectors, 	// other selectors
	    parser,		// parser
	    data,		// data
	    atom,		// atom
	    prefix,		// prefix
	    header,		// header
	    separator,		// separator
	    line,		// line
	    paragraph,		// paragraph
	    standard,		// standard
	    define,		// define
	    undefine,		// undefine
	    test,		// test
	    begin,		// begin
	    end,		// end
	    print,		// print
	    block,		// block
	    pass,		// pass
	    mapped,		// mapped
	    lexeme,		// lexeme
	    selector,		// selector
	    selectors,		// selectors
	    options,		// options
	    group,		// group
	    lexical,		// lexical
	    master,		// master
	    implied,		// implied
	    subprefix,		// subprefix
	    reformatter,	// reformatter
	    default_lexeme,	// default
	    with,		// with
	    parsing,		// parsing
	    after,		// after
	    before,		// before
	    at,			// at
	    plus,		// +
	    minus,		// -
	    dotdotdot,		// ...
	    yes,		// yes
	    no,			// no
	    keep,		// keep
	    enabled,		// enabled
	    disabled,		// disabled
	    star_parser,    	// *PARSER*
	    parser_colon,   	// *PARSER*:
	    parser_test_colon,	// *PARSER* *TEST*:
	    ID,			// ID
	    character;		// character
    }

    extern min::gen_format name_format;
	// Same as min::default_gen_format except
	// that {str,lab}_{pre,post}fix are all "".

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
    MAPPED_PREFIX	= 0xFFFFFFFA,
    IMPLIED_PREFIX	= 0xFFFFFFF9,
    IMPLIED_HEADER	= 0xFFFFFFF8,
    OPERATOR		= 0xFFFFFFF7,
    DERIVED		= 0xFFFFFFF6,

    TEMPORARY_TT	= 0xFFFFF000,
      // TEMPORARY_TT + n for 0 <= n < 63 may be used
      // by reformatters for temporary token types.

    MAX_LEXEME		= ll::lexeme::standard
                            ::MAX_TYPE
};
inline bool is_lexeme ( min::uns32 token_type )
{
    return token_type <= MAX_LEXEME;
}

const min::float64 MAX_NATURAL = 1e15 - 1;
    // A MIN number token value is a natural_t iff
    // it encodes an integer in the range
    // [0, MAX_NATURAL].

// Return true iff a float is an integer in the range
// [0,MAX_NATURAL].
//
inline bool is_natural ( min::float64 f )
{
    return ( 0 <= f && f <= MAX_NATURAL
             &&
	     (min::int64) f == f );
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
	//	MAPPED_PREFIX
	//	    A prefix token produced by mapping
	//	    a lexeme.  Like PREFIX but cannot be
	//	    an ISOLATED_HEADER (see ll_parser_
	//	    bracketed.cc).
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
	//	IMPLIED_HEADER
	//	    Implied paragraph or line header.
	//	    Handled similarly to IMPLIED_PREFIX
	//	    except when used for error detec-
	//	    tion (e.g., prefix with paragraph
	//	    group can only follow a IMPLIED_
	//	    HEADER).
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
        // MIN string value equal to the lexeme for some
	// lexeme types.  See the type above for non-
	// lexeme type values.

    const min::gen value_type;
        // For a BRACKETED token, the .initiator of its
	// MIN value, if any, or the prefix group of its
	// .type, if that is `line', `paragraph', or
	// `reset'.
	//
	// For a PREFIX, IMPLIED_PREFIX, or IMPLIED_
	// HEADER token, the prefix table entry of the
	// prefix .type found using the selectors in
	// effect when the prefix was parsed, if any,
	// or the .type of its MIN value otherwise.
	//
	// In all other cases, MISSING.

    min::phrase_position position;
        // Position of the first character of the token
        // and of the first character AFTER the token,
	// or the end of input.

    min::int32 indent;
        // Indent of the first character AFTER the
	// token, or of the end of input.  Used in the
	// indent_t tokens to determine line indent.
	// See ll_lexeme.h for details of how character
	// and end of input indent is computed.
	//
	// Signed only so that signed relative indenta-
	// tion can be easily computed.

    const ll::parser::token next, previous;
        // Doubly linked list pointers for tokens.
};

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
// setting its type.  Its value is set to
// min::MISSING().
//
ll::parser::token new_token ( uns32 type );

// Free token and return NULL_STUB.  Token is moved from
// the allocated list to the free list after its value
// is set to MISSING.
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

// Given a token value, return the token type and token
// value_type appropriate to that token value.  This
// is as follows:
//
//     If the token value is an object with either an
//     .initiator or a .terminator, the token type is
//     BRACKETED and the token value_type is the
//     .initiator.
//
//     Otherwise if the token value is an object with
//     a .type and NO elements, the token type is PREFIX
//     and the token value_type is the .type.
//
//     Otherwise if the token value is an object with a
//     .type and SOME elements, the token type is
//     BRACKETED and the token value type is MISSING.
//
//     Otherwise if the token value is an object, the
//     token type is BRACKETABLE and the token value
//     type is MISSING.
//
//     Otherwise if the token value is not NONE, the
//     token type is DERIVED and the token value type is
//     MISSING.
//
//     Otherwise if the token value is NONE, the token
//     type is 0 and the token value type is MISSING.
//
min::uns32 find_token_type 
	( min::ref<min::gen> value_type,
	  min::gen value );
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
    // command, other than `begin/end block ...',
    // that may require action by a pass.  Examples are
    // `define/undefine/print ...'.  Vp[i0 ..] contains
    // the command, vp[0 .. i0-1] is ignored, and ppvec
    // is the phrase position vector of vp.
    //
    // This pass does NOT have to recognize or process
    // the command, which may be for another pass.  In
    // this case min::FAILURE() is returned.
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
    ALL_OPT       			= 0xFFFFFull,

    EAINDENT_OPT			= 1ull << 0,
    EALEINDENT_OPT			= 1ull << 1,
    EALTINDENT_OPT			= 1ull << 2,
    EAPBREAK_OPT			= 1ull << 3,
    EALSEP_OPT				= 1ull << 4,
    EAOCLOSING_OPT			= 1ull << 5,
    EIPARAGRAPH_OPT			= 1ull << 6,
    EPREFIX_OPT				= 1ull << 7,
    ETPREFIX_OPT			= 1ull << 8,
    EHEADER_OPT				= 1ull << 9,
    STICKY_OPT				= 1ull << 10,
    CONTINUING_OPT			= 1ull << 11,

    ALL_EA_OPT    = EAINDENT_OPT
	          + EALEINDENT_OPT
	          + EALTINDENT_OPT
	          + EAPBREAK_OPT
	          + EALSEP_OPT
	          + EAOCLOSING_OPT,

    ALL_ENABLE_OPT = EIPARAGRAPH_OPT
	           + EPREFIX_OPT
		   + ETPREFIX_OPT
		   + EHEADER_OPT,

    DEFAULT_EA_OPT = EALEINDENT_OPT
	           + EALSEP_OPT
	           + EAOCLOSING_OPT,

    DEFAULT_ENABLE_OPT = EIPARAGRAPH_OPT
	               + EHEADER_OPT,

    DEFAULT_OPT = DEFAULT_EA_OPT
                + DEFAULT_ENABLE_OPT,

    // Selectors:
    //
    SELECTOR_OFFSET			= 20,

    ALL_SELECTORS 			= -1ull << 20,

    ALWAYS_SELECTOR			= 1ull << 20,
        // Hidden selector that is always on.

    // Builtin selectors:
    //
    TOP_LEVEL_SELECTOR			= 1ull << 21,
    LINE_LEVEL_SELECTOR			= 1ull << 22,
    DATA_SELECTOR			= 1ull << 23,
    ATOM_SELECTOR			= 1ull << 24,

    COMMAND_SELECTORS = ALL_SELECTORS
                      - ALWAYS_SELECTOR,


    TOP_LEVEL_OFF_SELECTORS = TOP_LEVEL_SELECTOR
                            + LINE_LEVEL_SELECTOR,
    TOP_LEVEL_SELECTORS = COMMAND_SELECTORS
                        - TOP_LEVEL_OFF_SELECTORS,
    TOP_LEVEL_OPT = ALL_EA_OPT
                  + ALL_ENABLE_OPT
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

    const min::id_map id_map;
        // Map of input ID's (as in @ID) to stubs.
	//
	// id_map->ID_character equals min::NO_UCHAR
	// if ID lexemes are disabled.

    const ll::parser::table::lexeme_map lexeme_map;
	// lexeme_map[t] is the stack of lexeme map
	// entries for lexeme type t; == NULL_STUB if
	// no entries for t.

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

    const ll::parser::bracketed::indentation_mark
	    top_level_indentation_mark;
	// Top level indentation mark for parse
	// function.
    ll::parser::table::flags selectors;
        // Cache equal to
	//     top_level_indentation_mark->
	//         parsing_selectors.or_flags 
	//   | TOP_LEVEL_OFF_SELECTORS
	//   | ALWAYS_SELECTOR

    // Parser state:
    //
    uns32 lexical_master;
        // Last lexical master set for scanner, or
	// ll::lexeme::MISSING_MASTER if none set.
	// This is the lexical_master_index set
	// (NOT the lexical table ID).

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
	// a blank line is encountered, at the
	// beginning of an indented paragraph, or
	// when an end of file is encountered.  Set
	// false by a non-indent, non-comment, non-
	// line-break, non-end-of-file token.
	//
	// This value should only be used when at the
	// beginning of a logical line.

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
           (min::gen) ll::parser::lexeme::top_level :
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
MIN_REF ( min::id_map, id_map,
          ll::parser::parser )
MIN_REF ( ll::parser::table::lexeme_map, lexeme_map,
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
// When the parser is created, if standard_components is
// not zero, ll::parser::standard::init_input is called
// and ll::parser::standard::define_standard is then
// called with standard_components as an argument.
//
void init ( min::ref<ll::parser::parser> parser,
            ll::parser::table::flags
	        standard_components =
		    ll::parser::table::ALL_FLAGS );

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

// Create lexeme map if it does not exist.  Parser
// scanner must be defined.
//
inline void init_lexeme_map
	( ll::parser::parser parser )
{
    if ( parser->lexeme_map != min::NULL_STUB )
        return;

    MIN_REQUIRE ( parser->scanner != min::NULL_STUB );
    min::obj_vec_ptr vp =
	parser->scanner->lexeme_type_table;
    ll::parser::lexeme_map_ref(parser) =
	ll::parser::table::create_lexeme_map 
	    ( min::size_of ( vp ) );
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
	  min::gen line_separator,
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
    // The function may change `first'.
    //
    // `position' may include tokens not passed to the
    // reformatter, such as brackets in a bracketed
    // expression, or a line separator.  If there
    // is a line_separator, it is passed as a separate
    // argument (which is otherwise MISSING), and
    // position.end is the position of the end of the
    // line_separator.  The line_separator may be
    // non-MISSING for prefix reformatters, but is
    // generally MISSING for other reformatters.
    //
    // If true is returned, the caller of the function
    // may call ll::parser::compact.  The bracket pass
    // has the brackets surrounding the subexpression
    // that the reformatter declined to reformat, and
    // calls compact with a BRACKETING parameter and
    // .initiator and .terminator attributes.  The
    // operator pass calls compact to enclose the sub-
    // expression in a new PURELIST, unless the sub-
    // expression consists of a single BRACKETED,
    // BRACKTABLE, or PURELIST token, in which case
    // compact is not called and nothing is done to the
    // single token.  In all cases parser, pass->next,
    // selectors, first, next, position, and trace_flags
    // are passed to the `compact' function.
    //
    // For the bracketed pass, the list of tokens passed
    // to the reformatted does NOT include the brackets,
    // but the position does include them.

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
      min::uns32 minimum_arguments, 
      min::uns32 maximum_arguments,
      ll::parser::reformatter_function
          reformatter_function,
      min::ref<ll::parser::reformatter> stack );

// Parser Functions
// ------ ---------

// Return the lexical type of a min::gen value according
// to ll::lexeme::standard::lexical_type_of, unless
// g is a number.
//
inline min::uns32 lexical_type_of ( min::gen g )
{
    if ( min::is_num ( g ) )
    {
        min::float64 f = min::float_of ( g );
	if ( ! std::isfinite ( f ) )
	    return ll::lexeme::standard
	             ::numeric_word_t;
	else if ( is_natural ( f ) )
	    return ll::lexeme::standard::natural_t;
	else
	    return ll::lexeme::standard::number_t;
    }
    else
	return ll::lexeme::standard
		 ::lexical_type_of ( g );
}

// Compute new_flags that set the selectors to the
// selectors argument, turning on these and the ALWAYS_
// SELECTOR and turning off all other selectors.
//
inline ll::parser::table::new_flags parsing_selectors
        ( ll::parser::table::flags selectors )
{
    return ll::parser::table::new_flags
        ( selectors,
	  ll::parser::COMMAND_SELECTORS & ~ selectors,
	  0 );
}

// Ditto for options.
//
inline ll::parser::table::new_flags parsing_options
        ( ll::parser::table::flags options )
{
    return ll::parser::table::new_flags
        ( options, ll::parser::ALL_OPT & ~ options, 0 );
}

// Get and set parser selectors and options in
// parser->top_level_indentation_mark.  Selectors or
// options not in TOP_LEVEL_SELECTORS or TOP_LEVEL_OPT
// are masked out and ignored.
//
// Note: options are included in the returned
// selectors value or selectors argument.
//
ll::parser::table::flags get_selectors
	( ll::parser::parser parser = default_parser );
void set_selectors
        ( ll::parser::table::flags selectors,
	  ll::parser::parser parser = default_parser );

// Get and set parser->lexical_master.  Note the lexical
// master gotten is the last one set by ll::parser::set_
// lexical_master, and is often NOT the current master
// table of the scanner, which may not have a name or
// index.
//
const uns32 MISSING_MASTER = ll::lexeme::MISSING_MASTER;
inline uns32 get_lexical_master
	( ll::parser::parser parser = default_parser )
{
    return parser->lexical_master;
}

inline void set_lexical_master
	( min::uns32 lexical_master,
	  ll::parser::parser parser = default_parser )
{
    parser->lexical_master = lexical_master;
    ll::lexeme::set_lexical_master
	  ( lexical_master, parser->scanner );
}

// Find lexical master given master name and vice versa.
// Returns MISSING_MASTER or min::MISSING() if not
// found.
//
inline min::uns32 get_lexical_master
	( min::gen master_name,
	  ll::parser::parser parser = default_parser )
{
    return ll::lexeme::lexical_master_index
        ( master_name, parser->scanner );
}
inline min::gen get_master_name
	( min::uns32 lexical_master,
	  ll::parser::parser parser = default_parser )
{
    return ll::lexeme::lexical_master_name
        ( lexical_master, parser->scanner );
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

// Find an entry with given key_label, key_type.subtype,
// and ALWAYS_SELECTOR in the key_table, or if none,
// make one with given block_level and position.  Return
// entry found or made.
//
template <typename T>
    inline const min::packed_struct_updptr<T> find
	( min::gen key_label,
	  min::packed_struct_with_base
	      < T, ll::parser::table::root_struct >
	      key_type,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  ll::parser::table::key_table key_table )
{
    typedef min::packed_struct_updptr<T> T_ptr;
    min::locatable_var<T_ptr> key_entry
        ( (T_ptr) ll::parser::table::find
	      ( key_label, key_type.subtype,
	        ll::parser::ALWAYS_SELECTOR,
	        key_table ) );

    if ( key_entry == min::NULL_STUB )
    {
        key_entry = key_type.new_stub();
	label_ref(key_entry) = key_label;
	key_entry->selectors =
	    ll::parser::ALWAYS_SELECTOR;
	key_entry->block_level = block_level;
	key_entry->position = position;
	ll::parser::table::push
	    ( key_table,
	      (ll::parser::table::root) key_entry );
    }
    return key_entry;
}

// Locate the key prefix in the key table that
// corresponds to the longest available string of tokens
// beginning with `current'.  If `next' is NULL_STUB,
// tokens are added to the token list as necessary, but
// otherwise only tokens before `next' are considered.
// Only lexeme token types that are symbols as per
// symbol_mask are considered.  If `next' is NULL_STUB,
// it is assumed that the token list finally ends with
// an end-of-file token, and this cannot be part of any
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

// Options for set_attr_...
//
enum { NEW, NEW_OR_SAME, ADD_TO_SET, ADD_TO_MULTISET };

// Given an attribute pointer ap located at an attribute
// and a bracketed expression `flags' designating attri-
// bute flags, set the designated flags of the attri-
// bute.  If part of `flags' has erroneous format, print
// an error message for that part and ignore the part.
//
// If option is NEW, attribute is required to NOT have
// previous flags.  If option is NEW_OR_SAME, attribute
// is required to either have no previous flags, or to
// have a flags equal to the new flags (in which case
// nothing is done).  If the option is ADD_TO_SET or
// ADD_TO_MULTISET, each flag is added to the existing
// set of attribute flags, unless it is already in that
// set.
//
// If option requirements are not met, this function
// does nothing but print an error message.  True is
// returned if there is no option error; false if there
// is an option error.  Flag parsing errors are NOT
// option errors, and simply print an error message and
// ignore the associated part of the `flags' argument.
//
bool set_attr_flags
	( ll::parser::parser parser,
	  min::attr_insptr & ap,
	  min::gen flags,
	  unsigned option = NEW,
	  const min::flag_parser * flag_parser
	      = min::standard_attr_flag_parser,
	  min::packed_vec_ptr<min::ustring> flag_names
	      = min::standard_attr_flag_names );

// Given an attribute pointer ap located at an attribute
// and a bracketed expression `flags' designating attri-
// bute flags, test whether the flags pointed at by ap
// are exactly the designated flags.  Return true if so,
// and false otherwise.  If false is returned, print an
// error message for each flag found to mismatch.
//
// If part of `flags' has erroneous format, print an
// error message for that part and ignore the part.
//
bool test_attr_flags
	( ll::parser::parser parser,
	  min::attr_insptr & ap,
	  min::gen flags,
	  const min::flag_parser * flag_parser
	      = min::standard_attr_flag_parser,
	  min::packed_vec_ptr<min::ustring> flag_names
	      = min::standard_attr_flag_names,
	  min::unsptr n = 100 );
	      // n is hidden argument: first try
	      // length of min::get_flags vector.

// Given an attribute pointer ap located at an attribute
// and a value, set attribute to the value.  Require
// that the values be legal (not condition codes, not
// auxiliary values, not NONE, etc., as per min::is_
// attr_legal).  If the attribute is a double-arrow
// attribute (min::reverse_name_of is not NONE), require
// that the value be an object.
//
// If option is NEW, attribute is required to NOT have
// a previous value.  If option is NEW_OR_SAME,
// attribute is required to either have no previous
// value, or to have a value equal to the new value (in
// which case nothing is done).  If the option is
// ADD_TO_SET, the value is added to the multiset of
// attribute values, unless it is already in that
// multiset.  If the option is ADD_TO_MULTISET, the
// value is added to the multiset.
//
// If requirements are not met, this function does
// nothing but print an error message.  True is returned
// if there is no error; false if there is an error.
// Any error message uses `pos' to determine the posi-
// tion of the value (not the name) in the input stream.
//
bool set_attr_value
	( ll::parser::parser parser,
	  min::attr_insptr & ap,
	  min::gen value,
	  min::phrase_position const & pos,
	  unsigned option = NEW );

// Given an attribute pointer ap located at an attribute
// and a bracketed expression `multivalue' containing
// multiple attribute value, set the designated values
// of the attribute using set_attr_value.  If option
// is NEW there must be no previous values.  If NEW_OR_
// SAME either there must be no previous values are the
// value sets of multivalue and ap must be the same as
// sets, with duplicates allowed for both `sets'.
// Otherwise if option is ADD_TO_SET, values in multi-
// value not already in the multiset of ap values are
// added to the multiset of ap values.  If the option
// is ADD_TO_MULTISET, values in the multivalue are
// added to the multiset of ap values.  Returns true
// if no error messages and false if error messages.
//
bool set_attr_multivalue
	( ll::parser::parser parser,
	  min::attr_insptr & ap,
	  min::gen multivalue,
	  unsigned option = NEW );

// First, invokes the given pass using execute_pass_
// parse on the expression consisting of the tokens
// beginning with `first' and ending just before `next'.
// If the pass argument is NULL_STUB, this has no
// effect.
//
// Then replaces the expression tokens (which may have
// been changed by the pass) by a single expression
// token.  Adds the m attributes whose names and values
// are given, and allows for the latter addition of n
// attributes.  Sets the position of the new expression
// token from the given argument.  The resulting
// expression token is in first == next->previous.  Its
// type is nominally given in the type argument, which
// should be either BRACKETED, or BRACKETABLE.  However,
// as special cases, the type argument can also be
// BRACKETING or PURELIST: see below.
//
// If the type argument is BRACKETABLE, the m attributes
// MUST NOT include any .type, .initiator, or .termina-
// tor attributes.
//
// If the type argument is BRACKETING, the m attributes
// can only include .type, .initiator, and/or .termina-
// tor attributes.  The resulting expression is
// BRACKETED if m != 0 and PURELIST if m == 0.
//
// If the type argument is PURELIST, m == 0 is required,
// and the resulting expression is PURELIST.
//
// Natural number and quoted string tokens in the
// expression are converted as per convert_token.
//
// If after invoking the given pass and applying
// convert_token, there is only one element, m == 0,
// and the type argument is NOT PURELIST, the position
// of the token is set to the position argument and the
// token is returned as is, without making an expression
// containing it.
//
// Otherwise a new object is created with space for the
// one element per expression token, the m attributes,
// one .position attribute, and n attributes to be added
// later.  The .position of the new object is set from
// the position argument and the positions of its
// element tokens.
//
// The type of the resulting expression is set to
// PURELIST if m == 0, BRACKETED if the type argument is
// BRACKETING and m != 0, and the type argument other-
// wise.
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
// The trace_subexpression function is called with the
// final output subexpression token to process any
// TRACE_SUBEXPRESSION_... trace_flags.
//
struct attr
{
    min::gen name;
    min::gen value;  // min::NONE() if no value.
    min::phrase_position value_pos;
    min::gen multivalue;
    min::gen flags;
    min::gen reverse_name;
    attr ( min::gen name, min::gen value )
        : name ( name ), value ( value ),
	  multivalue ( min::MISSING() ),
	  flags ( min::MISSING() ),
	  reverse_name ( min::MISSING() )
	  { value_pos.begin =
	    value_pos.end = min::MISSING_POSITION; }
    attr ( void )
        : name ( min::MISSING() ),
	  value ( min::NONE() ),
	  multivalue ( min::MISSING() ),
	  flags ( min::MISSING() ),
	  reverse_name ( min::MISSING() )
	  { value_pos.begin =
	    value_pos.end = min::MISSING_POSITION; }
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
      ll::lexeme::standard::symbol_mask;
      // Word, numeric, natural, number, mark,
      // separator.
const min::uns64 IGNORED_SCAN_MASK =
      ( 1ull << ll::lexeme::standard
                          ::horizontal_space_t )
    + ( 1ull << ll::lexeme::standard::indent_t )
    + ( 1ull << ll::lexeme::standard
                          ::premature_end_of_file_t )
    + ( 1ull << ll::lexeme::standard::start_of_file_t )
    + ( 1ull << ll::lexeme::standard::end_of_file_t );
const min::uns64 END_SCAN_MASK =
      ( 1ull << ll::lexeme::standard
                          ::premature_end_of_file_t )
    + ( 1ull << ll::lexeme::standard::end_of_file_t );

// Other type masks:
//
const min::uns64 LABEL_HEADER_MASK =
      ( 1ull << ll::lexeme::standard::word_t )
    + ( 1ull << ll::lexeme::standard::numeric_t )
    + ( 1ull << ll::lexeme::standard::quoted_string_t );
const min::uns64 LABEL_COMPONENT_MASK =
      ( 1ull << ll::lexeme::standard::word_t )
    + ( 1ull << ll::lexeme::standard::numeric_t )
    + ( 1ull << ll::lexeme::standard::quoted_string_t )
    + ( 1ull << ll::lexeme::standard::natural_t )
    + ( 1ull << ll::lexeme::standard::number_t );
const min::uns64 VALUE_COMPONENT_MASK =
      ( 1ull << ll::lexeme::standard::word_t )
    + ( 1ull << ll::lexeme::standard::numeric_t )
    + ( 1ull << ll::lexeme::standard::quoted_string_t )
    + ( 1ull << ll::lexeme::standard::natural_t )
    + ( 1ull << ll::lexeme::standard::number_t );
const min::uns64 QUOTED_STRING_MASK =
      ( 1ull << ll::lexeme::standard::quoted_string_t );

// Given an object vector pointer vp pointing at an
// expression, and an index i of an element in the
// object attribute vector, then if the element is
// a quoted string, increment i by one and a return
// the min::gen value containing the quoted string
// characters proper.
//
// If the element does not exist (i >= size_of ( vp)) or
// is not a quoted string, this function just returns
// min::MISSING().
//
inline min::gen scan_quoted_string
    ( min::obj_vec_ptr & vp, min::uns32 & i,
      ll::parser::parser parser )
{
    if ( i >= min::size_of ( vp ) )
            return min::MISSING();
    min::gen element = vp[i];
    if (    min::get ( element, min::dot_type )
	 != ll::parser::lexeme::doublequote )
	return min::MISSING();
    min::obj_vec_ptr ep = element;
    if ( min::size_of ( ep ) != 1 )
	return min::MISSING();
    ++ i;
    return ep[0];
}

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
// is not a word or natural or is equal to end_value.
//
// If i has been incremented at least once, make and
// return a label from the elements scanned over.  If
// there is only 1 element, return just that element.
// If there is more than one, return the MIN label
// containing the elements.  If there are no elements,
// return min::NONE().
//
min::gen scan_simple_name
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  min::gen end_value = min::NONE() );

// Mode values for scan_label_or_value and
// make_label_or_value.
//
enum scan_mode {
    LABEL_MODE		= 1,
        // First label component must be selected in
	// LABEL_HEADER_MASK.  Rest must be selected
	// in LABEL_COMPONENT_MASK.
    VALUE_MODE		= 2,
        // All label components must be selected in
	// VALUE_COMPONENT_MASK.
    DATA_MODE		= 3
        // All legal label components are accepted,
	// namely strings, numbers, and sublabels.
};

// Given an object vector pointer vp pointing at an
// expression, and an index i of an element in the
// vector, then repetatively increment i if vp[i] is not
// an object other than a quoted string and is not equal
// to end_value.
//
// Discard any vp[i] that is not selected according to
// the mode while printing an error message.
//
// If there are no elements, print an error message and
// return NONE().   If there are discarded elements,
// return min::NONE().
//
// Otherwise if there is only one element, return it,
// but if there is more than one, return the MIN label
// containing the elements.
//
// DATA_MODE is NOT supported.  Parser is only used to
// print error messages.
//
min::gen scan_label_or_value
	( ll::parser::parser parser,
	  min::obj_vec_ptr & vp, min::uns32 & i,
	  ll::parser::scan_mode mode,
	  min::gen end_value = min::NONE() );

// Given the token sequence from `first' to `next', make
// a MIN label containing these tokens' values if there
// is more than one token.  Announce tokens not selected
// by mode as errors.
//
// If `first' == `next' (and therefore the result is
// empty), a new first token is made and inserted before
// `next', the `first' argument is updated to point at
// the new token, the type of the new token is set to
// DERIVED, and the new token position is set to the
// empty string just before `next'.  If the mode is
// DATA_MODE, the token value is set to the empty MIN
// label.  Otherwise an error is announced and the token
// value is set to min::NONE().  This is the only case
// where `first' is updated.
//
// Otherwise all tokens but the first are removed, the
// type of the first token is set to DERIVED, and its
// position is set to include the positions of all the
// tokens.  If there are errors, the value of the first
// token is set to min::NONE().  If there are no errors
// and there is only one token, the value of first and
// only token is left alone.  If there are no errors and
// there is more than one token, the value of the first
// token is set to a MIN label containing all the token
// values.
//
// True is returned if there are no errors, and false is
// returned if there are errors.  The first token value
// is set to min::NONE() iff there are errors.
//
bool make_label_or_value
	( ll::parser::parser parser,
	  ll::parser::token & first,
	  ll::parser::token next,
	  ll::parser::scan_mode mode );

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
	  min::gen end_value = min::NONE() );

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

// Ensure there is a next token.
//
inline void ensure_next
	( ll::parser::parser parser,
	  ll::parser::token current )
{
    if ( current->next == parser->first
         &&
	 current->type != ll::lexeme::standard
	                            ::end_of_file_t )
    {
	parser->input->add_tokens
	    ( parser, parser->input );
	MIN_REQUIRE
	    ( current->next != parser->first );
    }
}

// Return `token indent - paragraph_indent' and warn
// if token indent is too near paragraph_indent, i.e.,
// if the absolute value returned is < indentation_
// offset but not 0.  Require token->type == indent_t.
//
inline min::int32 relative_indent
	( ll::parser::parser parser,
	  min::int32 indentation_offset,
	  ll::parser::token token,
	  min::int32 paragraph_indent )
{
    MIN_REQUIRE
        (    token->type
	  == ll::lexeme::standard::indent_t );

    int relative_indent =
        token->indent - paragraph_indent;
    if (    relative_indent != 0
	 && relative_indent < indentation_offset 
	 && relative_indent > - indentation_offset )
    {
	char buffer[200];
	sprintf ( buffer, "lexeme indent %d too near"
			  " paragraph indent %d",
			  token->indent,
			  paragraph_indent );
	min::phrase_position position = token->position;
	position.begin = position.end;
	ll::parser::parse_warn
	     ( parser, position, buffer );
    }
    return relative_indent;
}

} }

# endif // LL_PARSER_H
