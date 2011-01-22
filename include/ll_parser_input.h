// Layers Language Parser Inputs
//
// File:	ll_parser_input.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Jan 22 00:58:24 EST 2011
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

// Set a parser input to produce a stream of lexemes
// read from a standard lexeme scanner.  Error messages
// are printed on the parser->err stream unless input->
// trace is true, in which case they are printed on the
// parser->trace stream.
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
// tokens the `end' position may be the beginning of the
// next line.
//
// If a scanner == NULL_STUB is given, a scanner is
// created with default parameters and standard lexeme
// program (see ll_lexeme_standard.h), and a pointer to
// the new scanner is stored in the argument.  The
// erroneous_atom function of the new scanner is set.
//
void init_input
	( input_ptr & input = default_input );

// Set a scanner to the standard lexeme program (see
// ll_lexeme_standard.h) and initialize the scanner.
// Create the scanner if its variable is NULL_STUB.
// Equivalent to ll::lexeme::init_scanner followed
// by installing the standand lexeme program in the
// scanner.
//
void init_scanner
	( ll::lexeme::scanner_ptr & scanner =
	      ll::lexeme::default_scanner );

} }

# endif // LL_PARSER_INPUT_H
