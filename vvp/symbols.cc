/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
#ident "$Id: symbols.cc,v 1.3 2001/05/09 04:23:19 steve Exp $"
#endif

# include  "symbols.h"
# include  <string.h>
# include  <assert.h>

struct symbol_table_s {
      struct tree_node_*root;
};

/*
 * This is a B-Tree data structure, where there are nodes and
 * leaves.
 *
 * Nodes have a bunch of pointers to children. Each child pointer has
 * associated with it a key that is the largest key referenced by that
 * child. So, if the key being searched for has a value <= the first
 * key of a node, then the value is in the first child.
 *
 * leaves have a sorted table of key-value pairs. The search can use a
 * simple binary search to find an item. Each key represents an item.
 */

const unsigned leaf_width = 511;
const unsigned node_width = 511;

struct tree_node_ {
      bool leaf_flag;
      unsigned count;
      struct tree_node_*parent;

      union {
	    struct {
		  char*key;
		  symbol_value_t val;
	    } leaf[leaf_width];

	    struct tree_node_*child[node_width];
      };

};

static inline char* node_last_key(struct tree_node_*node)
{
      while (node->leaf_flag == false)
	    node = node->child[node->count-1];

      return node->leaf[node->count-1].key;
}

/*
 * Allocate a new symbol table means creating the table structure and
 * a root node, and initializing the pointers and members of the root
 * node. 
 */
symbol_table_t new_symbol_table(void)
{
      symbol_table_t tbl = new struct symbol_table_s;
      tbl->root = new struct tree_node_;
      tbl->root->leaf_flag = false;
      tbl->root->count = 0;
      tbl->root->parent = 0;
      return tbl;
}

static void split_node_(struct tree_node_*cur)
{
      assert(0);
}

/*
 * This function takes a leaf node and splits in into two. Move half
 * the leaf keys into the new node, and add the new leaf into the
 * parent node.
 */
static struct tree_node_* split_leaf_(struct tree_node_*cur)
{
      assert(cur->leaf_flag);
      assert(cur->parent);
      assert(! cur->parent->leaf_flag);

	/* Create a new leaf to hold half the data from the old leaf. */
      struct tree_node_*new_leaf = new struct tree_node_;
      new_leaf->leaf_flag = true;
      new_leaf->count = cur->count / 2;
      new_leaf->parent = cur->parent;

	/* Move the last half of the data from the end of the old leaf
	   to the beggining of the new leaf. At the same time, reduce
	   the size of the old leaf. */
      unsigned idx1 = new_leaf->count;
      unsigned idx2 = cur->count;
      while (idx1 > 0) {
	    idx1 -= 1;
	    idx2 -= 1;
	    new_leaf->leaf[idx1] = cur->leaf[idx2];
	    cur->count -= 1;
      }

      assert(new_leaf->count > 0);
      assert(cur->count > 0);

      unsigned idx = 0;
      while (cur->parent->child[idx] != cur) {
	    assert(idx < cur->parent->count);
	    idx += 1;
      }

      idx += 1;

      for (unsigned tmp = cur->parent->count ;  tmp > idx ;  tmp -= 1)
	    cur->parent->child[tmp] = cur->parent->child[tmp-1];

      cur->parent->child[idx] = new_leaf;
      cur->parent->count += 1;

      if (cur->parent->count == node_width)
	    split_node_(cur->parent);

      return new_leaf;
}


/*
 * This function searches tree recursively for the key. If the value
 * is not found (and we are at a leaf) then set the key with the given
 * value. If the key is found, set the value only if the force_flag is
 * true.
 */

static symbol_value_t find_value_(symbol_table_t tbl, struct tree_node_*cur,
				 const char*key, symbol_value_t val,
				 bool force_flag)
{
      if (cur->leaf_flag) {

	    unsigned idx = 0;
	    for (;;) {
		    /* If we run out of keys in the leaf, then add
		       this at the end of the leaf. */
		  if (idx == cur->count) {
			cur->leaf[idx].key = strdup(key);
			cur->leaf[idx].val = val;
			cur->count += 1;
			if (cur->count == leaf_width)
			      split_leaf_(cur);

			return val;
		  }

		  int rc = strcmp(key, cur->leaf[idx].key);

		    /* If we found the key already in the table, then
		       set the new value and break. */
		  if (rc == 0) {
			if (force_flag)
			      cur->leaf[idx].val = val;
			return cur->leaf[idx].val;
		  }

		    /* If this key goes before the current key, then
		       push all the following entries back and add
		       this key here. */
		  if (rc < 0) {
			for (unsigned tmp = cur->count; tmp > idx; tmp -= 1)
			      cur->leaf[tmp] = cur->leaf[tmp-1];

			cur->leaf[idx].key = strdup(key);
			cur->leaf[idx].val = val;
			cur->count += 1;
			if (cur->count == leaf_width)
			      split_leaf_(cur);

			return val;
		  }

		  idx += 1;
	    }

      } else {

	    unsigned idx = 0;
	    for (;;) {
		  if (idx == cur->count) {
			idx -= 1;
			return find_value_(tbl, cur->child[idx],
					  key, val, force_flag);
		  }

		  int rc = strcmp(key, node_last_key(cur->child[idx]));
		  if (rc <= 0) {
			return find_value_(tbl, cur->child[idx],
					  key, val, force_flag);
		  }

		  idx += 1;
	    }
      }

      assert(0);
      { symbol_value_t tmp;
        tmp.num = 0;
        return tmp;
      }
}

void sym_set_value(symbol_table_t tbl, const char*key, symbol_value_t val)
{
      if (tbl->root->count == 0) {
	      /* Handle the special case that this is the very first
		 value in the symbol table. Create the first leaf node
		 and initialize the pointers. */
	    struct tree_node_*cur = new struct tree_node_;
	    cur->leaf_flag = true;
	    cur->parent = tbl->root;
	    cur->count = 1;
	    cur->leaf[0].key = strdup(key);
	    cur->leaf[0].val = val;

	    tbl->root->count = 1;
	    tbl->root->child[0] = cur;
      } else {
	    find_value_(tbl, tbl->root, key, val, true);
      }
}

symbol_value_t sym_get_value(symbol_table_t tbl, const char*key)
{
      symbol_value_t def;
      def.num = 0;

      if (tbl->root->count == 0) {
	      /* Handle the special case that this is the very first
		 value in the symbol table. Create the first leaf node
		 and initialize the pointers. */
	    struct tree_node_*cur = new struct tree_node_;
	    cur->leaf_flag = true;
	    cur->parent = tbl->root;
	    cur->count = 1;
	    cur->leaf[0].key = strdup(key);
	    cur->leaf[0].val = def;

	    tbl->root->count = 1;
	    tbl->root->child[0] = cur;
	    return cur->leaf[0].val;
      } else {
	    return find_value_(tbl, tbl->root, key, def, false);
      }
}


/*
 * $Log: symbols.cc,v $
 * Revision 1.3  2001/05/09 04:23:19  steve
 *  Now that the interactive debugger exists,
 *  there is no use for the output dump.
 *
 * Revision 1.2  2001/03/18 00:37:55  steve
 *  Add support for vpi scopes.
 *
 * Revision 1.1  2001/03/11 00:29:39  steve
 *  Add the vvp engine to cvs.
 *
 */

