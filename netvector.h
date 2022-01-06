#ifndef IVL_netvector_H
#define IVL_netvector_H
/*
 * Copyright (c) 2012-2014 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include  "nettypes.h"
# include  "ivl_target.h"
# include  <vector>

class netvector_t : public ivl_type_s {

    public:
      explicit netvector_t(const netranges_t&packed, ivl_variable_type_t type);

	// special case: there is a single packed dimension and we
	// know it in the form [<msb>:<lsb>]. This step saves me
	// creating a netrange_t for this single item.
      explicit netvector_t(ivl_variable_type_t type, long msb, long lsb,
			   bool signed_flag =false);

	// Special case: scalar object--no packed dimensions at all.
      explicit netvector_t(ivl_variable_type_t type);

      ~netvector_t();

	// Vectors can be interpreted as signed or unsigned when
	// handled as vectors.
      inline void set_signed(bool flag) { signed_ = flag; }
      inline bool get_signed(void) const { return signed_; }

      inline void set_isint(bool flag) { isint_ = flag; }
      inline bool get_isint(void) const { return isint_; }

      inline bool get_scalar(void) const { return packed_dims_.empty(); }

      void set_implicit(bool implicit) { implicit_ = implicit; }
      bool get_implicit() const { return implicit_; }

      ivl_variable_type_t base_type() const;
      const netranges_t&packed_dims() const;

      bool packed(void) const;
      long packed_width() const;
      netranges_t slice_dimensions() const;

      std::ostream& debug_dump(std::ostream&) const;

    public:
	// Some commonly used predefined types
      static netvector_t atom2s64;
      static netvector_t atom2u64;
      static netvector_t atom2s32;
      static netvector_t atom2u32;
      static netvector_t atom2s16;
      static netvector_t atom2u16;
      static netvector_t atom2s8;
      static netvector_t atom2u8;
      static netvector_t time_signed;
      static netvector_t time_unsigned;
      static netvector_t scalar_bool;
      static netvector_t scalar_logic;
      static const netvector_t*integer_type(bool is_signed = true);

    private:
      bool test_compatibility(ivl_type_t that) const;
      bool test_equivalence(ivl_type_t that) const;

    private:
      netranges_t packed_dims_;
      ivl_variable_type_t type_;
      bool signed_    : 1;
      bool isint_     : 1;		// original type of integer
      bool implicit_  : 1;
};

inline netvector_t::netvector_t(const netranges_t &pd,
				ivl_variable_type_t type)
: packed_dims_(pd), type_(type), signed_(false), isint_(false), implicit_(false)
{
}

inline const netranges_t& netvector_t::packed_dims() const
{
      return packed_dims_;
}

inline static std::ostream& operator << (std::ostream&out, const netvector_t&obj)
{
      return obj.debug_dump(out);
}

#endif /* IVL_netvector_H */
