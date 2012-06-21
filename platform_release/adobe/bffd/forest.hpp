/****************************************************************************************************/

#ifndef BFFD_FOREST_HPP
#define BFFD_FOREST_HPP

#include <boost/cstdint.hpp>

#include <adobe/enum_ops.hpp>

/****************************************************************************************************/

struct inspection_position_t
{
    inspection_position_t() :
        byte_offset_m(0),
        bit_offset_m(0)
    { }

    void normalize()
    {
        // can be expensive so we make this lazy

        byte_offset_m += bit_offset_m / 8;
        bit_offset_m %= 8;
    }

    boost::uint64_t byte_offset_m; // byte offset in file
    boost::uint8_t  bit_offset_m;  // sub-byte offset in file -- typically 0
};

/****************************************************************************************************/

enum node_flags_t
{
    flags_none_k         = 0,

    /* general flags */
    type_atom_k          = 1 << 0UL,
    type_const_k         = 1 << 1UL,
    type_struct_k        = 1 << 2UL,
    type_skip_k          = 1 << 3UL,
    is_array_root_k      = 1 << 4UL,
    is_array_element_k   = 1 << 5UL,

    /* atom flags */
    atom_is_big_endian_k = 1 << 6UL,
};

ADOBE_DEFINE_BITSET_OPS(node_flags_t);

template <typename EnumType>
inline void enum_set(EnumType& enumeration, EnumType flag, bool value = true)
{
    if (value)
        enumeration |= flag;
    else
        enumeration &= ~flag;
}

template <typename EnumType>
inline bool enum_get(const EnumType& enumeration, EnumType flag)
{
    return (enumeration & flag) != 0;
}

/****************************************************************************************************/

enum atom_base_type_t
{
    atom_unknown_k = 0,
    atom_signed_k,
    atom_unsigned_k,
    atom_float_k,
};

/****************************************************************************************************/

struct node_t
{
    node_t() :
        flags_m(flags_none_k),
        type_m(atom_unknown_k),
        start_offset_m(-1),
        end_offset_m(-1),
        cardinal_m(0),
        bit_count_m(0),
        evaluated_m(false)
    { }

    void set_flag(node_flags_t flag, bool value = true)
        { enum_set(flags_m, flag, value); }
    bool get_flag(node_flags_t flag) const
        { return enum_get(flags_m, flag); }

    /* general fields */
    node_flags_t          flags_m;
    atom_base_type_t      type_m;
    adobe::name_t         name_m;

    /* struct or array root fields */
    boost::uint64_t       start_offset_m; // change to inspection_position_t for bit support
    boost::uint64_t       end_offset_m;   // change to inspection_position_t for bit support

    /* array root / array element fields */
    boost::uint64_t       cardinal_m; // size for array root; index for array element

    /* atom fields */
    boost::uint64_t       bit_count_m;
    inspection_position_t location_m; // atom's location in the binary file

    /* const fields */
    adobe::array_t        expression_m;
    bool                  evaluated_m; // we do lazy evaluation; cache the result and flag
    adobe::any_regular_t  evaluated_value_m;

    /* struct fields */
    adobe::name_t         struct_name_m;
};

#define NODE_PROPERTY_NAME           (&node_t::name_m)

#define NODE_PROPERTY_IS_ATOM        type_atom_k
#define NODE_PROPERTY_IS_CONST       type_const_k
#define NODE_PROPERTY_IS_SKIP        type_skip_k
#define NODE_PROPERTY_IS_STRUCT      type_struct_k

#define ARRAY_ROOT_PROPERTY_SIZE     (&node_t::cardinal_m)

#define ARRAY_ELEMENT_VALUE_INDEX    (&node_t::cardinal_m)

#define ATOM_PROPERTY_BASE_TYPE      (&node_t::type_m)
#define ATOM_PROPERTY_IS_BIG_ENDIAN  atom_is_big_endian_k
#define ATOM_PROPERTY_BIT_COUNT      (&node_t::bit_count_m)
#define ATOM_VALUE_LOCATION          (&node_t::location_m)

#define SKIP_VALUE_LOCATION          (&node_t::location_m)

#define CONST_VALUE_EXPRESSION       (&node_t::expression_m)
#define CONST_VALUE_IS_EVALUATED     (&node_t::evaluated_m)
#define CONST_VALUE_EVALUATED_VALUE  (&node_t::evaluated_value_m)

#define STRUCT_PROPERTY_STRUCT_NAME  (&node_t::struct_name_m)

/****************************************************************************************************/

typedef node_t                              forest_node_t;
typedef adobe::forest<forest_node_t>        inspection_forest_t;
typedef inspection_forest_t::iterator       inspection_branch_t;
typedef inspection_forest_t::const_iterator const_inspection_branch_t;
typedef adobe::depth_fullorder_iterator<boost::range_iterator<inspection_forest_t>::type> depth_full_iterator_t;
typedef std::auto_ptr<inspection_forest_t>  auto_forest_t;

template <typename T>
struct node_member
{
    typedef T(node_t::*value)() const;
};

/****************************************************************************************************/
// A property is something generic to the node's type, like structure name or endianness.
// If the node is an array element we get the property from the parent, otherwise itself.
template <typename T>
T node_property(const_inspection_branch_t branch, T(node_t::*member_function)()const)
{
    const forest_node_t& node(*branch);
    bool                 is_array_element(node.get_flag(is_array_element_k));

    if (is_array_element)
        return node_property<T>(adobe::find_parent(branch), member_function);

    return (node.*member_function)();
}

template <typename T>
T node_property(const_inspection_branch_t branch, T node_t::*member)
{
    const forest_node_t& node(*branch);
    bool                 is_array_element(node.get_flag(is_array_element_k));

    if (is_array_element)
        return node_property<T>(adobe::find_parent(branch), member);

    return node.*member;
}

static bool node_property(const_inspection_branch_t branch, node_flags_t flag)
{
    const forest_node_t& node(*branch);
    bool                 is_array_element(node.get_flag(is_array_element_k));

    if (is_array_element)
        return node_property(adobe::find_parent(branch), flag);

    return node.get_flag(flag);
}

/****************************************************************************************************/
// A value is something specific to this node, like location or name.
// If the node is an array root, we get the value from the first child, otherwise itself.
template <typename T>
T node_value(const_inspection_branch_t branch, T(node_t::*member_function)()const)
{
    const forest_node_t& node(*branch);
    bool                 is_array_root(node.get_flag(is_array_root_k));

    if (is_array_root)
    {
        inspection_forest_t::const_child_iterator first_child(adobe::child_begin(branch));
        inspection_forest_t::const_child_iterator last_child(adobe::child_end(branch));

        if (first_child != last_child)
            return node_value<T>(first_child.base(), member_function);
    }

    return (node.*member_function)();
}

template <typename T>
T node_value(const_inspection_branch_t branch, T node_t::*member)
{
    const forest_node_t& node(*branch);
    bool                 is_array_root(node.get_flag(is_array_root_k));

    if (is_array_root)
    {
        inspection_forest_t::const_child_iterator first_child(adobe::child_begin(branch));
        inspection_forest_t::const_child_iterator last_child(adobe::child_end(branch));

        if (first_child != last_child)
            return node_value<T>(first_child.base(), member);
    }

    return node.*member;
}

/****************************************************************************************************/
// BFFD_FOREST_HPP
#endif

/****************************************************************************************************/
