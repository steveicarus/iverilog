#ifndef IVL_StringHeap_H
#define IVL_StringHeap_H
/*
 * Copyright (c) 2002-2021 Stephen Williams (steve@icarus.com)
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

# include  <string>

class perm_string {

    public:
      perm_string() : text_(0) { }
      perm_string(const perm_string&that) : text_(that.text_) { }
      ~perm_string() { }

      inline bool nil() const { return text_ == 0; }

      perm_string& operator = (const perm_string&that)
      { text_ = that.text_; return *this; }

      const char*str() const { return text_; }
      operator const char* () const { return str(); }

	// This is an escape for making perm_string objects out of
	// literals. For example, perm_string::literal("Label"); Please
	// do *not* cheat and pass arbitrary const char* items here.
      static perm_string literal(const char*t) { return perm_string(t); }

    private:
      friend class StringHeap;
      friend class StringHeapLex;
      explicit perm_string(const char*t) : text_(t) { };

    private:
      const char*text_;
};

extern const perm_string empty_perm_string;
extern bool operator == (perm_string a, perm_string b);
extern bool operator == (perm_string a, const char* b);
extern bool operator != (perm_string a, perm_string b);
extern bool operator != (perm_string a, const char* b);
extern bool operator >  (perm_string a, perm_string b);
extern bool operator <  (perm_string a, perm_string b);
extern bool operator >= (perm_string a, perm_string b);
extern bool operator <= (perm_string a, perm_string b);
extern std::ostream& operator << (std::ostream&out, perm_string that);

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
      perm_string make(const char*);

    private:
      char*cell_base_;
      unsigned cell_ptr_;

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
      perm_string make(const char*);
      perm_string make(const std::string&);

      unsigned add_count() const;
      unsigned add_hit_count() const;
      void cleanup();

    private:
      enum { HASH_SIZE = 4096 };
      const char*hash_table_[HASH_SIZE];

      unsigned add_count_;
      unsigned hit_count_;

    private: // not implemented
      StringHeapLex(const StringHeapLex&);
      StringHeapLex& operator= (const StringHeapLex&);
};

#endif /* IVL_StringHeap_H */
