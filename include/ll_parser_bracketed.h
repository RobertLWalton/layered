// Layered Languages Bracketed Subexpression Parser
//
// File:	ll_parser_bracketed.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Jun 10 03:17:45 EDT 2019
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
//	Bracket Type Table
//	Bracketed Subexpression Pass
//	Parse Bracketed Subexpression Function
//	Bracketed Compact Functions

// Usage and Setup
// ----- --- -----

# ifndef LL_PARSER_BRACKETED_H
# define LL_PARSER_BRACKETED_H

# include <ll_parser.h>

namespace ll { namespace parser { namespace bracketed {


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

extern min::locatable_var<ll::parser::reformatter>
		untyped_reformatter_stack;


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

    uns32 paragraph_lexical_master;
    uns32 line_lexical_master;
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
	  min::uns32 paragraph_lexical_master,
	  min::uns32 line_lexical_master,
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

    ll::parser::table::flags prefix_selectors;
	// A typed bracketed subexpression with this
	// typed opening can be a PREFIX iff the
	// selectors of its context and this have
	// a selector in common: defaults to
	// PAR::ALL_SELECTORS.

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
	  ll::parser::table::flags prefix_selectors,
	  ll::parser::table::key_table bracket_table );

// Data used during typed bracketed subexpression parse.
//
struct typed_data
{
    ll::parser::bracketed::typed_opening typed_opening;
        // Typed_opening read just before the call to
	// parse_bracketed_subexpression.
    ll::parser::table::flags context_selectors;
        // Selectors in the context in which the typed_
	// opening appears.
    ll::parser::table::flags attribute_selectors;
        // Selectors to be used to parse type and
	// attributes.  Also the selectors initially
	// passed as selectors argument to parse_
	// bracketed_subexpression.
    ll::parser::table::flags element_selectors;
        // Selectors to be used to parse elements.
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
	// that type label.  If the type "" was inferred
	// due to an error, or due to the `{|' construc-
	// tion, set to "" (min::empty_str).  MISSING
	// otherwise.  There is a TYPE token for this
	// type iff it is not MISSING or "".
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
//                                    ATTR_REVERSE?
//           | ATTR_LABEL ATTR_FLAGS? ATTR_MULTIVALUE
//                                    ATTR_REVERSE?
//           | ATTR_LABEL ATTR_FLAGS?
//           | ATTR_TRUE
//           | ATTR_FALSE
//
// Note the TYPE token need not be first in the initial
// segment.
//
// TEMPORARY_TT is in ll::parser
//
const min::uns32 TYPE            = TEMPORARY_TT + 0;
    // .type value.  If the .type value is "", there
    // is NO TYPE token.
const min::uns32 ATTR_LABEL      = TEMPORARY_TT + 1;
const min::uns32 ATTR_FLAGS      = TEMPORARY_TT + 2;
const min::uns32 ATTR_VALUE      = TEMPORARY_TT + 3;
const min::uns32 ATTR_MULTIVALUE = TEMPORARY_TT + 4;
const min::uns32 ATTR_REVERSE    = TEMPORARY_TT + 5;
    // Attribute label, flags, value, and reverse
    // label.
const min::uns32 ATTR_TRUE       = TEMPORARY_TT + 6;
const min::uns32 ATTR_FALSE      = TEMPORARY_TT + 7;
    // Attribute label for attribute with TRUE or
    // FALSE value implied.


// Bracket Type Table
// ------- ---- -----

struct bracket_type_struct;
typedef min::packed_struct_updptr<bracket_type_struct>
    bracket_type;
extern const uns32 & BRACKET_TYPE;
    // Subtype of min::packed_struct
    //              <bracket_type_struct>.

struct bracket_type_struct
    : public ll::parser::table::root_struct
{
    // Packed_struct subtype is BRACKET_TYPE.

    ll::parser::table::new_flags new_selectors;

    // Prefix Data
    //
    // [Prefix data could be made into a separate
    // structure pointed at by bracket_type, but it
    // is anticipated that most bracket_types will
    // have prefix data, so this is not done.]
    //
    ll::parser::table::flags prefix_selectors;
    min::gen group;
    min::gen implied_subprefix;
    min::gen implied_subprefix_type;
        // This is just a cache of the .type of
	// the implied prefix, or MISSING if none.
    min::uns32 paragraph_lexical_master;
    min::uns32 line_lexical_master;
        // ll::lexeme::MISSING_MASTER if missing.
    ll::parser::reformatter reformatter;
    ll::parser::reformatter_arguments
	    reformatter_arguments;
	// Reformater and its arguments, or (both) NULL_
	// STUB if none.
};

MIN_REF ( ll::parser::table::root, next,
          ll::parser::bracketed::bracket_type )
MIN_REF ( min::gen, label,
          ll::parser::bracketed::bracket_type )
MIN_REF ( min::gen, group,
          ll::parser::bracketed::bracket_type )
MIN_REF ( min::gen, implied_subprefix,
          ll::parser::bracketed::bracket_type )
MIN_REF ( min::gen, implied_subprefix_type,
          ll::parser::bracketed::bracket_type )
MIN_REF ( ll::parser::reformatter, reformatter,
          ll::parser::bracketed::bracket_type )
MIN_REF ( ll::parser::reformatter_arguments,
	  reformatter_arguments,
          ll::parser::bracketed::bracket_type )

// Create a bracket_type definition entry with given
// label and parameters, and push it into the
// given bracket_type_table.
//
void push_bracket_type
	( min::gen bracket_type_label,
	  ll::parser::table::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  ll::parser::table::new_flags new_selectors,
	  ll::parser::table::flags prefix_selectors,
	  min::gen group,
	  min::gen implied_subprefix,
	  min::gen implied_subprefix_type,
	  min::uns32 paragraph_lexical_master,
	  min::uns32 line_lexical_master,
	      // May be ll::lexeme::MISSING_MASTER
	  ll::parser::reformatter reformatter,
	  ll::parser::reformatter_arguments
	      reformatter_arguments,
	  ll::parser::table::key_table
	      bracket_type_table );

extern min::locatable_var<ll::parser::reformatter>
    bracket_type_reformatter_stack;

// Bracketed Subexpression Pass
// --------- ------------- ----

struct middle_break
    // Middle break begin and end values as UTF-8
    // character strings, and length in bytes of
    // each.  Middle breaks are disabled if lengths
    // are 0.
{
    char       begin[32];
    char       end[32];
    min::uns32 begin_length;
    min::uns32 end_length;
};

struct block_struct
    // Saves bracketed pass information for a block.
{
    min::int32 indentation_offset;
        // Saved indentation offset.

    min::gen string_concatenator;
        // Saved string_concatenator.

    ll::parser::bracketed::middle_break middle_break;
    	// Saved middle break information.
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
	  min::gen string_concatenator,
	  ll::parser::bracketed::middle_break
	      middle_break )
{
    ll::parser::bracketed::block_struct b =
        { indentation_offset,
	  string_concatenator,
	  middle_break };
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

    const ll::parser::table::key_table
	    bracket_type_table;
        // Table for bracket types.

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

    ll::parser::bracketed::middle_break middle_break;
    	// Middle break info.

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
MIN_REF ( ll::parser::table::key_table,
          bracket_type_table,
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

// The parse_bracketed_subexpression function moves
// `current' to the end of a subexpression, calling
// ll::parser::ensure_next if more tokens are needed.
// The bracketed subexpression may be of the kinds
// listed below.  Sub-subexpressions are recognized
// and converted into single tokens of types BRACKETED,
// BRACKETABLE, PURELIST, PREFIX, or DERIVED during the
// scan of the subexpression.  This function calls
// itself recursively to accomplish this.
//
// The parsed subexpression is NOT compacted and tokens
// in it are left untouched with the following excep-
// tions.  Line breaks, indent, and comment tokens are
// deleted.  After doing this, quoted strings are merged
// within a logical line if separated by a string con-
// catenator or if consecutive when no string concatena-
// tor is required.  Sub-subexpressions of the sub-
// expression being scanned are converted to single
// tokens.  Logical line terminating line separators are
// deleted. Implied headers are inserted.  Mapped tokens
// are replaced.  Some special editing of tokens is
// done for typed bracketed subexpressions (see below).
//
// In most cases MISSING_POSITION is returned.  If a
// a logical line terminating line separator was
// deleted, the position of the end of the separator
// is returned.
//
// This function never alters tokens before the value
// of `current' at the time the function is called, and
// assumes there is always at least one such token,
// e.g., the start-file token.  Similarly this function
// never alters the `current' token it returns or tokens
// after this `current' token, and as the end-of-file
// token is never part of the bracketed subexpresion,
// there is always a token after the subexpression.  The
// tokens scanned by this function therefore consist of
// all tokens from the initial `current->previous' token
// through the final `current->previous' token.  This
// token list may be empty in certain cases (e.g.,
// scanning a comment logical line).  The caller of this
// function should save `current->previous' before
// calling this function.
//
// As line breaks are not deleted until after brackets,
// indentation marks, etc are recognized, multi-lexeme
// brackets etc. are not recognized if they straddle
// a line break.
//
// The following kinds of expression can be processed.
// Unless otherwise specified the parsing selectors used
// during the scan are those given by the `selectors'
// argument.
//
// Logical Line Expressions:
//
//     This function is called by top level code to
//     parse logical lines.  All other calls to this
//     function are recursive calls.
//
//     AFTER recognizing the indent before a logical
//     line, or the logical line separator at the
//     end of a previous line, this function is called
//     with `current' equal to the first token after
//     the indent or line separator, and with a line_
//     variables argument and an empty bracket stack
//     (NULL bracket_stack_p).
//
//     This function uses the line_variables argument
//     and options in the selectors argument to specify
//     parameters that are used to detect the end the
//     logical line being scanned, and returns with
//     `current' equal to the indent or end-of-file
//     that ends the logical line.  But if the line is
//     terminated by a line separator, the line
//     separator is deleted by this function and
//     `current' is returned as the token after the
//     line separator.
//
//     If a logical line is terminated by a line
//     separator, the return value of this function is
//     the position of the end of the line separator.
//     In ALL other cases the return value is MISSING_
//     POSITION.
//
//     If a logical line is terminated by an indent
//     token, no lexemes beyond the lexeme of this token
//     have been read, so the lexical analyser can be
//     reconfigured to read the non-blank portion of the
//     following line.
//
//     A logical line is in the paragraph beginning
//     position if it is at the beginning of the input
//     file, the beginning of an indented paragraph,
//     or after a blank line, or if it is separated from
//     such a position only by preceding comment lines.
//
//     Line_variables contains several sets of line_data
//     which are used while parsing the line.  The
//     paragraph set is copied to the current set by
//     parse_paragraph_element at the beginning of a
//     logical line in paragraph beginning position if
//     the CONTINUING_OPT flag in the current line_data
//     (before replacement) is off.  This last flag is
//     set only by the line_data of a paragraph headed
//     by a prefix whose type has the `continuing'
//     option.  See the line_variables description below
//     for more information about the sets of line_data
//     it contains.
//
//     When parse_paragraph_element calls this function
//     to parse a logical line after an indent, it sets
//     the lexical master that will be in effect
//     immediately after the indent token from the
//     current line_data.  The lexical master is set to
//     the current line_data paragraph_lexical_master
//     if the logical line is in paragraph beginning
//     position, and to the line_lexical_master other-
//     wise.  Parse_paragraph_element also passes the
//     current line_data selectors to this function when
//     parsing a logical line.
//
//     If the current line_data has an implied header,
//     it is prepended to the logical line as a prefix
//     header, and the logical line is scanned as a
//     prefix-0-list (see below).  If the line begins
//     with an explicit header in the same group as the
//     implicit header, the implicit header is deleted,
//     using the rule that an implicit header prefix-n-
//     list must always have at least one element.
//
//     Logical lines can contain prefix separators.
//     See prefix-n-lists below.
//
// Indented Paragraph Expressions:
//
//     Indented paragraphs begin when an indentation
//     mark is recognized at the end of a physical line.
//     The indented paragraph has logical lines if the
//     first indent token after the indentation mark
//     is indented more than the current paragraph
//     indent, and if so, the paragraph indent is reset
//     to the indent of this token.  The paragraph
//     logical lines are parsed and collected into
//     paragraph elements by the parse_paragraph_element
//     function, and these are compacted into a para-
//     graph by the compact_paragraph function.
//
// Top Level Logical Lines:
//
//    This function is called at the top level with line
//    variables specifying paragraph_indent as 0,
//    line_sep as `top_level_indentation_mark->line_sep'
//    which is `;', all line_data as having selectors,
//    paragraph_lexical_master, and line_lexical_master
//    taken from the parser (e.g., parser->selectors),
//    and bracket_stack_p = NULL.  After processing any
//    parsing commands (i.e., a **PARSE** paragraph),
//    the line_data are reset to the parser values
//    (e.g., parser->selectors).
//
// Untyped Bracketed Expressions:
//
//     AFTER recoginizing the opening bracket and moving
//     `current' to the following token, this function
//     is called with a top bracket_stack entry that has
//     the opening bracket parser table entry as its
//     `opening_bracket' value.  This function normally
//     returns with `current' equal to the first token
//     of the corresponding closing bracket and the
//     tokens of the closing bracket delimited in the
//     bracket stack entry by closing_first and closing_
//     next.  If the expression terminates without a
//     closing bracket because that was omitted in the
//     input and the omission was detected, then this
//     function returns with closing_first == closing_
//     next == the token before which the omitted
//     closing bracket should be inserted.
//
//     This call uses the line_variables argument and
//     options in the selectors argument to specify
//     parameters that are used to detect the end the
//     logical line in which the bracketed expression
//     occurs.  A bracketed expression CANNOT cross
//     logical lines, and the end of the current logical
//     line signals an omitted closing bracket.
//
//     The EALSEP_OPT flag is removed by the new_
//     selectors member of any opening bracket parser
//     table entry, so line separators cannot end
//     logical lines inside bracketed subexpressions.
//
//     If a closing bracket is encountered that matches
//     the opening bracket in a bracket_stack entry
//     that is NOT the top of the bracket_stack, and the
//     EAOCLOSING option is set in the selectors argu-
//     ment, the closing bracket for the subexpression
//     being scanned has been omitted and should be
//     inserted before the closing bracket that was
//     found.
//
//     For untyped bracketed sub-subexpressions this
//     function calls itself recursively with selectors
//     modified by the sub-subexpression opening bracket
//     parser table entry new_selectors member.  This
//     function then removes the opening and any closing
//     brackets of the sub-subexpression, applies any
//     reformatter specified by the opening bracket
//     parser table entry of the sub-subexpression, and
//     if there is no reformatter, or if requested by
//     the reformatter, compacts the sub-subexpression
//     making its opening bracket its .initiator and its
//     closing bracket (even if omitted) its .termina-
//     tor.  Compact is called with BRACKETING token
//     type and the selectors used to scan the sub-sub-
//     expression.
//
//     Untypes bracketed subexpressions can contain
//     prefix separators.  See prefix-n-lists below.
//
// Typed Bracketed Expressions:
//
//     These are like to untyped bracketed expressions
//     but with the following differences.
//
//     When this function is called to parse a typed
//     bracketed subexpression, the typed_data argument
//     is NOT NULL, and is used during the parse and
//     also returns information to the caller.  Type and
//     attribute labels and values become special tokens
//     with types ATTR_... and are moved to the front of
//     the list of tokens returned, as specified above
//     with the typed_data definition.  When a typed
//     opening bracket is encountered and the subsequent
//     recursive call to this function is completed,
//     these special tokens are used to set the attri-
//     butes of the compacted sub-subexpression.  See
//     typed_data structure above for more details on
//     the ATTR_... token types.
//
//     Also when parsing a typed bracketed subexpres-
//     sion, the selectors are switched when scanning
//     the type and attributes parts to the attr_
//     selectors provided by the typed opening defini-
//     tion.  The selectors used to parse the subexpres-
//     sion elements are the same as for an untyped
//     bracketed subexpression: i.e., the value of the
//     selectors argument to this function.  The options
//     are NOT switched.
//
//     When a typed bracketed sub-subexpression is
//     compacted, if it has no elements and its opening
//     bracket (e.g. '{' with `code' selector) allows
//     prefix separators, then the compacted token is
//     given the PREFIX token type and that token is a
//     prefix separator.  As soon as a PREFIX token is
//     created, this function calls itself recursively
//     to parse the prefix-n-list sub-subexpression
//     headed by the PREFIX token, see prefix-n-lists
//     below.
//
//     However typed bracketed subexpressions CANNOT
//     themselves contain PREFIX tokens or prefix-n-
//     lists.
//
// Prefix-N-List Expressions:
//
//     When a PREFIX token is recognized, or one is
//     implicitly inserted, this function calls itself
//     recursively to scan the part of the prefix-n-list
//     headed by the PREFIX token that is before the
//     next PREFIX token or the end of the prefix-n-
//     list.  Before calling itself the PREFIX token and
//     its prefix group are pushed into the bracket
//     stack.
//
//     The call to scan part of the prefix-n-list is
//     ended by the end of a logical line, by the end
//     of any containing untyped bracketed subexpres-
//     sion, or by encountering another PREFIX token
//     that has a group equal to a bracket stack entry
//     that is after any untyped bracket entry in the
//     stack.
//
//     Upon return from a call to scan part of a prefix-
//     n-list, the part is compacted by the compact_
//     prefix_list function.
//
struct line_data
{
    // Data used to initialize parsing of a logical
    // line.

    min::uns32 paragraph_lexical_master;
    min::uns32 line_lexical_master;
        // The lexical masters to be set at the start of
	// the line, or MISSING_MASTER if none.  The
	// paragraph lexical master is used if the line
	// is in paragraph beginning position, and the
	// line lexical master is used otherwise.
    ll::parser::table::flags selectors;
        // The selectors to be set at the beginning of
	// the line.
    min::gen implied_header;
        // The header implied at the start of the line,
	// or MISSING if none.
    ll::parser::bracketed::bracket_type header_entry;
        // The bracket type table entry associated with
	// the implied_header, as found using the
	// selectors member above.  NOT USED if implied_
	// header is MISSING.  This is a cache and can
	// be recomputed from implied_header and
	// selectors above.
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
    // graphs of the indented paragraph.
    //
    // WARNING: this ASSUMES that the parser tables are
    // not modified DURING logical line parsing.  Break-
    // ing this assumption breaks the garbage collector.
    //
    // Also, sticky paragraph.implied headers need to be
    // copied to `sticky_header' element in order to
    // protect them from garbage collection.
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
    bool at_paragraph_indent;
        // First token of logical line is at the
	// paragraph indent (i.e., immediately after
	// an indent token at that indent).  Used to
	// detect erroneous comment at beginning
	// of non-comment logical line that does
	// not follow a line separator.

    // Variables that must be initialized before
    // indented paragraph lines are parsed, and are used
    // by parse_paragraph_element.
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
	// to current->previous by parse_bracketed_
	// subexpression.
    bool at_paragraph_beginning;
        // True iff logical line is in paragraph
	// beginning position.  Initialized to parser->
	// at_paragraph_beginning by parse_bracketed_
	// subexpression.

    // Copy of STICKY paragraph.implied_header below,
    // made to protect it from garbage collection.
    //
    min::locatable_gen sticky_header;

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

    ll::parser::token prefix;
    min::gen prefix_group;
        // If prefix is not min::NULL_STUB, this is the
	// prefix separator whose recognition made this
	// entry and prefix_group is its group.

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
          prefix ( min::NULL_STUB ),
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
// .selectors must be cleared.  Also current->previous
// should be saved so it can be used to determined
// whether a token was returned by this function.
//
// When this function parses a logical line, it compacts
// it with a call to compact_logical_line.
//
// When this function has parsed all the lines of a
// headed paragraph, it compacts that paragraph with a
// call to compact_paragraph.
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
// If separator_found is true, set the .terminator
// attribute of the expanded PREFIX to separator
// and update the end position of the expanded PREFIX
// token to separator_found.
//
// If the first token is an IMPLIED_HEADER or IMPLIED_
// PREFIX token, the value of this token is replaced by
// a copy and then treated as if it were a PREFIX token.
//
// If the only element of the result would be a
// PURELIST, then instead of adding the PURELIST as a
// single element to the result, add all the elements of
// the PURELIST (effectively merging the PURELIST and
// the PREFIX).
//
void compact_prefix_list
	( ll::parser::parser parser,
	  ll::parser::pass pass,
	  ll::parser::table::flags selectors,
	  ll::parser::token first,
	  ll::parser::token next,
	  const min::position & separator_found,
	  min::gen separator,
	  ll::parser::table::flags trace_flags );

} } }

# endif // LL_PARSER_BRACKETED_H
