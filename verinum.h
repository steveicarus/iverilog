#ifndef __verinum_H
#define __verinum_H
/*
 * Copyright (c) 1998 Stephen Williams (steve@icarus.com)
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
#ident "$Id: verinum.h,v 1.2 1998/11/09 18:55:35 steve Exp $"
#endif

# include  <string>

/*
 * Numbers in verlog are multibit strings, where each bit has 4
 * possible values: 0, 1, x or z. The verinum number is store in
 * little-endian format. This means that if the long value is 2b'10,
 * get(0) is 0 and get(1) is 1.
 */
class verinum {

    public:
      enum V { V0, V1, Vx, Vz };

      verinum();
      verinum(const string&str);
      verinum(const V*v, unsigned nbits);
      verinum(unsigned long val, unsigned bits);
      verinum(const verinum&);
      ~verinum();

	// Number of significant bits in this number.
      unsigned len() const { return nbits_; }

	// Individual bits can be accessed with the get and set
	// methods.
      V get(unsigned idx) const;
      V set(unsigned idx, V val);

      V operator[] (unsigned idx) const { return get(idx); }


      unsigned long as_ulong() const;
      signed long   as_long() const;
      string as_string() const;

      bool is_string() const { return string_flag_; }

    private:
      V* bits_;
      unsigned nbits_;

	// These are some convenience flags that help us do a better
	// job of pretty-printing values.
      bool string_flag_;

    private: // not implemented
      verinum& operator= (const verinum&);
};


class ostream;
ostream& operator<< (ostream&, const verinum&);
ostream& operator<< (ostream&, verinum::V);

/*
 * $Log: verinum.h,v $
 * Revision 1.2  1998/11/09 18:55:35  steve
 *  Add procedural while loops,
 *  Parse procedural for loops,
 *  Add procedural wait statements,
 *  Add constant nodes,
 *  Add XNOR logic gate,
 *  Make vvm output look a bit prettier.
 *
 * Revision 1.1  1998/11/03 23:29:08  steve
 *  Introduce verilog to CVS.
 *
 */
#endif
