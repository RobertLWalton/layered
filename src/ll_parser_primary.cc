// Layered Languages Primary Parser Pass
//
// File:	ll_parser_primary.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Fri Feb  7 05:38:32 AM EST 2025
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Table of Contents
//
//	Usage and Setup
//	Primary Table Entries
//	Primary Parser Pass
//	Primary Parsing Functions
//	Primary Parse Function
//	Primary Pass Command Function

// Usage and Setup
// ----- --- -----

# include <ll_lexeme_standard.h>
# include <ll_parser_primary.h>
# include <ll_parser_command.h>
# define MUP min::unprotected
# define LEXSTD ll::lexeme::standard
# define PAR ll::parser
# define PARLEX ll::parser::lexeme
# define TAB ll::parser::table
# define COM ll::parser::command
# define PRIM ll::parser::primary
# define PRIMLEX ll::parser::primary::lexeme

min::locatable_gen PRIMLEX::primary;
min::locatable_gen PRIMLEX::primary_subexpressions;
min::locatable_gen PRIMLEX::variable;
min::locatable_gen PRIMLEX::function;
min::locatable_gen PRIMLEX::level;
min::locatable_gen PRIMLEX::depth;
min::locatable_gen PRIMLEX::location;
min::locatable_gen PRIMLEX::module;
min::locatable_gen PRIMLEX::parentheses;
min::locatable_gen PRIMLEX::square_brackets;

static min::locatable_gen opening_double_quote; // ``
static min::locatable_gen test;  		// test

static min::locatable_gen func_bool_expression[2];
    // Locatable purelists whose only element is
    // the corresponding element of func_bool_values.
    // Placed here because they must be locatable.

static void initialize ( void )
{
    PRIMLEX::primary = min::new_str_gen ( "primary" );
    PRIMLEX::primary_subexpressions =
        min::new_lab_gen
	    ( "primary", "subexpressions" );
    PRIMLEX::variable = min::new_str_gen ( "variable" );
    PRIMLEX::function = min::new_str_gen ( "function" );
    PRIMLEX::level = min::new_str_gen ( "level" );
    PRIMLEX::depth = min::new_str_gen ( "depth" );
    PRIMLEX::location = min::new_str_gen ( "location" );
    PRIMLEX::module = min::new_str_gen ( "module" );
    PRIMLEX::parentheses =
        min::new_lab_gen ( "(", ")" );
    PRIMLEX::square_brackets =
        min::new_lab_gen ( "[", "]" );
    ::opening_double_quote = min::new_str_gen ( "``" );
    ::test = min::new_str_gen ( "test" );

    PRIM::func_default_op = min::new_str_gen ( "?=" );

    PRIM::func_bool_values =
        min::new_lab_gen ( "FALSE", "TRUE" );
    PRIM::func_negators =
        min::new_lab_gen ( "no", "NO", "not", "NOT" );

    min::lab_ptr lp = PRIM::func_bool_values;
    min::obj_vec_insptr vp;
    for ( min::uns32 i = 0; i < 2; ++ i )
    {
	::func_bool_expression[i] =
	    min::new_obj_gen ( 1 );
	vp = ::func_bool_expression[i];
	min::attr_push ( vp ) = lp[i];
    }

    PAR::push_new_pass
        ( PRIMLEX::primary, PRIM::new_pass );
}
static min::initializer initializer ( ::initialize );

// Primary Table Entries
// ------- ----- -------

static min::uns32 var_gen_disp[] = {
    min::DISP ( & PRIM::var_struct::label ),
    min::DISP ( & PRIM::var_struct::module ),
    min::DISP_END };

static min::uns32 var_stub_disp[] = {
    min::DISP ( & PRIM::var_struct::next ),
    min::DISP ( & PRIM::var_struct::previous ),
    min::DISP ( & PRIM::var_struct::last ),
    min::DISP_END };

static min::packed_struct_with_base
	<PRIM::var_struct, TAB::root_struct>
    var_type ( "ll::parser::primary::var_type",
	        ::var_gen_disp,
	        ::var_stub_disp );
const min::uns32 & PRIM::VAR = ::var_type.subtype;

PRIM::var PRIM::create_var
	( min::gen var_label,
	  TAB::flags selectors,
	  const min::phrase_position & position,
	  min::uns16 level,
	  min::uns16 depth,
	  min::uns32 flags,
	  min::uns32 location,
	  min::gen module )
{
    min::locatable_var<PRIM::var> var
        ( ::var_type.new_stub() );

    PRIM::label_ref(var) = var_label;
    var->selectors = selectors;
    var->block_level = ( (min::uns32) level << 16 )
    		     + depth;
    var->position = position;

    var->flags = flags;
    var->location = location;
    PRIM::module_ref(var) = module;

    return var;
}

static min::uns32 func_gen_disp[] = {
    min::DISP ( & PRIM::func_struct::label ),
    min::DISP ( & PRIM::func_struct::module ),
    min::DISP_END };

static min::uns32 func_stub_disp[] = {
    min::DISP ( & PRIM::func_struct::next ),
    min::DISP ( & PRIM::func_struct::previous ),
    min::DISP ( & PRIM::func_struct::last ),
    min::DISP ( & PRIM::func_struct::args ),
    min::DISP ( & PRIM::func_struct::arg_lists ),
    min::DISP ( & PRIM::func_struct::term_table ),
    min::DISP_END };

static min::packed_struct_with_base
	<PRIM::func_struct, TAB::root_struct>
    func_type ( "ll::parser::primary::func_type",
	        ::func_gen_disp,
	        ::func_stub_disp );
const min::uns32 & PRIM::FUNC = ::func_type.subtype;

static min::uns32 arg_gen_disp[] = {
    min::DISP ( & PRIM::arg_struct::name ),
    min::DISP ( & PRIM::arg_struct::default_value ),
    min::DISP_END };

static min::packed_vec <PRIM::arg_struct>
    args_type ( "ll::parser::primary::args_type",
                ::arg_gen_disp );
const min::uns32 & PRIM::ARGS = ::args_type.subtype;

static min::uns32 arg_list_gen_disp[] = {
    min::DISP ( & PRIM::arg_list_struct::term_name ),
    min::DISP ( & PRIM::arg_list_struct::brackets ),
    min::DISP_END };

static min::packed_vec <PRIM::arg_list_struct>
    arg_lists_type
        ( "ll::parser::primary::arg_lists_type",
	  ::arg_list_gen_disp );
const min::uns32 & PRIM::ARG_LISTS =
    ::arg_lists_type.subtype;

PRIM::func PRIM::create_func
	( TAB::flags selectors,
	  const min::phrase_position & position,
	  min::uns16 level,
	  min::uns16 depth,
	  min::uns32 flags,
	  min::uns32 location,
	  min::gen module,
	  min::uns32 term_table_size )
{
    min::locatable_var<PRIM::func> func
        ( ::func_type.new_stub() );

    func->selectors = selectors;
    func->block_level = ( (min::uns32) level << 16 )
    		      + depth;
    func->position = position;

    func->flags = flags;
    func->location = location;
    PRIM::module_ref(func) = module;

    PRIM::args_ref(func) =
        (PRIM::args) ::args_type.new_stub();
    PRIM::arg_lists_ref(func) =
        (PRIM::arg_lists) ::arg_lists_type.new_stub();
    PRIM::term_table_ref(func) =
        TAB::create_key_table ( term_table_size );

    return func;
}

PRIM::func PRIM::push_op
	( min::gen op_name,
	  TAB::key_table symbol_table,
	  min::uns8 op_code_1,
	  min::uns8 op_code_2,
	  bool is_infix,
	  min::uns32 base_flags )
{
    min::uns32 flags =
    	( ( (min::uns32) op_code_1 << 24 )
	  +
    	  ( (min::uns32) op_code_2 << 16 )
	  +
	  base_flags );

    min::locatable_var<PRIM::func> func
        ( PRIM::create_func
	    ( PAR::ALL_SELECTORS,
	      PAR::top_level_position, 0, 0,
	      flags, 0, min::MISSING(), 0 ) );

    min::gen labv[3] =
	{ PRIMLEX::parentheses, op_name,
	  PRIMLEX::parentheses };
    PRIM::label_ref(func) =
	min::new_lab_gen ( labv, is_infix ? 3 : 2 );

    min::locatable_gen X
        ( min::new_str_gen ( "X" ) );
    min::locatable_gen Y
        ( min::new_str_gen ( "Y" ) );

    // Argument before operator.
    //
    PRIM::push_arg ( X, min::NONE(), func );
    PRIM::push_arg_list
	( min::NONE(), 1, 0, 1, PRIMLEX::parentheses,
	  func );
    func->number_initial_arg_lists = 1;

    if ( is_infix )
    {
	// Argument after operator
	PRIM::push_arg ( Y, min::NONE(), func );
	PRIM::push_arg_list
	    ( op_name, 1, 1, 1, PRIMLEX::parentheses,
	      func );
	func->number_following_arg_lists = 1;
    }

    TAB::push ( symbol_table, (TAB::root) func );
    return func;
}

PRIM::func PRIM::push_builtin_func
	( min::gen func_name,
	  TAB::key_table symbol_table,
	  min::uns8 op_code,
	  min::uns32 number_of_arguments,
	  min::uns32 base_flags )
{
    min::uns32 flags =
    	( ( (min::uns32) op_code << 24 )
	  +
	  base_flags );

    min::locatable_var<PRIM::func> func
        ( PRIM::create_func
	    ( PAR::ALL_SELECTORS,
	      PAR::top_level_position, 0, 0,
	      flags, 0, min::MISSING(), 0 ) );

    PRIM::label_ref(func) = func_name;

    for ( min::uns32 i = 0;
          i < number_of_arguments; ++ i )
    {
        char aname[10];
	std::sprintf ( aname, "A%d", i );
	min::locatable_gen A
	    ( min::new_str_gen ( aname ) );

	PRIM::push_arg ( A, min::NONE(), func );
    }
    PRIM::push_arg_list
	( func_name, number_of_arguments, 0, 1,
	  PRIMLEX::parentheses, func );
    func->number_following_arg_lists = 1;

    TAB::push ( symbol_table, (TAB::root) func );
    return func;
}

static min::uns32 func_term_gen_disp[] = {
    min::DISP ( & PRIM::func_term_struct::label ),
    min::DISP_END };

static min::uns32 func_term_stub_disp[] = {
    min::DISP ( & PRIM::func_term_struct::next ),
    min::DISP ( & PRIM::func_term_struct::previous ),
    min::DISP ( & PRIM::func_term_struct::last ),
    min::DISP_END };

static min::packed_struct_with_base
	<PRIM::func_term_struct, TAB::root_struct>
    func_term_type
        ( "ll::parser::primary::func_term_type",
	  ::func_term_gen_disp,
	  ::func_term_stub_disp );
const min::uns32 & PRIM::FUNC_TERM =
    ::func_term_type.subtype;

PRIM::func_term PRIM::create_func_term
    ( min::gen func_term_name )
{
    min::locatable_var<PRIM::func_term> func_term
        ( ::func_term_type.new_stub() );

    PRIM::label_ref(func_term) = func_term_name;
    func_term->selectors = PAR::ALL_SELECTORS;
    func_term->block_level = 0;
    return func_term;
}

// Primary Parser Pass
// ------- ------ ----

static min::uns32 primary_pass_gen_disp[] =
{
    min::DISP ( & PRIM::primary_pass_struct::name ),
    min::DISP ( & PRIM::primary_pass_struct
    		      ::modifying_ops ),
    min::DISP_END
};

static min::uns32 primary_pass_stub_disp[] =
{
    min::DISP ( & PRIM::primary_pass_struct::parser ),
    min::DISP ( & PRIM::primary_pass_struct::next ),
    min::DISP ( & PRIM::primary_pass_struct
                      ::primary_table ),
    min::DISP_END
};

static min::packed_struct_with_base
	<PRIM::primary_pass_struct, PAR::pass_struct>
    primary_pass_type
        ( "ll::parser::primary::primary_pass_type",
	  ::primary_pass_gen_disp,
	  ::primary_pass_stub_disp );
const min::uns32 & PRIM::PRIMARY_PASS =
    ::primary_pass_type.subtype;

static void primary_pass_place
	( PAR::parser parser,
	  PAR::pass pass )
{
    PRIM::primary_pass primary_pass =
        (PRIM::primary_pass) pass;

    int index = TAB::find_name
        ( parser->trace_flag_name_table,
	  PRIMLEX::primary_subexpressions );
    MIN_REQUIRE
      ( (unsigned) index < 8 * sizeof ( TAB::flags ) );
    primary_pass->trace_subexpressions =
        1ull << index;
}

static void primary_pass_reset
	( PAR::parser parser,
	  PAR::pass pass )
{
    PRIM::primary_pass primary_pass =
        (PRIM::primary_pass) pass;

    TAB::key_table primary_table =
        primary_pass->primary_table;

    min::uns64 collected_entries,
               collected_key_prefixes;

    TAB::end_block
        ( primary_table, 0,
	  collected_key_prefixes, collected_entries );
}

static min::gen primary_pass_end_block
	( PAR::parser parser,
	  PAR::pass pass,
	  const min::phrase_position & position,
	  min::gen name )
{
    PRIM::primary_pass primary_pass =
	(PRIM::primary_pass) pass;
    TAB::key_table primary_table =
	primary_pass->primary_table;

    min::uns64 collected_entries,
               collected_key_prefixes;

    min::uns32 block_level =
        PAR::block_level ( parser );
    MIN_REQUIRE ( block_level > 0 );
    TAB::end_block
        ( primary_table, block_level,
	  collected_key_prefixes, collected_entries );

    return min::SUCCESS();
}

static void primary_parse ( PAR::parser parser,
		            PAR::pass pass,
		            TAB::flags selectors,
		            PAR::token & first,
		            PAR::token next );

static min::gen primary_pass_command
	( PAR::parser parser,
	  PAR::pass pass,
	  min::obj_vec_ptr & vp, min::uns32 i0,
          min::phrase_position_vec ppvec );

PAR::pass PRIM::new_pass ( PAR::parser parser )
{
    min::locatable_var<PRIM::primary_pass> primary_pass
        ( ::primary_pass_type.new_stub() );

    PRIM::name_ref ( primary_pass ) = PRIMLEX::primary;

    PRIM::primary_table_ref ( primary_pass ) =
        TAB::create_key_table ( 1024 );

    PRIM::modifying_ops_ref ( primary_pass ) =
        min::new_obj_gen ( 100, 50 );

    primary_pass->parser_command =
	::primary_pass_command;
    primary_pass->parse = ::primary_parse;
    primary_pass->place = ::primary_pass_place;
    primary_pass->reset = ::primary_pass_reset;
    primary_pass->end_block = ::primary_pass_end_block;

    return (PAR::pass) primary_pass;
}

PRIM::primary_pass PRIM::init_primary
	( PAR::parser parser,
	  PAR::pass next )
{
    PAR::pass pass =
        PAR::find_on_pass_stack
	    ( parser, PRIMLEX::primary );
    if ( pass != min::NULL_STUB )
        return (PRIM::primary_pass) pass;

    min::locatable_var<PRIM::primary_pass> primary_pass
        ( (PRIM::primary_pass)
	  PRIM::new_pass ( parser ) );
    PAR::place_before
        ( parser, (PAR::pass) primary_pass, next );
    return primary_pass;
}

// Primary Parsing Functions
// ------- ------- ---------

// Print representation of function.  If default_op is
// NONE, do not print default values even if they exist.
//
static void print_func
	( PRIM::func func, PAR::parser parser,
	  min::gen default_op = min::NONE() ) 
{
    min::uns32 len = func->arg_lists->length;
    for ( min::uns32 j = 0; j < len; ++ j )
    {
	PRIM::arg_list_struct
	    arg_list = func->arg_lists[j];

	if ( arg_list.term_name != min::NONE() )
	    parser->printer << min::pgen_name
	        ( arg_list.term_name );

	min::uns32 k = arg_list.first;
	min::uns32 kend = k + arg_list.number_of_args;
	min::lab_ptr brackets = arg_list.brackets;
	parser->printer
	    << min::pgen_never_quote ( brackets[0] );
	for ( ; k < kend; ++ k )
	{
	    PRIM::arg_struct arg = func->args[k];
	    parser->printer
	        << min::pgen_name ( arg.name );
	    if ( default_op != min::NONE()
	         &&
		 arg.default_value != min::NONE() )
		parser->printer
		    << " " << default_op
		    << " " << arg.default_value;
	    if ( k + 1 < kend )
		parser->printer << ", ";
	}
	parser->printer
	    << min::pgen_never_quote ( brackets[1] );
    }
    if ( len == func->number_initial_arg_lists )
    {
        min::lab_ptr lp = func->label;
	parser->printer <<
	    min::pgen_name ( lp[1] );
    }
}

// Print scan_primary rejection message.
//
static void print_reject
	( PAR::parser parser,
	  PRIM::func func,
	  const char * message1,
	  const min::op & message2 = min::pnop,
	  const char * message3 = "",
	  const min::op & message4 = min::pnop,
	  const char * message5 = "",
	  const min::op & message6 = min::pnop,
	  const char * message7 = "",
	  const min::op & message8 = min::pnop,
	  const char * message9 = "" )
{
    parser->printer << min::bol << min::bom
                    << min::set_indent ( 11 )
	            << "REJECTING: ";
    ::print_func ( func, parser );
    parser->printer << min::indent
		    << message1 << message2
		    << message3 << message4
		    << message5 << message6
		    << message7 << message8
		    << message9 << min::eom;
}

min::gen PRIM::scan_name
    ( min::obj_vec_ptr & vp, min::uns32 & i,
      min::uns64 initial_types,
      min::uns64 following_types,
      min::uns64 outside_quotes_types,
      min::uns64 inside_quotes_types )
{
    MIN_REQUIRE
        (   (   ( initial_types | following_types )
              & (1ull << LEXSTD::not_a_lexeme_t ) )
	  == 0 );

    min::uns32 s = min::size_of ( vp );
    min::gen labbuf[s - i];
    min::uns32 j = 0;
    min::uns64 types = initial_types;
    for ( ; i < s; ++i )
    {
	min::uns64 quotes_types = outside_quotes_types;
        min::gen c = vp[i];
	if ( min::is_obj ( c ) )
	{
	    c = PAR::quoted_string_value ( c );
	    quotes_types = inside_quotes_types;
	}
	min::uns64 t =
	    ( 1ull << PAR::lexical_type_of ( c ) );
	if ( ( t & types & quotes_types ) == 0 )
	    break;
	labbuf[j++] = c;
	types = following_types;
    }
    if ( j == 0 ) return min::NONE();
    else if ( j == 1 ) return labbuf[0];
    else return min::new_lab_gen ( labbuf, j );
}

min::uns64 PRIM::var_initial_types =
    (1ull << LEXSTD::word_t);
min::uns64 PRIM::var_following_types =
      (1ull << LEXSTD::word_t)
    + (1ull << LEXSTD::natural_t);
min::uns64 PRIM::var_outside_quotes_types =
      (1ull << LEXSTD::word_t)
    + (1ull << LEXSTD::natural_t);
min::uns64 PRIM::var_inside_quotes_types =
    0;

min::uns64 PRIM::func_term_initial_types =
      (1ull << LEXSTD::word_t)
    + (1ull << LEXSTD::mark_t)
    + (1ull << LEXSTD::separator_t);
min::uns64 PRIM::func_term_following_types =
      (1ull << LEXSTD::word_t)
    + (1ull << LEXSTD::mark_t)
    + (1ull << LEXSTD::separator_t)
    + (1ull << LEXSTD::natural_t);
min::uns64 PRIM::func_term_outside_quotes_types =
      (1ull << LEXSTD::word_t)
    + (1ull << LEXSTD::natural_t);
min::uns64 PRIM::func_term_inside_quotes_types =
      (1ull << LEXSTD::word_t)
    + (1ull << LEXSTD::mark_t)
    + (1ull << LEXSTD::separator_t);

min::uns64 PRIM::call_term_initial_types =
      (1ull << LEXSTD::word_t)
    + (1ull << LEXSTD::mark_t)
    + (1ull << LEXSTD::separator_t);
min::uns64 PRIM::call_term_following_types =
      (1ull << LEXSTD::word_t)
    + (1ull << LEXSTD::mark_t)
    + (1ull << LEXSTD::separator_t)
    + (1ull << LEXSTD::natural_t);
min::uns64 PRIM::call_term_outside_quotes_types =
      (1ull << LEXSTD::word_t)
    + (1ull << LEXSTD::natural_t)
    + (1ull << LEXSTD::mark_t)
    + (1ull << LEXSTD::separator_t);
min::uns64 PRIM::call_term_inside_quotes_types =
      (1ull << LEXSTD::word_t)
    + (1ull << LEXSTD::mark_t)
    + (1ull << LEXSTD::separator_t);

min::gen PRIM::scan_func_label
    ( min::obj_vec_ptr & vp, min::uns32 & i,
      PAR::parser parser )
{
    min::uns32 s = min::size_of(vp);
    min::gen labbuf[s - i];
    min::uns32 j = 0;
    for ( ; i < s && min::is_obj(vp[i]); ++ i )
    {
        min::gen initiator =
	    min::get ( vp[i], min::dot_initiator );
	if ( initiator == PARLEX::left_parenthesis )
	    labbuf[j++] = PRIMLEX::parentheses;
	else
	if ( initiator == PARLEX::left_square )
	    labbuf[j++] = PRIMLEX::square_brackets;
	else
	    break;
    }
    min::locatable_gen term
        ( PRIM::scan_func_term_name ( vp, i ) );
    if ( j == 0 ) return term;

    if ( term != min::NONE() )
    {
	min::lab_ptr labp = term;
	if ( labp != min::NULL_STUB )
	{
	    min::uns32 len = min::lablen ( labp );
	    for ( min::uns32 k = 0; k < len; )
	        labbuf[j++] = labp[k++];
	}
	else
	    labbuf[j++] = term;
    }

    if ( j == 1 ) return labbuf[0];
    else return min::new_lab_gen ( labbuf, j );
}

min::locatable_gen PRIM::func_default_op;
min::locatable_gen PRIM::func_bool_values;
min::locatable_gen PRIM::func_negators;
    // See initialize function.

min::uns32 PRIM::func_term_table_size = 16;

inline min::uns32 process_arg
    ( PRIM::func func, min::gen arg,
      const min::phrase_position & pos,
      min::gen default_op,
      PAR::parser parser )
{
    min::uns32 errors = 0;
    min::obj_vec_ptr avp = arg;
    if ( avp == min::NULL_STUB )
    {
	PAR::parse_error
	    ( parser, pos,
	      "bad argument; argument ignored" );
	return 1;
    }
    min::uns32 asize = min::size_of ( avp );
    min::locatable_gen name, default_value;
    if ( asize == 3 && avp[1] == default_op )
    {
	name = avp[0];
	default_value = avp[2];
    }
    else
    {
	name = arg;
        avp = min::NULL_STUB;
	default_value = min::NONE();
    }
    if ( min::is_str ( name ) )
    {
        min::uns64 t =
	    ( 1ull << PAR::lexical_type_of ( name ) );
	if (   ( t & PRIM::var_initial_types
	           & PRIM::var_outside_quotes_types )
	     == 0 )
	{
	    PAR::parse_error
		( parser, pos,
		  "bad argument name; name ignored" );
	    name = min::MISSING();
	    ++ errors;
	}
    }
    else if ( ! min::is_obj ( name ) )
    {
	PAR::parse_error
	    ( parser, pos,
	      "bad argument name; name ignored" );
	name = min::MISSING();
	++ errors;
    }
    else
    {
        min::obj_vec_ptr nvp = name;
	min::uns32 ni = 0;
	name = PRIM::scan_var_name ( nvp, ni );
	if ( ni < min::size_of ( nvp ) )
	{
	    PAR::parse_error
		( parser, pos,
		  "bad argument name; name ignored" );
	    name = min::MISSING();
	    ++ errors;
	}
    }

    PRIM::push_arg ( name, default_value, func );

    return errors;
}

PRIM::func PRIM::scan_func_prototype
    ( min::obj_vec_ptr & vp, min::uns32 & i,
      min::phrase_position_vec ppvec,
      PAR::parser parser,
      TAB::flags selectors,
      min::uns16 level,
      min::uns16 depth,
      min::uns32 flags,
      min::uns32 location,
      min::gen module,
      min::gen default_op,
      min::gen bool_values,
      min::uns32 term_table_size )
{
    min::uns32 s = min::size_of ( vp );
    MIN_REQUIRE ( i < s );
    min::locatable_var<PRIM::func> func
        ( PRIM::create_func
	      ( selectors, ppvec[i],
	        level, depth,
		flags, location, module,
		term_table_size ) );
	// func->position.end is reset below.
    min::gen labbuf[s-i];
    min::uns32 j = 0;  // Indexes labbuf.
    bool label_done =  false;
        // Set true by first omitable arg list
	// or when state set to AFTER_SECOND_TERM.
    min::locatable_var<PRIM::func_term> func_term;
    enum state { BEFORE_FIRST_TERM, AFTER_FIRST_TERM,
                                    AFTER_SECOND_TERM };
    state st = BEFORE_FIRST_TERM;
    min::locatable_gen term_name ( min::NONE() );
    min::uns32 errors = 0;
    while ( i < s )
    {
	// Scan argument lists.
	//
	min::uns32 first_arg_list =
	    func->arg_lists->length;
	bool is_bool = false;
        while ( i < s && min::is_obj ( vp[i] ) )
	{
	    min::uns32 first = func->args->length;
	    min::uns32 number_required_args = 0;
	    min::locatable_gen brackets;
	    min::gen initiator =
	        min::get ( vp[i], min::dot_initiator );
	    if ( initiator == min::NONE() )
	        break;  // May be quoted string.
	    if ( initiator == PARLEX::left_parenthesis )
	        brackets = PRIMLEX::parentheses;
	    else
	    if ( initiator == PARLEX::left_square )
	        brackets = PRIMLEX::square_brackets;
	    else
	        break;  // May be : paragraph.
	    min::gen sep =
	        min::get ( vp[i], min::dot_separator );
	    if ( sep == min::NONE() )
	    {
	        errors += ::process_arg
		    ( func, vp[i], ppvec[i],
		      default_op, parser );
		if ( first + 1 == func->args->length )
		{
		    // Argument created without error.

		    min::obj_vec_ptr dvp =
			(func->args + first)->
			    default_value;
		    if ( dvp != min::NULL_STUB )
		    {
		        // Argument has default value.

			if (    func->arg_lists->length
			     == first_arg_list
		             &&
			     min::size_of ( dvp ) == 1
			     &&
		                min::labfind
		                   ( dvp[0],
				     bool_values )
		             >= 0 )
			    is_bool = true;
		    }
		    else
			number_required_args = 1;
		}
	    }
	    else // sep != NONE
	    {
		min::phrase_position_vec alppvec =
		    min::get
		        ( vp[i], min::dot_position );
			// Get must be before making
			// alvp.
		min::obj_vec_ptr alvp = vp[i];
	        min::uns32 alsize =
		    min::size_of ( alvp );

		for ( min::uns32 k = 0;
		      k < alsize; ++ k )
		    errors += ::process_arg
			( func, alvp[k], alppvec[k],
			  default_op, parser );

		bool default_found = false;
		for ( min::uns32 k = 0;
		      k < alsize; ++ k )
		{
		    PRIM::arg_struct arg =
		        func->args[first+k];
		    if (    arg.default_value
		         == min::NONE() )
		    {
		        ++ number_required_args;
		        if ( default_found )
			{
			    PAR::parse_error
			        ( parser,
				  alppvec[k],
				  "argment with NO"
				  " default found"
				  " AFTER argument with"
				  " default" );
			    ++ errors;
			    break;
			}
		    }
		    else
		        default_found = true;
		}
	    }
	    if ( number_required_args > 0 )
	    {
	        if ( ! label_done )
		    labbuf[j++] = brackets;
	    }
	    else
	        label_done = true;

	    min::uns32 number_of_args = 
	        func->args->length - first;
	    if ( number_of_args != 1 )
	        is_bool = false;
	    PRIM::push_arg_list
	        ( term_name, number_of_args, first,
		  number_required_args,
		  brackets, func );
	    term_name = min::NONE();
	    ++ i;
	}
	min::uns32 number_arg_lists =
	    func->arg_lists->length - first_arg_list;
	if ( number_arg_lists != 1 )
	    is_bool = false;

	min::uns32 term_begin = i;
	term_name = PRIM::scan_func_term_name ( vp, i );

	if ( st == BEFORE_FIRST_TERM )
	{
	    func->number_initial_arg_lists =
	        number_arg_lists;
	    if ( term_name == min::NONE() )
	    {
	        if ( j < 1 )
		{
		    PAR::parse_error
			( parser, func->position,
			  "function prototype"
			  " not found" );
		    return min::NULL_STUB;
		}
		func->position.end =
		    ( ppvec + (i-1))->end;

	        if ( j < 2 )
		{
		    PAR::parse_error
			( parser, func->position,
			  "function prototype"
			  " must have at least"
			  " 2 argument lists"
			  " or a function term" );
		    return min::NULL_STUB;
		}
		else
		if ( errors > 0 )
		    return min::NULL_STUB;

		min::locatable_gen label
		    ( min::new_lab_gen ( labbuf, j ) );
		    // j must be at least 2;
		PRIM::label_ref(func) = label;
		func->number_following_arg_lists = 0;
		return func;
	    }
	    else // term_name != NONE
	    if ( ! label_done )
	    {
		min::lab_ptr labp = term_name;
		if ( labp != min::NULL_STUB )
		{
		    min::uns32 len =
		        min::lablen ( labp );
		    for ( min::uns32 k = 0; k < len; )
			labbuf[j++] = labp[k++];
		}
		else
		    labbuf[j++] = term_name;
	    }
	    st = AFTER_FIRST_TERM;
	}
	else
	{
	    if ( st == AFTER_FIRST_TERM )
	    {
		func->number_following_arg_lists =
		    number_arg_lists;
		st = AFTER_SECOND_TERM;
		label_done = true;
	    }
	    else
	    {
		// st == AFTER_SECOND_TERM
		//
		func_term->first_arg_list =
		    first_arg_list;
		func_term->number_arg_lists =
		    number_arg_lists;
		func_term->is_bool = is_bool;
		func_term->position.end =
		    (ppvec + (term_begin-1))->end;
		if (    TAB::find_key_prefix
		            ( func_term->label,
			      func->term_table )
		     == min::NULL_STUB )
		    TAB::push
			( func->term_table,
			  (TAB::root) func_term );
		else
		{
		    PAR::parse_error
			( parser, func->position,
			  "function prototype has more"
			  " than one function term"
			  " named `",
			  min::pgen_name
			      ( func_term->label ),
			  "'" );
		    ++ errors;
		}
	    }
	    if ( term_name == min::NONE() )
	        break;

    	    func_term =
	        PRIM::create_func_term ( term_name );
	    func_term->position = ppvec[term_begin];
	}
    }

    min::locatable_gen label;
    if ( j > 1 )
	label = min::new_lab_gen ( labbuf, j );
    else
        label = labbuf[0];
    PRIM::label_ref(func) = label;

    if ( errors > 0 )
	return min::NULL_STUB;
    else
        return func;
}


static TAB::key_prefix find_key_prefix
    ( min::obj_vec_ptr & vp, min::uns32 & i,
      min::uns32 & quoted_i,
      TAB::key_table symbol_table )
{
    if ( symbol_table == min::NULL_STUB )
        return min::NULL_STUB;

    min::uns32 phash = min::labhash_initial;
    min::uns32 tab_len = symbol_table->length;
    min::uns32 mask = tab_len - 1;
    MIN_REQUIRE ( ( tab_len & mask ) == 0 );
    TAB::key_prefix previous = min::NULL_STUB;
    min::uns32 size = min::size_of ( vp );
    while ( i < size )
    {
        min::gen e = vp[i];
	if ( min::is_obj ( e ) )
	{
	    e = PAR::quoted_string_value ( e );
	    if ( e != min::NONE() )
	    {
		if ( i < quoted_i ) quoted_i = i;
	    }
	    else
	    {
	        min::gen initiator =
		    min::get
		        ( vp[i], min::dot_initiator );
		if (    initiator
		     == PARLEX::left_parenthesis )
		    e = PRIMLEX::parentheses;
		else
		if (    initiator
		     == PARLEX::left_square )
		    e = PRIMLEX::square_brackets;
		else
		if ( initiator == min::NONE()
		     &&
		     PAR::is_purelist ( vp[i] ) )
		    e = PRIMLEX::parentheses;
		else
		    break;
	    }
	}
	if ( ! min::is_name ( e ) )
	    break;
	min::uns32 hash = min::hash ( e );

	// Compute hash of this elements key prefix.
	//
	phash = min::labhash ( phash, hash );
	if ( previous != min::NULL_STUB ) hash = phash;

	// Locate key prefix.
	//
	TAB::key_prefix key_prefix =
	    symbol_table[hash & mask];
	while ( key_prefix != min::NULL_STUB )
	{
	    if ( key_prefix->key_element == e
	         &&
		 key_prefix->previous == previous )
		break;
	    key_prefix = key_prefix->next;
	}
	if ( key_prefix == min::NULL_STUB ) break;
	previous = key_prefix;
	++ i;
    }

    return previous;
}

bool PRIM::scan_primary
    ( min::obj_vec_ptr & vp, min::uns32 & i,
      min::phrase_position_vec ppvec,
      PAR::parser parser,
      TAB::flags selectors,
      TAB::key_prefix & key_prefix,
      TAB::root & root,
      min::uns32 & quoted_i,
      min::ref<argument_vector> argument_vector,
      TAB::key_table symbol_table,
      bool print_rejections,
      min::gen bool_values,
      min::gen negators )
{
    min::uns32 original_i = i;
    min::uns32 iend = min::size_of ( vp );

    if ( argument_vector == min::NULL_STUB )
        argument_vector = (PRIM::argument_vector)
	    min::gen_packed_vec_type.new_stub
	        ( iend - i );

    PRIM::argument_vector av = argument_vector;
	// Get rid of min::ref.
    min::pop ( av, av->length );

    if ( key_prefix == min::NULL_STUB )
    {
        quoted_i = iend;
    	for ( key_prefix =
	          ::find_key_prefix
		      ( vp, i, quoted_i, symbol_table );
	      key_prefix != min::NULL_STUB;
	      key_prefix = key_prefix->previous, -- i )
	for ( root = key_prefix->first;
	      root != min::NULL_STUB;
	      root = root->next )
	{
	    if ( root->selectors & selectors )
		goto CHECK_TYPE;
	}
	return false;
    }

RETRY:

    min::pop ( av, av->length );

    {
        MIN_REQUIRE ( root != min::NULL_STUB );
        while ( true )
	{
	    root = root->next;
	    while ( root == min::NULL_STUB )
	    {
	        key_prefix = key_prefix->previous;
		-- i;
		if ( key_prefix == min::NULL_STUB )
		    return false;
		root = key_prefix->first;
	    }

	    if ( root->selectors & selectors )
	        goto CHECK_TYPE;
	}
	MIN_REQUIRE ( false ); // Should not come here.
    }

CHECK_TYPE:

    min::uns32 after_lookup = i;

    PRIM::func func = (PRIM::func) root;
    if ( func == min::NULL_STUB )
    {
        // var found

        while ( i < iend )
	{
	    if (    min::get
	    		( vp[i], min::dot_initiator )
		 == PARLEX::left_square )
	        min::push(av) = vp[i++];
	    else
	    {
		i = after_lookup;
		goto RETRY;
		// Cannot go to REJECT because we
		// will cross variable allocations.
	    }
	}
        return true;
    }

    i = original_i;

    if ( (   func->flags
	   & (   PRIM::VALUE_OPERATOR
	       | PRIM::LOGICAL_OPERATOR ) )
	 && quoted_i >= after_lookup )
    {
        
	min::push(av) = vp[i + 0];
	min::push(av) = vp[i + 2];
        return func;
    }

    bool first = true;
    min::gen negator = min::NONE();
    bool is_bool = false;
    min::uns32 j = 0;
    min::uns32 jend =
        j + func->number_initial_arg_lists;
    min::gen args[func->args->length];
    for ( uns32 k = 0; k < func->args->length; ++ k )
        args[k] = min::NONE();
    min::uns32 number_args = func->args->length;
        // Will be shortened for operator calls.
    PRIM::func_term func_term = min::NULL_STUB;
    while ( true )
    {
#       define TERM ( func_term == min::NULL_STUB ? \
                      min::NONE() : func_term->label )
        bool arg_list_found = false;
	PRIM::arg_list_struct arg_list;
	    // For an is_bool term this will be the
	    // one and only arg_list;
        for ( ; j < jend; ++ j )
	{
	    arg_list = func->arg_lists[j];
#	    define VAR(k) min::pgen_name \
                ( (func->args + k)->name )
	    if ( i >= iend )
	    {
	        if (    arg_list.brackets
		     != PRIMLEX::parentheses )
		{
		    if ( print_rejections )
			::print_reject
			    ( parser, func,
			      "",
			      min::pgen_name
				 ( arg_list.brackets ),
			      " bracketed argument"
			      " list expected but end"
			      " of reference expression"
			      " found" );
		    goto REJECT;
		}
		else if ( first )
		{
		    if ( print_rejections )
			::print_reject
			    ( parser, func,
			      "( ) bracketed argument"
			      " list expected but end"
			      " of reference expression"
			      " found" );
		    goto REJECT;
		}
		continue;
	    }
	    min::gen initiator =
		min::get ( vp[i], min::dot_initiator );

	    if (    initiator 
		 == PARLEX::left_parenthesis )
	    {
	        if (    arg_list.brackets
		     != PRIMLEX::parentheses )
		{
		    if ( print_rejections )
			::print_reject
			    ( parser, func,
			      "",
			      min::pgen_name
				 ( arg_list.brackets ),
			      " bracketed argument"
			      " list expected but (...)"
			      " bracketed argument"
			      " list ",
			      min::pgen ( vp[i] ),
			      " found" );
		    goto REJECT;
		}
	    }
	    else if (    initiator 
		      == PARLEX::left_square )
	    {
	        if (    arg_list.brackets
		     == PRIMLEX::parentheses )
		{
		    if ( first )
		    {
			if ( print_rejections )
			    ::print_reject
				( parser, func,
				  "(...) argument list"
				  " expected before"
				  " first term but "
				  " non-()-bracketed "
				  " argument list ",
				  min::pgen ( vp[i] ),
				  " found" );
			goto REJECT;
		    }
		    continue;
		}
	    }
	    else if ( PAR::is_purelist ( vp[i] ) )
	    {
	        if (    arg_list.brackets
		     != PRIMLEX::parentheses )
		{
		    if ( print_rejections )
			::print_reject
			    ( parser, func,
			      "",
			      min::pgen_name
				 ( arg_list.brackets ),
			      " bracketed argument"
			      " list expected but"
			      " implicitly bracketed"
			      " argument list ",
			      min::pgen ( vp[i] ),
			      " found" );
		    goto REJECT;
		}
	    }
	    else
	    {
	        if (    arg_list.brackets
		     != PRIMLEX::parentheses )
		{
		    if ( print_rejections )
			::print_reject
			    ( parser, func,
			      "",
			      min::pgen_name
				 ( arg_list.brackets ),
			      " bracketed argument"
			      " list expected but"
			      " non-argument list ",
			      min::pgen ( vp[i] ),
			      " found" );
		    goto REJECT;
		}
		if ( first )
		{
		    if ( print_rejections )
			::print_reject
			    ( parser, func,
			      "(...) bracketed argument"
			      " list expected before"
			      " first function-term but"
			      " non-argument list ",
			      min::pgen ( vp[i] ),
			      " found" );
		    goto REJECT;
		}
		continue;
	    }

	    // Process actual argument list.
	    //
	    arg_list_found = true;
	    min::gen sep =
	        min::get ( vp[i], min::dot_separator );
		// Get before creating avp.
	    min::obj_vec_ptr avp = vp[i];
	    min::uns32 as = min::size_of ( avp );
	    if ( sep == min::NONE() )
	    {
	        if ( as == 0 )
		{
		    if ( first )
		    {
			if ( print_rejections )
			    ::print_reject
				( parser, func,
				  "empty argument list"
				  " found before first"
				  " function-term" );
			goto REJECT;
		    }
		    continue;
		}
		if (    args[arg_list.first]
		     != min::NONE() )
		{
		    if ( print_rejections )
			::print_reject
			    ( parser, func,
			      "prototype variable ",
			      VAR ( arg_list.first ),
			      " given two values" );
		    goto REJECT;
		}
		args[arg_list.first] = vp[i];
	    }
	    else
	    {
	        if ( as > arg_list.number_of_args )
		{
		    avp = min::NULL_STUB;
		    if ( ! print_rejections )
		        /* Do nothing */;
		    else if ( TERM != min::NONE() )
			::print_reject
			    ( parser, func,
			      "argument list ",
			      min::pgen ( vp[i] ),
			      " for function term ",
			      min::pgen_name ( TERM ),
			      " is too long" );
		    else if ( first )
			::print_reject
			    ( parser, func,
			      "argument list ",
			      min::pgen ( vp[i] ),
			      " before first function"
			      " term name is too"
			      " long" );
		    else
			::print_reject
			    ( parser, func,
			      "argument list ",
			      min::pgen ( vp[i] ),
			      " just after first"
			      " function term name"
			      " is too long" );
		    goto REJECT;
		}
		for ( min::uns32 k = 0; k < as; ++ k )
		{
		    if (    args[arg_list.first + k]
		         != min::NONE() )
		    {
			if ( print_rejections )
			    ::print_reject
				( parser, func,
				  "prototype variable ",
				  VAR (   arg_list.first
					+ k ),
				  " given two values" );
			goto REJECT;
		    }
		    args[arg_list.first+k] = avp[k];
		}
	    }
	    ++ i;

#	    undef VAR
	}

	if ( is_bool && ! arg_list_found )
	{
	    min::uns32 k =
	        ( negator != min::NONE() ? 0 : 1 );
	    args[arg_list.first] =
	        ::func_bool_expression[k];
	}
	else if ( negator != min::NONE() )
	{
	    if ( print_rejections )
		::print_reject
		    ( parser, func,
		      "bool term ",
		      min::pgen_name ( TERM ),
		      " has BOTH negator and"
		      " argument list" );
	    goto REJECT;
	}

	if ( first )
	{
	    MIN_ASSERT
	        ( i == original_i
		     + func->number_initial_arg_lists,
		  "number of argument lists before"
		  " first function term does not match"
		  " func->label" );
	    min::locatable_gen first_call_term_name
	        ( PRIM::scan_call_term_name ( vp, i ) );
	    MIN_ASSERT
	        ( first_call_term_name != min::NONE(),
		  "first call term name missing" );

	    jend = j + func->number_following_arg_lists;
	    first = false;
	    continue;
	}

	if ( i >= iend ) break;

	if ( min::labfind ( vp[i], negators ) >= 0 )
	{
	    negator = vp[i++];
	    if ( i >= iend )
	    {
		if ( print_rejections )
		    ::print_reject
			( parser, func,
			  "negator found just before"
			  " end of reference"
			  " expression" );
		goto REJECT;
	    }
	}
	else negator = min::NONE();

	min::uns32 quoted_ai = iend;  // Ignored.
    	TAB::key_prefix kp = ::find_key_prefix
	    ( vp, i, quoted_ai, func->term_table );

	if ( kp == min::NULL_STUB )
	{
	    if ( print_rejections )
		::print_reject
		    ( parser, func,
		      "expected function term just"
		      " after ",
		      min::pgen ( vp[i-1] ),
		      " but found ",
		      min::pgen ( vp[i] ),
		      " instead" );
	    goto REJECT;
	}

	func_term = (PRIM::func_term) kp->first;
	MIN_REQUIRE ( func_term != min::NULL_STUB );
	is_bool = func_term->is_bool;
	if ( negator != min::NONE() && ! is_bool )
	{
	    if ( print_rejections )
		::print_reject
		    ( parser, func,
		      "non-boolean function term ",
		      min::pgen_name ( TERM ),
		      " proceeded by a negator" );
	    goto REJECT;
	}

	j = func_term->first_arg_list;
	jend = j + func_term->number_arg_lists;

#       undef TERM
    }

    for ( min::uns32 k = 0; k < number_args; ++ k )
    {
        if ( args[k] == min::NONE() )
	{
	    PRIM::arg_struct arg = func->args[k];
	    args[k] = arg.default_value;
	    if ( args[k] == min::NONE() )
	    {
		if ( print_rejections )
		    ::print_reject
			( parser, func,
			  "prototype variable ",
			  min::pgen_name ( arg.name ),
			  " was not given a value" );
		goto REJECT;
	    }
	}
    }

    for ( min::uns32 k = 0; k < number_args; ++ k )
	min::push(av) = args[k];
	    // Push one at a time to update
	    // gc flags properly.

    return true;

REJECT:
    i = after_lookup;
    goto RETRY;
}



// Primary Parse Function
// ------- ----- --------

static void primary_parse ( PAR::parser parser,
		            PAR::pass pass,
		            TAB::flags selectors,
		            PAR::token & first,
		            PAR::token next )
{
    //  Everything is done in the Primary Parsing
    //  Functions.  Ideally the primary pass would
    //  have all-zero selectors, but just in case
    //  the following will do.
    //
    PAR::execute_pass_parse
        ( parser, pass, selectors, first, next );
}

// Primary Pass Command Function
// ------- ---- ------- --------

static min::gen primary_pass_command
	( PAR::parser parser,
	  PAR::pass pass,
	  min::obj_vec_ptr & vp, min::uns32 i0,
          min::phrase_position_vec ppvec )
{
    PRIM::primary_pass primary_pass =
        (PRIM::primary_pass) pass;

    min::uns32 size = min::size_of ( vp );

    min::uns32 i = i0;
        // vp[i] is next lexeme or subexpression to
	// scan in the define/undefine expression.

    min::gen command = vp[i++];

    if ( command != PARLEX::define
         &&
	 command != PARLEX::undefine
         &&
	 command != ::test
         &&
	 command != PARLEX::print )
        return min::FAILURE();

    if ( i >= size || vp[i++] != PRIMLEX::primary )
        return min::FAILURE();

    min::gen type = min::MISSING();
    if ( command != ::test )
    {
	if ( i >= size
	     ||
	     ( vp[i] != PRIMLEX::variable
	       &&
	       vp[i] != PRIMLEX::function ) )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "expected `variable' or `function'"
		  " after" );
	type = vp[i++];
    }

    if ( i >= size
	 ||
	 ! min::is_obj ( vp[i] )
	 ||
	    min::get ( vp[i], min::dot_initiator )
	 != ::opening_double_quote )
	return PAR::parse_error
	    ( parser, ppvec[i-1],
	      "expected ``...'' quoted expression"
	      " after" );

    min::phrase_position_vec nppvec =
	min::get ( vp[i], min::dot_position );
	// Must get before creating nvp.
    min::obj_vec_ptr nvp = vp[i];
    min::uns32 nsize = min::size_of ( nvp );
    min::uns32 ni = 0;
    ++ i;

    min::locatable_gen name = min::MISSING();
    if ( type == PRIMLEX::function )
    {
	name = PRIM::scan_func_label
	    ( nvp, ni, parser );
    }
    else if ( type == PRIMLEX::variable )
    {
        name = PRIM::scan_var_name ( nvp, ni );

	if ( ni < nsize )
	    return PAR::parse_error
		( parser, nppvec[ni],
		  "illegal name component" );
    }

    if ( name == min::NONE() )
    {
        if ( command == PARLEX::print )
	{
	    min::gen labbuf[1];
	    name = new_lab_gen ( labbuf, 0 );
	}
	else
	    return PAR::parse_error
		( parser, ppvec[i],
		  "``...'' quoted expression is"
		  " empty" );
    }


    if ( command == PARLEX::print )
    {
	min::uns32 indent =
	    COM::print_command ( parser, ppvec );

	parser->printer
	    << min::bom << min::no_auto_break
	    << min::set_indent ( indent + 4 );
	if ( type == PRIMLEX::function )
	    parser->printer
		<< min::indent << "Modifying Ops: "
		<< min::save_indent
		<< min::pgen
		       ( primary_pass->modifying_ops )
		<< min::restore_indent;

	int count = 0;

	{
	    TAB::key_table_iterator primary_it
		( primary_pass->primary_table );
	    while ( true )
	    {
		TAB::root root = primary_it.next();
		if ( root == min::NULL_STUB ) break;

		if ( min::is_subsequence
			 ( name, root->label ) < 0 )
		    continue;

		PRIM::var var = (PRIM::var) root;
		PRIM::func func = (PRIM::func) root;

		const char * type_name;
		min::uns32 flags;
		min::uns32 location;
		min::gen module;

		if ( type == PRIMLEX::variable )
		{
		    if ( var == min::NULL_STUB )
			continue;
		    type_name = "variable";
		    flags     = var->flags;
		    location  = var->location;
		    module    = var->module;
		}
		else // type == PRIMLEX::function
		{
		    if ( func == min::NULL_STUB )
			continue;
		    type_name = "function";
		    flags     = func->flags;
		    location  = func->location;
		    module    = func->module;
		}

		min::gen block_name =
		    PAR::block_name
			( parser,
			  root->block_level );
	        parser->printer
		        << min::indent
		        << "block "
		        << min::pgen_name
			       ( block_name )
		        << ": "
		        << min::save_indent
		        << type_name << " ``";

		if ( type == PRIMLEX::variable )
		    parser->printer << min::pgen_name
		        ( root->label );
		else // type == PRIMLEX::function
		    ::print_func
		        ( func, parser,
			  PRIM::func_default_op );

		parser->printer << "''";
		if (    (   root->selectors
		          & PAR::ALL_SELECTORS )
		     != PAR::ALL_SELECTORS )
		{
		    parser->printer
			    << " " << min::set_break;
		    COM::print_flags
			( root->selectors,
			  PAR::COMMAND_SELECTORS,
			  parser->selector_name_table,
			  parser );
		}
		if ( flags != 0 )
		    parser->printer
			<< " " << min::set_break
			<< "with flags "
			<< min::puns ( flags, "%8X" );
		if ( location != 0 )
		    parser->printer
			<< " " << min::set_break
			<< "with location "
			<< location;
		if ( module != min::MISSING() )
		    parser->printer
			<< " " << min::set_break
			<< "in module "
			<< min::pgen ( module );

		parser->printer << min::restore_indent;
		++ count;
	    }
	}

	if ( count == 0 )
	    parser->printer << min::indent
	                    << "nothing found";
	parser->printer << min::eom;

    	return PAR::PRINTED;
    }

    // Scan selectors.
    //
    TAB::flags selectors;
    min::gen sresult = COM::scan_flags
	    ( vp, i, selectors, PAR::COMMAND_SELECTORS,
	      parser->selector_name_table,
	      parser->selector_group_name_table,
	      parser );
    if ( sresult == min::ERROR() )
	return min::ERROR();
    else if ( sresult == min::FAILURE() )
	return PAR::parse_error
	    ( parser, ppvec[i-1],
	      "expected bracketed selector list"
	      " after" );
    else MIN_REQUIRE ( sresult == min::SUCCESS() );

    if ( command == PARLEX::define )
    {
	min::uns32 location = 0;
	min::locatable_gen module ( min::MISSING() );
	while ( i < size && vp[i] == PARLEX::with )
	{
	    ++ i;
	    if ( i < size
		 &&
		 vp[i] == PRIMLEX::location )
	    {
		++ i;
		if ( i >= size )
		    return PAR::parse_error
			( parser, ppvec[i-1],
			  "expected location integer"
			  " after" );
		min::float64 p =
		    MUP::direct_float_of ( vp[i] );
		if ( ! std::isfinite ( p )
		     ||
		     p < 0
		     ||
		     p > ( 1ull << 32 )
		     ||
		     (min::uns32) p != p )
		    return PAR::parse_error
			( parser, ppvec[i],
			  "location is not an integer"
			  " in range [0,2^32)" );
		location = (min::uns32) p;
		++ i;
		continue;
	    }
	    if ( i < size
		 &&
		 vp[i] == PRIMLEX::module )
	    {
		++ i;
		if ( i >= size )
		    return PAR::parse_error
			( parser, ppvec[i-1],
			  "expected module name"
			  " after" );
		if ( ! min::is_str ( vp[i] ) )
		    return PAR::parse_error
			( parser, ppvec[i],
			  "module name is not a"
			  " string" );
		module = vp[i];
		++ i;
		continue;
	    }

	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "expected `location' or"
		  " `module' after" );

	}
	if ( i < size )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "extra stuff after" );

	min::uns32 block_level =
	    PAR::block_level ( parser );

	if ( type == PRIMLEX::function )
	{
	    ni = 0;
	    min::locatable_var<PRIM::func> func =
		PRIM::scan_func_prototype
		  ( nvp, ni, nppvec, parser,
		    selectors,
		    0, block_level,
		    0, location, module );
	    if ( func == min::NULL_STUB )
	        return min::ERROR();
	    TAB::push
	        ( primary_pass->primary_table,
		  (TAB::root) func );
	}
	else
	{
	    min::locatable_var<PRIM::var> var
	        ( PRIM::create_var
		    ( name, selectors, nppvec->position,
		      0, block_level,
		      0, location, module ) );
	    PRIM::push_var
	        ( primary_pass->primary_table, var );
	}
    }

    else if ( command == PARLEX::undefine )
    {
	if ( i < size )
	    return PAR::parse_error
		( parser, ppvec[i-1],
		  "extra stuff after" );
	TAB::key_prefix key_prefix =
	    TAB::find_key_prefix
	        ( name,
		  primary_pass->primary_table );

	min::uns32 count = 0;

	if ( key_prefix != min::NULL_STUB )
	for ( TAB::root root = key_prefix->first;
	      root != min::NULL_STUB;
	      root = root->next )
	{
	    PRIM::var var = (PRIM::var) root;
	    PRIM::func func = (PRIM::func) root;
	    if ( type == PRIMLEX::variable ?
	         var == min::NULL_STUB :
	         func == min::NULL_STUB )
	        continue;
	        
	    if (    ( root->selectors & selectors )
		 == 0 )
		continue;

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
    else // if ( command == ::test )
    {
        TAB::root root = min::NULL_STUB;
        TAB::key_prefix key_prefix = min::NULL_STUB;
	min::locatable_var<PRIM::argument_vector>
	    argument_vector ( min::NULL_STUB );
	if ( ni >= nsize )
	    return PAR::parse_error
		( parser, nppvec->position,
		  "expression empty" );

        min::uns32 indent = min::print_line_column
	    ( ppvec->file, ppvec->position.begin,
	      parser->printer->print_format,
	      min::standard_line_format );
	parser->printer
	    << min::bom << min::set_indent ( indent )
	    << min::indent
	    << min::set_indent ( indent + 4 )
	    << "test primary: ";
	for ( min::uns32 j = 0; j < nsize; ++ j )
	    parser->printer
	        << min::space_if_after_indent
		<< min::pgen ( nvp[j] );
	parser->printer
	    << min::set_indent ( indent );

	min::uns32 quoted_i;
	bool found = PRIM::scan_primary
	    ( nvp, ni, nppvec, parser, selectors,
	      key_prefix, root, quoted_i,
	      argument_vector,
	      primary_pass->primary_table,
	      true );

	PRIM::var var = (PRIM::var) root;
	PRIM::func func = (PRIM::func) root;

	if ( ! found )
	    parser->printer
		<< min::indent
		<< "-- no definition found";
	else if ( var != min::NULL_STUB )
	{
	    parser->printer
		<< min::indent
		<< min::set_indent ( indent + 4 )
		<< "-- found variable: "
		<< min::pgen_name ( root->label)
		<< " ===> "
		<< min::pgen ( var->module )
		<< " "
		<< var->location;
	    if ( var->flags != 0 )
		parser->printer
		    << " "
		    << min::puns ( var->flags, "%8X" );
	}
	else if ( func != min::NULL_STUB )
	{
	    min::uns32 flags = func->flags;
	    parser->printer
		<< min::indent
		<< min::set_indent ( indent + 4 )
		<< ( flags & PRIM::VALUE_OPERATOR ?
		     "-- found value op: " :
		     flags & PRIM::LOGICAL_OPERATOR ?
		     "-- found logical op: " :
		     "-- found function: " );
	    min::uns32 len = func->arg_lists->length;
	    for ( min::uns32 j = 0; j < len; ++ j )
	    {
		PRIM::arg_list_struct arg_list =
			func->arg_lists[j];
		if ( arg_list.term_name != min::NONE() )
		    parser->printer
		        << min::space_if_after_indent
			<< min::pgen_name
			    ( arg_list.term_name );
		min::uns32 k = arg_list.first;
		min::uns32 kend =
		    k + arg_list.number_of_args;
		for ( ; k < kend; ++ k )
		    parser->printer
		        << min::space_if_after_indent
		        << min::pgen
			       ( argument_vector[k] );
	    }
	    if ( len == func->number_initial_arg_lists )
	    {
		min::lab_ptr lp = func->label;
		parser->printer <<
		    min::pgen_name ( lp[1] );
	    }
	    parser->printer
		<< " ===> "
		<< min::pgen ( func->module )
		<< " "
		<< func->location;
	    if ( func->flags != 0 )
		parser->printer
		    << " "
		    << min::puns ( func->flags, "%8X" );
	}

	parser->printer << min::eom;
	return PAR::PRINTED;
	    // Suppresses printing command again.
    }

    return min::SUCCESS();
}
