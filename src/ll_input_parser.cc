// Layers Language Standard Input Parser
//
// File:	ll_input_parser.cc
// Author:	Bob Walton (walton@acm.org)
// Date:	Mon Dec 13 19:30:45 EST 2010
//
// The authors have placed this program in the public
// domain; they make no warranty and accept no liability
// for this program.

// Lines beginning with `    /// ' are the same as in
// the Standard Lexical Program appendix of ../doc/
// layered-introduction.tex.  As a consequence, this
// file uses 80 columns instead of the system default 56
// columns.

// Table of Contents
//
//	Usage and Setup
//	Input Parser Data
//	Input Parser

// Usage and Setup
// ----- --- -----

# include <ll_lexeme_standard.h>
# include <ll_parser_pass.h>
# define PARS ll::parser

// Input Parser Data
// ----- ------ ----

namespace ll { namespace parser {

struct input_parser : public pass_struct
{
    ll::lexeme::scanner_ptr scanner;
};

// Input Parser
// ----- ------

