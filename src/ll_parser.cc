// Layers Language Parser Pass Functions
//
// File:	ll__parser.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu Mar 17 13:56:03 EDT 2011
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
# define MUP min::unprotected
# define LEX ll::lexeme
# define LEXSTD ll::lexeme::standard
# define PAR ll::parser
# define TAB ll::parser::table

static min::locatable_gen initiator;
static min::locatable_gen terminator;
static min::locatable_gen separator;
static min::locatable_gen doublequote;
static min::locatable_gen number_sign;

static struct initializer {
    initializer ( void )
    {
        ::initiator =
	    min::new_str_gen ( ".initiator" );
        ::terminator =
	    min::new_str_gen ( ".terminator" );
        ::separator =
	    min::new_str_gen ( ".separator" );
        ::doublequote =
	    min::new_str_gen ( "\"" );
        ::number_sign =
	    min::new_str_gen ( "#" );
    }
} init;

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

static int min_string_length = 80;

PAR::string PAR::new_string
	( min::uns32 n, const min::uns32 * s )
{
    min::locatable_var<PAR::string_insptr> str;
    str = (PAR::string_insptr) ::free_strings;
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
    min::locatable ( str, str->next ) =
        (PAR::string_insptr) ::free_strings;
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
static min::locatable_var<PAR::token> free_tokens;

static int number_free_tokens = 0;

static int max_token_free_list_size = 1000;

PAR::token PAR::new_token ( min::uns32 type )
{
    min::locatable_var<PAR::token> token;
    token = remove ( ::free_tokens );
    if ( token == min::NULL_STUB )
        token = ::token_type.new_stub();
    else
        -- ::number_free_tokens;
    token->value = min::MISSING();
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

    token->value = min::MISSING();
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
	( min::ptr<PAR::input> input,
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
	( min::ptr<PAR::output> output,
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
	( min::ptr<PAR::pass> pass,
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
        min::locatable ( pass, pass->next ) =
	    parser->pass_stack;
	min::locatable ( parser, parser->pass_stack ) =
	    pass;
    }
    else
    {
        PAR::pass current = parser->pass_stack;
	for ( ; current != NULL_STUB;
	        current = current->next )
	{
	    if ( current == previous )
	    {
	        min::locatable ( pass, pass->next ) =
		    current->next;
		min::locatable
		    ( current, current->next ) = pass;
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

min::locatable_var<PAR::parser> PAR::default_parser;

void PAR::init ( min::ptr<PAR::parser> parser )
{
    if ( parser == NULL_STUB )
    {
        parser = ::parser_type.new_stub();
	parser->indent_offset = 2;
	min::locatable
	    ( parser, parser->bracket_table ) =
	    ::table_type.new_stub ( 256 );
	min::push ( parser->bracket_table, 256 );
	min::locatable ( parser, parser->split_table ) =
	    ::split_table_type.new_stub ( 256 );
	min::push ( parser->split_table, 256 );
    }

    PAR::token token;
    while (    ( token = PAR::remove
                     ( min::locatable
		           ( parser, parser->first ) ) )
            != NULL_STUB )
        PAR::free ( token );

    parser->eof = false;
    parser->finished_tokens = 0;
}

void PAR::init_input_stream
	( min::ptr<PAR::parser> parser,
	  std::istream & in,
	  min::uns32 print_flags,
	  min::uns32 spool_lines )
{
    init ( parser );

    min::init_input_stream
        ( min::locatable ( parser, parser->input_file ),
	  in, print_flags, spool_lines );
}

void PAR::init_input_file
	( min::ptr<PAR::parser> parser,
	  min::file ifile,
	  min::uns32 print_flags,
	  min::uns32 spool_lines )
{
    init ( parser );

    min::init_input_file
        ( min::locatable ( parser, parser->input_file ),
	  ifile, print_flags, spool_lines );
}

bool PAR::init_input_named_file
	( min::ptr<PAR::parser> parser,
	  min::gen file_name,
	  min::uns32 print_flags,
	  min::uns32 spool_lines )
{
    init ( parser );

    return min::init_input_named_file
        ( min::locatable ( parser, parser->input_file ),
	  file_name, print_flags, spool_lines );
}

void PAR::init_input_string
	( min::ptr<PAR::parser> parser,
	  const char * data,
	  min::uns32 print_flags,
	  min::uns32 spool_lines )
{
    init ( parser );

    min::init_input_string
        ( min::locatable ( parser, parser->input_file ),
	  data, print_flags, spool_lines );
}

void PAR::init_output_stream
	( min::ptr<PAR::parser> parser,
	  std::ostream & out )
{
    init ( parser );

    min::init_output_stream
        ( min::locatable ( parser, parser->printer ),
	  out );
}

// Make an expression consisting of the tokens beginning
// with `first' and ending just before `next'.  Replace
// these tokens by the resulting EXPRESSION token.  Add
// the given .initiator and .terminator if these are not
// min::MISSING(), but also allow for later addition of
// these and for later addition of a .separator.  The
// resulting token is next->previous.
//
// Any token in the expression being made that is not
// a SYMBOL or EXPRESSION is replaced by an expression
// whose sole element is the token string of the token
// and whose .initiator is # for a number and " for a
// quoted string.
//
static void compact
	( PAR::parser parser,
	  PAR::token first, PAR::token next,
	  min::gen initiator = min::MISSING(),
	  min::gen terminator = min::MISSING() )
{
    min::uns32 n = 0;
    for ( PAR::token t = first;
          t != next; t = t->next )
        ++ n;

    min::locatable_gen exp;
    exp = min::new_obj_gen ( 3, 12 + n );
    min::obj_vec_insptr expvp ( exp );
    for ( min::uns32 i = 0; i <= n; ++ i )
	min::attr_push(expvp) = min::MISSING();

    min::locatable_gen element;
    min::locatable_gen str;
    min::locatable_var<PAR::token> token;

    while ( n -- )
    {
        token = next->previous;
	if ( token->type != PAR::SYMBOL
	     &&
	     token->type != PAR::EXPRESSION )
	{
	    str = min::new_str_gen
		( token->string.begin_ptr(),
		  token->string->length );
	    element = min::new_obj_gen ( 1, 10 );
	    min::obj_vec_insptr elemvp ( element );

	    min::attr_push(elemvp) = str;

	    if (    token->type
	         == LEXSTD::quoted_string_t )
		str = ::doublequote;
	    else if ( token->type == LEXSTD::number_t )
		str = ::number_sign;
	    else if (    token->type
	              == LEXSTD::natural_number_t )
		str = ::number_sign;
	    else
		MIN_ABORT ( "unexpected token type" );

	    min::attr_insptr elemap ( elemvp ); 
	    min::locate ( elemap, ::initiator );
	    min::set ( elemap, str );
	}
	else
	    element = token->value;

	min::set_attr ( expvp, n, element );

	PAR::free
	    ( PAR::remove
	          ( min::locatable
		        ( parser, parser->first ),
		    token ) );
    }

    if (    initiator != min::MISSING()
         || terminator != min::MISSING() )
    {
	min::attr_insptr expap ( expvp ); 

	if ( initiator != min::MISSING() )
	{
	    min::locate ( expap, ::initiator );
	    min::set ( expap, initiator );
	}

	if ( terminator != min::MISSING() )
	{
	    min::locate ( expap, ::terminator );
	    min::set ( expap, terminator );
	}
    }

    token = PAR::new_token ( PAR::EXPRESSION );
    min::locatable ( token, token->value ) =
        (min::gen) exp;

    PAR::put_before ( parser->first, next, token );
}

// Remove n tokens from before `next', where n is the
// number of elements of `label' (== 1 if `label' is
// a symbol or number).
//
void remove ( PAR::parser parser,
              PAR::token next, min::gen label )
{
    min::uns32 n = 1;
    if ( min::is_lab ( label ) )
        n = min::lablen ( label );
    while ( n -- )
        PAR::remove
	    ( min::locatable
	        ( parser, parser->first ),
	      next->previous );
}


// Parse an explicit subexpression that begins with the
// `current' token (which is just after the opening
// bracket or indentation mark).  If more tokens are
// needed, call parser->input.  Upon return, `current'
// is the first token AFTER the parsed expression.
//
// If indented_paragraph is true, the expression was
// begun by an indentation mark.  The current token is
// the line-break token after the indentation mark (and
// this is deleted).  The next non-line-break token sets
// the indentation associated with the indentation mark.
//
// Otherwise, if indented_paragraph is false, the
// expression was begun by the opening bracket, and the
// current token is the first token after the opening
// bracket.
//
// The paragraph_indent argument defines the currently
// active indentation (which can be - parser->indent_
// offset if there is none).  This is the indent set
// by some indentation mark PREVIOUS to any that
// caused this call to parse_explicit_subexpression.
// If any token is seen with this indent or less, the
// current subexpression terminates just before that
// token.
//
// The closing_stack argument defines a stack of closing
// brackets which correspond to the currently active
// open brackets.  If any are recognized, the current
// subexpression terminates just after the recoginized
// bracket, and returns the recognized bracket in the
// closing_bracket argument (which is otherwise set to
// NULL_STUB).  To be recoginized, a closing bracket
// must be active according as per its selectors.  So
// its possible for a closing bracket not at the top of
// the stack to be missed because the selectors have
// been changed and it is not active.  If a closing
// bracket is returned, `current' is the first token
// AFTER the closing bracket.
//
// SUBSUBexpressions are converted to an EXPRESSION
// token whose value is a list.  The tokens beginning
// with the initial value of `current' can be edited.
// The caller should save `current->previous' before
// calling this function, so it and `current' as
// returned by this function can be used to delimit
// the subexpression.  Note that in the case of the
// top level call, there may be no `current->previous',
// and parser->first will be the first token of the
// subexpression.
//
// This function calls itself recursively if it finds
// an opening bracket or an indentation mark.  The
// selectors determine which bracket and indentation
// mark definitions are active.  When this function
// calls itself recursively, upon return it wraps all
// the tokens of the sub-subexpression found into a
// single EXPRESSION token, attaches opening brackets
// and indentation marks as .initiator's of that expres-
// sion, and any closing bracket as a .terminator of
// that expression.
//
// Line_break tokens are deleted.  Gluing indentation
// marks are split from line-ending tokens.  Bracket
// recognition preceeds token splitting and line_break
// deletion: so the last lexeme of a bracket cannot be
// the first part of a split token (this should not be
// a problem as the last lexeme of a bracket should be
// a separator), and multi-lexeme brackets cannot
// straddle line_breaks.
//
// This function is called at the top level with indent
// <= - parser->indent_offset and closing_bracket =
// NULL_STUB.
//
struct closing_stack
{
    TAB::closing_bracket closing_bracket;
    closing_stack * previous;
        // Stack is NULL terminated.
};
static void parse_explicit_subexpression
	( PAR::parser parser,
	  bool indented_paragraph,
	  PAR::token & current,
	  TAB::closing_bracket & closing_bracket,
	  ::closing_stack & closing_stack,
	  min::int32 paragraph_indent,
	  TAB::selectors selectors )
{
    closing_bracket = min::NULL_STUB;

    min::int32 line_indent =
        - parser->indent_offset;
    PAR::token line_first;
    while ( true )
    {
        // Truncate if end of file.
	//
	if ( current->type == LEXSTD::end_of_file_t )
	{
	    if ( line_indent >= 0
	         &&
		 current != line_first )
	    {
	        ::compact
		    ( parser, line_first, current );
		break;
	    }
	}

	// Delete line breaks.
	//
	else if ( current->type = LEXSTD::line_break_t )
	{
	    if ( current->next == parser->first )
	    {
		parser->input->add_tokens
		    ( parser, parser->input );
		assert
		    ( current->next != parser->first );
	    }

	    current = current->next;
	    remove
	        ( min::locatable
		    ( parser, parser->first ),
		  current->previous );
	    continue;
	}

	// Complain if token indent is too near
	// paragraph indent.
	//
	int near = (min::int32) current->begin.column
	         - paragraph_indent;
	if ( near < 0 ) near = - near;
	if (    near != 0
	     && near < parser->indent_offset )
	{
	    parser->printer
	        << min::bom << min::set_indent ( 7 )
		<< "ERROR: lexeme indent "
		<< current->begin.column
		<< " too near paragraph indent "
		<< paragraph_indent
		<< "; "
		<< LEX::pline_numbers
		       ( parser->input_file,
		         current->begin,
			 current->end )
	        << ":" << min::eom;
	    LEX::print_item_lines
		( parser->printer,
		  parser->input_file,
		  current->begin,
		  current->end );
	}

	// Complain if token indent is too near
	// line indent.
	//
	near = (min::int32) current->begin.column
	     - line_indent;
	if ( near < 0 ) near = - near;
	if (    near != 0
	     && near < parser->indent_offset )
	{
	    parser->printer
	        << min::bom << min::set_indent ( 7 )
		<< "ERROR: lexeme indent "
		<< current->begin.column
		<< " too near line indent "
		<< line_indent
		<< "; "
		<< LEX::pline_numbers
		       ( parser->input_file,
		         current->begin,
			 current->end )
	        << ":" << min::eom;
	    LEX::print_item_lines
		( parser->printer,
		  parser->input_file,
		  current->begin,
		  current->end );
	}

	// Truncate subexpression if token is at or
	// before line indent.
	//
	if (       (min::int32) current->begin.column
	        <= line_indent
	     && current != line_first )
	{
	    assert ( indented_paragraph );

	    ::compact ( parser, line_first, current );
	    line_first = current;
	}

	// Truncate subexpression if token is at or
	// before paragraph indent.
	//
	if (    (min::int32) current->begin.column
	     <= paragraph_indent )
	    break;

	// Set line_indent if appropriate.
	//
	if ( indented_paragraph && line_indent < 0 )
	{
	    line_indent = current->begin.column;
	    line_first = current;
	}

	// Lookup tokens in bracket table.
	//
	TAB::root root =
	    find_entry ( parser, current, selectors,
	                 parser->bracket_table );
	if ( root != min::NULL_STUB )
	{
	    min::uns32 subtype =
	        min::packed_subtype_of ( root );
	    if ( subtype == TAB::OPENING_BRACKET )
	    {
	        TAB::opening_bracket opening_bracket =
		    (TAB::opening_bracket) root;

		TAB::selectors new_selectors =
		    selectors;
		new_selectors |=
		    opening_bracket->new_selectors
		                    .or_selectors;
		new_selectors &= ~
		    opening_bracket->new_selectors
		                    .not_selectors;
		new_selectors ^=
		    opening_bracket->new_selectors
		                    .xor_selectors;

		::closing_stack cstack;
		cstack.previous = & closing_stack;
		cstack.closing_bracket =
		    opening_bracket->closing_bracket;

		PAR::token previous = current->previous;
		::parse_explicit_subexpression
		    ( parser, false, current,
		      closing_bracket, cstack,
		      line_indent >= 0 ?
		          line_indent :
			  paragraph_indent,
		      new_selectors );
		PAR::token first = previous->next;

		::remove ( parser, first,
		           opening_bracket->label );
		if (    opening_bracket->closing_bracket
		     != closing_bracket )
		{
		    // TBD
		}
		else
		    ::remove ( parser, current,
		               closing_bracket->label );
		::compact ( parser, first, current,
			    opening_bracket->label,
			    closing_bracket->label );
	    }
	    else if ( subtype == TAB::CLOSING_BRACKET )
	    {
		// TBD
	    }
	    else if ( subtype == TAB::INDENTATION_MARK )
	    {
		// TBD
	    }
	}
	else
	{
	    // Move to next token.
	    //
	    if ( current->next == parser->first )
	    {
		parser->input->add_tokens
		    ( parser, parser->input );
		assert
		    ( current->next != parser->first );
	    }
	    current = current->next;
	}
    }
}


void PAR::parse ( PAR::parser parser )
{
    if ( parser->scanner != NULL_STUB )
    {
        LEX::scanner scanner = parser->scanner;
        if ( parser->input_file != scanner->input_file )
	{
	    if ( parser->input_file == NULL_STUB )
	        min::locatable
		    ( parser, parser->input_file ) =
			scanner->input_file;
	    else if ( scanner->input_file == NULL_STUB )
	        min::locatable
		    ( scanner, scanner->input_file ) =
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
	        min::locatable
		    ( parser, parser->printer ) =
			scanner->printer;
	    else if ( scanner->printer == NULL_STUB )
	        min::locatable
		    ( scanner, scanner->printer ) =
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

    while ( ! parser->eof )
        parser->input->add_tokens
	    ( parser, parser->input );
}

// Parser Functions
// ------ ---------

TAB::key_prefix PAR::find_key_prefix
	( PAR::parser parser,
	  PAR::token & current,
	  TAB::table table )
{
    uns32 phash = min::labhash_initial;
    uns32 table_len = table->length;
    uns32 mask = table_len - 1;
    MIN_ASSERT ( ( table_len & mask ) == 0 );
    TAB::key_prefix previous = NULL_STUB;
    while ( true )
    {
        if ( current->type != SYMBOL ) break;

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
	    if ( parser->eof ) break;

	    parser->input->add_tokens
		( parser, parser->input);
	    if ( current->next == parser->first ) break;
	}

	current = current->next;
    }

    return previous;
}

TAB::root PAR::find_entry
	( PAR::parser parser,
	  PAR::token & current,
	  TAB::selectors selectors,
	  TAB::table table )
{
    for ( TAB::key_prefix kprefix =
              find_key_prefix
	          ( parser, current, table );
          kprefix != NULL_STUB;
	  kprefix = kprefix->previous,
	  current = current->previous )
    for ( TAB::root root = kprefix->first;
	  root != NULL_STUB;
	  root = root->next )
    {
	if ( root->selectors & selectors )
	    return root;
    }
    return NULL_STUB;
}
