/****************************************************************************************************/

#include <adobe/bffd/common.hpp>

#include <adobe/implementation/expression_parser.hpp>
#include <adobe/virtual_machine.hpp>

/****************************************************************************************************/

DEFINE_KEY(atom_base_type);
DEFINE_KEY(atom_bit_count_expression);
DEFINE_KEY(atom_is_big_endian_expression);
DEFINE_KEY(const_expression);
DEFINE_KEY(field_conditional_type);
DEFINE_KEY(field_if_expression);
DEFINE_KEY(field_name);
DEFINE_KEY(field_offset_expression);
DEFINE_KEY(field_callback);
DEFINE_KEY(field_size_expression);
DEFINE_KEY(field_type);
DEFINE_KEY(field_while_expression);
DEFINE_KEY(invariant_expression);
DEFINE_KEY(notify_expression);
DEFINE_KEY(skip_expression);
DEFINE_KEY(struct_type_name);
DEFINE_KEY(size); // while parameter
DEFINE_KEY(value); // while parameter
DEFINE_KEY(inclusive); // while parameter

DEFINE_VALUE(field_type_atom);
DEFINE_VALUE(field_type_const);
DEFINE_VALUE(field_type_invariant);
DEFINE_VALUE(field_type_notify);
DEFINE_VALUE(field_type_skip);
DEFINE_VALUE(field_type_struct);
DEFINE_VALUE(main);
DEFINE_VALUE(this);

/****************************************************************************************************/

namespace {

/****************************************************************************************************/

template <std::size_t ByteCount>
inline rawbytes_t stream_in(std::istream& input)
{
    rawbytes_t buffer(ByteCount, 0);

    input.read(reinterpret_cast<char*>(&buffer[0]), ByteCount);

    if (input.eof())
    {
        input.clear();
    }
    else if (input.fail())
    {
        input.clear();

        std::stringstream error;
        std::streamoff    offset(input.tellg());
        error << "Input stream fail; offset: " << offset << ", bytes: " << ByteCount;
        throw std::runtime_error(error.str());
    }

    return buffer;
}

inline rawbytes_t stream_in(std::istream& input, std::size_t byte_count)
{
    if (byte_count == 1)
        return stream_in<1>(input);
    else if (byte_count == 2)
        return stream_in<2>(input);
    else if (byte_count == 4)
        return stream_in<4>(input);
    else if (byte_count == 8)
        return stream_in<8>(input);

    std::stringstream error;
    error << "stream_in: byte count " << byte_count << " not supported.";
    throw std::runtime_error(error.str());
}

/****************************************************************************************************/

template <typename T>
inline adobe::any_regular_t convert_raw(const rawbytes_t& raw)
{
    T value(*reinterpret_cast<const T*>(&raw[0]));

    return adobe::any_regular_t(static_cast<double>(value));
}

inline adobe::any_regular_t convert_raw(const rawbytes_t& raw,
                                        std::size_t       byte_count,
                                        atom_base_type_t  base_type)
{
    if (base_type == atom_unknown_k)
        throw std::runtime_error("convert_raw: unknown atom base type");

    if (byte_count == 1)
    {
        if (base_type == atom_signed_k)
            return convert_raw<boost::int8_t>(raw);
        else if (base_type == atom_unsigned_k)
            return convert_raw<boost::uint8_t>(raw);
        else if (base_type == atom_float_k)
            throw std::runtime_error("convert_raw: 8-bit float atom not supported.");
    }
    else if (byte_count == 2)
    {
        if (base_type == atom_signed_k)
            return convert_raw<boost::int16_t>(raw);
        else if (base_type == atom_unsigned_k)
            return convert_raw<boost::uint16_t>(raw);
        else if (base_type == atom_float_k)
            throw std::runtime_error("convert_raw: 16-bit float atom not supported.");
    }
    else if (byte_count == 4)
    {
        BOOST_STATIC_ASSERT((sizeof(float) == 4));

        if (base_type == atom_signed_k)
            return convert_raw<boost::int32_t>(raw);
        else if (base_type == atom_unsigned_k)
            return convert_raw<boost::uint32_t>(raw);
        else if (base_type == atom_float_k)
            return convert_raw<float>(raw);
    }
    else if (byte_count == 8)
    {
        BOOST_STATIC_ASSERT((sizeof(double) == 8));

        if (base_type == atom_signed_k)
            return convert_raw<boost::int64_t>(raw);
        else if (base_type == atom_unsigned_k)
            return convert_raw<boost::uint64_t>(raw);
        else if (base_type == atom_float_k)
            return convert_raw<double>(raw);
    }

    std::stringstream error;
    error << "convert_raw: byte count " << byte_count << " not supported.";
    throw std::runtime_error(error.str());
}

/****************************************************************************************************/
#if 0
#pragma mark -
#endif
/****************************************************************************************************/

struct contextual_evaluation_engine_t
{
    contextual_evaluation_engine_t(const adobe::array_t& expression,
                                   inspection_branch_t   main_branch,
                                   inspection_branch_t   current_node,
                                   std::istream&         input);

    adobe::any_regular_t evaluate(bool finalize = true);

private:
    // evaluation vm custom callbacks
    adobe::any_regular_t named_index_lookup(const adobe::any_regular_t& value,
                                            adobe::name_t               name,
                                            bool                        throwing);
    adobe::any_regular_t numeric_index_lookup(const adobe::any_regular_t& value,
                                              std::size_t                 index);
    adobe::any_regular_t stack_variable_lookup(adobe::name_t name);
    adobe::any_regular_t array_function_lookup(adobe::name_t         name,
                                               const adobe::array_t& parameter_set);

    // helpers
    adobe::any_regular_t finalize_lookup(const adobe::any_regular_t& found);
    inspection_branch_t  regular_to_branch(const adobe::any_regular_t& name_or_branch);

    const adobe::array_t& expression_m;
    inspection_branch_t   main_branch_m;
    inspection_branch_t   current_node_m;
    std::istream&         input_m;
    bool                  finalize_m;
};

/****************************************************************************************************/

contextual_evaluation_engine_t::contextual_evaluation_engine_t(const adobe::array_t& expression,
                                                               inspection_branch_t   main_branch,
                                                               inspection_branch_t   current_node,
                                                               std::istream&         input) :
    expression_m(expression),
    main_branch_m(main_branch),
    current_node_m(current_node),
    input_m(input)
{ }

/****************************************************************************************************/

template <typename T, typename U>
void bad_cast_details(const adobe::bad_cast& error, const std::string& details)
{
    static const adobe::bad_cast test(adobe::type_info<T>(), adobe::type_info<U>());
    static const std::string     test_string(test.what());

    if (std::string(error.what()) == test_string)
        throw std::runtime_error(details);
}

/****************************************************************************************************/

adobe::any_regular_t contextual_evaluation_engine_t::evaluate(bool finalize)
try
{
    adobe::virtual_machine_t vm;

    finalize_m = finalize;

    vm.set_variable_lookup(boost::bind(&contextual_evaluation_engine_t::stack_variable_lookup,
                                       boost::ref(*this), _1));
    vm.set_named_index_lookup(boost::bind(&contextual_evaluation_engine_t::named_index_lookup,
                                          boost::ref(*this), _1, _2, true));
    vm.set_numeric_index_lookup(boost::bind(&contextual_evaluation_engine_t::numeric_index_lookup,
                                            boost::ref(*this), _1, _2));
    vm.set_array_function_lookup(boost::bind(&contextual_evaluation_engine_t::array_function_lookup,
                                             boost::ref(*this), _1, _2));

    vm.evaluate(expression_m);

    adobe::any_regular_t result(vm.back());

    vm.pop_back();

    return result;
}
catch (const adobe::bad_cast& error)
{
    // If in the course of eval we get a bad cast from double to name we can
    // deduce the user might have forgotten an @ symbol when appropriate, e.g.,
    //     str(my_field) instead of str(@my_field)
    // Another possible error is when a user tries to fetch the subfield of
    // an address, which is not allowed:
    //     @field.subfield
    // Another possible error is when an address is passed when a value is
    // expected, a la
    //     byte(@field), which expects an offset instead of an address

    std::string path(build_path(main_branch_m, current_node_m));

    bad_cast_details<double, adobe::name_t>(error, "Address failure -- possible '@' missing in " + path);
    bad_cast_details<adobe::name_t, inspection_branch_t>(error, "Address failure -- possible '@' extra in " + path);
    bad_cast_details<inspection_branch_t, double>(error, "Address failure -- possible '@' extra in " + path);

    throw error;
}
catch (const std::exception& error)
{
    std::stringstream error_details;
    error_details << error.what() << " in " << build_path(main_branch_m, current_node_m);
    throw std::runtime_error(error_details.str());
}

/****************************************************************************************************/

adobe::any_regular_t contextual_evaluation_engine_t::finalize_lookup(const adobe::any_regular_t& found)
{
    // converts our branch into its value when its an atom or a const.
    // can be bypassed by passing false to evaluate(), in which case
    // the evaluated result is always an inspection branch.

    if (!finalize_m)
        return found;

    inspection_branch_t branch(found.cast<inspection_branch_t>());

    if (branch->get_flag(is_array_root_k) ||
        node_property(branch, NODE_PROPERTY_IS_STRUCT))
    {
        return found;
    }
    else if (node_property(branch, NODE_PROPERTY_IS_ATOM))
    {
        atom_base_type_t      base_type(node_property(branch, ATOM_PROPERTY_BASE_TYPE));
        bool                  is_big_endian(node_property(branch, ATOM_PROPERTY_IS_BIG_ENDIAN));
        boost::uint64_t       bit_count(node_property(branch, ATOM_PROPERTY_BIT_COUNT));
        inspection_position_t position(node_value(branch, ATOM_VALUE_LOCATION));

        return fetch_and_evaluate(input_m, position, bit_count, base_type, is_big_endian);
    }
    else if (node_property(branch, NODE_PROPERTY_IS_CONST))
    {
        if (node_value(branch, CONST_VALUE_IS_EVALUATED) == false)
        {
            const adobe::array_t& const_expression(node_value(branch, CONST_VALUE_EXPRESSION));

            // Note (fbrereto): Evaluate at the point of the const declaration,
            //                  not at the current branch.
            branch->evaluated_value_m = contextual_evaluation_of<adobe::any_regular_t>(const_expression,
                                                                                       main_branch_m,
                                                                                       adobe::find_parent(branch),
                                                                                       input_m);
            branch->evaluated_m = true;
        }

        return node_value(branch, CONST_VALUE_EVALUATED_VALUE);
    }

    return found;
}

/****************************************************************************************************/

adobe::any_regular_t contextual_evaluation_engine_t::named_index_lookup(const adobe::any_regular_t& value,
                                                                        adobe::name_t               name,
                                                                        bool                        throwing)
{
    inspection_branch_t                 branch(value.cast<inspection_branch_t>());
    inspection_forest_t::child_iterator iter(adobe::child_begin(branch));
    inspection_forest_t::child_iterator last(adobe::child_end(branch));

    for (; iter != last; ++iter)
        if (iter->name_m == name)
            return finalize_lookup(adobe::any_regular_t(inspection_branch_t(iter.base())));

    if (throwing)
    {
        std::stringstream error;
        error << "Subfield '" << name << "' not found.";
        throw std::runtime_error(error.str());
    }

    return adobe::any_regular_t();
}

/****************************************************************************************************/

adobe::any_regular_t contextual_evaluation_engine_t::numeric_index_lookup(const adobe::any_regular_t& value,
                                                                          std::size_t                 index)
{
    inspection_branch_t branch(value.cast<inspection_branch_t>());

    if (!adobe::has_children(branch))
        throw std::range_error("Array is empty");

    inspection_forest_t::child_iterator iter(adobe::child_begin(branch));
    inspection_forest_t::child_iterator last(adobe::child_end(branch));
    std::size_t                         i(0);

    while (true)
    {
        if (i == index)
            return finalize_lookup(adobe::any_regular_t(inspection_branch_t(iter.base())));

        if (++iter == last)
        {
            std::stringstream error;
            error << "Array index " << index << " out of range [ 0 .. " << i << " ]";
            throw std::range_error(error.str());
        }

        ++i;
    }

    return adobe::any_regular_t();
}

/****************************************************************************************************/

adobe::any_regular_t contextual_evaluation_engine_t::stack_variable_lookup(adobe::name_t name)
{
    inspection_branch_t current_node(current_node_m);

    if (name == value_main)
        return adobe::any_regular_t(main_branch_m);
    else if (name == value_this)
        return adobe::any_regular_t(current_node);

    while (true)
    {
        adobe::any_regular_t subfield(named_index_lookup(adobe::any_regular_t(current_node),
                                                         name,
                                                         false));

        // named_index_lookup handles finalization
        if (subfield != adobe::any_regular_t())
            return subfield;

        current_node.edge() = adobe::forest_leading_edge;

        if (current_node == main_branch_m)
            break;

        current_node = adobe::find_parent(current_node);
    }

    std::stringstream error;
    error << "Subfield '" << name << "' not found at or above " << build_path(main_branch_m, current_node_m);
    throw std::runtime_error(error.str());
}

/****************************************************************************************************/

inspection_branch_t contextual_evaluation_engine_t::regular_to_branch(const adobe::any_regular_t& name_or_branch)
{
    if (name_or_branch.type_info() == adobe::type_info<inspection_branch_t>())
    {
        return name_or_branch.cast<inspection_branch_t>();
    }
    else if (name_or_branch.type_info() == adobe::type_info<adobe::name_t>())
    {
        // Otherwise, converts a user-specified name-as-path to an inspection branch

        std::stringstream      input(name_or_branch.cast<adobe::name_t>().c_str());
        adobe::line_position_t position(__FILE__, __LINE__);
        adobe::array_t         expression;

        adobe::expression_parser(input, position).require_expression(expression);

        return contextual_evaluation_of<inspection_branch_t>(expression,
                                                             main_branch_m,
                                                             current_node_m,
                                                             input_m);
    }

    throw std::runtime_error("Expected but was not passed ref(@field) or @field.");
}

/****************************************************************************************************/

adobe::any_regular_t contextual_evaluation_engine_t::array_function_lookup(adobe::name_t         name,
                                                                           const adobe::array_t& parameter_set)
{
    DEFINE_VALUE(byte);
    DEFINE_VALUE(sizeof);
    DEFINE_VALUE(startof);
    DEFINE_VALUE(endof);
    DEFINE_VALUE(card);
    DEFINE_VALUE(str);
    DEFINE_VALUE(path);
    DEFINE_VALUE(indexof);

    if (name == value_sizeof)
    {
        if (parameter_set.empty())
            throw std::runtime_error("sizeof(): @field_name expected");

        boost::uint64_t start_offset(0);
        boost::uint64_t end_offset(-1);

        if (parameter_set.size() == 1)
        {
            inspection_branch_t leaf(regular_to_branch(parameter_set[0]));
            
            start_offset = starting_offset_for(leaf);
            end_offset = ending_offset_for(leaf);
        }
        else if (parameter_set.size() == 2)
        {
            inspection_branch_t leaf1(regular_to_branch(parameter_set[0]));
            inspection_branch_t leaf2(regular_to_branch(parameter_set[1]));
            
            start_offset = starting_offset_for(leaf1);
            end_offset = ending_offset_for(leaf2);
        }
        else
        {
            throw std::runtime_error("sizeof(): too many parameters");
        }

        boost::uint64_t size(end_offset - start_offset + 1);

        return adobe::any_regular_t(static_cast<double>(size));
    }
    else if (name == value_startof)
    {
        if (parameter_set.empty())
            throw std::runtime_error("startof(): @field_name expected");

        inspection_branch_t leaf(regular_to_branch(parameter_set[0]));
        boost::uint64_t     start_offset(starting_offset_for(leaf));

        return adobe::any_regular_t(static_cast<double>(start_offset));
    }
    else if (name == value_endof)
    {
        if (parameter_set.empty())
            throw std::runtime_error("endof(): @field_name expected");

        inspection_branch_t leaf(regular_to_branch(parameter_set[0]));
        boost::uint64_t     end_offset(ending_offset_for(leaf));

        return adobe::any_regular_t(static_cast<double>(end_offset));
    }
    else if (name == value_byte)
    {
        if (parameter_set.empty())
            throw std::runtime_error("byte(): offset expected");

        std::streamoff offset(static_cast<std::streamoff>(parameter_set[0].cast<double>()));
        unsigned char  buffer[5];

        input_m.seekg(offset);
        input_m.read(reinterpret_cast<char*>(&buffer[0]), 1);

        return adobe::any_regular_t(static_cast<double>(buffer[0]));
    }
    else if (name == value_card)
    {
        if (parameter_set.empty())
            throw std::runtime_error("card(): @field_name expected");

        inspection_branch_t array(regular_to_branch(parameter_set[0]));

        if (!array->get_flag(is_array_root_k))
            throw std::runtime_error("card(): field is not an array");

        return adobe::any_regular_t(static_cast<double>(node_property(array, ARRAY_ROOT_PROPERTY_SIZE)));
    }
    else if (name == value_str)
    {
        if (parameter_set.empty())
            throw std::runtime_error("str(): @field_name expected");

        inspection_branch_t leaf(regular_to_branch(parameter_set[0]));
        boost::uint64_t     start_offset(starting_offset_for(leaf));
        boost::uint64_t     end_offset(ending_offset_for(leaf));
        boost::uint64_t     size(end_offset - start_offset + 1);
        std::vector<char>   str(static_cast<std::size_t>(size), 0);

        if (node_property(leaf, NODE_PROPERTY_IS_CONST))
            throw std::runtime_error("str(): cannot take the string of a const");

        input_m.seekg(static_cast<std::streamoff>(start_offset));
        input_m.read(&str[0], str.size());

        return adobe::any_regular_t(adobe::string_t(str.begin(), str.end()));
    }
    else if (name == value_path)
    {
        adobe::any_regular_t path(adobe::static_name_t("this"));

        if (!parameter_set.empty())
            path = parameter_set[0];

        inspection_branch_t leaf(regular_to_branch(path));

        return adobe::any_regular_t(adobe::string_t(build_path(main_branch_m, leaf)));
    }
    else if (name == value_indexof)
    {
        adobe::any_regular_t path(adobe::static_name_t("this"));

        if (!parameter_set.empty())
            path = parameter_set[0];

        inspection_branch_t leaf(regular_to_branch(path));

        if (!leaf->get_flag(is_array_element_k))
        {
            adobe::name_t     name(node_property(leaf, NODE_PROPERTY_NAME));
            std::stringstream error;
            error << "indexof(): field '" << name << "' is not an array element";
            throw std::runtime_error(error.str());
        }

        return adobe::any_regular_t(static_cast<double>(node_value(leaf, ARRAY_ELEMENT_VALUE_INDEX)));
    }

    std::stringstream error;
    error << "Function '" << name << "' not found";
    throw std::runtime_error(error.str());
}

/****************************************************************************************************/

} // namespace

/****************************************************************************************************/
#if 0
#pragma mark -
#endif
/****************************************************************************************************/

rawbytes_t fetch(std::istream&                input,
                 const inspection_position_t& location,
                 boost::uint64_t              bit_count)
{
    if (location.bit_offset_m != 0)
        throw std::runtime_error("bit-level fetching not implemented");

    input.seekg(static_cast<std::streamoff>(location.byte_offset_m));

    return stream_in(input, static_cast<std::size_t>(bit_count / 8));
}

/****************************************************************************************************/

adobe::any_regular_t evaluate(const rawbytes_t& raw,
                              boost::uint64_t   bit_count,
                              atom_base_type_t  base_type,
                              bool              is_big_endian)
{
    rawbytes_t  byte_set(raw);

#if __LITTLE_ENDIAN__
    if (is_big_endian)
        std::reverse(byte_set.begin(), byte_set.end());
#endif
#if __BIG_ENDIAN__
    if (!is_big_endian)
        std::reverse(byte_set.begin(), byte_set.end());
#endif

    return convert_raw(byte_set, static_cast<std::size_t>(bit_count / 8), base_type);
}

/****************************************************************************************************/

adobe::any_regular_t fetch_and_evaluate(std::istream&                input,
                                        const inspection_position_t& location,
                                        boost::uint64_t              bit_count,
                                        atom_base_type_t             base_type,
                                        bool                         is_big_endian)
{
    return evaluate(fetch(input, location, bit_count), bit_count, base_type, is_big_endian);
}

/****************************************************************************************************/

template <>
adobe::any_regular_t contextual_evaluation_of(const adobe::array_t& expression,
                                              inspection_branch_t   main_branch,
                                              inspection_branch_t   current_node,
                                              std::istream&         input)
{
    return contextual_evaluation_engine_t(expression, main_branch, current_node, input).evaluate();
}

template <>
inspection_branch_t contextual_evaluation_of(const adobe::array_t& expression,
                                             inspection_branch_t   main_branch,
                                             inspection_branch_t   current_node,
                                             std::istream&         input)
{
    return contextual_evaluation_engine_t(expression, main_branch, current_node, input).evaluate(false).cast<inspection_branch_t>();
}

/****************************************************************************************************/

boost::uint64_t starting_offset_for(inspection_branch_t branch)
{
    bool is_struct(node_property(branch, type_struct_k));
    bool is_array_root(branch->get_flag(is_array_root_k));

    if (is_struct || is_array_root)
        return branch->start_offset_m;

    inspection_position_t position(node_value(branch, ATOM_VALUE_LOCATION));

    return position.byte_offset_m;
}

/****************************************************************************************************/

boost::uint64_t ending_offset_for(inspection_branch_t branch)
{
    bool is_struct(node_property(branch, type_struct_k));
    bool is_array_root(branch->get_flag(is_array_root_k));

    if (is_struct || is_array_root)
        return branch->end_offset_m;

    inspection_position_t position(node_value(branch, ATOM_VALUE_LOCATION));
    boost::uint64_t       bit_count(node_property(branch, ATOM_PROPERTY_BIT_COUNT));

    return (position.byte_offset_m + bit_count / 8) - 1;
}

/****************************************************************************************************/

std::string build_path(const_inspection_branch_t main, const_inspection_branch_t current)
{
#if !BOOST_WINDOWS
    using std::isalpha;
#endif

    std::string result;

    while (true)
    {
        std::string path_segment;
        bool        is_array_element(current->get_flag(is_array_element_k));

        if (is_array_element)
        {
            std::stringstream indexstr;
            boost::uint64_t   index(node_value(current, ARRAY_ELEMENT_VALUE_INDEX));

            indexstr << '[' << index << ']';

            path_segment = indexstr.str();
        }
        else
        {
            adobe::name_t name(node_property(current, NODE_PROPERTY_NAME));

            if (name)
                path_segment = name.c_str();
        }

        if (!path_segment.empty())
        {
            if (!result.empty() && isalpha(result[0]))
                result.insert(0, std::string("."));

            result.insert(0, path_segment);
        }

        current.edge() = adobe::forest_leading_edge;

        if (current == main)
            break;

        current = adobe::find_parent(current);
    }

    return result;
}

/****************************************************************************************************/
