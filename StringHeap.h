#ifndef __StringHeap_H
#define __StringHeap_H
/*
 * Copyright (c) 2002-2003 Stephen Williams (steve@icarus.com)
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
#ident "$Id: StringHeap.h,v 1.4 2003/03/01 06:25:30 steve Exp $"
#endif

/*
 * The string heap is a way to permanently allocate strings
 * efficiently. They only take up the space of the string characters
 * and the terminating nul, there is no malloc overhead.
 */
class StringHeap {

    public:
      StringHeap();
      ~StringHeap();

      const char*add(const char*);

    private:
      enum { HEAPCELL = 0x10000 };

      char*cell_base_;
      unsigned cell_ptr_;
      unsigned cell_count_;

    private: // not implemented
      StringHeap(const StringHeap&);
      StringHeap& operator= (const StringHeap&);
};

/*
 * A lexical string heap is a string heap that makes an effort to
 * return the same pointer for identical strings. This saves further
 * space by not allocating duplicate strings, so in a system with lots
 * of identifiers, this can theoretically save more space.
 */
class StringHeapLex  : private StringHeap {

    public:
      StringHeapLex();
      ~StringHeapLex();

      const char*add(const char*);

      unsigned add_count() const;
      unsigned add_hit_count() const;

    private:
      enum { HASH_SIZE = 4096 };
      const char*hash_table_[HASH_SIZE];

      unsigned add_count_;
      unsigned hit_count_;

    private: // not implemented
      StringHeapLex(const StringHeapLex&);
      StringHeapLex& operator= (const StringHeapLex&);
};

/*
 * $Log: StringHeap.h,v $
 * Revision 1.4  2003/03/01 06:25:30  steve
 *  Add the lex_strings string handler, and put
 *  scope names and system task/function names
 *  into this table. Also, permallocate event
 *  names from the beginning.
 *
 * Revision 1.3  2003/01/16 21:44:46  steve
 *  Keep some debugging status.
 *
 * Revision 1.2  2002/08/12 01:34:58  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.1  2002/08/04 19:13:16  steve
 *  dll uses StringHeap for named items.
 *
 */
#endif
