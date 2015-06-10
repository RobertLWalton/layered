// Layered Languages Standard Input Parser
//
// File:	ll_parser_standard_input.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Jun 10 03:54:59 EDT 2015
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Lexical Type Support
//	Standard Input Parser
//	Erroneous Atom Announce
//	Add Tokens

// Usage and Setup
// ----- --- -----

# include <ll_lexeme_standard.h>
# include <ll_parser.h>
# include <ll_parser_standard.h>
# define MUP min::unprotected
# define LEX ll::lexeme
# define LEXSTD ll::lexeme::standard
# define PAR ll::parser
# define PARSTD ll::parser::standard

// Standard Input Parser
// -------- ----- ------

min::locatable_var<PAR::input>
    PARSTD::input;
min::locatable_var<LEX::erroneous_atom>
    PARSTD::erroneous_atom;

static min::uns32 input_add_tokens
	( PAR::parser parser,
	  PAR::input input );
static void input_init
	( PAR::parser parser,
	  PAR::input input )
{
    LEX::init ( PAR::scanner_ref(parser) );
}
static void erroneous_atom_announce
	( min::uns32 first, min::uns32 last,
	  min::uns32 type, LEX::scanner scanner,
	  LEX::erroneous_atom erroneous_atom );

void PARSTD::init_input
	( min::ref<PAR::parser> parser )
{
    PAR::init ( PARSTD::input,
                ::input_add_tokens,
		::input_init );
    LEX::init ( PARSTD::erroneous_atom,
                ::erroneous_atom_announce );

    PAR::init ( parser );
    LEXSTD::init_standard_program();
    LEX::init_program
	( PAR::scanner_ref(parser),
          LEXSTD::default_program );
    input_ref(parser) = PARSTD::input;
    LEX::erroneous_atom_ref(parser->scanner) =
        PARSTD::erroneous_atom;
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
    case LEXSTD::misplaced_vertical_t:
	message = "ERROR: vertical control character"
	          " sequence with no line feed; ";
	break;
    case LEXSTD::illegal_control_t:
	message = "ERROR: illegal control"
	          " character sequence; ";
	break;
    case LEXSTD::unrecognized_character_t:
	message = "ERROR: unrecognized"
	          " character sequence; ";
	break;
    case LEXSTD::unrecognized_escape_t:
	message = "ERROR: unrecognized escape"
	          " sequence; ";
	break;
    case LEXSTD::misplaced_horizontal_t:
	message = "ERROR: horizontal characters other"
	          " than single space; ";
	break;
    default:
	message = "ERROR: system error: unrecognized"
	          " erroneous atom type; ";
	break;
    }

    scanner->printer
        << min::bom
	<< min::set_indent ( 7 )
	<< message
	<< LEX::pline_numbers
	        ( scanner, first, next )
	<< ":" << min::eom;
    LEX::print_phrase_lines
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
    bool trace = (   parser->trace_flags
                   & PAR::TRACE_PARSER_INPUT );

    min::uns32 first, next, count = 0;
    min::locatable_var<PAR::token> token;
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
	case LEXSTD::horizontal_space_t:
	    continue;

	case LEXSTD::premature_end_of_string_t:
	    message = "ERROR: premature end of"
	              " string; ";
	    break;
	case LEXSTD::premature_end_of_file_t:
	    message = "ERROR: premature end of file; ";
	    break;
	case LEXSTD::misplaced_vertical_t:
	    message = "ERROR: vertical control"
	              " character sequence with no line"
		      " feed; ";
	    break;
	case LEXSTD::illegal_control_t:
	    message = "ERROR: illegal control character"
	              " sequence; ";
	    break;
	case LEXSTD::unrecognized_character_t:
	    message = "ERROR: unrecognized character"
	              " sequence; ";
	    break;
	}

	if ( message != NULL )
	{
	    printer
		<< min::bom
		<< min::set_indent ( 7 )
		<< message
		<< LEX::pline_numbers
		       ( scanner, first, next )
		<< ":" << min::eom;
	    LEX::print_phrase_lines
	        ( printer, scanner, first, next );

	    if ( skip ) continue;
	}

	token = PAR::new_token( type );
	token->position.begin =
	    first < input_buffer->length ?
	    input_buffer[first] :
	    scanner->next_position;
	token->position.end =
	    next < input_buffer->length ?
	    input_buffer[next] :
	    scanner->next_position;
	token->indent =
	    first < input_buffer->length ?
	    (&input_buffer[first])->indent :
	    scanner->next_indent;

	switch ( type )
	{
	case LEXSTD::word_t:
	case LEXSTD::mark_t:
	case LEXSTD::separator_t:
	case LEXSTD::natural_t:
	{
	    value_ref(token) = min::new_str_gen
	        ( min::begin_ptr_of
		      ( translation_buffer ),
		  translation_buffer->length );
	    break;
	}
	case LEXSTD::quoted_string_t:
	case LEXSTD::numeric_t:
	{
	    int length = translation_buffer->length;
	    PAR::string_ref(token) =
	        PAR::new_string
		    ( length,
		      min::begin_ptr_of
			  ( translation_buffer ) );
	    break;
	}
	case LEXSTD::comment_t:
	case LEXSTD::line_break_t:
	    break;
	case LEXSTD::end_of_file_t:
	    parser->eof = true;
	    break;
	}

	PAR::put_at_end
	    ( PAR::first_ref(parser), token );
	++ count;

	if ( trace )
	{
	    printer
	        << LEXSTD::type_names[type]
		<< ": ";
	    if ( token->value != min::MISSING() )
	        printer
		    << min::save_print_format
		    << min::graphic_only
		    << min::pgen_quote ( token->value )
		    << min::restore_print_format
		    << ": ";
	    else if ( token->string != min::NULL_STUB )
	    {
	        printer << min::save_print_format
		        << min::graphic_only;
		min::print_unicode
		    ( printer,
		      token->string->length,
		      min::begin_ptr_of
			       ( token->string ),
		      min::quote_all_str_format );
		printer << min::restore_print_format
		        << ": ";
	    }
	    printer
		<< LEX::pline_numbers
		        ( scanner, first, next )
		<< ":" << min::eol;
	    LEX::print_phrase_lines
		( printer, scanner, first, next );
	}

	if ( token->type == LEXSTD::end_of_file_t
	     ||
	     token->type == LEXSTD::line_break_t )
	    break;
    }
    return count;
}
