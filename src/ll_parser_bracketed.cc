// Layered Languages Bracketed Subexpression Parser
//
// File:	ll_parser_bracketed.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Jun  3 15:23:37 EDT 2015
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Brackets
//	Indentation Marks
//	Bracketed Subexpression Pass
//	Bracketed Subexpression Parser Functions
//	Bracketed Subexpression Parser
//	Bracketed Pass Command Function

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
static min::locatable_gen full;
static min::locatable_gen lines;
static min::locatable_gen bracketed_subexpressions;
static min::locatable_gen offset;
static min::locatable_gen top;

static void initialize ( void )
{
    ::bracket = min::new_str_gen ( "bracket" );
    ::indentation = min::new_str_gen
			    ( "indentation" );
    ::mark = min::new_str_gen ( "mark" );
    ::gluing = min::new_str_gen ( "gluing" );
    ::full = min::new_str_gen ( "full" );
    ::lines = min::new_str_gen ( "lines" );
    ::bracketed_subexpressions =
        min::new_lab_gen
	    ( "bracketed", "subexpressions" );
    ::offset = min::new_str_gen ( "offset" );
    ::top = min::new_str_gen ( "top" );
}
static min::initializer initializer ( ::initialize );

// Brackets
// --------

static min::uns32 opening_bracket_stub_disp[] = {
    min::DISP ( & BRA::opening_bracket_struct::next ),
    min::DISP ( & BRA::opening_bracket_struct
                     ::closing_bracket ),
    min::DISP_END };

static min::packed_struct_with_base
	<BRA::opening_bracket_struct, TAB::root_struct>
    opening_bracket_type
	( "ll::parser::table::opening_bracket_type",
	  TAB::root_gen_disp,
	  ::opening_bracket_stub_disp );
const min::uns32 & BRA::OPENING_BRACKET =
    opening_bracket_type.subtype;

static min::uns32 closing_bracket_stub_disp[] = {
    min::DISP ( & BRA::closing_bracket_struct::next ),
    min::DISP ( & BRA::closing_bracket_struct
                     ::opening_bracket ),
    min::DISP_END };

static min::packed_struct_with_base
	<BRA::closing_bracket_struct, TAB::root_struct>
    closing_bracket_type
	( "ll::parser::table::closing_bracket_type",
	  TAB::root_gen_disp,
	  ::closing_bracket_stub_disp );
const min::uns32 & BRA::CLOSING_BRACKET =
    closing_bracket_type.subtype;

BRA::opening_bracket
    BRA::push_brackets
	( min::gen opening_label,
	  min::gen closing_label,
	  TAB::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  const TAB::new_flags & new_selectors,
	  bool full_lines,
	  TAB::key_table bracket_table )
{
    min::locatable_var<BRA::opening_bracket> opening
        ( ::opening_bracket_type.new_stub() );
    min::locatable_var<BRA::closing_bracket> closing
        ( ::closing_bracket_type.new_stub() );

    label_ref(opening) = opening_label;
    label_ref(closing) = closing_label;

    closing_bracket_ref(opening) = closing;
    opening_bracket_ref(closing) = opening;

    opening->selectors = selectors;
    closing->selectors = TAB::ALL_FLAGS;

    opening->block_level = block_level;
    closing->block_level = block_level;

    opening->position = position;
    closing->position = position;

    opening->new_selectors = new_selectors;
    opening->full_lines = full_lines;

    TAB::push ( bracket_table, (TAB::root) opening );
    TAB::push ( bracket_table, (TAB::root) closing );

    return opening;
}

// Indentation Marks
// ----------- -----

static min::uns32 indentation_mark_stub_disp[] = {
    min::DISP ( & BRA::indentation_mark_struct::next ),
    min::DISP ( & BRA::indentation_mark_struct
                     ::line_separator ),
    min::DISP ( & BRA::indentation_mark_struct
                     ::indentation_split ),
    min::DISP_END };

static min::packed_struct_with_base
	<BRA::indentation_mark_struct, TAB::root_struct>
    indentation_mark_type
	( "ll::parser::table::indentation_mark_type",
	  TAB::root_gen_disp,
	  ::indentation_mark_stub_disp );
const min::uns32 & BRA::INDENTATION_MARK =
    indentation_mark_type.subtype;

static min::uns32 line_separator_stub_disp[] = {
    min::DISP ( & BRA::line_separator_struct::next ),
    min::DISP ( & BRA::line_separator_struct
                     ::indentation_mark ),
    min::DISP_END };

static min::packed_struct_with_base
	<BRA::line_separator_struct, TAB::root_struct>
    line_separator_type
	( "ll::parser::table::line_separator_type",
	  TAB::root_gen_disp,
	  ::line_separator_stub_disp );
const min::uns32 & BRA::LINE_SEPARATOR =
    line_separator_type.subtype;

static min::uns32 indentation_split_stub_disp[] = {
    min::DISP ( & BRA::indentation_split_struct
                     ::next ),
    min::DISP ( & BRA::indentation_split_struct
                     ::indentation_mark ),
    min::DISP_END };

static min::packed_vec
	<min::uns8,BRA::indentation_split_struct>
    indentation_split_type
	( "ll::parser::table::indentation_split_type",
	  NULL, NULL, NULL,
	  ::indentation_split_stub_disp );

static min::uns32 split_table_stub_disp[] = {
    0, min::DISP_END };

static min::packed_vec<BRA::indentation_split>
    split_table_type
	( "ll::parser::table::split_table_type",
	  NULL, ::split_table_stub_disp );

BRA::split_table BRA::create_split_table ( void )
{
    return ::split_table_type.new_stub ( 256 );
}

BRA::indentation_mark
    BRA::push_indentation_mark
	( min::gen mark_label,
	  min::gen separator_label,
	  TAB::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  const TAB::new_flags & new_selectors,
	  TAB::key_table bracket_table,
	  BRA::split_table split_table )
{
    min::locatable_var<BRA::indentation_mark> imark
        ( ::indentation_mark_type.new_stub() );
    label_ref(imark) = mark_label;
    imark->selectors = selectors;
    imark->block_level = block_level;
    imark->position = position;
    imark->new_selectors = new_selectors;
    TAB::push ( bracket_table, (TAB::root) imark );

    if ( separator_label != min::MISSING() )
    {
	min::locatable_var<BRA::line_separator>
	    separator
	    ( ::line_separator_type.new_stub() );
	label_ref(separator) = separator_label;
	separator->selectors = TAB::ALL_FLAGS;
	separator->block_level = block_level;
	separator->position = position;
	indentation_mark_ref(separator) = imark;
	line_separator_ref(imark) = separator;
	TAB::push ( bracket_table,
	            (TAB::root) separator );
    }

    if ( split_table != NULL_STUB )
    {
        MIN_ASSERT ( min::is_str ( mark_label ),
	             "mark_label argument is not a"
		     " string" );
	min::str_ptr s ( mark_label );
	min::unsptr length = min::strlen ( s );
	min::locatable_var<BRA::indentation_split>
	    isplit;
	isplit = ::indentation_split_type.new_stub
			( length );
	min::push
	    ( isplit, length,
	      (min::ptr<min::uns8>)
	      min::begin_ptr_of ( s ) );
	indentation_mark_ref(isplit) = imark;
	indentation_split_ref(imark) = isplit;

	min::uns8 lastb = s[length - 1];

	min::ptr<BRA::indentation_split> p =
	    & split_table[lastb];
	while ( * p != NULL_STUB
	        &&
		(*p)->length > length )
	    p = & next_ref ( * p );
	next_ref(isplit) = * p;
	* p = isplit;
    }

    return imark;
}

void BRA::end_block
	( BRA::split_table split_table,
	  uns32 block_level,
	  uns64 & collected_entries )
{
    collected_entries = 0;
    for ( min::uns32 i = 0;
          i < split_table->length; ++ i )
    {
        BRA::indentation_split previous = NULL_STUB; 
	for ( BRA::indentation_split current =
	          split_table[i];
	      current != NULL_STUB;
	      current = current->next )
	{
	    if (   current->indentation_mark
	                   ->block_level
	         > block_level )
	    {
	        ++ collected_entries;
		if ( previous == NULL_STUB )
		    split_table[i] = current->next;
		else
		    next_ref ( previous ) =
		        current->next;
	    }
	        else previous = current;
	}
    }
}

// Bracketed Subexpression Pass
// --------- ------------- ----

static min::packed_vec<min::int32>
    indentation_offset_stack_type
        ( "ll::parser::bracketed"
	    "::indentation_offset_stack_type" );

static min::uns32 bracketed_pass_stub_disp[] =
{
    min::DISP ( & BRA::bracketed_pass_struct
                     ::parser ),
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

static void bracketed_pass_place
	( PAR::parser parser,
	  PAR::pass pass )
{
    // Bracketed pass must be at top of stack.
    //
    MIN_ASSERT ( parser->pass_stack == pass,
                 "bracketed pass is not at top of"
		 " parser pass_stack ");

    BRA::bracketed_pass bracketed_pass =
        (BRA::bracketed_pass) pass;

    int index = TAB::find_name
        ( parser->trace_flag_name_table,
	  ::bracketed_subexpressions );
    MIN_REQUIRE
      ( (unsigned) index < 8 * sizeof ( TAB::flags ) );
    bracketed_pass->trace_subexpressions =
        1ull << index;
}

static void bracketed_pass_reset
	( PAR::parser parser,
	  PAR::pass pass )
{
    BRA::bracketed_pass bracketed_pass =
        (BRA::bracketed_pass) pass;
    TAB::key_table bracket_table =
        bracketed_pass->bracket_table;
    BRA::split_table split_table =
        bracketed_pass->split_table;

    min::uns64 collected_entries,
               collected_key_prefixes;

    BRA::end_block
        ( split_table, 0, collected_entries );
    TAB::end_block
        ( bracket_table, 0,
	  collected_key_prefixes, collected_entries );
    bracketed_pass->indentation_offset = 2;
}

static min::gen bracketed_pass_begin_block
	( PAR::parser parser,
	  PAR::pass pass,
	  const min::phrase_position & position,
	  min::gen name )
{
    BRA::bracketed_pass bracketed_pass =
        (BRA::bracketed_pass) pass;

    min::push ( bracketed_pass->
                    indentation_offset_stack ) =
	bracketed_pass->indentation_offset;

    return min::SUCCESS();
}


static min::gen bracketed_pass_end_block
	( PAR::parser parser,
	  PAR::pass pass,
	  const min::phrase_position & position,
	  min::gen name )
{
    BRA::bracketed_pass bracketed_pass =
        (BRA::bracketed_pass) pass;
    TAB::key_table bracket_table =
        bracketed_pass->bracket_table;
    BRA::split_table split_table =
        bracketed_pass->split_table;

    min::uns64 collected_entries,
               collected_key_prefixes;

    min::uns32 block_level =
        PAR::block_level ( parser );
    MIN_REQUIRE ( block_level > 0 );

    BRA::end_block
        ( split_table, block_level - 1,
	  collected_entries );
    TAB::end_block
        ( bracket_table, block_level - 1,
	  collected_key_prefixes, collected_entries );

    bracketed_pass->indentation_offset =
        min::pop
	    ( bracketed_pass->
	          indentation_offset_stack );

    return min::SUCCESS();
}

static min::gen bracketed_pass_command
	( PAR::parser parser,
	  PAR::pass pass,
	  min::obj_vec_ptr & vp,
          min::phrase_position_vec ppvec );

PAR::pass BRA::new_pass ( void )
{
    min::locatable_var<BRA::bracketed_pass>
	bracketed_pass
	    ( ::bracketed_pass_type.new_stub() );

    BRA::name_ref ( bracketed_pass ) = ::top;

    bracketed_pass->place =
        ::bracketed_pass_place;
    bracketed_pass->reset =
        ::bracketed_pass_reset;
    bracketed_pass->begin_block =
        ::bracketed_pass_begin_block;
    bracketed_pass->end_block =
        ::bracketed_pass_end_block;
    bracketed_pass->parser_command =
        ::bracketed_pass_command;
    bracketed_pass->indentation_offset = 2;
    bracket_table_ref(bracketed_pass) =
	TAB::create_key_table ( 256 );
    split_table_ref(bracketed_pass) =
	BRA::create_split_table();
    min::push ( bracketed_pass->split_table, 256 );
    indentation_offset_stack_ref(bracketed_pass) =
        ::indentation_offset_stack_type.new_stub ( 16 );

    return (PAR::pass) bracketed_pass;
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
	    MIN_REQUIRE
	        ( first->type == LEXSTD::numeric_t
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
	    MIN_REQUIRE
	        ( first->type == LEXSTD::word_t
		  ||
		     first->type
		  == LEXSTD::natural_t );

	label[i] = first->value;
    }
    if ( n == 1 ) return label[0];
    else return min::new_lab_gen ( label, n );
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
	  min::int32 indentation_offset,
	  PAR::token token,
	  min::int32 indent )
{
    MIN_REQUIRE ( token->indent != LEX::AFTER_GRAPHIC );

    int relative_indent =
        (min::int32) token->indent - indent;
    if (    relative_indent != 0
	 && relative_indent < indentation_offset 
	 && relative_indent > - indentation_offset )
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
	  BRA::indentation_mark indentation_mark,
	  BRA::bracket_stack * bracket_stack_p )
{
    BRA::bracketed_pass pass =
        (BRA::bracketed_pass) parser->pass_stack;

    BRA::indentation_mark indentation_found =
        min::NULL_STUB;
	// If not NULL_STUB, current token is an end-of-
	// line and current->previous token is the last
	// token of an indentation mark.
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

    TAB::flags trace_flags = parser->trace_flags;
    if ( trace_flags & pass->trace_subexpressions )
    {
	trace_flags &=
	      PAR::TRACE_SUBEXPRESSION_ELEMENTS
	    + PAR::TRACE_SUBEXPRESSION_DETAILS
	    + PAR::TRACE_SUBEXPRESSION_LINES
	    + PAR::TRACE_KEYS;
	if ( trace_flags == 0 )
	    trace_flags =
	        PAR::TRACE_SUBEXPRESSION_ELEMENTS;
    }
    else
        trace_flags = 0;

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
	    MIN_REQUIRE
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
		BRA::indentation_split split =
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
			       (   ! min::begin_ptr_of
				       ( sp )
				 + (  length
			            - split->length ),
			         ! & split[0],
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
			MIN_REQUIRE
			    (    split_backup 
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
		    MIN_REQUIRE
		        (    next->next
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
		         ( parser,
			   pass->indentation_offset,
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

		    MIN_REQUIRE
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
			    MIN_REQUIRE
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

			    PAR::attr attributes[2];
			    unsigned n = 0;
			    attributes[n++] =
			        PAR::attr
			          ( min::dot_type,
				    PAR::new_line );

			    if ( separator_found )
			    {
			        min::gen terminator =
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
				attributes[n++] =
				    PAR::attr
				      ( min::
				        dot_terminator,
					terminator );
			    }

			    PAR::compact
				( parser,
				  pass->next,
				  selectors,
				  PAR::BRACKETED,
				  trace_flags,
				  first, next,
				  position,
				  n, attributes );
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

		    MIN_REQUIRE ( next == current );
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
		          ( min::dot_type,
			    indentation_found->
			        label ) };

		PAR::compact
		    ( parser, pass->next,
		      selectors, PAR::BRACKETED,
		      trace_flags,
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
	    MIN_REQUIRE (    current->type
	                  == LEXSTD::line_break_t );
	    next = current->next;
	    MIN_REQUIRE (    next->type
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
		          ( parser,
			    pass->indentation_offset,
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

	MIN_REQUIRE
	    ( indentation_found == min::NULL_STUB );
	MIN_REQUIRE
	    ( current->type != LEXSTD::end_of_file_t );
	MIN_REQUIRE
	    ( current->type != LEXSTD::line_break_t );
	MIN_REQUIRE
	    ( current->type != LEXSTD::comment_t );

	// Process quoted strings.
	//
	if ( current->type == LEXSTD::quoted_string_t )
	{
	    split_backup = min::NULL_STUB;

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
	    // opening bracket or indentation mark, or a
	    // not necessarily selected closing bracket
	    // or line separator that matches a symbol
	    // active because of the bracket_stack or
	    // indentation_mark arguments.
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

	    if ( trace_flags & PAR::TRACE_KEYS )
	        parser->printer
		    << "BRACKETED SUBEXPRESSION PARSER"
		       " FOUND "
		    << ( root->selectors & selectors ?
		         "SELECTED KEY " :
			 "UNSELECTED KEY " )
		    << min::pgen_quote ( root->label )
		    << min::indent << " OF SUBTYPE "
		    << min::name_of_packed_subtype
		           ( min::packed_subtype_of
			         ( root ) )
		    << min::eol;

	    if ( subtype == BRA::OPENING_BRACKET
	         &&
		 ( selectors & root->selectors ) != 0 )
	    {
		BRA::opening_bracket opening_bracket =
		    (BRA::opening_bracket) root;

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

		bool full_lines =
		    opening_bracket->full_lines;

		BRA::bracket_stack cstack
		    ( full_lines ? NULL :
		                   bracket_stack_p );
		cstack.opening_bracket =
		    opening_bracket;

		PAR::token previous = current->previous;
		BRA::parse_bracketed_subexpression
		    ( parser, new_selectors,
		      current,
		      full_lines ?
			  - pass->indentation_offset :
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
			<< min::pgen_quote
			     ( opening_bracket->
			       closing_bracket->
				   label )
			<< " inserted before ";

		    if ( next->value != min::MISSING() )
		        parser->printer
			    << min::pgen_quote
			      ( next->value )
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
			    << "\"... \"; ";

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
			      ( min::dot_initiator,
			        opening_bracket->
				    label ),
			  PAR::attr
			        ( min::dot_terminator,
			          opening_bracket->
			              closing_bracket->
				          label ) };

		    PAR::compact
		        ( parser, pass->next,
			  selectors,
			  PAR::BRACKETED,
			  trace_flags,
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
		    MIN_REQUIRE (    cstack.closing_next
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
			      ( min::dot_initiator,
			        opening_bracket->
				    label ),
			  PAR::attr
			        ( min::dot_terminator,
			        opening_bracket->
			            closing_bracket->
				        label ) };

		    PAR::compact
		        ( parser, pass->next,
			  selectors,
			  PAR::BRACKETED,
			  trace_flags,
			  first, current, position,
			  2, attributes, 1 );
		    break;
		}
	    }

	    else if ( subtype == BRA::CLOSING_BRACKET )
	    {
		BRA::closing_bracket closing_bracket =
		    (BRA::closing_bracket) root;

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
	    else if ( subtype == BRA::INDENTATION_MARK
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
			(BRA::indentation_mark) root;
		    break;
		}

		// Indentation mark not at end of line
		// or end of file; reject key.
	    }
	    else if (    subtype
	              == BRA::LINE_SEPARATOR )
	    {
		BRA::line_separator
		    line_separator =
			(BRA::line_separator) root;
                if (    line_separator->indentation_mark
		     == indentation_mark )
		    return true;

		// Indentation separator does not match
		// indentation_mark argument; reject
		// key.
	    }

	    if ( trace_flags & PAR::TRACE_KEYS )
	        parser->printer
		    << "BRACKETED SUBEXPRESSION PARSER"
		       " REJECTED KEY "
		    << min::pgen_quote ( root->label )
		    << min::eol;

	    root = PAR::find_next_entry
	               ( parser, current, key_prefix,
			 TAB::ALL_FLAGS, root );
	}
    }

    return false;
}

// Bracketed Pass Command Function
// --------- ---- ------- --------

enum definition_type
    { BRACKET,
      INDENTATION_MARK };

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
    unsigned i = 1;
        // vp[i] is next lexeme or subexpression to
	// scan in the define/undefine expression.
    min::gen command = vp[i];
        // define, undefine, or print.
    definition_type type;
        // Type of command (see above).
    bool gluing = false;
        // True if `command gluing ...', false if not.
    unsigned min_names, max_names;
        // Minimum and maximum number of names allowed.

    if ( command != PAR::define
         &&
	 command != PAR::undefine
         &&
	 command != PAR::print )
        return min::FAILURE();
    ++ i;

    if ( vp[i] == ::indentation
         &&
	 i + 1 < size
	 &&
	 vp[i+1] == ::offset )
    {
        if ( command == PAR::print )
	{
	    if ( i + 2 < size )
		return PAR::parse_error
		    ( parser, ppvec[i+1],
		      "unexpected stuff after" );

	    COM::print_command ( vp, parser );
	    parser->printer
		<< ":" << min::eol
		<< min::bom << min::no_auto_break
		<< min::set_indent ( 4 );

	    min::int32 offset =
	        bracketed_pass->indentation_offset; 
	    for ( min::uns32 i =
	              bracketed_pass->
		          indentation_offset_stack->
			      length;
		  0 <= i; -- i )
	    {
	        min::gen block_name =
		    ( i == 0 ?
		      (min::gen) PAR::top_level :
		      (&parser->block_stack[i-1])
		          ->name );

	        parser->printer << min::indent
		                << "block "
				<< min::pgen_name
				     ( block_name )
				<< ": " << offset;

		if ( i == 0 ) break;

		offset =
		    bracketed_pass->
		        indentation_offset_stack[i-1];
	    }

	    parser->printer << min::eom;
	    return COM::PRINTED;
	}
	else if ( command != PAR::define )
	    return min::FAILURE();

	else if ( i + 2 >= size
	          ||
		  ! min::strto
	                ( bracketed_pass->
			      indentation_offset,
		          vp[i+2], 10 ) )
	    return PAR::parse_error
		( parser, ppvec[i+1],
		  "expected reasonable sized integer"
		  " after" );

	else if ( i + 3 < size )
	    return PAR::parse_error
		( parser, ppvec[i+2],
		  "unexpected stuff after" );

	return min::SUCCESS();
    }

    if ( command == PAR::print )
    {
        if ( vp[i] != ::bracket )
	    return min::FAILURE();
	min_names = 1;
	max_names = 1;
	++ i;
    }
    else if ( vp[i] == ::bracket )
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
	          + ( 1ull << LEXSTD::natural_t )
	          + ( 1ull << LEXSTD::numeric_t ),

	            ( 1ull << LEXSTD::
		                  horizontal_space_t )
	          + ( 1ull << LEXSTD::
		              premature_end_of_file_t )
	          + ( 1ull << LEXSTD::end_of_file_t ),

	            ( 1ull << LEXSTD::
		              premature_end_of_file_t )
	          + ( 1ull << LEXSTD::end_of_file_t ),
		  command == PAR::print );

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
	     vp[i] != PAR::dotdotdot )
	    break;

	++ i;
    }

    if ( number_of_names < min_names )
	return PAR::parse_error
	    ( parser, ppvec->position,
	      "too few quoted names in" );

    if ( command == PAR::print )
    {
	if ( i < size )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "unexpected stuff after" );

	COM::print_command ( vp, parser );

	parser->printer
	    << ":" << min::eol
	    << min::bom << min::no_auto_break
	    << min::set_indent ( 4 );

	int count = 0;
	TAB::key_table_iterator it
	    ( bracketed_pass->bracket_table );
	while ( true )
	{
	    TAB::root root = it.next();
	    if ( root == min::NULL_STUB ) break;

	    if ( min::is_subsequence
		     ( name[0], root->label ) < 0 )
		continue;

	    min::uns32 subtype =
		min::packed_subtype_of ( root );

	    if ( subtype != BRA::OPENING_BRACKET
	         &&
		 subtype != BRA::INDENTATION_MARK )
	        continue;

	    ++ count;

	    min::gen block_name =
		PAR::block_name
		    ( parser,
		      root->block_level );
	    parser->printer
		<< min::indent
		<< "block "
		<< min::pgen_name ( block_name )
		<< ": " << min::save_indent;

	    if ( subtype == BRA::OPENING_BRACKET )
	    {
		BRA::opening_bracket opening_bracket =
		    (BRA::opening_bracket) root;
		BRA::closing_bracket closing_bracket =
		    opening_bracket->closing_bracket;

		parser->printer
		    << "bracket "
		    << min::pgen_quote
		           ( opening_bracket->label )
		    << " ... "
		    << min::pgen_quote
		           ( closing_bracket->label )
		    << " " << min::set_break;
		COM::print_flags
		    ( root->selectors,
		      parser->selector_name_table,
		      parser );

		TAB::new_flags new_selectors =
		    opening_bracket->new_selectors;

		if ( ! TAB::is_empty
			   ( new_selectors ) )
		{
		    parser->printer
		        << min::indent
			<< "with parsing"
			   " selectors ";
		    COM::print_new_flags
			( new_selectors,
			  parser->
			      selector_name_table,
			  parser );
		}

		if ( opening_bracket->full_lines )
		    parser->printer
			<< min::indent
			<< "with full lines";
	    }
	    else if ( subtype == BRA::INDENTATION_MARK )
	    {
		BRA::indentation_mark indentation_mark =
		    (BRA::indentation_mark) root;
		BRA::line_separator line_separator =
		    indentation_mark->line_separator;
		bool gluing =    
		    (    indentation_mark
		             ->indentation_split
		      != min::NULL_STUB );

		parser->printer
		    << ( gluing ?  "gluing " : "" )
		    << "indentation mark "
		    << min::pgen_quote
		        ( indentation_mark->label );
		if ( line_separator != min::NULL_STUB )
		    parser->printer
		        << " ... "
			<< min::pgen_quote
			    ( line_separator->label );
		parser->printer
		    << " " << min::set_break;
		COM::print_flags
		    ( root->selectors,
		      parser->selector_name_table,
		      parser );

		TAB::new_flags new_selectors =
		    indentation_mark->new_selectors;

		if ( ! TAB::is_empty
			   ( new_selectors ) )
		{
		    parser->printer
			<< min::indent
			<< "with parsing"
			   " selectors ";
		    COM::print_new_flags
			( new_selectors,
			  parser->
			      selector_name_table,
			  parser );
		}
	    }
	    else
	    {
		MIN_ABORT
		    ( "bad parser print type" );
	    }

	    parser->printer << min::restore_indent
			    << min::eol;
	}

	if ( count == 0 )
	    parser->printer << min::indent
	                    << "not found";
	parser->printer << min::eom;

        return COM::PRINTED;
    }

    // Some type specific error checking common to
    // define/undefine.
    //
    switch ( type )
    {
    case ::BRACKET:
        break;

    case ::INDENTATION_MARK:
    {
	min::uns32 ltype =
	    LEXSTD::lexical_type_of ( name[0] );
	if ( gluing
	     &&
	     ltype != LEXSTD::mark_t
	     &&
	     ltype != LEXSTD::separator_t )
	    return PAR::parse_error
		( parser, ppvec[5],
		  "gluing indentation mark name ",
		  min::pgen_quote ( name[0] ),
		  " is not a mark or separator in" );
	break;
    }
    }

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

    if ( command == PAR::define ) switch ( type )
    {
    case ::BRACKET:
    {
	bool full_lines = false;
	TAB::new_flags new_selectors;
	    // Inited to zeroes.
	while ( i < size && vp[i] == PAR::with )
	{
	    ++ i;
	    if ( i + 1 < size
		 &&
		 vp[i] == PAR::parsing
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
		      vp[i+1] == ::lines )
	    {
		i += 2;
		full_lines = true;
	    }
	    else
		return PAR::parse_error
		    ( parser, ppvec[i-1],
		      "expected `parsing selectors'"
		      " or `full lines' after" );
	}
	if ( i < size )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "expected `with' after" );

	BRA::push_brackets
	    ( name[0], name[1],
	      selectors,
	      PAR::block_level ( parser ),
	      ppvec->position,
	      new_selectors, full_lines,
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
		 vp[i] == PAR::parsing
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
		      " or `full lines' after" );
	}
	if ( i < size )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "expected `with' after" );

	BRA::push_indentation_mark
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
		       (BRA::split_table)
		       min::NULL_STUB );

	break;
    }
    default:
	MIN_ABORT ( "bad parser define type" );
    }
    else /* if command == PAR::undefine */
    {
	if ( i < size )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "unexpected stuff after" );

	TAB::key_prefix key_prefix =
	    TAB::find_key_prefix
	        ( name[0],
		  bracketed_pass->bracket_table );

	min::uns32 count = 0;

	if ( key_prefix != min::NULL_STUB )
	for ( TAB::root root = key_prefix->first;
	      root != min::NULL_STUB;
	      root = root->next )
	{
	    if ( ( root->selectors & selectors ) == 0 )
		continue;

	    min::uns32 subtype =
		min::packed_subtype_of ( root );

	    switch ( type )
	    {
	    case ::BRACKET:
	    {
		if ( subtype != BRA::OPENING_BRACKET )
		    continue;

		BRA::opening_bracket opening_bracket =
		    (BRA::opening_bracket) root;
		BRA::closing_bracket closing_bracket =
		    opening_bracket->closing_bracket;

		if ( closing_bracket->label != name[1] )
		    continue;

		break;
	    }

	    case ::INDENTATION_MARK:
	    {
		if ( subtype != BRA::INDENTATION_MARK )
		    continue;

		BRA::indentation_mark indentation_mark =
		    (BRA::indentation_mark) root;
		BRA::line_separator line_separator =
		    indentation_mark->line_separator;
		BRA::indentation_split
			indentation_split =
		    indentation_mark->indentation_split;
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
		if ( indentation_split == min::NULL_STUB
		     &&
		     gluing )
		    continue;
		if ( indentation_split != min::NULL_STUB
		     &&
		     ! gluing )
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

	    ++ count;
	}

	if ( count == 0 )
	    PAR::parse_warn
		( parser, ppvec->position,
		  "undefine found no definition" );
	else if ( count > 1 )
	    PAR::parse_warn
		( parser, ppvec->position,
		  "undefine cancelled more than one"
		  " definition" );
    }

    return min::SUCCESS();
}
