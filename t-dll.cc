/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: t-dll.cc,v 1.12 2000/10/07 19:45:43 steve Exp $"
#endif

# include  "compiler.h"
# include  "t-dll.h"
# include  <dlfcn.h>
# include  <malloc.h>

static struct dll_target dll_target_obj;

/*
 * This function locates an ivl_scope_t object that matches the
 * NetScope object. The search works by looking for the parent scope,
 * then scanning the parent scope for the NetScope object.
 */
static ivl_scope_t find_scope(ivl_scope_t root, const NetScope*cur)
{
      ivl_scope_t parent, tmp;

      if (const NetScope*par = cur->parent()) {
	    parent = find_scope(root, par);

      } else {
	    assert(root->self == cur);
	    return root;
      }

      for (tmp = parent->child_ ;  tmp ;  tmp = tmp->sibling_)
	    if (tmp->self == cur)
		  return tmp;

      return 0;
}

void scope_add_logic(ivl_scope_t scope, ivl_net_logic_t net)
{
      if (scope->nlog_ == 0) {
	    scope->nlog_ = 1;
	    scope->log_ = (ivl_net_logic_t*)malloc(sizeof(ivl_net_logic_t));
	    scope->log_[0] = net;

      } else {
	    scope->nlog_ += 1;
	    scope->log_ = (ivl_net_logic_t*)
		  realloc(scope->log_, scope->nlog_*sizeof(ivl_net_logic_t));
	    scope->log_[scope->nlog_-1] = net;
      }

}

bool dll_target::start_design(const Design*des)
{
      dll_path_ = des->get_flag("DLL");
      dll_ = dlopen(dll_path_.c_str(), RTLD_NOW);
      if (dll_ == 0) {
	    cerr << dll_path_ << ": " << dlerror() << endl;
	    return false;
      }

      stmt_cur_ = 0;

	// Initialize the design object.
      des_.self = des;
      des_.root_ = (ivl_scope_t)calloc(1, sizeof(struct ivl_scope_s));
      des_.root_->self = des->find_root_scope();

      start_design_ = (start_design_f)dlsym(dll_, LU "target_start_design" TU);
      end_design_   = (end_design_f)  dlsym(dll_, LU "target_end_design" TU);

      net_const_  = (net_const_f) dlsym(dll_, LU "target_net_const" TU);
      net_event_  = (net_event_f) dlsym(dll_, LU "target_net_event" TU);
      net_logic_  = (net_logic_f) dlsym(dll_, LU "target_net_logic" TU);
      net_probe_  = (net_probe_f) dlsym(dll_, LU "target_net_probe" TU);
      net_signal_ = (net_signal_f)dlsym(dll_, LU "target_net_signal" TU);
      process_    = (process_f)   dlsym(dll_, LU "target_process" TU);
      scope_      = (scope_f)     dlsym(dll_, LU "target_scope" TU);

      (start_design_)(&des_);
      return true;
}

void dll_target::end_design(const Design*)
{
      (end_design_)(&des_);
      dlclose(dll_);
}

bool dll_target::bufz(const NetBUFZ*net)
{
      struct ivl_net_logic_s *obj = new struct ivl_net_logic_s;

      assert(net->pin_count() == 2);

      obj->type_ = IVL_LO_BUFZ;

      obj->npins_ = 2;
      obj->pins_ = new ivl_nexus_t[2];

      assert(net->pin(0).nexus()->t_cookie());
      obj->pins_[0] = (ivl_nexus_t) net->pin(0).nexus()->t_cookie();

      assert(net->pin(1).nexus()->t_cookie());
      obj->pins_[1] = (ivl_nexus_t) net->pin(1).nexus()->t_cookie();

      assert(net->scope());
      ivl_scope_t scope = find_scope(des_.root_, net->scope());
      assert(scope);

      scope_add_logic(scope, obj);

      if (net_logic_) {
	    (net_logic_)(net->name(), obj);

      } else {
	    cerr << dll_path_ << ": internal error: target DLL lacks "
		 << "target_net_logic function." << endl;
      }

      return true;
}

void dll_target::event(const NetEvent*net)
{
      if (net_event_) {
	    (net_event_)(net->full_name().c_str(), 0);

      } else {
	    cerr << dll_path_ << ": internal error: target DLL lacks "
		 << "target_net_event function." << endl;
      }

      return;
}

void dll_target::logic(const NetLogic*net)
{
      struct ivl_net_logic_s *obj = new struct ivl_net_logic_s;

      switch (net->type()) {
	  case NetLogic::AND:
	    obj->type_ = IVL_LO_AND;
	    break;
	  case NetLogic::BUF:
	    obj->type_ = IVL_LO_BUF;
	    break;
	  case NetLogic::OR:
	    obj->type_ = IVL_LO_OR;
	    break;
	  case NetLogic::XOR:
	    obj->type_ = IVL_LO_XOR;
	    break;
	  default:
	    assert(0);
	    obj->type_ = IVL_LO_NONE;
	    break;
      }

      obj->npins_ = net->pin_count();
      obj->pins_ = new ivl_nexus_t[obj->npins_];
      for (unsigned idx = 0 ;  idx < obj->npins_ ;  idx += 1) {
	    const Nexus*nex = net->pin(idx).nexus();
	    assert(nex->t_cookie());
	    obj->pins_[idx] = (ivl_nexus_t) nex->t_cookie();
      }

      assert(net->scope());
      ivl_scope_t scope = find_scope(des_.root_, net->scope());
      assert(scope);

      scope_add_logic(scope, obj);

      if (net_logic_) {
	    (net_logic_)(net->name(), obj);

      } else {
	    cerr << dll_path_ << ": internal error: target DLL lacks "
		 << "target_net_logic function." << endl;
      }
}

bool dll_target::net_const(const NetConst*net)
{
      unsigned idx;
      char*bits;

      struct ivl_net_const_s *obj = new struct ivl_net_const_s;

      obj->width_ = net->pin_count();
      if (obj->width_ <= sizeof(char*)) {
	    bits = obj->b.bit_;

      } else {
	    obj->b.bits_ = (char*)malloc(obj->width_);
	    bits = obj->b.bits_;
      }

      for (idx = 0 ;  idx < obj->width_ ;  idx += 1)
	    switch (net->value(idx)) {
		case verinum::V0:
		  bits[idx] = '0';
		  break;
		case verinum::V1:
		  bits[idx] = '1';
		  break;
		case verinum::Vx:
		  bits[idx] = 'x';
		  break;
		case verinum::Vz:
		  bits[idx] = 'z';
		  break;
	    }

	/* Connect to all the nexus objects. Note that the one-bit
	   case can be handled more efficiently without allocating
	   array space. */
      if (obj->width_ == 1) {
	    const Nexus*nex = net->pin(0).nexus();
	    assert(nex->t_cookie());
	    obj->n.pin_ = (ivl_nexus_t) nex->t_cookie();

      } else {
	    obj->n.pins_ = new ivl_nexus_t[obj->width_];
	    for (unsigned idx = 0 ;  idx < obj->width_ ;  idx += 1) {
		  const Nexus*nex = net->pin(idx).nexus();
		  assert(nex->t_cookie());
		  obj->n.pins_[idx] = (ivl_nexus_t) nex->t_cookie();
	    }
      }

	/* All done, call the target_net_const function if it exists. */
      if (net_const_) {
	    int rc = (net_const_)(net->name(), obj);
	    return rc == 0;

      } else {
	    cerr << dll_path_ << ": internal error: target DLL lacks "
		 << "target_net_const function." << endl;
	    return false;
      }

      return false;
}

void dll_target::net_probe(const NetEvProbe*net)
{
      if (net_probe_) {
	    int rc = (net_probe_)(net->name(), 0);
	    return;

      } else {
	    cerr << dll_path_ << ": internal error: target DLL lacks "
		 << "target_net_probe function." << endl;
	    return;
      }

      return;
}

void dll_target::scope(const NetScope*net)
{
      ivl_scope_t scope;

      if (net->parent() == 0) {
	    assert(des_.root_->self == net);
	    scope = des_.root_;

      } else {
	    scope = new struct ivl_scope_s;
	    scope->self = net;

	    ivl_scope_t parent = find_scope(des_.root_, net->parent());
	    assert(parent != 0);

	    scope->sibling_= parent->child_;
	    parent->child_ = scope;
      }

      if (scope_)
	    (scope_)(scope);
}

void dll_target::signal(const NetNet*net)
{
      ivl_signal_t obj = (ivl_signal_t)calloc(1, sizeof(struct ivl_signal_s));

	/* Attach the signal to the ivl_scope_t object that contains
	   it. This involves growing the sigs_ array in the scope
	   object, or creating the sigs_ array if this is the first
	   signal. */
      obj->scope_ = find_scope(des_.root_, net->scope());
      assert(obj->scope_);

      if (obj->scope_->nsigs_ == 0) {
	    assert(obj->scope_->sigs_ == 0);
	    obj->scope_->nsigs_ = 1;
	    obj->scope_->sigs_ = (ivl_signal_t*)malloc(sizeof(ivl_signal_t));

      } else {
	    assert(obj->scope_->sigs_);
	    obj->scope_->nsigs_ += 1;
	    obj->scope_->sigs_ = (ivl_signal_t*)
		  realloc(obj->scope_->sigs_,
			  obj->scope_->nsigs_*sizeof(ivl_signal_t));
	    obj->scope_->sigs_[obj->scope_->nsigs_-1] = obj;
      }

	/* Save the privitive properties of the signal in the
	   ivl_signal_t object. */

      obj->width_ = net->pin_count();
      obj->signed_= 0;

      switch (net->port_type()) {

	  case NetNet::PINPUT:
	    obj->port_ = IVL_SIP_INPUT;
	    break;

	  case NetNet::POUTPUT:
	    obj->port_ = IVL_SIP_OUTPUT;
	    break;

	  case NetNet::PINOUT:
	    obj->port_ = IVL_SIP_INOUT;
	    break;

	  default:
	    obj->port_ = IVL_SIP_NONE;
	    break;
      }

      switch (net->type()) {

	  case NetNet::REG:
	  case NetNet::INTEGER:
	    obj->type_ = IVL_SIT_REG;
	    break;

	  case NetNet::SUPPLY0:
	    obj->type_ = IVL_SIT_SUPPLY0;
	    break;

	  case NetNet::SUPPLY1:
	    obj->type_ = IVL_SIT_SUPPLY1;
	    break;

	  case NetNet::TRI:
	    obj->type_ = IVL_SIT_TRI;
	    break;

	  case NetNet::TRI0:
	    obj->type_ = IVL_SIT_TRI0;
	    break;

	  case NetNet::TRI1:
	    obj->type_ = IVL_SIT_TRI1;
	    break;

	  case NetNet::TRIAND:
	    obj->type_ = IVL_SIT_TRIAND;
	    break;

	  case NetNet::TRIOR:
	    obj->type_ = IVL_SIT_TRIOR;
	    break;

	  case NetNet::WAND:
	    obj->type_ = IVL_SIT_WAND;
	    break;

	  case NetNet::WIRE:
	  case NetNet::IMPLICIT:
	    obj->type_ = IVL_SIT_WIRE;
	    break;

	  case NetNet::WOR:
	    obj->type_ = IVL_SIT_WOR;
	    break;

	  default:
	    obj->type_ = IVL_SIT_NONE;
	    break;
      }

	/* Get the nexus objects for all the pins of the signal. If
	   the signal has only one pin, then write the single
	   ivl_nexus_t object into n.pin_. Otherwise, make an array of
	   ivl_nexus_t cookies.

	   When I create an ivl_nexus_t object, store it in the
	   t_cookie of the Nexus object so that I find it again when I
	   next encounter the nexus. */

      if (obj->width_ == 1) {
	    const Nexus*nex = net->pin(0).nexus();
	    if (nex->t_cookie()) {
		  obj->n.pin_ = (ivl_nexus_t)nex->t_cookie();

	    } else {
		  ivl_nexus_t tmp = (ivl_nexus_t)
			calloc(1, sizeof(struct ivl_nexus_s));
		  tmp->self = nex;
		  nex->t_cookie(tmp);
		  obj->n.pin_ = tmp;
	    }

      } else {
	    unsigned idx;

	    obj->n.pins_ = (ivl_nexus_t*)
		  calloc(obj->width_, sizeof(ivl_nexus_t));

	    for (idx = 0 ;  idx < obj->width_ ;  idx += 1) {
		  const Nexus*nex = net->pin(idx).nexus();
		  if (nex->t_cookie()) {
			obj->n.pins_[idx] = (ivl_nexus_t)nex->t_cookie();

		  } else {
			ivl_nexus_t tmp = (ivl_nexus_t)
			      calloc(1, sizeof(struct ivl_nexus_s));
			tmp->self = nex;
			nex->t_cookie(tmp);
			obj->n.pins_[idx] = tmp;
		  }
	    }
      }

	/* Invoke the target_net_signal function of the loaded target
	   module, if it exists. */

      if (net_signal_) {
	    int rc = (net_signal_)(net->name(), obj);
	    return;

      } else {
	    cerr << dll_path_ << ": internal error: target DLL lacks "
		 << "target_net_signal function." << endl;
	    return;
      }
}

extern const struct target tgt_dll = { "dll", &dll_target_obj };


/*
 * $Log: t-dll.cc,v $
 * Revision 1.12  2000/10/07 19:45:43  steve
 *  Put logic devices into scopes.
 *
 * Revision 1.11  2000/10/06 23:46:51  steve
 *  ivl_target updates, including more complete
 *  handling of ivl_nexus_t objects. Much reduced
 *  dependencies on pointers to netlist objects.
 *
 * Revision 1.10  2000/10/05 05:03:01  steve
 *  xor and constant devices.
 *
 * Revision 1.9  2000/09/30 02:18:15  steve
 *  ivl_expr_t support for binary operators,
 *  Create a proper ivl_scope_t object.
 *
 * Revision 1.8  2000/09/24 15:46:00  steve
 *  API access to signal type and port type.
 *
 * Revision 1.7  2000/09/18 01:24:32  steve
 *  Get the structure for ivl_statement_t worked out.
 *
 * Revision 1.6  2000/08/27 15:51:51  steve
 *  t-dll iterates signals, and passes them to the
 *  target module.
 *
 *  Some of NetObj should return char*, not string.
 *
 * Revision 1.5  2000/08/26 00:54:03  steve
 *  Get at gate information for ivl_target interface.
 *
 * Revision 1.4  2000/08/20 04:13:57  steve
 *  Add ivl_target support for logic gates, and
 *  make the interface more accessible.
 *
 * Revision 1.3  2000/08/19 18:12:42  steve
 *  Add target calls for scope, events and logic.
 *
 * Revision 1.2  2000/08/14 04:39:57  steve
 *  add th t-dll functions for net_const, net_bufz and processes.
 *
 * Revision 1.1  2000/08/12 16:34:37  steve
 *  Start stub for loadable targets.
 *
 */

