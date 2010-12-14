// Layers Language Standard Parsers
//
// File:	ll_parser_standard.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Dec 13 19:08:32 EST 2010
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
    //    word		    NAME, SYMBOL, string value
    //    natural_number    if < 2**28:
    //				NAME, NATURAL_NUMBER,
    //				      number value
    //			    else:
    //			        LEXEME, natural_number,
    //				        translated
    //					string
    //    number	    LEXEME, number,
    //				    translated string
    //    mark		    NAME, SYMBOL, string value
    //    quoted_string	    LEXEME, quoted_string,
    //				    translated string
    //    separator	    NAME, SYMBOL, string value
    //    comment	    no token
    //    horizontal_space  no token
    //    line_break        LEXEME, line_break,
    //				    translated string
    //    end_of_file       LEXEME, end_of_file,
    //				    translated string
    //    bad_end_of_line   LEXEME, line_break,
    //				    translated string,
    //				    output error message
    //    bad_end_of_file   no token,
    //				    output error message
    //    unrecognized_character
    //			    no token,
    //				    output error message
    //    unrecognized_escape_character
    //			    no token,
    //				    output error message
    //			    erroneous atom,
    //				    output error message
    //    unrecognized_escape_sequence
    //			    no token,
    //				    output error message
    //			    erroneous atom,
    //				    output error message
    //    non_letter_escape_sequence
    //			    erroneous atom,
    //				    output error message
    //
    pass_ptr create_input_pass
        ( ll::lexeme::scanner_ptr scanner );

} } }

# endif // LL_PARSER_STANDARD_H
