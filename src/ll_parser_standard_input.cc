// Layered Languages Standard Input Parser
//
// File:	ll_parser_standard_input.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Sep 20 03:34:12 EDT 2021
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
# define TAB ll::parser::table

// Standard Input Parser
// -------- ----- ------

static min::uns32 erroneous_atom_stub_disp[] = {
    min::DISP ( & PARSTD::erroneous_atom_struct
                        ::parser ),
    min::DISP_END };

static min::packed_struct_with_base
	<PARSTD::erroneous_atom_struct,
	 LEX::erroneous_atom_struct>
    erroneous_atom_type
	( "ll::parser::standard::erroneous_atom_type",
	  NULL, ::erroneous_atom_stub_disp );
const min::uns32 & PARSTD::ERRONEOUS_ATOM =
    ::erroneous_atom_type.subtype;

min::locatable_var<PAR::input>
    PARSTD::input;
min::locatable_var<PARSTD::erroneous_atom>
    PARSTD::default_erroneous_atom;

static min::uns32 input_add_tokens
	( PAR::parser parser,
	  PAR::input input );
static void input_init
	( PAR::parser parser,
	  PAR::input input )
{
    LEX::init ( PAR::scanner_ref(parser) );
    PAR::init_lexeme_map ( parser );
}
static void erroneous_atom_announce
	( min::uns32 first, min::uns32 last,
	  min::uns32 type, LEX::scanner scanner,
	  LEX::erroneous_atom erroneous_atom );

void PARSTD::init_input
	( min::ref<PAR::parser> parser )
{
    MIN_REQUIRE ( parser->scanner == min::NULL_STUB );

    PAR::init ( PARSTD::input,
                ::input_add_tokens,
		::input_init );

    PARSTD::default_erroneous_atom =
        ::erroneous_atom_type.new_stub();
    PARSTD::default_erroneous_atom->announce =
        ::erroneous_atom_announce;
    PARSTD::default_erroneous_atom->mode = LEX::NORMAL;
    PARSTD::default_erroneous_atom->count = 0;
    PARSTD::parser_ref(PARSTD::default_erroneous_atom) =
        parser;

    PAR::init ( parser );
    LEXSTD::init_standard_program();
    LEX::init_program
	( PAR::scanner_ref(parser),
          LEXSTD::default_program );
    input_ref(parser) = PARSTD::input;
    LEX::erroneous_atom_ref(parser->scanner) =
        (LEX::erroneous_atom)
	PARSTD::default_erroneous_atom;
}

// Erroneous Atom Announce
// --------- ---- --------

static void erroneous_atom_announce
	( min::uns32 first, min::uns32 next,
	  min::uns32 type, LEX::scanner scanner,
	  LEX::erroneous_atom erroneous_atom )
{
    const char * message;
    bool warning = false;
    switch ( type )
    {
    case LEXSTD::misplaced_vertical_t:
	message = "vertical control character"
	          " sequence with no line feed";
	warning = true;
	break;
    case LEXSTD::illegal_control_t:
	message = "illegal control"
	          " character sequence";
	break;
    case LEXSTD::unrecognized_character_t:
	message = "unrecognized"
	          " character sequence";
	break;
    case LEXSTD::unrecognized_escape_t:
	message = "unrecognized escape"
	          " sequence";
	break;
    case LEXSTD::misplaced_horizontal_t:
	message = "horizontal characters other"
	          " than single space";
	break;
    default:
	message = "system error: unrecognized"
	          " erroneous atom type";
	break;
    }

    PAR::parser parser =
        ((PARSTD::erroneous_atom) erroneous_atom)
	    ->parser;
    min::phrase_position position =
        LEX::phrase_position ( scanner, first, next );

    if ( warning )
	PAR::parse_warn ( parser, position, message );
    else
	PAR::parse_error ( parser, position, message );
}

// Add Tokens
// --- ------
//
inline bool is_letter ( min::Uchar c )
{
    if ( 'A' <= c && c <= 'Z' ) return true;
    if ( 'a' <= c && c <= 'z' ) return true;
    return false;
}
inline bool is_digit ( min::Uchar c )
{
    if ( '0' <= c && c <= '9' ) return true;
    return false;
}
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
    TAB::lexeme_map lexeme_map = parser->lexeme_map;
    min::uns32 lexeme_map_length = lexeme_map->length;
    min::Uchar ID_character =
        parser->id_map->ID_character;
    TAB::flags selectors = parser->selectors;
    min::locatable_gen symbol;
    while ( true )
    {

SCAN_NEXT_LEXEME:

        min::uns32 type =
	    LEX::scan ( first, next, scanner );

	const char * message = NULL;
	switch ( type )
	{
	case LEX::SCAN_ERROR:
	{
	    printer << min::error_message;
	    type =  LEXSTD::end_of_file_t;
	    break;
	}
	case LEXSTD::horizontal_space_t:
	    continue;

	case LEXSTD::premature_end_of_string_t:
	    message = "ERROR: string ended by"
	              " line break or end of file; ";
	    break;
	case LEXSTD::premature_end_of_file_t:
	    message = "ERROR: end of file not preceeded"
	              " by a line break; ";
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

	    ++ parser->error_count;

	    continue;
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
	    next < input_buffer->length ?
	    (&input_buffer[next])->indent :
	    scanner->next_indent;

	TAB::root r =
	    ( type < lexeme_map_length ?
	      lexeme_map[type] :
	      (TAB::root) min::NULL_STUB );

	while ( r != min::NULL_STUB )
	{
	    if ( r->selectors & selectors ) break;
	    r = r->next;
	}

	if ( r != min::NULL_STUB )
	{
	    TAB::lexeme_map_entry e =
		(TAB::lexeme_map_entry) r;
	    if (     e->lexical_master
		  != LEX::MISSING_MASTER )
		PAR::set_lexical_master
		    ( e->lexical_master, parser );

	    if ( e->token_value == min::NONE() )
	    {
		PAR::free ( token );
		goto SCAN_NEXT_LEXEME;
	    }
	    else
	    if ( ! min::is_obj ( e->token_value ) )
		PAR::value_ref(token) =
		    e->token_value;
	    else
		PAR::value_ref(token) =
		    min::copy
			( e->token_value, 10 );

	    token->type = e->token_type;
	    PAR::value_type_ref(token) =
	        e->token_value_type;
	}
	else switch ( type )
	{
	case LEXSTD::numeric_t:
	{
	    min::uns32 length =
	        translation_buffer->length;
	    if (    length >= 2
	         &&    translation_buffer[0]
		    == ID_character
		 &&
		 translation_buffer[1] != '0' )
	    {
	        min::uns32 ID = 0;
		min::uns32 i = 1;
		const min::uns32 ID_limit =
		    ( 0xFFFFFFFF - 9 ) / 10;
		for ( ; i < length; ++ i )
		{
		    min::Uchar c =
		        translation_buffer[i];
		    if ( c < '0' || '9' < c ) break;
		    if ( ID > ID_limit ) break;
		    ID = 10 * ID + ( c - '0' );
		}
		if ( i == length )
		{
		    // numeric token has the right
		    // format.
		    //
		    min::id_map id_map = parser->id_map;

		    min::gen value =
		        min::map_get ( id_map, ID );
		    if ( value == min::NONE() )
		    {
			PAR::value_ref(token) =
			  min::new_preallocated_gen(ID);
			min::map_set
			    ( id_map, ID,
			      token->value );
		    }
		    else
		    {
			PAR::value_ref(token) = value;
			if ( min::is_preallocated
			         ( value ) )
			    min::increment_preallocated
			         ( value );
		    }

		    token->type = PAR::find_token_type
		    	( PAR::value_type_ref(token),
			  token->value );
		    break;
		}
		// else falls through
	    }
	}
	// falls through
	case LEXSTD::natural_t:
	{
	    // Numeric or natural.
	    //
	    value_ref(token) = min::new_str_gen
	        ( min::begin_ptr_of
		      ( translation_buffer ),
		  translation_buffer->length );
	    min::str_ptr sp ( token->value );
	    min::unsptr length = strlen ( sp );
	    min::float64 val;
	    min::unsptr i = 0;
	    min::strto ( val, sp, i );
	    if ( i == length )
	    {
	        value_ref(token) =
		    min::new_num_gen ( val );
		if (    0 <= val
		     && val <= PAR::MAX_NATURAL
		     && (min::int64) val == val )
		    token->type = LEXSTD::natural_t;
		else
		    token->type = PAR::NUMBER;
	    }
	    else
	        token->type = LEXSTD::numeric_t;
	    break;
	}
	case LEXSTD::mark_t:
	case LEXSTD::separator_t:
	case LEXSTD::quoted_string_t:
	{
	    value_ref(token) = min::new_str_gen
	        ( min::begin_ptr_of
		      ( translation_buffer ),
		  translation_buffer->length );
	    break;
	}
	case LEXSTD::word_t:
	{
	    min::uns32 length =
	        translation_buffer->length;
	    value_ref(token) = min::new_str_gen
	        ( min::begin_ptr_of
		      ( translation_buffer ),
		  length );
	    if (    length >= 3
	         &&    translation_buffer[0]
		    == ID_character
	         &&    translation_buffer[length-1]
		    == ID_character
	         && is_letter ( translation_buffer[1] )
	       )
	    {
		min::uns32 i;
		for ( i = 2; i < length-1; ++ i )
		{
		    min::Uchar c =
			translation_buffer[i];
		    if ( is_letter ( c ) ) continue;
		    if ( is_digit ( c ) ) continue;
		    break;
		}
		if ( i == length-1 )
		{
		    // word token has the right format.
		    //
		    min::id_map id_map = parser->id_map;
		    symbol = token->value;
		        // Made locatable.

		    min::gen value =
		        min::map_get ( id_map, symbol );
		    if ( value == min::NONE() )
		    {
			PAR::value_ref(token) =
			  min::new_preallocated_gen(0);
			min::map_set
			    ( id_map, symbol,
			      token->value );
		    }
		    else
		    {
			PAR::value_ref(token) = value;
			if ( min::is_preallocated
			         ( value ) )
			    min::increment_preallocated
			         ( value );
		    }

		    token->type = PAR::find_token_type
		    	( PAR::value_type_ref(token),
			  token->value );
		}
	    }
	    break;
	}
	case LEXSTD::comment_t:
	case LEXSTD::line_break_t:
	case LEXSTD::indent_t:
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
	        << ( token->type == PAR::NUMBER ?
		     "NUMBER" :
		     LEXSTD::type_names[type] )
		<< ": ";
	    if ( token->value != min::MISSING() )
	        printer
		    << min::save_print_format
		    << min::graphic_only
		    << min::pgen_quote ( token->value )
		    << min::restore_print_format
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
	        token->type
	     == LEXSTD::indent_t
	     ||
	     count >= 100
	   )
	    break;
    }
    return count;
}
