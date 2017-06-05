// Layered Languages Parser Inputs
//
// File:	ll_parser_standard.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Jun  5 12:21:04 EDT 2017
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Standard Parser Input
//	Standard Parser Block
//	Standard Parser Brackets
//	Standard Parser Operators

// Usage and Setup
// ----- --- -----

# ifndef LL_PARSER_STANDARD_H
# define LL_PARSER_STANDARD_H

# include <ll_lexeme.h>
# include <ll_parser.h>
# include <ll_parser_table.h>
# include <ll_parser_oper.h>

// Standard Parser Input
// -------- ------ -----

namespace ll { namespace parser { namespace standard {

struct erroneous_atom_struct;
    // Subclass of ll::lexeme::erroneous_atom.
typedef min::packed_struct_updptr
	    <erroneous_atom_struct>
        erroneous_atom;
extern const uns32 & ERRONEOUS_ATOM;
    // Subtype of ll::lexeme::ERRONEOUS_ATOM.
struct erroneous_atom_struct :
    public ll::lexeme::erroneous_atom_struct
{
    const ll::parser::parser parser;
};
MIN_REF ( ll::parser::parser, parser,
          ll::parser::standard::erroneous_atom )

extern min::locatable_var
	<ll::parser::standard::erroneous_atom>
    default_erroneous_atom;

extern min::locatable_var<ll::parser::input>
    input;

// Set a parser input to produce a stream of tokens
// encoding lexemes read from a standard lexeme scanner.
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
//	natural_t
//
// These are called `symbol tokens'.  Other tokens have
// min::MISSING() token values.
//
// Tokens with the following types have PAR::string
// token strings equal to their lexeme's translation
// character strings:
//
//	numeric_t
//	quoted_string_t
//
// Other tokens have min::NULL_STUB token strings.
//
// Tokens with the following types do NOT have their
// lexeme's translation strings recorded in the token:
//
//	indent_t
//	indent_before_comment_t
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
//	premature_end_of_string_t
//	premature_end_of_file_t
//	misplaced_vertical_t
//	illegal_control_t
//	unrecognized_character_t
//
// The following erroneous atoms are announced as
// errors and otherwise ignored:
//
//	misplaced_vertical_t
//	illegal_control_t
//	unrecognized_character_t
//	unrecognized_escape_t
//	misplaced_horizontal_t
//
// The token position encodes the position of the token
// lexeme string, even for tokens whose lexeme transla-
// tion strings are discarded.  The indent of the first
// character AFTER the token, or of the end of input, is
// recorded in the token indent, for use with indent_t
// and indent_before_comment_t tokens to determine line
// indent.
//
// When called this closure function adds tokens until
// it has added a token of one of the types:
//
//	indent_t
//	indent_before_comment_t
//	end_of_file_t
//
// This permits the scanner master table to be switched
// just before the first non-horizontal-space lexeme
// on a line is scanned.  This closure function may also
// stop adding tokens after some reasonable number of
// tokens, such as 100, have been added, to keep memory
// usage within bounds (in case input contains a lot
// of consecutive blank lines).
//
// Note that natural_t tokens have min::gen token values
// that are strings and not numbers, in spite of the 1-1
// correspondence between natural lexeme numbers and
// strings.
//
// This init_input function may only be called if parser
// or parser->scanner is NULL_STUB.  This function
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

// Standard Parser Block
// -------- ------ -----

// Begin the standard parser block and set
// parser->selectors to the "code" selector.
//
void init_block ( ll::parser::parser parser );


// Standard Parser Lexeme Map
// -------- ------ ------ ---

// Set parser->lexeme_map to map the standard set
// of layered language special lexemes, and set
// parser->{paragaph,line}_lexical_master to produce
// these special lexemes at top level.
//
void init_lexeme_map ( ll::parser::parser parser );


// Standard Parser Brackets
// -------- ------ --------

// Set the bracketed_pass->bracket_table for the
// standard set of layered language brackets and
// indentation marks.
//
// Set parser->selectors to `code + top level'.
//
// Note: bracketed_pass = parser->pass_stack, as it is
// always the first pass in pass_stack.
//
void init_brackets ( ll::parser::parser parser );


// Standard Prefixes
// -------- --------

// Set the bracketed_pass->bracket_table for the
// standard set of layered language prefixes.
//
void init_prefixes ( ll::parser::parser parser );


// Standard Parser Operators
// -------- ------ ---------

// Add an oper_pass to the parser and set its oper_table
// to the standard operators.  Return the oper_pass.
//
ll::parser::oper::oper_pass init_oper
    ( ll::parser::parser parser,
      ll::parser::pass next = NULL_STUB );


} } }

# endif // LL_PARSER_STANDARD_H
