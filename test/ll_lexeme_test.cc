// Layers Language Lexical Analyzer Test
//
// File:	ll_lexeme_test.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Wed Apr 14 04:20:31 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/04/14 13:30:04 $
//   $RCSfile: ll_lexeme_test.cc,v $
//   $Revision: 1.6 $

// Table of Contents
//
//	Usage and Setup
//	External Runtime
//	Program Construction Test

// Usage and Setup
// ----- --- -----

# include <ll_lexeme.h>
# include <iostream>
# include <iomanip>
# include <cstdlib>
# include <cstring>
# include <cassert>
# define LEX ll::lexeme
using std::cout;
using std::endl;
using std::setw;
using std::ios;
using std::ostream;
using LEX::uns8;
using LEX::uns32;

using LEX::ACCEPT;
using LEX::KEEP;
using LEX::TRANSLATE;
using LEX::TRANSLATE_HEX;
using LEX::TRANSLATE_OCT;
using LEX::GOTO;
using LEX::SHORTCUT;


// Program Construction Test
// ------- ------------ ----

static void create_program_1 ( void )
{

    uns32 master = LEX::create_program();

    uns32 atable1 =
        LEX::create_atom_table ( LEX::LEXEME, 1 );
    uns32 dispatcher1 =
        LEX::create_dispatcher ( 10, 4 );
    uns8 map1[10] = { 0, 1, 2, 3, 4, 5, 4, 3, 2, 1 };
    uns32 tmap1 =
        LEX::create_type_map
	    ( '0', '9', map1 );
    uns32 tmap2 =
        LEX::create_type_map ( 'a', 'z', 4 );
    uns32 translation[3] = { 'X', 'Y', 'Z' };
    uns32 instruction1 =
        LEX::create_instruction
	    ( KEEP(1)+TRANSLATE(3)+GOTO,
	      atable1,
	      translation );
    uns32 instruction2 =
        LEX::create_instruction ( ACCEPT );

    LEX::attach ( atable1, instruction1 );
    LEX::attach ( atable1, dispatcher1 );

    LEX::attach ( dispatcher1, tmap1 );
    LEX::attach ( dispatcher1, tmap2 );

    LEX::attach ( dispatcher1, 1, instruction1 );
    LEX::attach ( dispatcher1, 2, dispatcher1 );
    LEX::attach ( dispatcher1, 4, instruction2 );

    cout << "Uncooked Program 1:" << endl << endl;
    LEX::print_program ( cout, false );
    cout << "Cooked Program 1:" << endl << endl;
    LEX::print_program ( cout, true );
}

int main ( int argc )
{
    create_program_1();
}
