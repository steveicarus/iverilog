#ifndef __vvp_island_H
#define __vvp_island_H
/*
 * Copyright (c) 2008-2011 Stephen Williams (steve@icarus.com)
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

# include  "config.h"
# include  "vvp_net.h"
# include  "symbols.h"
# include  "schedule.h"
# include  <list>
# include  <cassert>

/*
* Islands are mutually connected bidirectional meshes that have a
* discipline other than the implicit ddiscipline of the rest of the
* run time.
*
* In the vvp input, an island is created with this record:
*
*    <label> .island ;
*
* The <label> is the name given to the island. Records after this
* build up the contents of the island. Ports are created like this:
*
*    <label> .port <island>, <src> ;
*    <label> .import <island>, <src> ;
*    <label> .export <island> ;
*
* The .port, .import and .export records create I/O, input and output
* ports. The <label> is the name that branches within the island can
* use to link to the port, and the <island> is the label for the
* island. The input and I/O ports have a <src> label that links to the
* source net from the ddiscrete domain.
*
* Branches within the island may only reference labels within the
* island. This keeps the nets of the ocean of digital away from the
* branches of analog within the island.
*/

struct vvp_island_branch;
class vvp_island_node;

class vvp_island  : private vvp_gen_event_s {

    public:
      vvp_island();
      virtual ~vvp_island();

	// Ports call this method to flag that something happened at
	// the input. The island will use this to create an active
	// event. The run_run() method will then be called by the
	// scheduler to process whatever happened.
      void flag_island();

	// This is the method that is called, eventually, to process
	// whatever happened. The derived island class implements this
	// method to give the island its character.
      virtual void run_island() =0;

    protected:
	// The base class collects a list of all the branches in the
	// island. The derived island class can access this list for
	// scanning the mesh.
      vvp_island_branch*branches_;

    public: /* These methods are used during linking. */

	// Add a port to the island. The key is added to the island
	// ports symbol table.
      void add_port(const char*key, vvp_net_t*net);
	// Add a branch to the island.
      void add_branch(vvp_island_branch*branch, const char*pa, const char*pb);

      vvp_net_t* find_port(const char*key);

	// Call this method when linking is done.
      void compile_cleanup(void);

    private:
      void run_run();
      bool flagged_;

    private:
	// During link, the vvp_island keeps these symbol tables for
	// mapping labels local to the island. When linking is done,
	// the compile_cleanup() method removes these tables.
      symbol_map_s<vvp_net_t>*ports_;
      symbol_map_s<vvp_island_branch>*anodes_;
      symbol_map_s<vvp_island_branch>*bnodes_;
};

/*
* An island port is a functor that connects to the ddiscrete
* discipline outside the island. (There is also a vvp_net_t object
* that refers to this port.) When data comes to the port from outside,
* it is collected and saved, and the island is notified. When code
* inside the island sends data out of the island, it uses the "out"
* pointer from the vvp_net_t that refers to this object.
*/

class vvp_island_port  : public vvp_net_fun_t {

    public:
      explicit vvp_island_port(vvp_island*ip);
      ~vvp_island_port();

      virtual void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                             vvp_context_t);
      virtual void recv_vec4_pv(vvp_net_ptr_t port, const vvp_vector4_t&bit,
				unsigned base, unsigned wid, unsigned vwid,
                                vvp_context_t);
      virtual void recv_vec8(vvp_net_ptr_t port, const vvp_vector8_t&bit);

      vvp_vector8_t invalue;
      vvp_vector8_t outvalue;
      vvp_vector8_t value;

    private:
      vvp_island*island_;

    private: // not implemented
      vvp_island_port(const vvp_island_port&);
      vvp_island_port& operator = (const vvp_island_port&);
};

inline vvp_vector8_t island_get_value(vvp_net_t*net)
{
      vvp_island_port*fun = dynamic_cast<vvp_island_port*>(net->fun);
      return fun->invalue;
}

extern void island_send_value(vvp_net_t*net, const vvp_vector8_t&val);

/*
* Branches are connected together to form a mesh of branches. Each
* endpoint (there are two) connects circularly to other branch
* endpoints that are connected together. This list of endpoints forms
* a node. Thus it is possible for branches to fully specify the mesh
* of the island.
*/

typedef vvp_sub_pointer_t<vvp_island_branch> vvp_branch_ptr_t;

struct vvp_island_branch {
      virtual ~vvp_island_branch();
	// Keep a list of branches in the island.
      vvp_island_branch*next_branch;
	// branch mesh connectivity. There is a pointer for each end
	// that participates in a circular list.
      vvp_sub_pointer_t<vvp_island_branch> link[2];
	// Port connections
      vvp_net_t*a;
      vvp_net_t*b;
};

/*
 * This function collects into the conn list all the branch ends
 * that are connected together with the reference branch endpoint
 * cur. All these branch ends together form a "node" of the branch
 * network. (In other words, this does *not* follow the branches.)
 */
extern void island_collect_node(std::list<vvp_branch_ptr_t>&conn, vvp_branch_ptr_t cur);

/*
 * These functions support compile/linking.
 */
extern void compile_island_base(char*label, vvp_island*use_island);
extern vvp_island* compile_find_island(const char*island_name);

#endif
