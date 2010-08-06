// Layers Language Lexical Analyzer MIN Runtime
//
// File:	ll_lexeme_min.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri Aug  6 00:40:52 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.
//
// RCS Info (may not be true date or author):
//
//   $Author: walton $
//   $Date: 2010/04/06 15:46:13 $
//   $RCSfile: ll_lexeme_min.cc,v $
//   $Revision: 1.1 $

// Table of Contents
//
//	Usage and Setup
//	Buffer Implementation

// Usage and Setup
// ----- --- -----

# include <min.h>
# include <ll_lexeme.h>

# define MUP min::unprotected
# define LEX ll::lexeme

// Buffer Implementation
// ------ --------------

struct pvheader
{
    const min::uns32 type;
    const min::uns32 length;
    const min::uns32 max_length;
};

template < typename T >
struct ext_buffer : public LEX::buffer<T>
{

    typedef typename
        min::packed_vec<pvheader,T> pvt;

    pvt pvtype;

    min::packed_vec<pvheader,T>::insertable_pointer pvp;
        // Invariant:
	//   this->length <= this->max_length
	//                == pvp->length
	//                == pvp->max_length

    min::static_gen<1> holder;
    min::gen & pv;

    ext_buffer ( void ) :
	pvtype ( "LL Lexeme Packed Buffer" ),
        pv ( holder[0] )
    {
	this->base = NULL;
	this->header_size = 0;
	this->length = 0;
	this->max_length = 0;
	this->length_increment = 1000;
    }
    ~ ext_buffer ( void )
    {
	if ( pv != min::MISSING )
	    min::deallocate ( MUP::stub_of ( pv ) );
    }
    void resize ( LEX::uns32 new_max_length );
};

template < typename T >
void ext_buffer<T>::resize ( LEX::uns32 new_max_length )
{
    if ( this->max_length == new_max_length ) return;
    else if ( this->max_length == 0 )
    {
        pv = pvtype.new_gen ( new_max_length );
	min::initialize ( pvp, pv );
	this->base = (LEX::uns8 **)
	    MUP::pointer_ref_of
	        ( MUP::stub_of ( pv ) );
    }
    else if ( new_max_length == 0 )
    {
        min::deinitialize ( pvp );
	min::deallocate ( MUP::stub_of ( pv ) );
    }
    else
	min::resize ( pvp, new_max_length );

    this->max_length = new_max_length;
    if ( this->length > this->max_length )
	this->length = this->max_length;
    else
        min::push ( pvp, this->max_length
			 -
			 this->length,
			 (T *) NULL );
}
template void ext_buffer<LEX::uns32>::resize
	( LEX::uns32 new_max_length );
template void ext_buffer<LEX::inchar>::resize
	( LEX::uns32 new_max_length );

ext_buffer<LEX::uns32> ext_program;
ext_buffer<LEX::inchar> ext_input_buffer;
ext_buffer<LEX::uns32> ext_translation_buffer;

LEX::buffer<LEX::uns32> & LEX::program =
    ext_program;
LEX::buffer<LEX::inchar> & LEX::input_buffer =
    ext_input_buffer;
LEX::buffer<LEX::uns32> & LEX::translation_buffer =
    ext_translation_buffer;
