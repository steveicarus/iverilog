/*
 * Copyright (c) 2008-2021 Stephen Williams (steve@icarus.com)
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

# include "config.h"

# include <iostream>

# include  <typeinfo>
# include  <cstdlib>
# include  "compiler.h"
# include  "netlist.h"
# include  "netmisc.h"
# include  "ivl_target_priv.h"
# include  "ivl_assert.h"

using namespace std;

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

NetTran::NetTran(NetScope*scope__, perm_string n, ivl_switch_type_t tt,
                 unsigned width)
: NetNode(scope__, n, has_enable(tt)? 3 : 2), type_(tt), wid_(width)
{
      pin(0).set_dir(Link::PASSIVE);
      pin(1).set_dir(Link::PASSIVE);
      if (pin_count() == 3) {
	    pin(2).set_dir(Link::INPUT); // Enable
      }
      part_ = 0;
      off_ = 0;
}

NetTran::NetTran(NetScope*scope__, perm_string n, unsigned wid, unsigned part, unsigned off)
: NetNode(scope__, n, 2), type_(IVL_SW_TRAN_VP), wid_(wid), part_(part), off_(off)
{
      pin(0).set_dir(Link::PASSIVE);
      pin(1).set_dir(Link::PASSIVE);
}

NetTran::~NetTran()
{
}

unsigned NetTran::vector_width() const
{
      return wid_;
}

unsigned NetTran::part_width() const
{
      return part_;
}

unsigned NetTran::part_offset() const
{
      return off_;
}

void join_island(NetPins*obj)
{
      IslandBranch*branch = dynamic_cast<IslandBranch*> (obj);

	// If this is not even a branch, then stop now.
      if (branch == 0)
	    return;

	// If this is a branch, but already given to an island, then
	// stop.
      if (branch->island_)
	    return;

      list<NetObj*> uncommitted_neighbors;

	// Look for neighboring objects that might already be in
	// islands. If we find something, then join that island.
      for (unsigned idx = 0 ; idx < obj->pin_count() ; idx += 1) {
	    Nexus*nex = obj->pin(idx).nexus();
	    for (Link*cur = nex->first_nlink() ; cur ; cur = cur->next_nlink()) {
		  unsigned pin;
		  NetPins*tmp_pins;
		  cur->cur_link(tmp_pins, pin);

		  NetObj*tmp = dynamic_cast<NetObj*> (tmp_pins);
		  if (tmp == 0)
			continue;

		    // Skip self.
		  if (tmp == obj)
			continue;

		    // If tmp is not a branch, then skip it.
		  IslandBranch*tmp_branch = dynamic_cast<IslandBranch*> (tmp);
		  if (tmp_branch == 0)
			continue;

		    // If tmp is an uncommitted branch, then save
		    // it. When I finally choose an island for self,
		    // these branches will be scanned so that they join
		    // this island as well.
		  if (tmp_branch->island_ == 0) {
			uncommitted_neighbors.push_back(tmp);
			continue;
		  }

		  ivl_assert(*obj, branch->island_==0 || branch->island_==tmp_branch->island_);

		    // We found an existing island to join. Join it
		    // now. Keep scanning in order to find more neighbors.
		  if (branch->island_ == 0) {
			if (debug_elaborate)
			      cerr << obj->get_fileline() << ": debug: "
				   << "Join branch to existing island." << endl;
			branch->island_ = tmp_branch->island_;
			ivl_assert(*obj, branch->island_->discipline == tmp_branch->island_->discipline);

		  } else if (branch->island_ != tmp_branch->island_) {
			cerr << obj->get_fileline() << ": internal error: "
			     << "Oops, Found 2 neighboring islands." << endl;
			ivl_assert(*obj, 0);
		  }
	    }
      }

	// If after all that we did not find an island to join, then
	// start the island not and join it.
      if (branch->island_ == 0) {
	    branch->island_ = new ivl_island_s;
	    branch->island_->discipline = branch->discipline_;
	    if (debug_elaborate)
		  cerr << obj->get_fileline() << ": debug: "
		       << "Create new island for this branch" << endl;
      }

	// Now scan all the uncommitted neighbors I found. Calling
	// join_island() on them will cause them to notice me in the
	// process, and thus they will join my island. This process
	// will recurse until all the connected branches join this island.
      for (list<NetObj*>::iterator cur = uncommitted_neighbors.begin()
		 ; cur != uncommitted_neighbors.end() ; ++ cur ) {
	    join_island(*cur);
      }
}
