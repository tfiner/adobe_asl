/****************************************************************************************************/

#ifndef BFFD_COMMON_HPP
#define BFFD_COMMON_HPP

#include <boost/cstdint.hpp>

#include <adobe/forest.hpp>
#include <adobe/array.hpp>
#include <adobe/dictionary.hpp>

#include <vector>

#include <adobe/bffd/forest.hpp>

/****************************************************************************************************/

typedef std::vector<boost::uint8_t> rawbytes_t;

/****************************************************************************************************/

rawbytes_t           fetch(std::istream&                input,
                           const inspection_position_t& location,
                           boost::uint64_t              bit_count);
adobe::any_regular_t evaluate(const rawbytes_t& raw,
                              boost::uint64_t   bit_length,
                              atom_base_type_t  base_type,
                              bool              is_big_endian);
adobe::any_regular_t fetch_and_evaluate(std::istream&                input,
                                        const inspection_position_t& location,
                                        boost::uint64_t              bit_count,
                                        atom_base_type_t             base_type,
                                        bool                         is_big_endian);

/****************************************************************************************************/

#define DECLARE_KEY(x) \
extern adobe::aggregate_name_t key_##x

#define DECLARE_VALUE(x) \
extern adobe::aggregate_name_t value_##x

#define DECLARE_PRIVATE_KEY(x) \
extern adobe::aggregate_name_t private_key_##x

#define DEFINE_KEY(x) \
adobe::aggregate_name_t key_##x =  { #x }

#define DEFINE_VALUE(x) \
adobe::aggregate_name_t value_##x = { #x }

#define DEFINE_PRIVATE_KEY(x) \
adobe::aggregate_name_t private_key_##x = { "."#x }

/****************************************************************************************************/

DECLARE_KEY(atom_base_type);
DECLARE_KEY(atom_bit_count_expression);
DECLARE_KEY(atom_is_big_endian_expression);
DECLARE_KEY(const_expression);
DECLARE_KEY(field_conditional_type);
DECLARE_KEY(field_if_expression);
DECLARE_KEY(field_name);
DECLARE_KEY(field_offset_expression);
DECLARE_KEY(field_callback);
DECLARE_KEY(field_size_expression);
DECLARE_KEY(field_type);
DECLARE_KEY(field_while_expression);
DECLARE_KEY(invariant_expression);
DECLARE_KEY(notify_expression);
DECLARE_KEY(skip_expression);
DECLARE_KEY(struct_type_name);
DECLARE_KEY(size); // while parameter
DECLARE_KEY(value); // while parameter
DECLARE_KEY(inclusive); // while parameter

DECLARE_VALUE(field_type_atom);
DECLARE_VALUE(field_type_const);
DECLARE_VALUE(field_type_invariant);
DECLARE_VALUE(field_type_notify);
DECLARE_VALUE(field_type_skip);
DECLARE_VALUE(field_type_struct);
DECLARE_VALUE(main);
DECLARE_VALUE(this);

/****************************************************************************************************/

enum conditional_expression_t
{
    none_k = 0,
    if_k,
    else_k
};

/****************************************************************************************************/

boost::uint64_t starting_offset_for(inspection_branch_t branch);
boost::uint64_t ending_offset_for(inspection_branch_t branch);

/****************************************************************************************************/

std::string build_path(const_inspection_branch_t main, const_inspection_branch_t branch);

/****************************************************************************************************/

template <typename T>
T contextual_evaluation_of(const adobe::array_t& expression,
                           inspection_branch_t   main_branch,
                           inspection_branch_t   current_branch,
                           std::istream&         input)
{
    return contextual_evaluation_of<adobe::any_regular_t>(expression, main_branch, current_branch, input).cast<T>();
}

/****************************************************************************************************/

template <>
adobe::any_regular_t contextual_evaluation_of(const adobe::array_t& expression,
                                              inspection_branch_t   main_branch,
                                              inspection_branch_t   current_branch,
                                              std::istream&         input);

template <>
inspection_branch_t  contextual_evaluation_of(const adobe::array_t& expression,
                                              inspection_branch_t   main_branch,
                                              inspection_branch_t   current_branch,
                                              std::istream&         input);

/****************************************************************************************************/

template <typename T>
struct temp_assignment
{
    temp_assignment(T& variable, const T& value) :
        variable_m(variable),
        old_value_m(variable)
    {
        variable_m = value;
    }

    ~temp_assignment()
    {
        variable_m = old_value_m;
    }

private:
    T& variable_m;
    T  old_value_m;
};

/****************************************************************************************************/
// BFFD_COMMON_HPP
#endif

/****************************************************************************************************/
