/****************************************************************************************************/

#include <adobe/bffd/analyzer.hpp>

/****************************************************************************************************/
#if 0
#pragma mark -
#endif
/****************************************************************************************************/

namespace {

/****************************************************************************************************/

class analysis_error_t : public std::runtime_error
{
    typedef std::runtime_error _super;

public:
    explicit analysis_error_t(const std::string& error) : 
        _super(error)
    { }
};

/****************************************************************************************************/

template <typename T>
const T& value_for(const adobe::dictionary_t& dict, adobe::name_t key)
{
    adobe::dictionary_t::const_iterator found(dict.find(key));

    if (found == dict.end())
    {
        std::stringstream error;
        error << "Key " << key << " not found";
        throw std::runtime_error(error.str());
    }

    return found->second.cast<T>();
}

/****************************************************************************************************/

template <typename T>
const T& value_for(const adobe::dictionary_t& dict, adobe::name_t key, const T& default_value)
{
    adobe::dictionary_t::const_iterator found(dict.find(key));

    return found == dict.end() ? default_value : found->second.cast<T>();
}

/****************************************************************************************************/

void throw_duplicate_field_name(adobe::name_t name)
{
    std::stringstream error;
    error << "Duplicate field name '" << name << "'";
    throw std::runtime_error(error.str());
}

bool has_field_named(const adobe::array_t& structure,
                     adobe::name_t         field)
{
    for (adobe::array_t::const_iterator iter(structure.begin()), last(structure.end()); iter != last; ++iter)
    {
        const adobe::dictionary_t&          cur_field(iter->cast<adobe::dictionary_t>());
        adobe::dictionary_t::const_iterator field_name(cur_field.find(key_field_name));

        if (field_name != cur_field.end() &&
            field_name->second.cast<adobe::name_t>() == field)
            return true;
    }

    return false;
}

void check_duplicate_field_name(const bffd_analyzer_t::structure_type& current_structure, adobe::name_t name)
{
    if (has_field_named(current_structure, name))
        throw_duplicate_field_name(name);
}

/****************************************************************************************************/

} // namespace

/****************************************************************************************************/
#if 0
#pragma mark -
#endif
/****************************************************************************************************/

void bffd_analyzer_t::set_current_structure(adobe::name_t structure_name)
{
    current_structure_m = &structure_map_m[structure_name];

#if 0
    std::cerr << "Current structure now: " << structure_name << std::endl;
#endif
}

/****************************************************************************************************/

void bffd_analyzer_t::add_field(adobe::name_t              field_name,
                                const adobe::dictionary_t& field_parameters)
{
    if (current_structure_m == 0)
        return;

    structure_type& current_structure(*current_structure_m);

    check_duplicate_field_name(current_structure, field_name);

    current_structure.push_back(adobe::any_regular_t(field_parameters));

#if 0
    std::cerr << "  Added field " << field_name << std::endl;
#endif
}

/****************************************************************************************************/

void bffd_analyzer_t::add_const(adobe::name_t              const_name,
                                const adobe::dictionary_t& const_parameters)
{
    if (current_structure_m == 0)
        return;

    structure_type& current_structure(*current_structure_m);

    check_duplicate_field_name(current_structure, const_name);

    current_structure.push_back(adobe::any_regular_t(const_parameters));

#if 0
    std::cerr << "  Added const " << const_name << std::endl;
#endif
}

/****************************************************************************************************/

void bffd_analyzer_t::add_invariant(adobe::name_t         invariant_name,
                                    const adobe::array_t& expression)
{
    if (current_structure_m == 0)
        return;

    structure_type& current_structure(*current_structure_m);

    check_duplicate_field_name(current_structure, invariant_name);

    adobe::dictionary_t current_field;

    // fill in the field here
    current_field[key_field_type].assign(value_field_type_invariant);
    current_field[key_field_name].assign(invariant_name);
    current_field[key_invariant_expression].assign(expression);

    current_structure.push_back(adobe::any_regular_t(current_field));

#if 0
    std::cerr << "  Added invariant " << invariant_name << std::endl;
#endif
}

/****************************************************************************************************/

void bffd_analyzer_t::add_notify(const adobe::dictionary_t& notify_parameters)
{
    if (current_structure_m == 0)
        return;

    structure_type& current_structure(*current_structure_m);

    current_structure.push_back(adobe::any_regular_t(notify_parameters));

#if 0
    std::cerr << "  Added notify" << std::endl;
#endif
}

/****************************************************************************************************/

void bffd_analyzer_t::add_skip(adobe::name_t              skip_name,
                               const adobe::dictionary_t& skip_parameters)
{
    if (current_structure_m == 0)
        return;

    structure_type& current_structure(*current_structure_m);

    check_duplicate_field_name(current_structure, skip_name);

    current_structure.push_back(adobe::any_regular_t(skip_parameters));

#if 0
    std::cerr << "  Added skip " << skip_name << std::endl;
#endif
}

/****************************************************************************************************/

bool bffd_analyzer_t::analyze_binary()
{
    position_m = inspection_position_t();
    forest_m->clear();

    inspection_branch_t  branch(new_branch(forest_m->begin()));
    forest_node_t&       branch_data(*branch);

    branch_data.name_m = value_main;
    branch_data.struct_name_m = value_main;
    branch_data.set_flag(type_struct_k);

    return analyze_with_structure(structure_for(value_main), branch);
}

/****************************************************************************************************/

inspection_branch_t bffd_analyzer_t::new_branch(inspection_branch_t with_parent)
{
    with_parent.edge() = adobe::forest_trailing_edge;

    return forest_m->insert(with_parent, forest_node_t());
}

/****************************************************************************************************/

bool bffd_analyzer_t::analyze_with_structure(const structure_type& structure,
                                             inspection_branch_t   parent)
try
{
    static const adobe::array_t empty_array_k;

    temp_assignment<inspection_branch_t> node_stack(current_leaf_m, parent);
    bool                                 last_conditional_value(false);

    for (structure_type::const_iterator iter(structure.begin()), last(structure.end()); iter != last; ++iter)
    {
        const adobe::dictionary_t& field(iter->cast<adobe::dictionary_t>());
        adobe::name_t              type(value_for<adobe::name_t>(field, key_field_type));
        adobe::name_t              name(value_for<adobe::name_t>(field, key_field_name));

        last_name_m = name;

        if (type == value_field_type_invariant)
        {
            const adobe::array_t& expression(value_for<adobe::array_t>(field, key_invariant_expression));
            bool                  holds(contextual_evaluation_of<bool>(expression, forest_m->begin(), current_leaf_m, input_m));

            if (!holds)
            {
                std::stringstream error;
                error << "Error (" << build_path(forest_m->begin(), current_leaf_m) << "): invariant '" << name << "' failed to hold.";
                std::cerr << error.str() << std::endl;
                return false;
            }

            continue;
        }

        bool                     has_conditional_type(field.count(key_field_conditional_type) != 0);
        conditional_expression_t conditional_type(has_conditional_type ? value_for<conditional_expression_t>(field, key_field_conditional_type) : none_k);
        bool                     is_conditional(conditional_type != none_k);

        // if our field is conditional check if condition holds; if not continue past field
        if (is_conditional)
        {
            if (conditional_type == else_k)
            {
                // if we've already found our true expression in this block skip this one
                if (last_conditional_value)
                    continue;

                last_conditional_value = true;
            }
            else // conditional_type == if_k
            {
                const adobe::array_t& if_expression(value_for<adobe::array_t>(field, key_field_if_expression));

                last_conditional_value = contextual_evaluation_of<bool>(if_expression, forest_m->begin(), current_leaf_m, input_m);
            }

            if (last_conditional_value)
            {
                // jump in to the conditional block but use the same parent as the
                // on that came in - this will add the conditional block's items
                // to the parent, flattening out the conditional.

                adobe::name_t if_block_name(value_for<adobe::name_t>(field, key_struct_type_name));

                if (analyze_with_structure(structure_for(if_block_name), parent) == false)
                    return false;
            }

            // we're done with this conditional whether true or false
            continue;
        }

        if (type == value_field_type_notify)
        {
            const adobe::array_t& expression(value_for<adobe::array_t>(field, key_notify_expression));
            adobe::array_t        argument_set(contextual_evaluation_of<adobe::array_t>(expression, forest_m->begin(), current_leaf_m, input_m));
            std::stringstream     result;

            adobe::copy(argument_set, std::ostream_iterator<adobe::any_regular_t>(result));

            std::cout << result.str() << std::endl;

            continue;
        }

        // From this point on we've actually added a node to the analysis forest.
        // So if you want to do anything during analysis without affecting the
        // forest, do it above this comment. (Also, make sure this comment stays
        // with the sub_branch creation so we have something obvious to demarcate
        // when a node is added to the forest.)

        inspection_branch_t sub_branch(new_branch(parent));
        forest_node_t&      branch_data(*sub_branch);

        temp_assignment<inspection_branch_t> node_stack(current_leaf_m, sub_branch);

        branch_data.name_m = name;

        if (type == value_field_type_struct)
            branch_data.set_flag(type_struct_k);
        else if (type == value_field_type_atom)
            branch_data.set_flag(type_atom_k);
        else if (type == value_field_type_const)
            branch_data.set_flag(type_const_k);
        else if (type == value_field_type_skip)
            branch_data.set_flag(type_skip_k);
        else
            throw std::runtime_error("analysis error: unknown type");

        if (type == value_field_type_const)
        {
            branch_data.expression_m = value_for<adobe::array_t>(field, key_const_expression);

            continue;
        }
        else if (type == value_field_type_skip)
        {
            // skip is different in that its parameter is unit BYTES not bits
            const adobe::array_t& skip_expression(value_for<adobe::array_t>(field, key_skip_expression));
            boost::uint64_t       byte_count(static_cast<boost::uint64_t>(contextual_evaluation_of<double>(skip_expression, forest_m->begin(), current_leaf_m, input_m)));

            branch_data.start_offset_m = position_m.byte_offset_m;

            if (parent->start_offset_m == boost::uint64_t(-1))
                parent->start_offset_m = position_m.byte_offset_m;

            // skip the bytes here
            branch_data.bit_count_m = byte_count * 8;
            branch_data.location_m = make_location(branch_data.bit_count_m);

            branch_data.end_offset_m = position_m.byte_offset_m - 1;
            parent->end_offset_m = branch_data.end_offset_m;

            continue;
        }

        const adobe::array_t& size_count_expression(value_for<adobe::array_t>(field, key_field_size_expression));
        const adobe::array_t& while_expression(value_for<adobe::array_t>(field, key_field_while_expression));
        bool                  has_size_expression(!size_count_expression.empty());
        bool                  has_while_expression(!while_expression.empty());
        std::size_t           size_count(has_size_expression ?
                                         static_cast<std::size_t>(contextual_evaluation_of<double>(size_count_expression, forest_m->begin(), current_leaf_m, input_m)) :
                                         0);

        if (has_size_expression || has_while_expression)
        {
            branch_data.set_flag(is_array_root_k);
            branch_data.cardinal_m = 0; // loops will overwrite
        }

        inspection_position_t position_save(position_m);
        const adobe::array_t& offset_expression(value_for<adobe::array_t>(field, key_field_offset_expression));
        bool                  remote_position(!offset_expression.empty());

        if (remote_position)
        {
            // if our field's data is not at the next immediate offset,
            // temporarily set the position marker to that offset location
            // and restore it later.
            position_m.bit_offset_m = 0;
            position_m.byte_offset_m = static_cast<boost::uint64_t>(contextual_evaluation_of<double>(offset_expression, forest_m->begin(), current_leaf_m, input_m));
        }
        else
        {
            // If it's not a remote position we want to cache the start
            // offset for this node in its parent, but only if it hasn't
            // already been set.
            if (parent->start_offset_m == boost::uint64_t(-1))
                parent->start_offset_m = position_m.byte_offset_m;
        }

        const adobe::array_t& callback_expression(value_for<adobe::array_t>(field, key_field_callback));
        bool                  has_callback_expression(!callback_expression.empty());

        if (type == value_field_type_struct)
        {
            adobe::name_t struct_name(value_for<adobe::name_t>(field, key_struct_type_name));

            branch_data.struct_name_m = struct_name;

            if (has_while_expression || has_size_expression)
            {
                // a loop means branch_data is an array root, so we must
                // update the start and end offset for it.
                branch_data.start_offset_m = position_m.byte_offset_m;
                branch_data.end_offset_m = position_m.byte_offset_m;

                if (has_while_expression)
                {
                    while (contextual_evaluation_of<bool>(while_expression, forest_m->begin(), current_leaf_m, input_m))
                    {
                        inspection_branch_t array_element_branch(new_branch(sub_branch));
                        forest_node_t&      array_element_data(*array_element_branch);

                        array_element_data.set_flag(is_array_element_k);
                        array_element_data.cardinal_m = branch_data.cardinal_m++;

                        if (analyze_with_structure(structure_for(struct_name), array_element_branch) == false)
                            return false;

                        // We keep the end offset up to date because it
                        // may be used by the while predicate
                        branch_data.end_offset_m = position_m.byte_offset_m - 1;
                    }
                }
                else if (has_size_expression)
                {
                    while (branch_data.cardinal_m != size_count)
                    {
                        inspection_branch_t array_element_branch(new_branch(sub_branch));
                        forest_node_t&      array_element_data(*array_element_branch);

                        array_element_data.set_flag(is_array_element_k);
                        array_element_data.cardinal_m = branch_data.cardinal_m++;

                        if (analyze_with_structure(structure_for(struct_name), array_element_branch) == false)
                            return false;
                    }
                }

                // One final update to the root's end offset should does the trick
                branch_data.end_offset_m = position_m.byte_offset_m - 1;
            }
            else // singleton
            {
                if (analyze_with_structure(structure_for(struct_name), sub_branch) == false)
                    return false;
            }

            // If the client has specified a callback and this node's callback
            // expression evaluates to true, post to the callback.
            if (has_callback_expression &&
                contextual_evaluation_of<bool>(callback_expression, forest_m->begin(), current_leaf_m, input_m) &&
                monitor_m)
                monitor_m(input_m, forest_m->begin(), sub_branch);                
        }
        else if (type == value_field_type_atom)
        {
            const adobe::array_t& bit_count_expression(value_for<adobe::array_t>(field, key_atom_bit_count_expression));
            const adobe::array_t& is_big_endian_expression(value_for<adobe::array_t>(field, key_atom_is_big_endian_expression));
            bool                  is_big_endian(contextual_evaluation_of<bool>(is_big_endian_expression, forest_m->begin(), current_leaf_m, input_m));

            branch_data.bit_count_m = static_cast<boost::uint64_t>(contextual_evaluation_of<double>(bit_count_expression, forest_m->begin(), current_leaf_m, input_m));
            branch_data.type_m = value_for<atom_base_type_t>(field, key_atom_base_type);
            branch_data.set_flag(atom_is_big_endian_k, is_big_endian);

            if (has_while_expression || has_size_expression)
            {
                // a loop means branch_data is an array root, so we must
                // update the start and end offset for it.
                branch_data.start_offset_m = position_m.byte_offset_m;
                branch_data.end_offset_m = position_m.byte_offset_m;

                if (has_while_expression)
                {
                    adobe::dictionary_t while_parameters(contextual_evaluation_of<adobe::dictionary_t>(while_expression, forest_m->begin(), current_leaf_m, input_m));
                    std::size_t         while_bit_count(static_cast<std::size_t>(value_for<double>(while_parameters, key_size)));
                    boost::uint64_t     while_stop_value(static_cast<boost::uint64_t>(value_for<double>(while_parameters, key_value)));
                    bool                while_inclusive(value_for<bool>(while_parameters, key_inclusive, false));
                    boost::uint64_t     running_count(0);
                    boost::uint64_t     running_stop_value(0);
                    boost::uint64_t     running_stop_value_mask(while_bit_count == 8 ?  0xff :
                                                                while_bit_count == 16 ? 0xffff :
                                                                while_bit_count == 32 ? 0xffffffff :
                                                                                        -1);

                    input_m.seekg(static_cast<std::streamoff>(position_m.byte_offset_m));

                    while (true)
                    {
                        unsigned char c;

                        input_m.read(reinterpret_cast<char*>(&c), 1);

                        running_stop_value = (running_stop_value << 8 | c) & running_stop_value_mask;

                        if (running_stop_value != while_stop_value || while_inclusive)
                            ++running_count;
                        else if (!while_inclusive)
                            running_count -= while_bit_count / 8 - 1;

                        if (running_stop_value == while_stop_value)
                            break;
                    }

                    for (boost::uint64_t i(0); i < running_count; ++i)
                    {
                        inspection_branch_t array_element_branch(new_branch(sub_branch));
                        forest_node_t&      array_element_data(*array_element_branch);

                        array_element_data.set_flag(is_array_element_k);
                        array_element_data.cardinal_m = branch_data.cardinal_m++;
                        array_element_data.location_m = make_location(branch_data.bit_count_m);
                    }
                }
                else if (has_size_expression)
                {
                    while (branch_data.cardinal_m != size_count)
                    {
                        inspection_branch_t array_element_branch(new_branch(sub_branch));
                        forest_node_t&      array_element_data(*array_element_branch);

                        array_element_data.set_flag(is_array_element_k);
                        array_element_data.cardinal_m = branch_data.cardinal_m++;
                        array_element_data.location_m = make_location(branch_data.bit_count_m);
                    }
                }

                // One final update to the root's end offset should does the trick
                branch_data.end_offset_m = position_m.byte_offset_m - 1;
            }
            else // singleton
            {
                branch_data.location_m = make_location(branch_data.bit_count_m);
            }

            // If the client has specified a callback and this node's callback
            // expression evaluates to true, post to the callback.
            if (has_callback_expression &&
                contextual_evaluation_of<bool>(callback_expression, forest_m->begin(), current_leaf_m, input_m) &&
                monitor_m)
                monitor_m(input_m, forest_m->begin(), sub_branch);                
        }

        if (remote_position)
        {
            // restore the position marker if it was tweaked to read this field
            position_m = position_save;
        }
        else
        {
            // If it's not a remote position we want to cache the end
            // offset for this node in its parent. Because the nodes
            // are in increasing order in the file we overwrite the
            // end offset with whatever the most current value is.
            parent->end_offset_m = position_m.byte_offset_m - 1;
        }
    }

    return true;
}
catch (const analysis_error_t& error)
{
    throw error;
}
catch (const std::exception& error)
{
    std::stringstream error_details;
    error_details << "While analyzing '" << last_name_m << "': " << error.what();
    throw analysis_error_t(error_details.str());
}

/****************************************************************************************************/

const bffd_analyzer_t::structure_type& bffd_analyzer_t::structure_for(adobe::name_t structure_name)
{
    structure_map_t::const_iterator structure(structure_map_m.find(structure_name));

    if (structure == structure_map_m.end())
    {
        std::stringstream error;
        error << "Could not find structure '" << structure_name << "'";
        throw std::runtime_error(error.str());
    }

    return structure->second;
}

/****************************************************************************************************/

inspection_position_t bffd_analyzer_t::make_location(boost::uint64_t bit_count)
{
    inspection_position_t result(position_m);
    boost::uint64_t       byte_count(bit_count / 8);

    bit_count %= 8;

    position_m.byte_offset_m += byte_count;
    position_m.bit_offset_m += static_cast<boost::uint8_t>(bit_count);

    return result;
}

/****************************************************************************************************/
