// Layered Languages Parser Commands
//
// File:	ll_parser_command.h
// Author:	Bob Walton (walton@acm.org)
// Date:	Tue Sep 14 12:48:38 EDT 2021
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Usage and Setup
// ----- --- -----

# ifndef LL_PARSER_COMMAND_H
# define LL_PARSER_COMMAND_H

# include <ll_parser.h>

namespace ll { namespace parser { namespace command {

// Prints
//
//   ERROR: in <file:line-#> expected <what> <where>:
//     <file-lines-with-pp underscored>
//
// and returns min::ERROR().
//
min::gen expected_error
	( min::printer printer,
	  min::file file,
	  min::phrase_position pp,
	  const char * what,
	  const char * where = "after" );

// Prints `extraneous stuff after <file,pp>' error and
// returns min::ERROR().
//
min::gen extra_stuff_after_error
	( min::printer printer,
	  min::file file,
	  min::phrase_position pp );


// Prints `<what_is> should be <should_be>' warning
// where what_is is at <file,pp>.
//
void misspell_warning
	( min::printer printer,
	  min::file file,
	  min::phrase_position pp,
	  min::gen what_is,
	  min::gen should_be );

// Executes:
//     parser->printer << min::indent << prefix
//                     << "... lexical master ...";
//
// as necessary to output both lexical masters if
// they are not PAR::MISSING_MASTER.
//
void print_lexical_master
	( ll::parser::parser parser,
	  min::uns32 paragraph_master,
	  min::uns32 line_master,
	  const char * prefix = "with " );

// Return true iff vp[i ...] is
//
//     [paragraph|line]? lexical master
//
inline bool is_lexical_master
	( min::obj_vec_ptr & vp,
	  min::uns32 i,
	  min::uns32 size )
{
    if ( i + 1 >= size )
        return false;
    else
    if ( vp[i] == ll::parser::lexeme::lexical )
        return vp[i+1] == ll::parser::lexeme::master;
    else
    if ( i + 2 >= size )
        return false;
    else
    if ( vp[i] != ll::parser::lexeme::paragraph
         &&
	 vp[i] != ll::parser::lexeme::line )
        return false;
    else
        return vp[i+1] == ll::parser::lexeme::lexical
	       &&
	       vp[i+2] == ll::parser::lexeme::master;
}

// Get lexical master if `is_lexical_master' returns
// true.  Set {paragraph,line}_lexical master
// accordingly.  Return min::ERROR() if error and
// min::SUCCESS() if not.
//
inline min::gen get_lexical_master
	( ll::parser::parser parser,
	  min::obj_vec_ptr & vp,
	  min::phrase_position_vec ppvec,
	  min::uns32 & i,
	  min::uns32 & paragraph_lexical_master,
	  min::uns32 & line_lexical_master )
{
    min::gen ltype = vp[i];
    if ( ltype == ll::parser::lexeme::lexical )
        i += 2;
    else
        i += 3;

    min::phrase_position position = ppvec[i];
    min::locatable_gen master_name
	( ll::parser::scan_name
	    ( vp, i, parser,
	      ll::parser::lexeme::with ) );
    if ( master_name == min::ERROR() )
	return min::ERROR();
    position.end = (& ppvec[i-1])->end;

    min::uns32 lexical_master =
	ll::parser::get_lexical_master
	    ( master_name, parser );
    if (    lexical_master
	 == ll::parser::MISSING_MASTER )
	return ll::parser::parse_error
	    ( parser, position,
	      "`",
	      min::pgen_quote
		  ( master_name ),
	      "' does NOT name a lexical"
	      " master" );
    if ( ltype == ll::parser::lexeme::line )
	line_lexical_master =
	    lexical_master;
    else
    if ( ltype == ll::parser::lexeme::paragraph )
	paragraph_lexical_master =
	    lexical_master;
    else
	line_lexical_master =
	paragraph_lexical_master =
	    lexical_master;

    return min::SUCCESS();
}

// If vp[i] is a ()-bracketed subexpression, treat it
// as an argument list, store the specified set of
// arguments in the arg_vec vector, increment i, and
// return min::SUCCESS().  If it is not a ()-bracketed
// subexpression, just return min::FAILURE().  If it is
// a ()-bracketed subexpression with an error, print
// error messages to parser->printer, increment i, and
// return min::ERROR().
//
// An argument may be empty, a simple name, a quoted
// name, or a ()-parenthesized argument sublist.
// Arguments are separated by commas.  An empty argument
// denotes min:MISSING().  It is assumed that the
// ()-bracketed subexpression does NOT have a
// separator and any commas are simply elements of the
// subexpression (ditto for argument sublists).
//
// If min::FAILURE() is not returned, arg_vec is created
// if it does not exist, emptied if it does exists, and
// then any successfully scanned arguments are pushed
// into it.  If min::ERROR() is returned, arg_vec is
// undefined.
//
min::gen scan_args
	( min::obj_vec_ptr & vp, min::uns32 & i,
          min::locatable_gen & args,
	  ll::parser::parser parser );

// If vp[i] is a []-bracketed subexpression, treat it
// as a flag list, store the specified set of flags in
// the flags argument, increment i, and return
// min::SUCCESS().  If it is not a []-bracketed
// subexpression, just return min::FAILURE().
// If it is a []-bracketed subexpression with an error,
// print error messages to parser->printer, increment i,
// and return min::ERROR().
//
// It is an error if any flag is not an allowed_flags.
// It is NOT an error if a flag group contains a flag
// not in allowed_flags.  However, only bits for flags
// in allowed_flags are set in flags.
//
// It is assumed that the []-bracketed subexpression
// does NOT have a separator and any comma separators
// are simply elements of the subexpression.
//
min::gen scan_flags
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  ll::parser::table::flags & flags,
	  ll::parser::table::flags allowed_flags,
	  ll::parser::table::name_table name_table,
	  ll::parser::table::key_table group_name_table,
	  ll::parser::parser parser );

// Ditto but scan a flag modifier list into new_flags
// instead of a flag list into flags.  If allow_flag_
// list is true, accept either a flag modifier list or a
// flag list.  A flag list is encoded by setting
// or_flags = flags, not_flags = ~flags & allowed_flags,
// and xor_flags = 0).
//
// Only bits in allowed_flags are set in or_flags,
// not_flags, and xor_flags.
//
min::gen scan_new_flags
	( min::obj_vec_ptr & vp, min::uns32 & i,
	  ll::parser::table::new_flags & new_flags,
	  ll::parser::table::flags allowed_flags,
	  ll::parser::table::name_table name_table,
	  ll::parser::table::key_table group_name_table,
	  ll::parser::parser parser,
	  bool allow_flag_list = false );

// Print flag modifier list.  If allow_flag_list is
// true and new_flags is a flag list (see below),
// then a flag list is printed instead.  The list is
// surrounded by [ ] brackets and `,' is the separator.
// If a modifier list is being printed, the operator
// characters `+', `-', and `^' are printed before the
// flag name.  Illegal modifier lists result in more
// than one operator for the same flag name (e.g.,
// `+^').
//
// Only the subset of the flags sets (or/not/xor_flags)
// selected by allowed_flags is considered.  The test
// for a flag list is:
//
//        or_flags & allowed_flags
//     == ( ~ not_flags ) & allowed_flags
// and
//     xor_flags & allowed_flags == 0,
//
void print_new_flags
	( const ll::parser::table::new_flags
	    & new_flags,
	  ll::parser::table::flags allowed_flags,
	  ll::parser::table::name_table name_table,
	  ll::parser::parser parser,
	  bool allow_flag_list = false );

// Print flag list.  Flags with min::MISSING() name and
// flags not in allowed_flags are ignored.
//
void print_flags
	( ll::parser::table::flags flags,
	  ll::parser::table::flags allowed_flags,
	  ll::parser::table::name_table name_table,
	  ll::parser::parser parser );

// Given an object that is the result of parsing a
// `parser test:' indented paragraph, print appropriate
// output
//
void parser_test_execute_command
	( ll::parser::parser parser,
	  min::gen indented_paragraph );

// Given an object that is the result of parsing a
// `parser:' indented paragraph, execute the parser
// commands in the paragraph.
//
void parser_execute_command
	( ll::parser::parser parser,
	  min::gen indented_paragraph );

// Given a vector pointer vp to an expression, test if
// the subexpression vp[i0 ..] is a parser command.  Do
// nothing but return min::FAILURE() if no.  If yes,
// process the command, and if there is no error return
// ll::parser::command::PRINTED if the command was
// printed (as by the `parser print ...' commands) or
// min::SUCCESS() otherwise, but if there is an error,
// print an error message to parser->printer and return
// min::ERROR().  Note that vp[0 .. i0-1] is ignored.
//
min::gen parser_execute_command
	( min::obj_vec_ptr & vp, min::uns32 i0,
	  ll::parser::parser parser );

// First prints the lines delimited by parser->message_
// header if parser->message_header.begin is not
// MISSING_POSITION, and if it prints these lines, sets
// parser->message_header.begin to MISSING_POSITION.
//
// Normally parser->message_header is set to point at
// the first character of the `parser:' indentation mark
// when that is recognized.
//
// Then print the command lines defined by ppvec->file
// and ppvec->position, and return the column of ppvec->
// position.begin (the indentation of command) in these
// printed lines.
//
min::uns32 print_command
	( ll::parser::parser parser,
	  min::phrase_position_vec ppvec );

} } }

# endif // LL_PARSER_COMMAND_H
