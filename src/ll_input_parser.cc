// Layers Language Standard Input Parser
//
// File:	ll_input_parser.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Jan  8 10:10:15 EST 2011
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

struct input_parser : public PAR::pass_struct
{
    LEX::scanner_ptr scanner;
    std::ostream * err;  // Error message stream.
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

static void erroneous_atom
	( min::uns32 first, min::uns32 last,
	  min::uns32 type, LEX::scanner_ptr scanner );
static min::uns32 input_parser_get
	( PAR::pass_ptr out, PAR::pass_ptr in );
PAR::pass_ptr PARSTD::create_input_pass
    ( LEX::scanner_ptr scanner,
      std::ostream & err )
{
    input_parser_ptr pass =
        input_parser_type.new_stub();
    pass->scanner = scanner;
    pass->err = & err;
    pass->get = ::input_parser_get;
    scanner->erroneous_atom = ::erroneous_atom;
    scanner->erroneous_atom_data = pass;
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
    input_parser_ptr pass =
        (input_parser_ptr) scanner->erroneous_atom_data;
    uns32 column = LEX::print_lexeme_lines
	( * pass->err, scanner, first, last );
    LEX::print_message
        ( * pass->err, scanner, column, message );
}

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
	const char * message = NULL;
	bool skip = true;
	switch ( type )
	{
	case LEX::SCAN_ERROR:
	{
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
	    uns32 column = LEX::print_lexeme_lines
		( * pass->err, scanner, first, last );
	    LEX::print_message
		( * pass->err, scanner,
		  column, message );
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
	case LEXSTD::end_of_file_t:
	    break;
	}
    }
    return count;
}
