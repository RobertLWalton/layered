// Layers Language Standard Parsers
//
// File:	ll_parser_standard.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Jan 12 15:23:26 EST 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Parser Definition Stack
//	Pass Construction

// Usage and Setup
// ----- --- -----

# ifndef LL_PARSER_STANDARD_H
# define LL_PARSER_STANDARD_H

# include <ll_lexeme.h>
# include <ll_parser_pass.h>
# include <ll_parser_table.h>

// Parser Definition Stack
// ------ ---------- -----

namespace ll { namespace parser { namespace standard {

// There is one subexpression struct for each current
// subexpression.
//
struct subexpression_struct
{
    // Pass that recognizes explicit subexpressions.
    //
    pass_ptr explicit_subexpression_pass;
};

struct definition_stack_struct;
typedef min::packed_struct_updptr
	    <definition_stack_struct>
    definition_stack_ptr;
struct definition_stack_struct
{
    uns32 control;

    // Lexical scanner with standard program.
    //
    LEX::scanner_ptr scanner;

    // Input pass that outputs lexemes.
    //
    pass_ptr input_pass;

    // Explict subexpression parsing pass stack.
    // There is one explicit subexpression parsing pass
    // for each current explicit subexpression.
    //
    min::packed_vec_insptr<pass_ptr>
        explicit_subexpression_pass;





};

} } }

// Pass Construction
// ---- ------------

namespace ll { namespace parser { namespace standard {

    // Create an input pass that outputs a stream of
    // lexemes as read from a standard lexeme scanner.
    // Error messages are printed on the err stream and
    // trace messages on trace stream (which always
    // takes precedence over the err stream for error
    // messages).  The scanner->erronous_atom parameter
    // is set by this function.
    //
    // The lexeme types as in ll_lexeme_standard.h are
    // processed as follows:
    //
    //    word_t	    SYMBOL, string value
    //    natural_number_t  if < 2**28:
    //				NATURAL_NUMBER,
    //				number value
    //			    else:
    //			        natural_number_t,
    //				string
    //    number_t	    number_t, string
    //    mark_t	    SYMBOL, string value
    //    quoted_string_t   quoted_string_t, string
    //    separator_t	    SYMBOL, string value
    //
    //    comment_t	    no token
    //    horizontal_space_t
    //			    no token
    //
    //    line_break_t      line_break_t
    //    end_of_file_t     end_of_file_t
    //
    //    bad_end_of_line_t line_break_t
    //			    output error message
    //    bad_end_of_file_t end_of_file_t
    //			    output error message
    //
    //    unrecognized_character_t
    //			    no token,
    //			    output error message
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
    // The token kind is either the lexeme type or
    // SYMBOL or NATURAL_NUMBER.
    //
    // The translation_buffer is recorded as either
    // a min::gen `string value' for SYMBOLs, a min::gen
    // `number value' for NATURAL_NUMBERs, or a ll::
    // parser::string for lexeme type kinds.  Some
    // tokens, such as line_break_t tokens, do not
    // record the translation_buffer at all.
    //
    // Note that indentation and line breaks are
    // implicitly encoded in the token `begin' and
    // `end' positions.  Also note that all tokens
    // output by this pass are on a single line, though
    // in the case of line_break_t tokens the `end'
    // position may be the beginning of the next line.
    //
    pass_ptr create_input_pass
        ( ll::lexeme::scanner_ptr scanner =
	      ll::lexeme::default_scanner,
	  std::ostream & err = * default_err,
	  std::ostream & trace = * default_trace );

} } }

# endif // LL_PARSER_STANDARD_H
