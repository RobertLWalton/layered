// Layers Language Parser Inputs
//
// File:	ll_parser_input.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri Feb 25 13:19:55 EST 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Input Parser

// Usage and Setup
// ----- --- -----

# ifndef LL_PARSER_INPUT_H
# define LL_PARSER_INPUT_H

# include <ll_lexeme.h>
# include <ll_parser.h>

// Input Parser
// ----- ------

namespace ll { namespace parser {

extern min::locatable_ptr<ll::lexeme::erroneous_atom>
    default_standard_erroneous_atom;

extern min::locatable_ptr<ll::parser::input>
    default_standard_input;

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
//			        string
//    number_t		    number_t, string
//    mark_t		    SYMBOL, string value
//    quoted_string_t       quoted_string_t, string
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
//    unrecognized_escape_character_t
//			    no token,
//			        output error message
//			    erroneous atom,
//			        output error message
//    unrecognized_escape_sequence_t
//			    no token,
//			        output error message
//			    erroneous atom,
//			        output error message
//    non_letter_escape_sequence_t
//			    erroneous atom,
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
// Also note that all tokens output by this pass are on
// a single line, though in the case of line_break_t
// tokens the `end' position may be the beginning of
// a subsequent line.
//
// This function may only be called if parser or
// parser->scanner is NULL_STUB.  This function
// creates ll::parser::default_standard_erroneous_atom/
// input and executes:
//
//	ll::parser::init ( parser );
//	ll::lexeme::standard::init_standard_program();
//	ll::lexeme::init_program
//		( parser->scanner,
//		  ll::lexeme::standard
//			    ::default_program );
//	parser->input =
//	    ll::parser::default_standard_input;
//	parser->scanner->erroneous_atom =
//	    ll::parser::default_standard_erroneous_atom;
//
void init_standard_input
	( ll::parser::parser & parser );

} }

# endif // LL_PARSER_INPUT_H
