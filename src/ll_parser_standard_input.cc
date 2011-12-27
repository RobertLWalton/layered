// Layers Language Standard Input Parser
//
// File:	ll_parser_standard_input.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Sun Dec 25 18:04:46 EST 2011
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

// Lexical Type Support
// ------- ---- -------

const LEX::uns8
    LEXSTD::internal::lexeme_type_table[256] = { 0 };

static struct initializer
{
    initializer ( void )
    {
        LEX::uns8 * p = (LEX::uns8 *)
	    LEXSTD::internal::lexeme_type_table;
	for ( char * q = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	                 "abcdefghijklmnopqrstuvwxyz";
	      * q; ) p[*q++] = LEXSTD::word_t; 
	for ( unsigned c = 128; c < 256; ++ c )
	      p[c] = LEXSTD::word_t; 
	      // Only words can begin with a non-ASCII
	      // character.
	for ( unsigned c = '0'; c <= '9'; ++ c )
	      p[c] = LEXSTD::number_t; 
	for ( char * q = "+-*~@#$%^&=|>_!?:<./"; * q; )
	      p[*q++] = LEXSTD::mark_t; 
	for ( char * q = ";,`'()[]{}"; * q; )
	      p[*q++] = LEXSTD::separator_t; 
    }
} init;


// Standard Input Parser
// -------- ----- ------

min::locatable_var<PAR::input>
    PARSTD::input;
min::locatable_var<LEX::erroneous_atom>
    PARSTD::erroneous_atom;

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
    case LEXSTD::unrecognized_escape_seq_t:
	message = "ERROR: unrecognized escape"
	          " sequence; ";
	break;
    case LEXSTD::misplaced_control_char_t:
	message = "ERROR: misplaced control"
	          " character; ";
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
    bool trace = ( parser->trace & PAR::TRACE_INPUT );

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

	case LEXSTD::premature_end_of_line_t:
	    message = "ERROR: premature end of line; ";
	    break;
	case LEXSTD::premature_end_of_file_t:
	    message = "ERROR: premature end of file; ";
	    break;
	case LEXSTD::misplaced_char_t:
	    message = "ERROR: misplaced character; ";
	    break;
	case LEXSTD::misplaced_space_t:
	    message = "ERROR: misplaced space; ";
	    break;
	case LEXSTD::ascii_escape_seq_t:
	    message = "ERROR: ascii escape sequence; ";
	    break;
	case LEXSTD::non_letter_escape_seq_t:
	    message = "ERROR: non-letter escape"
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
	    input_buffer[first].indent :
	    scanner->next_indent;

	switch ( type )
	{
	case LEXSTD::word_t:
	case LEXSTD::mark_t:
	case LEXSTD::separator_t:
	case LEXSTD::natural_number_t:
	{
	    value_ref(token) = min::new_str_gen
	        ( translation_buffer.begin_ptr(),
		  translation_buffer->length );
	    break;
	}
	case LEXSTD::quoted_string_t:
	case LEXSTD::number_t:
	{
	    int length = translation_buffer->length;
	    min::uns32 * p =
		translation_buffer.begin_ptr();
	    PAR::string_ref(token) =
	        PAR::new_string ( length, p );
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
	        << LEXSTD::type_name[type]
		<< ": ";
	    if ( token->value != min::MISSING() )
	        printer
		    << min::push_parameters
		    << min::graphic
		    << min::pgen ( token->value,
		                   & ::str_format )
		    << min::pop_parameters
		    << ": ";
	    else if ( token->string != min::NULL_STUB )
	        printer
		    << min::push_parameters
		    << min::graphic
		    << min::punicode
		            ( token->string->length,
			      token->string.begin_ptr()
			    )
		    << min::pop_parameters
		    << ": ";
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
