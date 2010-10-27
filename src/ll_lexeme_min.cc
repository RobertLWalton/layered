// Layers Language Lexical Analyzer MIN Runtime
//
// File:	ll_lexeme_min.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Oct 27 08:24:29 EDT 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Buffer Implementation

// Usage and Setup
// ----- --- -----

# include <min.h>
# include <ll_lexeme.h>
# include <ll_lexeme_ndl_data.h>

# define MUP min::unprotected
# define LEX ll::lexeme
# define LEXNDLDATA ll::lexeme::ndl::data

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

    min::packed_vec_insptr<pvheader,T> pvp;
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
	this->header_size = pvtype.header_size;
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
	pvp = pv;
	this->base = (LEX::uns8 **)
	    & MUP::ptr_ref_of ( MUP::stub_of ( pv ) );
    }
    else if ( new_max_length == 0 )
    {
	pvp = min::NULL_STUB;
	min::deallocate ( MUP::stub_of ( pv ) );
    }
    else
	min::resize ( pvp, new_max_length );

    this->max_length = new_max_length;
    if ( this->length > this->max_length )
	this->length = this->max_length;
    else if ( this->length < this->max_length )
        min::push ( pvp, this->max_length
			 -
			 this->length,
			 (T *) NULL );
}
template void ext_buffer<LEX::uns32>::resize
	( LEX::uns32 new_max_length );
template void ext_buffer<LEX::inchar>::resize
	( LEX::uns32 new_max_length );

static ext_buffer<LEX::uns32> ext_program;
static ext_buffer<LEX::inchar> ext_input_buffer;
static ext_buffer<LEX::uns32> ext_translation_buffer;

LEX::buffer<LEX::uns32> & LEX::program =
    ext_program;
LEX::buffer<LEX::inchar> & LEX::input_buffer =
    ext_input_buffer;
LEX::buffer<LEX::uns32> & LEX::translation_buffer =
    ext_translation_buffer;

static ext_buffer<LEX::uns32> ext_uns32_stack;
static ext_buffer<LEXNDLDATA::dispatcher>
    ext_dispatchers;
static ext_buffer<LEXNDLDATA::instruction>
    ext_instructions;

LEX::buffer<LEX::uns32> & LEXNDLDATA::uns32_stack =
    ext_uns32_stack;
LEX::buffer<LEXNDLDATA::dispatcher> &
    LEXNDLDATA::dispatchers = ext_dispatchers;
LEX::buffer<LEXNDLDATA::instruction>
    & LEXNDLDATA::instructions = ext_instructions;
