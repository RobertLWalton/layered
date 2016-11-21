// Layered Languages Bracketed Subexpression Parser
//
// File:	ll_parser_bracketed.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Nov 21 07:37:09 EST 2016
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

    min::gen implied_paragraph_header;

    uns32 lexical_master;
        // ll::lexeme::MISSING if missing.
};

MIN_REF ( ll::parser::table::root, next,
          ll::parser::bracketed::indentation_mark )
MIN_REF ( min::gen, label,
          ll::parser::bracketed::indentation_mark )
MIN_REF ( ll::parser::bracketed::line_sep,
          line_sep,
          ll::parser::bracketed::indentation_mark )
MIN_REF ( min::gen, implied_paragraph_header,
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
	  min::gen implied_paragraph_header,
	      // May be min::MISSING()
	  min::uns32 lexical_master,
	      // May be ll::lexeme::MISSING
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
    ll::parser::token elements;
        // First token of elements, or NULL_STUB if
	// there are no elements.
    ll::parser::token attributes;
        // First token of the list of attributes after
	// the last typed opening or middle, or NULL_
	// STUB if none.
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
    min::uns32 subtype;
        // Subtype of last key found.
    min::gen type;
        // If typed_opening is followed immediately by a
	// mark, the value of that mark.  If it is fol-
	// lowed by a non-mark type label, the value of
	// that type label.  MISSING otherwise.
    bool has_mark_type;
        // True if and only if typed_opening is followed
	// by a mark, which becomes the type.
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
    min::uns32 lexical_master;
};

MIN_REF ( ll::parser::table::root, next,
          ll::parser::bracketed::prefix )
MIN_REF ( min::gen, label,
          ll::parser::bracketed::prefix )
MIN_REF ( min::gen, group,
          ll::parser::bracketed::prefix )
MIN_REF ( min::gen, implied_subprefix,
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
	  min::uns32 lexical_master,
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

    min::gen string_concatenator;
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
// subexpression, calling parser->input if more tokens
// are needed.  Return the end position of any line
// separator found (the line separator is deleted), or
// PARAGRAPH_END if current token is an indent token
// preceeded by a blank line with possible intervening
// comment lines or an end of file, or min::MISSING_
// POSITION otherwise.  Note that the first two returns
// only occur if they are enabled by parsing options.
//
// The parsed subexpression is NOT compacted and tokens
// in it are left untouched with the following excep-
// tions.  Line breaks, comments, indent, and indent
// before comment tokens are deleted.  After doing this,
// consecutive quoted strings are merged if the second
// is in the same line as the first or in a continuation
// line following the line of the first.  Any subexpres-
// sion terminating line separator is also deleted.
// Sub-subexpressions are identified and each is replac-
// ed by a single BRACKETED, BRACKETABLE, PURELIST,
// PREFIX, or DERIVED token.  If the typed_data argu-
// ment is not NULL, tokens representing attibute labels
// and values are modified and given temporary token
// types as given above with the typed_data definition.
// If the line_variables argument is not NULL, a PREFIX
// token that is a copy of the default line_prefix may
// be inserted at the beginning of a line and used by
// the prefix parsing pass to return a BRACKETED MIN
// object.
//
// It is assumed that there are always more tokens
// available via parser->input until an end-of-file
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
//       == the indent argument and the EALEINDENT (end
//       at less than or equal to indent) option is on,
//       MISSING_POSITION is returned.  The returned
//       token sequence may be empty.  `current' is the
//       indent token.
//
//   (4) If an indent token is encountered with indent
//       < the indent argument and the EALTINDENT (end
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
extern const min::position PARAGRAPH_END;

struct bracket_stack
{
    // Either opening_bracket != NULL_STUB or prefix_
    // type != MISSING.

    ll::parser::bracketed
              ::opening_bracket opening_bracket;
        // If not NULL_STUB, this identifies the opening
	// bracket whose recognition made this entry.

    min::gen prefix_type;
    min::gen prefix_group;
        // If not MISSING, these identify the .type of
	// the prefix separator whose recognition made
	// this entry and the group of this type (if
	// any).

    ll::parser::token closing_first, closing_next;
        // If these are NULL_STUB, this entry is open.
	// The bracketed subexpression may be closed
	// anyway by a non-indented line or end of file.
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
	  prefix_group ( min::MISSING() ),
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

struct line_variables
    // This can ONLY appear IN THE STACK as it contains
    // locatable_gen elements.
    //
    // A non-NULL line_variables pointer argument is
    // passed to parse_bracketed_subexpression ONLY if
    // the latter is being asked to delimit a logical
    // line.  The data in line_variables can be modified
    // by such a call to parse_bracketed_subpexression
    // for use in subsequent parsing.
{
    // The selectors argument to parse_bracketed_sub-
    // expression is the selectors and options set by
    // the indentation mark for the indented paragraph
    // containing the line.  These are also referred
    // to as the paragraph selectors.

    min::uns32 paragraph_master;
    min::uns32 line_master;
        // Lexical master table index to be used to
	// scan a logical line in paragraph beginning
	// or non-paragraph beginning position,
	// respectively.  Default to 0.

    min::locatable_gen paragraph_header;
        // Default header for paragraph with no
	// explicit bracketed or lexical header.
	// Defaults to MISSING.
    min::locatable_gen line_header;
        // Default header for line with no
	// explicit bracketed or lexical header.
	// Defaults to MISSING.

    ll::parser::table::flags header_selectors;
        // Selectors and options set by paragraph
	// header.  Defaults to paragraph selectors.
    ll::parser::table::flags line_selectors;
        // Selectors and options set by line header.
	// Defaults to header selectors.

    min::uns32 instructions;
        // Optional line beginning algorithm instruc-
	// tions (e.g. PARAGRAPH_KEEP).  Defaults to 0.
};

min::position parse_bracketed_subexpression
	( ll::parser::parser parser,
	  ll::parser::table::flags selectors,
	  ll::parser::token & current,
	  min::int32 indent,
	  ll::parser::bracketed::line_sep
	      line_sep,
	  ll::parser::bracketed::typed_data
	      * typed_data,
	  ll::parser::bracketed::line_variables
	      * line_variables,
	  bracket_stack * bracket_stack_p  = NULL );

} } }

# endif // LL_PARSER_BRACKETED_H
