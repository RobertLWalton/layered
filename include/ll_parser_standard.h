// Layers Language Standard Parsers
//
// File:	ll_parser_standard.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Dec 27 21:44:07 EST 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Pass Construction

// Usage and Setup
// ----- --- -----

# ifndef LL_PARSER_STANDARD_H
# define LL_PARSER_STANDARD_H

# include <ll_lexeme.h>
# include <ll_parser_pass.h>

// Pass Construction
// ---- ------------

namespace ll { namespace parser { namespace standard {

    // Create an input pass that outputs a stream of
    // lexemes as read from a standard lexeme scanner.
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
    //    comment_t	    no token
    //    horizontal_space_t
    //			    no token
    //    line_break_t      line_break_t, string
    //    end_of_file_t     end_of_file_t, string
    //    bad_end_of_line_t line_break_t, string,
    //			    output error message
    //    bad_end_of_file_t no token,
    //			    output error message
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
    // a min::gen `string value' for SYMBOLs or a
    // ll::parser::string for lexeme type kinds.
    //
    pass_ptr create_input_pass
        ( ll::lexeme::scanner_ptr scanner );

} } }

# endif // LL_PARSER_STANDARD_H
