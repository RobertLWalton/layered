// Layers Language Lexical Analyzer Test
//
// File:	ll_lexeme_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat May  8 05:10:30 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/05/08 09:24:11 $
//   $RCSfile: ll_lexeme_test.cc,v $
//   $Revision: 1.22 $

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
using LEX::ELSE;
using LEX::ERRONEOUS_ATOM;
using LEX::GOTO;
using LEX::CALLRETURN;
using LEX::OUTPUT;

// Setup input to lexical scanner to read the given
// string of characters and then return end of file.
//
void set_lex_input ( uns32 * input, uns32 length );


// Program Construction Test
// ------- ------------ ----

static void check_attach
        ( uns32 target_ID, uns32 component_ID )
{
    if ( ! LEX::attach ( target_ID, component_ID ) )
        cout << "ATTACH_ERROR" << endl
	     << LEX::error_message << endl;
}

static void check_attach
        ( uns32 target_ID, uns32 ctype,
	  uns32 component_ID )
{
    if ( ! LEX::attach
              ( target_ID, ctype, component_ID ) )
        cout << "ATTACH_ERROR" << endl
	     << LEX::error_message << endl;
}

static void create_program_1 ( void )
{

    uns32 master = LEX::create_program();

    uns32 atable1 =
        LEX::create_atom_table ( 5 );
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
	      atable1, 0,
	      translation );
    uns32 instruction2 =
        LEX::create_instruction ( ACCEPT );

    check_attach ( atable1, instruction1 );
    check_attach ( atable1, dispatcher1 );

    check_attach ( dispatcher1, tmap1 );
    check_attach ( dispatcher1, tmap2 );

    check_attach ( dispatcher1, 1, instruction1 );
    check_attach ( dispatcher1, 2, dispatcher1 );
    check_attach ( dispatcher1, 4, instruction2 );

    cout << "Uncooked Program 1:" << endl << endl;
    LEX::print_program ( cout, false );
    cout << "Cooked Program 1:" << endl << endl;
    LEX::print_program ( cout, true );
}

static void create_program_2 ( void )
{

    uns32 master = LEX::create_program();
    uns32 whitespace =
        LEX::create_atom_table ( LEX::WHITESPACE );
    uns32 symbol =
        LEX::create_atom_table ( 5 );
    uns32 number =
        LEX::create_atom_table ( 6 );
    uns32 oper =
        LEX::create_atom_table ( 7 );
    uns32 separator = 8;
        // Separator type for OUTPUT.

    const uns8 white = 1;
    const uns8 letter = 2;
    const uns8 digit = 3;
    const uns8 point = 4;
    const uns8 sep = 5;
    const uns8 op = 6;
    const uns8 escape = 7;
    const uns8 err_atom = 8;

    uns8 cmap[128] =
        {
            0,          // NUL '\0'
            0,		// SOH (start of heading)
            0,		// STX (start of text)
            0,		// ETX (end of text)
            0,		// EOT (end of transmission)
            0,		// ENQ (enquiry)
            0,		// ACK (acknowledge)
            0,		// BEL '\a' (bell)
            0,		// BS  '\b' (backspace)
            white,	// HT  '\t' (horizontal tab)
            white,	// LF  '\n' (new line)
            white,	// VT  '\v' (vertical tab)
            white,	// FF  '\f' (form feed)
            white,	// CR  '\r' (carriage ret)
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
            0,		// '
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
            escape,	// \  '\\'
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
            err_atom	// DEL
        };

    uns32 tmap = LEX::create_type_map ( 0, 127, cmap );
    uns32 master_dispatcher =
        LEX::create_dispatcher ( 3, 10 );
    check_attach ( master, master_dispatcher );
    check_attach ( master_dispatcher, tmap );
    uns32 symbol_instruction =
        LEX::create_instruction
	    ( KEEP(0)+CALLRETURN, symbol );
    uns32 number_instruction =
        LEX::create_instruction
	    ( KEEP(0)+GOTO, number );
    uns32 whitespace_instruction =
        LEX::create_instruction
	    ( ACCEPT+GOTO, whitespace );
    uns32 operator_instruction =
        LEX::create_instruction
	    ( KEEP(0)+GOTO, oper );
    uns32 separator_instruction =
        LEX::create_instruction
	    ( ACCEPT+OUTPUT, 0, separator );
    uns32 error_instruction =
        LEX::create_instruction
	    ( ACCEPT+OUTPUT, 0, LEX::ERROR );
    uns32 err_atom_instruction =
        LEX::create_instruction
	    ( ERRONEOUS_ATOM+TRANSLATE(0), 0, 100 );
    check_attach ( master_dispatcher, 0,
                   error_instruction );
    check_attach ( master_dispatcher, white,
                   whitespace_instruction );
    check_attach ( master_dispatcher, letter,
                   symbol_instruction );
    check_attach ( master_dispatcher, escape,
                   symbol_instruction );
    check_attach ( master_dispatcher, digit,
                   number_instruction );
    check_attach ( master_dispatcher, point,
                   number_instruction );
    check_attach ( master_dispatcher, sep,
                   separator_instruction );
    check_attach ( master_dispatcher, op,
                   operator_instruction );

    uns32 master_instruction =
        LEX::create_instruction
	    ( KEEP(0)+GOTO, master );
    uns32 return_instruction =
        LEX::create_instruction
	    ( KEEP(0)+CALLRETURN, 0 );
    uns32 accept_instruction =
        LEX::create_instruction ( ACCEPT );

    uns32 symbol_dispatcher =
        LEX::create_dispatcher ( 3, 10 );
    check_attach ( symbol_dispatcher, tmap );

    check_attach ( symbol, symbol_dispatcher );
    check_attach ( symbol, return_instruction );
    check_attach ( symbol_dispatcher, letter,
                   accept_instruction );
    check_attach ( symbol_dispatcher, digit,
                   accept_instruction );
    check_attach ( symbol_dispatcher, err_atom,
                   err_atom_instruction );

    // Arrange so \ooo can appear in a symbol if
    // ooo encodes a letter.
    //
    uns32 letter_dispatcher =
        LEX::create_dispatcher ( 5, 2 );
    uns32 letter_tmap1 =
        LEX::create_type_map ( 'a', 'z', 1 );
    uns32 letter_tmap2 =
        LEX::create_type_map ( 'A', 'Z', 1 );
    check_attach ( letter_dispatcher, letter_tmap1 );
    check_attach ( letter_dispatcher, letter_tmap2 );

    uns32 oct_tmap =
        LEX::create_type_map ( '0', '7', 1 );
    uns32 oct_dispatcher1 =
        LEX::create_dispatcher ( 3, 2 );
    check_attach ( oct_dispatcher1, oct_tmap );
    uns32 oct_dispatcher2 =
        LEX::create_dispatcher ( 3, 2 );
    check_attach ( oct_dispatcher2, oct_tmap );
    uns32 oct_dispatcher3 =
        LEX::create_dispatcher ( 3, 2 );
    check_attach ( oct_dispatcher3, oct_tmap );
    check_attach ( symbol_dispatcher,
                   escape, oct_dispatcher1 );
    check_attach ( oct_dispatcher1,
                   1, oct_dispatcher2 );
    check_attach ( oct_dispatcher2,
                   1, oct_dispatcher3 );
    uns32 translate_oct_instruction =
        LEX::create_instruction
	    ( TRANSLATE_OCT(1,0)+ELSE,
	      0, 0, NULL, letter_dispatcher,
	                  err_atom_instruction );
    check_attach ( oct_dispatcher3,
                   1, translate_oct_instruction );

    uns32 fraction = LEX::create_atom_table ( 6 );
    uns32 fraction_instruction =
        LEX::create_instruction
	    ( ACCEPT+GOTO, fraction );

    uns32 digit_instruction =
        LEX::create_instruction
	    ( ACCEPT+GOTO, number );
    uns32 number_dispatcher =
        LEX::create_dispatcher ( 5, 10 );
    uns32 digit_map =
        LEX::create_type_map ( '0', '9', 1 );
    uns32 point_map =
        LEX::create_type_map ( '.', '.', 2 );
    check_attach ( number, number_dispatcher );
    check_attach ( number, master_instruction );
    check_attach ( number_dispatcher, digit_map );
    check_attach ( number_dispatcher, point_map );
    check_attach ( number_dispatcher, 1,
    	           digit_instruction );
    check_attach ( number_dispatcher, 2,
    	           fraction_instruction );

    uns32 fraction_dispatcher =
        LEX::create_dispatcher ( 3, 10 );
    check_attach ( fraction, fraction_dispatcher );
    check_attach ( fraction, master_instruction );
    check_attach ( fraction_dispatcher, digit_map );
    check_attach ( fraction_dispatcher, 1,
    	           fraction_instruction );

    uns32 whitespace_dispatcher =
        LEX::create_dispatcher ( 3, 10 );
    check_attach ( whitespace, whitespace_dispatcher );
    check_attach ( whitespace, master_instruction );
    check_attach ( whitespace_dispatcher, tmap );
    check_attach ( whitespace_dispatcher, white,
                   whitespace_instruction );

    uns32 operation_dispatcher =
        LEX::create_dispatcher ( 9, 10 );
    check_attach ( oper, operation_dispatcher );
    check_attach ( oper, master_instruction );
    uns32 plus_map =
        LEX::create_type_map ( '+', '+', 1 );
    uns32 minus_map =
        LEX::create_type_map ( '-', '-', 2 );
    uns32 divide_map =
        LEX::create_type_map ( '/', '/', 3 );
    uns32 times_map =
        LEX::create_type_map ( '*', '*', 3 );
    uns32 master_accept =
        LEX::create_instruction
	    ( ACCEPT+GOTO, master );
    uns32 plus_dispatcher =
        LEX::create_dispatcher ( 9, 10 );
    uns32 minus_dispatcher =
        LEX::create_dispatcher ( 9, 10 );
    check_attach ( operation_dispatcher, plus_map );
    check_attach ( operation_dispatcher, minus_map );
    check_attach ( operation_dispatcher, divide_map );
    check_attach ( operation_dispatcher, times_map );
    check_attach ( operation_dispatcher, 1,
                   plus_dispatcher );
    check_attach ( operation_dispatcher, 1,
                   master_accept );
    check_attach ( operation_dispatcher, 2,
                   minus_dispatcher );
    check_attach ( operation_dispatcher, 2,
                   master_accept );
    check_attach ( operation_dispatcher, 3,
                   master_accept );
    check_attach ( plus_dispatcher, plus_map );
    check_attach ( plus_dispatcher, 1,
                   master_accept );
    check_attach ( minus_dispatcher, minus_map );
    check_attach ( minus_dispatcher, 2,
                   master_accept );

    cout << "Cooked Program 2:" << endl << endl;
    LEX::print_program ( cout, true );
}

static void erroneous_atom
    ( uns32 first, uns32 last, uns32 type )
{
    char buffer[1000];
    LEX::spinput ( buffer, first, last );
    cout << "Erroneous Atom, Kind = "
         << LEX::pmode ( type )
         << " Atom = " << buffer << endl;
}

void test_program ( uns32 * input, uns32 length )
{
    LEX::erroneous_atom = ::erroneous_atom;

    cout << endl
         << "Testing Lexical Scan of:" << endl;
    for ( uns32 i = 0; i < length; ++ i )
        cout << LEX::pchar ( input[i] );
    cout << endl << endl;

    set_lex_input ( input, length );
    LEX::init_scan();
    char buffer[1000];
    while ( true )
    {
	uns32 first, last;
        uns32 type = LEX::scan ( first, last );
	cout << "Scan Returned " << LEX::pmode ( type );
	if ( type == LEX::END_OF_FILE )
	{
	    cout << endl;
	    break;
	}
	else if ( type == LEX::SCAN_ERROR )
	{
	    cout << endl << LEX::error_message << endl;
	    break;
	}
	else
	    cout << " Input Buffer: ";

	LEX::spinput ( buffer, first, last );
	cout << buffer << " Translation: ";
	for ( uns32 i = 0;
	      i < LEX::translation_buffer.length; ++ i )
	    cout << LEX::pchar
	              ( LEX::translation_buffer[i] );
	cout << endl;
    }
}

int main ( int argc )
{
    create_program_1();
    create_program_2();
    uns32 input1[14] = {
        ' ', 'a', 'b',
	' ', '3', '.', '4',
	' ', 'x', '+', 'y', '+', '+', 'z' };
    test_program ( input1, 14 );
    uns32 input2[37] = {
        '*', 'a', '*', '*',
        'b', '+', '+', '+',
	'c', '(', '+', 'd', 0177, 'e',
	'%', 'f', '0',
	'-', '-', '1', '.', '2', '.', '3',
	0, 1,
	'A', '\\', '1', '0', '2',
	'C', '\\', '1', '0', '0',
	'D' };
    test_program ( input2, 37 );
    LEX::scan_trace_out= & cout;
    test_program ( input2, 37 );
}
