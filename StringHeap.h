#ifndef __StringHeap_H
#define __StringHeap_H
/*
 * Copyright (c) 2002 Stephen Williams (steve@icarus.com)
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
#ident "$Id: StringHeap.h,v 1.1 2002/08/04 19:13:16 steve Exp $"
#endif

class StringHeap {

    public:
      StringHeap();
      ~StringHeap();

      const char*add(const char*);

    private:
      enum { HEAPCELL = 0x10000 };

      char*cell_base_;
      unsigned cell_ptr_;

    private: // not implemented
      StringHeap(const StringHeap&);
      StringHeap& operator= (const StringHeap&);
};

/*
 * $Log: StringHeap.h,v $
 * Revision 1.1  2002/08/04 19:13:16  steve
 *  dll uses StringHeap for named items.
 *
 */
#endif
