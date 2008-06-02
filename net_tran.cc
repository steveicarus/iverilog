/*
 * Copyright (c) 2008 Stephen Williams (steve@icarus.com)
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

# include "config.h"

# include <iostream>

# include  <typeinfo>
# include  <cstdlib>
# include  "compiler.h"
# include  "netlist.h"
# include  "netmisc.h"
# include  "ivl_target_priv.h"
# include  "ivl_assert.h"

static bool has_enable(ivl_switch_type_t tt)
{
      switch (tt) {
	  case IVL_SW_TRANIF0:
	  case IVL_SW_TRANIF1:
	  case IVL_SW_RTRANIF0:
	  case IVL_SW_RTRANIF1:
	    return true;
	  default:
	    return false;
      }
}

NetTran::NetTran(NetScope*scope, perm_string n, ivl_switch_type_t tt)
    : NetNode(scope, n, has_enable(tt)? 3 : 2), type_(tt)
{
      pin(0).set_dir(Link::PASSIVE); pin(0).set_name(perm_string::literal("A"), 0);
      pin(1).set_dir(Link::PASSIVE); pin(1).set_name(perm_string::literal("B"), 0);
      if (pin_count() == 3) {
	    pin(2).set_dir(Link::INPUT);
	    pin(2).set_name(perm_string::literal("E"), 0);
      }
}

NetTran::~NetTran()
{
}

void join_island(NetObj*obj)
{
      IslandBranch*branch = dynamic_cast<IslandBranch*> (obj);
      if (branch == 0)
	    return;

      ivl_assert(*obj, branch->island == 0);
      struct ivl_island_s*use_island = 0;

      for (unsigned idx = 0 ; idx < obj->pin_count() ; idx += 1) {
	    Nexus*nex = obj->pin(idx).nexus();
	    for (Link*cur = nex->first_nlink() ; cur ; cur = cur->next_nlink()) {
		  unsigned pin;
		  NetObj*tmp;
		  cur->cur_link(tmp, pin);
		  if (tmp == obj)
			continue;

		  IslandBranch*tmp_branch = dynamic_cast<IslandBranch*> (tmp);
		  if (tmp_branch == 0)
			continue;

		  ivl_assert(*tmp, tmp_branch->island);
		  ivl_assert(*obj, use_island==0 || use_island==tmp_branch->island);
		  use_island = tmp_branch->island;
	    }
      }

      if (use_island == 0) {
	    use_island = new ivl_island_s;
	    use_island->discipline = 0;
	    if (debug_elaborate)
		  cerr << obj->get_fileline() << ": debug: "
		       << "Create new island for this branch" << endl;
      } else {
	    if (debug_elaborate)
		  cerr << obj->get_fileline() << ": debug: "
		       << "Join this brach to existing island." << endl;
      }

      branch->island = use_island;
}
