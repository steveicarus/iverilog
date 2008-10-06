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

# include  "vvp_island.h"
# include  "compile.h"
# include  "symbols.h"
# include  "schedule.h"
# include  <iostream>
# include  <list>
# include  <assert.h>
# include  <stdlib.h>
# include  <string.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif

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

class vvp_island_branch;
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

      virtual void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit);
      virtual void recv_vec4_pv(vvp_net_ptr_t port, const vvp_vector4_t&bit,
				unsigned base, unsigned wid, unsigned vwid);
      virtual void recv_vec8(vvp_net_ptr_t port, const vvp_vector8_t&bit);

      vvp_vector8_t invalue;
      vvp_vector8_t outvalue;

    private:
      vvp_island*island_;

    private: // not implemented
      vvp_island_port(const vvp_island_port&);
      vvp_island_port& operator = (const vvp_island_port&);
};

static vvp_vector8_t get_value(vvp_net_t*net)
{
      vvp_island_port*fun = dynamic_cast<vvp_island_port*>(net->fun);
      return fun->invalue;
}

static void send_value(vvp_net_t*net, const vvp_vector8_t&val)
{
      vvp_island_port*fun = dynamic_cast<vvp_island_port*>(net->fun);
      if (fun->outvalue .eeq(val))
	    return;

      fun->outvalue = val;
      vvp_send_vec8(net->out, fun->outvalue);
}

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

	// Behavior. (This stuff should be moved to a derived
	// class. The members here are specific to the tran island
	// class.)
      bool run_test_enabled();
      void run_resolution();
      bool active_high;
      bool enabled_flag;
      vvp_net_t*en;
      int flags;
      unsigned width, part, offset;
};

/*
 * Implementations...
 */
vvp_island::vvp_island()
{
      flagged_ = false;
      branches_ = 0;
      ports_ = 0;
      anodes_ = 0;
      bnodes_ = 0;
}

vvp_island::~vvp_island()
{
      assert(0);
}

void vvp_island::flag_island()
{
      if (flagged_ == true)
	    return;

      schedule_generic(this, 0, false, false);
      flagged_ = true;
}

/*
* This method handles the callback from the scheduler. It does basic
* housecleaning and calls the run_island() method implemented by the
* derived class.
*/
void vvp_island::run_run()
{
      flagged_ = false;
      run_island();
}


void vvp_island::add_port(const char*key, vvp_net_t*net)
{
      if (ports_ == 0)
	    ports_ = new symbol_map_s<vvp_net_t>;

      ports_->sym_set_value(key, net);
}

void vvp_island::add_branch(vvp_island_branch*branch, const char*pa, const char*pb)
{
      vvp_island_branch*cur;
      assert(ports_);
      branch->a = ports_->sym_get_value(pa);
      branch->b = ports_->sym_get_value(pb);
      assert(branch->a && branch->b);

      vvp_branch_ptr_t ptra (branch, 0);
      vvp_branch_ptr_t ptrb (branch, 1);
      if (anodes_ == 0)
	    anodes_ = new symbol_map_s<vvp_island_branch>;
      if (bnodes_ == 0)
	    bnodes_ = new symbol_map_s<vvp_island_branch>;

      if ((cur = anodes_->sym_get_value(pa))) {
	    branch->link[0] = cur->link[0];
	    cur->link[0] = ptra;
      } else if ((cur = bnodes_->sym_get_value(pa))) {
	    branch->link[0] = cur->link[1];
	    cur->link[1] = ptra;
      } else {
	    branch->link[0] = ptra;
	    anodes_->sym_set_value(pa, branch);
      }

      if ((cur = anodes_->sym_get_value(pb))) {
	    branch->link[1] = cur->link[0];
	    cur->link[0] = ptrb;
      } else if ((cur = bnodes_->sym_get_value(pb))) {
	    branch->link[1] = cur->link[1];
	    cur->link[1] = ptrb;
      } else {
	    branch->link[1] = ptrb;
	    bnodes_->sym_set_value(pb, branch);
      }

      branch->next_branch = branches_;
      branches_ = branch;
}

vvp_net_t* vvp_island::find_port(const char*key)
{
      if (ports_ == 0)
	    return 0;
      else
	    return ports_->sym_get_value(key);
}

void vvp_island::compile_cleanup()
{
      if (ports_) {
	    delete ports_;
	    ports_ = 0;
      }
      if (anodes_) {
	    delete anodes_;
	    anodes_ = 0;
      }
      if (bnodes_) {
	    delete bnodes_;
	    bnodes_ = 0;
      }
}

vvp_island_port::vvp_island_port(vvp_island*ip)
: island_(ip)
{
}

vvp_island_port::~vvp_island_port()
{
}

void vvp_island_port::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit)
{
      recv_vec8(port, vvp_vector8_t(bit, 6, 6));
}

void vvp_island_port::recv_vec4_pv(vvp_net_ptr_t port, const vvp_vector4_t&bit,
				   unsigned base, unsigned wid, unsigned vwid)
{
      vvp_vector8_t tmp(bit, 6, 6);
      if (invalue.size()==0) {
	    assert(tmp.size() == wid);
	    invalue = part_expand(tmp, vwid, base);
      } else {
	    assert(invalue.size() == vwid);
	    for (unsigned idx = 0 ; idx < wid ; idx += 1) {
		  if ((base+idx) >= invalue.size())
			break;
		  invalue.set_bit(base+idx, tmp.value(idx));
	    }
      }

      island_->flag_island();
}


void vvp_island_port::recv_vec8(vvp_net_ptr_t port, const vvp_vector8_t&bit)
{
      if (invalue .eeq(bit))
	    return;

      invalue = bit;
      island_->flag_island();
}

vvp_island_branch::~vvp_island_branch()
{
}

/* **** TRANIF SUPPORT **** */

class vvp_island_tran : public vvp_island {

    public:
      void run_island();
};

void vvp_island_tran::run_island()
{
	// Test to see if any of the branches are enabled.
      bool runnable = false;
      for (vvp_island_branch*cur = branches_ ; cur ; cur = cur->next_branch) {
	    runnable |= cur->run_test_enabled();
      }
      if (runnable == false)
	    return;

      for (vvp_island_branch*cur = branches_ ; cur ; cur = cur->next_branch)
	    cur->run_resolution();
}

bool vvp_island_branch::run_test_enabled()
{
      flags = 0;

      vvp_island_port*ep = en? dynamic_cast<vvp_island_port*> (en->fun) : 0;

	// If there is no ep port (no "enabled" input) then this is a
	// tran branch. Assume it is always enabled.
      if (ep == 0) {
	    enabled_flag = true;
	    return true;
      }

      enabled_flag = false;
      vvp_bit4_t enable_val = ep->invalue.value(0).value();

      if (active_high==true && enable_val != BIT4_1)
	    return false;

      if (active_high==false && enable_val != BIT4_0)
	    return false;

      enabled_flag = true;
      return true;
}

static vvp_branch_ptr_t next(vvp_branch_ptr_t cur)
{
      vvp_island_branch*ptr = cur.ptr();
      unsigned ab = cur.port();
      return ptr->link[ab];
}

static void collect_node(list<vvp_branch_ptr_t>&conn, vvp_branch_ptr_t cur)
{
      conn .push_back(cur);
      for (vvp_branch_ptr_t idx = next(cur) ; idx != cur ; idx = next(idx))
	    conn.push_back(idx);
}

static void mark_done_flags(list<vvp_branch_ptr_t>&connections)
{
      for (list<vvp_branch_ptr_t>::iterator idx = connections.begin()
		 ; idx != connections.end() ; idx ++ ) {

	    vvp_island_branch*tmp_ptr = idx->ptr();
	    unsigned tmp_ab = idx->port();
	    tmp_ptr->flags |= 1 << tmp_ab;
      }
}

static void mark_visited_flags(list<vvp_branch_ptr_t>&connections)
{
      for (list<vvp_branch_ptr_t>::iterator idx = connections.begin()
		 ; idx != connections.end() ; idx ++ ) {

	    vvp_island_branch*tmp_ptr = idx->ptr();
	    unsigned tmp_ab = idx->port();
	    tmp_ptr->flags |= 4 << tmp_ab;
      }
}

static void clear_visited_flags(list<vvp_branch_ptr_t>&connections)
{
      for (list<vvp_branch_ptr_t>::iterator idx = connections.begin()
		 ; idx != connections.end() ; idx ++ ) {

	    vvp_island_branch*tmp_ptr = idx->ptr();
	    unsigned tmp_ab = idx->port();
	    tmp_ptr->flags &= ~(4 << tmp_ab);
      }
}

static vvp_vector8_t get_value_from_branch(vvp_branch_ptr_t cur);

static void resolve_values_from_connections(vvp_vector8_t&val,
					    list<vvp_branch_ptr_t>&connections)
{
      for (list<vvp_branch_ptr_t>::iterator idx = connections.begin()
		 ; idx != connections.end() ; idx ++ ) {
	    vvp_vector8_t tmp = get_value_from_branch(*idx);
	    if (val.size() == 0)
		  val = tmp;
	    else if (tmp.size() != 0)
		  val = resolve(val, tmp);
      }
}

static vvp_vector8_t get_value_from_branch(vvp_branch_ptr_t cur)
{
      vvp_island_branch*ptr = cur.ptr();
      unsigned ab = cur.port();
      unsigned ab_other = ab^1;

	// If the branch link is disabled, return nil.
      if (ptr->enabled_flag == false)
	    return vvp_vector8_t();

      vvp_branch_ptr_t other (ptr, ab_other);

	// If the branch other side is already visited, return nil.
      if (ptr->flags & (4<<ab_other))
	    return vvp_vector8_t();

	// Other side net, and port value.
      vvp_net_t*net_other = ab? ptr->a : ptr->b;
      vvp_vector8_t val_other = get_value(net_other);

	// recurse
      list<vvp_branch_ptr_t> connections;
      collect_node(connections, other);
      mark_visited_flags(connections);

      resolve_values_from_connections(val_other, connections);

	// Remove visited flag
      clear_visited_flags(connections);

      if (val_other.size() == 0)
	    return val_other;

      if (ptr->width) {
	    if (ab == 0) {
		  val_other = part_expand(val_other, ptr->width, ptr->offset);

	    } else {
		  val_other = val_other.subvalue(ptr->offset, ptr->part);

	    }
      }

      return val_other;
}

static void push_value_through_branches(const vvp_vector8_t&val,
					list<vvp_branch_ptr_t>&connections)
{
      for (list<vvp_branch_ptr_t>::iterator idx = connections.begin()
		 ; idx != connections.end() ; idx ++ ) {

	    vvp_island_branch*tmp_ptr = idx->ptr();
	    unsigned tmp_ab = idx->port();
	    unsigned other_ab = tmp_ab^1;

	      // If other side already done, skip
	    if (tmp_ptr->flags & (1<<other_ab))
		  continue;

	      // If link is not enabled, skip.
	    if (! tmp_ptr->enabled_flag)
		  continue;

	    vvp_net_t*other_net = other_ab? tmp_ptr->b : tmp_ptr->a;

	    if (tmp_ptr->width == 0) {
		    // Mark this end as done
		  tmp_ptr->flags |= (1 << other_ab);
		  send_value(other_net, val);

	    } if (other_ab == 1) {
		    // Mark as done
		  tmp_ptr->flags |= (1 << other_ab);
		  vvp_vector8_t tmp = val.subvalue(tmp_ptr->offset, tmp_ptr->part);
		  send_value(other_net, tmp);
	    } else {
		    // Otherwise, the other side is not fully
		    // specified, so we can't take this shortcut.
	    }
      }
}

void vvp_island_branch::run_resolution()
{
	// Collect all the branch endpoints that are joined to my A
	// side.
      list<vvp_branch_ptr_t> connections;
      bool processed_a_side = false;
      vvp_vector8_t val;

      if ((flags & 1) == 0) {
	    processed_a_side = true;
	    vvp_branch_ptr_t a_side(this, 0);
	    collect_node(connections, a_side);

	      // Mark my A side as done. Do this early to prevent recursing
	      // back. All the connections that share this port are also
	      // done. Make sure their flags are set appropriately.
	    mark_done_flags(connections);

	    val = get_value(a);
	    mark_visited_flags(connections); // Mark as visited.


	      // Now scan the other sides of all the branches connected to
	      // my A side. The get_value_from_branch() will recurse as
	      // necessary to depth-first walk the graph.
	    resolve_values_from_connections(val, connections);

	      // A side is done.
	    send_value(a, val);

	      // Clear the visited flags. This must be done so that other
	      // branches can read this input value.
	    clear_visited_flags(connections);

	      // Try to push the calculated value out through the
	      // branches. This is useful for A-side results because
	      // there is a high probability that the other side of
	      // all the connected branches is fully specified by this
	      // result.
	    push_value_through_branches(val, connections);
      }

	// If the B side got taken care of by above, then this branch
	// is done. Stop now.
      if (flags & 2)
	    return;

	// Repeat the above for the B side.

      connections.clear();
      collect_node(connections, vvp_branch_ptr_t(this, 1));
      mark_done_flags(connections);

      if (enabled_flag && processed_a_side) {
	      // If this is a connected branch, then we know from the
	      // start that we have all the bits needed to complete
	      // the B side. Even if the B side is a part select, the
	      // simple part select must be correct because the
	      // recursive resolve_values_from_connections above must
	      // of cycled back to the B side of myself when resolving
	      // the connections.
	    if (width != 0)
		  val = val.subvalue(offset, part);

      } else {

	      // If this branch is not enabled, then the B-side must
	      // be processed on its own.
	    val = get_value(b);
	    mark_visited_flags(connections);
	    resolve_values_from_connections(val, connections);
	    clear_visited_flags(connections);
      }

      send_value(b, val);
}

/* **** COMPILE/LINK SUPPORT **** */

/*
* We need to keep an island symbol table to make island labels to
* islands, and we need a list of the islands that we can run through
* during cleanup. After linking is done, the compile_island_cleanup() is
* called to erase both.
*/
static list<vvp_island*> island_list;
static symbol_map_s<vvp_island>* island_table = 0;

void compile_island(char*label, char*type)
{
      if (island_table == 0)
	    island_table = new symbol_map_s<vvp_island>;

      vvp_island*use_island = 0;

      if (strcmp(type,"tran") == 0) {
	    use_island = new vvp_island_tran;
      } else {
	    assert(0);
      }

      island_table->sym_set_value(label, use_island);
      island_list.push_back(use_island);
      free(label);
      free(type);
}

void compile_island_port(char*label, char*island, char*src)
{
      assert(island_table);
      vvp_island*use_island = island_table->sym_get_value(island);
      assert(use_island);
      free(island);

      vvp_net_t*net = new vvp_net_t;
      vvp_island_port*fun = new vvp_island_port(use_island);
      net->fun = fun;

	// Get the source from outside the island
      input_connect(net, 0, src);

	// Define the functor outside the island.
      define_functor_symbol(label, net);

	// Also define it inside the island.
      use_island->add_port(label, net);

      free(label);
}

void compile_island_export(char*label, char*island)
{
      fprintf(stderr, "XXXX %s .export %s;\n", label, island);
      free(label);
      free(island);
}

void compile_island_import(char*label, char*island, char*src)
{
      assert(island_table);
      vvp_island*use_island = island_table->sym_get_value(island);
      assert(use_island);
      free(island);

      vvp_net_t*net = new vvp_net_t;
      vvp_island_port*fun = new vvp_island_port(use_island);
      net->fun = fun;

	// Get the source from outside the island
      input_connect(net, 0, src);

	// Define the functor only inside the island.
      use_island->add_port(label, net);

      free(label);
}

void compile_island_tranif(int sense, char*island, char*pa, char*pb, char*pe)
{
      assert(island_table);
      vvp_island*use_island = island_table->sym_get_value(island);
      assert(use_island);
      free(island);

      vvp_island_branch*br = new vvp_island_branch;
      if (sense)
	    br->active_high = true;
      else
	    br->active_high = false;

      if (pe == 0) {
	    br->en = 0;
      } else {
	    br->en = use_island->find_port(pe);
	    assert(br->en);
	    free(pe);
      }

      br->width = 0;
      br->part = 0;
      br->offset = 0;

      use_island->add_branch(br, pa, pb);

      free(pa);
      free(pb);
}

void compile_island_tranvp(char*island, char*pa, char*pb,
			   unsigned wid, unsigned par, unsigned off)
{
      assert(island_table);
      vvp_island*use_island = island_table->sym_get_value(island);
      assert(use_island);
      free(island);

      vvp_island_branch*br = new vvp_island_branch;
      br->active_high = false;
      br->en = 0;
      br->width = wid;
      br->part = par;
      br->offset = off;

      use_island->add_branch(br, pa, pb);

      free(pa);
      free(pb);
}

void compile_island_cleanup(void)
{
	// Call the per-island cleanup to get rid of local symbol tables.
      for (list<vvp_island*>::iterator cur = island_list.begin()
		 ; cur != island_list.end() ; cur ++ ) {
	    (*cur)->compile_cleanup();
      }

      island_list.clear();

	// Remove the island symbol table itself.
      if (island_table) {
	    delete island_table;
	    island_table = 0;
      }
}
