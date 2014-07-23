#ifndef IVL_property_qual_H
#define IVL_property_qual_H
/*
 * Copyright (c) 2013-2014 Stephen Williams (steve@icarus.com)
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

class property_qualifier_t {
    public:
      static inline property_qualifier_t make_none()
      { property_qualifier_t res; res.mask_ = 0; return res; }

      static inline property_qualifier_t make_static()
      { property_qualifier_t res; res.mask_ = 1; return res; }

      static inline property_qualifier_t make_protected()
      { property_qualifier_t res; res.mask_ = 2; return res; }

      static inline property_qualifier_t make_local()
      { property_qualifier_t res; res.mask_ = 4; return res; }

      static inline property_qualifier_t make_rand()
      { property_qualifier_t res; res.mask_ = 8; return res; }

      static inline property_qualifier_t make_randc()
      { property_qualifier_t res; res.mask_ = 16; return res; }

      static inline property_qualifier_t make_const()
      { property_qualifier_t res; res.mask_ = 32; return res; }

      inline property_qualifier_t operator | (property_qualifier_t r)
      { property_qualifier_t res; res.mask_ = mask_ | r.mask_; return res; }

    public:
      inline bool test_static() const    { return mask_ & 1; }
      inline bool test_protected() const { return mask_ & 2; }
      inline bool test_local() const     { return mask_ & 4; }
      inline bool test_rand() const      { return mask_ & 8; }
      inline bool test_randc() const     { return mask_ & 16; }
      inline bool test_const() const     { return mask_ & 32; }

    private:
      int mask_;
};

#endif /* IVL_property_qual_H */
