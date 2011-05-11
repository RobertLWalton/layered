// Layers Language Parser Inputs
//
// File:	ll_parser_standard.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue May 10 21:33:04 EDT 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Standard Parser Input
//	Standard Parser Brackets

// Usage and Setup
// ----- --- -----

# ifndef LL_PARSER_STANDARD_H
# define LL_PARSER_STANDARD_H

# include <ll_lexeme.h>
# include <ll_parser.h>
# include <ll_parser_table.h>

// Standard Parser Input
// -------- ------ -----

namespace ll { namespace parser { namespace standard {

extern min::locatable_var<ll::lexeme::erroneous_atom>
    erroneous_atom;

extern min::locatable_var<ll::parser::input>
    input;

// Set a parser input to produce a stream of lexemes
// read from a standard lexeme scanner.
//
// The lexeme types as in ll_lexeme_standard.h are
// processed as follows:
//
//    word_t		    SYMBOL, string value
//    natural_number_t      if < 2**28:
//			        NATURAL_NUMBER,
//			        number value
//			    else:
//			        natural_number_t,
//			        token string
//    number_t		    number_t, token string
//    mark_t		    SYMBOL, string value
//    quoted_string_t       quoted_string_t,
//			    token string
//    separator_t	    SYMBOL, string value
//
//    comment_t		    no token
//    horizontal_space_t    no token
//
//    line_break_t          line_break_t
//    end_of_file_t         end_of_file_t
//
//    bad_end_of_line_t     line_break_t,
//			        output error message
//    bad_end_of_file_t     end_of_file_t,
//			        output error message
//
//    unrecognized_character_t
//			    no token,
//			        output error message
//    non_letter_escape_sequence_t
//			    ignore erroneous atom,
//				output error message
//
// The token type is either the lexeme type or SYMBOL
// or NATURAL_NUMBER.
//
// The translation_buffer is recorded as either a min::
// gen `string value' for SYMBOLs, a min::gen `number
// value' for NATURAL_NUMBERs, or a ll::parser::string
// for lexeme types.  Some tokens, such as line_break_t
// tokens, do not record the translation_buffer at all.
//
// Note that indentation and line breaks are implicitly
// encoded in the token `begin' and `end' positions;
// Horizontal space and comment lexemes are ignored.
// Also note that all tokens output by this input
// closure are on a single line, though in the case of
// line_break_t tokens the `end' position may be the
// beginning of a subsequent line.
//
// This function may only be called if parser or
// parser->scanner is NULL_STUB.  This function
// creates ll::parser::default_standard_erroneous_atom/
// input and executes:
//
//	ll::parser::init ( parser );
//	ll::lexeme::standard::init_standard_program();
//	ll::lexeme::init_program
//		( ll::parser::scanner_ref(parser),
//		  ll::lexeme::standard
//			    ::default_program );
//	min::parser::input_ref(parser) =
//	    ll::parser::default_standard_input;
//	ll::lexeme::erroneous_atom_ref(parser->scanner)
//	    =
//	    ll::parser::default_standard_erroneous_atom;
//
void init_input ( min::ref<ll::parser::parser> parser );

// Standard Parser Brackets
// -------- ------ --------

// Standard Selectors:
//
const ll::parser::table::selectors
    CODE	= ( 1ull << 0 ),
    MATH	= ( 1ull << 1 ),
    TEXT	= ( 1ull << 2 );

// Set the parser->bracket_table and parser->split_table
// for the standard set of layered language brackets and
// indentation marks.
//
// Also set parser->selectors = CODE;
//
void init_brackets
    ( min::ref<ll::parser::parser> parser );

} } }

# endif // LL_PARSER_STANDARD_H
