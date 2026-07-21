#ifndef IVL_netaarray_H
#define IVL_netaarray_H
/*
 * Copyright (c) 2026 Icarus UVM track
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 */

# include  "nettypes.h"
# include  "ivl_target.h"

/*
 * Associative array type.
 *
 * First vertical slice (UVM Tier A #2): string-keyed arrays only.
 * Both `int aa[string];` and `int aa[*];` elaborate to this type with
 * string keys. Integer keys, class elements, and nested AAs are deferred.
 */
class netaarray_t : public netarray_t {

    public:
      explicit netaarray_t(ivl_type_t vec);
      ~netaarray_t() override;

      ivl_variable_type_t base_type() const override;

      inline bool get_signed() const override
      { return element_type()->get_signed(); }

      inline ivl_variable_type_t element_base_type() const
      { return element_type()->base_type(); }

      inline unsigned long element_width(void) const
      { return element_type()->packed_width(); }

      std::ostream& debug_dump(std::ostream&) const override;

    private:
      bool test_compatibility(ivl_type_t that) const override;
      bool test_equivalence(ivl_type_t that) const override;
};

#endif /* IVL_netaarray_H */
