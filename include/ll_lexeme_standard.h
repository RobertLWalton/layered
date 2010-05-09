// Layers Language Standard Lexical Analyzer
//
// File:	ll_lexeme_standard.h
// Author:	Bob Walton (walton@seas.harvard.edu)
// Date:	Sat May  8 18:49:06 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/05/09 01:41:51 $
//   $RCSfile: ll_lexeme_standard.h,v $
//   $Revision: 1.2 $

// Table of Contents
//
//	Usage and Setup
//	Lexical Types
//	Program Construction

// Usage and Setup
// ----- --- -----

# ifndef LL_LEXEME_STANDARD_H
# define LL_LEXEME_STANDARD_H

# include <ll_lexeme.h>


// Lexical Types
// ------- -----

namespace ll { namespace lexeme { namespace standard {

    using ll::lexeme::uns32;

    const uns32 word_t			= 1;
    const uns32 natural_number_t	= 2;
    const uns32 number_t		= 3;
    const uns32 mark_t			= 4;
    const uns32 quoted_string_t		= 5;
    const uns32 sep_t			= 6;

    const uns32 comment_t		= 7;
    const uns32 horizontal_space_t	= 8;
    const uns32 line_break_t		= 9;
    const uns32 end_of_file_t		= 10;

    const uns32 bad_end_of_line_t	= 11;
    const uns32 bad_end_of_file_t	= 12;
    const uns32 bad_character_t		= 13;
    const uns32 bad_escape_sequence_t	= 14;

} } }


// Program Construction
// ------- ------------

namespace ll { namespace lexeme { namespace standard {

    void create_standard_program ( void );

} } }

# endif // LL_LEXEME_STANDARD_H
