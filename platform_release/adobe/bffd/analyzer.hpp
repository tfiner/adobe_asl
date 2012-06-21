/****************************************************************************************************/

#ifndef BFFD_ANALYZER_HPP
#define BFFD_ANALYZER_HPP

#include <iostream>
#include <map>

#include <boost/cstdint.hpp>
#include <boost/function.hpp>

#include <adobe/array.hpp>
#include <adobe/dictionary.hpp>
#include <adobe/forest.hpp>

#include <adobe/bffd/common.hpp>

/****************************************************************************************************/

class bffd_analyzer_t
{
public:
    typedef adobe::array_t structure_type;
    typedef boost::function<void (std::istream&       stream,
                                  inspection_branch_t top,
                                  inspection_branch_t node)> monitor_proc_t;

    explicit bffd_analyzer_t(std::istream& binary_file) :
        input_m(binary_file),
        current_structure_m(0),
        forest_m(new inspection_forest_t)
    { }

    // if the structure has not been previously specified it will be created
    void set_current_structure(adobe::name_t structure_name);

    // these operations take place on the last set_current_structure structure
    void add_field(adobe::name_t              field_name,
                   const adobe::dictionary_t& field_parameters);

    void add_const(adobe::name_t              const_name,
                   const adobe::dictionary_t& const_parameters);

    void add_invariant(adobe::name_t         invariant_name,
                       const adobe::array_t& expression);

    void add_notify(const adobe::dictionary_t& notify_parameters);

    void add_skip(adobe::name_t              skip_name,
                  const adobe::dictionary_t& skip_parameters);

    // callback
    void monitor(const monitor_proc_t& proc)
        { monitor_m = proc; }

    // analysis and results routines
    bool analyze_binary();

    auto_forest_t forest()
        { return forest_m; }

private:
    typedef std::map<adobe::name_t, structure_type> structure_map_t;

    // inspection related
    inspection_branch_t   new_branch(inspection_branch_t with_parent);
    bool                  analyze_with_structure(const structure_type& structure,
                                                 inspection_branch_t   branch);
    const structure_type& structure_for(adobe::name_t structure_name);
    inspection_position_t make_location(boost::uint64_t bit_count);

    std::istream&         input_m;
    structure_map_t       structure_map_m;
    structure_type*       current_structure_m;
    inspection_branch_t   current_leaf_m;
    auto_forest_t         forest_m;
    inspection_position_t position_m;
    adobe::name_t         last_name_m; // for better error reporting
    monitor_proc_t        monitor_m;
};

/****************************************************************************************************/
// BFFD_ANALYZER_HPP
#endif

/****************************************************************************************************/
