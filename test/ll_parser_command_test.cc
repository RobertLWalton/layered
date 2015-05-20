// Layered Languages Parser Command Test
//
// File:	ll_parser_command_test.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon May 18 04:32:28 EDT 2015
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

# include <ll_parser.h>
# include <ll_parser_standard.h>
# include <iostream>
# include <cassert>
# define PAR ll::parser
# define PARSTD ll::parser::standard
using std::cout;

int main ( int argc, const char * argv[] )
{
    min::initialize();
    PAR::init ( PAR::default_parser, false );
    PARSTD::init_input ( PAR::default_parser );
    PARSTD::init_block ( PAR::default_parser );
    PAR::init_input_stream
        ( PAR::default_parser, std::cin );
    PAR::init_ostream
        ( PAR::default_parser, std::cout );
    PAR::init_line_display
        ( PAR::default_parser,
	  min::DISPLAY_PICTURE );
    PAR::default_parser->trace_flags |=
        PAR::TRACE_PARSER_COMMANDS;

# ifdef NONE_SUCH

    // Since subexpressions are not being parsed, we
    // change the gen_formats to not suppress spaces
    // between lexemes and not quote brackets.
    //
    min::gen_format altered_line_gen_format =
        * min::line_gen_format;
    min::obj_format altered_line_obj_format =
        * min::line_obj_format;
    min::gen_format altered_line_element_gen_format =
        * min::line_element_gen_format;
    min::obj_format altered_line_element_obj_format =
        * min::line_element_obj_format;

    altered_line_gen_format.obj_format =
        & altered_line_obj_format;
    altered_line_obj_format.top_element_format =
        & altered_line_element_gen_format;
    altered_line_element_gen_format.obj_format =
        & altered_line_element_obj_format;
    altered_line_element_gen_format.str_format =
        min::quote_non_graphic_str_format;
    altered_line_element_obj_format.obj_sep =
        (const min::ustring *) "\x01\x01" " ";

    PAR::default_parser->subexpression_gen_format =
        & altered_line_gen_format;

# endif // NONE_SUCH

    PAR::parse();
}
