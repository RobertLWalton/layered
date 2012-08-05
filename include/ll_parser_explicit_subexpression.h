// Layers Language Explicit Subexpression Parser
//
// File:	ll_parser.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun Aug  5 05:53:05 EDT 2012
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Explicit Subexpression Parser Functions

// Usage and Setup
// ----- --- -----

# ifndef LL_PARSER_EXPLICIT_SUBEXPRESSION_H
# define LL_PARSER_EXPLICIT_SUBEXPRESSION_H

# include <ll_parser.h>

namespace ll { namespace parser {
//
// Parse an explicit subexpression beginning with the
// `current' token and calling parser->input if more
// tokens are needed.
//
// The parsed subexpression is NOT compacted; its end is
// identified and its SUBSUBexpressions are compacted.
// Line breaks are deleted, and after line break
// deletion, consecutive quoted strings are merged.
//
// It is assumed that there are always more tokens
// available via parser->input until an end-of-file
// token is encountered, and the end-of-file is never
// part of the explicit subexpression.  Therefore there
// is always a token immediately after the recognized
// subexpression.  This token is returned as the updated
// `current' argument value to mark the end of the
// recognized subexpression.
//
// The subexpression is either a paragraph line, includ-
// ing indented continuations, or is a subexpression
// begun by an unnamed or named opening bracket.  In the
// first case `current' is initially the first token of
// the paragraph line, and in the second case `current'
// is initially the first token after the unnamed or
// named opening bracket.
//
// In either case the selectors are those in the selec-
// tors argument to this function, which are computed by
// this function's caller using the indentation mark or
// opening bracket that caused this function to be
// called.
//
// In either case the subexpression terminates just
// before any line break whose next non-line-break, non-
// comment token has an indent less than or equal to the
// `indent' argument.  Note that this feature can be
// disabled by setting the `indent' argument to MINUS
// parser->indent_offset.
//
// If the subexpression is a paragraph line, the initial
// `current' token when this function is called should
// be the first token of the paragraph line, should not
// be a line break, and should not have an indent less
// than the `indent' argument.
//
// The subexpression also terminates just before any
// line break whose next non-line-break, non-comment
// token is an end-of-file, and just before an end-of-
// file that does not follow a line-break (the top
// level caller of this function announces an error in
// this last case).
//
// If the indentation_mark argument is not NULL_STUB,
// the subexpression will terminate just after any
// indentation_separator that matches the indentation_
// mark.  Note that this indentation_separator must be
// outside any subsubexpression.  In this case this
// function returns `true', whereas in all other cases
// it returns `false'.
//
// Comment tokens that are not a full line (i.e., that
// follow a non-comment token on the same line) are
// deleted and ignored.
// 
// The `bracket_stack' specifies brackets that need to
// be closed.  When an entry in this stack is made, the
// entry is considered to be `open'.  When a closing
// bracket corresponding to one of these entries is
// recognized, that entry, and any other entries between
// that entry and the top of the stack are marked as
// `closed'.
//
// This function does not actually know whether or not
// the subexpression is a paragraph line or is bracket-
// ed.  The rules for terminating the subexpression are
// the same in either case.
//
// Normally a paragraph line subexpression will be ter-
// minated by a non-indented token after a line-break
// or by an end-of-file, and no bracket stack entries
// will be closed.  Normally a bracketed subexpression
// will be terminated by its closing bracket which will
// match the top entry on the bracket stack, and only
// that top entry will be closed.
//
// It is possible for a paragraph line to be normally
// terminated by a closing bracket, in which case the
// paragraph line consists of all tokens in the sub-
// expression up to the closing bracket, and the clos-
// ing bracket also terminates an outer subexpression.
//
// Subexpressions that would be empty subparagraph lines
// are ignored.  Thus a closing bracket indented by
// `indent' will not produce an empty subparagraph line.
// However, lines terminated by indentation_separators
// are NOT considered to be empty in this sense (they
// are given a .terminator).
// 
// If the closing unnamed or named bracket of a sub-
// expression is omitted, then when the subexpression is
// terminated either no bracket stack entry or more than
// one entry will be closed.  The caller can recognize
// this situation, announce the bracket omission, and
// insert the omitted bracket.
//
// To be recognized, closing brackets, named middles,
// indentation separators, etc. must be active as per
// the selectors.  So it is possible for an unnamed or
// named closing bracket or an indentation separator to
// be missed because the selectors have been changed and
// something is not active, though this will only happen
// if there is some other missing bracket or there has
// been an error in the way selectors have been defined
// for the brackets.
//
// If a closing unnamed or named bracket not correspond-
// ing to any bracket stack entry is recognized, it is
// announced as an error and ignored.
//
// Indentation separators are recognized only if they
// match an intentation_mark argument to this function.
// Otherwise they are not recognized, and their tokens
// may be part of some other recognized bracket symbol
// table entry.
//
// The end of the subexpression is specified by the
// `current' token upon return by this function, and
// also by the number of bracket_stack entries that have
// been marked as closed.  If NO bracket_stack entries
// have been marked closed, then `current' is the first
// token AFTER the subexpression, and is either a line
// break, an end of file, or the token after an indenta-
// tion separator.  If it is a line break, then any sub-
// sequent line breaks and comments will have been de-
// leted, current will be the line break, and current->
// next will be the next non-line-break, non-comment
// token read after the line break and any subsequent
// deleted line breaks and comments, and may be the non-
// indented token that terminated the line or it may be
// an end of file.
//
// If a bracket_stack entry has been marked closed, then
// `current' will be the first token AFTER the closing
// unnamed or named bracket whose recognition terminated
// the subexpression.
//
// Note that if any bracket_stack entry is marked as
// closed, then the top bracket_stack entry will be
// marked as closed.  If more than one bracket_stack
// entry is marked as closed, then the closing bracket
// of the top entry was omitted.
//
// This function calls itself recursively if it finds
// an opening unnamed or named bracket or an indentation
// mark.  The selectors determine which bracket and
// indentation mark definitions are active.  When this
// function calls itself recursively, upon return it
// wraps all the tokens of the sub-subexpression found
// into a single expression token (even if this is an
// empty list).  It also replaces nonnatural numbers and
// quoted strings in the sub-subexpression by expres-
// sions whose sole elements are the translation strings
// of the token lexemes and whose .initiators are # for
// number and " for quoted string.
//
// More specifically, bracketed SUBSUBexpressions are
// converted to a list.  For unnamed brackets, the
// .initiator and .terminator of this list are set to
// the labels of the opening and closing brackets of the
// subsubexpression.  For named brackets the .initiator,
// .middle, and .terminator are set to the labels of the
// named opening, named middle, and named closing, and
// the .name, .arguments, and .keys attributes are set
// as computed by the ::named_attributes function (at-
// tributes whose values would be MISSING are not set).
//
// SUBSUBexpressions introduced by an indentation mark
// are converted to a list of lists.  The outer list
// is a list of lines and has the indentation mark label
// as its .initiator.  The inner lists are paragraph
// line subexpressions and have "\n" as their .termina-
// tor if they do not end with an indentation separator,
// and have the indentation separator label as their
// .terminator otherwise (and the indentation separator
// at the end of the paragarph line is omitted from the
// inner list).
//
// When this function detects a subsubexpression with a
// missing unnamed or named closing bracket, this func-
// tion produces an error message, and proceeds as if
// the closing bracket were inserted just before the
// closing bracket or line break that terminates the
// subsubexpression.
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
// Line_break tokens are deleted.  Gluing indentation
// marks are split from line-ending tokens.  When a
// gluing indentation mark is split, the scan backs up
// to the first mark or separator that is AFTER any non-
// mark-non-separator, any subsubexpression, and any
// sequence of marks and separators found in the
// bracket table.  If this is nothing else, it is the
// first part of the mark that was split.
//
// A consequence of all this is that a gluing indenta-
// tion may be appended to the last token in a multi-
// token closing bracket that ends in a mark, but
// because the closing bracket can change the current
// selectors, the selectors in effect when the gluing
// indentation mark was split might NOT in be the same
// as those in effect when the split formerly-glued
// indentation mark is rescanned, and the rescanned
// indentation mark might not be recognized.  This situ-
// ation is unusual, and can be prevented by not permit-
// ting closing brackets that can be glued to indenta-
// tion marks to change the selectors used to recognize
// the indentation marks.
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
// indentation mark, has a bracket stack consisting
// solely of one entry for the unnamed opening bracket.
//
struct bracket_stack
{
    // Exactly one of `opening_bracket' and `named_
    // opening' is != NULL_STUB:
    //
    ll::parser::table::opening_bracket opening_bracket;
        // If not NULL_STUB, this identifies the opening
	// bracket whose recognition made this entry.
    ll::parser::table::named_opening named_opening;
        // If not NULL_STUB, this identifies the named
	// opening whose recognition made this entry.

    ll::parser::token opening_first, opening_next;
        // For named_openings only, the first token
	// AFTER the named opening and the first token
	// OF the named middle ; thus bounding the named
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

    ll::parser::bracket_stack * previous;
        // Stack is NULL terminated.

    bracket_stack
	    ( ll::parser::bracket_stack * previous )
        : opening_bracket ( min::NULL_STUB ),
          named_opening ( min::NULL_STUB ),
          opening_first ( min::NULL_STUB ),
          opening_next ( min::NULL_STUB ),
          closing_first ( min::NULL_STUB ),
          closing_next ( min::NULL_STUB ),
	  previous ( previous ) {}
};

inline bool is_closed ( ll::parser::bracket_stack * p )
{
    return    p != NULL
           && p->closing_first != min::NULL_STUB;
}

bool parse_explicit_subexpression
	( ll::parser::parser parser,
	  ll::parser::table::selectors selectors,
	  ll::parser::token & current,
	  min::int32 indent,
	  ll::parser::table::indentation_mark
	      indentation_mark,
	  bracket_stack * bracket_stack_p  = NULL );

} }

# endif // LL_PARSER_EXPLICIT_SUBEXPRESSION_H
