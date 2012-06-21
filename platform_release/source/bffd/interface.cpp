/****************************************************************************************************/

#include <adobe/bffd/interface.hpp>

#include <fstream>

#include <adobe/istream.hpp>
#include <adobe/implementation/expression_parser.hpp>

/****************************************************************************************************/

namespace {

/****************************************************************************************************/
#if 0
#pragma mark -
#endif
/****************************************************************************************************/

inline void indent_stream(std::ostream& stream, std::size_t count)
    { for (; count != 0; --count) stream << "  "; }

void stream_out(const adobe::any_regular_t& value,
                boost::uint64_t             bit_length,
                atom_base_type_t            base_type,
                std::ostream&               s)
{
    if (bit_length % 8 != 0)
    {
        std::cerr << "sub-byte sizes not supported yet." << std::endl;

        return;
    }

    double d(value.cast<double>());

    if (base_type == atom_signed_k)
        s << static_cast<signed long long>(d);
    else if (base_type == atom_unsigned_k)
        s << static_cast<unsigned long long>(d);
    else if (base_type == atom_float_k)
        s << d;
}

/****************************************************************************************************/

} // namespace

/****************************************************************************************************/
#if 0
#pragma mark -
#endif
/****************************************************************************************************/

bffd_interface_t::bffd_interface_t(std::istream& binary_file,
                                   auto_forest_t forest) :
    input_m(binary_file),
    forest_m(forest),
    node_m(forest_m->begin())
{
    command_map_m.insert(command_map_t::value_type("q",                   &bffd_interface_t::quit));
    command_map_m.insert(command_map_t::value_type("quit",                &bffd_interface_t::quit));
    command_map_m.insert(command_map_t::value_type("?",                   &bffd_interface_t::help));
    command_map_m.insert(command_map_t::value_type("help",                &bffd_interface_t::help));
    command_map_m.insert(command_map_t::value_type("ll",                  &bffd_interface_t::print_structure));
    command_map_m.insert(command_map_t::value_type("ls",                  &bffd_interface_t::print_structure));
    command_map_m.insert(command_map_t::value_type("ps",                  &bffd_interface_t::print_structure));
    command_map_m.insert(command_map_t::value_type("print_struct",        &bffd_interface_t::print_structure));
    command_map_m.insert(command_map_t::value_type("pb",                  &bffd_interface_t::print_branch));
    command_map_m.insert(command_map_t::value_type("print_branch",        &bffd_interface_t::print_branch));
    command_map_m.insert(command_map_t::value_type("str",                 &bffd_interface_t::print_string));
    command_map_m.insert(command_map_t::value_type("print_string",        &bffd_interface_t::print_string));
    command_map_m.insert(command_map_t::value_type("cd",                  &bffd_interface_t::step_in));
    command_map_m.insert(command_map_t::value_type("si",                  &bffd_interface_t::step_in));
    command_map_m.insert(command_map_t::value_type("step_in",             &bffd_interface_t::step_in));
    command_map_m.insert(command_map_t::value_type("so",                  &bffd_interface_t::step_out));
    command_map_m.insert(command_map_t::value_type("step_out",            &bffd_interface_t::step_out));
    command_map_m.insert(command_map_t::value_type("t",                   &bffd_interface_t::top));
    command_map_m.insert(command_map_t::value_type("top",                 &bffd_interface_t::top));
    command_map_m.insert(command_map_t::value_type("df",                  &bffd_interface_t::detail_field));
    command_map_m.insert(command_map_t::value_type("detail_field",        &bffd_interface_t::detail_field));
    command_map_m.insert(command_map_t::value_type("do",                  &bffd_interface_t::detail_offset));
    command_map_m.insert(command_map_t::value_type("detail_offset",       &bffd_interface_t::detail_offset));
    command_map_m.insert(command_map_t::value_type("ee",                  &bffd_interface_t::evaluate_expression));
    command_map_m.insert(command_map_t::value_type("eval",                &bffd_interface_t::evaluate_expression));
    command_map_m.insert(command_map_t::value_type("evaluate_expression", &bffd_interface_t::evaluate_expression));
    command_map_m.insert(command_map_t::value_type("duf",                 &bffd_interface_t::dump_field));
    command_map_m.insert(command_map_t::value_type("dump_field",          &bffd_interface_t::dump_field));
    command_map_m.insert(command_map_t::value_type("duo",                 &bffd_interface_t::dump_offset));
    command_map_m.insert(command_map_t::value_type("dump_offset",         &bffd_interface_t::dump_offset));
    command_map_m.insert(command_map_t::value_type("sf",                  &bffd_interface_t::save_field));
    command_map_m.insert(command_map_t::value_type("save_field",          &bffd_interface_t::save_field));
}

/****************************************************************************************************/

void bffd_interface_t::quit(const command_segment_set_t&)
{
    throw user_quit_exception_t();
}

/****************************************************************************************************/

void bffd_interface_t::help(const command_segment_set_t&)
{
    std::cout << "Please consult the readme for more detailed help." << std::endl;
    std::cout << "\n";
    std::cout << "quit (q)\n";
    std::cout << "---------\n";
    std::cout << "\n";
    std::cout << "Terminates the BFFD\n";
    std::cout << "\n";
    std::cout << "help (?)\n";
    std::cout << "---------\n";
    std::cout << "\n";
    std::cout << "Prints BFFD help\n";
    std::cout << "\n";
    std::cout << "print_struct (ps) (ls) (ll)\n";
    std::cout << "---------------------------\n";
    std::cout << "\n";
    std::cout << "Displays a synopsis of the structure at the current path.\n";
    std::cout << "\n";
    std::cout << "print_branch (pb)\n";
    std::cout << "-----------------\n";
    std::cout << "\n";
    std::cout << "Displays a complete synopsis of the current structure at the current path.\n";
    std::cout << "Executing print_branch at $main$ will output the entire contents of the\n";
    std::cout << "analysis. For leaf structures this command is equivalent to print_struct.\n";
    std::cout << "\n";
    std::cout << "print_string <path> (str)\n";
    std::cout << "------------------\n";
    std::cout << "\n";
    std::cout << "Displays the field specified by the path as a string. Values that have no\n";
    std::cout << "graphical representation (i.e., std::isgraph(c) == false) are output as their\n";
    std::cout << "ASCII value prepended with a '\'.\n";
    std::cout << "\n";
    std::cout << "step_in <path> (si) (cd)\n";
    std::cout << "------------------\n";
    std::cout << "\n";
    std::cout << "Sets the path to the structure defined by the path. This can be done both\n";
    std::cout << "relative to the current path and absolutely from $main$.\n";
    std::cout << "\n";
    std::cout << "step_out (so)\n";
    std::cout << "-------------\n";
    std::cout << "\n";
    std::cout << "Sets the path to be the parent structure of the current path. Note that in the\n";
    std::cout << "case of an array element, the parent structure is the array root and not the\n";
    std::cout << "structure that contains the array.\n";
    std::cout << "\n";
    std::cout << "top (t)\n";
    std::cout << "-------\n";
    std::cout << "\n";
    std::cout << "Sets the path to $main$\n";
    std::cout << "\n";
    std::cout << "detail_field <path> (df)\n";
    std::cout << "------------------------\n";
    std::cout << "\n";
    std::cout << "Prints out detailed information about the path specified.\n";
    std::cout << "\n";
    std::cout << "detail_offset <offset> (do)\n";
    std::cout << "------------------------\n";
    std::cout << "\n";
    std::cout << "Searches the binary file analysis for the atom that interprets the byte at the\n";
    std::cout << "provided offset. Currently only local data is included in the search (not remote\n";
    std::cout << "data) though its inclusion is planned for a future release.\n";
    std::cout << "\n";
    std::cout << "evaluate_expression <expression> (eval) (ee)\n";
    std::cout << "-------------------\n";
    std::cout << "\n";
    std::cout << "Allows for the evaluation of an expression whose result is immediately output.\n";
    std::cout << std::endl;

}

/****************************************************************************************************/

void bffd_interface_t::print_branch(const command_segment_set_t&)
{
    inspection_branch_t begin(adobe::leading_of(node_m));
    inspection_branch_t end(adobe::trailing_of(node_m));

    print_branch_depth_range(std::make_pair(depth_full_iterator_t(begin),
                                            depth_full_iterator_t(++end)));
}

/****************************************************************************************************/

void bffd_interface_t::print_structure(const command_segment_set_t&)
{
    print_node(adobe::leading_of(node_m), true, 0);

    inspection_forest_t::child_iterator iter(adobe::child_begin(node_m));
    inspection_forest_t::child_iterator last(adobe::child_end(node_m));

    for (; iter != last; ++iter)
        print_node(iter.base(), false, 1);

    print_node(adobe::trailing_of(node_m), true, 0);
}

/****************************************************************************************************/

void bffd_interface_t::print_string(const command_segment_set_t& parameters)
{
#if !BOOST_WINDOWS
    using std::isgraph;
#endif

    std::size_t parameter_size(parameters.size());

    if (parameter_size < 2)
    {
        std::cerr << "Usage: print_string <expression>" << std::endl;
        return;
    }

    std::string expression_string;

    for (std::size_t i(1); i < parameter_size; ++i)
        expression_string += parameters[i] + std::string(" ");

    try
    {
        std::stringstream      input(expression_string);
        adobe::line_position_t position(__FILE__, __LINE__);
        adobe::array_t         expression;

        adobe::expression_parser(input, position).require_expression(expression);

        inspection_branch_t node(contextual_evaluation_of<inspection_branch_t>(expression,
                                                                               forest_m->begin(),
                                                                               node_m,
                                                                               input_m));
        boost::uint64_t            start_byte_offset(starting_offset_for(node));
        boost::uint64_t            end_byte_offset(ending_offset_for(node));
        boost::uint64_t            size(end_byte_offset - start_byte_offset + 1);
        std::vector<unsigned char> str(static_cast<std::size_t>(size), 0);

        input_m.seekg(static_cast<std::streamoff>(start_byte_offset));
        input_m.read(reinterpret_cast<char*>(&str[0]), str.size());

        for (std::size_t i(0); i < size; ++i)
        {
            char c(str[i]);

            if (isgraph(c))
                std::cout << c;
            else
            {
                std::cout << "\\x" << std::hex;
                std::cout.width(2);
                std::cout.fill('0');
                std::cout << (static_cast<int>(c) & 0xff) << std::dec;
            }
        }

        std::cout << std::endl;
    }
    catch (const std::exception& error)
    {
        std::cerr << "print_string error: " << error.what() << std::endl;

        return;
    }
}

/****************************************************************************************************/

void bffd_interface_t::step_in(const command_segment_set_t& parameters)
{
    if (parameters.size() != 2)
    {
        std::cerr << "Usage: step_in <substruct>" << std::endl;
        return;
    }

    static const std::string up_k("..");

    const std::string& substruct(parameters[1]);

    if (substruct == up_k)
    {
        static const command_segment_set_t step_out_cmd_k(1, std::string("step_out"));

        step_out(step_out_cmd_k);
    }
    else
    {
        try
        {
            node_m = expression_to_node(substruct);
        }
        catch (const std::exception& error)
        {
            std::cerr << "step_in error: " << error.what() << std::endl;
        }
    }
}

/****************************************************************************************************/

void bffd_interface_t::step_out(const command_segment_set_t& parameters)
{
    if (parameters.size() != 1)
    {
        std::cerr << "Usage: step_out" << std::endl;
        return;
    }

    node_m.edge() = adobe::forest_leading_edge;

    if (node_m != forest_m->begin())
        node_m = adobe::find_parent(node_m);
}

/****************************************************************************************************/

void bffd_interface_t::top(const command_segment_set_t& parameters)
{
    if (parameters.size() != 1)
    {
        std::cerr << "Usage: top" << std::endl;
        return;
    }

    node_m = forest_m->begin();
}

/****************************************************************************************************/

void bffd_interface_t::detail_field(const command_segment_set_t& parameters)
{
    if (parameters.size() != 2)
    {
        std::cerr << "Usage: detail_field field" << std::endl;
        return;
    }

    try
    {
        detail_node(expression_to_node(parameters[1]));
    }
    catch (const std::exception& error)
    {
        std::cerr << "detail_field error: " << error.what() << std::endl;

        return;
    }
}

/****************************************************************************************************/

void bffd_interface_t::detail_offset(const command_segment_set_t& parameters)
{
    if (parameters.size() != 2)
    {
        std::cerr << "Usage: detail_offset offset" << std::endl;
        return;
    }

    boost::uint64_t     offset(std::atoi(parameters[1].c_str()));
    inspection_branch_t current(forest_m->begin());
    boost::uint64_t     analysis_end(ending_offset_for(current));

    if (offset > analysis_end)
    {
        std::cerr << "Error: offset " << offset << " beyond analyzed range (" << analysis_end << ")" << std::endl;

        return;
    }

    while (true)
    {
        inspection_forest_t::child_iterator iter(adobe::child_begin(current));
        inspection_forest_t::child_iterator last(adobe::child_end(current));

        if (iter == last)
            break;

        for (; iter != last; ++iter)
        {
            inspection_branch_t iter_base(iter.base());
            bool                is_valid(node_property(iter_base, NODE_PROPERTY_IS_STRUCT) ||
                                         node_property(iter_base, NODE_PROPERTY_IS_ATOM) ||
                                         node_property(iter_base, NODE_PROPERTY_IS_SKIP));
            boost::uint64_t     start_byte_offset(starting_offset_for(iter_base));
            boost::uint64_t     end_byte_offset(ending_offset_for(iter_base));

            if (is_valid && start_byte_offset <= offset && end_byte_offset >= offset)
            {
                current = iter_base;

                break;
            }
        }
    }

    if (node_property(current, NODE_PROPERTY_IS_STRUCT) ||
        node_property(current, NODE_PROPERTY_IS_ATOM) ||
        node_property(current, NODE_PROPERTY_IS_SKIP))
    {
        detail_node(current);
    }
    else
    {
        std::cerr << "Could not find details about offset " << offset << std::endl;
    }
}

/****************************************************************************************************/

void bffd_interface_t::evaluate_expression(const command_segment_set_t& parameters)
{
    std::size_t parameter_size(parameters.size());

    if (parameter_size < 2)
    {
        std::cerr << "Usage: evaluate_expression <expression>" << std::endl;
        return;
    }

    std::string expression_string;

    for (std::size_t i(1); i < parameter_size; ++i)
        expression_string += parameters[i] + std::string(" ");

    try
    {
        std::stringstream      input(expression_string);
        adobe::line_position_t position(__FILE__, __LINE__);
        adobe::array_t         expression;

        adobe::expression_parser(input, position).require_expression(expression);

        std::cout << contextual_evaluation_of<adobe::any_regular_t>(expression,
                                                                    forest_m->begin(),
                                                                    node_m,
                                                                    input_m)
                  << std::endl;
    }
    catch (const std::exception& error)
    {
        std::cerr << "evaluate_expression error: " << error.what() << std::endl;

        return;
    }
}

/****************************************************************************************************/

void bffd_interface_t::dump_field(const command_segment_set_t& parameters)
{
    if (parameters.size() < 2)
    {
        std::cerr << "Usage: dump_field field1 <field2>" << std::endl;
        return;
    }

    boost::uint64_t start_offset(0);
    boost::uint64_t end_offset(0);

    if (parameters.size() == 2)
    {
        inspection_branch_t leaf(expression_to_node(parameters[1]));
        
        start_offset = starting_offset_for(leaf);
        end_offset = ending_offset_for(leaf) + 1;
    }
    else if (parameters.size() == 3)
    {
        inspection_branch_t leaf1(expression_to_node(parameters[1]));
        inspection_branch_t leaf2(expression_to_node(parameters[2]));
        
        start_offset = starting_offset_for(leaf1);
        end_offset = ending_offset_for(leaf2) + 1;
    }
    else
    {
        std::cerr << "dump_field error: too many parameters" << std::endl;
        return;
    }

    dump_range(start_offset, end_offset);
}

/****************************************************************************************************/

void bffd_interface_t::dump_offset(const command_segment_set_t& parameters)
{
    if (parameters.size() != 3)
    {
        std::cerr << "Usage: dump_offset start_offset end_offset" << std::endl;
        return;
    }
    
    dump_range(std::atoi(parameters[1].c_str()),
               std::atoi(parameters[2].c_str()) + 1);
}

/****************************************************************************************************/

void bffd_interface_t::save_field(const command_segment_set_t& parameters)
{
    if (parameters.size() < 2)
    {
        std::cerr << "Usage: save_field field1 <field2> file" << std::endl;
        return;
    }
    
    boost::uint64_t start_offset(0);
    boost::uint64_t end_offset(0);
    std::string     filename;

    if (parameters.size() == 3)
    {
        inspection_branch_t leaf(expression_to_node(parameters[1]));
        
        start_offset = starting_offset_for(leaf);
        end_offset = ending_offset_for(leaf) + 1;

        filename = parameters[2];
    }
    else if (parameters.size() == 4)
    {
        inspection_branch_t leaf1(expression_to_node(parameters[1]));
        inspection_branch_t leaf2(expression_to_node(parameters[2]));
        
        start_offset = starting_offset_for(leaf1);
        end_offset = ending_offset_for(leaf2) + 1;

        filename = parameters[3];
    }
    else
    {
        std::cerr << "save_field error: too many parameters" << std::endl;
        return;
    }

    save_range(start_offset, end_offset, filename);
}

/****************************************************************************************************/

void bffd_interface_t::save_range(boost::uint64_t first, boost::uint64_t last, const std::string& filename)
{
    std::ofstream output(filename.c_str());

    if (output.fail())
    {
        std::cerr << "save_range error: '" << filename << "' failed to open for write." << std::endl;
        return;
    }

    input_m.seekg(static_cast<std::streamoff>(first));

    static const std::size_t block_size_k(1024); // 1K chunks; easy enough to adjust
    boost::uint64_t          range_size(last - first);
    boost::uint64_t          block_count(range_size / block_size_k);
    boost::uint64_t          leftovers(range_size - (block_count * block_size_k));
    std::vector<char>        block_buffer(block_size_k, 0);

    input_m.read(&block_buffer[0], static_cast<std::streamsize>(leftovers));
    output.write(&block_buffer[0], static_cast<std::streamsize>(leftovers));

    for (std::size_t i(0); i < block_count; ++i)
    {
        input_m.read(&block_buffer[0], block_size_k);
        output.write(&block_buffer[0], block_size_k);
    }
}

/****************************************************************************************************/

void bffd_interface_t::dump_range(boost::uint64_t first, boost::uint64_t last)
{
#if !BOOST_WINDOWS
    using std::isgraph;
#endif

    boost::uint64_t   size(last - first);
    boost::uint64_t   n(0);
    std::vector<char> char_dump;

    input_m.seekg(static_cast<std::streamoff>(first));

    while (n != size)
    {
        char c(input_m.get());

        if (input_m.fail())
        {
            input_m.clear();
            std::cerr << "Input stream failure reading byte " << n << std::endl;
            return;
        }

        std::cout.width(2);
        std::cout.fill('0');
        std::cout << std::hex << (static_cast<unsigned int>(c) & 0xff) << std::dec;

        if (n % 4 == 3)
            std::cout << ' ';

        char_dump.push_back(isgraph(c) ? c : '.');

        if (char_dump.size() == 16)
        {
            adobe::copy(char_dump, std::ostream_iterator<char>(std::cout));
            char_dump.clear();
            std::cout << std::endl;
        }

        ++n;
    }

    if (!char_dump.empty())
    {
        while (n % 16 != 0)
        {
            if (n % 4 == 3)
                std::cout << ' ';

            std::cout << "  ";

            ++n;
        }

        adobe::copy(char_dump, std::ostream_iterator<char>(std::cout));
        std::cout << std::endl;
    }
}

/****************************************************************************************************/

void bffd_interface_t::command_line()
{
    static std::vector<char> cl_buffer_s(1024, 0);

    do
    {
        command_segment_set_t command_segment_set(split_command_string(&cl_buffer_s[0]));

        if (!command_segment_set.empty())
        {
            command_map_t::const_iterator found(command_map_m.find(command_segment_set[0]));

            if (found == command_map_m.end())
                std::cout << "'" << command_segment_set[0] << "' not recognized. Type '?' or 'help' for help." << std::endl;
            else
                (this->*found->second)(command_segment_set);
        }

        path_m = build_path(forest_m->begin(), node_m);

        std::cout << '$' << path_m << "$ ";
    } while (std::cin.getline(&cl_buffer_s[0], cl_buffer_s.size()));
}

/****************************************************************************************************/

std::vector<std::string> bffd_interface_t::split_command_string(const std::string& command)
{
#if !BOOST_WINDOWS
    using std::isspace;
#endif

    std::vector<std::string> result;
    std::string              segment;
    std::vector<char>        command_buffer(command.begin(), command.end());

    // faster performance to pop items off the back instead of the front
    std::reverse(command_buffer.begin(), command_buffer.end());

    while (!command_buffer.empty())
    {
        char c(command_buffer.back());

        if (isspace(c))
        {
            result.push_back(segment);
            segment = std::string();

            while (!command_buffer.empty() && isspace(command_buffer.back()))
                command_buffer.pop_back();
        }
        else
        {
            segment += c;

            command_buffer.pop_back();
        }
    }

    if (!segment.empty())
        result.push_back(segment);

    return result;
}

/****************************************************************************************************/

inspection_branch_t bffd_interface_t::expression_to_node(const std::string& expression_string)
{
    std::stringstream      input(expression_string);
    adobe::line_position_t position(__FILE__, __LINE__);
    adobe::array_t         expression;

    adobe::expression_parser(input, position).require_expression(expression);

    return contextual_evaluation_of<inspection_branch_t>(expression,
                                                         forest_m->begin(),
                                                         node_m,
                                                         input_m);
}

/****************************************************************************************************/

template <typename R>
void bffd_interface_t::print_branch_depth_range(const R& f)
{
    typedef typename boost::range_iterator<R>::type iterator;

    for (iterator iter(boost::begin(f)), last(boost::end(f)); iter != last; ++iter)
        print_node(iter, true);
}

/****************************************************************************************************/

void bffd_interface_t::print_node(inspection_branch_t branch, bool recursing, std::size_t depth)
{
    bool          is_atom(node_property(branch, NODE_PROPERTY_IS_ATOM));
    bool          is_const(node_property(branch, NODE_PROPERTY_IS_CONST));
    bool          is_struct(node_property(branch, NODE_PROPERTY_IS_STRUCT));
    bool          is_skip(node_property(branch, NODE_PROPERTY_IS_SKIP));
    adobe::name_t name(node_property(branch, NODE_PROPERTY_NAME));
    bool          is_array_element(branch->get_flag(is_array_element_k));
    bool          is_array_root(branch->get_flag(is_array_root_k));

    if (branch.edge() == adobe::forest_leading_edge)
    {
        indent_stream(std::cout, depth);

        if (is_struct)
        {
            adobe::name_t struct_name(node_property(branch, STRUCT_PROPERTY_STRUCT_NAME));

            std::cout << '(' << struct_name << ") ";
        }
        else if (is_atom)
        {
            atom_base_type_t      base_type(node_property(branch, ATOM_PROPERTY_BASE_TYPE));
            bool                  is_big_endian(node_property(branch, ATOM_PROPERTY_IS_BIG_ENDIAN));
            boost::uint64_t       bit_count(node_property(branch, ATOM_PROPERTY_BIT_COUNT));
            inspection_position_t position(node_value(branch, ATOM_VALUE_LOCATION));

            std::cout << '('
                      << (base_type == atom_signed_k   ? 's' :
                          base_type == atom_unsigned_k ? 'u' :
                          base_type == atom_float_k    ? 'f' :
                                                         'X')
                      << bit_count
                      << (bit_count > 8 ? is_big_endian ? "b" : "l" : "")
                      << ") ";
        }
        else if (is_const)
        {
            std::cout << "(const) ";
        }
        else if (is_skip)
        {
            std::cout << "(skip) ";
        }

        std::cout << name;

        if (is_array_root)
        {
            boost::uint64_t size(node_property(branch, ARRAY_ROOT_PROPERTY_SIZE));

            std::cout << '[' << size << ']';
        }
        else if (is_array_element)
        {
            boost::uint64_t index(node_value(branch, ARRAY_ELEMENT_VALUE_INDEX));

            std::cout << '[' << index << ']';
        }
        else if (is_skip)
        {
            boost::uint64_t start_byte_offset(starting_offset_for(branch));
            boost::uint64_t end_byte_offset(ending_offset_for(branch));
            boost::uint64_t size(end_byte_offset - start_byte_offset + 1);

            std::cout << '[' << size << ']';
        }

        if (is_const)
        {
            std::cout << ": ";

            if (node_value(branch, CONST_VALUE_IS_EVALUATED))
            {
                std::cout << node_value(branch, CONST_VALUE_EVALUATED_VALUE)
                          /*<< " (cached)"*/;
            }
            else
            {
                const adobe::array_t& const_expression(node_value(branch, CONST_VALUE_EXPRESSION));

                std::cout << contextual_evaluation_of<adobe::any_regular_t>(const_expression,
                                                                            forest_m->begin(),
                                                                            adobe::find_parent(branch),
                                                                            input_m);
            }
        }

        if (!is_array_root && is_atom)
        {
            atom_base_type_t      base_type(node_property(branch, ATOM_PROPERTY_BASE_TYPE));
            bool                  is_big_endian(node_property(branch, ATOM_PROPERTY_IS_BIG_ENDIAN));
            boost::uint64_t       bit_count(node_property(branch, ATOM_PROPERTY_BIT_COUNT));
            inspection_position_t position(node_value(branch, ATOM_VALUE_LOCATION));

            std::cout << ": ";

            stream_out(fetch_and_evaluate(input_m, position, bit_count, base_type, is_big_endian),
                       bit_count,
                       base_type,
                       std::cout);
        }

        if (recursing && (is_struct || is_array_root))
        {
            std::cout << std::endl;
            indent_stream(std::cout, depth);
            std::cout << "{";
        }

        std::cout << std::endl;
    }
    else if (recursing) // trailing edge and recursing
    {
        if (is_struct || is_array_root)
        {
            indent_stream(std::cout, depth);
            std::cout << "}" << std::endl;
        }
    }
}

/****************************************************************************************************/

void bffd_interface_t::print_node(depth_full_iterator_t branch, bool recursing)
{
    print_node(branch.base(), recursing, branch.depth());
}

/****************************************************************************************************/

void bffd_interface_t::detail_struct_or_array_node(inspection_branch_t struct_node)
{
    bool is_array_root(struct_node->get_flag(is_array_root_k));
    bool is_struct(node_property(struct_node, NODE_PROPERTY_IS_STRUCT));

    std::cout << "     path: " << build_path(forest_m->begin(), struct_node) << std::endl
              << "     type: " << (is_array_root ? "array" : "struct") << std::endl;

    if (is_struct)
        std::cout << "   struct: " << node_property(struct_node, STRUCT_PROPERTY_STRUCT_NAME) << std::endl;

    if (is_array_root)
        std::cout << " elements: " << node_property(struct_node, ARRAY_ROOT_PROPERTY_SIZE) << std::endl;

    inspection_forest_t::child_iterator first_child(adobe::child_begin(struct_node));
    inspection_forest_t::child_iterator last_child(adobe::child_end(struct_node));

    if (first_child == last_child)
        return;

    boost::uint64_t start_byte_offset(starting_offset_for(struct_node));
    boost::uint64_t end_byte_offset(ending_offset_for(struct_node));
    boost::uint64_t size(end_byte_offset - start_byte_offset + 1);

    std::cout << "    bytes: [ " << start_byte_offset << " .. " << end_byte_offset << " ]" << std::endl
              << "     size: " << size << " bytes";

    if (size >= 1024)
        std::cout << " (" << size / 1024.0 << " KB)";

    if (size >= 1024 * 1024)
        std::cout << " (" << size / (1024.0 * 1024) << " MB)";

    if (size >= 1024 * 1024 * 1024)
        std::cout << " (" << size / (1024.0 * 1024 * 1024) << " GB)";

    std::cout << std::endl;
}

/****************************************************************************************************/

void bffd_interface_t::detail_atom_node(inspection_branch_t atom_node)
{
    atom_base_type_t      base_type(node_property(atom_node, ATOM_PROPERTY_BASE_TYPE));
    bool                  is_big_endian(node_property(atom_node, ATOM_PROPERTY_IS_BIG_ENDIAN));
    boost::uint64_t       bit_count(node_property(atom_node, ATOM_PROPERTY_BIT_COUNT));
    inspection_position_t position(node_value(atom_node, ATOM_VALUE_LOCATION));
    rawbytes_t            raw(fetch(input_m, position, bit_count));
    adobe::any_regular_t  value(evaluate(raw, bit_count, base_type, is_big_endian));

    std::cout << "     path: " << build_path(forest_m->begin(), atom_node) << std::endl
              << "   format: " << bit_count << "-bit "
                               << (base_type == atom_signed_k   ? "signed" :
                                   base_type == atom_unsigned_k ? "unsigned" :
                                   base_type == atom_float_k    ? "float" :
                                                                  "<ERR>") << ' '
                               << (bit_count > 8 ? is_big_endian ? "big" : "little" : "")
                               << std::endl
              << "   offset: " << position.byte_offset_m;

    if (position.bit_offset_m != 0)
        std::cout << " @ bit " << static_cast<int>(position.bit_offset_m);

    std::cout << std::endl;

    std::cout << "      raw: " << std::hex;
    for (std::size_t i(0); i < raw.size(); ++i)
    {
        std::cout << "0x";
        std::cout.width(2);
        std::cout.fill('0');
        std::cout << static_cast<int>(raw[i]) << ' ';
    }
    std::cout << std::dec << std::endl;

    std::cout << "    value: " << std::dec;
    stream_out(value, bit_count, base_type, std::cout);
    std::cout << " (0x" << std::hex;
    stream_out(value, bit_count, base_type, std::cout);
    std::cout << ")" << std::endl << std::dec;
}

/****************************************************************************************************/

void bffd_interface_t::detail_skip(inspection_branch_t skip_node)
{
    inspection_position_t position(node_value(skip_node, SKIP_VALUE_LOCATION));

    std::cout << "     path: " << build_path(forest_m->begin(), skip_node) << std::endl
              << "     type: skip" << std::endl;

    if (position.bit_offset_m != 0)
        std::cout << " @ bit " << static_cast<int>(position.bit_offset_m);

    boost::uint64_t start_byte_offset(starting_offset_for(skip_node));
    boost::uint64_t end_byte_offset(ending_offset_for(skip_node));
    boost::uint64_t size(end_byte_offset - start_byte_offset + 1);

    std::cout << "    bytes: [ " << start_byte_offset << " .. " << end_byte_offset << " ]" << std::endl;
    std::cout << "     size: " << size << " bytes";

    std::cout << std::endl;
}

/****************************************************************************************************/

void bffd_interface_t::detail_node(inspection_branch_t node)
{
    bool is_struct(node_property(node, NODE_PROPERTY_IS_STRUCT));
    bool is_skip(node_property(node, NODE_PROPERTY_IS_SKIP));
    bool is_array_root(node->get_flag(is_array_root_k));

    if (is_struct || is_array_root)
    {
        detail_struct_or_array_node(node);
    }
    else if (is_skip)
    {
        detail_skip(node);
    }
    else // atom (either singleton or array element)
    {
        detail_atom_node(node);
    }
}

/****************************************************************************************************/
