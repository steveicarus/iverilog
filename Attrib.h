#ifndef IVL_Attrib_H
#define IVL_Attrib_H
/*
 * Copyright (c) 2000-2014 Stephen Williams (steve@icarus.com)
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

# include  "StringHeap.h"
# include  "verinum.h"

/*
 * This class keeps a map of key/value pairs. The map can be set from
 * an STL map, or by setting individual mappings.
 */
class Attrib {

    public:
      Attrib();
      virtual ~Attrib();

      const verinum&attribute(perm_string key) const;
      void attribute(perm_string key, const verinum&value);
      bool has_compat_attributes(const Attrib&that) const;


	/* Provide a means of iterating over the entries in the map. */
      unsigned       attr_cnt() const;
      perm_string    attr_key(unsigned idx) const;
      const verinum& attr_value(unsigned idx) const;


    private:
      struct cell_ {
	    perm_string  key;
	    verinum val;
      };

      unsigned nlist_;
      struct cell_*list_;

    private: // not implemented
      Attrib(const Attrib&);
      Attrib& operator= (const Attrib&);
};

#endif /* IVL_Attrib_H */
