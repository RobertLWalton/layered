// Layered Languages Bracketed Subexpression Parser
//
// File:	ll_parser_bracketed.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu Apr 13 02:54:09 EDT 2017
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

    const ll::parser::bracketed::opening_bracket
          opening_bracket;
        // The opposing bracket of the closing bracket.
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
MIN_REF ( ll::parser::bracketed::opening_bracket,
          opening_bracket,
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

    const ll::parser::bracketed::indentation_mark
    	    indentation_mark;
        // The indentation_mark for which this is a
	// separator.
};

MIN_REF ( ll::parser::table::root, next,
          ll::parser::bracketed::line_sep )
MIN_REF ( min::gen, label,
          ll::parser::bracketed::line_sep )
MIN_REF ( ll::parser::bracketed::indentation_mark,
          indentation_mark,
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

    const ll::parser::bracketed::typed_opening
          typed_opening;
        // The opening bracket for the typed middle.
};

struct typed_double_middle_struct : 
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is TYPED_DOUBLE_MIDDLE.

    const ll::parser::bracketed::typed_opening
          typed_opening;
        // The opening bracket for the typed double
	// middle.
};

struct typed_attr_begin_struct : 
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is TYPED_ATTR_BEGIN.

    const ll::parser::bracketed::typed_opening
          typed_opening;
        // The opening bracket for the attr_begin.
};

struct typed_attr_equal_struct : 
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is TYPED_ATTR_EQUAL.

    const ll::parser::bracketed::typed_opening
          typed_opening;
        // The opening bracket for the attr_equal.
};

struct typed_attr_sep_struct : 
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is TYPED_ATTR_SEP.

    const ll::parser::bracketed::typed_opening
          typed_opening;
        // The opening bracket for the attr_
	// separator.
};

struct typed_attr_negator_struct : 
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is TYPED_ATTR_NEGATOR.

    const ll::parser::bracketed::typed_opening
          typed_opening;
        // The opening bracket for the attr_
	// negator.
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
MIN_REF ( ll::parser::bracketed::typed_opening,
          typed_opening,
          ll::parser::bracketed::typed_middle )

MIN_REF ( ll::parser::table::root, next,
          ll::parser::bracketed::typed_double_middle )
MIN_REF ( min::gen, label,
          ll::parser::bracketed::typed_double_middle )
MIN_REF ( ll::parser::bracketed::typed_opening,
          typed_opening,
          ll::parser::bracketed::typed_double_middle )

MIN_REF ( ll::parser::table::root, next,
          ll::parser::bracketed::typed_attr_begin )
MIN_REF ( min::gen, label,
          ll::parser::bracketed::typed_attr_begin )
MIN_REF ( ll::parser::bracketed::typed_opening,
          typed_opening,
          ll::parser::bracketed::typed_attr_begin )

MIN_REF ( ll::parser::table::root, next,
          ll::parser::bracketed::typed_attr_equal )
MIN_REF ( min::gen, label,
          ll::parser::bracketed::typed_attr_equal )
MIN_REF ( ll::parser::bracketed::typed_opening,
          typed_opening,
          ll::parser::bracketed::typed_attr_equal )

MIN_REF ( ll::parser::table::root, next,
          ll::parser::bracketed::typed_attr_sep )
MIN_REF ( min::gen, label,
          ll::parser::bracketed::typed_attr_sep )
MIN_REF ( ll::parser::bracketed::typed_opening,
          typed_opening,
          ll::parser::bracketed::typed_attr_sep )

MIN_REF ( ll::parser::table::root, next,
          ll::parser::bracketed
	            ::typed_attr_negator )
MIN_REF ( min::gen, label,
          ll::parser::bracketed
	            ::typed_attr_negator )
MIN_REF ( ll::parser::bracketed::typed_opening,
          typed_opening,
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
// TBD
//
// It is assumed that there are always more tokens
// available via ll::parser::ensure_next until an end-of-file
// token is encountered, and the end-of-file is never
// part of the bracketed subexpression.  Therefore there
// is always a token immediately after the recognized
// subexpression.  This token is returned as the updated
// `current' argument value to mark the end of the
// recognized subexpression.  If this token is an indent
// token, no lexemes beyond the lexeme of this token
// have been read, so the lexical analyser can be
// reconfigured to read the non-blank portion of the
// current line.
//
// The token list, beginning with the initial value of
// `current', is edited by this function.  The caller
// should save `current->previous' before calling this
// function, so it and `current' as returned by this
// function can be used to delimit the subexpression.
// `current->previous' always exists as the start
// of file token is always present.
//
// The `bracket_stack' argument specifies bracketed or
// prefix separator subexpressions that need to be
// closed.  When an entry in this stack is made, the
// entry is considered to be `open'.  When a closing
// bracket or prefix separator corresponding to one of
// these entries is recognized, that entry, and any
// other entries between that entry and the top of the
// stack are marked as `closed'.  The entries toward
// the top of the stack represent inner subexpressions,
// so discovery of a closing bracket or prefix separator
// can close more than one stack entry.
//
// A prefix separator can only close prefix separator
// entries between the top of the stack and the topmost
// bracket entry.
//
// A closing bracket can close prefix separator entries
// and also bracket entries whose closing brackets are
// missing.
//
// An end of file, a recognized line separator matching
// the `line_sep' argument, a non-indented line, or a
// blank line can cause this function to return WITHOUT
// closing ANY bracket stack entries, depending upon
// parsing option settings.  The sequence of tokens
// between the initial value of current->previous and
// the final value of current is called the returned
// token sequence, and may or may not be empty.  Given
// this, a return without any closed bracket stack
// entries may be caused by the following:
//
//   (1) An end file.  If the EAPBREAK (end at paragraph
//       break) option is on, PARAGRAPH_END is returned;
//       else MISSING_POSITION is returned.  The return-
//       ed token sequence may be empty.  `current' is
//       the end of file token.
//
//   (2) If a blank line is encountered when the
//       returned token sequence is NOT empty, and the
//       EAPBREAK option is on, PARAGRAPH_END is
//       returned.  `current' is the first indent token
//       after the blank line.
//
//   (3) If an indent token is encountered with indent
//       == the paragraph_indent and the EALEINDENT (end
//       at less than or equal to indent) option is on,
//       MISSING_POSITION is returned.  The returned
//       token sequence may be empty.  `current' is the
//       indent token.
//
//   (4) If an indent token is encountered with indent
//       < the paragraph_indent and the EALTINDENT (end
//       at less than indent) option OR the EALEINDENT
//       option is on, MISSING_POSITION is returned.
//       The returned token sequence may be empty.
//       `current' is the indent token.
//
//   (5) If a line separator equal to the line_sep
//       argument is encountered and the EALSEP (end at
//       line separator) option is on, the end position
//       of the line separator is returned.  The
//       returned token sequence may be empty.  The line
//       separator is deleted, and `current' is set to
//       immediately after its previous position.
//
// Closing brackets are recognized if and only if their
// corresponding opening brackets are in the bracket
// stack.  Prefix separators are recognized as closing
// subexpressions only if a prefix separator with the
// same .type appears in the bracket stack above any
// bracket entry in that stack.
//
// This function calls itself recursively if it finds
// an opening bracket, or an indentation mark, or a
// subexpression beginning prefix separator.  The
// `selectors' argument determines which opening bracket
// and indentation mark definitions are active.  The
// `selectors' argument has no affect on closing bracket
// or line separator recognition.
//
// If the subexpression is a typed bracketed subexpres-
// sion, the `typed_data' argument is not NULL and
// contains the typed_opening that prefixes the sub-
// expression, `current' is the first token after this
// typed_opening, the normal selectors have been saved
// in typed_data->saved_selectors and the non-option
// part of the normal selectors has been replaced by
// typed_data->typed_opening->attr_selectors in the
// selectors argument.  The selectors argument and
// typed_data->saved_selectors are switched by every
// typed_middle corresponding to the typed_opening
// scanned while scanning the subexpression, so the
// selectors argument with attr_selectors is used to
// scan types and attributes, and saved_selectors is
// used to scan list elements.  Selectors have no
// affect on recognition of keys such as typed_middle
// and typed_attr_....
//
// When this function calls itself recursively to parse
// a bracketed sub-subexpression, upon return it calls
// any reformatter associated with the opening_bracket
// that started the sub-subexpression, and either the
// reformatter or a subsequence call to `compact' wraps
// all the tokens of the sub-subexpression into a single
// BRACKETED, BRACKATABLE, PURELIST, PREFIX, or DERIVED
// token (even if this is an empty list).  `Compact'
// also converts string tokens (quoted strings and num-
// erics) in the result as per `convert_token'.  If the
// opening_bracket is not a typed_opening, the resulting
// MIN object is given the opening and closing brackets
// as its .initiator and .terminator, and is NOT given
// a .type attribute, unless the reformatter creates its
// own MIN object (e.g., the special reformatter creates
// a special value).  If the opening_bracket is a
// typed_opening, the MIN object is given a .type
// attribute, but NO .initiator or .terminator.
//
// Sub-subexpressions introduced by an indentation mark
// are converted to a list of lists.  The outer list
// is a list of lines and has the indentation mark label
// as its .initiator and min::INDENTED_PARAGRAPH() as
// its .terminator.  The inner lists are paragraph line
// subexpressions and have min::LOGICAL_LINE() as their
// .initiator and "\n" or the line ending line separator
// as their .terminator.  Inner lists that would be
// empty but for their .initiator and .terminator are
// deleted.
//
// As line breaks are not deleted until after brackets,
// indentation marks, etc are recognized, multi-lexeme
// brackets etc. are not recognized if they straddle
// a line break.
//
// This function is called at the top level with zero
// indent, parser->selectors, `top_level_indentation_
// mark->line_sep' which is `;', and bracket_stack =
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

    // Variables that must be initialized before
    // indented paragraph lines are parsed.
    //
    ll::parser::bracketed::line_sep line_sep;
        // Line separator that can end the logical line
	// if EALSEP_OPT is set, or NULL_STUB if none.
    ll::parser::token last_paragraph;
        // Last token whose value has a .type with
	// group `paragraph' that has not be compacted
	// by compact_paragraph, or NULL_STUB if none.
	// Initialized to NULL_STUB.
    min::int32 paragraph_indent;
        // Paragraph indentation for use in ending
	// paragraph.
    min::int32 indentation_offset;
        // Value of indentation offset to be used in
	// determining end of paragraph.
    bool at_paragraph_end;
        // True if current token is end of file or
	// an indent token with indent < paragarph_
	// indent.  Initialized to false.

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

    ll::parser::bracketed::line_variables *
	    line_variables;
        // Line variables at time opening bracket
	// or prefix described above was found.

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
          line_variables ( NULL ),
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

} } }

# endif // LL_PARSER_BRACKETED_H
