/****************************************************************************************************/

#include <adobe/bffd/parser.hpp>

#include <adobe/algorithm/sorted.hpp>
#include <adobe/implementation/token.hpp>
#include <adobe/array.hpp>
#include <adobe/string.hpp>

/****************************************************************************************************/

namespace {

/****************************************************************************************************/
#if 0
#pragma mark -
#endif
/*************************************************************************************************/

DEFINE_KEY(big);
DEFINE_KEY(const);
DEFINE_KEY(else);
DEFINE_KEY(float);
DEFINE_KEY(if);
DEFINE_KEY(invariant);
DEFINE_KEY(little);
DEFINE_KEY(notify);
DEFINE_KEY(signed);
DEFINE_KEY(skip);
DEFINE_KEY(struct);
DEFINE_KEY(tell);
DEFINE_KEY(unsigned);
DEFINE_KEY(while);

adobe::aggregate_name_t keyword_table[] =
{
    key_big,
    key_const,
    key_else,
    key_float,
    key_if,
    key_invariant,
    key_little,
    key_notify,
    key_signed,
    key_skip,
    key_struct,
    key_tell,
    key_unsigned,
    key_while,
};

/*************************************************************************************************/

bool keyword_lookup(const adobe::name_t& name)
{
#ifndef NDEBUG
    static bool inited_s = false;
    if (!inited_s)
    {
        assert(adobe::is_sorted(keyword_table));
        inited_s = true;
    }
#endif

    return binary_search(keyword_table, name, adobe::less(), adobe::constructor<adobe::name_t>()) != boost::end(keyword_table);
}

/****************************************************************************************************/

} // namespace

/****************************************************************************************************/

bffd_parser_t::bffd_parser_t(std::istream&                  in,
                             const adobe::line_position_t&  position,
                             const set_structure_proc_t&    set_structure_proc,
                             const add_field_proc_t&        add_field_proc,
                             const add_const_proc_t&        add_const_proc,
                             const add_invariant_proc_t&    add_invariant_proc,
                             const add_notify_proc_t&       add_notify_proc,
                             const add_skip_proc_t&         add_skip_proc) :
    adobe::expression_parser(in, position),
    set_structure_proc_m(set_structure_proc),
    add_field_proc_m(add_field_proc),
    add_const_proc_m(add_const_proc),
    add_invariant_proc_m(add_invariant_proc),
    add_notify_proc_m(add_notify_proc),
    add_skip_proc_m(add_skip_proc)
{
    if (!set_structure_proc_m)
        throw std::runtime_error("A set structure callback is required");

    if (!add_field_proc_m)
        throw std::runtime_error("An add field callback is required");

    if (!add_const_proc_m)
        throw std::runtime_error("An add const callback is required");

    if (!add_invariant_proc_m)
        throw std::runtime_error("An add invariant callback is required");

    if (!add_notify_proc_m)
        throw std::runtime_error("An add notify callback is required");

    if (!add_skip_proc_m)
        throw std::runtime_error("An add skip callback is required");

    set_keyword_extension_lookup(boost::bind(&keyword_lookup, _1));
}

/****************************************************************************************************/

void bffd_parser_t::parse()
{
    try
    {
        if (!is_struct_set())
            throw_exception("Format description must not be empty");

        require_token(adobe::eof_k);
    }
    catch (const adobe::stream_error_t& error)
    {
        // Necessary to keep stream_error_t from being caught by the next catch
        throw error;
    }
    catch (const std::exception& error)
    {
        putback();

        throw adobe::stream_error_t(error, next_position());
    }
}

/****************************************************************************************************/

bool bffd_parser_t::is_struct_set()
{
    if (!is_comment() && !is_struct())
        return false;

    while (is_struct_set())
    { }

    return true;
}

/****************************************************************************************************/

bool bffd_parser_t::is_comment()
{
    adobe::string_t comment;

    return is_trail_comment(comment) || is_lead_comment(comment);
}

/****************************************************************************************************/

bool bffd_parser_t::is_struct()
{
    if (!is_keyword(key_struct))
        return false;

    adobe::name_t structure_name;

    if (!is_identifier(structure_name))
        throw_exception("Structure identifier expected");

    require_token(adobe::open_brace_k);

    try
    {
        current_struct_m = structure_name;

        set_structure_proc_m(structure_name);
    }
    catch (const std::exception& error)
    {
        putback();
        throw adobe::stream_error_t(error, next_position());
    }

    is_statement_set();

    require_token(adobe::close_brace_k);

    return true;
}

/****************************************************************************************************/

bool bffd_parser_t::is_statement_set()
{
    if (!is_scope_or_statement())
        return false;

    while (is_statement_set())
    { }

    return true;
}

/****************************************************************************************************/

bool bffd_parser_t::is_scope_or_statement()
{
    return is_conditional_scope() || is_statement();
}

/****************************************************************************************************/

bool bffd_parser_t::is_conditional_scope()
{
    if (!is_if_scope())
        return false;

    is_comment();

    is_else_scope();

    return true;
}

/****************************************************************************************************/

bool bffd_parser_t::is_if_scope()
{
    if (!is_keyword(key_if))
        return false;

    adobe::array_t expression;

    require_expression(expression);

    static std::size_t          if_uid_s(0);
    static const adobe::array_t empty_array_k;

    std::stringstream lambda_identifier;
    lambda_identifier << ".if_" << ++if_uid_s;

    adobe::name_t       if_identifier(lambda_identifier.str().c_str());
    adobe::dictionary_t if_lambda_parameters;

    if_lambda_parameters[key_field_if_expression].assign(expression);
    if_lambda_parameters[key_field_conditional_type].assign(conditional_expression_t(if_k));
    if_lambda_parameters[key_field_name].assign(if_identifier);
    if_lambda_parameters[key_field_size_expression].assign(empty_array_k);
    if_lambda_parameters[key_field_while_expression].assign(empty_array_k);
    if_lambda_parameters[key_field_offset_expression].assign(empty_array_k);
    if_lambda_parameters[key_field_type].assign(value_field_type_struct);
    if_lambda_parameters[key_struct_type_name].assign(if_identifier);

    add_field_proc_m(if_identifier, if_lambda_parameters);

    require_scope_content(if_identifier);

    return true;
}

/****************************************************************************************************/

bool bffd_parser_t::is_else_scope()
{
    if (!is_keyword(key_else))
        return false;

    static std::size_t          else_uid_s(0);
    static const adobe::array_t empty_array_k;

    std::stringstream lambda_identifier;
    lambda_identifier << ".else_" << ++else_uid_s;

    adobe::name_t       else_identifier(lambda_identifier.str().c_str());
    adobe::dictionary_t else_lambda_parameters;

    else_lambda_parameters[key_field_if_expression].assign(empty_array_k);
    else_lambda_parameters[key_field_conditional_type].assign(conditional_expression_t(else_k));
    else_lambda_parameters[key_field_name].assign(else_identifier);
    else_lambda_parameters[key_field_size_expression].assign(empty_array_k);
    else_lambda_parameters[key_field_while_expression].assign(empty_array_k);
    else_lambda_parameters[key_field_offset_expression].assign(empty_array_k);
    else_lambda_parameters[key_field_type].assign(value_field_type_struct);
    else_lambda_parameters[key_struct_type_name].assign(else_identifier);

    add_field_proc_m(else_identifier, else_lambda_parameters);

    require_scope_content(else_identifier);

    return true;
}

/****************************************************************************************************/

void bffd_parser_t::require_scope_content(adobe::name_t scope_name)
{
    // Temporarily scope the lambda struct and add the statement sets it includes
    {
        temp_assignment<adobe::name_t> structure_name(current_struct_m, scope_name);

        set_structure_proc_m(current_struct_m);

        if (is_token(adobe::open_brace_k))
        {
            is_statement_set();

            require_token(adobe::close_brace_k);
        }
        else if (!is_scope_or_statement())
        {
            throw_exception("Expected scope or statement");
        }
    }

    // set back to our original structure in the client
    set_structure_proc_m(current_struct_m);
}

/****************************************************************************************************/

bool bffd_parser_t::is_statement()
{
    if (is_comment())
        return true;

    bool success(is_invariant() ||
                 is_constant()  ||
                 is_notify()    ||
                 is_skip()      ||
                 is_field()); // field should be last because atoms only
                              // require an expression which most everything
                              // falls into; the more explicit stuff should 
                              // come first.

    if (success)
        require_token(adobe::semicolon_k);

    return success;
}

/****************************************************************************************************/

bool bffd_parser_t::is_invariant()
{
    if (!is_keyword(key_invariant))
        return false;

    adobe::name_t name;

    if (is_identifier(name) == false)
        throw_exception("invariant identifier required");

    adobe::array_t expression;

    if (is_expression(expression) == false)
        throw_exception("invariant expression required");

    add_invariant_proc_m(name, expression);

    return true;
}

/****************************************************************************************************/

bool bffd_parser_t::is_constant()
{
    if (!is_keyword(key_const))
        return false;

    adobe::name_t name;

    if (is_identifier(name) == false)
        throw_exception("const identifier required");

    adobe::array_t expression;

    if (is_expression(expression) == false)
        throw_exception("const expression required");

    adobe::dictionary_t field_parameters;

    field_parameters[key_field_type].assign(value_field_type_const);
    field_parameters[key_field_name].assign(name);
    field_parameters[key_const_expression].assign(expression);

    add_const_proc_m(name, field_parameters);

    return true;
}

/****************************************************************************************************/

bool bffd_parser_t::is_notify()
{
    if (!is_keyword(key_notify))
        return false;

    adobe::array_t argument_list_expression;

    if (is_argument_list(argument_list_expression) == false)
        throw_exception("argument list required");

    adobe::dictionary_t field_parameters;

    field_parameters[key_field_type].assign(value_field_type_notify);
    field_parameters[key_field_name].assign(value_field_type_notify);
    field_parameters[key_notify_expression].assign(argument_list_expression);

    add_notify_proc_m(field_parameters);

    return true;
}

/****************************************************************************************************/

bool bffd_parser_t::is_skip()
{
    if (!is_keyword(key_skip))
        return false;

    adobe::name_t name;

    if (is_identifier(name) == false)
        throw_exception("skip identifier required");

    adobe::array_t expression;

    if (is_expression(expression) == false)
        throw_exception("skip expression required");

    adobe::dictionary_t field_parameters;

    field_parameters[key_field_type].assign(value_field_type_skip);
    field_parameters[key_field_name].assign(name);
    field_parameters[key_skip_expression].assign(expression);

    add_skip_proc_m(name, field_parameters);

    return true;
}

/****************************************************************************************************/

bool bffd_parser_t::is_field()
{
    adobe::name_t    struct_identifier;
    atom_base_type_t atom_type(atom_unknown_k);
    adobe::array_t   bit_count_expression;
    adobe::array_t   is_big_endian_expression;
    bool             struct_field(is_struct_field(struct_identifier));
    bool             atom_field(!struct_field &&
                                is_atom_field(atom_type,
                                                  bit_count_expression,
                                                  is_big_endian_expression));

    if (!struct_field && !atom_field)
        return false;

    adobe::name_t field_identifier;

    if (!is_identifier(field_identifier))
        throw_exception("Field identifier expected");

    adobe::array_t field_size_expression;
    bool           is_while(false);
    adobe::array_t offset_expression;
    adobe::array_t callback_expression;

    is_field_size(field_size_expression, is_while); // optional
    is_offset(offset_expression); // optional
    is_callback(callback_expression); // optional

    try
    {
        static const adobe::array_t empty_array_k;
        adobe::dictionary_t         field_parameters;

        field_parameters[key_field_name].assign(field_identifier);
        field_parameters[key_field_size_expression].assign(is_while ? empty_array_k : field_size_expression);
        field_parameters[key_field_while_expression].assign(is_while ? field_size_expression : empty_array_k);
        field_parameters[key_field_offset_expression].assign(offset_expression);
        field_parameters[key_field_callback].assign(callback_expression);

        // add the field to the current structure description
        if (struct_field)
        {
            field_parameters[key_field_type].assign(value_field_type_struct);
            field_parameters[key_struct_type_name].assign(struct_identifier);
        }
        else
        {
            field_parameters[key_field_type].assign(value_field_type_atom);
            field_parameters[key_atom_base_type].assign(atom_type);
            field_parameters[key_atom_bit_count_expression].assign(bit_count_expression);
            field_parameters[key_atom_is_big_endian_expression].assign(is_big_endian_expression);
        }

        add_field_proc_m(field_identifier, field_parameters);
    }
    catch (const std::exception& error)
    {
        putback();
        throw adobe::stream_error_t(error, next_position());
    }

    return true;
}

/****************************************************************************************************/

bool bffd_parser_t::is_struct_field(adobe::name_t& struct_identifier)
{
    if (!is_keyword(key_struct))
        return false;

    if (!is_identifier(struct_identifier))
        throw_exception("Structure identifier expected");

    return true;
}

/****************************************************************************************************/

bool bffd_parser_t::is_atom_field(atom_base_type_t& atom_base_type,
                                  adobe::array_t&   bit_count_expression,
                                  adobe::array_t&   is_big_endian_expression)
{
    static const adobe::array_t is_little_endian_expression_k(1, adobe::any_regular_t(false));
    static const adobe::array_t is_big_endian_expression_k(1, adobe::any_regular_t(true));

    if (is_keyword(key_signed))
        atom_base_type = atom_signed_k;
    else if (is_keyword(key_unsigned))
        atom_base_type = atom_unsigned_k;
    else if (is_keyword(key_float))
        atom_base_type = atom_float_k;
    else
        return false;

    require_expression(bit_count_expression);

    if (is_keyword(key_little))
        is_big_endian_expression = is_little_endian_expression_k;
    else if (is_keyword(key_big))
        is_big_endian_expression = is_big_endian_expression_k;
    else
        require_expression(is_big_endian_expression);

    return true;
}

/****************************************************************************************************/

bool bffd_parser_t::is_field_size(adobe::array_t& field_size_expression, bool& is_while)
{
    if (is_token(adobe::open_bracket_k))
    {
        if (!is_expression(field_size_expression))
            throw_exception("Field size cannot be empty");

        require_token(adobe::close_bracket_k);

        is_while = false;
    }
    else if (is_keyword(key_while))
    {
        if (!is_expression(field_size_expression))
            throw_exception("Field size cannot be empty");

        is_while = true;
    }
    else
    {
        return false;
    }

    return true;
}

/****************************************************************************************************/

bool bffd_parser_t::is_offset(adobe::array_t& offset_expression)
{
    if (!is_token(adobe::at_k))
        return false;

    if (!is_expression(offset_expression))
        throw_exception("Offset cannot be empty");

    return true;
}

/****************************************************************************************************/

bool bffd_parser_t::is_callback(adobe::array_t& callback_expression)
{
    static const adobe::array_t is_tell_expression_k(1, adobe::any_regular_t(true));

    bool success(true);

    if (is_keyword(key_tell))
        callback_expression = is_tell_expression_k;
    else if (is_expression(callback_expression))
        { }
    else
        success = false;

    return success;
}

/****************************************************************************************************/
