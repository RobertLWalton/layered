// Layers Language Lexical Analyzer Test
//
// File:	ll_lexeme_test.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Fri Apr 16 04:16:03 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/04/16 08:21:59 $
//   $RCSfile: ll_lexeme_test.cc,v $
//   $Revision: 1.7 $

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

static void create_program_2 ( void )
{

    uns32 master = LEX::create_program();
    uns32 whitespace =
        LEX::create_atom_table ( LEX::WHITESPACE, 1 );
    uns32 symbol =
        LEX::create_atom_table ( LEX::LEXEME, 2 );
    uns32 number =
        LEX::create_atom_table ( LEX::LEXEME, 3 );
    uns32 separator =
        LEX::create_atom_table ( LEX::LEXEME, 4 );
    uns32 error =
        LEX::create_atom_table ( LEX::ERROR, 5 );

    const uns8 white = 1;
    const uns8 letter = 2;
    const uns8 digit = 3;
    const uns8 point = 4;
    const uns8 sep = 5;
    const uns8 op = 6;

    uns8 cmap[128] =
        {
            0,		// NUL ’\0’
            0,		// SOH (start of heading)
            0,		// STX (start of text)
            0,		// ETX (end of text)
            0,		// EOT (end of transmission)
            0,		// ENQ (enquiry)
            0,		// ACK (acknowledge)
            0,		// BEL ’\a’ (bell)
            0,		// BS  ’\b’ (backspace)
            white,	// HT  ’\t’ (horizontal tab)
            white,	// LF  ’\n’ (new line)
            white,	// VT  ’\v’ (vertical tab)
            white,	// FF  ’\f’ (form feed)
            white,	// CR  ’\r’ (carriage ret)
            0,		// SO  (shift out)
            0,		// SI  (shift in)
            0,		// DLE (data link escape)
            0,		// DC1 (device control 1)
            0,		// DC2 (device control 2)
            0,		// DC3 (device control 3)
            0,		// DC4 (device control 4)
            0,		// NAK (negative ack.)
            0,		// SYN (synchronous idle)
            0,		// ETB (end of trans. blk)
            0,		// CAN (cancel)
            0,		// EM  (end of medium)
            0,		// SUB (substitute)
            0,		// ESC (escape)
            0,		// FS  (file separator)
            0,		// GS  (group separator)
            0,		// RS  (record separator)
            0,		// US  (unit separator)
            white,	// SPACE
            0,		// !
            0,		// "
            0,		// #
            0,		// $
            0,		// %
            0,		// &
            0,		// ’
            sep,	// (
            sep,	// )
            op,		// *
            op,		// +
            sep,	// ,
            op,		// -
            point,	// .
            op,		// /
            digit,	// 0
            digit,	// 1
            digit,	// 2
            digit,	// 3
            digit,	// 4
            digit,	// 5
            digit,	// 6
            digit,	// 7
            digit,	// 8
            digit,	// 9
            0,		// :
            sep,	// ;
            0,		// <
            0,		// =
            0,		// >
            0,		// ?
            0,		// @
            letter,	// A
            letter,	// B
            letter,	// C
            letter,	// D
            letter,	// E
            letter,	// F
            letter,	// G
            letter,	// H
            letter,	// I
            letter,	// J
            letter,	// K
            letter,	// L
            letter,	// M
            letter,	// N
            letter,	// O
            letter,	// P
            letter,	// Q
            letter,	// R
            letter,	// S
            letter,	// T
            letter,	// U
            letter,	// V
            letter,	// W
            letter,	// X
            letter,	// Y
            letter,	// Z
            sep,	// [
            0,		// \  ’\\’
            sep,	// ]
            0,		// ^
            0,		// _
            0,		// `
            letter,	// a
            letter,	// b
            letter,	// c
            letter,	// d
            letter,	// e
            letter,	// f
            letter,	// g
            letter,	// h
            letter,	// i
            letter,	// j
            letter,	// k
            letter,	// l
            letter,	// m
            letter,	// n
            letter,	// o
            letter,	// p
            letter,	// q
            letter,	// r
            letter,	// s
            letter,	// t
            letter,	// u
            letter,	// v
            letter,	// w
            letter,	// x
            letter,	// y
            letter,	// z
            sep,	// {
            0,		// |
            sep,	// }
            0,		// ~
            0,		// DEL
        };
    uns32 tmap = LEX::create_type_map ( 0, 127, cmap );
    uns32 master_dispatcher =
        LEX::create_dispatcher ( 3, 10 );
    LEX::attach ( master, master_dispatcher );
    LEX::attach ( master_dispatcher, tmap );
    uns32 symbol_instruction =
        LEX::create_instruction
	    ( KEEP(0)+GOTO, symbol );
    LEX::attach ( master_dispatcher, letter,
                  symbol_instruction );


    cout << "Cooked Program 2:" << endl << endl;
    LEX::print_program ( cout, true );
}

int main ( int argc )
{
    create_program_1();
    create_program_2();
}
