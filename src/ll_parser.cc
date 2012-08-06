// Layers Language Parser Functions
//
// File:	ll__parser.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Aug  6 04:29:29 EDT 2012
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
# include <ll_parser_explicit_subexpression.h>
# include <ll_parser_definitions.h>
# define MUP min::unprotected
# define LEX ll::lexeme
# define LEXSTD ll::lexeme::standard
# define PAR ll::parser
# define TAB ll::parser::table

min::locatable_gen PAR::dot_position;
min::locatable_gen PAR::dot_initiator;
min::locatable_gen PAR::dot_terminator;
min::locatable_gen PAR::dot_separator;
min::locatable_gen PAR::dot_middle;
min::locatable_gen PAR::dot_name;
min::locatable_gen PAR::dot_arguments;
min::locatable_gen PAR::dot_keys;
min::locatable_gen PAR::dot_oper;
min::locatable_gen PAR::doublequote;
min::locatable_gen PAR::number_sign;
min::locatable_gen PAR::new_line;
min::locatable_gen PAR::semicolon;
min::locatable_gen PAR::left_square;
min::locatable_gen PAR::right_square;
min::locatable_gen PAR::comma;
min::locatable_gen PAR::parser_lexeme;
min::locatable_gen PAR::error_operator;
min::locatable_gen PAR::error_operand;

static void initialize ( void )
{
    PAR::dot_position
	= min::new_lab_gen ( ".", "position" );
    PAR::dot_initiator
	= min::new_lab_gen ( ".", "initiator" );
    PAR::dot_terminator
	= min::new_lab_gen ( ".", "terminator" );
    PAR::dot_separator
	= min::new_lab_gen ( ".", "separator" );
    PAR::dot_middle
	= min::new_lab_gen ( ".", "middle" );
    PAR::dot_name
	= min::new_lab_gen ( ".", "name" );
    PAR::dot_arguments
	= min::new_lab_gen ( ".", "arguments" );
    PAR::dot_keys
	= min::new_lab_gen ( ".", "keys" );
    PAR::dot_oper
	= min::new_lab_gen ( ".", "operator" );

    PAR::doublequote = min::new_str_gen ( "\"" );
    PAR::number_sign = min::new_str_gen ( "#" );
    PAR::new_line = min::new_str_gen ( "\n" );
    PAR::semicolon = min::new_str_gen ( ";" );
    PAR::left_square = min::new_str_gen ( "[" );
    PAR::right_square = min::new_str_gen ( "]" );
    PAR::comma = min::new_str_gen ( "," );

    PAR::parser_lexeme = min::new_str_gen ( "parser" );

    PAR::error_operator =
        min::new_str_gen ( "ERROR'OPERATOR" );
    PAR::error_operand =
        min::new_str_gen ( "ERROR'OPERAND" );
}
static min::initializer initializer ( ::initialize );

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
static min::locatable_var<PAR::string_insptr>
    free_strings;

static int number_free_strings = 0;

static int max_string_free_list_size = 100;

static unsigned min_string_length = 80;

PAR::string PAR::new_string
	( min::uns32 n, const min::uns32 * s )
{
    min::locatable_var<PAR::string_insptr> str
        ( (PAR::string_insptr) ::free_strings );

    if ( str == NULL_STUB )
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
    next_ref(str) = NULL_STUB;
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
	return NULL_STUB;
    }

    PAR::string_insptr str =
        (PAR::string_insptr) string;
    next_ref(str) = ::free_strings;
    ::free_strings = str;
    ++ ::number_free_strings;
    return NULL_STUB;
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
static min::locatable_var<PAR::token> free_tokens;

static int number_free_tokens = 0;

static int max_token_free_list_size = 1000;

PAR::token PAR::new_token ( min::uns32 type )
{
    min::locatable_var<PAR::token> token
        ( remove ( ::free_tokens ) );
    if ( token == NULL_STUB )
        token = ::token_type.new_stub();
    else
        -- ::number_free_tokens;
    value_ref(token) = min::MISSING();
    string_ref(token) = NULL_STUB;
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

    value_ref(token) = min::MISSING();
    string_ref(token) = free_string ( token->string );
    put_at_end ( ::free_tokens, token );
    ++ ::number_free_tokens;
}

void PAR::set_max_token_free_list_size ( int n )
{
    ::max_token_free_list_size = n;
    if ( n >= 0 ) while ( ::number_free_tokens > n )
    {
        min::deallocate ( remove ( ::free_tokens ) );
	-- ::number_free_tokens;
    }
}

// Parser Closures
// ------ --------

static min::packed_struct<PAR::input_struct>
    input_type ( "ll::parser::input_type" );

void PAR::init
	( min::ref<PAR::input> input,
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
	( min::ref<PAR::output> output,
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
	( min::ref<PAR::pass> pass,
	  PAR::run_pass run_pass,
	  PAR::init_pass init_pass )
{
    if ( pass == NULL_STUB )
        pass = ::pass_type.new_stub();

    pass->run_pass = run_pass;
    pass->init_pass = init_pass;
}

void PAR::place
	( PAR::parser parser,
	  PAR::pass pass,
	  PAR::pass next )
{
    MIN_ASSERT ( pass->next == NULL_STUB );

    PAR::pass current = parser->pass_stack;
    if ( current == NULL_STUB )
    {
        MIN_ASSERT ( next == NULL_STUB );
	pass_stack_ref(parser) = pass;
    }
    else
    {
	while ( current->next != next )
	{
	    current = current->next;
	    MIN_ASSERT ( current != NULL_STUB );
	}
	next_ref(pass) = current->next;
	next_ref(current) = pass;
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
    min::DISP ( & PAR::parser_struct
                     ::selector_name_table ),
    min::DISP ( & PAR::parser_struct::first ),
    min::DISP_END
};

static min::packed_struct<PAR::parser_struct>
    parser_type ( "ll::parser::parser_type",
                  NULL, ::parser_stub_disp );

min::locatable_var<PAR::parser> PAR::default_parser;

void PAR::init ( min::ref<PAR::parser> parser )
{
    if ( parser == NULL_STUB )
    {
        parser = ::parser_type.new_stub();
	parser->indent_offset = 2;
	bracket_table_ref(parser) =
	    TAB::create_table ( 256 );
	min::push ( parser->bracket_table, 256 );
	split_table_ref(parser) =
	    TAB::create_split_table();
	min::push ( parser->split_table, 256 );
	selector_name_table_ref(parser) =
	    TAB::create_selector_name_table();
	    
    }
    else
    {
	PAR::token token;  // WARNING:: not locatable.
	while (    ( token = PAR::remove
			 ( PAR::first_ref(parser)) )
		!= NULL_STUB )
	    PAR::free ( token );

	parser->eof = false;
	parser->finished_tokens = 0;
	parser->error_count = 0;
    }
}

void PAR::init_input_stream
	( min::ref<PAR::parser> parser,
	  std::istream & in,
	  min::uns32 print_flags,
	  min::uns32 spool_lines )
{
    init ( parser );

    min::init_input_stream
        ( input_file_ref(parser),
	  in, print_flags, spool_lines );
}

void PAR::init_input_file
	( min::ref<PAR::parser> parser,
	  min::file ifile,
	  min::uns32 print_flags,
	  min::uns32 spool_lines )
{
    init ( parser );

    min::init_input_file
        ( input_file_ref(parser),
	  ifile, print_flags, spool_lines );
}

bool PAR::init_input_named_file
	( min::ref<PAR::parser> parser,
	  min::gen file_name,
	  min::uns32 print_flags,
	  min::uns32 spool_lines )
{
    init ( parser );

    return min::init_input_named_file
        ( input_file_ref(parser),
	  file_name, print_flags, spool_lines );
}

void PAR::init_input_string
	( min::ref<PAR::parser> parser,
	  min::ptr<const char> data,
	  min::uns32 print_flags,
	  min::uns32 spool_lines )
{
    init ( parser );

    min::init_input_string
        ( input_file_ref(parser),
	  data, print_flags, spool_lines );
}

void PAR::init_ostream
	( min::ref<PAR::parser> parser,
	  std::ostream & out )
{
    init ( parser );

    min::init_ostream ( printer_ref(parser), out );
}

void PAR::parse ( PAR::parser parser )
{
    // Initialize parser parameters.
    //
    if ( parser->scanner != NULL_STUB )
    {
        LEX::scanner scanner = parser->scanner;
        if ( parser->input_file != scanner->input_file )
	{
	    if ( parser->input_file == NULL_STUB )
	        input_file_ref(parser) =
			scanner->input_file;
	    else if ( scanner->input_file == NULL_STUB )
		LEX::input_file_ref(scanner) =
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
	        printer_ref(parser) =
			scanner->printer;
	    else if ( scanner->printer == NULL_STUB )
	        LEX::printer_ref(scanner) =
			parser->printer;
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
    if (    parser->output != NULL_STUB
         && parser->output->init != NULL)
	( * parser->input->init )
	    ( parser, parser->input );
    for ( PAR::pass pass = parser->pass_stack;
    	  pass != min::NULL_STUB;
	  pass = pass->next )
    {
	if ( pass->init_pass != NULL )
	    ( * pass->init_pass ) ( parser, pass );
    }

    // True if last lexeme was a line break, so an end-
    // of-file is OK.
    //
    bool eof_ok = true;

    // Go to the first non-line-break non-comment token.
    //
    parser->input->add_tokens
	( parser, parser->input );
    PAR::token current = parser->first;
    assert ( current != NULL_STUB );
    while ( current->type == LEXSTD::line_break_t
            ||
	    current->type == LEXSTD::comment_t )
    {
	eof_ok =
	    ( current->type == LEXSTD::line_break_t );

	if ( current->next == parser->first )
	{
	    parser->input->add_tokens
		( parser, parser->input );
	    assert (    current->next
		     != parser->first );
	}
	current = current->next;
	PAR::free
	    ( PAR::remove ( first_ref(parser),
			    current->previous ) );
    }

    // Top level loop.
    //
    bool first_lexeme = true;
    while ( true )
    {
        bool trace =
	    (   parser->trace
	      & PAR::TRACE_EXPLICIT_SUBEXPRESSIONS );

        // If end of file terminate loop.
	//
        if ( current->type == LEXSTD::end_of_file_t )
	{
	    if ( ! eof_ok )
	    {
		parser->printer
		    << min::bom
		    << min::set_indent ( 7 )
		    << "ERROR: line break missing"
		       " from end of file; "
		    << min::pline_numbers
			   ( parser->input_file,
			     current->position )
		    << ":" << min::eom;
		min::print_phrase_lines
		    ( parser->printer,
		      parser->input_file,
		      current->position );
		++ parser->error_count;
	    }
	    break;
	}

	// If first lexeme check its indent is 0.
	//
	if ( first_lexeme )
	{
	    first_lexeme = false;
	    if ( current->indent != 0 )
	    {
		parser->printer
		    << min::bom
		    << min::set_indent ( 7 )
		    << "ERROR: first non-comment lexeme"
		       " is indented; "
		    << min::pline_numbers
			   ( parser->input_file,
			     current->position )
		    << ":" << min::eom;
		min::print_phrase_lines
		    ( parser->printer,
		      parser->input_file,
		      current->position );
		++ parser->error_count;
	    }
	}

	// Get subexpression.  First is the first token
	// of the subexpression.
	//
	PAR::token previous =
	    current == parser->first ?
	    (PAR::token) NULL_STUB :
	    current->previous;

	bool separator_found =
	    PAR::parse_explicit_subexpression
		( parser, parser->selectors,
		  current,
		  0, TAB::top_level_indentation_mark,
		  NULL );

	PAR::token first =
	    previous == NULL_STUB ?
	    parser->first :
	    previous->next;

        // If subexpression is not empty, compact it.
	//
	if ( first != current )
	{

	    min::phrase_position position =
	        { first->position.begin,
	          current->previous->position.end };

	    min::gen terminator = PAR::new_line;
	    if ( separator_found )
	    {
	        // If subexpression ends with an inden-
		// tation separator (e.g., `;'), delete
		// the separator from the subexpression
		// and make it into the terminator.
		//
		terminator =
		    TAB::top_level_indentation_mark
			     ->indentation_separator
			     ->label;
		PAR::remove
		    ( parser, current, terminator );
	    }

	    min::gen g = first->value;
	    bool maybe_parser_definition =
	        ( g == PAR::parser_lexeme );

	    PAR::attr attributes[1] =
		{ PAR::attr ( PAR::dot_terminator,
		              terminator ) };

	    min::uns32 error_count_save =
	        parser->error_count;

	    PAR::compact
		( parser, parser->pass_stack,
	          parser->selectors,
		  PAR::BRACKETED, trace,
		  first, current, position,
		  1, attributes );

	    if ( parser->error_count == error_count_save
	         &&
		 maybe_parser_definition )
	    {
	        min::obj_vec_ptr vp ( first->value );
		if ( vp != NULL_STUB )
		{
		    min::gen result =
		        parser_execute_definition
			    ( vp, parser );
		    if ( result == min::SUCCESS() )
			PAR::free
			    ( PAR::remove
				  ( first_ref(parser),
				    first )
			    );
		    else if ( result == min::ERROR() )
		    {
		        // TBD
		    }
		}
	    }
	}

        // As there is no bracket stack, the token after
	// the subexpression is either a line break, end
	// of file, or token after an indentation sepa-
	// rator.  In the case of a line break, it must
	// be deleted.  End of file's are OK only at the
	// beginning or after such line deleted breaks.
	//
        if ( current->type == LEXSTD::line_break_t )
	{
	    if ( current->next == parser->first )
	    {
		parser->input->add_tokens
		    ( parser, parser->input );
		assert (    current->next
			 != parser->first );
	    }
	    current = current->next;
	    PAR::free
		( PAR::remove ( first_ref(parser),
			        current->previous ) );
	    eof_ok = true;
	}
	else eof_ok = false;
    }
}

// Parser Functions
// ------ ---------

TAB::key_prefix PAR::find_key_prefix
	( PAR::parser parser,
	  PAR::token & current,
	  TAB::table table,
	  PAR::token next )
{
    uns32 phash = min::labhash_initial;
    uns32 table_len = table->length;
    uns32 mask = table_len - 1;
    MIN_ASSERT ( ( table_len & mask ) == 0 );
    TAB::key_prefix previous = NULL_STUB;
    while ( true )
    {
	if ( current == next
	     ||
             current->value == min::MISSING()
	     ||
	     ! is_lexeme ( current->type ) )
	    break;

	min::gen e = current->value;
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

        if ( current->next == parser->first )
	{
	    parser->input->add_tokens
		( parser, parser->input);
	    assert
	        ( current->next != parser->first );
	}

	current = current->next;
    }

    return previous;
}

TAB::root PAR::find_entry
	( PAR::parser parser,
	  PAR::token & current,
	  TAB::key_prefix & key_prefix,
	  TAB::selectors selectors,
	  TAB::table table,
	  PAR::token next )
{
    for ( key_prefix =
	      find_key_prefix
		  ( parser, current, table, next );
          key_prefix != NULL_STUB;
	  key_prefix = key_prefix->previous,
	  current = current->previous )
    for ( TAB::root root = key_prefix->first;
	  root != NULL_STUB;
	  root = root->next )
    {
	if ( root->selectors & selectors )
	    return root;
    }
    return NULL_STUB;
}

TAB::root PAR::find_next_entry
	( PAR::parser parser,
	  PAR::token & current,
	  TAB::key_prefix & key_prefix,
	  TAB::selectors selectors,
	  TAB::root last_entry )
{
    while ( true )
    {
        last_entry = last_entry->next;
	while ( last_entry == NULL_STUB )
	{
	    key_prefix = key_prefix->previous;
	    current = current->previous;
	    if ( key_prefix == NULL_STUB )
		return NULL_STUB;
	    last_entry = key_prefix->first;
	}

	if ( last_entry->selectors & selectors )
	    return last_entry;
    }
}

void PAR::put_empty_before
	( ll::parser::parser parser,
	  ll::parser::token t )
{
    PAR::token token = new_token ( PAR::BRACKETABLE );
    put_before ( PAR::first_ref(parser), t, token );

    min::phrase_position position =
        { t->position.begin, t->position.begin };
    token->position = position;
    min::locatable_var
	    <min::phrase_position_vec_insptr>
	pos;
    min::init ( pos, parser->input_file, position, 0 );

    PAR::value_ref(token) = min::new_obj_gen ( 3, 1 );
    min::obj_vec_insptr vp ( token->value );
    min::attr_insptr ap ( vp );
    min::locate ( ap, PAR::dot_position );
    min::set ( ap, min::new_stub_gen ( pos ) );
}

void PAR::put_empty_after
	( ll::parser::parser parser,
	  ll::parser::token t )
{
    PAR::token token = new_token ( PAR::BRACKETABLE );
    put_before
        ( PAR::first_ref(parser), t->next, token );

    min::phrase_position position =
        { t->position.end, t->position.end };
    token->position = position;
    min::locatable_var
	    <min::phrase_position_vec_insptr>
	pos;
    min::init ( pos, parser->input_file, position, 0 );

    PAR::value_ref(token) = min::new_obj_gen ( 3, 1 );
    min::obj_vec_insptr vp ( token->value );
    min::attr_insptr ap ( vp );
    min::locate ( ap, PAR::dot_position );
    min::set ( ap, min::new_stub_gen ( pos ) );
}

void PAR::compact
	( PAR::parser parser,
	  PAR::pass pass,
	  PAR::table::selectors selectors,
	  min::uns32 type, bool trace,
	  PAR::token & first, PAR::token next,
	  min::phrase_position position,
	  min::uns32 m,
	  PAR::attr * attributes,
	  min::uns32 n )
{
    if ( pass != min::NULL_STUB )
	(* pass->run_pass )
	     ( parser, pass, selectors, first, next );

    if ( first->next == next
         &&
	 first->type == PAR::BRACKETABLE
	 &&
	 type == PAR::BRACKETED )
    {
	min::obj_vec_insptr vp ( first->value );
	min::attr_insptr ap ( vp );

	min::locate ( ap, PAR::dot_position );
	min::phrase_position_vec_insptr pos =
	    min::get ( ap );
	pos->position = position;

	while ( m -- )
	{
	    assert (    attributes->value
		     != min::MISSING() );
	    min::locate ( ap, attributes->name );
	    min::set ( ap, attributes->value );
	    ++ attributes;
	}
	first->type = PAR::BRACKETED;
	first->position = position;
    }
    else
    {
	// Temporary min::gen locatable.
	//
	min::locatable_gen exp;
	min::locatable_var
		<min::phrase_position_vec_insptr>
	    pos;

	// Count tokens.  Also replace non-natural
	// numbers and quoted strings by subexpressions.
	//
	min::uns32 t = 0;
	for ( PAR::token current = first;
	      current != next;
	      ++ t, current = current->next )
	{
	    if ( current->value == min::MISSING() )
		PAR::convert_token ( current );
	}

	// Count the number of non-MISSING extra
	// attributes.
	//
	min::uns32 c = n + m + 1;
	    // Add 1 for position.

	// Hash table size.
	//
	min::uns32 h = c;
	if ( h == 0 ) h = 1;
	if ( h > 4 ) h = 4;

	exp = min::new_obj_gen ( 3*( c + 2 ) + t, h );
	min::obj_vec_insptr expvp ( exp );

	min::init ( pos, parser->input_file,
	            position, t );

	for ( PAR::token current = first;
	      current != next; )
	{
	    min::attr_push(expvp) = current->value;
	    min::push ( pos ) = current->position;

	    current = current->next;
	    PAR::free
		( PAR::remove
		      ( PAR::first_ref(parser),
			current->previous ) );
	}

	min::attr_insptr expap ( expvp );
	min::locate ( expap, PAR::dot_position );
	min::set ( expap, min::new_stub_gen ( pos ) );

	while ( m -- )
	{
	    assert (    attributes->value
	             != min::MISSING() );
	    min::locate ( expap, attributes->name );
	    min::set ( expap, attributes->value );
	    ++ attributes;
	}

	first = PAR::new_token ( type );
	PAR::put_before
	    ( first_ref(parser), next, first );

	PAR::value_ref(first) = exp;
	first->position = position;
    }

    if ( trace )
    {
	min::uns32 GEN_FLAGS = min::GRAPHIC_STR_FLAG
	                     + min::BRACKET_LAB_FLAG;
	parser->printer
	    << ( first->type == PAR::BRACKETED ?
		 "BRACKETED EXPRESSION: " :
		 "BRACKETABLE EXPRESSION: " )
	    << min::pgen ( first->value,
	                   GEN_FLAGS, GEN_FLAGS )
	    << min::indent
	    << min::bom
	    << min::flush_pgen ( first->value )
	    << min::indent
	    << min::pline_numbers
		    ( parser->input_file,
		      position )
	    << ":" << min::eom;
	min::print_phrase_lines
	    ( parser->printer,
	      parser->input_file, position );
    }
}

PAR::token PAR::find_separator
	( min::uns32 & count,
	  PAR::token first,
	  PAR::token next,
	  min::gen * separator,
	  min::uns32 n )
{
    count = 0;

    for ( ; first != next;
            first = first->next, ++ count )
    {
	if ( n == 0 ) continue;

	if ( first->value != separator[0] ) continue;
	if ( n == 1 ) break;

	PAR::token t = first->next;
	min::uns32 i = 1;
	for ( ; i < n; ++ i, t = t->next )
	{
	    if ( t == next ) break;
	    if ( t->value != separator[i] ) break;
	}
	if ( i == n ) break;
    }
    return first;
}

min::gen PAR::get_initiator ( min::gen v )
{
    if ( ! min::is_obj ( v ) ) return min::MISSING();
    min::obj_vec_ptr vp ( v );
    min::attr_ptr ap ( vp );
    min::locate ( ap, PAR::dot_initiator );
    min::gen result = min::get ( ap );
    if ( result == min::NONE()
         ||
	 result == min::MULTI_VALUED() )
	return min::MISSING();
    else
    	return result;
}

min::gen PAR::scan_name_string_label
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  ll::parser::parser parser,
	  min::uns64 accepted_types,
	  min::uns64 ignored_types,
	  min::uns64 end_types )
{
    if ( i >= min::size_of ( vp ) )
        return min::MISSING();

    min::gen element = vp[i];

    if ( get_initiator ( element ) != PAR::doublequote )
        return min::MISSING();

    min::obj_vec_ptr ep = element;
    if ( min::size_of ( ep ) != 1 )
        return min::MISSING();

    min::str_ptr sp = ep[0];

    ++ i;

    if ( parser->name_scanner == NULL_STUB )
    {
         LEX::init_program
	     ( name_scanner_ref ( parser ),
	       parser->scanner->program );
         LEX::init_printer
	     ( name_scanner_ref ( parser ),
	       parser->scanner->printer );
    }

    LEX::init_input_string
	( name_scanner_ref ( parser ),
	  min::begin_ptr_of ( sp ),
	  parser->input_file->print_flags );

    return LEX::scan_name_string
	( name_scanner_ref ( parser ),
	  accepted_types, ignored_types, end_types );
}

void PAR::convert_token ( PAR::token token )
{
    assert ( token->value == min::MISSING() );

    min::gen initiator;

    if (    token->type
	 == LEXSTD::quoted_string_t )
	initiator = PAR::doublequote;
    else
    {
	assert (    token->type
		 == LEXSTD::number_t );

	initiator = PAR::number_sign;
    }


    PAR::value_ref(token)
	= min::new_obj_gen ( 10, 1 );
    min::obj_vec_insptr elemvp
	( token->value );
    min::attr_push(elemvp) = min::MISSING();

    min::attr ( elemvp, 0 ) =
                    min::new_str_gen
			( min::begin_ptr_of
			      ( token->string ),
			  token->string->length );
    PAR::string_ref(token) =
	PAR::free_string ( token->string );

    min::attr_insptr elemap ( elemvp ); 
    min::locate ( elemap, PAR::dot_initiator );
    min::set ( elemap, initiator );

    token->type = PAR::BRACKETED;
}
