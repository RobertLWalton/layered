// Layers Language Lexical Analyzer Test Runtime
//
// File:	ll_lexeme_test_runtime.cc
// Author:	Bob Walton (walton@deas.harvard.edu)
// Date:	Mon Apr 12 14:04:46 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/04/13 01:06:11 $
//   $RCSfile: ll_lexeme_test_runtime.cc,v $
//   $Revision: 1.4 $

// Table of Contents
//
//	Usage and Setup
//	Test Runtime

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

// Test Runtime
// ---- -------

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

uns32 * lex_input = NULL;
uns32   lex_input_length = 0;

void set_lex_input ( uns32 * input, uns32 length )
{
    lex_input = input;
    lex_input_length = length;
}
uns32 LEX::read_input ( void )
{
    if ( lex_input == NULL ) return 0;

    uns32 p = LEX::input_buffer.allocate
    		( lex_input_length );
    memcpy ( & LEX::input_buffer[p], lex_input,
    	     lex_input_length * sizeof ( uns32 ) );
    lex_input = NULL;
    return 1;
}
