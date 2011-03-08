// Layers Language Parser Pass Functions
//
// File:	ll__parser.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Mar  8 10:14:50 EST 2011
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Strings
//	Tokens
//	Parser Closures
//	Parser
//	Parser Functions

// Usage and Setup
// ----- --- -----

# include <ll_lexeme_standard.h>
# include <ll_parser.h>
# include <ll_parser_input.h>
# define LEX ll::lexeme
# define LEXSTD ll::lexeme::standard
# define PAR ll::parser
# define TAB ll::parser::table

// Strings
// -------

static min::uns32 string_stub_disp[] =
{
    min::DISP ( & PAR::token_struct::next ),
    min::DISP_END
};

static min::packed_vec<min::uns32,PAR::string_struct>
    string_type ( "ll::parser::string_type",
                  NULL, ::string_stub_disp );

// Free list of strings.
//
static min::locatable_ptr<PAR::string_insptr>
    free_strings;

static int number_free_strings = 0;

static int max_string_free_list_size = 100;

static int min_string_length = 80;

PAR::string PAR::new_string
	( min::uns32 n, const min::uns32 * s )
{
    min::locatable_ptr<PAR::string_insptr>
        str = ::free_strings;
    if ( str == min::NULL_STUB )
    {
        min::uns32 m = n;
	if ( m < ::min_string_length )
	    m = ::min_string_length;
        str = ::string_type.new_stub ( m );
    }
    else
    {
        -- ::number_free_strings;
	::free_strings = str->next;
	if ( str->max_length < n )
	    min::resize ( str, n );
	min::pop ( str, str->length );
    }
    str->next = min::NULL_STUB;
    min::push ( str, n, s );
    return (PAR::string) str;
}

PAR::string PAR::free_string ( PAR::string string )
{
    if ( string == NULL_STUB )
        return NULL_STUB;

    if ( ::max_string_free_list_size >= 0
         &&
            ::number_free_strings
	 >= ::max_string_free_list_size )
    {
        min::deallocate ( string );
	return min::NULL_STUB;
    }

    PAR::string_insptr str =
        (PAR::string_insptr) string;
    str->next = ::free_strings;
    ::free_strings = str;
    ++ ::number_free_strings;
    return min::NULL_STUB;
}

void PAR::set_max_string_free_list_size ( int n )
{
    ::max_string_free_list_size = n;
    if ( n >= 0 ) while ( ::number_free_strings > n )
    {
	PAR::string_insptr string = ::free_strings;
	::free_strings = string->next;
        min::deallocate ( string );
	-- ::number_free_strings;
    }
}

void PAR::resize ( PAR::string string )
{
    PAR::string_insptr s = (PAR::string_insptr) string;
    min::resize ( s, s->length );
}

// Tokens
// ------

static min::uns32 token_gen_disp[] =
{
    min::DISP ( & PAR::token_struct::value ),
    min::DISP_END
};

static min::uns32 token_stub_disp[] =
{
    min::DISP ( & PAR::token_struct::string ),
    min::DISP ( & PAR::token_struct::next ),
    min::DISP ( & PAR::token_struct::previous ),
    min::DISP_END
};

static min::packed_struct<PAR::token_struct>
    token_type ( "ll::parser::token",
                 ::token_gen_disp,
                 ::token_stub_disp );

// Free list of tokens.
//
static min::locatable_ptr<PAR::token> free_tokens;

static int number_free_tokens = 0;

static int max_token_free_list_size = 1000;

PAR::token PAR::new_token ( min::uns32 type )
{
    min::locatable_ptr<PAR::token> token =
	remove ( ::free_tokens );
    if ( token == min::NULL_STUB )
        token = ::token_type.new_stub();
    else
        -- ::number_free_tokens;
    token->value = min::MISSING;
    token->string = min::NULL_STUB;
    token->type = type;
    return token;
}

void PAR::free ( PAR::token token )
{
    if ( ::max_token_free_list_size >= 0
         &&
            ::number_free_tokens
	 >= ::max_token_free_list_size )
    {
	free_string ( token->string );
        min::deallocate ( token );
	return;
    }

    token->value = min::MISSING;
    token->string = free_string ( token->string );
    put_at_end ( ::free_tokens, token );
    ++ ::number_free_tokens;
}

void PAR::set_max_token_free_list_size ( int n )
{
    ::max_token_free_list_size = n;
    if ( n >= 0 ) while ( ::number_free_tokens > n )
    {
	free_string ( ::free_tokens->string );
        min::deallocate ( remove ( ::free_tokens ) );
	-- ::number_free_tokens;
    }
}

// Parser Closures
// ------ --------

static min::packed_struct<PAR::input_struct>
    input_type ( "ll::parser::input_type" );

void PAR::init
	( PAR::input & input,
	  uns32 (*add_tokens)
	      ( PAR::parser parser, PAR::input input ),
	  void (*init)
	      ( PAR::parser parser, PAR::input input ) )
{
    if ( input == NULL_STUB )
        input = ::input_type.new_stub();

    input->add_tokens = add_tokens;
    input->init = init;
}

static min::packed_struct<PAR::output_struct>
    output_type ( "ll::parser::output_type" );

void PAR::init
	( PAR::output & output,
	  void (*remove_tokens)
	      ( PAR::parser parser,
	        PAR::output output ),
	  void (*init)
	      ( PAR::parser parser,
	        PAR::output output ) )
{
    if ( output == NULL_STUB )
        output = ::output_type.new_stub();

    output->remove_tokens = remove_tokens;
    output->init = init;
}

static min::packed_struct<PAR::pass_struct>
    pass_type ( "ll::parser::pass_type" );

void PAR::init
	( PAR::pass & pass,
	  bool (*run)
	      ( PAR::parser parser, PAR::pass pass,
	        PAR::token & first, PAR::token end ),
	  void (*init)
	      ( PAR::parser parser, PAR::pass pass ) )
{
    if ( pass == NULL_STUB )
        pass = ::pass_type.new_stub();

    pass->run = run;
    pass->init = init;
}

void PAR::place
	( PAR::parser parser,
	  PAR::pass pass,
	  PAR::pass previous )
{
    if ( previous == NULL_STUB )
    {
        pass->next = parser->pass_stack;
	parser->pass_stack = pass;
    }
    else
    {
        PAR::pass current = parser->pass_stack;
	for ( ; current != NULL_STUB;
	        current = current->next )
	{
	    if ( current == previous )
	    {
	        pass->next = current->next;
		current->next = pass;
		return;
	    }
	}
	MIN_ABORT
	    ( "PAR::place could not find `previous'" );
    }
}

// Parser
// ------

static min::uns32 parser_stub_disp[] =
{
    min::DISP ( & PAR::parser_struct::input ),
    min::DISP ( & PAR::parser_struct::output ),
    min::DISP ( & PAR::parser_struct::pass_stack ),
    min::DISP ( & PAR::parser_struct::scanner ),
    min::DISP ( & PAR::parser_struct::input_file ),
    min::DISP ( & PAR::parser_struct::printer ),
    min::DISP ( & PAR::parser_struct::bracket_table ),
    min::DISP ( & PAR::parser_struct::split_table ),
    min::DISP ( & PAR::parser_struct::first ),
    min::DISP_END
};

static min::packed_struct<PAR::parser_struct>
    parser_type ( "ll::parser::parser_type",
                  NULL, ::parser_stub_disp );

static min::uns32 table_stub_disp[] =
{
    0,
    min::DISP_END
};

static min::packed_vec<TAB::key_prefix>
    table_type ( "ll::parser::table::table_type",
    		  NULL, ::table_stub_disp );

static min::packed_vec<TAB::indentation_split>
    split_table_type
        ( "ll::parser::table::split_table_type",
   	  NULL, ::table_stub_disp );

min::locatable_ptr<PAR::parser> PAR::default_parser;

void PAR::init ( PAR::parser & parser )
{
    if ( parser == NULL_STUB )
    {
        parser = ::parser_type.new_stub();
	parser->indent_offset = 2;
	parser->bracket_table =
	    ::table_type.new_stub ( 256 );
	min::push ( parser->bracket_table, 256 );
	parser->split_table =
	    ::split_table_type.new_stub ( 256 );
	min::push ( parser->split_table, 256 );
    }

    PAR::token token;
    while (    ( token = PAR::remove ( parser->first ) )
            != NULL_STUB )
        PAR::free ( token );

    parser->eof = false;
    parser->finished_tokens = 0;
}

void PAR::init_input_stream
	( PAR::parser & parser,
	  std::istream & in,
	  min::uns32 print_flags,
	  min::uns32 spool_lines )
{
    init ( parser );

    min::init_input_stream
        ( parser->input_file, in,
	  print_flags, spool_lines );
}

void PAR::init_input_file
	( PAR::parser & parser,
	  min::file ifile,
	  min::uns32 print_flags,
	  min::uns32 spool_lines )
{
    init ( parser );

    min::init_input_file
        ( parser->input_file, ifile,
	  print_flags, spool_lines );
}

bool PAR::init_input_named_file
	( PAR::parser & parser,
	  min::gen file_name,
	  min::uns32 print_flags,
	  min::uns32 spool_lines )
{
    init ( parser );

    return min::init_input_named_file
        ( parser->input_file, file_name,
	  print_flags, spool_lines );
}

void PAR::init_input_string
	( PAR::parser & parser,
	  const char * data,
	  min::uns32 print_flags,
	  min::uns32 spool_lines )
{
    init ( parser );

    min::init_input_string
        ( parser->input_file, data,
	  print_flags, spool_lines );
}

void PAR::init_output_stream
	( PAR::parser & parser,
	  std::ostream & out )
{
    init ( parser );

    min::init_output_stream
        ( parser->printer, out );
}

// Parse an explicit subexpression that begins with the
// `first' token (which is just after the opening
// bracket or indentation mark).  If more tokens are
// needed, call parser->input.
//
// If closing_bracket is NULL_STUB, the expression was
// begun by an indentation mark.  The first token is the
// line-break token after the indentation mark (and this
// should be deleted).  The next non-line-break token
// sets the indentation associated with the indentation
// mark.
//
// Otherwise, if closing_bracket != NULL_STUB, the
// expression was begun by the opening bracket corres-
// ponding to the closing bracket, and the first token
// is the first token after the opening bracket.
//
// This function identifies all the tokens in the sub-
// expression and returns pointers to the first of these
// in `first' and to the first token after those in the
// subexpression in `end' (note that here is always an
// end-of-file token so there will always be such a 
// token).  SUBSUBexpresions are converted to an
// EXPRESSION token whose value is a list.  If there are
// no tokens in the resullting subexpression, `first' is
// set equal to `end'.
//
// Finding a token with indentation <= indent terminates
// the subexpression.  If a closing_bracket != NULL_
// STUB, this also produces an error message.
//
// Finding a closing bracket other than closing_bracket
// produces an error message and terminates the expres-
// sion.
//
// This function calls itself recursively if it finds
// an opening bracket or an indentation mark.  The
// selectors determine which bracket and indentation
// mark definitions are active.  When this function
// calls itself recursively, upon return it wraps all
// the tokens of the sub-subexpression found into a
// single EXPRESSION token.
//
// Line_break tokens are deleted.  Gluing indentation
// marks are split from line-ending tokens.  Bracket
// recognition preceeds token splitting and line_break
// deletion: so the last lexeme of a bracket cannot be
// the first part of a split token (this should not be
// a problem as the last lexeme of a bracked should be
// a separator), and multi-lexeme brackets cannot
// straddle line_breaks.
//
// This function is called at the top level with
// indent = a very negative integer and closing_
// bracket = NULL_STUB.
//
static void parse_explicit_subexpression
	( PAR::parser parser,
	  PAR::token & first,
	  PAR::token & end,
	  TAB::closing_bracket closing_bracket,
	  min::int32 indent,
	  TAB::selectors selectors )
{
    PAR::token next = first;
    bool is_first = true;
    while ( true )
    {
	// If we have run off end of parser token list,
	// get more tokens.
	//
        if ( next == parser->first )
	{
	    assert ( ! parser->eof );
	    next = next->previous;
	    parser->input->add_tokens
		( parser, parser->input );
	    next = next->next;
	    assert ( next != parser->first );
	    if ( is_first ) first = next;
	}

	// Delete line breaks.
	//
	if ( next->type = LEXSTD::line_break_t )
	{
	    if ( is_first ) first = next->next;
	    remove ( parser->first, next );
	    continue;
	}

	// Complain if token indent is too near
	// paragraph indent.
	//
	int near = (min::int32) first->begin.column
	         - indent;
	if ( near < 0 ) near = - near;
	if (    near != 0
	     && near < parser->indent_offset )
	{
	    parser->printer
	        << min::bom << min::set_indent ( 7 )
		<< "ERROR: lexeme indent "
		<< first->begin.column
		<< " too near paragraph indent "
		<< indent
		<< "; "
		<< min::pline_numbers
		       ( parser->input_file,
		         first->begin.line,
			 next->begin.line )
	        << ":" << min::eom;
	    LEX::print_item_lines
		( parser->printer,
		  parser->input_file,
		  first->begin,
		  first->end );
	}

	// Truncate subexpression if token is at or
	// before indent.
	//
	if (    (min::int32) first->begin.column
	     <= indent + parser->indent_offset )
	{
	    if ( closing_bracket != min::NULL_STUB )
	    {
	    }

	}
    }
    if ( is_first ) first = next;
    end = next;
}


void PAR::parse ( PAR::parser parser )
{
    if ( parser->scanner != NULL_STUB )
    {
        LEX::scanner scanner = parser->scanner;
        if ( parser->input_file != scanner->input_file )
	{
	    if ( parser->input_file == NULL_STUB )
	        parser->input_file =
		    scanner->input_file;
	    else if ( scanner->input_file == NULL_STUB )
	        scanner->input_file =
		    parser->input_file;
	    else MIN_ABORT
	        ( "input_file of parser and"
		  " parser->scanner are not the same" );
	}
	else if ( parser->input_file == NULL_STUB )
	    MIN_ABORT
	        ( "parser->input_file not defined" );

        if ( parser->printer != scanner->printer )
	{
	    if ( parser->printer == NULL_STUB )
	        parser->printer =
		    scanner->printer;
	    else if ( scanner->printer == NULL_STUB )
	        scanner->printer = parser->printer;
	    else MIN_ABORT
	        ( "printer of parser and"
		  " parser->scanner are not the same" );
	}
	else if ( parser->printer == NULL_STUB )
	    MIN_ABORT
	        ( "parser->printer not defined" );
    }

    if ( parser->input->init != NULL)
	( * parser->input->init )
	    ( parser, parser->input );

    while ( ! parser->eof )
        parser->input->add_tokens
	    ( parser, parser->input );
}

// Parser Functions
// ------ ---------

TAB::key_prefix PAR::find_key_prefix
	( PAR::parser parser,
	  PAR::token first,
	  TAB::table table )
{
    uns32 phash = min::labhash_initial;
    uns32 table_len = table->length;
    uns32 mask = table_len - 1;
    MIN_ASSERT ( ( table_len & mask ) == 0 );
    TAB::key_prefix previous = NULL_STUB;
    while ( true )
    {
        if ( first->type != SYMBOL ) break;

	min::gen e = first->value;
	uns32 hash;
	if ( min::is_str ( e ) )
	    hash = min::strhash ( e );
	else if ( min::is_num ( e ) )
	{
	    int v = min::int_of ( e );
	    MIN_ASSERT ( 0 <= v && v < (1<<28) );
	    hash = min::numhash ( e );
	}
	else
	    MIN_ABORT ( "bad key element type" );

	// Compute hash of this element's key prefix.
	//
	phash = min::labhash ( phash, hash );
	if ( previous != NULL_STUB ) hash = phash;

	// Locate key prefix.
	//
	TAB::key_prefix key_prefix =
	    table[hash & mask];
	while ( key_prefix != NULL_STUB )
	{
	    if ( key_prefix->key_element == e
	         &&
		 key_prefix->previous == previous )
	        break;
	    key_prefix = key_prefix->next;
	}
	if ( key_prefix == NULL_STUB ) break;

	previous = key_prefix;

        if ( first->next == parser->first )
	{
	    if ( parser->eof ) break;

	    parser->input->add_tokens
		( parser, parser->input);
	    if ( first->next == parser->first ) break;
	}

	first = first->next;
    }

    return previous;
}
