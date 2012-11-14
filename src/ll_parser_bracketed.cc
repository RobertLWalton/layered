// Layers Language Bracketed Subexpression Parser
//
// File:	ll_parser_bracketed.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Nov 14 11:55:12 EST 2012
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Bracketed Subexpression Pass
//	Bracketed Subexpression Parser Functions
//	Bracketed Subexpression Parser
//	Bracketed Pass Command Execution Function

// Usage and Setup
// ----- --- -----

# include <ll_lexeme_standard.h>
# include <ll_parser.h>
# include <ll_parser_command.h>
# include <ll_parser_bracketed.h>
# define LEX ll::lexeme
# define LEXSTD ll::lexeme::standard
# define PAR ll::parser
# define TAB ll::parser::table
# define COM ll::parser::command
# define BRA ll::parser::bracketed

static min::locatable_gen bracket;
static min::locatable_gen indentation;
static min::locatable_gen mark;
static min::locatable_gen gluing;
static min::locatable_gen named;
static min::locatable_gen dotdotdot;
static min::locatable_gen parsing;
static min::locatable_gen full;
static min::locatable_gen line;
static min::locatable_gen bracketed_subexpression;

static void initialize ( void )
{
    ::bracket = min::new_str_gen ( "bracket" );
    ::indentation = min::new_str_gen
			    ( "indentation" );
    ::mark = min::new_str_gen ( "mark" );
    ::gluing = min::new_str_gen ( "gluing" );
    ::named = min::new_str_gen ( "named" );
    ::dotdotdot = min::new_str_gen ( "..." );
    ::parsing = min::new_str_gen ( "parsing" );
    ::full = min::new_str_gen ( "full" );
    ::line = min::new_str_gen ( "line" );
    ::bracketed_subexpression =
        min::new_lab_gen
	    ( "bracketed", "subexpression" );
}
static min::initializer initializer ( ::initialize );

// Bracketed Subexpression Pass
// --------- ------------- ----

static min::uns32 bracketed_pass_stub_disp[] =
{
    min::DISP ( & BRA::bracketed_pass_struct
                     ::next ),
    min::DISP ( & BRA::bracketed_pass_struct
                     ::bracket_table ),
    min::DISP ( & BRA::bracketed_pass_struct
                     ::split_table ),
    min::DISP_END
};

static min::packed_struct_with_base
	<BRA::bracketed_pass_struct,
	 PAR::pass_struct>
    bracketed_pass_type
        ( "ll::parser::bracketed::bracketed_pass_type",
	  NULL,
	  ::bracketed_pass_stub_disp );
const min::uns32 & BRA::BRACKETED_PASS =
    ::bracketed_pass_type.subtype;

static void bracketed_pass_reset
	( PAR::parser parser,
	  PAR::pass pass )
{
    BRA::bracketed_pass bracketed_pass =
        (BRA::bracketed_pass) pass;
    TAB::table bracket_table =
        bracketed_pass->bracket_table;
    TAB::split_table split_table =
        bracketed_pass->split_table;

    min::uns64 collected_entries,
               collected_key_prefixes;

    TAB::end_block
        ( split_table, 0, collected_entries );
    TAB::end_block
        ( bracket_table, 0,
	  collected_key_prefixes, collected_entries );
    bracketed_pass->indent_offset = 2;
}

static min::gen bracketed_pass_end_block
	( PAR::parser parser,
	  PAR::pass pass,
	  min::obj_vec_ptr & vp,
	  min::phrase_position_vec ppvec,
	  min::gen name )
{
    BRA::bracketed_pass bracketed_pass =
        (BRA::bracketed_pass) pass;
    TAB::table bracket_table =
        bracketed_pass->bracket_table;
    TAB::split_table split_table =
        bracketed_pass->split_table;

    min::uns64 collected_entries,
               collected_key_prefixes;

    min::uns32 block_level =
        PAR::block_level ( parser );
    assert ( block_level > 0 );

    TAB::end_block
        ( split_table, block_level - 1,
	  collected_entries );
    TAB::end_block
        ( bracket_table, block_level - 1,
	  collected_key_prefixes, collected_entries );

    return min::SUCCESS();
}

static min::gen bracketed_pass_command
	( PAR::parser parser,
	  PAR::pass pass,
	  min::obj_vec_ptr & vp,
          min::phrase_position_vec ppvec );

BRA::bracketed_pass BRA::place
	( PAR::parser parser )
{
    min::locatable_var<BRA::bracketed_pass>
	bracketed_pass
	    ( ::bracketed_pass_type.new_stub() );

    bracketed_pass->reset =
        ::bracketed_pass_reset;
    bracketed_pass->end_block =
        ::bracketed_pass_end_block;
    bracketed_pass->parser_command =
        ::bracketed_pass_command;
    bracketed_pass->indent_offset = 2;
    bracket_table_ref(bracketed_pass) =
	TAB::create_table ( 256 );
    min::push ( bracketed_pass->bracket_table, 256 );
    split_table_ref(bracketed_pass) =
	TAB::create_split_table();
    min::push ( bracketed_pass->split_table, 256 );

    int index = TAB::find_name
        ( parser->trace_flag_name_table,
	  ::bracketed_subexpression );
    assert
      ( (unsigned) index < 8 * sizeof ( TAB::flags ) );
    bracketed_pass->trace_subexpressions =
        1ull << index;

    PAR::place
        ( parser, (PAR::pass) bracketed_pass,
	  parser->pass_stack );
    return bracketed_pass;
}

// Bracketed Subexpression Parser Functions
// --------- ------------- ------ ---------

// These are all static or inline; if any are useful
// elsewhere they can be put in the PAR namespace.

// Given a token sequence with n tokens, return a label
// whose elements are the values of the tokens.  If
// there are 0 tokens, return min::MISSING().  If there
// is just one, return its value.  Otherwise return a
// min::gen label.
//
// If any of the tokens are quoted strings or non-
// natural numbers, convert these to have values equal
// to their their strings and free their strings.
//
// It is a programming error if any token is not a word,
// number, or quoted string.
//
static min::gen make_label
	( PAR::token first, min::uns32 n )
{
    if ( n == 0 ) return min::MISSING();

    // Optimization.
    //
    if ( n == 1 && first->value != min::MISSING() )
        return first->value;

    min::gen label[n];

    for ( min::uns32 i = 0; i < n;
          ++ i, first = first->next )
    {
        if ( first->value == min::MISSING() )
	{
	    MIN_ASSERT
	        ( first->type == LEXSTD::number_t
		  ||
		     first->type
		  == LEXSTD::quoted_string_t );

	    PAR::value_ref(first) =
		min::new_str_gen
		    ( min::begin_ptr_of
		          ( first->string ),
		      first->string->length );
	    PAR::string_ref(first) =
	        PAR::free_string ( first->string );
	}
	else
	    MIN_ASSERT
	        ( first->type == LEXSTD::word_t
		  ||
		     first->type
		  == LEXSTD::natural_number_t );

	label[i] = first->value;
    }
    if ( n == 1 ) return label[0];
    else return min::new_lab_gen ( label, n );
}

// Compute attributes from a named bracket or named
// operator.  The `name', which is a label, is computed.
// If there are arguments, an `arguments' list is
// computed.  If there are keys, a `keys' list is
// computed.
//
// The named bracket or named operator is defined by
// `first' and `next'.  `first' is the first token AFTER
// the named opening, and `next' is the first token of
// the named middle (for a named bracket) or named
// closing (for a named operator).  No tokens are
// deleted, but some may have their values modified
// (e.g., quoted string tokens).
//
// `first' MUST BE a word.  The name is this word plus
// any following words and natural numbers.
//
// The arguments are any subexpressions, non-natural
// numbers, and quoted strings following the name but
// proceeding any named separator.  If there are no
// arguments, `arguments' is set to MISSING.  Otherwise
// `arguments' is set to a list that has only list
// elements, namely the arguments.  Any quoted string
// or non-natural number argument is converted as per
// PAR::convert_token before being put into this list.
// 
// A key is any sequence of words, numbers, or quoted
// strings following a named separator and preceeding
// any next named separator.  There MUST not be any
// marks, separators, or subexpressions in keys.  If
// there are no keys, `keys' is set to MISSING.  Other-
// wise, `keys' is set to a list object whose list
// elements are the keys, which are min::gen strings or
// min::gen labels.  The keys are made by ::make_label,
// and any quoted string or non-natural number tokens
// in keys are converted to min::gen strings equal to
// the translation string of the token lexeme.  The
// keys list object has .initiator and .separator BOTH
// equal to named_opening->separator->label.
//
// If a key is an empty string, a parsing error is
// announced and the key is ignored.
//
static void named_attributes
	( PAR::parser parser,
	  min::ref<min::gen> name,
	  min::ref<min::gen> arguments,
	  min::ref<min::gen> keys,
	  TAB::named_opening named_opening,
	  PAR::token first, PAR::token next )
{
    MIN_ASSERT ( first != next );
    MIN_ASSERT ( first->type == LEXSTD::word_t );

    // Temporary min::gen locatable.
    //
    min::locatable_gen exp;

    // Recast named_separator as a vector of min::gen
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

    // Count elements of name.
    //
    min::uns32 n = 0;
    PAR::token t = first;
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
    PAR::token tnext = PAR::find_separator
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
	        PAR::convert_token ( t );
	    min::attr_push(argp) = t->value;
	}
    }

    t = tnext;

    // Count the number of non-empty keys and announce
    // empty keys as errors.
    //
    min::uns32 sepcount = 0;
    min::uns32 keycount;

    while ( tnext != next )
    {
        tnext = PAR::skip ( tnext, seplen );
        tnext = PAR::find_separator
	    ( keycount, tnext, next,
	      separator, seplen );
	if ( keycount > 0 ) ++ sepcount;
	else
	{
	    parser->printer
		<< min::bom << min::set_indent ( 7 )
		<< "ERROR: empty key in named bracket"
		<< " or operator; "
		<< min::pline_numbers
		       ( parser->input_file,
			 tnext->position )
		<< ":" << min::eom;
	    min::print_phrase_lines
		( parser->printer,
		  parser->input_file,
		  tnext->position );
	    ++ parser->error_count;
	}
    }

    if ( sepcount == 0 )
        keys = min::MISSING();
    else
    {
        keys = min::new_obj_gen ( sepcount + 6, 2 );
	min::obj_vec_insptr keysvp ( keys );
	while ( t != next )
	{
	    t = PAR::skip ( t, seplen );
	    tnext = PAR::find_separator
		( keycount, t, next,
		  separator, seplen );
	    if ( keycount != 0 )
	    {
		exp = ::make_label ( t, keycount );
		min::attr_push(keysvp) = exp;
	    }
	    t = tnext;
	}
	min::attr_insptr keysap ( keysvp );

	min::locate ( keysap, PAR::dot_initiator );
	min::set ( keysap,
	           named_opening->named_separator
		                ->label );
	min::locate ( keysap, PAR::dot_separator );
	min::set ( keysap,
	           named_opening->named_separator
		                ->label );
    }
}

// Complain that token indent is too near indent.
//
static void complain_near_indent
	( PAR::parser parser,
	  PAR::token token,
	  min::int32 indent )
{
    parser->printer
	<< min::bom << min::set_indent ( 7 )
	<< "ERROR: lexeme indent "
	<< token->indent
	<< " too near paragraph indent "
	<< indent
	<< "; "
	<< min::pline_numbers
	       ( parser->input_file,
		 token->position )
	<< ":" << min::eom;
    min::print_phrase_lines
	( parser->printer,
	  parser->input_file,
	  token->position );
    ++ parser->error_count;
}

// Return token indent - indent and complain if token
// indent is too near indent.  Assert token has an
// indent.
//
inline min::int32 relative_indent
	( PAR::parser parser,
	  min::int32 indent_offset,
	  PAR::token token,
	  min::int32 indent )
{
    assert ( token->indent != LEX::AFTER_GRAPHIC );

    int relative_indent =
        (min::int32) token->indent - indent;
    if (    relative_indent != 0
	 && relative_indent < indent_offset 
	 && relative_indent > - indent_offset )
        ::complain_near_indent
	    ( parser, token, indent );
    return relative_indent;
}

// Bracketed Subexpression Parser
// --------- ------------- ------

bool BRA::parse_bracketed_subexpression
	( PAR::parser parser,
	  TAB::flags selectors,
	  PAR::token & current,
	  min::int32 indent,
	  TAB::indentation_mark indentation_mark,
	  BRA::bracket_stack * bracket_stack_p )
{
    BRA::bracketed_pass pass =
        (BRA::bracketed_pass) parser->pass_stack;

    TAB::indentation_mark indentation_found =
        min::NULL_STUB;
	// If not NULL_STUB, current token is an end-of-
	// line and current->previous token is the last
	// token of an indentation mark.
    TAB::named_opening named_opening =
        min::NULL_STUB;
	// If not NULL_STUB, a named opening bracket,
	// named closing bracket, or named operator
	// with this symbol table entry is being
	// scanned.
    bool is_named_opening_bracket;
        // If named_opening != NULL_STUB, this is true
	// if a named opening bracket or named operator
	// is being scanned, and false if a named
	// closing bracket is being scanned.
    PAR::token named_first = min::NULL_STUB;
        // If named_opening != NULL_STUB, this is the
	// first token after the named opening if is_
	// named_opening_bracket is true or after the
	// named middle if is_named_opening_bracket is
	// false.
    PAR::token split_backup = min::NULL_STUB;
        // If an indentation mark is split, back up
	// to this point if not NULL_STUB.
	//
	// This is the first word, number, mark, or
	// separator that is after the first previous
	// quoted string or found key.  So this is set
	// to the current token if that is a word,
	// number, mark or separator and split_backup is
	// NULL_STUB, and is set to NULL_STUB by every
	// other token and after finding a token
	// sequence in the bracket table.

    bool trace =
        (   parser->trace_flags
	  & pass->trace_subexpressions );
    bool trace_keys =
           trace
        &&    ( parser->trace_flags & PAR::TRACE_KEYS )
	   != 0;

    while ( true )
    {
        // Truncate if end of file.
	//
	if ( current->type == LEXSTD::end_of_file_t )
	    return false;

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
		 indentation_found == min::NULL_STUB )
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
		    split = pass->split_table[lastc];
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
			       (   (const char *)
			           min::begin_ptr_of
				       ( sp )
				 + (  length
			            - split->length ),
			         & split[0],
				 split->length )
			     == 0 ) break;
		    }
		    if ( split != min::NULL_STUB )
		    {
			// We have found a gluing mark
			// at the end of of the last
			// mark.
			//
			// Put new token between
			// current->previous and
			// current.  Remember that we
			// may have split_backup ==
			// current->previous, so we
			// want to preserve current->
			// previous as the last token
			// of the indentation mark.

		        PAR::value_ref
			    (current->previous) =
			    min::new_str_gen
			        ( min::begin_ptr_of
				      ( sp ),
				    length
				  - split->length );
			PAR::put_before
			    ( PAR::first_ref(parser),
			      current,
			      PAR::new_token
			          ( LEXSTD::mark_t ) );
			PAR::value_ref
			    (current->previous) =
			    split->indentation_mark
			          ->label;

			// Fix up the positions and
			// indentations in the tokens.
			// The indentation mark has
			// split->length bytes.
			//
			current->previous
			       ->position.end =
			    current->previous
			           ->previous
				   ->position.end;
			current->previous
			       ->position.begin =
			    current->previous
			           ->position.end;
			current->previous
			       ->position.begin.offset
			    -= split->length;
			current->previous->indent
			    = LEX::AFTER_GRAPHIC;
			current->previous->previous
			       ->position.end
			    = current->previous
			             ->position.begin;

			// Back up to split_backup which
			// will equal the mark that was
			// split or be before that.
			//
			assert (    split_backup 
			         != min::NULL_STUB );
			current = split_backup;
			continue;
		    }
		}

	    }

	    // Come here if last token of line was NOT
	    // split.

	    // Move forward to next token that is not
	    // a line break or full line comment.
	    //
	    PAR::token next = current->next;
	    while ( true )
	    {
		if ( next->type != LEXSTD::line_break_t
		     &&
		     next->type != LEXSTD::comment_t )
		    break;

		if ( next->next == parser->first )
		{
		    parser->input->add_tokens
			( parser, parser->input );
		    assert (    next->next
			     != parser->first );
		}
		next = next->next;
	    }

	    min::uns32 next_indent =
	        next->type == LEXSTD::end_of_file_t ?
		0 :
		next->indent;

	    // Delete the line breaks and full line
	    // comments skipped (keeping the line break
	    // at `current') and find the bounds of any
	    // comments that are not indented as much
	    // as the indent of next.
	    //
	    bool iic_exists = false;
	    min::phrase_position iic_position;
		// Data on insufficiently indented
		// comments.  Includes begin of first
		// such and end of last such.

	    while ( current->next != next )
	    {
		if (    current->next->type
		     == LEXSTD::comment_t
		     &&
		       current->next->indent
		     < next_indent )
		{
		    if ( ! iic_exists )
		    {
		        iic_exists = true;
			iic_position.begin =
			    current->next
			           ->position.begin;
		    }
		    iic_position.end =
		        current->next->position.end;
		}
		    
		PAR::free
		    ( PAR::remove ( first_ref(parser),
				    current->next ) );
	    }

	    // Issue warning for any insufficiently
	    // indented comments.
	    //
	    if ( iic_exists )
	    {
		parser->printer
		    << min::bom
		    << min::set_indent ( 9 )
		    << "WARNING: comments NOT indented"
		       " as much as following line; "
		    << min::pline_numbers
			   ( parser->input_file,
			     iic_position )
		    << ":" << min::eom;
		min::print_phrase_lines
		    ( parser->printer,
		      parser->input_file,
		      iic_position );
	    }

	    if ( indentation_found != min::NULL_STUB )
	    {
		// Tokens that bracket lines to be
		// scanned.  mark_end is the last token
		// of the indentation mark, and next is
		// the first token after the paragraph.
		//
		PAR::token mark_end = current->previous;
		PAR::token next = current;

		// Scan lines of paragraph.
		//
		// First be sure paragraph has some
		// lines.
		//
		if (    current->next->type
		     != LEXSTD::end_of_file_t
		     &&
		     relative_indent
		         ( parser, pass->indent_offset,
			   current->next, indent )
			 > 0 )
		{
		    // Compute selectors and paragraph
		    // indent for indented subparagraph.
		    //
		    TAB::flags new_selectors =
			selectors;
		    new_selectors |=
			indentation_found->new_selectors
					 .or_flags;
		    new_selectors &= ~
			indentation_found->new_selectors
					 .not_flags;
		    new_selectors ^=
			indentation_found->new_selectors
					 .xor_flags;

		    min::int32 paragraph_indent =
		        current->next->indent;

		    MIN_ASSERT
		        (    (unsigned) paragraph_indent
			  != LEX::AFTER_GRAPHIC );

		    // Delete line break.
		    //
		    current = current->next;
		    PAR::free
			( PAR::remove
			    ( first_ref(parser),
			      current->previous ) );

		    while ( true )
		    {
			// Find a paragraph line.
			//
			PAR::token previous =
			    current->previous;
			bool separator_found =
			  BRA::
			   parse_bracketed_subexpression
				( parser, new_selectors,
				  current,
				  paragraph_indent,
				  indentation_found,
				  bracket_stack_p );
			PAR::token first =
			    previous->next;
			next = current;
			if ( BRA::is_closed
			         ( bracket_stack_p ) )
			{
			    assert
			        ( ! separator_found );
			    next = bracket_stack_p
			              ->closing_first;
			}

			// Compact line subsubexp.
			//
			if ( first != next )
			{
			    min::phrase_position
			        position;
			    position.begin =
			        first->position.begin;
			    position.end =
			        next->previous
				    ->position.end;

			    min::gen terminator =
			        PAR::new_line;
			    if ( separator_found )
			    {
			        terminator =
				  indentation_found
				  ->
				  line_separator->label;

			        PAR::remove
				    ( parser,
				      next,
				      terminator );
				first = previous->next;
				    // In case first was
				    // removed.
			    }

			    PAR::attr attributes[1] =
			      { PAR::attr
			          ( PAR::dot_terminator,
				    terminator ) };

			    PAR::compact
				( parser,
				  pass->next,
				  selectors,
				  PAR::BRACKETED, trace,
				  first, next,
				  position,
				  1, attributes );
			}

			// See if there are more lines
			// in the paragraph.
			//
			if ( separator_found )
			    continue;
			else if
			    ( BRA::is_closed
			          ( bracket_stack_p )
			      ||
			         current->type
			      == LEXSTD::end_of_file_t
			      ||
			         current->next->type
			      == LEXSTD::end_of_file_t 
			      ||
			      ( current->next->indent
			        !=
				LEX::AFTER_GRAPHIC
				&&
			          (int)
				  current->next->indent
			        < paragraph_indent ) )
			    break;

			// Delete line break.
			//
			current = current->next;
			PAR::free
			    ( PAR::remove
				( first_ref(parser),
				  current->previous ) );
		    }

		    assert ( next == current );
		}

		PAR::token first = mark_end->next;
		min::phrase_position position;
		position.begin =
		    PAR::remove
			( parser, first,
			  indentation_found->label );
		position.end = next->previous
		                   ->position.end;

		PAR::attr attributes[1] =
		    { PAR::attr
		          ( PAR::dot_initiator,
			    indentation_found->
			        label ) };

		PAR::compact
		    ( parser, pass->next,
		      selectors, PAR::BRACKETED, trace,
		      first, next, position,
		      1, attributes );

		// Terminate subexpression if closing
		// bracket was found during indentation
		// processing, or if current token is an
		// end of file.
		// 
		if ( BRA::is_closed ( bracket_stack_p )
		     ||
		        current->type
		     == LEXSTD::end_of_file_t )
		    return false;

		// Otherwise fall through to process
		// line break at current that is after
		// indented lines.
		//
		indentation_found = min::NULL_STUB;
	    }

	    // If indentation was found, current may
	    // have changed.  In any case, it is a
	    // line break followed by a token that is
	    // not a line break or comment.
	    //
	    assert (    current->type
	             == LEXSTD::line_break_t );
	    next = current->next;
	    assert (    next->type
	             != LEXSTD::line_break_t
		     &&
		        next->type
		     != LEXSTD::comment_t );

	    // Truncate expression if line break is
	    // followed by an end of file.
	    //
	    if ( next->type == LEXSTD::end_of_file_t )
		return false;

	    // Now next is neither a line break or end
	    // of file or comment.

	    // Truncate subexpression if next token
	    // indent is at or before indent argument.
	    //
	    if ( next->indent != LEX::AFTER_GRAPHIC
	         &&
		    relative_indent
		          ( parser, pass->indent_offset,
			    next, indent )
		 <= 0 )
		return false;

	    // Remove line break and move to next token.
	    //
	    PAR::free ( PAR::remove ( first_ref(parser),
		                      current ) );
	    current = next;
	    split_backup = min::NULL_STUB;
	    continue;
	}
	else if ( current->type == LEXSTD::comment_t )
	{
	    // Comment that follows non-comment tokens
	    // on a line.  Remove and continue.

	    current = current->next;
	    PAR::free
		( PAR::remove ( first_ref(parser),
		                current->previous ) );
	    continue;
	}

	assert
	    ( indentation_found == min::NULL_STUB );
	assert
	    ( current->type != LEXSTD::end_of_file_t );
	assert
	    ( current->type != LEXSTD::line_break_t );
	assert
	    ( current->type != LEXSTD::comment_t );

	// Process quoted strings.
	//
	if ( current->type == LEXSTD::quoted_string_t )
	{
	    split_backup = min::NULL_STUB;

	    if ( named_opening != min::NULL_STUB
	         &&
		 !  is_named_opening_bracket )
		named_opening = min::NULL_STUB;

	    if ( current != parser->first
	         &&
		    current->previous->type
		 == LEXSTD::quoted_string_t )
	    {
	        // Merge current and current->previous,
		// which are both quoted strings.
		//
	        min::push
		    ( (PAR::string_insptr)
		          current->previous->string,
		      current->string->length,
		      min::begin_ptr_of
		          ( current->string ) );
		current->previous->position.end =
		    current->position.end;
		current = current->next;
		PAR::free
		    ( PAR::remove
			( first_ref(parser),
			  current->previous ) );
	    }
	    else
		current = current->next;

	    continue;
	}

	// If lookup key in bracket table.
	//
	PAR::token saved_current = current;
	PAR::token saved_split_backup = split_backup;
	split_backup = min::NULL_STUB;
	    // Assume for the moment that we will find
	    // an active bracket table entry.

	TAB::key_prefix key_prefix;
	TAB::root root =
	    find_entry ( parser, current, key_prefix,
			 TAB::ALL_FLAGS,
			 pass->bracket_table );

	while ( true )
	{
	    // Each iteration of this loop examines the
	    // found entry to see if it is a selected
	    // opening unnamed bracket, named opening,
	    // or indentation mark, or a not necessarily
	    // selected closing unnamed bracket, line
	    // separator, named key separator, named
	    // closing, or named middle closing that
	    // matches a symbol active because of the
	    // bracket_stack or indentation_mark
	    // arguments.
	    //
	    if ( root == min::NULL_STUB )
	    {
	        // No active bracket table entry found.

		split_backup = saved_split_backup;
		if ( split_backup == min::NULL_STUB )
		    split_backup = saved_current;
		current = saved_current->next;
		break;
	    }

	    min::uns32 subtype =
		min::packed_subtype_of ( root );

	    if ( trace_keys )
	        parser->printer
		    << "BRACKETED SUBEXPRESSION PARSER"
		       " FOUND "
		    << ( root->selectors & selectors ?
		         "SELECTED KEY " :
			 "UNSELECTED KEY " )
		    << min::pgen
		           ( root->label,
			     min::BRACKET_STR_FLAG )
		    << min::indent << " OF SUBTYPE "
		    << min::name_of_packed_subtype
		           ( min::packed_subtype_of
			         ( root ) )
		    << min::eol;

	    if ( subtype == TAB::OPENING_BRACKET
	         &&
		 ( selectors & root->selectors ) != 0 )
	    {
	        if ( named_opening != min::NULL_STUB
		     &&
		     !  is_named_opening_bracket )
		    named_opening = min::NULL_STUB;

		TAB::opening_bracket opening_bracket =
		    (TAB::opening_bracket) root;

		TAB::flags new_selectors =
		    selectors;
		new_selectors |=
		    opening_bracket->new_selectors
				    .or_flags;
		new_selectors &= ~
		    opening_bracket->new_selectors
				    .not_flags;
		new_selectors ^=
		    opening_bracket->new_selectors
				    .xor_flags;

		bool full_line =
		    opening_bracket->full_line;

		BRA::bracket_stack cstack
		    ( full_line ? NULL :
		                  bracket_stack_p );
		cstack.opening_bracket =
		    opening_bracket;

		PAR::token previous = current->previous;
		BRA::parse_bracketed_subexpression
		    ( parser, new_selectors,
		      current,
		      full_line ?
			  - pass->indent_offset :
			  indent,
		      min::NULL_STUB,
		      & cstack );
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
			   " closing bracket "
			<< min::pgen
			     ( opening_bracket->
			       closing_bracket->
				   label,
			       min::BRACKET_STR_FLAG )
			<< " inserted before ";

		    if ( next->value != min::MISSING() )
		        parser->printer
			    << min::pgen
			      ( next->value,
			        min::BRACKET_STR_FLAG )
			    << "; ";
		    else if (    next->type
		              == LEXSTD::line_break_t )
		        parser->printer
			    << "end of line; ";
		    else if (    next->type
		              == LEXSTD::end_of_file_t )
		        parser->printer
			    << "end of file; ";
		    else if
		        (    next->type
		          == LEXSTD::quoted_string_t )
		        parser->printer
			    << "\"...\"; ";

		    parser->printer
			<< min::pline_numbers
			       ( parser->input_file,
				 next->position )
			<< ":" << min::eom;
		    min::print_phrase_lines
			( parser->printer,
			  parser->input_file,
			  next->position );
		    ++ parser->error_count;

		    min::phrase_position position;
		    position.end =
			next->previous->position.end;
		    position.begin =
			PAR::remove
			    ( parser, first,
			      opening_bracket->label );

		    PAR::attr attributes[2] =
			{ PAR::attr
			      ( PAR::dot_initiator,
			        opening_bracket->
				    label ),
			  PAR::attr
			        ( PAR::dot_terminator,
			          opening_bracket->
			              closing_bracket->
				          label ) };

		    PAR::compact
		        ( parser, pass->next,
			  selectors,
			  PAR::BRACKETED, trace,
			  first, next, position,
			  2, attributes, 1 );

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
		    return false;
		}
		else
		{
		    assert (    cstack.closing_next
		             == current );

		    min::phrase_position position;
		    position.end =
			current->previous->position.end;
		    PAR::remove ( parser, current,
			          cstack.opening_bracket
			              ->closing_bracket
				      ->label );
		    position.begin =
			PAR::remove
			    ( parser, first,
			      opening_bracket->label );

		    PAR::attr attributes[2] =
			{ PAR::attr
			      ( PAR::dot_initiator,
			        opening_bracket->
				    label ),
			  PAR::attr
			        ( PAR::dot_terminator,
			        opening_bracket->
			            closing_bracket->
				        label ) };

		    PAR::compact
		        ( parser, pass->next,
			  selectors,
			  PAR::BRACKETED, trace,
			  first, current, position,
			  2, attributes, 1 );
		    break;
		}
	    }

	    else if ( subtype == TAB::CLOSING_BRACKET )
	    {
		TAB::closing_bracket closing_bracket =
		    (TAB::closing_bracket) root;

		for ( BRA::bracket_stack * p =
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

			for ( BRA::bracket_stack *
			          q = bracket_stack_p;
			      q != p;
			      q = q->previous )
			    q->closing_first =
			        q->closing_next =
				    saved_current;

			return false;
		    }
		}

		// Closing bracket does not match any
		// bracket stack entry; reject key.
	    }
	    else if ( subtype == TAB::INDENTATION_MARK
	              &&
		         ( selectors & root->selectors )
		      != 0 )
	    {
                if (    current->type
		     == LEXSTD::line_break_t
		     ||
		        current->type
		     == LEXSTD::end_of_file_t )
		{
		    indentation_found =
			(TAB::indentation_mark) root;
		    break;
		}

		// Indentation mark not at end of line
		// or end of file; reject key.
	    }
	    else if (    subtype
	              == TAB::LINE_SEPARATOR )
	    {
		TAB::line_separator
		    line_separator =
			(TAB::line_separator) root;
                if (    line_separator->indentation_mark
		     == indentation_mark )
		    return true;

		// Indentation separator does not match
		// indentation_mark argument; reject
		// key.
	    }
	    else if ( subtype == TAB::NAMED_OPENING 
	              &&
		         ( selectors & root->selectors )
		      != 0 )
	    {
	        if ( current->type == LEXSTD::word_t
		     ||
	             current->type == LEXSTD::number_t )
		{
		    // Possible start of named opening
		    // bracket or named operator.

		    named_opening =
			(TAB::named_opening) root;
		    is_named_opening_bracket = true;
		    named_first = current;
		    break;
		}

		// Name does not begin with word or
		// number; reject key.
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

		// Named separator does not match
		// current named opening; reject key.
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
		    // The end of a named opening
		    // bracket has been found.

		    BRA::bracket_stack cstack
			( bracket_stack_p );
		    cstack.named_opening =
			named_opening;
		    cstack.opening_first = named_first;
		    cstack.opening_next = saved_current;

		    PAR::token middle_last =
		        current->previous;
		    BRA::parse_bracketed_subexpression
			( parser, selectors,
			  current,
			  indent, min::NULL_STUB,
			  & cstack );

		    PAR::token next = current;
		        // Token just after last sub-
			// expression token AFTER any
			// named closing bracket has
			// been deleted.
		    bool done = false;
		        // Set if a bracket that was
			// not ours was found, so we
			// need to terminate this
			// call to parse_bracketed_
			// subexpression.
		    min::phrase_position position;
		        // Beginning of opening named
			// bracket and end of closing
			// named bracket.

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

			position.end =
			    next->previous
			         ->position.end;

			parser->printer
			    << min::bom
			    << min::set_indent ( 7 )
			    << "ERROR: missing named"
			       " closing bracket "
			    << min::pgen
			       ( named_opening
			         ->named_middle_closing
				 ->label,
			         min::BRACKET_STR_FLAG )
			    << " inserted; "
			    << min::pline_numbers
				   ( parser->input_file,
				     next->position )
			    << ":" << min::eom;
			min::print_phrase_lines
			    ( parser->printer,
			      parser->input_file,
			      next->position );
			++ parser->error_count;

			done = true;
		    }
		    else
		    {
		        position.end =
			    current->previous
			           ->position.end;
		        PAR::remove
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

		    position.begin =
		        PAR::remove
			    ( parser,
			      cstack.opening_first,
			      named_opening->label );

		    PAR::attr attributes[6] =
			{ PAR::attr
			      ( PAR::dot_initiator,
			        named_opening->label ),
			  PAR::attr
			        ( PAR::dot_terminator,
			        named_opening->
			            named_closing->
				        label ),
			  PAR::attr
			        ( PAR::dot_name, name )
			};
		    min::uns32 c = 3;
		    if ( named_opening->named_middle
		         !=
			 min::NULL_STUB )
		        attributes[c++] =
			    PAR::attr
			        ( PAR::dot_middle,
		                  named_opening->
				      named_middle->
				          label );
		    if ( arguments != min::MISSING() )
		        attributes[c++] =
			    PAR::attr
			        ( PAR::dot_arguments,
				  arguments );
		    if ( keys != min::MISSING() )
		        attributes[c++] =
			    PAR::attr
			        ( PAR::dot_keys, keys );

		    PAR::token first =
		        middle_last->next;
		    PAR::compact
		        ( parser, pass->next,
			  selectors,
			  PAR::BRACKETED, trace,
			  first, current, position,
			  c, attributes );
			  
		    assert (    first->type
			     == PAR::BRACKETED );
		    assert (    first
			     == middle_last->next );

		    PAR::remove ( parser,
		                  cstack.opening_first,
			          middle_last->next );

		    if ( done ) return false;
		    else	break;
		}
		else if (    named_opening
		          == min::NULL_STUB )
		{
		    for ( BRA::bracket_stack * p =
			      bracket_stack_p;
			  p != NULL;
			  p = p->previous )
		    {
			if (    p->named_opening
			     == named_middle->
				    named_opening )
			{
			    // The possible start of a
			    // named closing bracket has
			    // been found.

			    named_opening =
				named_middle->
				    named_opening;
			    is_named_opening_bracket =
			        false;
			    named_first = current;
			    break;
			}
		    }
		    if ( named_opening != NULL_STUB )
		        break;
		}

		// Named middle does not match any
		// active named opening or any bracket
		// stack entry; reject key.
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
			// A named operator has been
			// found.

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

			min::phrase_position position;
			position.begin =
			    PAR::remove
				( parser,
				  named_first,
				  named_opening->label
				);
			position.end =
			    current->previous
			           ->position.end;

			PAR::remove
			    ( parser,
			      named_first,
			      current );

			PAR::token t =
			    PAR::new_token
			        ( PAR::BRACKETED );
			t->position = position;

			PAR::put_before
			    ( first_ref(parser),
			      current, t );
			value_ref(t) =
			    min::new_obj_gen
			        ( 12, 5 );

			min::obj_vec_insptr tvp
			    ( t->value );
			min::attr_insptr tap ( tvp );

			min::locate
			    ( tap, PAR::dot_initiator );
			min::set
			    ( tap,
			      named_opening->label );

			min::locate
			    ( tap,
			      PAR::dot_terminator );
			min::set
			    ( tap,
			      named_opening->
			          named_closing->
				  label );

			min::locate
			    ( tap, PAR::dot_name );
			min::set ( tap, name );

			if (    arguments
			     != min::MISSING() )
			{
			    min::locate
				( tap,
				  PAR::dot_arguments );
			    min::set
				( tap, arguments );
			}

			if ( keys != min::MISSING() )
			{
			    min::locate
			        ( tap, PAR::dot_keys );
			    min::set ( tap, keys );
			}

			break;
		    }

		    // The end of a possible named
		    // closing bracket has been found.
		    //
		    // The named closing bracket must
		    // match a bracket stack entry, or
		    // it is discarded as not really
		    // being a named closing bracket.

		    for ( BRA::bracket_stack * p =
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
				     ( cp->type
				       !=
				       LEXSTD::word_t
				       &&
				       cp->type
				       !=
				       LEXSTD::number_t
				     )
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
				PAR::backup
				    ( named_first,
				      named_opening->
				          named_middle->
					  label );
			    p->closing_next = current;

			    for ( BRA::bracket_stack
			              * q =
				      bracket_stack_p;
				  q != p;
				  q = q->previous )
				q->closing_first =
				  q->closing_next =
				    p->closing_first;

			    return false;
			}
		    }
		}

		// Named closing does not match active
		// named opening or middle; or named
		// closing bracket does not match any
		// bracket stack entry; reject key.
	    }
	    else if
	        ( subtype == TAB::NAMED_MIDDLE_CLOSING )
	    {
		TAB::named_middle_closing
		    named_middle_closing =
		    (TAB::named_middle_closing) root;

		for ( BRA::bracket_stack * p =
			  bracket_stack_p;
		      p != NULL;
		      p = p->previous )
		{
		    if (    p->named_opening
			 == named_middle_closing->
			        named_opening )
		    {
		        p->closing_first =
			    saved_current;
			p->closing_next = current;

			for ( BRA::bracket_stack
				* q = bracket_stack_p;
				  q != p;
				  q = q->previous )
				q->closing_first =
				    q->closing_next =
					saved_current;

			return false;
		    }
		}

		// Named middle closing does not match
		// any bracket stack entry; reject key.
	    }

	    if ( trace_keys )
	        parser->printer
		    << "BRACKETED SUBEXPRESSION PARSER"
		       " REJECTED KEY "
		    << min::pgen
		           ( root->label,
			     min::BRACKET_STR_FLAG )
		    << min::eol;

	    root = PAR::find_next_entry
	               ( parser, current, key_prefix,
			 TAB::ALL_FLAGS, root );
	}
    }

    return false;
}

// Bracketed Pass Command Execution Function
// --------- ---- ------- --------- --------

enum definition_type
    { BRACKET,
      INDENTATION_MARK,
      NAMED_BRACKET };

static min::gen bracketed_pass_command
	( PAR::parser parser,
	  PAR::pass pass,
	  min::obj_vec_ptr & vp,
          min::phrase_position_vec ppvec )
{
    BRA::bracketed_pass bracketed_pass =
        (BRA::bracketed_pass) pass;

    min::uns32 size = min::size_of ( vp );

    // Scan keywords before names.
    //
    bool define;
        // True if define, false if undefine.
    definition_type type;
        // Type of define or undefine.
    unsigned i = 1;
        // vp[i] is next lexeme or subexpression to
	// scan in the define/undefine expression.
    bool gluing = false;
        // True if `define/undefine gluing ...', false
	// if not.
    unsigned min_names, max_names;
        // Minimum and maximum number of names allowed.

    if ( vp[i] == PAR::define )
        define = true;
    else if ( vp[i] == PAR::undefine )
        define = false;
    else
        return min::FAILURE();
    ++ i;

    if ( vp[i] == ::bracket )
    {
        type = ::BRACKET;
	min_names = 2;
	max_names = 2;
	++ i;
    }
    else if ( vp[i] == ::indentation
              &&
	      i + 1 < size
	      &&
	      vp[i + 1] == ::mark )
    {
	type = ::INDENTATION_MARK;
	min_names = 1;
	max_names = 2;
	i += 2;
    }
    else if ( vp[i] == ::gluing
              &&
	      i + 2 < size
	      &&
	      vp[i + 1] == ::indentation
	      &&
	      vp[i + 2] == ::mark )
    {
	type = ::INDENTATION_MARK;
	min_names = 1;
	max_names = 2;
	gluing = true;
	i += 3;
    }
    else if ( vp[i] == ::named
              &&
	      i + 1 < size
              &&
	      vp[i + 1] == ::bracket )
    {
	type = ::NAMED_BRACKET;
	min_names = 2;
	max_names = 6;
	i += 2;
    }
    else
        return min::FAILURE();

    // Scan mark names.
    //
    min::locatable_gen name[max_names+1];
    unsigned number_of_names = 0;

    while ( true )
    {
	// Scan a name.
	//
	name[number_of_names] =
	    PAR::scan_name_string_label
	        ( vp, i, parser,

	            ( 1ull << LEXSTD::mark_t )
	          + ( 1ull << LEXSTD::separator_t )
	          + ( 1ull << LEXSTD::word_t )
	          + ( 1ull << LEXSTD::number_t ),

	            ( 1ull << LEXSTD::
		                  horizontal_space_t )
	          + ( 1ull << LEXSTD::end_of_file_t ),

	            ( 1ull << LEXSTD::end_of_file_t ) );

	if ( name[number_of_names] == min::ERROR() )
	    return min::ERROR();
	else if (    name[number_of_names]
	          == min::MISSING() )
	    return PAR::parse_error
	        ( parser, ppvec[i-1],
		  "expected quoted name after" );
	else
	    ++ number_of_names;

	if ( number_of_names > max_names )
	    return PAR::parse_error
	        ( parser, ppvec->position,
		  "too many quoted names in" );

	if ( i >= size
	     ||
	     vp[i] != ::dotdotdot )
	    break;

	++ i;
    }

    if ( number_of_names < min_names )
	return PAR::parse_error
	    ( parser, ppvec->position,
	      "too few quoted names in" );

    TAB::flags selectors;
    min::gen sresult = COM::scan_flags
	    ( vp, i, selectors,
	      parser->selector_name_table, parser );
    if ( sresult == min::ERROR() )
	return min::ERROR();
    else if ( sresult == min::MISSING() )
	return PAR::parse_error
	    ( parser, ppvec[i-1],
	      "expected bracketed selector list"
	      " after" );

    // Some type specific error checking common to
    // define and undefine.
    //
    switch ( type )
    {
    case ::BRACKET:
        break;

    case ::INDENTATION_MARK:
    {
	if ( gluing
	     &&
		LEXSTD::lexical_type_of ( name[0] )
	     != LEXSTD::mark_t )
	    return PAR::parse_error
		( parser, ppvec[5],
		  "gluing indentation mark name ",
		  min::pgen ( name[0],
		              min::BRACKET_STR_FLAG ),
		  " is not a mark in" );
	break;
    }
    case ::NAMED_BRACKET:
    {
	bool separator_present =
	    ( number_of_names % 2 == 1 );
	bool middle_present =
	    ( number_of_names >= 4 );

	min::unsptr m = 1 + separator_present;
	if (    middle_present
	     && name[m] != name[m+1] )
	{
	    min::phrase_position pp;
	    pp.begin = ppvec[m].begin;
	    pp.end   = ppvec[m+1].end;
	    parser->printer
		<< min::bom << min::set_indent ( 7 )
		<< "ERROR: named middles "
		<< min::pgen ( name[m] )
		<< " and "
		<< min::pgen ( name[m+1] )
		<< " do not match in "
		<< min::pline_numbers
		       ( ppvec->file, pp )  
		<< ":" << min::eom;
	    min::print_phrase_lines
		( parser->printer,
		  ppvec->file, pp );
	    ++ parser->error_count;
	    return min::ERROR();
	}

	break;
    }
    }

    if ( define ) switch ( type )
    {
    case ::BRACKET:
    {
	bool full_line = false;
	TAB::new_flags new_selectors;
	    // Inited to zeroes.
	while ( i < size && vp[i] == PAR::with )
	{
	    ++ i;
	    if ( i + 1 < size
		 &&
		 vp[i] == ::parsing
		 &&
		 vp[i+1] == PAR::selectors )
	    {
		i += 2;
		min::gen result =
		    COM::scan_new_flags
			( vp, i, new_selectors,
	                  parser->selector_name_table,
			  parser );
		if ( result == min::ERROR() )
		    return min::ERROR();
		else if ( result == min::FAILURE() )
		    return PAR::parse_error
			( parser, ppvec[i-1],
			  "expected bracketed selector"
			  " modifier list after" );
	    }
	    else if ( i + 1 < size
		      &&
		      vp[i] == ::full
		      &&
		      vp[i+1] == ::line )
	    {
		i += 2;
		full_line = true;
	    }
	    else
		return PAR::parse_error
		    ( parser, ppvec[i-1],
		      "expected `parsing selectors'"
		      " or `full line' after" );
	}
	if ( i < size )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "expected `with' after" );

	TAB::push_brackets
	    ( name[0], name[1],
	      selectors,
	      PAR::block_level ( parser ),
	      ppvec->position,
	      new_selectors, full_line,
	      bracketed_pass->bracket_table );

	break;
    }
    case ::INDENTATION_MARK:
    {
	TAB::new_flags new_selectors;
	    // Inited to zeroes.
	while ( i < size && vp[i] == PAR::with )
	{
	    ++ i;
	    if ( i + 1 < size
		 &&
		 vp[i] == ::parsing
		 &&
		 vp[i+1] == PAR::selectors )
	    {
		i += 2;
		min::gen result =
		    COM::scan_new_flags
			( vp, i, new_selectors,
	                  parser->selector_name_table,
			  parser );
		if ( result == min::ERROR() )
		    return min::ERROR();
		else if ( result == min::FAILURE() )
		    return PAR::parse_error
			( parser, ppvec[i-1],
			  "expected bracketed selector"
			  " modifier list after" );
	    }
	    else
		return PAR::parse_error
		    ( parser, ppvec[i-1],
		      "expected `parsing selectors'"
		      " or `full line' after" );
	}
	if ( i < size )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "expected `with' after" );

	TAB::push_indentation_mark
	    ( name[0],
	      number_of_names == 2 ?
		  (min::gen) name[1] :
		  min::MISSING(),
	      selectors,
	      PAR::block_level ( parser ),
	      ppvec->position,
	      new_selectors,
	      bracketed_pass->bracket_table,
	      gluing ? bracketed_pass->split_table :
		       (TAB::split_table)
		       min::NULL_STUB );

	break;
    }
    case ::NAMED_BRACKET:
    {
	if ( i < size )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "expected end of statement after" );

	bool separator_present =
	    ( number_of_names % 2 == 1 );
	bool middle_present =
	    ( number_of_names >= 4 );

	min::gen named_opening = name[0];
	min::gen named_separator =
	    ( separator_present ?
	      (min::gen) name[1] : min::MISSING() );
	min::gen named_middle =
	    ( middle_present ?
	      (min::gen) name[1+separator_present] :
	      min::MISSING() );
	min::gen named_closing =
	    name[1 + separator_present
		   + 2 * middle_present ];

	// compute named_middle_closing if
	// necessary.
	//
	min::locatable_gen named_middle_closing
	    ( min::MISSING() );;
	min::gen middle_last = named_middle;
	min::gen closing_first = named_closing;
	min::unsptr middle_length = 1;
	min::unsptr closing_length = 1;

	min::lab_ptr middle_ptr ( named_middle );
	min::lab_ptr closing_ptr ( named_closing );
	if ( middle_ptr != min::NULL_STUB )
	{
	    middle_length =
		min::length_of ( middle_ptr );
	    middle_last =
		middle_ptr[middle_length-1];
	}
	if ( closing_ptr != min::NULL_STUB )
	{
	    closing_length =
		min::length_of ( closing_ptr );
	    closing_first =
		closing_ptr[closing_length-1];
	}
	min::uns32 middle_last_type =
	    LEXSTD::lexical_type_of
		( middle_last );
	min::uns32 closing_first_type =
	    LEXSTD::lexical_type_of
		( closing_first );
	if ( middle_last_type == closing_first_type
	     ||
	     middle_last_type != LEXSTD::separator_t
	   )
	{
	    min::str_ptr middle_last_ptr
		( middle_last );
	    min::str_ptr closing_first_ptr
		( closing_first );
	    min::unsptr middle_last_length =
		min::strlen ( middle_last_ptr );
	    min::unsptr closing_first_length =
		min::strlen ( closing_first_ptr );
	    char new_string [   middle_last_length
			      + closing_first_length
			      + 1 ];
	    strcpy
		( & new_string[0],
		  & middle_last_ptr[0] );
	    strcpy
		( & new_string[middle_last_length],
		  & closing_first_ptr[0] );
	    named_middle_closing =
		min::new_str_gen ( new_string );
	    if (   middle_length
		 + closing_length > 2 )
	    {
		min::gen element
		    [middle_length + closing_length
				   - 1];
		memcpy ( & element[0],
			 & middle_ptr[0],
			   ( middle_length - 1 )
			 * sizeof ( min::gen ) );
		memcpy ( & element[middle_length],
			 & closing_ptr[0],
			   ( closing_length - 1 )
			 * sizeof ( min::gen ) );
		element[middle_length - 1] =
		    named_middle_closing;
		named_middle_closing =
		    min::new_lab_gen
			( element, 
			    middle_last_length
			  + closing_first_length
			  - 1 );
	    }
	}

	TAB::push_named_brackets
	    ( named_opening,
	      named_separator,
	      named_middle,
	      named_closing,
	      named_middle_closing,
	      selectors,
	      PAR::block_level ( parser ),
	      ppvec->position,
	      bracketed_pass->bracket_table );

	break;
    }
    default:
	MIN_ABORT ( "bad parser define type" );
    }
    else /* if ( ! define ) */ 
    {
	if ( i < size )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "unexpected stuff after" );

	TAB::key_prefix key_prefix =
	    TAB::find_key_prefix
	        ( name[0],
		  bracketed_pass->bracket_table );

	if ( key_prefix != min::NULL_STUB )
	for ( TAB::root root = key_prefix->first;
	      root != min::NULL_STUB;
	      root = root->next )
	{
	    if (    ( root->selectors & selectors )
		 == 0 )
		continue;

	    min::uns32 subtype =
		min::packed_subtype_of ( root );

	    switch ( type )
	    {
	    case ::BRACKET:
	    {
		if ( subtype != TAB::OPENING_BRACKET )
		    continue;

		TAB::opening_bracket opening_bracket =
		    (TAB::opening_bracket) root;
		TAB::closing_bracket closing_bracket =
		    opening_bracket->closing_bracket;

		if ( closing_bracket->label != name[1] )
		    continue;

		break;
	    }

	    case ::INDENTATION_MARK:
	    {
		if ( subtype != TAB::INDENTATION_MARK )
		    continue;

		TAB::indentation_mark indentation_mark =
		    (TAB::indentation_mark) root;
		TAB::line_separator line_separator =
		    indentation_mark->line_separator;
		if ( line_separator == min::NULL_STUB
		     &&
		     number_of_names == 2 )
		    continue;
		if ( line_separator != min::NULL_STUB
		     &&
		     number_of_names == 1 )
		    continue;
		if ( line_separator != min::NULL_STUB
		     &&
		     line_separator->label != name[1] )
		    continue;

		break;
	    }
	    case ::NAMED_BRACKET:
	    {
		if ( subtype != TAB::NAMED_OPENING )
		    continue;

		bool separator_present =
		    ( number_of_names % 2 == 1 );
		bool middle_present =
		    ( number_of_names >= 4 );

		TAB::named_opening named_opening =
		    (TAB::named_opening) root;
		TAB::named_separator named_separator =
		    named_opening->named_separator;
		TAB::named_closing named_closing =
		    named_opening->named_closing;
		TAB::named_middle named_middle =
		    named_opening->named_middle;

		if ( (    named_separator
		       == min::NULL_STUB )
		     !=
		     separator_present )
		    continue;
		if ( named_separator != min::NULL_STUB
		     &&
		     named_separator->label != name[1] )
		    continue;
		if ( ( named_middle == min::NULL_STUB )
		     !=
		     middle_present )
		    continue;
		if ( named_middle != min::NULL_STUB
		     &&
		         named_middle->label
		     != name[1+separator_present] )
		    continue;
		if ( named_closing->label
		     !=
		     name[1 + separator_present
			    + 2 * middle_present ] )
		    continue;

		break;
	    }
	    default:
		MIN_ABORT
		    ( "bad parser undefine type" );
	    }

	    TAB::push_undefined
	        ( parser->undefined_stack,
		  root, selectors );
	}
    }

    return min::SUCCESS();
}
