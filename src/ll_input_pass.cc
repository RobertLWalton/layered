// Layers Language Standard Input Parser
//
// File:	ll_input_pass.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu Jan 13 09:14:35 EST 2011
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
# define PARSTD ll::parser::standard
using namespace PAR;

// Input Parser Data
// ----- ------ ----

struct input_pass_struct : public PAR::pass_struct
{
    LEX::scanner_ptr scanner;
};
typedef min::packed_struct_updptr<input_pass_struct>
    input_pass_ptr;

static min::uns32 input_pass_stub_disp[] =
{
    min::DISP ( & PAR::pass_struct::in ),
    min::DISP ( & PAR::pass_struct::first ),
    min::DISP ( & input_pass_struct::scanner ),
    min::DISP_END
};

static min::packed_struct<input_pass_struct>
    input_pass_type
    ( "ll::parser::standard::input_pass_type",
      NULL, ::input_pass_stub_disp,
      min::packed_struct<PAR::pass_struct>::id );

static void erroneous_atom
	( min::uns32 first, min::uns32 last,
	  min::uns32 type, LEX::scanner_ptr scanner );
static min::uns32 input_pass_get
	( PAR::pass_ptr out, PAR::pass_ptr in );
PAR::pass_ptr PARSTD::create_input_pass
    ( LEX::scanner_ptr scanner,
      std::ostream & err,
      std::ostream & trace )
{
    input_pass_ptr pass =
        input_pass_type.new_stub();
    pass->scanner = scanner;
    pass->err = & err;
    pass->trace = & trace;
    pass->get = ::input_pass_get;
    scanner->erroneous_atom = ::erroneous_atom;
    scanner->erroneous_atom_data = pass;
    return (pass_ptr) pass;
}

// Input Parser
// ----- ------

static void erroneous_atom
	( min::uns32 first, min::uns32 last,
	  min::uns32 type, LEX::scanner_ptr scanner )
{
    const char * message;
    switch ( type )
    {
    case LEXSTD::unrecognized_escape_character_t:
	message = "unrecognized escape character";
	break;
    case LEXSTD::unrecognized_escape_sequence_t:
	message = "unrecognized escape sequence";
	break;
    case LEXSTD::non_letter_escape_sequence_t:
	message = "non-letter escape sequence";
	break;
    default:
	message = "system error: bad erroneous"
		  " atom type";
	break;
    }
    input_pass_ptr pass =
        (input_pass_ptr) scanner->erroneous_atom_data;
    std::ostream * err = pass->trace;
    if ( err == NULL ) err = pass->err;
    if ( err != NULL )
    {
	uns32 column = LEX::print_lexeme_lines
	    ( * err, scanner, first, last );
	LEX::print_message
	    ( * err, scanner, column, message );
    }
}

static min::uns32 input_pass_get
	( PAR::pass_ptr out, PAR::pass_ptr in )
{
    input_pass_ptr pass = (input_pass_ptr) in;
    if ( pass->eop ) return 0;

    LEX::scanner_ptr scanner = pass->scanner;
    std::ostream * trace = pass->trace;

    min::uns32 first, last, count = 0;
    while ( true )
    {
        min::uns32 type =
	    LEX::scan ( first, last, scanner );
	const char * message = NULL;
	bool skip = true;
	switch ( type )
	{
	case LEX::SCAN_ERROR:
	{
	    std::ostream * err = pass->trace;
	    if ( err == NULL ) err = pass->err;
	    if ( err != NULL )
	        (* err) << scanner->error_message
		        << std::endl;
	    pass->eop = true;
	    return count;
	}
	case LEXSTD::comment_t:
	case LEXSTD::horizontal_space_t:
	    continue;
	case LEXSTD::bad_end_of_line_t:
	    message = "bad end of line";
	    type = LEXSTD::line_break_t;
	    skip = false;
	    break;
	case LEXSTD::bad_end_of_file_t:
	    message = "bad end of file";
	    type = LEXSTD::end_of_file_t;
	    skip = false;
	    break;
	case LEXSTD::unrecognized_character_t:
	    message = "unrecognized character";
	    break;
	case LEXSTD::unrecognized_escape_character_t:
	    message = "unrecognized escape character";
	    break;
        case LEXSTD::unrecognized_escape_sequence_t:
	    message = "unrecognized escape sequence";
	    break;
	}
	if ( message != NULL )
	{
	    std::ostream * err = pass->trace;
	    if ( err == NULL ) err = pass->err;
	    if ( err != NULL )
	    {
		uns32 column = LEX::print_lexeme_lines
		    ( * err, scanner, first, last );
		LEX::print_message
		    ( * err, scanner, column, message );
	    }
	    if ( skip ) continue;
	}

	PAR::token_ptr token = PAR::new_token( type );
	if ( first < scanner->input_buffer->length )
	    token->begin = (LEX::position)
	        scanner->input_buffer[first];
	else
	    token->begin = scanner->next_position;

	if ( last + 1 < scanner->input_buffer->length )
	    token->end = (LEX::position)
	        scanner->input_buffer[last+1];
	else
	    token->end = scanner->next_position;

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
	    token->type = PAR::SYMBOL;
	    token->value = min::new_str_gen ( buffer );
	    break;
	}
	case LEXSTD::natural_number_t:
	{
	    int length =
	        scanner->translation_buffer->length;
	    assert ( length > 0 );
	    if ( length <= 9
	         &&
		 ( length == 1
		   ||
		   scanner->translation_buffer[0] != '0'
		 ) )
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
		    token->type =
		        PAR::NATURAL_NUMBER;
		    token->value =
		        min::new_num_gen ( (int) v );
		    break;
		}
		// else fall through if >= (1<28).
	    }
	    // else fall through if number has a high
	    // order `0' digit or length indicates
	    // number must be >= (10**9) > (1<28).
	}
	case LEXSTD::quoted_string_t:
	case LEXSTD::number_t:
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
	case LEXSTD::line_break_t:
	    break;
	case LEXSTD::end_of_file_t:
	    pass->eop = true;
	    break;
	}

	PAR::put_at_end ( out->first, token );

	if ( trace != trace )
	{
	    uns32 column = LEX::print_item_lines
		( * trace, scanner,
		  token->begin, token->end );
	    if ( token->value == min::MISSING
		 &&
		 token->string == NULL_STUB )
	    {
		LEX::print_message
		    ( * trace, scanner, column,
		      LEXSTD::type_name[type] );
	    }
	    else
	    {
		char buffer
		    [ 80 + scanner->indent
		      +
			LEX::MAX_UNICODE_BYTES
		      * scanner->translation_buffer
		               ->length ];
		char * p = buffer;
		for ( uns32 i = 0;
		      i < scanner->indent; ++ i )
		    * p ++ = ' ';
		column = scanner->indent;
		column += sprintf
		    ( buffer + column, "%s: ",
		      LEXSTD::type_name[type] );
		p = buffer + column
		  + sptranslation
		        ( buffer + column, column,
		          LEX::ENFORCE_LINE_LENGTH,
			  scanner );
		* p = 0;
		* trace << buffer << std::endl;
	    }
	}

	if ( token->type == LEXSTD::end_of_file_t
	     ||
	     token->type == LEXSTD::line_break_t )
	    break;
    }
    return count;
}
