// Layered Languages Bracketed Subexpression Parser
//
// File:	ll_parser_bracketed.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Jul 13 07:25:28 EDT 2015
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
enum {
    // Option Flags
    //
    FULL_LINES		= 1
};
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

    uns32 options;
        // Option flags, e.g., FULL_LINES.

};
struct closing_bracket_struct : 
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is CLOSING_BRACKET.

    const ll::parser::bracketed::opening_bracket
          opening_bracket;
        // The opposing bracket of the closing bracket.
};

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
	  uns32 options,
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

struct line_separator_struct;
typedef min::packed_struct_updptr
	    <line_separator_struct>
        line_separator;
extern const uns32 & LINE_SEP;
    // Subtype of min::packed_struct
    //		       <line_separator_struct>.

struct indentation_mark_struct : 
	public ll::parser::table::root_struct
{
    ll::parser::table::new_flags new_selectors;

    const ll::parser::bracketed::line_separator
	    line_separator;
};

MIN_REF ( min::gen, label,
          ll::parser::bracketed::indentation_mark )
MIN_REF ( ll::parser::bracketed::line_separator,
          line_separator,
          ll::parser::bracketed::indentation_mark )

struct line_separator_struct : 
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is LINE_SEP.

    const ll::parser::bracketed::indentation_mark
    	    indentation_mark;
        // The indentation_mark for which this is a
	// separator.
};

MIN_REF ( min::gen, label,
          ll::parser::bracketed::line_separator )
MIN_REF ( ll::parser::bracketed::indentation_mark,
          indentation_mark,
          ll::parser::bracketed::line_separator )

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

// The following are put in a key_table that is private
// to a typed_opening.
//
struct typed_middle_struct;
typedef min::packed_struct_updptr
	    <typed_middle_struct>
        typed_middle;
extern const uns32 & TYPED_MIDDLE;
    // Subtype of min::packed_struct
    //		       <typed_middle_struct>.

struct typed_attribute_begin_struct;
typedef min::packed_struct_updptr
	    <typed_attribute_begin_struct>
        typed_attribute_begin;
extern const uns32 & TYPED_ATTR_BEGIN;
    // Subtype of min::packed_struct
    //		       <typed_attribute_begin_struct>.

struct typed_attribute_equal_struct;
typedef min::packed_struct_updptr
	    <typed_attribute_equal_struct>
        typed_attribute_equal;
extern const uns32 & TYPED_ATTR_EQUAL;
    // Subtype of min::packed_struct
    //		       <typed_attribute_equal_struct>.

struct typed_attribute_separator_struct;
typedef min::packed_struct_updptr
	    <typed_attribute_separator_struct>
        typed_attribute_separator;
extern const uns32 & TYPED_ATTR_SEP;
    // Subtype of
    //     min::packed_struct
    //	       <typed_attribute_separator_struct>.

struct typed_attribute_negator_struct;
typedef min::packed_struct_updptr
	    <typed_attribute_negator_struct>
        typed_attribute_negator;
extern const uns32 & TYPED_ATTR_NEGATOR;
    // Subtype of
    //     min::packed_struct
    //	       <typed_attribute_negator_struct>.

struct typed_attribute_flags_opening_struct;
typedef min::packed_struct_updptr
	    <typed_attribute_flags_opening_struct>
        typed_attribute_flags_opening;
extern const uns32 & TYPED_ATTR_FLAGS_OPENING;
    // Subtype of
    //     min::packed_struct
    //	       <typed_attribute_flags_opening_struct>.

struct typed_attribute_flags_separator_struct;
typedef min::packed_struct_updptr
	    <typed_attribute_flags_separator_struct>
        typed_attribute_flags_separator;
extern const uns32 & TYPED_ATTR_FLAGS_SEP;
    // Subtype of
    //     min::packed_struct
    //	       <typed_attribute_flags_separator_struct>.

struct typed_attribute_flags_closing_struct;
typedef min::packed_struct_updptr
	    <typed_attribute_flags_closing_struct>
        typed_attribute_flags_closing;
extern const uns32 & TYPED_ATTR_FLAGS_CLOSING;
    // Subtype of
    //     min::packed_struct
    //	       <typed_attribute_flags_closing_struct>.

struct typed_attribute_multivalue_opening_struct;
typedef min::packed_struct_updptr
	    <typed_attribute_multivalue_opening_struct>
        typed_attribute_multivalue_opening;
extern const uns32 & TYPED_ATTR_MULTIVALUE_OPENING;
    // Subtype of
    //   min::packed_struct
    //	   <typed_attribute_multivalue_opening_struct>.

struct typed_attribute_multivalue_separator_struct;
typedef min::packed_struct_updptr
	   <typed_attribute_multivalue_separator_struct>
        typed_attribute_multivalue_separator;
extern const uns32 &
	TYPED_ATTR_MULTIVALUE_SEP;
    // Subtype of
    //   min::packed_struct
    //	  <typed_attribute_multivalue_separator_struct>.

struct typed_attribute_multivalue_closing_struct;
typedef min::packed_struct_updptr
	    <typed_attribute_multivalue_closing_struct>
        typed_attribute_multivalue_closing;
extern const uns32 & TYPED_ATTR_MULTIVALUE_CLOSING;
    // Subtype of
    //   min::packed_struct
    //	   <typed_attribute_multivalue_closing_struct>.

struct typed_opening_struct :
	public ll::parser::bracketed
	                 ::opening_bracket_struct
{
    // Packed_struct subtype is TYPED_OPENING.
    // Typed_opening is closed by closing_bracket.
    
    const ll::parser::table::key_table key_table;
        // This key_table contains the following compo-
	// nents which are used by the typed_bracketed_
	// reformatter when it scans the contents of a
	// typed bracketed subexpression.

    // The following components are recorded here only
    // so they can be output by the parser print
    // command.  They are NOT used during parsing -
    // the key_table is used instead.
    //
    const ll::parser::bracketed::typed_middle
          typed_middle;
    const ll::parser::bracketed::typed_attribute_begin
          typed_attribute_begin;
    const ll::parser::bracketed::typed_attribute_equal
          typed_attribute_equal;
    const ll::parser::bracketed
                    ::typed_attribute_separator
          typed_attribute_separator;
    const ll::parser::bracketed
                    ::typed_attribute_negator
          typed_attribute_negator;
    const ll::parser::bracketed
                    ::typed_attribute_flags_opening
          typed_attribute_flags_opening;
    const ll::parser::bracketed
                    ::typed_attribute_flags_separator
          typed_attribute_flags_separator;
    const ll::parser::bracketed
                    ::typed_attribute_flags_closing
          typed_attribute_flags_closing;
    const ll::parser::bracketed
            ::typed_attribute_multivalue_opening
          typed_attribute_multivalue_opening;
    const ll::parser::bracketed
            ::typed_attribute_multivalue_separator
          typed_attribute_multivalue_separator;
    const ll::parser::bracketed
            ::typed_attribute_multivalue_closing
          typed_attribute_multivalue_closing;
};

struct typed_middle_struct : 
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is TYPED_MIDDLE.

    const ll::parser::bracketed::typed_opening
          typed_opening;
        // The opening bracket for the typed middle.
};

struct typed_attribute_begin_struct : 
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is TYPED_ATTR_BEGIN.

    const ll::parser::bracketed::typed_opening
          typed_opening;
        // The opening bracket for the attribute_begin.
};

struct typed_attribute_equal_struct : 
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is TYPED_ATTR_EQUAL.

    const ll::parser::bracketed::typed_opening
          typed_opening;
        // The opening bracket for the attribute_equal.
};

struct typed_attribute_separator_struct : 
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is TYPED_ATTR_SEP.

    const ll::parser::bracketed::typed_opening
          typed_opening;
        // The opening bracket for the attribute_
	// separator.
};

struct typed_attribute_negator_struct : 
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is TYPED_ATTR_NEGATOR.

    const ll::parser::bracketed::typed_opening
          typed_opening;
        // The opening bracket for the attribute_
	// negator.
};

struct typed_attribute_flags_opening_struct : 
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is TYPED_ATTR_FLAGS_
    // OPENING.

    const ll::parser::bracketed::typed_opening
          typed_opening;
        // The opening bracket for the attribute_
	// flags_opening.
};

struct typed_attribute_flags_separator_struct : 
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is TYPED_ATTR_FLAGS_SEP.

    const ll::parser::bracketed::typed_opening
          typed_opening;
        // The opening bracket for the attribute_
	// flags_separator.
};

struct typed_attribute_flags_closing_struct : 
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is TYPED_ATTR_FLAGS_
    // CLOSING.

    const ll::parser::bracketed::typed_opening
          typed_opening;
        // The opening bracket for the attribute_
	// flags_closing.
};

struct typed_attribute_multivalue_opening_struct : 
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is TYPED_ATTR_MULTIVALUE_
    // OPENING.

    const ll::parser::bracketed::typed_opening
          typed_opening;
        // The opening bracket for the attribute_
	// multivalue_opening.
};

struct typed_attribute_multivalue_separator_struct : 
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is TYPED_ATTR_MULTIVALUE_
    // SEP.

    const ll::parser::bracketed::typed_opening
          typed_opening;
        // The opening bracket for the attribute_
	// multivalue_separator.
};

struct typed_attribute_multivalue_closing_struct : 
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is TYPED_ATTR_MULTIVALUE_
    // CLOSING.

    const ll::parser::bracketed::typed_opening
          typed_opening;
        // The opening bracket for the attribute_
	// multivalue_closing.
};

MIN_REF ( min::gen, label,
          ll::parser::bracketed::typed_opening )
MIN_REF ( ll::parser::table::key_table,
          key_table,
          ll::parser::bracketed::typed_opening )
MIN_REF ( ll::parser::bracketed::typed_middle,
          typed_middle,
          ll::parser::bracketed::typed_opening )
MIN_REF ( ll::parser::bracketed::closing_bracket,
          closing_bracket,
          ll::parser::bracketed::typed_opening )
MIN_REF ( ll::parser::bracketed::typed_attribute_begin,
          typed_attribute_begin,
          ll::parser::bracketed::typed_opening )
MIN_REF ( ll::parser::bracketed::typed_attribute_equal,
          typed_attribute_equal,
          ll::parser::bracketed::typed_opening )
MIN_REF ( ll::parser::bracketed
                    ::typed_attribute_separator,
          typed_attribute_separator,
          ll::parser::bracketed::typed_opening )
MIN_REF ( ll::parser::bracketed
                    ::typed_attribute_negator,
          typed_attribute_negator,
          ll::parser::bracketed::typed_opening )
MIN_REF ( ll::parser::bracketed
                    ::typed_attribute_flags_opening,
          typed_attribute_flags_opening,
          ll::parser::bracketed::typed_opening )
MIN_REF ( ll::parser::bracketed
                    ::typed_attribute_flags_separator,
          typed_attribute_flags_separator,
          ll::parser::bracketed::typed_opening )
MIN_REF ( ll::parser::bracketed
                    ::typed_attribute_flags_closing,
          typed_attribute_flags_closing,
          ll::parser::bracketed::typed_opening )
MIN_REF ( ll::parser::bracketed
            ::typed_attribute_multivalue_opening,
          typed_attribute_multivalue_opening,
          ll::parser::bracketed::typed_opening )
MIN_REF ( ll::parser::bracketed
            ::typed_attribute_multivalue_separator,
          typed_attribute_multivalue_separator,
          ll::parser::bracketed::typed_opening )
MIN_REF ( ll::parser::bracketed
            ::typed_attribute_multivalue_closing,
          typed_attribute_multivalue_closing,
          ll::parser::bracketed::typed_opening )

MIN_REF ( min::gen, label,
          ll::parser::bracketed::typed_middle )
MIN_REF ( ll::parser::bracketed::typed_opening,
          typed_opening,
          ll::parser::bracketed::typed_middle )

MIN_REF ( min::gen, label,
          ll::parser::bracketed::typed_attribute_begin )
MIN_REF ( ll::parser::bracketed::typed_opening,
          typed_opening,
          ll::parser::bracketed::typed_attribute_begin )

MIN_REF ( min::gen, label,
          ll::parser::bracketed::typed_attribute_equal )
MIN_REF ( ll::parser::bracketed::typed_opening,
          typed_opening,
          ll::parser::bracketed::typed_attribute_equal )

MIN_REF ( min::gen, label,
          ll::parser::bracketed
	            ::typed_attribute_separator )
MIN_REF ( ll::parser::bracketed::typed_opening,
          typed_opening,
          ll::parser::bracketed
	            ::typed_attribute_separator )

MIN_REF ( min::gen, label,
          ll::parser::bracketed
	            ::typed_attribute_negator )
MIN_REF ( ll::parser::bracketed::typed_opening,
          typed_opening,
          ll::parser::bracketed
	            ::typed_attribute_negator )

MIN_REF ( min::gen, label,
          ll::parser::bracketed
	            ::typed_attribute_flags_opening )
MIN_REF ( ll::parser::bracketed::typed_opening,
          typed_opening,
          ll::parser::bracketed
	            ::typed_attribute_flags_opening )

MIN_REF ( min::gen, label,
          ll::parser::bracketed
	            ::typed_attribute_flags_separator )
MIN_REF ( ll::parser::bracketed::typed_opening,
          typed_opening,
          ll::parser::bracketed
	            ::typed_attribute_flags_separator )

MIN_REF ( min::gen, label,
          ll::parser::bracketed
	            ::typed_attribute_flags_closing )
MIN_REF ( ll::parser::bracketed::typed_opening,
          typed_opening,
          ll::parser::bracketed
	            ::typed_attribute_flags_closing )

MIN_REF ( min::gen, label,
          ll::parser::bracketed
	    ::typed_attribute_multivalue_opening )
MIN_REF ( ll::parser::bracketed::typed_opening,
          typed_opening,
          ll::parser::bracketed
	    ::typed_attribute_multivalue_opening )

MIN_REF ( min::gen, label,
          ll::parser::bracketed
	    ::typed_attribute_multivalue_separator )
MIN_REF ( ll::parser::bracketed::typed_opening,
          typed_opening,
          ll::parser::bracketed
	    ::typed_attribute_multivalue_separator )

MIN_REF ( min::gen, label,
          ll::parser::bracketed
	    ::typed_attribute_multivalue_closing )
MIN_REF ( ll::parser::bracketed::typed_opening,
          typed_opening,
          ll::parser::bracketed
	    ::typed_attribute_multivalue_closing )

ll::parser::bracketed::typed_opening
    push_typed_brackets
	( min::gen typed_opening,
	  min::gen typed_middle,
	  min::gen typed_closing,
	  ll::parser::table::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  const ll::parser::table::new_flags
	      & new_selectors,
	  min::gen typed_attribute_begin,
	  min::gen typed_attribute_equal,
	  min::gen typed_attribute_separator,
	  min::gen typed_attribute_negator,
	  min::gen typed_attribute_flags_opening,
	  min::gen typed_attribute_flags_separator,
	  min::gen typed_attribute_flags_closing,
	  min::gen typed_attribute_multivalue_opening,
	  min::gen typed_attribute_multivalue_separator,
	  min::gen typed_attribute_multivalue_closing,
	  ll::parser::table::key_table bracket_table );


// Bracketed Subexpression Pass
// --------- ------------- ----

typedef min::packed_vec_insptr<int32>
        indentation_offset_stack;

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

    int32 indentation_offset;
        // Amount the indentation of a line has to be
	// offset from the indentation of an indented
	// paragraph in order to avoid error messages.
	// Must be signed integer so indentations can
	// be set to - indentation_offset and indenta-
	// tion check computations are signed, but is
	// always >= 0.  Defaults to 2.

    const ll::parser::bracketed
                    ::indentation_offset_stack
            indentation_offset_stack;
	// Stack of indentation_offset values saved by
	// current parser blocks.

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
MIN_REF ( ll::parser::bracketed
                    ::indentation_offset_stack,
          indentation_offset_stack,
          ll::parser::bracketed::bracketed_pass )

// Return a new bracketed subexpression pass.
//
ll::parser::pass new_pass ( void );

// Parse Bracketed Subexpression Function
// ----- --------- ------------- --------

// Move `current' to the end of the current bracketed
// subexpression, calling parser->input if more tokens
// are needed.
//
// The parsed subexpression is NOT compacted and tokens
// in it are left untouched with the following excep-
// tions.  Line breaks and comments are deleted.  After
// doing this, consecutive quoted strings are merged if
// the second is in the same line as the first or in
// a continuation line following the line of the first.
// SUBSUBexpressions are identified and replaced by
// BRACKETED tokens.
//
// It is assumed that there are always more tokens
// available via parser->input until an end-of-file
// token is encountered, and the end-of-file is never
// part of the bracketed subexpression.  Therefore there
// is always a token immediately after the recognized
// subexpression.  This token is returned as the updated
// `current' argument value to mark the end of the
// recognized subexpression.
//
// The token list, beginning with the initial value of
// `current', is edited by this function.  The caller
// should save `current->previous' before calling this
// function, so it and `current' as returned by this
// function can be used to delimit the subexpression.
// Note that in the case of the top level call, there
// may be no `current->previous', and parser->first will
// be the first token of the returned subexpression.
// For non-top-level calls, there is always an indenta-
// tion mark or opening bracket before the initial value
// of `current'.
//
// The `bracket_stack' argument specifies brackets that
// need to be found in order to close bracketed sub-
// expressions.  When an entry in this stack is made,
// the entry is considered to be `open'.  When a closing
// bracket corresponding to one of these entries is
// recognized, that entry, and any other entries between
// that entry and the top of the stack are marked as
// `closed'.  The entries toward the top of the stack
// represent inner subexpressions, so discovery of a
// closing bracket for one subexpression can terminate
// not just that expression but also subexpressions
// within it whose closing brackets are missing.
//
// If the subexpression is terminated by a closing
// bracket, that closing bracket must appear in the
// bracket stack. In this case the bracket stack entry
// of the closing bracket is `closed', any other
// entries between this entry and the top of the stack
// are marked `closed', `current' is positioned just
// after the found closing bracket, and false is
// returned.
//
// If the subexpression is terminated a line separator,
// the line separator must be that of the `indentation_
// mark' argument, and it must be outside SUBSUBexpres-
// sions.  In this case `current' is positioned just
// after the line separator, and true is returned.
// If the `indentation_mark' argument is NULL_STUB,
// this feature is disabled.
//
// If the subexpression is terminated by a line break
// whose first following non-comment, non-line break
// token has indent <= the `indent' argument, then
// `current' is positioned at the line break, all 
// line break and comment tokens following this are
// deleted, false is returned, and NO bracket entries
// are closed.  If `indent' equals MINUS parser->
// indentation_offset, this feature is disabled.
//
// If the subexpression is terminated by an end-of-file,
// `current' is positioned at the end-of-file, false is
// returned, and NO bracket entries are closed.
//
// This function calls itself recursively if it finds
// an opening bracket or an indentation mark.  The
// `selectors' argument determines which opening bracket
// and indentation mark definitions are active.  When
// this function calls itself recursively, upon return
// it wraps all the tokens of the sub-subexpression
// found into a single BRACKETED token (even if this is
// an empty list).  It also replaces quoted strings in
// the wrapped sub-subexpression by expressions whose 
// sole elements are the translation strings of the
// quoted string lexemes and whose .types are ".
//
// The `selectors' argument has no affect on closing
// bracket or line separator recognition.
// 
// More specifically, bracketed sub-subexpressions are
// converted to a list.  For untyped brackets, the
// .initiator and .terminator of this list are set to
// the labels of the opening and closing brackets of the
// subsubexpression, and the .type is set to a label
// that is a pair consisting first of the .initiator
// label and second the .terminator label.  For typed
// brackets just the .type is set.
//
// Sub-subexpressions introduced by an indentation mark
// are converted to a list of lists.  The outer list
// is a list of lines and has the indentation mark label
// as its .type.  The inner lists are paragraph line
// subexpressions and have "\n" as their .type if they
// do not end with an line separator, and have the line
// separator label as their .type otherwise (and the
// line separator at the end of the paragraph line is
// omitted from the inner list).  Inner lists that would
// be empty with just a "\n" type are deleted.
//
// As line breaks are not deleted until after brackets,
// indentation marks, etc are recognized, multi-lexeme
// brackets etc. are not recognized if they straddle
// a line break.
//
// This function is called at the top level with zero
// indent, the `top_level_indentation_mark' which has
// indentation separator `;', and bracket_stack == NULL.
//
// If an opening bracket is found with its `FULL_LINES'
// feature on, the resulting recursive call to this
// function has a disabled `indent', NULL_STUB indenta-
// tion mark, and has a bracket stack consisting solely
// of one entry for the opening bracket.
//
struct bracket_stack
{
    ll::parser::bracketed
              ::opening_bracket opening_bracket;
        // If not NULL_STUB, this identifies the opening
	// bracket whose recognition made this entry.

    ll::parser::token closing_first, closing_next;
        // If these are NULL_STUB, this entry is open.
	// Otherwise if first != next, they are the
	// first token of the closing bracket that
	// closed this entry, and the next token AFTER
	// this bracket, but if first == next, the
	// closing bracket that terminated this entry
	// was missing and should be inserted just
	// before next.

    ll::parser::bracketed::bracket_stack * previous;
        // Stack is NULL terminated.

    bracket_stack
	    ( ll::parser::bracketed
	                ::bracket_stack * previous )
        : opening_bracket ( min::NULL_STUB ),
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

bool parse_bracketed_subexpression
	( ll::parser::parser parser,
	  ll::parser::table::flags selectors,
	  ll::parser::token & current,
	  min::int32 indent,
	  ll::parser::bracketed::indentation_mark
	      indentation_mark,
	  bracket_stack * bracket_stack_p  = NULL );

} } }

# endif // LL_PARSER_BRACKETED_H
