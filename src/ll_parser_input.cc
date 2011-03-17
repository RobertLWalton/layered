// Layers Language Standard Input Parser
//
// File:	ll_parser_input.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu Mar 17 13:56:23 EDT 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Standard Input Parser
//	Erroneous Atom Announce
//	Add Tokens

// Usage and Setup
// ----- --- -----

# include <ll_lexeme_standard.h>
# include <ll_parser.h>
# include <ll_parser_input.h>
# define MUP min::unprotected
# define LEX ll::lexeme
# define LEXSTD ll::lexeme::standard
# define PAR ll::parser

// Standard Input Parser
// -------- ----- ------ ----

min::locatable_var<PAR::input>
    PAR::default_standard_input;
min::locatable_var<LEX::erroneous_atom>
    PAR::default_standard_erroneous_atom;

static min::uns32 input_add_tokens
	( PAR::parser parser,
	  PAR::input input );
static min::printer_format str_format;
static void input_init
	( PAR::parser parser,
	  PAR::input input )
{
    ::str_format = min::default_printer_format;
    ::str_format.str_prefix = "";
    ::str_format.str_postfix = "";
    LEX::init
        ( min::locatable ( parser, parser->scanner ) );
}
static void erroneous_atom_announce
	( min::uns32 first, min::uns32 last,
	  min::uns32 type, LEX::scanner scanner,
	  LEX::erroneous_atom erroneous_atom );

void PAR::init_standard_input
	( min::ptr<PAR::parser> parser )
{
    PAR::init ( PAR::default_standard_input,
                ::input_add_tokens,
		::input_init );
    LEX::init ( PAR::default_standard_erroneous_atom,
                ::erroneous_atom_announce );

    PAR::init ( parser );
    LEXSTD::init_standard_program();
    LEX::init_program
        ( min::locatable ( parser, parser->scanner ),
          LEXSTD::default_program );
    parser->input = PAR::default_standard_input;
    parser->scanner->erroneous_atom =
        PAR::default_standard_erroneous_atom;
}

// Erroneous Atom Announce
// --------- ---- --------

static void erroneous_atom_announce
	( min::uns32 first, min::uns32 next,
	  min::uns32 type, LEX::scanner scanner,
	  LEX::erroneous_atom erroneous_atom )
{
    const char * message;
    switch ( type )
    {
    case LEXSTD::unrecognized_character_t:
	message = "ERROR: unrecognized character; ";
	break;
    case LEXSTD::non_letter_escape_sequence_t:
	message = "ERROR: non-letter escape sequence; ";
	break;
    default:
	message = "ERROR: system error: bad erroneous"
		  " atom type; ";
	break;
    }

    scanner->printer
        << min::bom
	<< min::set_indent ( 7 )
	<< message
	<< LEX::pline_numbers
	        ( scanner, first, next )
	<< ":" << min::eom;
    LEX::print_item_lines
        ( scanner->printer, scanner, first, next );
}

// Add Tokens
// --- ------
static min::uns32 input_add_tokens
	( PAR::parser parser, PAR::input input )
{
    if ( parser->eof ) return 0;

    LEX::scanner scanner = parser->scanner;
    min::printer printer = parser->printer;
    LEX::input_buffer input_buffer =
        scanner->input_buffer;
    LEX::translation_buffer translation_buffer =
        scanner->translation_buffer;
    bool trace = ( parser->trace & PAR::TRACE_INPUT );

    min::uns32 first, next, count = 0;
    while ( true )
    {
        min::uns32 type =
	    LEX::scan ( first, next, scanner );

	const char * message = NULL;
	bool skip = true;
	switch ( type )
	{
	case LEX::SCAN_ERROR:
	{
	    printer << min::error_message;
	    parser->eof = true;
	    return count;
	}
	case LEXSTD::comment_t:
	case LEXSTD::horizontal_space_t:
	    continue;
	case LEXSTD::bad_end_of_line_t:
	    message = "ERROR: bad end of line; ";
	    type = LEXSTD::line_break_t;
	    skip = false;
	    break;
	case LEXSTD::bad_end_of_file_t:
	    message = "ERROR: bad end of file; ";
	    type = LEXSTD::end_of_file_t;
	    skip = false;
	    break;
	case LEXSTD::unrecognized_character_t:
	    message = "ERROR: unrecognized character; ";
	    break;
	}

	if ( message != NULL )
	{
	    scanner->printer
		<< min::bom
		<< min::set_indent ( 7 )
		<< message
		<< LEX::pline_numbers
		       ( scanner, first, next )
		<< ":" << min::eom;
	    LEX::print_item_lines
	        ( scanner->printer,
		  scanner, first, next );

	    if ( skip ) continue;
	}

	PAR::token token = PAR::new_token( type );
	token->begin = first < input_buffer->length ?
		       input_buffer[first] :
		       scanner->next_position;
	token->end  = next < input_buffer->length ?
		       input_buffer[next] :
		       scanner->next_position;

	switch ( type )
	{
	case LEXSTD::word_t:
	case LEXSTD::mark_t:
	case LEXSTD::separator_t:
	{
	    token->type = PAR::SYMBOL;
	    token->value = min::new_str_gen
	        ( translation_buffer.begin_ptr(),
		  translation_buffer->length );
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

	PAR::put_at_end
	    ( min::locatable ( parser, parser->first ),
	      token );
	++ count;

	if ( trace )
	{
	    scanner->printer
	        << LEXSTD::type_name[type]
		<< ": ";
	    if ( token->value != min::MISSING() )
	        scanner->printer
		    << min::push_parameters
		    << min::graphic
		    << min::pgen ( token->value,
		                   & ::str_format )
		    << min::pop_parameters
		    << ": ";
	    else if ( token->string != min::NULL_STUB )
	        scanner->printer
		    << min::push_parameters
		    << min::graphic
		    << min::punicode
		            ( token->string->length,
			      token->string.begin_ptr()
			    )
		    << min::pop_parameters
		    << ": ";
	    scanner->printer
		<< LEX::pline_numbers
		        ( scanner, first, next )
		<< ":" << min::eol;
	    LEX::print_item_lines
		( scanner->printer,
		  scanner, first, next );
	}

	if ( token->type == LEXSTD::end_of_file_t
	     ||
	     token->type == LEXSTD::line_break_t )
	    break;
    }
    return count;
}
