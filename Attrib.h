#ifndef __Attrib_H
#define __Attrib_H
/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#ifdef HAVE_CVS_IDENT
#ident "$Id: Attrib.h,v 1.5 2004/02/20 18:53:33 steve Exp $"
#endif

# include  "StringHeap.h"
# include  "verinum.h"

/*
 * This class keeps a map of key/value pairs. The map can be set from
 * an STL map, or by setting individual mappings.
 */
class Attrib {

    public:
      Attrib();
      ~Attrib();

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

/*
 * $Log: Attrib.h,v $
 * Revision 1.5  2004/02/20 18:53:33  steve
 *  Addtrbute keys are perm_strings.
 *
 * Revision 1.4  2002/08/12 01:34:58  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.3  2002/05/26 01:39:02  steve
 *  Carry Verilog 2001 attributes with processes,
 *  all the way through to the ivl_target API.
 *
 *  Divide signal reference counts between rval
 *  and lval references.
 *
 * Revision 1.2  2002/05/23 03:08:50  steve
 *  Add language support for Verilog-2001 attribute
 *  syntax. Hook this support into existing $attribute
 *  handling, and add number and void value types.
 *
 *  Add to the ivl_target API new functions for access
 *  of complex attributes attached to gates.
 *
 * Revision 1.1  2000/12/04 17:37:03  steve
 *  Add Attrib class for holding NetObj attributes.
 *
 */
#endif
