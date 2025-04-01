// Layered Languages Parser Inputs
//
// File:	ll_parser_standard.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Mar 31 08:10:53 PM EDT 2025
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Standard Parser Input
//	Define Standard

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
//	numeric_t
//
//	quoted_string_t
//
// The first four types are called `symbol tokens', and
// the last type is called a `quoted string token'.
//
// Tokens with the following lexeme types have min::gen
// token values equal to the min::strto conversion of
// the lexeme's translation character string to a
// min::float64:
//
//	numeric_word_t
//	natural_t
//	number_t
//
// These are called `number tokens'.
//
// Other lexical tokens have min::MISSING() token
// values.
//
// Tokens with the following types do NOT have their
// lexeme's translation strings recorded in the token:
//
//	indent_t
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
// recorded in the token indent, for use with the
// indent_t token to determine line indent.
//
// When called this closure function adds tokens until
// it has added a token of one of the types:
//
//	indent_t
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
// This closure function also looks up each token type
// in the parser lexeme_map and if found does the
// following:
//
//     (1) Resets the lexical master to the lexeme
//         map entry lexical_master if that is not
//         ll::lexeme::MISSING_MASTER.
//
//     (2) Deletes the token and does nothing else if
//         the lexical map entry token value is
//         min::NONE().
//
//     (3) Resets the token value from the lexeme map
//         entry.
//
// Lastly this closure function replaces numeric tokens
// that consist of the parser ID_character (if that is
// not NO_UCHAR) followed by digits with no high order
// zeros.  The ID defined by the digits is looked up in
// the parser id_map, and if found, the token value is
// replaced by the found value.  If not found, a PRE-
// ALLOCATED stub is created and installed for the ID
// in the id_map and as the token value.
//
// Whenever the token value is reset according to the
// parser lexeme_map or id_map, the token type and
// value_type are reset as per the ll::parser::find_
// token_type function.
//
// This init_input function may only be called if parser
// or parser->scanner is NULL_STUB.  This function
// creates ll::parser::default_standard_erroneous_atom/
// input and executes:
//
//	ll::parser::init ( parser, 0 );
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

// Standard Parser Components
// -------- ------ ----------

enum {
    LABEL			= 1 << 0,
        // Create and enable `label' selector.
    CODE			= 1 << 1,
        // Create and enable `code' selector.
    TEXT			= 1 << 2,
        // Create and enable `text' selector.
    MATH			= 1 << 3,
        // Create and enable `math' selector.
    ID				= 1 << 4,
        // Enable ID for TOP_LEVEL, LEXICAL_MAP,
	// INDENTATION_MARKS, and BRACKET_TYPES
	// below.
    TABLE			= 1 << 5,
        // Enable TABLE for TOP_LEVEL, LEXICAL_MAP,
	// INDENTATION_MARKS, and BRACKET_TYPES
	// below.

    ALL_QUALIFIERS		= LABEL
    				| CODE
    				| TEXT
    				| MATH
				| ID
    				| TABLE,

    BLOCK			= 1 << 6,
        // Create block named `standard'.

    TOP_LEVEL			= 1 << 7,
        // Set top level selector to first enabled of
	// CODE, TEXT, MATH; set top level lexical
	// masters according to ID and TABLE; set
	// ID character to @ if ID enabled.
    CONCATENATOR                = 1 << 8,
        // Set concatenator character to #.
    LEXEME_MAP                  = 1 << 9,
        // Set lexeme map according to ID and TABLE.

    ALL_SETUP			= TOP_LEVEL
    				| CONCATENATOR
    				| LEXEME_MAP,

    BRACKETS			= 1 << 10,
        // Set standard bracket entries omitting
	// DISabled CODE, TEXT, and MATH selectors.
    PARAGRAPH			= 1 << 11,
        // Set the {p}, {section}, {quote} entries
	// if TEXT is selected.
    SENTENCE			= 1 << 12,
        // Set the {s} entry if TEXT is selected.

    INDENTATION_MARKS		= CODE
				| ID,

    BRACKET_TYPES		= CODE
    				| TEXT
				| ID
				| TABLE
				| PARAGRAPH
				| SENTENCE,

    ALL_BRACKETS		= BRACKETS
				| INDENTATION_MARKS
				| BRACKET_TYPES,
	// Set all bracket related entries
	// omitting DISabled CODE, TEXT, and MATH
	// selectors.
    CONTROL_OPERATORS		= 1 << 13,
        // Set standard control operator entries
	// omitting DISabled CODE and MATH selectors.
    ITERATION_OPERATORS		= 1 << 14,
        // Set standard iteration operator entries
	// omitting DISabled CODE selectors.
    ASSIGNMENT_OPERATORS	= 1 << 15,
        // Set standard assignment operator entries
	// omitting DISabled CODE and MATH selectors.
    SELECTION_OPERATORS		= 1 << 16,
        // Set standard selector operator entries
	// omitting DISabled CODE and MATH selectors.
    LOGICAL_OPERATORS		= 1 << 17,
        // Set standard logical operator entries
	// omitting DISabled CODE and MATH selectors.
    COMPARISON_OPERATORS	= 1 << 18,
        // Set standard comparison operator entries
	// omitting DISabled CODE and MATH selectors.
    ARITHMETIC_OPERATORS	= 1 << 19,
        // Set standard arithmetic operator entries
	// omitting DISabled CODE and MATH selectors.
    BITWISE_OPERATORS		= 1 << 20,
        // Set standard bitwise operator entries
	// omitting DISabled CODE and MATH selectors.
    TEST_OPERATORS		= 1 << 21,
        // Set standard test operator entries
	// omitting DISabled CODE and MATH selectors.
    ALL_OPERATORS		= CONTROL_OPERATORS
                        	| ITERATION_OPERATORS
                        	| ASSIGNMENT_OPERATORS
                        	| SELECTION_OPERATORS
                        	| LOGICAL_OPERATORS
                        	| COMPARISON_OPERATORS
                        	| ARITHMETIC_OPERATORS
                        	| BITWISE_OPERATORS
                        	| TEST_OPERATORS,
        // Set all ..._OPERATOR entries.

    PRIMARY_PARSING		= 1 << 22,
        // Install primary pass which provides tables
	// for parsing primary separators and the
	// primary parsing functions (see
	// ll_parser_primary.h).
    PRIMARY_SEPARATORS		= 1 << 23,
        // Set standard primary separator entries
	// omitting DISabled CODE selectors.
    PRIMARY_OPERATORS		= 1 << 24,
        // Set operator entries using MEX instruction
	// codes in primary symbol table according to
	// the ..._OPERATORS flags (e.g., this flag and
	// ARITHMETIC_OPERATORS sets the symbol_table
	// entries for arithmetic operators).

    ALL				= 0xFFFFFFFF
        // Set all standard entries.

};

extern min::locatable_var<ll::parser::table::name_table>
    component_name_table;
extern min::locatable_var<ll::parser::table::key_table>
    component_group_name_table;

// Define Standard
// ------ --------

// Set the bracketed_pass->bracket_table for the
// standard set of layered language brackets and
// indentation marks, and bracketed_pass->bracket_
// type_table for standard bracket types.
//
// Note: bracketed_pass = parser->pass_stack, as it is
// always the first pass in pass_stack.
//
void define_brackets
    ( ll::parser::parser parser,
      ll::parser::table::flags components =
            CODE + TEXT + MATH + ID + TABLE
	  + ALL_BRACKETS );

// Set the oper_pass tables to the standard operators
// of various ..._OPERATOR components and the MATH and
// CODE components.  Add math and code selectors if
// needed and not present.  Add the oper_pass if needed
// and not present.
//
void define_operators
    ( ll::parser::parser parser,
      ll::parser::table::flags components =
          MATH + CODE + ALL_OPERATORS );

// Define primary pass, thus providing tables for
// primary parsing functions (ll_parser_primary.h),
//
void define_primary
    ( ll::parser::parser parser,
      ll::parser::table::flags components =
          PRIMARY_PARSING );

// Define all standard parser components indicated in
// the components argument.
//
void define_standard
    ( ll::parser::parser parser,
      ll::parser::table::flags components = ALL );

} } }

# endif // LL_PARSER_STANDARD_H
