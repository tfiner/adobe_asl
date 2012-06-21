/****************************************************************************************************/

#ifndef BFFD_PARSER_HPP
#define BFFD_PARSER_HPP

#include <adobe/config.hpp>

#if BOOST_WINDOWS
    #include <functional>
#else
    #include <tr1/functional>
#endif

#include <adobe/implementation/expression_parser.hpp>

#include <adobe/bffd/common.hpp>

/****************************************************************************************************/
/*
    translation_unit   = { struct_set }
    struct_set         = [ comment | struct ] { struct_set }
    comment            = trail_comment | lead_comment
    struct             = "struct" identifier '{' { statement_set } '}'
    statement_set      = scope_or_statement { statement_set }
    scope_or_statement = conditional_scope | statement
    conditional_scope  = if_scope { comment } { else_scope }
    if_scope           = "if" expression scope_content
    else_scope         = "else" scope_content
    scope_content      = '{' { statement_set } '}' | scope_or_statement
    statement          = comment | [ invariant | constant | notify | skip | field ] ';'
    invariant          = "invariant" identifier expression
    constant           = "const" identifier expression
    notify             = "notify" argument_list
    skip               = "skip" identifier expression
    field              = [ struct_field | atom_field ] identifier { field_size } { offset } { callback }
    struct_field       = "struct" identifier
    atom_field         = [ "float" | "unsigned" | "signed" ] expression [ "big" | "little" | expression ]
    field_size         = '[' expression ']' | "while" expression
    offset             = '@' expression
    callback           = "tell" | expression
*/
/****************************************************************************************************/

class bffd_parser_t : protected adobe::expression_parser
{
public:
    typedef std::tr1::function<void (adobe::name_t)>              set_structure_proc_t;
    typedef std::tr1::function<void (adobe::name_t,
                                     const adobe::dictionary_t&)> add_field_proc_t;
    typedef std::tr1::function<void (adobe::name_t,
                                     const adobe::dictionary_t&)> add_const_proc_t;
    typedef std::tr1::function<void (adobe::name_t,
                                     const adobe::array_t&)>      add_invariant_proc_t;
    typedef std::tr1::function<void (const adobe::dictionary_t&)> add_notify_proc_t;
    typedef std::tr1::function<void (adobe::name_t,
                                     const adobe::dictionary_t&)> add_skip_proc_t;

    bffd_parser_t(std::istream&                  in,
                  const adobe::line_position_t&  position,
                  const set_structure_proc_t&    set_structure_proc,
                  const add_field_proc_t&        add_field_proc,
                  const add_const_proc_t&        add_const_proc,
                  const add_invariant_proc_t&    add_invariant_proc,
                  const add_notify_proc_t&       add_notify_proc,
                  const add_skip_proc_t&         add_skip_proc);

//  translation_unit = { struct_set }.
    void parse();

private:
//  struct_set         = [ comment | struct ] { struct_set }
    bool is_struct_set();

//  comment            = [ trail_comment | lead_comment ]
    bool is_comment();

//  struct             = "struct" identifier '{' [ statement_set ] '}'
    bool is_struct();

//  statement_set      = scope_or_statement { statement_set }
    bool is_statement_set();

//  scope_or_statement = conditional_scope | statement
    bool is_scope_or_statement();

//  conditional_scope  = if_scope { comment } { else_scope }
    bool is_conditional_scope();

//  if_scope           = "if" expression scope_content
    bool is_if_scope();

//  else_scope         = "else" scope_content
    bool is_else_scope();

//  scope_content      = '{' { statement_set } '}' | scope_or_statement
    void require_scope_content(adobe::name_t scope_name);

//  statement          = [ comment | [ invariant | constant | notify | skip | field ] ';' ]
    bool is_statement();

//  invariant          = "invariant" expression
    bool is_invariant();

//  constant           = "const" identifier expression
    bool is_constant();

//  notify             = "notify" argument_list
    bool is_notify();

//  skip               = "skip" expression
    bool is_skip();

//  field              = [ struct_field | atom_field ] identifier { field_size } { offset }
    bool is_field();

//  struct_field       = "struct" identifier
    bool is_struct_field(adobe::name_t& struct_identifier);

//  atom_field         = [ "float" | "unsigned" | "signed" ] expression [ "big" | "little" | expression ]
    bool is_atom_field(atom_base_type_t& atom_base_type,
                       adobe::array_t&   bit_count_expression,
                       adobe::array_t&   is_big_endian_expression);

//  field_size         = '[' expression ']' | "while" expression
    bool is_field_size(adobe::array_t& field_size_expression, bool& is_while);

//  offset             = '@' expression
    bool is_offset(adobe::array_t& offset_expression);

//  callback           = "tell" | expression
    bool is_callback(adobe::array_t& callback_expression);

    set_structure_proc_t    set_structure_proc_m;
    add_field_proc_t        add_field_proc_m;
    add_const_proc_t        add_const_proc_m;
    add_invariant_proc_t    add_invariant_proc_m;
    add_notify_proc_t       add_notify_proc_m;
    add_skip_proc_t         add_skip_proc_m;
    adobe::name_t           current_struct_m;
};

/****************************************************************************************************/
// BFFD_PARSER_HPP
#endif

/****************************************************************************************************/
