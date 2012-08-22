// Layers Language Bracketed Subexpression Parser
//
// File:	ll_parser_bracketed.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Aug 22 15:29:30 EDT 2012
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

# ifndef LL_PARSER_BRACKETED_H
# define LL_PARSER_BRACKETED_H

# include <ll_parser.h>

namespace ll { namespace parser {
//
// Move `current' to the end of the current bracketed
// subexpression, calling parser->input if more tokens
// are needed.
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
// after the closing bracket found, and false is
// returned.
//
// If the subexpression is terminated a line separator,
// the line separator must be that of the `indentation_
// mark argument, and it must be outside SUBSUBexpres-
// sions.  In this case `current' is positioned just
// after the line separator, and true is returned.
// If the `indentation_mark' argument is NULL_STUB,
// this feature is disabled.
//
// If the subexpression is terminated by a line break
// whose first following non-comment, non-line break
// token has indent <= the `indent' argument, then
// `current' is positioned at the line break, false is
// returned, and NO bracket entries are closed.  If
// `indent' equals MINUS parser->indent_offset, this
// feature is disabled.
//
// If the subexpression is terminated by an end-of-file,
// `current' is positioned at the end-of-file, false is
// returned, and NO bracket entries are closed.
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
// bracket or line separator recognition.
//
// Subexpressions that would be empty subparagraph lines
// are ignored.  Thus a closing bracket indented by
// `indent' will not produce an empty subparagraph line.
// However, lines terminated by indentation_separators
// are NOT considered to be empty in this sense (they
// are given a .terminator).
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
// mark.  However, the mark remains split.
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

bool parse_bracketed_subexpression
	( ll::parser::parser parser,
	  ll::parser::table::selectors selectors,
	  ll::parser::token & current,
	  min::int32 indent,
	  ll::parser::table::indentation_mark
	      indentation_mark,
	  bracket_stack * bracket_stack_p  = NULL );

} }

# endif // LL_PARSER_BRACKETED_H
