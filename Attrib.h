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
#if !defined(WINNT)
#ident "$Id: Attrib.h,v 1.1 2000/12/04 17:37:03 steve Exp $"
#endif

# include  <string>
# include  <map>

/*
 * This class keeps a map of key/value pairs. The map can be set from
 * an STL map, or by setting individual mappings.
 */
class Attrib {

    public:
      Attrib();
      ~Attrib();

      void set_attributes(const map<string,string>&attr);
      string attribute(const string&key) const;
      void attribute(const string&key, const string&value);
      bool has_compat_attributes(const Attrib&that) const;


	/* Provide a means of iterating over the entries in the map. */
      unsigned size() const;
      string key(unsigned idx) const;
      string value(unsigned idx) const;

    private:
      struct cell_ {
	    string key;
	    string val;
      };

      unsigned nlist_;
      struct cell_*list_;

    private: // not implemented
      Attrib(const Attrib&);
      Attrib& operator= (const Attrib&);
};

/*
 * $Log: Attrib.h,v $
 * Revision 1.1  2000/12/04 17:37:03  steve
 *  Add Attrib class for holding NetObj attributes.
 *
 */
#endif
