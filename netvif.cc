/*
 * Copyright (c) 2026 Icarus UVM track
 */

# include  "netvif.h"
# include  <iostream>

using namespace std;

netvif_t::netvif_t(perm_string iface_name)
: iface_name_(iface_name)
{
}

netvif_t::~netvif_t()
{
}

ivl_variable_type_t netvif_t::base_type() const
{
      return IVL_VT_CLASS;
}

void netvif_t::add_member(perm_string name, ivl_type_t type)
{
      members_.push_back(name);
      member_types_.push_back(type);
}

int netvif_t::member_idx_from_name(perm_string name) const
{
      for (size_t idx = 0; idx < members_.size(); idx += 1) {
	    if (members_[idx] == name)
		  return static_cast<int>(idx);
      }
      return -1;
}

perm_string netvif_t::get_member_name(size_t idx) const
{
      return members_[idx];
}

ivl_type_t netvif_t::get_member_type(size_t idx) const
{
      return member_types_[idx];
}

ostream& netvif_t::debug_dump(ostream&o) const
{
      o << "virtual " << iface_name_;
      return o;
}

bool netvif_t::test_compatibility(ivl_type_t that) const
{
      const netvif_t*oth = dynamic_cast<const netvif_t*>(that);
      if (oth == 0)
	    return false;
      return iface_name_ == oth->iface_name_;
}

bool netvif_t::test_equivalence(ivl_type_t that) const
{
      return test_compatibility(that);
}
