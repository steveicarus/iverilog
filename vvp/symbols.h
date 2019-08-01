#ifndef IVL_symbols_H
#define IVL_symbols_H
/*
 * Copyright (c) 2001-2014 Stephen Williams (steve@icarus.com)
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

/*
 * The symbol_table_t is intended as a means to hold and quickly index
 * large symbol tables with small symbol values. That is, the value
 * should fit in a 32bit unsigned integer (not even necessarily a
 * pointer.)
 *
 * The key is an unstructured ASCII string, terminated by a
 * null. Items added to the table are not removed, unless the entire
 * table is deleted.
 *
 * The compiler uses symbol tables to help match up operands to
 * referenced objects in the source. The compiler knows by the context
 * that the symbol appears what kind of thing is referenced, and so
 * what symbol table to look in.
 */

# include  "config.h"
# include  "vvp_net.h"

/*
 * This is the basic type of a symbol table. It is opaque. Don't even
 * try to look inside. The actual implementation is given in the
 * symbols.cc source file.
 */
typedef class symbol_table_s *symbol_table_t;

typedef struct symbol_value_s {
      union {
	    vvp_net_t*net;
	    void*ptr;
      };
} symbol_value_t;


class symbol_table_s {
    public:
      explicit symbol_table_s();
      virtual ~symbol_table_s();

	// This method locates the value in the symbol table and sets its
	// value. If the key doesn't yet exist, create it.
      void sym_set_value(const char*key, symbol_value_t val);

	// This method locates the value in the symbol table and returns
	// it. If the value does not exist, create it, initialize it with
	// zero and return the zero value.
      symbol_value_t sym_get_value(const char*key);

    private:
      symbol_table_s(const symbol_table_s&) { assert(0); };
      struct tree_node_*root;
      struct key_strings*str_chunk;
      unsigned str_used;

      symbol_value_t find_value_(struct tree_node_*cur,
				 const char*key, symbol_value_t val,
				 bool force_flag);
      char*key_strdup_(const char*str);
};

/*
 * Create a new symbol table or release an existing one. A new symbol
 * table has no keys and no values. As a symbol table is built up, it
 * consumes more and more memory. When the table is no longer needed,
 * the delete_symbol_table method will delete the table, including all
 * the space for the keys.
 */
inline symbol_table_t new_symbol_table(void) { return new symbol_table_s; }
inline void delete_symbol_table(symbol_table_t tbl) { delete tbl; }

// These are obsolete, and here only to support older code.
inline void sym_set_value(symbol_table_t tbl, const char*key, symbol_value_t val)
{ tbl->sym_set_value(key, val); }

inline symbol_value_t sym_get_value(symbol_table_t tbl, const char*key)
{ return tbl->sym_get_value(key); }

/*
 * This template is a type-safe interface to the symbol table.
 */
template <class T> class symbol_map_s : private symbol_table_s {

    public:
      void sym_set_value(const char*key, T*val)
      { symbol_value_t tmp;
	tmp.ptr = val;
	symbol_table_s::sym_set_value(key, tmp);
      }

      T* sym_get_value(const char*key)
      { symbol_value_t val = symbol_table_s::sym_get_value(key);
	return reinterpret_cast<T*>(val.ptr);
      }
};

#endif /* IVL_symbols_H */
