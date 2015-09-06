// Layered Languages Bracketed Subexpression Parser
//
// File:	ll_parser_bracketed.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun Sep  6 13:34:34 EDT 2015
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
};

MIN_REF ( min::gen, label,
          ll::parser::bracketed::indentation_mark )
MIN_REF ( ll::parser::bracketed::line_sep,
          line_sep,
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

// Selectors for key_table below.
//
enum {
    MIDDLE_SELECTOR	= 1ull << 0,
        // For TYPED_MIDDLE
    ATTR_SELECTOR	= 1ull << 1,
        // For TYPED_ATTR_BEGIN/EQUAL/SEP
    NEGATOR_SELECTOR	= 1ull << 2
        // For TYPED_ATTR_NEGATOR
};

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
};

struct typed_middle_struct : 
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is TYPED_MIDDLE.

    const ll::parser::bracketed::typed_opening
          typed_opening;
        // The opening bracket for the typed middle.
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


MIN_REF ( min::gen, label,
          ll::parser::bracketed::typed_opening )
MIN_REF ( ll::parser::reformatter,
          reformatter,
          ll::parser::bracketed::typed_opening )
MIN_REF ( ll::parser::reformatter_arguments,
          reformatter_arguments,
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

MIN_REF ( min::gen, label,
          ll::parser::bracketed::typed_middle )
MIN_REF ( ll::parser::bracketed::typed_opening,
          typed_opening,
          ll::parser::bracketed::typed_middle )

MIN_REF ( min::gen, label,
          ll::parser::bracketed::typed_attr_begin )
MIN_REF ( ll::parser::bracketed::typed_opening,
          typed_opening,
          ll::parser::bracketed::typed_attr_begin )

MIN_REF ( min::gen, label,
          ll::parser::bracketed::typed_attr_equal )
MIN_REF ( ll::parser::bracketed::typed_opening,
          typed_opening,
          ll::parser::bracketed::typed_attr_equal )

MIN_REF ( min::gen, label,
          ll::parser::bracketed::typed_attr_sep )
MIN_REF ( ll::parser::bracketed::typed_opening,
          typed_opening,
          ll::parser::bracketed::typed_attr_sep )

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
	  min::gen typed_closing,
	  ll::parser::table::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  const ll::parser::table::new_flags
	      & element_selectors,
	  ll::parser::table::flags attr_selectors,
	  ll::parser::reformatter reformatter,
	  ll::parser::reformatter_arguments
	      reformatter_arguments,
	  uns32 options,
	  min::gen typed_attr_begin,
	  min::gen typed_attr_equal,
	  min::gen typed_attr_sep,
	  min::gen typed_attr_negator,
	  min::gen typed_attr_flags_initiator,
	  const min::flag_parser *
		   typed_attr_flag_parser,
	  min::gen typed_attr_multivalue_initiator,
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
// are needed.  Return the end position of any line
// separator found (the line separator is deleted), or
// return min::MISSING_POSITION if no line separator
// found.
//
// The parsed subexpression is NOT compacted and tokens
// in it are left untouched with the following excep-
// tions.  Line breaks and comments are deleted.  After
// doing this, consecutive quoted strings are merged if
// the second is in the same line as the first or in
// a continuation line following the line of the first.
// Any subexpression terminating line separator is also
// deleted.  Sub-subexpressions are identified and each
// is replaced by a single BRACKETED token.
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
// tion mark, opening bracket, or prefix separator
// before the initial value of `current'.
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
// An end of file, a line whose indent is not greater
// than the `indent' argument, or a recognized line
// separator matching the `line_sep' argument, causes
// this function to return WITHOUT closing ANY bracket
// stack entries.  A return without any closed bracket
// stack entries is caused by an end file if `current'
// is an end of file token.  It is caused by a line
// with indent not greater than the `indent' argument if
// `current' is a line break.  And it is caused by a
// line separator if the returned value is the position
// of the line separator and not min::MISSING_POSITION,
// in which case `current; is the token after the
// deleted line separator.
//
// Closing brackets are only recognized if their
// corresponding opening brackets are in the bracket
// stack.  Prefix separators are recognized as closing
// subexpressions only if a prefix separator with the
// same .type appears in the bracket stack above any
// bracket entry in that stack.
//
// If the subexpression is terminated a line separator,
// the line separator must be that of the `line_sep'
// argument.  In this case `current' is positioned just
// after the line separator, but the line separator
// is removed, and the end position of the line sepa-
// rator is returned.  NO bracket entries are closed.
// If the `line_sep' argument is NULL_STUB, this feature
// is disabled.  By default, the line_sep argument is
// not used for parsing bracketed sub-subexpressions,
// but is used for parsing prefix separator sub-sub-
// expressions.  There is an opening bracket option to
// override this default.
//
// If the subexpression is terminated by a line break
// whose first following non-comment, non-line break
// token T has indent <= the `indent' argument, then
// `current' is positioned at the line break, all 
// line break and comment tokens following this are
// deleted, current->next is token T, min::MISSING_
// POSITION is returned, and NO bracket entries are
// closed.  If `indent' equals MINUS parser->indenta-
// tion_offset, this feature is disabled.
//
// If the subexpression is terminated by an end-of-file,
// `current' is positioned at the end-of-file, min::
// MISSING_POSITION is returned, and NO bracket entries
// are closed.
//
// This function calls itself recursively if it finds
// an opening bracket, or an indentation mark, or a
// prefix separator at the beginning of the subexpres-
// sion.  The `selectors' argument determines which
// opening bracket and indentation mark definitions are
// active.  The `selectors' argument has no affect on
// closing bracket or line separator recognition.
//
// If the subexpression is a typed bracketed subexpres-
// sion, the `typed_opening' argument is not NULL_STUB,
// and is the typed_opening that prefixes the subexpres-
// sion.  In this case typed_opening->attr_selectors is
// used in place of the selectors argument before the
// first typed_middle corresponding to the typed_open-
// ing, and after the second such typed_middle.  Between
// the typed_middles the original selectors argument is
// used.  Selectors have no affect on typed_middle
// recognition.
//
// When this function calls itself recursively to parse
// a bracketed sub-subexpression, upon return it calls
// any reformatter associated with the opening_bracket
// that started the sub-subexpression, and either the
// reformatter or a subsequence call to `compact' wraps
// all the tokens of the sub-subexpression into a single
// BRACKETED or PREFIX token (even if this is an empty
// list).  It also converts tokens with MISSING value
// (quoted strings and numerics) in the result as per
// `convert_token'.  If the opening_bracket is not a
// typed_opening, the resulting MIN object is given
// the opening and closing brackets as its .initiator
// and .terminator, and is NOT given a .type attribute.
// If the opening_bracket is a typed_opening, the
// MIN object is given a .type attribute, but NO
// .initiator or .terminator.
//
// If an opening_bracket has its `FULL_LINES' option on,
// the recursive call to this function has a disabled
// `indent', NULL_STUB line_sep argument, and a bracket
// stack consisting solely of one entry for the opening
// bracket.
// 
// Sub-subexpressions introduced by an indentation mark
// are converted to a list of lists.  The outer list
// is a list of lines and has the indentation mark label
// as its .type.  The inner lists are paragraph line
// subexpressions and have "\n" as their .type, and if
// they end with a line separator, have that as their
// .terminator.  Inner lists that would be empty, with
// just a "\n" .type and no .terminator, are deleted.
//
// As line breaks are not deleted until after brackets,
// indentation marks, etc are recognized, multi-lexeme
// brackets etc. are not recognized if they straddle
// a line break.
//
// This function is called at the top level with zero
// indent, the `top_level_indentation_mark->line_sep'
// which is `;', and bracket_stack == NULL.
//
struct bracket_stack
{
    // Either opening_bracket != NULL_STUB or prefix_
    // type != MISSING.

    ll::parser::bracketed
              ::opening_bracket opening_bracket;
        // If not NULL_STUB, this identifies the opening
	// bracket whose recognition made this entry.

    min::gen prefix_type;
        // If not MISSING, this identifies the .type of
	// the prefix separator whose recognition made
	// this entry.

    ll::parser::token closing_first, closing_next;
        // If these are NULL_STUB, this entry is open.
	// The bracketed subexpression may be closed
	// anyway by a non-indented line or end of file.
	//
	// Otherwise if opening_bracket != NULL_STUB,
	// then if first != next, these entries are the
	// first token of the closing bracket that
	// closed this entry, and the next token AFTER
	// this bracket, but if first == next, the
	// closing bracket that terminated this entry
	// was missing and should be inserted just
	// before next.
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
	  min::int32 indent,
	  ll::parser::bracketed::line_sep
	      line_sep,
	  ll::parser::bracketed::typed_opening
	      typed_opening,
	  bracket_stack * bracket_stack_p  = NULL );

} } }

# endif // LL_PARSER_BRACKETED_H
