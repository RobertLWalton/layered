// Layered Languages Bracketed Subexpression Parser
//
// File:	ll_parser_bracketed.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Apr 15 06:45:07 EDT 2017
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Untyped Brackets
//	Indentation Marks
//	Typed Brackets
//	Prefix Table
//	Bracketed Subexpression Pass
//	Parse Bracketed Subexpression Function
//	Bracketed Compact Functions

// Usage and Setup
// ----- --- -----

# ifndef LL_PARSER_BRACKETED_H
# define LL_PARSER_BRACKETED_H

# include <ll_parser.h>

namespace ll { namespace parser { namespace bracketed {

extern min::locatable_var<ll::parser::reformatter>
		reformatter_stack;


// Untyped Brackets
// ------- --------

// Bracket definition.
//
struct opening_bracket_struct;
typedef min::packed_struct_updptr
	    <opening_bracket_struct>
        opening_bracket;
extern const uns32 & OPENING_BRACKET;
    // Subtype of min::packed_struct
    //		       <opening_bracket_struct>.
struct closing_bracket_struct;
typedef min::packed_struct_updptr
	    <closing_bracket_struct>
        closing_bracket;
extern const uns32 & CLOSING_BRACKET;
    // Subtype of min::packed_struct
    //		       <closing_bracket_struct>.
struct opening_bracket_struct :
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is OPENING_BRACKET.

    const ll::parser::bracketed::closing_bracket
          closing_bracket;
        // The opposing bracket of the opening bracket.

    ll::parser::table::new_flags new_selectors;
    	// New selectors associated with this opening
	// bracket.

    const ll::parser::reformatter reformatter;
    const ll::parser::reformatter_arguments
        reformatter_arguments;
	// Reformater and its arguments, or (both) NULL_
	// STUB if none.
};
struct closing_bracket_struct : 
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is CLOSING_BRACKET.
};

MIN_REF ( ll::parser::table::root, next,
          ll::parser::bracketed::opening_bracket )
MIN_REF ( min::gen, label,
          ll::parser::bracketed::opening_bracket )
MIN_REF ( ll::parser::bracketed::closing_bracket,
          closing_bracket,
          ll::parser::bracketed::opening_bracket )
MIN_REF ( ll::parser::reformatter,
          reformatter,
          ll::parser::bracketed::opening_bracket )
MIN_REF ( ll::parser::reformatter_arguments,
          reformatter_arguments,
          ll::parser::bracketed::opening_bracket )

MIN_REF ( ll::parser::table::root, next,
          ll::parser::bracketed::closing_bracket )
MIN_REF ( min::gen, label,
          ll::parser::bracketed::closing_bracket )

// Push entry into bracket table.  EALSEP_OPT is set
// in new_selectors.not_flags and cleared from
// new_selectors.{or,xor}_flags.
//
ll::parser::bracketed::opening_bracket
    push_brackets
	( min::gen opening_bracket,
	  min::gen closing_bracket,
	  ll::parser::table::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  const ll::parser::table::new_flags
	      & new_selectors,
	  ll::parser::reformatter reformatter,
	  ll::parser::reformatter_arguments
	      reformatter_arguments,
	  ll::parser::table::key_table bracket_table );


// Indentation Marks
// ----------- -----

// Indentation mark definition.
//
struct indentation_mark_struct;
typedef min::packed_struct_updptr
	    <indentation_mark_struct>
        indentation_mark;
extern const uns32 & INDENTATION_MARK;
    // Subtype of min::packed_struct
    //		       <indentation_mark_struct>.

struct line_sep_struct;
typedef min::packed_struct_updptr<line_sep_struct>
        line_sep;
extern const uns32 & LINE_SEP;
    // Subtype of min::packed_struct<line_sep_struct>.

struct indentation_mark_struct : 
	public ll::parser::table::root_struct
{
    ll::parser::table::new_flags new_selectors;

    const ll::parser::bracketed::line_sep line_sep;

    min::gen implied_header;

    min::gen implied_header_type;
        // .type of implied header, cached here.
	// May be NONE.

    uns32 lexical_master;
        // ll::lexeme::MISSING_MASTER if missing.
};

MIN_REF ( ll::parser::table::root, next,
          ll::parser::bracketed::indentation_mark )
MIN_REF ( min::gen, label,
          ll::parser::bracketed::indentation_mark )
MIN_REF ( ll::parser::bracketed::line_sep,
          line_sep,
          ll::parser::bracketed::indentation_mark )
MIN_REF ( min::gen, implied_header,
          ll::parser::bracketed::indentation_mark )
MIN_REF ( min::gen, implied_header_type,
          ll::parser::bracketed::indentation_mark )

struct line_sep_struct : 
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is LINE_SEP.
};

MIN_REF ( ll::parser::table::root, next,
          ll::parser::bracketed::line_sep )
MIN_REF ( min::gen, label,
          ll::parser::bracketed::line_sep )

ll::parser::bracketed::indentation_mark
    push_indentation_mark
	( min::gen mark_label,
	  min::gen separator_label,
	      // May be min::MISSING()
	  ll::parser::table::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  const ll::parser::table::new_flags
	      & new_selectors,
	  min::gen implied_header,
	      // May be min::MISSING()
	  min::uns32 lexical_master,
	      // May be ll::lexeme::MISSING_MASTER
	  ll::parser::table::key_table bracket_table );

// Typed Brackets
// ----- --------

// Typed bracket definition.

struct typed_opening_struct;
    // Subclass of opening_bracket.
typedef min::packed_struct_updptr
	    <typed_opening_struct>
        typed_opening;
extern const uns32 & TYPED_OPENING;
    // Subtype of min::packed_struct
    //		       <typed_opening_struct>.

// Typed_opening is closed by closing_bracket.

struct typed_middle_struct;
typedef min::packed_struct_updptr
	    <typed_middle_struct>
        typed_middle;
extern const uns32 & TYPED_MIDDLE;
    // Subtype of min::packed_struct
    //		       <typed_middle_struct>.

struct typed_double_middle_struct;
typedef min::packed_struct_updptr
	    <typed_double_middle_struct>
        typed_double_middle;
extern const uns32 & TYPED_DOUBLE_MIDDLE;
    // Subtype of min::packed_struct
    //		       <typed_double_middle_struct>.

struct typed_attr_begin_struct;
typedef min::packed_struct_updptr
	    <typed_attr_begin_struct>
        typed_attr_begin;
extern const uns32 & TYPED_ATTR_BEGIN;
    // Subtype of min::packed_struct
    //		       <typed_attr_begin_struct>.

struct typed_attr_equal_struct;
typedef min::packed_struct_updptr
	    <typed_attr_equal_struct>
        typed_attr_equal;
extern const uns32 & TYPED_ATTR_EQUAL;
    // Subtype of min::packed_struct
    //		       <typed_attr_equal_struct>.

struct typed_attr_sep_struct;
typedef min::packed_struct_updptr
	    <typed_attr_sep_struct>
        typed_attr_sep;
extern const uns32 & TYPED_ATTR_SEP;
    // Subtype of
    //     min::packed_struct
    //	       <typed_attr_sep_struct>.

struct typed_attr_negator_struct;
typedef min::packed_struct_updptr
	    <typed_attr_negator_struct>
        typed_attr_negator;
extern const uns32 & TYPED_ATTR_NEGATOR;
    // Subtype of
    //     min::packed_struct
    //	       <typed_attr_negator_struct>.

struct typed_opening_struct :
	public ll::parser::bracketed
	                 ::opening_bracket_struct
{
    // Packed_struct subtype is TYPED_OPENING.
    // Typed_opening is closed by closing_bracket.
    
    // Element selectors are in opening_bracket_struct::
    // new_selectors.  Attribute selectors are here:
    //
    ll::parser::table::flags attr_selectors;

    const ll::parser::bracketed::typed_middle
          typed_middle;
    const ll::parser::bracketed::typed_double_middle
          typed_double_middle;
	  // Equals NULL_STUB if not provided because
	  // concatenating 2 middles does not create
	  // a single lexeme.
    const ll::parser::bracketed::typed_attr_begin
          typed_attr_begin;
    const ll::parser::bracketed::typed_attr_equal
          typed_attr_equal;
    const ll::parser::bracketed::typed_attr_sep
          typed_attr_sep;
    const ll::parser::bracketed
                    ::typed_attr_negator
          typed_attr_negator;

    // Components used to process attribute flags and
    // multivalues.
    //
    const min::gen typed_attr_flags_initiator;
    const min::flag_parser * typed_attr_flag_parser;
    const min::gen typed_attr_multivalue_initiator;

    bool prefix_separators_allowed;
	// True if this typed opening can be used to
	// generate prefix separators.

};

struct typed_middle_struct : 
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is TYPED_MIDDLE.
};

struct typed_double_middle_struct : 
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is TYPED_DOUBLE_MIDDLE.
};

struct typed_attr_begin_struct : 
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is TYPED_ATTR_BEGIN.
};

struct typed_attr_equal_struct : 
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is TYPED_ATTR_EQUAL.
};

struct typed_attr_sep_struct : 
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is TYPED_ATTR_SEP.
};

struct typed_attr_negator_struct : 
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is TYPED_ATTR_NEGATOR.
};


MIN_REF ( ll::parser::table::root, next,
          ll::parser::bracketed::typed_opening )
MIN_REF ( min::gen, label,
          ll::parser::bracketed::typed_opening )
MIN_REF ( ll::parser::reformatter,
          reformatter,
          ll::parser::bracketed::typed_opening )
MIN_REF ( ll::parser::reformatter_arguments,
          reformatter_arguments,
          ll::parser::bracketed::typed_opening )
MIN_REF ( ll::parser::bracketed::typed_middle,
          typed_middle,
          ll::parser::bracketed::typed_opening )
MIN_REF ( ll::parser::bracketed::typed_double_middle,
          typed_double_middle,
          ll::parser::bracketed::typed_opening )
MIN_REF ( ll::parser::bracketed::closing_bracket,
          closing_bracket,
          ll::parser::bracketed::typed_opening )
MIN_REF ( ll::parser::bracketed::typed_attr_begin,
          typed_attr_begin,
          ll::parser::bracketed::typed_opening )
MIN_REF ( ll::parser::bracketed::typed_attr_equal,
          typed_attr_equal,
          ll::parser::bracketed::typed_opening )
MIN_REF ( ll::parser::bracketed::typed_attr_sep,
          typed_attr_sep,
          ll::parser::bracketed::typed_opening )
MIN_REF ( ll::parser::bracketed
                    ::typed_attr_negator,
          typed_attr_negator,
          ll::parser::bracketed::typed_opening )
MIN_REF ( min::gen,
          typed_attr_flags_initiator,
          ll::parser::bracketed::typed_opening )
MIN_REF ( min::gen,
          typed_attr_multivalue_initiator,
          ll::parser::bracketed::typed_opening )

MIN_REF ( ll::parser::table::root, next,
          ll::parser::bracketed::typed_middle )
MIN_REF ( min::gen, label,
          ll::parser::bracketed::typed_middle )

MIN_REF ( ll::parser::table::root, next,
          ll::parser::bracketed::typed_double_middle )
MIN_REF ( min::gen, label,
          ll::parser::bracketed::typed_double_middle )

MIN_REF ( ll::parser::table::root, next,
          ll::parser::bracketed::typed_attr_begin )
MIN_REF ( min::gen, label,
          ll::parser::bracketed::typed_attr_begin )

MIN_REF ( ll::parser::table::root, next,
          ll::parser::bracketed::typed_attr_equal )
MIN_REF ( min::gen, label,
          ll::parser::bracketed::typed_attr_equal )

MIN_REF ( ll::parser::table::root, next,
          ll::parser::bracketed::typed_attr_sep )
MIN_REF ( min::gen, label,
          ll::parser::bracketed::typed_attr_sep )

MIN_REF ( ll::parser::table::root, next,
          ll::parser::bracketed
	            ::typed_attr_negator )
MIN_REF ( min::gen, label,
          ll::parser::bracketed
	            ::typed_attr_negator )

// Push entry into bracket table.  EALSEP_OPT is set
// in element_selectors.not_flags and cleared from
// element_selectors.{or,xor}_flags.
//
ll::parser::bracketed::typed_opening
    push_typed_brackets
	( min::gen typed_opening,
	  min::gen typed_middle,
	  min::gen typed_double_middle,
	  min::gen typed_closing,
	  ll::parser::table::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  const ll::parser::table::new_flags
	      & element_selectors,
	  ll::parser::table::flags attr_selectors,
	  min::gen typed_attr_begin,
	  min::gen typed_attr_equal,
	  min::gen typed_attr_sep,
	  min::gen typed_attr_negator,
	  min::gen typed_attr_flags_initiator,
	  const min::flag_parser *
		   typed_attr_flag_parser,
	  min::gen typed_attr_multivalue_initiator,
	  bool prefix_separators_allowed,
	  ll::parser::table::key_table bracket_table );

// Data used during typed bracketed subexpression parse.
//
struct typed_data
{
    ll::parser::bracketed::typed_opening typed_opening;
        // Typed_opening read just before the call to
	// parse_bracketed_subexpression.
    ll::parser::table::flags saved_selectors;
        // Save of selectors argument to
	// parse_bracketed_subexpression.
    min::unsptr middle_count;
        // Count of typed_middles seen so far.
    min::unsptr attr_count;
        // Count of attributes found so far, including
	// the .type attribute.
    ll::parser::token start_previous;
	// start_previous->next is the first token after
	// the last key, or the first token after the
	// typed opening of the typed bracketed sub-
	// expression if no key found yet.  Since this
	// start_previous->next token can be changed
	// after start_previous has been saved, the
	// start_previous->next token cannot be directly
	// saved.
    min::uns32 subtype;
        // Subtype of last key found.
    ll::parser::token elements;
        // First token of elements, or NULL_STUB if
	// there are no elements.
    ll::parser::token attributes;
        // First token of the list of attributes found
	// so far that have not been moved to before
	// the subexpression elements.  Set when NULL_
	// STUB and an attribute label is found.  Reset
	// to NULL_STUB when the attributes in the list
	// are moved to before the subexpression
	// elements.
    min::position end_position;
        // End position of typed bracketed subexpres-
	// sion, or min::MISSING_POSITION if not
	// determined yet.  Set to the end of a token
	// that is being moved from the end of the
	// subexpression to some previous place in the
	// subexpression.  Also set to the end of a
	// key that is about to be removed.  Used to
	// determine the real end of a typed bracketed
	// subexpression when that is ended prematurely
	// by the closing bracket of a containing
	// expression, or the end of a logical line.
    min::gen type;
        // If typed_opening is followed immediately by a
	// mark, the value of that mark.  If it is fol-
	// lowed by a non-mark type label, the value of
	// that type label.  MISSING otherwise.
    bool has_mark_type;
        // True if and only if typed_opening is followed
	// by a mark, which becomes the type.
    min::phrase_position negator_position;
        // Position of last negator for error message
	// indicating negator ignored.  Only meaningful
	// if subtype is TYPED_ATTR_NEGATOR.
};

// Special token types used during typed bracketed
// subexpression parse.
//
// The parse_bracketed_subexpression function, when
// asked to parse a typed bracketed subexpression
// (i.e., when passed a non-NULL typed_data argument),
// returns a list of tokens whose initial segment
// has the temporary token types below, and whose final
// segment are the elements of the list and have other
// token types.
//
// The order of the initial segment tokens is given by:
//
//     initial_segment ::= attribute*
//
//     attribute
//         ::= TYPE
//           | ATTR_LABEL ATTR_FLAGS? ATTR_VALUE
//           | ATTR_LABEL ATTR_FLAGS? ATTR_MULTIVALUE
//           | ATTR_TRUE ATTR_FLAGS?
//           | ATTR_FALSE ATTR_FLAGS?
//
// Note the TYPE token need not be first in the initial
// segment.
//
// TEMPORARY_TT is in ll::parser
//
const min::uns32 TYPE            = TEMPORARY_TT + 0;
    // .type value
const min::uns32 ATTR_LABEL      = TEMPORARY_TT + 1;
const min::uns32 ATTR_FLAGS      = TEMPORARY_TT + 2;
const min::uns32 ATTR_VALUE      = TEMPORARY_TT + 3;
const min::uns32 ATTR_MULTIVALUE = TEMPORARY_TT + 4;
    // Attribute label, flags, and value.  Must be
    // consecutive tokens in that order.  There is
    // only 1 label token, 0 or 1 flag tokens,
    // and 1 value token.  The value token can
    // optionally be a multi-value token.
const min::uns32 ATTR_TRUE       = TEMPORARY_TT + 5;
const min::uns32 ATTR_FALSE      = TEMPORARY_TT + 6;
    // Attribute label for attribute with TRUE or
    // FALSE value implied.  May be followed by
    // ATTR_FLAGS token.


// Prefix Table
// ------ -----

struct prefix_struct;
typedef min::packed_struct_updptr<prefix_struct> prefix;
extern const uns32 & PREFIX;
    // Subtype of min::packed_struct<prefix_struct>.

struct prefix_struct
    : public ll::parser::table::root_struct
{
    // Packed_struct subtype is PREFIX.

    ll::parser::table::new_flags new_selectors;
    min::gen group;
    min::gen implied_subprefix;
    min::gen implied_subprefix_type;
        // This is just a cache of the .type of
	// the implied prefix, or MISSING if none.
    min::uns32 lexical_master;
        // ll::lexeme::MISSING_MASTER if missing.
};

MIN_REF ( ll::parser::table::root, next,
          ll::parser::bracketed::prefix )
MIN_REF ( min::gen, label,
          ll::parser::bracketed::prefix )
MIN_REF ( min::gen, group,
          ll::parser::bracketed::prefix )
MIN_REF ( min::gen, implied_subprefix,
          ll::parser::bracketed::prefix )
MIN_REF ( min::gen, implied_subprefix_type,
          ll::parser::bracketed::prefix )

// Create a prefix definition entry with given
// label and parameters, and push it into the
// given prefix_table.
//
void push_prefix
	( min::gen prefix_label,
	  ll::parser::table::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  ll::parser::table::new_flags new_selectors,
	  min::gen group,
	  min::gen implied_subprefix,
	  min::gen implied_subprefix_type,
	  min::uns32 lexical_master,
	      // May be ll::lexeme::MISSING_MASTER
	  ll::parser::table::key_table prefix_table );


// Bracketed Subexpression Pass
// --------- ------------- ----

struct block_struct
    // Saves bracketed pass information for a block.
{
    min::int32 indentation_offset;
        // Saved indentation offset.

    min::gen string_concatenator;
        // Saved string_concatenator.
};

typedef min::packed_vec_insptr
            <ll::parser::bracketed::block_struct>
	block_stack;

// Initialize a block stack with given max_length
// and length = 0.
//
void init_block_stack
    ( min::ref<ll::parser::bracketed::block_stack>
	  block_stack,
      min::uns32 max_length = 64 );

inline void push_block
	( ll::parser::bracketed::block_stack
	      block_stack,
	  min::int32 indentation_offset,
	  min::gen string_concatenator )
{
    ll::parser::bracketed::block_struct b =
        { indentation_offset, string_concatenator };
    min::push ( block_stack ) = b;
    min::unprotected::acc_write_update
        ( block_stack, string_concatenator );
}

struct bracketed_pass_struct;
typedef min::packed_struct_updptr<bracketed_pass_struct>
        bracketed_pass;
extern const uns32 & BRACKETED_PASS;
    // Subtype of
    //     min::packed_struct<bracketed_pass_struct>.
struct bracketed_pass_struct
    : public ll::parser::pass_struct
{
    // Packed_struct subtype is BRACKETED_PASS.

    const ll::parser::table::key_table bracket_table;
        // Hash table for brackets and indentation
	// marks.

    const ll::parser::table::key_table prefix_table;
        // Table for prefix separator types.

    int32 indentation_offset;
        // Amount the indentation of a line has to be
	// offset from the indentation of an indented
	// paragraph in order to avoid error messages.
	// Must be signed integer so indentations can
	// be set to - indentation_offset and indenta-
	// tion check computations are signed, but is
	// always >= 0.  Defaults to 2.

    const min::gen string_concatenator;
        // Token value of token used to concatenate
	// two quoted strings during logical line
	// parsing (e.g., #).  Token must have a
	// `value', e.g., be word, separator, or mark.
	// May also take the values:
	//
	//   min::DISABLED()
	//     Quoted strings are never concatenated.
	//
	//   min::ENABLED()
	//     Consecutive quoted strings in a logical
	//     line are concatenated (without any
	//     intervening mark).

    const ll::parser::bracketed::block_stack
            block_stack;
	// Stack of values saved by current parser
	// blocks.

    ll::parser::table::flags trace_subexpressions;
        // Trace flag named `bracketed subexpressions'
	// that traces bracketed subexpressions.

};

MIN_REF ( min::gen, name,
          ll::parser::bracketed::bracketed_pass )
MIN_REF ( ll::parser::parser, parser,
          ll::parser::bracketed::bracketed_pass )
MIN_REF ( ll::parser::pass, next,
          ll::parser::bracketed::bracketed_pass )
MIN_REF ( ll::parser::table::key_table, bracket_table,
          ll::parser::bracketed::bracketed_pass )
MIN_REF ( ll::parser::table::key_table, prefix_table,
          ll::parser::bracketed::bracketed_pass )
MIN_REF ( min::gen, string_concatenator,
          ll::parser::bracketed::bracketed_pass )
MIN_REF ( ll::parser::bracketed::block_stack,
          block_stack,
          ll::parser::bracketed::bracketed_pass )

// Return a new bracketed subexpression pass.
//
ll::parser::pass new_pass ( ll::parser::parser parser );

// Parse Bracketed Subexpression Function
// ----- --------- ------------- --------

// Move `current' to the end of the current bracketed
// subexpression, calling ll::parser::ensure_next if
// more tokens are needed.  The bracketed subexpression
// may be a logical line, an untyped bracketed subex-
// pression, a typed bracketes subexpression or typed
// prefix, or a prefix-n-list.  Other bracketed subex-
// pressions are recognized within the subexpressions
// just listed: namely indented paragraphs, implied
// headers, headed lines, and mapped lexemes.
//
// The parsed subexpression is NOT compacted and tokens
// in it are left untouched with the following excep-
// tions.  Line breaks, comments, indent, and indent
// before comment tokens are deleted.  After doing this,
// quoted strings are merged within a logical line if
// separated by a string concatenator or if consecutive
// when no string concatenator is required.  Bracketed
// sub-subexpressions of the subexpression being parsed
// are converted to single tokens.  Any logical line
// terminating line separator is deleted.
//
// In most cases MISSING_POSITION is returned.  If a
// a logical line terminating line separator was
// deleted, the position of the end of the separator
// is returned.
//
// Sub-subexpressions are identified and each is replac-
// ed by a single BRACKETED, BRACKETABLE, PURELIST, PRE-
// FIX, or DERIVED token.  If a typed bracketed subex-
// pression is being parsed (true iff the typed_data
// argument is not NULL), tokens representing attibute
// labels and values are modified and given temporary
// token types as specified above with the typed_data
// definition.  If a logical line is being parsed (true
// iff the bracketed_stack_p argument is NULL), implied
// header tokens may be inserted.
//
// It is assumed that there are always more tokens
// available via ll::parser::ensure_next until an
// end-of-file token is encountered.  The end-of-file
// token is never part of the bracketed subexpression,
// so there is always a token after the subexpression.
// Similarly the start-of-file token is assumed to be
// present, not be part of any subexpression, and never
// be deleted, so there is always a token before the
// subexpression.  The token immediately after the
// recognized subexpression is returned as the updated
// `current' argument value to mark the end of the
// recognized subexpression.  If this token is an indent
// token, no lexemes beyond the lexeme of this token
// have been read, so the lexical analyser can be
// reconfigured to read the non-blank portion of the
// following line.
//
// The token list, beginning with the initial value of
// `current', is edited by this function.  The caller
// should save `current->previous' before calling this
// function, so it and `current' as returned by this
// function can be used to delimit the subexpression.
// `current->previous' always exists as the start
// of file token is always present.
//
// This function parses a logical line if and only if
// the bracket_stack_p argument is NULL.  When parsing
// logical lines, the line_variables argument is used
// to control parsing.  The logical line ends according
// to the setting of the EAxxx_OPT options and the
// line_variables line_sep and paragraph_indent members.
// An end of file token or an indent token with indent
// less than the paragraph_indent always terminates a
// logical line.
//
// The line_variables also contains several sets of
// line_data which are used while parsing the line.
// The paragraph set is copied to the current set by
// parse_paragraph_element at the beginning of an
// indented paragraph, or after a blank line when the
// previous paragraph header is not CONTINUED or the
// line after the blank line begins with an explicit
// paragraph header.
//
// When parse_paragraph_element calls this function to
// parse paragraph lines, it sets the current line_data
// lexical master to be in effect immediately after any
// indent token just before the logical line, and sets
// the current line_data selectors to be in effect for
// parsing the logical line.
//
// If the current line_data has an implied header, it
// is prepended to the logical line as a prefix header,
// and the logical line is scanned as a prefix-0-list.
// If the line begins with an explicit header in the
// same group as the implicit header, the implicit
// header is deleted, using the rule that an implicit
// header prefix-n-list must always have at least one
// element.
//
// When this function encounters an active untyped
// opening bracket, it calls itself recursively to parse
// an untyped bracketed sub-subexpression, and when the
// recursive call returns, this function first calls
// any reformatting function that is provided for the
// opening bracket, and then if that function requests
// or does not exist, calls the `compact' function to
// compact the sub-subexpression into a BRACKETED token
// with the opening bracket as .initiator and closing
// bracket as .terminator.
//
// When this function is called (recursively) to parse
// an untyped bracketed subexpression, the top of the
// bracket stack (pointed at by the bracket_stack_p
// argument) contains an entry identifying the opening
// bracket of the subexpression.  The parse continues
// until the corresponding closing bracket is found,
// and this is delimited in the top bracket stack entry
// (via the closing_first and closing_next elements)
// and this function returns.  If the closing bracket is
// erroneously omitted, this function returns as if the
// closing bracket existed, and indicates in the top
// bracket stack entry where the closing bracket should
// have been placed (by setting closing_first ==
// closing_next == first token after subexpression).
// The bracket stack entries below the top specify the
// closing brackets which if they occur will close the
// top of the stack if the EAOCLOSING option is in
// effect.  Bracketed subexpressions must be inside a
// logical line, so the end of the logical line also
// closes all bracket stack entries.
//
// Typed bracketed subexpressions are treated similarly
// to untyped bracketed subexpressions, with the follow-
// ing differences.  When called recursively to parse a
// typed bracketed subexpression, the typed_data argu-
// ment is NOT NULL, and is used during the parse and
// also returns information to the caller.  Type and
// attribute labels and values become special tokens
// with types ATTR_... and are moved to the front of
// the list of tokens returned.  When a typed opening
// bracket is encountered and the subsequent recursive
// call is completed, these special tokens are used to
// set the attributes of the compacted sub-subexpres-
// sion.  See typed_data structure above for more
// details on the ATTR_... token types.
//
// Also when parsing a typed bracketed subexpression,
// the selectors are switched when scanning the type
// and attributes parts to the attr_selectors provided
// by the typed opening definition.  The selectors used
// to parse the subexpression elements are the same
// as for an untyped bracketed subexpression: i.e., the
// value of the selectors argument.  The options are NOT
// switched.
//
// When a typed bracketed sub-subexpression is
// compacted, if it has no elements and its opening
// bracket (e.g. '{' with `code' selector) allows
// prefix separators, then the compacted token is given
// the PREFIX token type and that token is a prefix
// separator.  If the token begins a logical line or an
// untyped bracketed subexpression, then it begins a
// prefix-0-list sub-subexpression, which is parsed
// by calling this function recursively with the
// top bracket stack entry specifying the prefix
// token.  Similarly a prefix separator at the beginning
// of a prefix-n-list begins a prefix-(n+1)-list.
// However, note that a prefix separator in a typed
// bracketed subexpression is an error (but generally
// '{' with `data' selector does not allow prefix
// separators so prefix separators can only occur in
// the elements list of a typed bracketed subexpres-
// sion).
//
// A prefix-n-list is ended by the end of a logical
// line or the end of any containing untyped bracketed
// subexpression.  It is continued by discovery of
// another prefix separator with the same .type or
// in the same prefix group as the prefix separator
// identified at the top of the bracket stack.  It
// is ended by discovery of another prefix separator
// whose .type or group is the same as the prefix
// separator identified in bracket stack entry
// between the next to top entry and the first non-
// prefix bracket stack entry or bottom of stack.
// Prefix-n-lists are compacted by the compact_prefix_
// list function.
//
// Indented paragraphs begin when an indentation
// mark is recognized at the end of a physical line.
// The indented paragraph has logical lines if the
// first indent token after the indentation mark
// is indented more than the current paragraph indent,
// and if so, the paragraph indent is set to the
// indent of this token.  The paragraph logical lines
// are parsed and collected into paragraph elements
// by the parse_paragraph_element function, and these
// are compacted into a paragraph by the compact_
// paragraph function.
//
// As line breaks are not deleted until after brackets,
// indentation marks, etc are recognized, multi-lexeme
// brackets etc. are not recognized if they straddle
// a line break.
//
// This function is called at the top level with zero
// indent, parser->selectors, `top_level_indentation_
// mark->line_sep' which is `;', and bracket_stack_p =
// NULL.
//
struct line_data
{
    // Data used to initialize parsing of a logical
    // line.

    min::uns32 lexical_master;
        // The lexical master to be set at the start of
	// the line, or MISSING_MASTER if none.
    ll::parser::table::flags selectors;
        // The selectors to be set at the beginning of
	// the line.
    min::gen implied_header;
        // The header implied at the start of the line,
	// or MISSING if none.
    ll::parser::bracketed::prefix header_entry;
        // The prefix table entry associated with the
	// implied_header, as found using the selectors
	// member above.  NOT USED if implied_header
	// is MISSING.  This is a cache and can be
	// recomputed from implied_header and selectors
	// above.
    ll::parser::table::flags header_selectors;
        // `selectors' above modified by the new_selec-
	// tors member of the header_entry.  Used to
	// parse the prefix-n-list headed by the
	// implied_header if that is not MISSING.  NOT
	// USED if implied_header is MISSING.  This is
	// a cache and can be recomputed from header_
	// entry and selectors above.
};

struct line_variables
    //
    // The line_variables structure contains information
    // about the current logical line being parsed.
    // There is one line_variables structure for each
    // indented paragraph, and its values are used to
    // communicate between the parses of the various
    // logical lines, headed lines, and headed para-
    // graphs of the indented paraagraph.
    //
    // WARNING: this ASSUMES that the parser tables are
    // not modified DURING logical line parsing.  Break-
    // ing this assumption breaks the garbage collector.
{
    // Variables that must be initialized before logical
    // lines are parsed, and are used while parsing a
    // logical line.
    //
    ll::parser::bracketed::line_sep line_sep;
        // Line separator that can end the logical line
	// if EALSEP_OPT is set, or NULL_STUB if none.
    min::int32 paragraph_indent;
        // Paragraph indentation for use in ending
	// paragraph.
    min::int32 indentation_offset;
        // Value of indentation offset to be used in
	// determining end of paragraph.

    // Variables that must be initialized before
    // indented paragraph lines are parsed, and are used
    // by parse_parargraph_element.
    //
    ll::parser::token last_paragraph;
        // Last token whose value has a .type with
	// group `paragraph' that has not be compacted
	// by compact_paragraph, or NULL_STUB if none.
	// Initialized to NULL_STUB.
    bool at_paragraph_end;
        // True if current token is end of file or
	// an indent token with indent < paragarph_
	// indent.  Initialized to false.

    // Variables initialized when parse_bracketed_sub-
    // expression is called to parse a logical line
    // (i.e., with bracketed_stack_p == NULL ):
    //
    ll::parser::token previous;
        // Token just before logical line.  Initialized
	// to current->previous.
    bool at_paragraph_beginning;
        // True iff logical line is in paragraph
	// beginning position.  Initialized to parser->
	// at_paragraph_beginning.

    // Logical line data, used to parse next logical
    // line.  Reset from paragraph data before being
    // used for line in paragraph beginning position
    // unless CONTINUING flag in current.selectors is
    // on.  Reset by explicit paragraph header.
    //
    line_data current;

    // Logical line data copied to `current' just before
    // the later is used for a logical line in paragraph
    // beginning position.  Reset by explicit paragraph
    // header (copied from indentation_paragraph if the
    // explicit paragraph header does NOT have STICKY
    // flag).
    //
    line_data paragraph;

    // Logical line data copied to `current' just after
    // implied paragraph header is inserted at beginning
    // of a logical line.  This data can be derived from
    // the paragraph line data above, and is therefore
    // a cache of that.
    //
    line_data implied_paragraph;

    // Values of paragraph and implied_paragraph data at
    // beginning of current indented paragraph.  Used to
    // reset paragraph and implied_paragraph data when
    // an explicit paragraph header WITHOUT the STICKY
    // flag is found.
    //
    line_data indentation_paragraph;
    line_data indentation_implied_paragraph;
};

struct bracket_stack
{
    // Either opening_bracket != NULL_STUB or prefix_
    // type != MISSING.

    ll::parser::bracketed
              ::opening_bracket opening_bracket;
        // If not NULL_STUB, this identifies the opening
	// bracket whose recognition made this entry.

    min::gen prefix_type;
    ll::parser::bracketed::prefix prefix_entry;
        // If prefix_type is not MISSING, this is the
	// .type of the prefix separator whose recogni-
	// tion made this entry, and the prefix_entry
	// is its entry in the prefix table, or
	// min::NULL_STUB if none.

    ll::parser::token closing_first, closing_next;
        // If these are NULL_STUB, this entry is open.
	// The bracketed subexpression may be closed
	// anyway by a non-indented line or end of file
	// or a found separator.
	//
	// Otherwise if opening_bracket != NULL_STUB,
	// then if first != next, these entries are the
	// first token of the closing bracket that
	// closed this entry, and the next token AFTER
	// this bracket.  But if first == next, the
	// closing bracket that terminated this entry
	// was missing and should be inserted just
	// before next.  In this last case, where first
	// and next are not NULL_STUB, then they are
	// both the first token of the closing bracket
	// that closed this entry (which does NOT match
	// the entry opening bracket).
	//
	// Otherwise if prefix_type != MISSING, then if
	// first != next, these entries are the single
	// PREFIX token that closed this entry and the
	// next token AFTER this PREFIX token, but if
	// first == next, this entry was closed by
	// something other than a PREFIX token with
	// matching prefix_type (it may be closed by a
	// closing_bracket or a PREFIX token that
	// matches a stack entry farther from the top of
	// the stack).

    ll::parser::bracketed::bracket_stack * previous;
        // Previous entry in stack.  Stack is NULL
	// terminated.

    bracket_stack
	    ( ll::parser::bracketed
	                ::bracket_stack * previous )
        : opening_bracket ( min::NULL_STUB ),
	  prefix_type ( min::MISSING() ),
	  prefix_entry ( min::NULL_STUB ),
          closing_first ( min::NULL_STUB ),
          closing_next ( min::NULL_STUB ),
	  previous ( previous ) {}
};

inline bool is_closed ( ll::parser::bracketed
                                  ::bracket_stack * p )
{
    return    p != NULL
           && p->closing_first != min::NULL_STUB;
}

min::position parse_bracketed_subexpression
	( ll::parser::parser parser,
	  ll::parser::table::flags selectors,
	  ll::parser::token & current,
	  ll::parser::bracketed::typed_data
	      * typed_data,
	  ll::parser::bracketed::line_variables
	      * line_variables,
	  bracket_stack * bracket_stack_p  = NULL );

// This function calls parse_bracketed_subexpression
// to parse logical lines, headed lines, and headed
// paragraphs that are elements of indented paragraphs
// or top level output tokens.  One token is returned
// each time this function is called in current->
// previous, unless there are no more elements in the
// paragraph (because the indented paragraph ended or
// an end of file was encountered), in which case no
// tokens are returned.
//
// Before calling this function to get the first element
// of a paragraph (or of the top level), line_variables
// must be allocated and initialized, and the
// CONTINUING_OPT flag in line_variables->current
// .selectors must be cleared.  Also current-previous
// should be saved so it can be determined whether a
// token was returned by this function.
//
// The trace_flags are passed to compact_logical_line
// and compact_paragraph.
//
// This function returns true iff the returned element
// is a logical line constructed from tokens the first
// of which equaled **PARSER**.  Such an element may be
// a special parser command.
//
bool parse_paragraph_element
	( ll::parser::parser parser,
	  ll::parser::token & current,
	  ll::parser::bracketed::line_variables
	      * line_variables,
	  ll::parser::table::flags trace_flags );

// Bracketed Compact Functions
// --------- ------- ---------

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
bool compact_prefix_list
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


} } }

# endif // LL_PARSER_BRACKETED_H
