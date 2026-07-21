#ifndef IVL_netvif_H
#define IVL_netvif_H
/*
 * Copyright (c) 2026 Icarus UVM track
 *
 * Virtual interface type (Tier A #3 vertical slice).
 */

# include  "nettypes.h"
# include  "StringHeap.h"
# include  <vector>

/*
 * A virtual interface is a handle to a concrete interface instance.
 * base_type() is IVL_VT_CLASS so existing object load/store plumbing
 * ( .var/cobj, %load/obj, %store/obj, class properties ) can be reused.
 * Distinguish with dynamic_cast<const netvif_t*>.
 */
class netvif_t : public ivl_type_s {

    public:
      explicit netvif_t(perm_string iface_name);
      ~netvif_t() override;

      ivl_variable_type_t base_type() const override;

      inline perm_string interface_name() const { return iface_name_; }

      void add_member(perm_string name, ivl_type_t type);
      int member_idx_from_name(perm_string name) const;
      size_t get_members() const { return members_.size(); }
      perm_string get_member_name(size_t idx) const;
      ivl_type_t get_member_type(size_t idx) const;

      std::ostream& debug_dump(std::ostream&) const override;

    private:
      bool test_compatibility(ivl_type_t that) const override;
      bool test_equivalence(ivl_type_t that) const override;

      perm_string iface_name_;
      std::vector<perm_string> members_;
      std::vector<ivl_type_t> member_types_;
};

#endif /* IVL_netvif_H */
