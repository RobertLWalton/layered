// Layers Language Standard Input Parser
//
// File:	ll_parser_input.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sat Jan 29 06:06:20 EST 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Standard Input Parser
//	Announce
//	Add Tokens

// Usage and Setup
// ----- --- -----

# include <ll_lexeme_standard.h>
# include <ll_parser.h>
# include <ll_parser_input.h>
# define LEX ll::lexeme
# define LEXSTD ll::lexeme::standard
# define PAR ll::parser
using namespace PAR;

// Standard Input Parser
// -------- ----- ------ ----

static min::uns32 input_add_tokens
	( PAR::parser_ptr parser,
	  PAR::input_ptr input );
static void erroneous_announce
	( min::uns32 first, min::uns32 last,
	  min::uns32 type, LEX::scanner_ptr scanner,
	  LEX::erroneous_ptr erroneous );

void PAR::init_standard_input ( parser_ptr parser )
{
    LEX::standard::create_standard_program();
    LEX::init_scanner ( LEX::default_scanner->program,
    			parser->scanner );

    if ( parser->input_file != NULL_STUB )
        parser->scanner->input_file =
	    parser->input_file;
    else
	parser->input_file =
	    parser->scanner->input_file;

    if ( parser->print != NULL_STUB )
        parser->scanner->print =
	    parser->print;
    else
	parser->print =
	    parser->scanner->print;

    parser->scanner->erroneous_atom = NULL_STUB;
    init_erroneous ( ::erroneous_announce,
                     parser->scanner->erroneous_atom );
    parser->input = NULL_STUB;
    init_input ( ::input_add_tokens, NULL,
                 parser->input );
}

// Announce
// --------

static void erroneous_announce
	( min::uns32 first, min::uns32 last,
	  min::uns32 type, LEX::scanner_ptr scanner,
	  LEX::erroneous_ptr erroneous )
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
    std::ostream * err = scanner->print->err;
    if ( err != NULL )
    {
	uns32 first_column, last_column;
	if ( LEX::print_lexeme_lines
	         ( * err, first_column, last_column,
		   scanner, first, last ) )
	    LEX::print_message
		( * err, first_column, last_column,
	          scanner->print, message );
    }
}

// Add Tokens
// --- ------
static min::uns32 input_add_tokens
	( PAR::parser_ptr parser, PAR::input_ptr input )
{
    if ( parser->eof ) return 0;

    LEX::scanner_ptr scanner = parser->scanner;
    std::ostream * trace =
        ( parser->trace & PAR::TRACE_INPUT ) ?
	parser->print->trace : NULL;

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
	    if ( parser->print->err != NULL )
	        (* parser->print->err)
		    << scanner->error_message
		    << std::endl;
	    parser->eof = true;
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
	    if ( parser->print->err != NULL )
	    {
		uns32 first_column, last_column;
		if ( LEX::print_lexeme_lines
			 ( * parser->print->err,
			   first_column, last_column,
			   scanner, first, last ) )
		    LEX::print_message
			( * parser->print->err,
			  first_column, last_column,
			  parser->print, message );
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
	    for ( min::uns32 i = 0; i < length; ++ i )
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
	    parser->eof = true;
	    break;
	}

	PAR::put_at_end ( parser->first, token );
	++ count;

	uns32 first_column, last_column;
	if ( trace != NULL
	     &&
	     LEX::print_item_lines
		( * trace, first_column, last_column,
		  parser->input_file,
		  parser->print->mode,
		  token->begin, token->end ) )
        {
	    if ( token->value == min::MISSING
		 &&
		 token->string == NULL_STUB )
	    {
		LEX::print_message
		    ( * trace,
		      first_column, last_column,
		      parser->print,
		      LEXSTD::type_name[type] );
	    }
	    else
	    {
		char buffer
		    [ 80 + parser->print->indent
		      +
			LEX::MAX_UNICODE_BYTES
		      * scanner->translation_buffer
		               ->length ];
		char * p = buffer;
		for ( uns32 i = 0;
		      i < parser->print->indent; ++ i )
		    * p ++ = ' ';
		uns32 column = parser->print->indent;
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

	if ( ( token->type == LEXSTD::end_of_file_t
	       ||
	       token->type == LEXSTD::line_break_t )
	     &&
	     count > 0 )
	    break;
    }
    return count;
}
