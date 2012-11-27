// Layers Language Bracketed Subexpression Parser
//
// File:	ll_parser_bracketed.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Nov 27 03:42:34 EST 2012
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Brackets
//	Named Brackets
//	Indentation Marks
//	Bracketed Subexpression Pass
//	Parse Bracketed Subexpression Function

// Usage and Setup
// ----- --- -----

# ifndef LL_PARSER_BRACKETED_H
# define LL_PARSER_BRACKETED_H

# include <ll_parser.h>

namespace ll { namespace parser { namespace bracketed {

// Brackets
// --------

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

    bool full_line;

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

MIN_REF ( min::gen, label,
          ll::parser::bracketed::closing_bracket )
MIN_REF ( ll::parser::bracketed::opening_bracket,
          opening_bracket,
          ll::parser::bracketed::closing_bracket )

ll::parser::bracketed::opening_bracket
    push_brackets
	( min::gen opening_label,
	  min::gen closing_label,
	  ll::parser::table::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  const ll::parser::table::new_flags
	      & new_selectors,
	  bool full_line,
	  ll::parser::table::table bracket_table );


// Named Brackets
// ----- --------

// `named_' abbreviates `named_bracket_'.

// Named bracket definition.

struct named_opening_struct;
typedef min::packed_struct_updptr
	    <named_opening_struct>
        named_opening;
extern const uns32 & NAMED_OPENING;
    // Subtype of min::packed_struct
    //		       <named_opening_struct>.

struct named_separator_struct;
typedef min::packed_struct_updptr
	    <named_separator_struct>
        named_separator;
extern const uns32 & NAMED_SEPARATOR;
    // Subtype of min::packed_struct
    //		       <named_separator_struct>.

struct named_middle_struct;
typedef min::packed_struct_updptr
	    <named_middle_struct>
        named_middle;
extern const uns32 & NAMED_MIDDLE;
    // Subtype of min::packed_struct
    //		       <named_middle_struct>.

struct named_closing_struct;
typedef min::packed_struct_updptr
	    <named_closing_struct>
        named_closing;
extern const uns32 & NAMED_CLOSING;
    // Subtype of min::packed_struct
    //		       <named_closing_struct>.

struct named_middle_closing_struct;
typedef min::packed_struct_updptr
	    <named_middle_closing_struct>
        named_middle_closing;
extern const uns32 & NAMED_MIDDLE_CLOSING;
    // Subtype of
    //     min::packed_struct
    //	       <named_middle_closing_struct>.

struct named_opening_struct : 
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is NAMED_OPENING.

    // Syntax is
    //
    //      named-opening ...
    //	    { named-separator ... }*
    //	    named-middle
    //	    ***
    //      named-middle ... named-closing
    //
    // or
    //
    //      named-opening ...
    //	    { named-separator ... }*
    //	    named-middle
    //	    ***
    //      named-middle-closing
    //
    // or
    //
    //      named-opening ...
    //	    { named-separator ... }*
    //	    named-closing
    //
    // Where ... is any sequence of words, numbers,
    // quoted strings, or unnamed bracketed subexpres-
    // sions (but NOT separators or marks outside
    // bracketed subexpressions), and *** is any
    // sequence of lexemes and bracketed subexpres-
    // sions (possibly including named ones).
    //
    // E.g, if the named-opening is <, the named-
    // separator is #, the named-middle is |, and
    // the named-closing is >, then
    //
    //	    < ... {# ...}* | *** | ... >
    // or
    //	    < ... {# ...}* | *** |>
    // or
    //	    < ... {# ...}* >
    //
    //
    // The named-middle may be MISSING.
    //
    // The named-separator may be MISSING.
    //
    // The named-middle-closing should be specified as
    // MISSING if the named-middle and named-closing
    // cannot glue together into a single lexeme.
    //
    // MISSING stub pointers are NULL_STUB and MISSING
    // min::gen labels are min::MISSING().

    const ll::parser::bracketed::named_separator
          named_separator;
    const ll::parser::bracketed::named_middle
          named_middle;
    const ll::parser::bracketed::named_closing
          named_closing;
    const ll::parser::bracketed
                    ::named_middle_closing
          named_middle_closing;

};

struct named_separator_struct : 
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is NAMED_SEPARATOR.

    const ll::parser::bracketed::named_opening
          named_opening;
        // Corresponding named opening bracket.
};

struct named_middle_struct : 
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is NAMED_MIDDLE.

    const ll::parser::bracketed::named_opening
          named_opening;
        // Corresponding named opening bracket.
};

struct named_closing_struct : 
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is NAMED_CLOSING.

    const ll::parser::bracketed::named_opening
          named_opening;
        // Corresponding named opening bracket.
};

struct named_middle_closing_struct :
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is NAMED_MIDDLE_CLOSING_
    // BRACKET.

    const ll::parser::bracketed::named_opening
          named_opening;
        // Corresponding named opening bracket.
};

MIN_REF ( min::gen, label,
          ll::parser::bracketed::named_opening )
MIN_REF ( ll::parser::bracketed::named_separator,
          named_separator,
          ll::parser::bracketed::named_opening )
MIN_REF ( ll::parser::bracketed::named_middle,
          named_middle,
          ll::parser::bracketed::named_opening )
MIN_REF ( ll::parser::bracketed::named_closing,
          named_closing,
          ll::parser::bracketed::named_opening )
MIN_REF ( ll::parser::bracketed
                    ::named_middle_closing,
          named_middle_closing,
          ll::parser::bracketed::named_opening )

MIN_REF ( min::gen, label,
          ll::parser::bracketed::named_separator )
MIN_REF ( ll::parser::bracketed::named_opening,
          named_opening,
          ll::parser::bracketed::named_separator )

MIN_REF ( min::gen, label,
          ll::parser::bracketed::named_middle )
MIN_REF ( ll::parser::bracketed::named_opening,
          named_opening,
          ll::parser::bracketed::named_middle )

MIN_REF ( min::gen, label,
          ll::parser::bracketed::named_closing )
MIN_REF ( ll::parser::bracketed::named_opening,
          named_opening,
          ll::parser::bracketed::named_closing )

MIN_REF ( min::gen, label,
          ll::parser::bracketed
	    ::named_middle_closing )
MIN_REF ( ll::parser::bracketed::named_opening,
          named_opening,
          ll::parser::bracketed
	    ::named_middle_closing )

ll::parser::bracketed::named_opening
    push_named_brackets
	( min::gen named_opening_label,
	  min::gen named_separator_label,
	      // May be min::MISSING().
	  min::gen named_middle_label,
	      // May be min::MISSING().
	  min::gen named_closing_label,
	  min::gen named_middle_closing_label,
	      // May be min::MISSING().
	  ll::parser::table::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  ll::parser::table::table bracket_table );

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
extern const uns32 & LINE_SEPARATOR;
    // Subtype of min::packed_struct
    //		       <line_separator_struct>.

// A gluing indentation mark has an associated indenta-
// tion split that contains the mark label, points at
// the indentation mark, and is entered in an indenta-
// tion split table.
//
struct indentation_split_struct;
typedef min::packed_vec_insptr
	    <uns8,indentation_split_struct>
        indentation_split;

// A split table has exactly 256 elements, each the
// head of a length-sorted list of indentation_splits.
// Each split is entered in the list indexed by the
// last byte of its indentation mark's label.
//
typedef min::packed_vec_insptr
	    <ll::parser::bracketed::indentation_split>
	split_table;

// Create an indentation split table.  Note that the
// length is always 256 (table index is uns8).
//
ll::parser::bracketed::split_table create_split_table
	( void );

struct indentation_mark_struct : 
	public ll::parser::table::root_struct
{
    ll::parser::table::new_flags new_selectors;

    const ll::parser::bracketed::line_separator
	    line_separator;

    const ll::parser::bracketed::indentation_split
	    indentation_split;
	// If gluing, the split for this indentation.
	// If non-gluing, NULL_STUB.
};

MIN_REF ( min::gen, label,
          ll::parser::bracketed::indentation_mark )
MIN_REF ( ll::parser::bracketed::line_separator,
          line_separator,
          ll::parser::bracketed::indentation_mark )
MIN_REF ( ll::parser::bracketed::indentation_split,
          indentation_split,
          ll::parser::bracketed::indentation_mark )

struct line_separator_struct : 
	public ll::parser::table::root_struct
{
    // Packed_struct subtype is LINE_SEPARATOR.

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

struct indentation_split_struct
{
    const min::uns32 control;
    const min::uns32 length;
    const min::uns32 max_length;

    const ll::parser::bracketed::indentation_split next;
        // Next in the length-sorted list of splits
	// whose head is an indentation_split table
	// element.

    const ll::parser::bracketed::indentation_mark
	    indentation_mark;
	// Indentation_mark associated with split.

    // The vector of min::uns8 elements is the indenta-
    // tion mark label in UTF-8.  Lexemes at the end of
    // a line can be checked to see if they end with
    // this.  The last byte is used as a hash in the
    // indentation split table, so, for example, most
    // line ending lexemes (e.g. those ending in
    // letters) will not need to be checked (while
    // those ending in `:' probably will be checked).

};

MIN_REF ( ll::parser::bracketed::indentation_split,
          next,
          ll::parser::bracketed::indentation_split )
MIN_REF ( ll::parser::bracketed::indentation_mark,
          indentation_mark,
          ll::parser::bracketed::indentation_split )

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
	  ll::parser::table::table bracket_table,
	  ll::parser::bracketed
	            ::split_table split_table =
	      NULL_STUB );
	      // NULL_STUB iff not gluing

// Remove from the split table all entries that point
// at indentation_marks which have block_level >
// block_level argument.  Return the number entries
// removed.
//
void end_block
	( ll::parser::bracketed
	            ::split_table split_table,
          uns32 block_level,
	  uns64 & collected_entries );

// Bracketed Subexpression Pass
// --------- ------------- ----

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

    const ll::parser::table::table bracket_table;
        // Hash table for brackets and indentation
	// marks.

    const ll::parser::bracketed
                    ::split_table split_table;
        // Table for indentation splits associated with
	// indentation marks that can be split.

    int32 indent_offset;
        // Amount the indentation of a line has to be
	// offset from the indentation of an indented
	// paragraph in order to avoid error messages.
	// Must be signed integer so indentations can
	// be set to - indent_offset and indentation
	// check computations are signed, but is
	// always >= 0.  Defaults to 2.

    ll::parser::table::flags trace_subexpressions;
        // Trace flag named `bracketed subexpressions'
	// that traces bracketed subexpressions.

};

MIN_REF ( ll::parser::pass, next,
          ll::parser::bracketed::bracketed_pass )
MIN_REF ( ll::parser::table::table, bracket_table,
          ll::parser::bracketed::bracketed_pass )
MIN_REF ( ll::parser::bracketed::split_table,
          split_table,
          ll::parser::bracketed::bracketed_pass )

// Create an bracketed subexpression pass and place it
// as the first pass on the parser->pass_stack.
// Return the new bracketed subexpression pass.
//
ll::parser::bracketed::bracketed_pass place
	( ll::parser::parser parser );

// Parse Bracketed Subexpression Function
// ----- --------- ------------- --------

// Move `current' to the end of the current bracketed
// subexpression, calling parser->input if more tokens
// are needed.
//
// The parsed subexpression is NOT compacted and tokens
// in it are left untouched with the following excep-
// tions.  Line breaks and comments are deleted.  After
// doing this, consecutive quoted strings are merged.
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
// indent_offset, this feature is disabled.
//
// If the subexpression is terminated by an end-of-file,
// `current' is positioned at the end-of-file, false is
// returned, and NO bracket entries are closed.
//
// This function calls itself recursively if it finds
// an opening unnamed or named bracket or an indentation
// mark.  The `selectors' argument determines which
// opening bracket and indentation mark definitions are
// active.  When this function calls itself recursively,
// upon return it wraps all the tokens of the sub-subex-
// pression found into a single BRACKETED token (even if
// this is an empty list).  It also replaces quoted
// strings in the wrapped sub-subexpression by expres-
// sions whose sole elements are the translation strings
// of the quoted string lexemes and whose .initiators
// are ".
//
// The `selectors' argument has no affect on closing
// bracket, named middle, named key separator, named
// closing, or line separator recognition.
// 
// More specifically, bracketed sub-subexpressions are
// converted to a list.  For unnamed brackets, the
// .initiator and .terminator of this list are set to
// the labels of the opening and closing brackets of the
// subsubexpression.  For named brackets the .initiator,
// .middle, and .terminator are set to the labels of the
// named opening, named middle, and named closing, and
// the .name is set to the bracket name.  If there are
// bracket arguments, they become the elements of a list
// that is the .arguments attribute.  Note that the
// element values of an .arguments attribute value have
// the same format as a sub-subexpression.  If there are
// bracket keys, they become the elements of a .keys
// attribute list, and the named key separator becomes
// both the .initiator and .separator of that list.
//
// Sub-subexpressions introduced by an indentation mark
// are converted to a list of lists.  The outer list
// is a list of lines and has the indentation mark label
// as its .initiator.  The inner lists are paragraph
// line subexpressions and have "\n" as their .termina-
// tor if they do not end with an line separator, and
// have the line separator label as their .terminator
// otherwise (and the line separator at the end of the
// paragraph line is omitted from the inner list).
// Inner lists that would be empty with just a "\n"
// terminator are deleted.
//
// Gluing indentation marks are split from line-ending
// tokens.  When a gluing indentation mark is split,
// the scan backs up to the first word, number, mark,
// or separator that is AFTER any token that is not
// a quoted string or part of a recognized symbol,
// and the tokens backed over are rescanned.  In un-
// usual situations, this can result in the indentation
// mark not being recognized during the rescan, because
// a bracket is recognized first during rescan and this
// changes the selectors so as not to recognize the
// mark.  However, in any case the mark remains split.
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
// If an unnamed opening bracket is found with its
// `full_line' feature on, the resulting recursive call
// to this function has a disabled `indent', NULL_STUB
// indentation mark, and has a bracket stack consisting
// solely of one entry for the unnamed opening bracket.
//
struct bracket_stack
{
    // Exactly one of `opening_bracket' and `named_
    // opening' is != NULL_STUB:
    //
    ll::parser::bracketed
              ::opening_bracket opening_bracket;
        // If not NULL_STUB, this identifies the opening
	// bracket whose recognition made this entry.
    ll::parser::bracketed::named_opening named_opening;
        // If not NULL_STUB, this identifies the named
	// opening whose recognition made this entry.

    ll::parser::token opening_first, opening_next;
        // For named_openings only, the first token
	// AFTER the named opening and the first token
	// OF the named middle; thus bounding the named
	// bracket name, arguments, and keys.

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
          named_opening ( min::NULL_STUB ),
          opening_first ( min::NULL_STUB ),
          opening_next ( min::NULL_STUB ),
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
