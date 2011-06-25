// Layers Language Parser Inputs
//
// File:	ll_parser_standard.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Jun 25 09:07:31 EDT 2011
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
// The lexeme types are as in ll_lexeme_standard.h.
//
// Tokens with the following lexeme types have min::gen
// token values equal to their lexeme's translation
// character strings:
//
//	word_t
//	mark_t
//	separator_t
//	natural_number_t
//
// Other tokens have min::MISSING() token values.
//
// Tokens with the following types have PAR::string
// token strings equal to their lexeme's translation
// character strings:
//
//	number_t
//	quoted_string_t
//
// Other tokens have min::NULL_STUB token strings.
//
// Tokens with the following types do NOT have their
// lexeme's translation strings recorded in the token:
//
//	line_break_t
//	end_of_file_t
//
// Lexemes of the following types are ignored; that is,
// they are not made into tokens:
//
//	comment_t
//	horizontal_space_t
//
// Lexemes of the following types are announced as
// errors and otherwise ignored, that is, they are not
// made into tokens:
//
//	premature_end_of_line_t
//	premature_end_of_file_t
//	misplaced_char_t
//	misplaced_space_t
//	ascii_escape_seq_t
//	non_letter_escape_seq_t
//
// The following erroneous atoms are announced as
// errors and otherwise ignored:
//
//	misplaced_control_char_t
//	unrecognized_escape_seq_t
//
// Note that indentation and line breaks are implicitly
// encoded in the token `begin' and `end' positions.
// Also note that all tokens output by this input
// closure except the line_break_t tokens are on a
// single line.  Lastly note that natural_number_t
// tokens have min::gen token values that are strings
// and not numbers; therefore 001, 01, and 1 have
// different min::gen token values.
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
