// Layers Language Standard Input Parser
//
// File:	ll_input_parser.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Dec 27 07:07:26 EST 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Input Parser Data
//	Input Parser

// Usage and Setup
// ----- --- -----

# include <ll_lexeme_standard.h>
# include <ll_parser_pass.h>
# include <ll_parser_standard.h>
# define LEX ll::lexeme
# define LEXSTD ll::lexeme::standard
# define PAR ll::parser

// Input Parser Data
// ----- ------ ----

struct input_parser : public PAR::pass_struct
{
    LEX::scanner_ptr scanner;
};
typedef min::packed_struct_updptr<input_parser>
    input_parser_ptr;

static min::uns32 input_parser_stub_disp[] =
{
    min::DISP ( & PAR::pass_struct::first ),
    min::DISP ( & input_parser::scanner ),
    min::DISP_END
};

min::packed_struct<input_parser> input_parser_type
    ( "input_parser",
      NULL, ::input_parser_stub_disp );

static min::uns32 input_parser_get
	( PAR::pass_ptr out, PAR::pass_ptr in );
PAR::pass_ptr PAR::standard::create_input_pass
    ( LEX::scanner_ptr scanner )
{
    input_parser_ptr pass =
        input_parser_type.new_stub();
    pass->scanner = scanner;
    pass->get = ::input_parser_get;
}

// Input Parser
// ----- ------

static min::uns32 input_parser_get
	( PAR::pass_ptr out, PAR::pass_ptr in )
{
    min::uns32 first, last, count = 0;
    input_parser_ptr pass = (input_parser_ptr) in;
    LEX::scanner_ptr scanner = pass->scanner;
    while ( true )
    {
        min::uns32 type =
	    LEX::scan ( first, last, scanner );
	if ( type == LEX::SCAN_ERROR )
	{
	}

	PAR::token_ptr token = PAR::new_token( type );
	LEX::inchar * icp;

	if ( first < scanner->input_buffer->length )
	    icp = & scanner->input_buffer[first];
	else
	    icp = & scanner->next_position;
	token->begin_line   = icp->line;
	token->begin_index  = icp->index;
	token->begin_column = icp->column;

	if ( last + 1 < scanner->input_buffer->length )
	    icp = & scanner->input_buffer[last+1];
	else
	    icp = & scanner->next_position;
	token->end_line   = icp->line;
	token->end_index  = icp->index;
	token->end_column = icp->column;

	switch ( type )
	{
	case LEXSTD::word_t:
	case LEXSTD::mark_t:
	case LEXSTD::separator_t:
	{
	    int length =
	        scanner->translation_buffer->length;
	    assert ( length > 0 );
	    char buffer[8*length+1];
	    char * p = buffer;
	    for ( min::uns32 i = first;
	          i <= last; ++ i )
	        p += spchar
		    ( p, scanner->translation_buffer[i],
		         LEX::UTF8 );
	    token->kind = PAR::SYMBOL;
	    token->value = min::new_str_gen ( buffer );
	    break;
	}
	case LEXSTD::natural_number_t:
	{
	    int length =
	        scanner->translation_buffer->length;
	    assert ( length > 0 );
	    if ( length <= 9 )
	    {
		min::uns32 v = 0;
		for ( min::uns32 i = 0;
		      i < length; ++ i )
		{
		    v *= 10;
		    v += scanner->translation_buffer[i]
		       - '0';
		}
		if ( v < (1<<28) )
		{
		    token->kind =
		        PAR::NATURAL_NUMBER;
		    token->value =
		        min::new_num_gen ( (int) v );
		    break;
		}
	    }
	}
	case LEXSTD::quoted_string_t:
	case LEXSTD::number_t:
	case LEXSTD::line_break_t:
	case LEXSTD::end_of_file_t:
	{
	    int length =
	        scanner->translation_buffer->length;
	    min::uns32 * p =
	        length == 0 ? NULL :
		& scanner->translation_buffer[0];
	    token->string =
	        PAR::new_string ( length, p );
	    break;
	}
	}
    }
    return count;
}
