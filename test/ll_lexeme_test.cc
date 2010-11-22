// Layers Language Lexical Analyzer Test
//
// File:	ll_lexeme_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Nov 22 06:34:12 EST 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

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
using LEX::inchar;

using LEX::MASTER;
using LEX::ATOM;

using LEX::ACCEPT;
using LEX::MATCH;
using LEX::KEEP;
using LEX::TRANSLATE_TO;
using LEX::TRANSLATE_HEX;
using LEX::TRANSLATE_OCT;
using LEX::REQUIRE;
using LEX::ELSE;
using LEX::ERRONEOUS_ATOM;
using LEX::OUTPUT;
using LEX::GOTO;
using LEX::CALL;
using LEX::RETURN;
using LEX::FAIL;

enum {
    WHITESPACE,
    ERROR,
    END_OF_FILE,
    SYMBOL,
    NUMBER,
    OPERATOR,
    SEPARATOR,
    MAX_TYPE = SEPARATOR };
static const char * const type_name[MAX_TYPE+1] = {
   "WHITESPACE",
   "ERROR",
   "END_OF_FILE",
   "SYMBOL",
   "NUMBER",
   "OPERATOR",
   "SEPARATOR" };


// External Runtime
// -------- -------

# ifndef  LL_NO_DATA_DEFINITIONS

template < typename T >
struct ext_buffer : public LEX::buffer<T>
{
    T * vector;

    ext_buffer ( void )
    {
	vector = NULL;
	this->base = (uns8 **) & vector;
	this->header_size = 0;
	this->length = 0;
	this->max_length = 0;
	this->length_increment = 1000;
    }
    ~ ext_buffer ( void )
    {
        delete [] vector;
    }
    void resize ( uns32 new_max_length );
};

template < typename T >
void ext_buffer<T>::resize ( uns32 new_max_length )
{
    if ( this->length > new_max_length )
        this->length = new_max_length;

    T * new_vector = NULL;
    if ( new_max_length > 0 )
    {
        new_vector = new T[new_max_length];
	memcpy ( new_vector, vector,
		 this->length * sizeof ( T ) );
    }
    delete [] vector;
    vector = new_vector;
    this->max_length = new_max_length;
}
template void ext_buffer<uns32>::resize
	( uns32 new_max_length );
template void ext_buffer<inchar>::resize
	( uns32 new_max_length );

ext_buffer<uns32> ext_program;
ext_buffer<inchar> ext_input_buffer;
ext_buffer<uns32> ext_translation_buffer;

LEX::buffer<uns32> & LEX::program =
    ext_program;
LEX::buffer<inchar> & LEX::input_buffer =
    ext_input_buffer;
LEX::buffer<uns32> & LEX::translation_buffer =
    ext_translation_buffer;

# endif  // LL_NO_DATA_DEFINITIONS

static uns32 * lex_input = NULL;
static uns32   lex_input_length = 0;
static uns32   lex_line = 0;
static uns32   lex_index = 0;
static uns32   lex_column = 0;

void set_lex_input ( uns32 * input, uns32 length )
{
    lex_input = input;
    lex_input_length = length;
    lex_line = 0;
    lex_index = 0;
    lex_column = 0;
}
static uns32 read_input ( void )
{
    if ( lex_input == NULL ) return 0;

    uns32 p = LEX::input_buffer.allocate
    		( lex_input_length );
    for ( uns32 i = 0; i < lex_input_length; ++ i )
    {
        LEX::input_buffer[p+i].character = lex_input[i];
        LEX::input_buffer[p+i].line = lex_line;
        LEX::input_buffer[p+i].index = lex_index;
        LEX::input_buffer[p+i].column = lex_column;

	if ( lex_input[i] == '\n' ) ++ lex_line;

	if ( lex_input[i] == '\n' ) lex_column = 0;
	else if ( lex_input[i] == '\f' ) lex_column = 0;
	else if ( lex_input[i] == '\v' ) lex_column = 0;
	else if ( lex_input[i] == '\t' )
	    lex_column += 8 - ( lex_column % 8 ); 
	else
	    ++ lex_column;
	++ lex_index;
    }
    lex_input = NULL;
    return 1;
}

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

    LEX::create_program();
    uns32 master = LEX::create_table ( MASTER );

    uns32 atable1 = LEX::create_table ( SYMBOL );
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
	    ( KEEP(1)+TRANSLATE_TO(3)+GOTO,
	      translation, 0, 0, 0, 0, 0, atable1 );
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
    LEX::create_program();
    uns32 master = LEX::create_table ( MASTER );

    // Create atom table for \ooo octal
    // character representations.
    //
    uns32 oct_atom =
        LEX::create_table ( ATOM );

    uns32 escape_tmap =
        LEX::create_type_map ( '\\', '\\', 1 );
    uns32 oct_tmap =
        LEX::create_type_map ( '0', '7', 1 );

    uns32 escape_dispatcher =
        LEX::create_dispatcher ( 3, 2 );
    check_attach ( escape_dispatcher, escape_tmap );
    uns32 oct_dispatcher1 =
        LEX::create_dispatcher ( 3, 2 );
    check_attach ( oct_dispatcher1, oct_tmap );
    uns32 oct_dispatcher2 =
        LEX::create_dispatcher ( 3, 2 );
    check_attach ( oct_dispatcher2, oct_tmap );
    uns32 oct_dispatcher3 =
        LEX::create_dispatcher ( 3, 2 );
    check_attach ( oct_dispatcher3, oct_tmap );

    check_attach ( oct_atom,
                   escape_dispatcher );
    check_attach ( escape_dispatcher,
                   1, oct_dispatcher1 );
    check_attach ( oct_dispatcher1,
                   1, oct_dispatcher2 );
    check_attach ( oct_dispatcher2,
                   1, oct_dispatcher3 );

    uns32 fail_instruction =
        LEX::create_instruction ( FAIL );
    uns32 translate_oct_instruction =
        LEX::create_instruction
	    ( TRANSLATE_OCT(1,0)+ELSE,
	      NULL, 0, 0, fail_instruction );
    check_attach ( oct_dispatcher3,
                   1, translate_oct_instruction );
    check_attach ( oct_atom,
                   fail_instruction );

    uns32 whitespace = LEX::create_table ( WHITESPACE );
    uns32 symbol = LEX::create_table ( SYMBOL );
    uns32 number = LEX::create_table ( NUMBER );
    uns32 oper = LEX::create_table ( OPERATOR );

    const uns8 white = 1;
    const uns8 letter = 2;
    const uns8 digit = 3;
    const uns8 point = 4;
    const uns8 sep = 5;
    const uns8 op = 6;
    const uns8 escape = 7;
    const uns8 err_atom = 8;

    uns32 end_of_file_instruction =
        LEX::create_instruction
	    ( ACCEPT+OUTPUT,
	      NULL, 0, 0, 0, 0, END_OF_FILE );
    check_attach ( master, end_of_file_instruction );

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
	    ( KEEP(0)+CALL,
	      NULL, 0, 0, 0, 0, 0, 0, symbol );
    uns32 number_instruction =
        LEX::create_instruction
	    ( KEEP(0)+GOTO,
	      NULL, 0, 0, 0, 0, 0, number );
    uns32 whitespace_instruction =
        LEX::create_instruction
	    ( ACCEPT+GOTO,
	      NULL, 0, 0, 0, 0, 0, whitespace );
    uns32 operator_instruction =
        LEX::create_instruction
	    ( KEEP(0)+GOTO, NULL, 0, 0, 0, 0, 0, oper );
    uns32 separator_instruction =
        LEX::create_instruction
	    ( ACCEPT+OUTPUT,
	      NULL, 0, 0, 0, 0, SEPARATOR );
    uns32 error_instruction =
        LEX::create_instruction
	    ( ACCEPT+OUTPUT, NULL, 0, 0, 0, 0, ERROR );
    uns32 err_atom_instruction =
        LEX::create_instruction
	    ( ERRONEOUS_ATOM+TRANSLATE_TO(0),
	      NULL, 0, 0, 0, 100 );
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
	    ( KEEP(0)+GOTO,
	      NULL, 0, 0, 0, 0, 0, master );
    uns32 return_instruction =
        LEX::create_instruction
	    ( KEEP(0)+RETURN );
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
    uns32 escape_instruction =
        LEX::create_instruction
	    ( MATCH+REQUIRE+ELSE,
	      NULL, oct_atom,
	            letter_dispatcher,
	            err_atom_instruction );
    check_attach ( symbol_dispatcher, escape,
                   escape_instruction );

    uns32 fraction = LEX::create_table ( NUMBER );
    uns32 fraction_instruction =
        LEX::create_instruction
	    ( ACCEPT+GOTO,
	      NULL, 0, 0, 0, 0, 0, fraction );

    uns32 digit_instruction =
        LEX::create_instruction
	    ( ACCEPT+GOTO,
	      NULL, 0, 0, 0, 0, 0, number );
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
	    ( ACCEPT+GOTO,
	      NULL, 0, 0, 0, 0, 0, master );
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
    char * p = buffer;
    p += sprintf ( p, "Erroneous Atom:" );
    unsigned column = p - buffer;
    p += LEX::sperroneous_atom
	    ( p, first, last, type,
	      column, true );
    cout << buffer << endl;
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
	char * p = buffer;
	p += sprintf ( p, "Scan Returned:" );
	unsigned column = p - buffer;

	if ( type == LEX::SCAN_ERROR )
	{
	    cout << buffer << " Scan Error:" << endl
	         << LEX::error_message << endl;
	    break;
	}
	else
	    p += LEX::splexeme ( p, first, last, type,
	                         column, true );
	cout << buffer << endl;

	if ( type == END_OF_FILE ) break;
    }
}

int main ( int argc )
{
    LEX::read_input = & ::read_input;
    LEX::type_name = ::type_name;
    LEX::max_type = MAX_TYPE;

    create_program_1();
    create_program_2();
    uns32 input1[14] = {
        ' ', 'a', 'b',
	' ', '3', '.', '4',
	' ', 'x', '+', 'y', '+', '+', 'z' };
    test_program ( input1, 14 );
    uns32 input2[41] = {
        '*', 'a', '*', '*',
        'b', '+', '+', '+',
	'c', '(', '+', 'd', 0177, 'e',
	'%', 'f', '0',
	'-', '-', '1', '.', '2', '.', '3',
	0, 1,
	'A', '\\', '1', '0', '2',
	'C', '\\', '1', '0', '0',
	'D', '\\', '1', '0',
	'E' };
    test_program ( input2, 41 );
    LEX::scan_trace_out= & cout;
    test_program ( input2, 41 );
}
