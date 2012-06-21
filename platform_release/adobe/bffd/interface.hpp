/****************************************************************************************************/

#ifndef BFFD_INTERFACE_HPP
#define BFFD_INTERFACE_HPP

#include <iostream>
#include <map>
#include <stdexcept>

#include <adobe/bffd/common.hpp>

/****************************************************************************************************/

class user_quit_exception_t : public std::exception
{
    virtual const char* what() const throw() { return "user quit"; }
};

/****************************************************************************************************/

class bffd_interface_t
{
public:
    bffd_interface_t(std::istream& binary_file,
                     auto_forest_t forest);

    void command_line();

private:
    typedef std::vector<std::string>                 command_segment_set_t;
    typedef void (bffd_interface_t::*command_proc_t)(const command_segment_set_t&);
    typedef std::map<std::string, command_proc_t>    command_map_t;

    // commands
    void quit(const command_segment_set_t&);
    void help(const command_segment_set_t&);
    void print_branch(const command_segment_set_t&);
    void print_structure(const command_segment_set_t&);
    void print_string(const command_segment_set_t&);
    void step_in(const command_segment_set_t&);
    void step_out(const command_segment_set_t&);
    void top(const command_segment_set_t&);
    void detail_field(const command_segment_set_t&);
    void detail_offset(const command_segment_set_t&);
    void evaluate_expression(const command_segment_set_t&);
    void dump_field(const command_segment_set_t&);
    void dump_offset(const command_segment_set_t&);
    void save_field(const command_segment_set_t&);

    // cl processing helpers
    command_segment_set_t split_command_string(const std::string& command);
    inspection_branch_t   expression_to_node(const std::string& expression_string);

    // output helpers
    template <typename R>
    void print_branch_depth_range(const R& f);
    void print_node(depth_full_iterator_t branch, bool recursing);
    void print_node(inspection_branch_t branch, bool recursing, std::size_t depth = 1);
    void print_structure_children(inspection_branch_t node);
    void detail_struct_or_array_node(inspection_branch_t struct_node);
    void detail_atom_node(inspection_branch_t atom_node);
    void detail_skip(inspection_branch_t skip_node);
    void detail_node(inspection_branch_t node);
    void dump_range(boost::uint64_t first, boost::uint64_t last);
    void save_range(boost::uint64_t first, boost::uint64_t last, const std::string& filename);

    std::istream&       input_m;
    auto_forest_t       forest_m;
    inspection_branch_t node_m;
    std::string         path_m;
    command_map_t       command_map_m;
};

/****************************************************************************************************/
// BFFD_INTERFACE_HPP
#endif

/****************************************************************************************************/
