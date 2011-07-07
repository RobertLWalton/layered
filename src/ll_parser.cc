// Layers Language Parser Functions
//
// File:	ll__parser.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Thu Jul  7 07:33:45 EDT 2011
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
# define MUP min::unprotected
# define LEX ll::lexeme
# define LEXSTD ll::lexeme::standard
# define PAR ll::parser
# define TAB ll::parser::table

static min::locatable_gen initiator;
static min::locatable_gen terminator;
static min::locatable_gen separator;
static min::locatable_gen named_op;
static min::locatable_gen arguments;
static min::locatable_gen keys;
static min::locatable_gen doublequote;
static min::locatable_gen number_sign;
static min::locatable_gen new_line;

static min::printer_format bracket_format =
    min::default_printer_format;

static struct initializer {
    initializer ( void )
    {
        ::initiator =
	    min::new_str_gen ( ".initiator" );
        ::terminator =
	    min::new_str_gen ( ".terminator" );
        ::separator =
	    min::new_str_gen ( ".separator" );
        ::named_op =
	    min::new_str_gen ( ".named_op" );
        ::arguments =
	    min::new_str_gen ( ".arguments" );
        ::keys =
	    min::new_str_gen ( ".keys" );
        ::doublequote =
	    min::new_str_gen ( "\"" );
        ::number_sign =
	    min::new_str_gen ( "#" );
        ::new_line =
	    min::new_str_gen ( "\n" );

	::bracket_format.str_prefix = "";
	::bracket_format.str_postfix = "";
	::bracket_format.lab_prefix = "";
	::bracket_format.lab_postfix = "";
	::bracket_format.lab_separator = "";
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
    next_ref(str) = min::NULL_STUB;
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
    next_ref(str) = ::free_strings;
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
    value_ref(token) = min::MISSING();
    string_ref(token) = min::NULL_STUB;
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
        next_ref(pass) = parser->pass_stack;
	pass_stack_ref(parser) = pass;
    }
    else
    {
        PAR::pass current = parser->pass_stack;
	for ( ; current != NULL_STUB;
	        current = current->next )
	{
	    if ( current == previous )
	    {
	        next_ref(pass) = current->next;
		next_ref(current) = pass;
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
	  const char * data,
	  min::uns32 print_flags,
	  min::uns32 spool_lines )
{
    init ( parser );

    min::init_input_string
        ( input_file_ref(parser),
	  data, print_flags, spool_lines );
}

void PAR::init_output_stream
	( min::ref<PAR::parser> parser,
	  std::ostream & out )
{
    init ( parser );

    min::init_output_stream
        ( printer_ref(parser),
	  out );
}

// Convert a non-natural number or quoted string token
// to an EXPRESSION token.  The expression has as its
// only element a min::gen string value equal to the
// translation string of the token's lexeme, and has as
// its .initiator either # for a non-natural number or
// " for a quoted string.
//
void convert_token ( PAR::token token )
{
    assert ( token->value == min::MISSING() );

    min::gen indicator;

    if (    token->type
	 == LEXSTD::quoted_string_t )
	indicator = ::doublequote;
    else
    {
	assert (    token->type
		 == LEXSTD::number_t );

	indicator = ::number_sign;
    }


    PAR::value_ref(token)
	= min::new_obj_gen ( 10, 1 );
    min::obj_vec_insptr elemvp
	( token->value );
    min::attr_push(elemvp) = min::MISSING();

    min::set_attr ( elemvp, 0,
                    min::new_str_gen
			( token->string.begin_ptr(),
			  token->string->length ) );
    PAR::string_ref(token) =
	PAR::free_string ( token->string );

    min::attr_insptr elemap ( elemvp ); 
    min::locate ( elemap, ::initiator );
    min::set ( elemap, indicator );

    token->type = PAR::EXPRESSION;
}

// Make an expression consisting of the tokens beginning
// with `first' and ending just before `next'.  Replace
// these tokens by the resulting EXPRESSION token.  Add
// the given .initiator and .terminator if these are not
// min::MISSING(), but also allow for later addition of
// these and for later addition of a .separator.  Set
// the begin and end positions of the new token from the
// given arguments.  The resulting token is next->
// previous.
//
// Any token in the expression being output that has a
// MISSING token value must be a non-natural number or
// quoted string.  These are replaced by a subexpression
// whose sole element is the token string of the token
// as a string general value and whose .initiator is #
// for a number or " for a quoted string.
//
static void compact
	( PAR::parser parser,
	  PAR::token first, PAR::token next,
	  LEX::position begin,
	  LEX::position end,
	  min::gen initiator = min::MISSING(),
	  min::gen terminator = min::MISSING() )
{
    // Temporary min::gen locatable.
    //
    min::locatable_gen exp;

    // Count tokens.  Also replace non-natural numbers
    // and quoted strings by subexpressions.
    //
    min::uns32 n = 0;
    for ( PAR::token current = first;
          current != next;
	  ++ n, current = current->next )
    {
	if ( current->value == min::MISSING() )
	    ::convert_token ( current );
    }

    exp = min::new_obj_gen ( 12 + n, 3 );
    min::obj_vec_insptr expvp ( exp );
    for ( min::uns32 i = 0; i < n; ++ i )
	min::attr_push(expvp) = min::MISSING();

    while ( n -- )
    {
        PAR::token t = next->previous;
	assert ( t != next );

	min::set_attr ( expvp, n, t->value );

	PAR::free
	    ( PAR::remove
	          ( PAR::first_ref(parser), t ) );
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

    PAR::token token =
        PAR::new_token ( PAR::EXPRESSION );
    PAR::value_ref(token) = exp;
    token->begin = begin;
    token->end = end;

    PAR::put_before ( first_ref(parser), next, token );

    if (   parser->trace
         & PAR::TRACE_EXPLICIT_SUBEXPRESSIONS )
    {
	    parser->printer
	        << "EXPRESSION: "
		<< min::pgen ( token->value )
		<< ": "
		<< LEX::pline_numbers
		        ( parser->input_file,
			  begin, end )
		<< ":" << min::eol;
	    LEX::print_item_lines
		( parser->printer,
		  parser->input_file, begin, end );
    }
}

// In a token sequence, find the next token subsequence
// that matches a given separator, or find the end of
// the token sequence.  The separator is represented as
// a vector of n min::gen values, where n == 0 if the
// separator is missing.  `first' is the first token of
// the token sequence and `next' is the next token AFTER
// the token sequence.
//
// The count of the number of tokens skipped is also
// returned.
//
static PAR::token find_separator
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

// Given a token sequence with n tokens, return a label
// whose elements are the values of the tokens.  If
// there are 0 tokens, return min::MISSING().  If there
// is just one, return its value.  Otherwise return a
// min::gen label.
//
// If any of the tokens are quoted strings or non-
// natural numbers, convert then to have values equal
// to their their strings and free their strings.
//
static min::gen make_label
	( PAR::token first, min::uns32 n )
{
    if ( n == 0 ) return min::MISSING();

    min::gen label[n];

    for ( min::uns32 i = 0; i < n;
          ++ i, first = first->next )
    {
        if ( first->value == min::MISSING() )
	{
	    PAR::value_ref(first) =
		min::new_str_gen
		    ( first->string.begin_ptr(),
		      first->string->length );
	    PAR::string_ref(first) =
	        free_string ( first->string );
	}

	label[i] = first->value;
    }
    if ( n == 1 ) return label[0];
    else return min::new_lab_gen ( label, n );
}

// Skip n tokens.
//
inline PAR::token skip ( PAR::token t, min::uns32 n )
{
    while ( n -- ) t = t->next;
    return t;
}

// Compute attributes from a named bracket or named
// operator.  The `name', which is a label, is computed.
// If there are arguments, an `arguments' list is
// computed (otherwise it is set to MISSING).  If there
// are keys, a `keys' list is computed.
//
// The named bracket or named operator is defined by
// `first' and `next'.  `first' is the first token AFTER
// the opening, and `next' is the first token of the
// middle (for a named bracket) or closing (for a named
// operator).  The entire named opening bracket or
// named operator is deleted.
// 
// If there are no keys, `keys' is set to MISSING.  If
// there is must one, `keys' is set to that key, which
// is a label.  If there is more than one, `keys' is
// set to a list of labels.
//
// Any token in the named bracket or operator that has a
// MISSING token value must be a non-natural number or
// quoted string.  For arguments, these are replaced by
// a subexpression whose sole element is the token
// string of the token as a string general value and
// whose .initiator is # for a number or " for a quoted
// string.  For keys, these are replaced by ordinary
// min::str strings and become label elements.
//
static void named_attributes
	( PAR::parser parser,
	  min::ref<min::gen> name,
	  min::ref<min::gen> arguments,
	  min::ref<min::gen> keys,
	  TAB::named_opening named_opening,
	  PAR::token first, PAR::token next )
{
    // Temporary min::gen locatables.
    //
    min::locatable_gen exp;

    // Recase named_separator as a vector of min::gen
    // elements.
    //
    TAB::named_separator nsep =
        named_opening->named_separator;
    min::gen sep =
	( nsep == min::NULL_STUB ? min::MISSING()
	                         : nsep->label );
    min::uns32 seplen =
        sep == min::MISSING() ? 0 :
	min::is_lab ( sep ) ? min::lablen ( sep ) :
	                      1;
    min::gen separator[seplen];
    if ( seplen == 1 ) separator[0] = sep;
    else if ( seplen > 1 )
        min::lab_of ( separator, seplen, sep );

    PAR::token t = first;
    min::uns32 n = 0;

    // Count elements of name.
    //
    while ( t != next
            &&
	    ( t->type == LEXSTD::word_t
              ||
	      t->type == LEXSTD::natural_number_t ) )
    {
        t = t->next;
	++ n;
    }

    // Construct name label.
    //
    assert ( n > 0 );
    name = ::make_label ( first, n );

    // Count arguments.
    //
    min::uns32 argcount;
    PAR::token tnext = ::find_separator
	( argcount, t, next, separator, seplen );

    // Make argument list.  Convert any quoted string
    // or (non-natural) numbers to subexpressions.
    //
    if ( argcount == 0 )
        arguments = min::MISSING();
    else
    {
        arguments = min::new_obj_gen ( argcount );
	min::obj_vec_insptr argp ( arguments );

	for ( ; t != tnext; t = t->next )
	{
	    if ( t->value == min::MISSING() )
	        ::convert_token ( t );
	    min::attr_push(argp) = t->value;
	}
    }

    t = tnext;

    // Count the number of separators.
    //
    min::uns32 sepcount = 0;
    min::uns32 keycount;

    while ( tnext != next )
    {
        tnext = ::skip ( tnext, seplen );
        tnext = ::find_separator
	    ( keycount, tnext, next,
	      separator, seplen );
	++ sepcount;
    }

    if ( sepcount == 0 )
        keys = min::MISSING();
    else if ( sepcount == 0 )
        keys = ::make_label ( t, keycount );
    else
    {
        keys = min::new_obj_gen ( sepcount );
	min::obj_vec_insptr keysp ( keys );
	while ( t != next )
	{
	    t = ::skip ( t, seplen );
	    tnext = ::find_separator
		( keycount, t, next,
		  separator, seplen );
	    exp = ::make_label ( t, keycount );
	    min::attr_push(keysp) = exp;
	    t = tnext;
	}
    }
}

// Remove n tokens from before `next', where n is the
// number of elements of `label' (== 1 if `label' is
// a symbol or number).  Return the begin position of
// the last token removed.  Free the removed tokens.
//
static LEX::position remove
        ( PAR::parser parser,
	  PAR::token next, min::gen label )
{
    LEX::position result;
    min::uns32 n = 1;
    if ( min::is_lab ( label ) )
        n = min::lablen ( label );
    while ( n -- )
    {
        result = next->previous->begin;
        PAR::free
	    ( PAR::remove
		  ( PAR::first_ref(parser),
		    next->previous ) );
    }
    return result;
}

inline void remove
	( PAR::parser parser,
	  PAR::token first,
	  PAR::token next )
{
    while ( first != next )
    {
	first = first->next;
	free
	  ( remove
	      ( first_ref(parser),
		first->previous )
	  );
    }
}

// Return the n'th token before `next', where n is the
// number of elements of `label' (== 1 if `label' is
// a symbol or number).

static PAR::token backup
        ( PAR::token next, min::gen label )
{
    min::uns32 n = 1;
    if ( min::is_lab ( label ) )
        n = min::lablen ( label );
    while ( n -- ) next = next->previous;
    return next;
}


// Parse an explicit subexpression beginning with the
// `current' token and calling parser->input if more
// tokens are needed.
//
// The parsed subexpression is NOT compacted; its end is
// identified and its SUBSUBexpressions are compacted.
// Line breaks are deleted, and after line break
// deletion, consecutive quoted strings are merged.
//
// It is assumed that there are always more tokens
// available until an end-of-file token is encountered,
// and the end-of-file is not part of the explicit
// subexpression.
//
// If indented_paragraph is true, the subexpression was
// begun by an indentation mark.  The `current' token is
// the line-break token after the indentation mark (and
// this will be deleted).  The next non-line-break token
// sets the indentation associated with the indentation
// mark.
//
// Otherwise, if indented_paragraph is false, the
// subexpression was begun by an opening bracket, and
// the `current' token is the first token after the
// opening bracket.
//
// In either case the selectors are those already
// computed by using the indentation mark or opening
// bracket.
//
// The paragraph_indent argument defines the currently
// active indentation (which can be - parser->indent_
// offset if there is none).  This is the indent set
// by some indentation mark PREVIOUS to any that
// caused this call to parse_explicit_subexpression.
// If any token is seen with this indent or less, the
// current subexpression terminates just before the
// line breaks before that token.
//
// The `bracket_stack' argument defines a stack of
// brackets that need to be closed.  When an entry in
// this stack is made, the entry is considered to be
// `open'.  When a closing bracket corresponding to
// one of these entries is recognized, that entry, and
// possibly other entries are marked as being `closed'.
// If there are no errors in the input, the found clos-
// ing bracket will correspond to the top entry of
// the stack, and only that entry will be closed.  But
// But if a closing bracket for a non-top stack entry
// is found, and then entries between it and the top of
// the stack are also marked closed, but in a manner
// that indicates their closing brackets were missing
// from the input.
//
// Recognition of a closing bracket that corresponds
// to an entry in the bracket stack causes this func-
// tion to terminate just after the recognized bracket.
// To be recognized, a closing bracket must be active as
// per the selectors.  So it is possible for a closing
// bracket to be missed because the selectors have been
// changed and it is not active, though this will only
// happen if there is some other missing bracket or
// there has been an error in the way selectors have
// been defined for the brackets.
//
// If a closing bracket is found to be missing from the
// input, an error message is produced, and the output
// of this function is computed as if the missing clos-
// ing bracket were not missing, that is, as if the mis-
// sing closing bracket were inserted in the input just
// before the point in the input where the absence of
// the missing closing bracket was first recognized.
// This point is just before another closing bracket or
// is at a newline which is followed by an insufficient-
// ly indented token.
//
// The end of the subexpression is identified by the
// `current' token upon return by this function, and
// also by the number of bracket_stack entries that have
// been marked as closed.  If NO bracket_stack entries
// have been marked closed, then `current' is the first
// token AFTER the subexpression, and is either a line
// break or an end of file.  In this case, if there are
// several line breaks in a row after the subexpression,
// all but the first will be deleted.
//
// If a bracket_stack entry has been marked closed, then
// `current' will be the first token AFTER the closing
// bracket whose recognition terminated the subexpres-
// sion.
//
// Note that if any bracket_stack entry is marked as
// closed, then the top bracket_stack entry will be
// marked as closed.
//
// SUBSUBexpressions are converted to an EXPRESSION
// token whose value is a list.  The tokens beginning
// with the initial value of `current' can be edited.
// The caller should save `current->previous', which
// is an indentation mark or the last token in an
// opening bracket, before calling this function, so it
// and `current' as returned by this function can be
// used to delimit the subexpression.  Note that in the
// case of the top level call, there may be no
// `current->previous', and parser->first will be the
// first token of the subexpression.
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
// that expression.  It also replaces non-natural num-
// bers and quoted strings in the sub-subexpression
// by subexpressions whose sole elements are the
// strings of the tokens and whose .initiators are
// # for number and " for quoted string.
//
// This function also compacts lines in an indented
// paragraph into EXPRESSION's.  These are given the
// .terminator "\n".
//
// Line_break tokens are deleted.  Gluing indentation
// marks are split from line-ending tokens.  Bracket
// recognition preceeds token splitting and line_break
// deletion: so the last lexeme of a bracket cannot be
// the first part of a split token (this should not be
// a problem as the last lexeme of a closing bracket
// should be a separator), and multi-lexeme brackets
// and indentation marks cannot straddle line_breaks.
//
// This function is called at the top level with indent
// <= - parser->indent_offset and bracket_stack == NULL.
//
struct bracket_stack
{
    TAB::opening_bracket opening_bracket;
        // If not NULL_STUB, this identifies the opening
	// bracket whose recognition made this entry.
    TAB::named_opening named_opening;
        // If not NULL_STUB, this identifies the named
	// opening bracket whose recognition made this
	// entry.

    PAR::token opening_first, opening_next;
        // For named_openings only, the first token
	// AFTER the named opening and the first token
	// OF the named middle ; thus bounding the named
	// bracket name, arguments, and keys.

    PAR::token closing_first, closing_next;
        // If these are NULL_STUB, this entry is open.
	// Otherwise if first != next, they are the
	// first token of the closing bracket that
	// closed this entry, and the next token AFTER
	// this bracket, but if first == next, the
	// closing bracket that terminated this entry
	// was missing and should be inserted just
	// before next.

    bracket_stack * previous;
        // Stack is NULL terminated.

    bracket_stack ( bracket_stack * previous )
        : opening_bracket ( min::NULL_STUB ),
          named_opening ( min::NULL_STUB ),
          opening_first ( min::NULL_STUB ),
          opening_next ( min::NULL_STUB ),
          closing_first ( min::NULL_STUB ),
          closing_next ( min::NULL_STUB ),
	  previous ( previous ) {}
};
static void parse_explicit_subexpression
	( PAR::parser parser,
	  bool indented_paragraph,
	  PAR::token & current,
	  ::bracket_stack * bracket_stack_p,
	  min::int32 paragraph_indent,
	  TAB::selectors selectors )
{
    // Bracketted subexpressions must be inside a line
    // in an indented paragraph.
    //
    assert (    ! indented_paragraph
             || bracket_stack_p == NULL );

    min::int32 line_indent =
        - parser->indent_offset;
    PAR::token line_first;
    TAB::indentation_mark indentation_mark =
        min::NULL_STUB;
	// If not NULL_STUB, last token was this
	// indentation mark.
    TAB::named_opening named_opening =
        min::NULL_STUB;
	// If not NULL_STUB, a named opening bracket
	// or named closing bracket with this table
	// entry is being scanned.
    bool is_named_opening_bracket;
        // If named_opening != NULL_STUB, this is true
	// if a named opening bracket is being scanned,
	// and false if a named closing bracket is being
	// scanned.
    PAR::token named_first = min::NULL_STUB;
        // If named_opening != NULL_STUB, this is the
	// first token after the named opening or named
	// middle.

    while ( true )
    {
        // Truncate if end of file.
	//
	if ( current->type == LEXSTD::end_of_file_t )
	    break;

	// Ensure there is a next token.
	//
	if ( current->next == parser->first )
	{
	    parser->input->add_tokens
		( parser, parser->input );
	    assert
		( current->next != parser->first );
	}

	// Process line breaks.
	//
	if ( current->type == LEXSTD::line_break_t )
	{
	    // Look at last token and see if we can
	    // split an indentation mark from it.
	    //
	    if ( current != parser->first
	         &&
		    current->previous->type
		 == LEXSTD::mark_t
		 &&
		 indentation_mark == min::NULL_STUB )
	    {
	        min::str_ptr sp
		    ( current->previous->value );
		min::uns32 length = min::strlen ( sp );
		TAB::indentation_split split =
		    min::NULL_STUB;
		if ( length != 0 )
		{
		    min::uns8 lastc =
			(min::uns8) sp[length-1];
		    split = parser->split_table[lastc];
		    for ( ; split != min::NULL_STUB;
		            split = split->next )
		    {
			if ( ( selectors
			       &
			       split->indentation_mark
			             ->selectors )
			     == 0 )
			    continue;
		        if ( split->length >= length )
			    continue;
			if ( memcmp
			       ( & sp[  length
			              - split->length],
			         & split[0],
				 split->length )
			     == 0 ) break;
		    }
		    if ( split != min::NULL_STUB )
		    {
			indentation_mark =
			    split->indentation_mark;

		        PAR::value_ref
			    (current->previous) =
			    min::new_str_gen
			        ( & sp[0],
				    length
				  - split->length );
			PAR::put_before
			    ( PAR::first_ref(parser),
			      current,
			      PAR::new_token
			          ( LEXSTD::mark_t ) );
			PAR::value_ref
			    (current->previous) =
			    indentation_mark->label;

			// Compute the number of columns
			// in the indentation mark.
			// Assume this is independent of
			// position in line (e.g., there
			// are no tabs in the indenta-
			// tion mark).
			//
			min::uns32 columns = 0;
			min::uns32 flags =
			    parser->input_file
			          ->print_flags;
			const char * p =
			    (const char *) & split[0];
			while ( * p )
			{
			    min::uns32 c =
			        min::utf8_to_unicode
				    ( p );
			    min::pwidth ( columns,
			                  c,
					  flags );
			}

			// Fix up the positions in the
			// tokens.  The indentation
			// mark has split->length bytes
			// and `columns' columns.
			//
			current->previous->end =
			    current->previous
			           ->previous->end;
			current->previous->begin =
			    current->previous->end;
			current->previous->begin.index
			    -= split->length;
			current->previous->begin.column
			    -= columns;
			current->previous->previous->end
			    = current->previous->begin;
		    }
		}

	    }

	    if ( indentation_mark != min::NULL_STUB )
	    {

		TAB::selectors new_selectors =
		    selectors;
		new_selectors |=
		    indentation_mark->new_selectors
		                     .or_selectors;
		new_selectors &= ~
		    indentation_mark->new_selectors
		                     .not_selectors;
		new_selectors ^=
		    indentation_mark->new_selectors
		                     .xor_selectors;

		PAR::token previous = current->previous;
		::parse_explicit_subexpression
		    ( parser, true, current,
		      NULL,
		      line_indent >= 0 ?
		          line_indent :
			  paragraph_indent,
		      new_selectors );

		PAR::token first = previous->next;
		LEX::position end =
		    current->previous->end;
		LEX::position begin =
		    ::remove
			( parser, first,
			  indentation_mark->label );

		::compact ( parser, first, current,
			    begin, end,
			    indentation_mark->label,
			    min::MISSING() );

		indentation_mark = min::NULL_STUB;

		continue;
	    }

	    // Get next non-line-break token and remove
	    // any line breaks that follow the current
	    // line break.
	    //
	    while ( true )
	    {
		if (    current->next->type
		     != LEXSTD::line_break_t )
		    break;

		free ( remove ( first_ref(parser),
		                current->next ) );

		if ( current->next == parser->first )
		{
		    parser->input->add_tokens
			( parser, parser->input );
		    assert (    current->next
		             != parser->first );
		}
	    }

	    PAR::token next = current->next;

	    // Truncate expression if line break is
	    // followed by an end of file.
	    //
	    if ( next->type == LEXSTD::end_of_file_t )
		break;

	    // Now next is neither a line break or end
	    // of file.

	    // Complain if next token indent is too near
	    // paragraph indent.
	    //
	    int near = (min::int32)
	    	       next->begin.column
		     - paragraph_indent;
	    if ( near < 0 ) near = - near;
	    if (    near != 0
		 && near < parser->indent_offset )
	    {
		parser->printer
		    << min::bom << min::set_indent ( 7 )
		    << "ERROR: lexeme indent "
		    << next->begin.column
		    << " too near paragraph indent "
		    << paragraph_indent
		    << "; "
		    << LEX::pline_numbers
			   ( parser->input_file,
			     next->begin,
			     next->end )
		    << ":" << min::eom;
		LEX::print_item_lines
		    ( parser->printer,
		      parser->input_file,
		      next->begin,
		      next->end );
	    }

	    // Truncate subexpression if next token
	    // indent is at or before paragraph indent.
	    //
	    if (    (min::int32) next->begin.column
		 <= paragraph_indent )
		break;

	    // Set line_indent if appropriate.
	    //
	    if ( indented_paragraph && line_indent < 0 )
	    {
		line_indent = next->begin.column;
		line_first = next;
	    }
	    else if ( line_indent >= 0 )
	    {
		// Complain if next token indent is too
		// near line indent.
		//
		near = (min::int32) next->begin.column
		     - line_indent;
		if ( near < 0 ) near = - near;
		if (    near != 0
		     && near < parser->indent_offset )
		{
		    parser->printer
			<< min::bom
			<< min::set_indent ( 7 )
			<< "ERROR: lexeme indent "
			<< next->begin.column
			<< " too near line indent "
			<< line_indent
			<< "; "
			<< LEX::pline_numbers
			       ( parser->input_file,
				 next->begin,
				 next->end )
			<< ":" << min::eom;
		    LEX::print_item_lines
			( parser->printer,
			  parser->input_file,
			  next->begin,
			  next->end );
		}

		// Truncate line if next token indent is
		// at or before line indent.
		//
		if (    (min::int32) next->begin.column
		     <= line_indent )
		{
		    assert ( indented_paragraph );

		    ::compact ( parser,
		                line_first, current,
				line_first->begin,
				current->previous->end,
				min::MISSING(),
				::new_line );
		    line_first = next;
		}
	    }

	    // Remove line break and move to next token.
	    //
	    free ( remove ( first_ref(parser),
		            current ) );
	    current = next;
	    continue;
	}

	indentation_mark = min::NULL_STUB;

	// Process tokens that are not separators or
	// marks.
	//
	if ( current->type != LEXSTD::separator_t
	     &&
	     current->type != LEXSTD::mark_t )
	{
	    if ( named_opening != min::NULL_STUB
	         &&
		 !  is_named_opening_bracket
		 && ( current->type == LEXSTD::number_t
		      ||
		         current->type
		      == LEXSTD::quoted_string_t
		    ) )
		named_opening = min::NULL_STUB;

	    if (    current->type
		 == LEXSTD::quoted_string_t
		 &&
	         current != parser->first
	         &&
		    current->previous->type
		 == LEXSTD::quoted_string_t )
	    {
	        min::push
		    ( (PAR::string_insptr)
		          current->previous->string,
		      current->string->length,
		      current->string + 0 );
		current->previous->end =
		    current->end;
		current = current->next;
		free ( remove ( first_ref(parser),
				current->previous ) );
	    }
	    else
		current = current->next;

	    continue;
	}

	assert ( current->type != PAR::EXPRESSION );

	// If not number or quoted string, look for
	// bracket or indentation mark.
	//
	PAR::token saved_current = current;
	TAB::key_prefix key_prefix;
	TAB::root root =
	    find_entry ( parser, current, key_prefix,
			 selectors,
			 parser->bracket_table );

	while ( true )
	{
	    if ( root == min::NULL_STUB )
	    {
		named_opening = min::NULL_STUB;
		current = saved_current->next;
		break;
	    }

	    min::uns32 subtype =
		min::packed_subtype_of ( root );

	    if ( subtype == TAB::OPENING_BRACKET )
	    {
	        if ( named_opening != min::NULL_STUB
		     &&
		     !  is_named_opening_bracket )
		    named_opening = min::NULL_STUB;

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

		::bracket_stack cstack
		    ( bracket_stack_p );
		cstack.opening_bracket =
		    opening_bracket;

		PAR::token previous = current->previous;
		::parse_explicit_subexpression
		    ( parser, false, current,
		      & cstack,
		      line_indent >= 0 ?
			  line_indent :
			  paragraph_indent,
		      new_selectors );
		PAR::token first = previous->next;

		if (    cstack.closing_next
		     == cstack.closing_first )
		{
		    // Found a closing bracket that is
		    // not ours, or found a line break
		    // or end of file that terminates a
		    // paragraph with the closing
		    // bracket missing.

		    // Compute location `next' just
		    // before which closing bracket
		    // should be inserted.
		    //
		    PAR::token next =
			(    cstack.closing_next
			  == min::NULL_STUB ?
			  current :
			  cstack.closing_next );

		    parser->printer
			<< min::bom
			<< min::set_indent ( 7 )
			<< "ERROR: missing"
			   " closing bracket `"
			<< min::pgen
			     ( opening_bracket->
			       closing_bracket->
				   label,
			       & ::bracket_format )
			<< "' inserted; "
			<< LEX::pline_numbers
			       ( parser->input_file,
				 next->begin,
				 next->end )
			<< ":" << min::eom;
		    LEX::print_item_lines
			( parser->printer,
			  parser->input_file,
			  next->begin,
			  next->end );


		    LEX::position end =
			next->previous->end;
		    LEX::position begin =
			::remove
			    ( parser, first,
			      opening_bracket->label );
		    ::compact
			( parser, first, next,
			  begin, end,
			  opening_bracket->label,
			  opening_bracket->
			      closing_bracket->
				  label );

		    if (    cstack.closing_next
			 == min::NULL_STUB )
		    {
			// Found a line break before
			// non-indented line or an end
			// of file when a closing
			// bracket was expected.  Go
			// to appropriate code above
			// to process.
			//
			break;
		    }

		    // Found a closing bracket that is
		    // not ours.  It must be in the
		    // bracket_stack and so needs to
		    // be kicked to our caller.
		    //
		    goto DONE;
		}
		else
		{
		    assert (    cstack.closing_next
		             == current );

		    LEX::position end =
			current->previous->end;
		    ::remove ( parser, current,
			       cstack.opening_bracket
			           ->closing_bracket
				   ->label );
		    LEX::position begin =
			::remove
			    ( parser, first,
			      opening_bracket->label );
		    ::compact ( parser, first, current,
				begin, end,
				opening_bracket->label,
				opening_bracket->
				    closing_bracket->
					label );
		    break;
		}
	    }

	    else if ( subtype == TAB::CLOSING_BRACKET )
	    {
		TAB::closing_bracket closing_bracket =
		    (TAB::closing_bracket) root;

		for ( ::bracket_stack * p =
			  bracket_stack_p;
		      p != NULL;
		      p = p->previous )
		{
		    if (    p->opening_bracket
		         != min::NULL_STUB
			 &&
		            p->opening_bracket
		             ->closing_bracket
			 == closing_bracket )
		    {
		        p->closing_first =
			    saved_current;
			p->closing_next = current;

			for ( ::bracket_stack * q =
				  bracket_stack_p;
			      q != p;
			      q = q->previous )
			    q->closing_first =
			        q->closing_next =
				    saved_current;

			goto DONE;
		    }
		}

		LEX::position end =
		    current->previous->end;
		LEX::position begin =
		    ::remove
			( parser, current,
			  closing_bracket->label );

		parser->printer
		    << min::bom
		    << min::set_indent ( 7 )
		    << "ERROR: spurious"
		       " closing bracket"
		       " found and ignored; "
		    << LEX::pline_numbers
			   ( parser->input_file,
			     begin, end )
		    << ":" << min::eom;
		LEX::print_item_lines
		    ( parser->printer,
		      parser->input_file,
		      begin, end );

		break;
	    }
	    else if ( subtype == TAB::INDENTATION_MARK )
	    {
		indentation_mark =
		    (TAB::indentation_mark) root;
		break;
	    }
	    else if ( subtype == TAB::NAMED_OPENING )
	    {
	        if ( current->type == LEXSTD::word_t
		     ||
		        current->type
		     == LEXSTD::natural_number_t )
		{
		    named_opening =
			(TAB::named_opening) root;
		    is_named_opening_bracket = true;
		    named_first = current;
		    break;
		}
	    }
	    else if (    subtype
	              == TAB::NAMED_SEPARATOR )
	    {
	        if ( named_opening != min::NULL_STUB
		     &&
		     is_named_opening_bracket
		     &&
		        (TAB::named_separator) root
		     == named_opening->named_separator )
		    break;
	    }
	    else if ( subtype == TAB::NAMED_MIDDLE )
	    {
	        TAB::named_middle named_middle =
		    (TAB::named_middle) root;

	        if ( named_opening != min::NULL_STUB
		     &&
		     is_named_opening_bracket
		     &&
		        named_middle
		     == named_opening->named_middle )
		{
		    ::bracket_stack cstack
			( bracket_stack_p );
		    cstack.named_opening =
			named_opening;
		    cstack.opening_first = named_first;
		    cstack.opening_next = saved_current;

		    PAR::token middle_last =
		        current->previous;
		    ::parse_explicit_subexpression
			( parser, false, current,
			  & cstack,
			  line_indent >= 0 ?
			      line_indent :
			      paragraph_indent,
			  selectors );

		    PAR::token next = current;
		        // Token just after last sub-
			// expression token AFTER any
			// named closing bracket has
			// been deleted.
		    bool done = false;
		        // Set if a bracket that was
			// not ours was found, so we
			// need to terminate this
			// call to parse_explicit_
			// subexpression.
		    LEX::position end;
		        // End of closing named bracket.

		    if (    cstack.closing_next
		         == cstack.closing_first )
		    {
			// Found a closing bracket that
			// is not ours, or found a line
			// break or end of file that
			// terminates a paragraph with
			// the closing bracket missing.

			// Compute location `next' just
			// before which closing bracket
			// should be inserted.
			//
			if (    cstack.closing_next
			     != min::NULL_STUB )
			    next = cstack.closing_next;

			end = next->previous->end;

			parser->printer
			    << min::bom
			    << min::set_indent ( 7 )
			    << "ERROR: missing named"
			       " closing bracket `"
			    << min::pgen
			       ( named_opening
			         ->named_middle_closing
				 ->label,
				 & ::bracket_format )
			    << "' inserted; "
			    << LEX::pline_numbers
				   ( parser->input_file,
				     next->begin,
				     next->end )
			    << ":" << min::eom;
			LEX::print_item_lines
			    ( parser->printer,
			      parser->input_file,
			      next->begin,
			      next->end );

			if (    cstack.closing_next
			     != min::NULL_STUB )
			    done = true;
		    }
		    else
		    {
		        end = current->previous->end;
		        ::remove
			    ( parser,
			      cstack.closing_first,
			      current );
		    }

		    min::locatable_gen name,
		    		       arguments,
				       keys;

		    ::named_attributes
			( parser,
			  name, arguments, keys,
			  named_opening,
			  cstack.opening_first,
			  cstack.opening_next );

		    assert ( name != min::MISSING() );

		    LEX::position begin =
		        ::remove
			    ( parser,
			      cstack.opening_first,
			      named_opening->label );
		    ::compact
		        ( parser,
			  middle_last->next,
			  current,
			  begin, end, name );
			  
		    assert
		        (    current->previous->type
			  == PAR::EXPRESSION );
		    assert
		        (    current->previous
			  == middle_last->next );

		    ::remove ( parser,
		               cstack.opening_first,
			       middle_last->next );

		    if ( arguments != min::MISSING()
		         ||
			 keys != min::MISSING() )
		    {
			min::obj_vec_insptr expvp
			    ( current->previous
			             ->value );
			min::attr_insptr expap
			    ( expvp );

			if (    arguments
			     != min::MISSING() )
			{
			    min::locate
				( expap, ::arguments );
			    min::set
				( expap, arguments );
			}

			if ( keys != min::MISSING() )
			{
			    min::locate
				( expap, ::keys );
			    min::set
				( expap, keys );
			}
		    }

		    if ( done ) goto DONE;
		    else	break;
		}
		else
		{
		    named_opening =
		        named_middle->named_opening;
		    is_named_opening_bracket = false;
		    named_first = current;
		    break;
		}
	    }
	    else if ( subtype == TAB::NAMED_CLOSING )
	    {
		TAB::named_closing named_closing =
		    (TAB::named_closing) root;

	        if ( named_opening != min::NULL_STUB
		     &&
		        named_closing
		     == named_opening->named_closing )
		{
		    if ( is_named_opening_bracket )
		    {

			min::locatable_gen name,
					   arguments,
					   keys;

			::named_attributes
			    ( parser,
			      name, arguments, keys,
			      named_opening,
			      named_first,
			      saved_current );

			assert
			    ( name != min::MISSING() );

			LEX::position begin =
			    ::remove
				( parser,
				  named_first,
				  named_opening->label );
			LEX::position end =
			    current->previous->end;

			::remove
			    ( parser,
			      named_first,
			      current );

			PAR::token t =
			    PAR::new_token
			        ( PAR::EXPRESSION );
			t->begin = begin;
			t->end = end;

			PAR::put_before
			    ( first_ref(parser),
			      current, t );
			value_ref(t) =
			    min::new_obj_gen
			        ( 12, 5 );

			min::obj_vec_insptr tvp
			    ( t->value );
			min::attr_insptr tap ( tvp );

			min::locate ( tap, ::named_op );
			min::set ( tap, name );

			if (    arguments
			     != min::MISSING() )
			{
			    min::locate
				( tap, ::arguments );
			    min::set
				( tap, arguments );
			}

			if ( keys != min::MISSING() )
			{
			    min::locate ( tap, ::keys );
			    min::set ( tap, keys );
			}

			break;
		    }

		    for ( ::bracket_stack * p =
			      bracket_stack_p;
			  p != NULL;
			  p = p->previous )
		    {
			if (    p->named_opening
			     == named_opening )
			{
			    PAR::token cp = named_first;
			    PAR::token op =
			        p->opening_first;
			    bool name_match = true;
			    while ( name_match )
			    {
			        if (    cp
				     == saved_current )
				    break;
				if (    op
				     == p->opening_next
				     ||
				        cp->type
				     != op->type
				     ||
				        cp->value
				     != op->value
				     ||
				        cp->value
				     == min::MISSING()
				   )
				{
				    name_match = false;
				    break;
				}
				cp = cp->next;
				op = op->next;
			    }
			    if ( ! name_match )
			        continue;

			    p->closing_first =
				saved_current;
			    p->closing_next = current;

			    for ( ::bracket_stack * q =
				      bracket_stack_p;
				  q != p;
				  q = q->previous )
				q->closing_first =
				    q->closing_next =
					saved_current;

			    goto DONE;
			}
		    }
		}
	    }
	    else if
	        ( subtype == TAB::NAMED_MIDDLE_CLOSING )
	    {
	        // TBD: if this is error and should be
		//      named closing bracket, repair

		TAB::named_middle_closing
		    named_middle_closing =
		    (TAB::named_middle_closing) root;

		for ( ::bracket_stack * p =
			  bracket_stack_p;
		      p != NULL;
		      p = p->previous )
		{
		    if (    p->named_opening
			 == named_opening )
		    {
		        p->closing_first =
			    saved_current;
			p->closing_next = current;

			for ( ::bracket_stack * q =
				  bracket_stack_p;
			      q != p;
			      q = q->previous )
			    q->closing_first =
			        q->closing_next =
				    saved_current;

			goto DONE;
		    }
		}

		LEX::position end =
		    current->previous->end;
		LEX::position begin =
		    ::remove
			( parser, current,
			  named_middle_closing->label );

		parser->printer
		    << min::bom
		    << min::set_indent ( 7 )
		    << "ERROR: spurious"
		       " middle closing bracket"
		       " found and ignored; "
		    << LEX::pline_numbers
			   ( parser->input_file,
			     begin, end )
		    << ":" << min::eom;
		LEX::print_item_lines
		    ( parser->printer,
		      parser->input_file,
		      begin, end );

		break;
	    }

	    root = PAR::find_next_entry
	               ( parser, current, key_prefix,
			 selectors, root );
	}
    }

DONE:

    // Compact any incomplete line.
    //
    if ( line_indent >= 0 )
	::compact
	    ( parser, line_first, current,
	      line_first->begin,
	      current->previous->end,
	      min::MISSING(),
	      ::new_line );
}

void PAR::parse ( PAR::parser parser )
{
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

    parser->input->add_tokens
	( parser, parser->input );

    PAR::token current = parser->first;
    assert ( current != NULL_STUB );
    parse_explicit_subexpression
	( parser, true, current,
	  NULL,
	  - parser->indent_offset,
	  parser->selectors );
    if ( current->type != LEXSTD::line_break_t )
    {
	parser->printer
	    << min::bom
	    << min::set_indent ( 7 )
	    << "ERROR: line break missing from end of"
	       " file; "
	    << LEX::pline_numbers
		   ( parser->input_file,
		     current->begin, current->end )
	    << ":" << min::eom;
	LEX::print_item_lines
	    ( parser->printer,
	      parser->input_file,
	      current->begin, current->end );
    }
    else
    {
        current = current->next;
	free ( remove ( first_ref(parser),
			current->previous ) );
    }

    assert ( current->type == LEXSTD::end_of_file_t );
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
        if ( current->value == min::MISSING() )
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
	  TAB::table table )
{
    for ( key_prefix = find_key_prefix
	                   ( parser, current, table );
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
	    if ( key_prefix == NULL_STUB )
		return NULL_STUB;
	    last_entry = key_prefix->first;
	}

	if ( last_entry->selectors & selectors )
	    return last_entry;
    }
}
