// Layers Language C++ Lexical Analyzer
//
// File:	ll_lexeme_c++.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu Jun  2 07:27:47 EDT 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Lexical Types
//	Program Construction

// Usage and Setup
// ----- --- -----

# ifndef LL_LEXEME_CPP_H
# define LL_LEXEME_CPP_H

# include <ll_lexeme.h>


// Lexical Types
// ------- -----

namespace ll { namespace lexeme { namespace cpp {

    using ll::lexeme::uns32;

    // Lexeme Types

    const uns32 horizontal_space_t		= 1;
    const uns32 comment_t			= 2;
    const uns32 bracketed_header_name_t		= 3;
    const uns32 quoted_header_name_t		= 4;
    const uns32 operator_t			= 5;
    const uns32 identifier_t			= 6;
    const uns32 dec_integer_t			= 7;
    const uns32 oct_integer_t			= 8;
    const uns32 hex_integer_t			= 9;
    const uns32 float_t				= 10;
    const uns32 pp_number_t			= 11;
    const uns32 character_literal_t		= 12;
    const uns32 u_character_literal_t		= 13;
    const uns32 U_character_literal_t		= 14;
    const uns32 L_character_literal_t		= 15;
    const uns32 string_literal_t		= 16;
    const uns32 u_string_literal_t		= 17;
    const uns32 U_string_literal_t		= 18;
    const uns32 L_string_literal_t		= 19;
    const uns32 newline_t			= 20;
    const uns32 end_of_file_t			= 21;

    // Erroneous Atom Types
    //
    const uns32 premature_newline_t		= 22;
    const uns32 premature_end_of_file_t		= 23;
    const uns32 misplaced_character_t		= 24;
    const uns32 ascii_universal_character_t	= 25;
    const uns32 misplaced_vertical_space_t	= 26;

    const unsigned MAX_TYPE = 26;

    extern const char * const type_name[MAX_TYPE+1];

    extern const char * type_code;
        // Type codes for ll:lexeme::test_input.

} } }


// Program Construction
// ------- ------------

namespace ll { namespace lexeme { namespace cpp {

    extern min::locatable_var<ll::lexeme::program>
        default_program;

    // If ll::lexeme::cpp::default_program is not NULL_
    // STUB, this function does nothing.  Otherwise it
    // creates the c++ lexical program in both
    //
    //		ll::lexeme::default_program
    // and	ll::lexeme::cpp::default_program
    //
    void init_cpp_program ( void );

} } }

# endif // LL_LEXEME_CPP_H
