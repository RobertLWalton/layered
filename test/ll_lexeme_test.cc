// Layers Language Lexical Analyzer Test
//
// File:	ll_lexeme_test.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Mon Apr 12 06:10:08 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/04/12 10:57:05 $
//   $RCSfile: ll_lexeme_test.cc,v $
//   $Revision: 1.3 $

// Table of Contents
//
//	Usage and Setup
//	External Runtime
//	Program Creation Test

// Usage and Setup
// ----- --- -----

# include <ll_lexeme.h>
# include <iostream>
# include <iomanip>
# include <cstdlib>
# include <cstring>
# include <cassert>
# define LLLEX ll::lexeme
using std::cout;
using std::endl;
using std::setw;
using std::ios;
using std::ostream;
using LLLEX::uns8;
using LLLEX::uns32;


// Program Creation Test
// ------- -------- ----

static void create_program_1 ( void )
{
    using LLLEX::ACCEPT;
    using LLLEX::DISCARD;
    using LLLEX::KEEP;
    using LLLEX::TRUNCATE;
    using LLLEX::TRANSLATE;
    using LLLEX::TRANSLATE_HEX;
    using LLLEX::TRANSLATE_OCT;
    using LLLEX::GOTO;
    using LLLEX::SHORTCUT;

    uns32 master = LLLEX::create_program();

    uns32 atable1 =
        LLLEX::create_atom_table ( LLLEX::LEXEME, 1 );
    uns32 dispatcher1 =
        LLLEX::create_dispatcher ( 10, 4 );
    uns8 map1[6] = { 0, 1, 2, 3, 4, 5 };
    uns32 tmap1 =
        LLLEX::create_type_map
	    ( '0', '5', map1 );
    uns32 tmap2 =
        LLLEX::create_type_map ( 'a', 'z', 9 );
    uns32 translation[3] = { 'X', 'Y', 'Z' };
    uns32 instruction1 =
        LLLEX::create_instruction
	    ( ACCEPT+TRUNCATE(1)+TRANSLATE(3)+GOTO,
	      atable1,
	      translation );

    LLLEX::attach ( atable1, instruction1 );
    LLLEX::attach ( atable1, dispatcher1 );

    LLLEX::attach ( dispatcher1, tmap1 );

    LLLEX::attach ( dispatcher1, 1, instruction1 );
    LLLEX::attach ( dispatcher1, 2, dispatcher1 );
}

int main ( int argc )
{
    create_program_1();
    LLLEX::print_program ( cout, false );
}
