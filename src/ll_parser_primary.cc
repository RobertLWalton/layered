// Layered Languages Primary Parser Pass
//
// File:	ll_parser_primary.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Wed Nov  1 23:52:02 EDT 2023
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

static min::locatable_gen opening_double_quote;  // ``

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
    PRIMLEX::parentheses = min::new_str_gen ( "()" );
    PRIMLEX::square_brackets =
        min::new_str_gen ( "[]" );
    ::opening_double_quote = min::new_str_gen ( "``" );

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
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  min::uns32 level,
	  min::uns32 depth,
	  min::uns32 location,
	  min::gen module )
{
    min::locatable_var<PRIM::var> var
        ( ::var_type.new_stub() );

    PRIM::label_ref(var) = var_label;
    var->selectors = selectors;
    var->block_level = block_level;
    var->position = position;

    var->level = level;
    var->depth = depth;
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

static min::packed_vec <PRIM::arg_list_struct>
    arg_lists_type
        ( "ll::parser::primary::arg_lists_type" );
const min::uns32 & PRIM::ARG_LISTS =
    ::arg_lists_type.subtype;

PRIM::func PRIM::create_func
	( min::gen func_label,
	  TAB::flags selectors,
	  min::uns32 block_level,
	  const min::phrase_position & position,
	  min::uns32 level,
	  min::uns32 depth,
	  min::uns32 location,
	  min::gen module,
	  min::uns32 number_initial_arg_lists,
	  min::uns32 number_following_arg_lists,
	  min::uns32 term_table_size )
{
    min::locatable_var<PRIM::func> func
        ( ::func_type.new_stub() );

    PRIM::label_ref(func) = func_label;
    func->selectors = selectors;
    func->block_level = block_level;
    func->position = position;

    func->level = level;
    func->depth = depth;
    func->location = location;
    PRIM::module_ref(func) = module;

    PRIM::args_ref(func) =
        (PRIM::args) ::args_type.new_stub();
    PRIM::arg_lists_ref(func) =
        (PRIM::arg_lists) ::arg_lists_type.new_stub();
    PRIM::term_table_ref(func) =
        TAB::create_key_table ( term_table_size );

    func->number_initial_arg_lists =
        number_initial_arg_lists;
    func->number_following_arg_lists =
        number_following_arg_lists;

    return func;
}

static min::uns32 func_term_gen_disp[] = {
    min::DISP ( & PRIM::func_struct::label ),
    min::DISP_END };

static min::uns32 func_term_stub_disp[] = {
    min::DISP ( & PRIM::func_struct::next ),
    min::DISP_END };

static min::packed_struct_with_base
	<PRIM::func_term_struct, TAB::root_struct>
    func_term_type
        ( "ll::parser::primary::func_term_type",
	  ::func_term_gen_disp,
	  ::func_term_stub_disp );
const min::uns32 & PRIM::FUNC_TERM =
    ::func_term_type.subtype;

void PRIM::push_func_term
    ( min::gen func_term_label,
      const min::phrase_position & position,
      min::uns32 first_arg_list,
      min::uns32 number_arg_lists,
      bool is_bool,
      PRIM::func func )
{
    min::locatable_var<PRIM::func_term> func_term
        ( ::func_term_type.new_stub() );

    PRIM::label_ref(func_term) = func_term_label;
    func_term->selectors = PAR::ALL_SELECTORS;
    func_term->block_level = 0;
    func_term->position = position;

    func_term->first_arg_list =
        first_arg_list;
    func_term->number_arg_lists =
        number_arg_lists;
    func_term->is_bool = is_bool;

    TAB::push
        ( func->term_table, (TAB::root) func_term );
}

// Primary Parser Pass
// ------- ------ ----

static min::uns32 primary_pass_gen_disp[] =
{
    min::DISP ( & PRIM::primary_pass_struct::name ),
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
        ( primary_table, block_level - 1,
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



// Primary Parse Function
// ------- ----- --------

static void primary_parse ( PAR::parser parser,
		            PAR::pass pass,
		            TAB::flags selectors,
		            PAR::token & first,
		            PAR::token next )
{
    //  Everything is done in the Primary Parsing
    //  Functions.
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
	 command != PARLEX::print )
        return min::FAILURE();

    if ( i >= size || vp[i++] != PRIMLEX::primary )
        return min::FAILURE();

    if ( i >= size
         ||
	 ( vp[i] != PRIMLEX::variable
	   &&
	   vp[i] != PRIMLEX::function ) )
	return PAR::parse_error
	    ( parser, ppvec[i-1],
	      "expected `variable' or `function'"
	      " after" );
    min::gen type = vp[i++];

    min::locatable_gen name;

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

    min::obj_vec_ptr nvp = vp[i];
    min::uns32 ni = 0;
    min::phrase_position_vec nppvec =
	min::get ( vp[i], min::dot_position );
    ++ i;

    if ( type == PRIMLEX::function )
    {
	min::locatable_var<PRIM::variables_vector>
	    variables;
	name = PRIM::scan_func_label
	    ( nvp, ni, parser );
    }
    else // type == PRIMLEX::variable
    {
        name = PRIM::scan_var_name
	    ( nvp, ni, parser );

	if ( ni < min::size_of ( nvp ) )
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
		min::uns32 location;
		min::gen module;

		if ( type == PRIMLEX::variable )
		{
		    if ( var == min::NULL_STUB )
			continue;
		    type_name = "variable";
		    location = var->location;
		    module = var->module;
		}
		else // type == PRIMLEX::function
		{
		    if ( func == min::NULL_STUB )
			continue;
		    type_name = "function";
		    location = func->location;
		    module = func->module;
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
		        << type_name << " "
		        << min::pgen_name
			    ( root->label )
		        << " " << min::set_break;
	        COM::print_flags
		    ( root->selectors,
		      PAR::COMMAND_SELECTORS,
		      parser->selector_name_table,
		      parser );
	        parser->printer
		        << min::indent
			<< "with location "
			<< location
			<< " in module "
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
	min::uns32 level = 0;
	min::uns32 depth = 0;
	min::uns32 location = 0;
	min::locatable_gen module ( min::MISSING() );
	while ( i < size && vp[i] == PARLEX::with )
	{
	    ++ i;
	    if ( i < size
		 &&
		 vp[i] == PRIMLEX::level )
	    {
		++ i;
		if ( i >= size )
		    return PAR::parse_error
			( parser, ppvec[i-1],
			  "expected level integer"
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
			  "level is not an integer"
			  " in range [0,2^32)" );
		level = (min::uns32) p;
		++ i;
		continue;
	    }
	    if ( i < size
		 &&
		 vp[i] == PRIMLEX::depth )
	    {
		++ i;
		if ( i >= size )
		    return PAR::parse_error
			( parser, ppvec[i-1],
			  "expected depth integer"
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
			  "depth is not an integer"
			  " in range [0,2^32)" );
		depth = (min::uns32) p;
		++ i;
		continue;
	    }
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
	    min::locatable_var<PRIM::variables_vector>
		variables;
	    ni = 0;
	    min::locatable_var<PRIM::func> func =
		PRIM::scan_func_prototype
		  ( nvp, ni, parser, variables,
		    selectors, block_level,
		    nppvec->position,
		    level, depth,
		    location, module );
	    if ( func == min::NULL_STUB )
	        return min::ERROR();
	    TAB::push
	        ( primary_pass->primary_table,
		  (TAB::root) func );
	}
	else
	{
	    min::locatable_var<PRIM::var> var =
	        PRIM::create_var
		    ( name, selectors, block_level,
		      nppvec->position,
		      level, depth,
		      location, module );
	    TAB::push
	        ( primary_pass->primary_table,
		  (TAB::root) var );
	}
    }

    else // if ( command == PARLEX::undefine )
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

    return min::SUCCESS();
}
