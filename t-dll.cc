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
#ident "$Id: t-dll.cc,v 1.30 2001/03/28 06:07:39 steve Exp $"
#endif

# include  "compiler.h"
# include  "t-dll.h"
# include  <malloc.h>

#if defined(HAVE_DLFCN_H)
inline ivl_dll_t ivl_dlopen(const char*name)
{ return dlopen(name,RTLD_NOW); }

inline void* ivl_dlsym(ivl_dll_t dll, const char*nm)
{ return dlsym(dll, nm); }

inline void ivl_dlclose(ivl_dll_t dll)
{ dlclose(dll); }

#elif defined(HAVE_DL_H)
inline ivl_dll_t ivl_dlopen(const char*name)
{ return shl_load(name, BIND_IMMEDIATE, 0); }

inline void* ivl_dlsym(ivl_dll_t dll, const char*nm)
{
      void*sym;
      int rc = shl_findsym(&dll, nm, TYPE_PROCEDURE, &sym);
      return (rc == 0) ? sym : 0;
}

inline void ivl_dlclose(ivl_dll_t dll)
{ shl_unload(dll); }

inline const char*dlerror(void)
{ return strerror( errno ); }
#endif

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
	    assert(strcmp(root->name_, cur->name().c_str()) == 0);
	    return root;
      }

      for (tmp = parent->child_ ;  tmp ;  tmp = tmp->sibling_)
	    if (strcmp(tmp->name_, cur->name().c_str()) == 0)
		  return tmp;

      return 0;
}

ivl_scope_t dll_target::lookup_scope_(const NetScope*cur)
{
      return find_scope(des_.root_, cur);
}

static ivl_nexus_t nexus_sig_make(ivl_signal_t net, unsigned pin)
{
      ivl_nexus_t tmp = new struct ivl_nexus_s;
      tmp->nptr_ = 1;
      tmp->ptrs_ = (struct ivl_nexus_ptr_s*)
	    malloc(sizeof(struct ivl_nexus_ptr_s));
      tmp->ptrs_[0].pin_  = pin;
      tmp->ptrs_[0].type_ = __NEXUS_PTR_SIG;
      tmp->ptrs_[0].l.sig = net;
      return tmp;
}

static void nexus_sig_add(ivl_nexus_t nex, ivl_signal_t net, unsigned pin)
{
      unsigned top = nex->nptr_ + 1;
      nex->ptrs_ = (struct ivl_nexus_ptr_s*)
	    realloc(nex->ptrs_, top * sizeof(struct ivl_nexus_ptr_s));
      nex->nptr_ = top;

      nex->ptrs_[top-1].type_= __NEXUS_PTR_SIG;
      nex->ptrs_[top-1].pin_ = pin;
      nex->ptrs_[top-1].l.sig= net;
}

static void nexus_log_add(ivl_nexus_t nex, ivl_net_logic_t net, unsigned pin)
{
      unsigned top = nex->nptr_ + 1;
      nex->ptrs_ = (struct ivl_nexus_ptr_s*)
	    realloc(nex->ptrs_, top * sizeof(struct ivl_nexus_ptr_s));
      nex->nptr_ = top;

      nex->ptrs_[top-1].type_= __NEXUS_PTR_LOG;
      nex->ptrs_[top-1].pin_ = pin;
      nex->ptrs_[top-1].l.log= net;
}

static void nexus_con_add(ivl_nexus_t nex, ivl_net_const_t net, unsigned pin)
{
      unsigned top = nex->nptr_ + 1;
      nex->ptrs_ = (struct ivl_nexus_ptr_s*)
	    realloc(nex->ptrs_, top * sizeof(struct ivl_nexus_ptr_s));
      nex->nptr_ = top;

      nex->ptrs_[top-1].type_= __NEXUS_PTR_CON;
      nex->ptrs_[top-1].pin_ = pin;
      nex->ptrs_[top-1].l.con= net;
}

static void nexus_lpm_add(ivl_nexus_t nex, ivl_lpm_t net, unsigned pin)
{
      unsigned top = nex->nptr_ + 1;
      nex->ptrs_ = (struct ivl_nexus_ptr_s*)
	    realloc(nex->ptrs_, top * sizeof(struct ivl_nexus_ptr_s));
      nex->nptr_ = top;

      nex->ptrs_[top-1].type_= __NEXUS_PTR_LPM;
      nex->ptrs_[top-1].pin_ = pin;
      nex->ptrs_[top-1].l.lpm= net;
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

void scope_add_event(ivl_scope_t scope, ivl_event_t net)
{
      if (scope->nevent_ == 0) {
	    scope->nevent_ = 1;
	    scope->event_ = (ivl_event_t*)malloc(sizeof(ivl_event_t));
	    scope->event_[0] = net;

      } else {
	    scope->nevent_ += 1;
	    scope->event_ = (ivl_event_t*)
		  realloc(scope->event_, scope->nevent_*sizeof(ivl_event_t));
	    scope->event_[scope->nevent_-1] = net;
      }

}

static void scope_add_lpm(ivl_scope_t scope, ivl_lpm_t net)
{
      if (scope->nlpm_ == 0) {
	    assert(scope->lpm_ == 0);
	    scope->nlpm_ = 1;
	    scope->lpm_ = (ivl_lpm_t*)malloc(sizeof(ivl_lpm_t));
	    scope->lpm_[0] = net;

      } else {
	    assert(scope->lpm_);
	    scope->nlpm_ += 1;
	    scope->lpm_   = (ivl_lpm_t*)
		  realloc(scope->lpm_,
			  scope->nlpm_*sizeof(ivl_lpm_t));
	    scope->lpm_[scope->nlpm_-1] = net;
      }
}

bool dll_target::start_design(const Design*des)
{
      dll_path_ = des->get_flag("DLL");
      dll_ = ivl_dlopen(dll_path_.c_str());
      if (dll_ == 0) {
	    cerr << dll_path_ << ": " << dlerror() << endl;
	    return false;
      }

      stmt_cur_ = 0;

	// Initialize the design object.
      des_.self = des;
      des_.root_ = new struct ivl_scope_s;
      des_.root_->name_ = strdup(des->find_root_scope()->name().c_str());
      des_.root_->child_ = 0;
      des_.root_->sibling_ = 0;
      des_.root_->nsigs_ = 0;
      des_.root_->sigs_ = 0;
      des_.root_->nlog_ = 0;
      des_.root_->log_ = 0;
      des_.root_->nlpm_ = 0;
      des_.root_->lpm_ = 0;
      des_.root_->type_ = IVL_SCT_MODULE;
      des_.root_->tname_ = des_.root_->name_;

      target_ = (target_design_f)ivl_dlsym(dll_, LU "target_design" TU);
      if (target_ == 0) {
	    cerr << dll_path_ << ": error: target_design entry "
		  "point is missing." << endl;
	    return false;
      }

      return true;
}

/*
 * Here ivl is telling us that the design is scanned completely, and
 * here is where we call the API to process the constructed design.
 */
int dll_target::end_design(const Design*)
{
      int rc = (target_)(&des_);
      ivl_dlclose(dll_);
      return rc;
}

/*
 * Add a bufz object to the scope that contains it.
 *
 * Note that in the ivl_target API a BUFZ device is a special kind of
 * ivl_net_logic_t device, so create an ivl_net_logic_t cookie to
 * handle it.
 */
bool dll_target::bufz(const NetBUFZ*net)
{
      struct ivl_net_logic_s *obj = new struct ivl_net_logic_s;

      assert(net->pin_count() == 2);

      obj->type_ = IVL_LO_BUFZ;

      obj->npins_ = 2;
      obj->pins_ = new ivl_nexus_t[2];

	/* Get the ivl_nexus_t objects connected to the two pins.

	   (We know a priori that the ivl_nexus_t objects have been
	   allocated, because the signals have been scanned before
	   me. This saves me the trouble of allocating them.) */

      assert(net->pin(0).nexus()->t_cookie());
      obj->pins_[0] = (ivl_nexus_t) net->pin(0).nexus()->t_cookie();
      nexus_log_add(obj->pins_[0], obj, 0);

      assert(net->pin(1).nexus()->t_cookie());
      obj->pins_[1] = (ivl_nexus_t) net->pin(1).nexus()->t_cookie();
      nexus_log_add(obj->pins_[1], obj, 1);


	/* Attach the logic device to the scope that contains it. */

      assert(net->scope());
      ivl_scope_t scope = find_scope(des_.root_, net->scope());
      assert(scope);

      obj->scope_ = scope;
      obj->name_ = strdup(net->name());
      scope_add_logic(scope, obj);

      return true;
}

void dll_target::event(const NetEvent*net)
{
      struct ivl_event_s *obj = new struct ivl_event_s;

      ivl_scope_t scope = find_scope(des_.root_, net->scope());
      obj->name = strdup(net->full_name().c_str());
      obj->scope = scope;
      scope_add_event(scope, obj);

      assert(net->nprobe() <= 1);

      if (net->nprobe() == 1) {
	    const NetEvProbe*pr = net->probe(0);
	    switch (pr->edge()) {
		case NetEvProbe::ANYEDGE:
		  obj->edge = IVL_EDGE_ANY;
		  break;
		case NetEvProbe::NEGEDGE:
		  obj->edge = IVL_EDGE_NEG;
		  break;
		case NetEvProbe::POSEDGE:
		  obj->edge = IVL_EDGE_POS;
		  break;
	    }

	    obj->npins = pr->pin_count();
	    obj->pins = (ivl_nexus_t*)calloc(obj->npins, sizeof(ivl_nexus_t));
#if 0
	    for (unsigned idx = 0 ;  idx < obj->npins ;  idx += 1) {
		  ivl_nexus_t nex = (ivl_nexus_t)
			pr->pin(idx).nexus()->t_cookie();
		  assert(nex);
		  obj->pins[idx] = nex;
	    }
#endif
      } else {
	    obj->npins = 0;
	    obj->pins  = 0;
	    obj->edge = IVL_EDGE_NONE;
      }

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
	  case NetLogic::BUFIF0:
	    obj->type_ = IVL_LO_BUFIF0;
	    break;
	  case NetLogic::BUFIF1:
	    obj->type_ = IVL_LO_BUFIF1;
	    break;
	  case NetLogic::NAND:
	    obj->type_ = IVL_LO_NAND;
	    break;
	  case NetLogic::NMOS:
	    obj->type_ = IVL_LO_NMOS;
	    break;
	  case NetLogic::NOR:
	    obj->type_ = IVL_LO_NOR;
	    break;
	  case NetLogic::NOT:
	    obj->type_ = IVL_LO_NOT;
	    break;
	  case NetLogic::NOTIF0:
	    obj->type_ = IVL_LO_NOTIF0;
	    break;
	  case NetLogic::NOTIF1:
	    obj->type_ = IVL_LO_NOTIF1;
	    break;
	  case NetLogic::OR:
	    obj->type_ = IVL_LO_OR;
	    break;
	  case NetLogic::RNMOS:
	    obj->type_ = IVL_LO_RNMOS;
	    break;
	  case NetLogic::RPMOS:
	    obj->type_ = IVL_LO_RPMOS;
	    break;
	  case NetLogic::PMOS:
	    obj->type_ = IVL_LO_PMOS;
	    break;
	  case NetLogic::XNOR:
	    obj->type_ = IVL_LO_XNOR;
	    break;
	  case NetLogic::XOR:
	    obj->type_ = IVL_LO_XOR;
	    break;
	  default:
	    assert(0);
	    obj->type_ = IVL_LO_NONE;
	    break;
      }

	/* Connect all the ivl_nexus_t objects to the pins of the
	   device. */

      obj->npins_ = net->pin_count();
      obj->pins_ = new ivl_nexus_t[obj->npins_];
      for (unsigned idx = 0 ;  idx < obj->npins_ ;  idx += 1) {
	    const Nexus*nex = net->pin(idx).nexus();
	    assert(nex->t_cookie());
	    obj->pins_[idx] = (ivl_nexus_t) nex->t_cookie();
	    nexus_log_add(obj->pins_[idx], obj, idx);
      }

      assert(net->scope());
      ivl_scope_t scope = find_scope(des_.root_, net->scope());
      assert(scope);

      obj->scope_= scope;
      obj->name_ = strdup(net->name());

      scope_add_logic(scope, obj);
}

void dll_target::lpm_ff(const NetFF*net)
{
      ivl_lpm_ff_t obj = new struct ivl_lpm_ff_s;
      obj->base.type  = IVL_LPM_FF;
      obj->base.name  = strdup(net->name());
      obj->base.width = net->width();
      obj->base.scope = find_scope(des_.root_, net->scope());
      assert(obj->base.scope);

      scope_add_lpm(obj->base.scope, &obj->base);

      const Nexus*nex;

	/* Set the clk signal to point to the nexus, and the nexus to
	   point back to this device. */
      nex = net->pin_Clock().nexus();
      assert(nex->t_cookie());
      obj->clk = (ivl_nexus_t) nex->t_cookie();
      assert(obj->clk);
      nexus_lpm_add(obj->clk, &obj->base, 0);

      if (obj->base.width == 1) {
	    nex = net->pin_Q(0).nexus();
	    assert(nex->t_cookie());
	    obj->q.pin = (ivl_nexus_t) nex->t_cookie();
	    nexus_lpm_add(obj->q.pin, &obj->base, 0);

	    nex = net->pin_Data(0).nexus();
	    assert(nex->t_cookie());
	    obj->d.pin = (ivl_nexus_t) nex->t_cookie();
	    nexus_lpm_add(obj->d.pin, &obj->base, 0);

      } else {
	    obj->q.pins = new ivl_nexus_t [obj->base.width * 2];
	    obj->d.pins = obj->q.pins + obj->base.width;

	    for (unsigned idx = 0 ;  idx < obj->base.width ;  idx += 1) {
		  nex = net->pin_Q(idx).nexus();
		  assert(nex->t_cookie());
		  obj->q.pins[idx] = (ivl_nexus_t) nex->t_cookie();
		  nexus_lpm_add(obj->q.pins[idx], &obj->base, 0);

		  nex = net->pin_Data(idx).nexus();
		  assert(nex->t_cookie());
		  obj->d.pins[idx] = (ivl_nexus_t) nex->t_cookie();
		  nexus_lpm_add(obj->d.pins[idx], &obj->base, 0);
	    }
      }
}

/*
 * The assignment l-values are captured by the assignment statements
 * themselves in the process handling.
 */
void dll_target::net_assign(const NetAssign_*)
{
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
	    nexus_con_add(obj->n.pin_, obj, 0);

      } else {
	    obj->n.pins_ = new ivl_nexus_t[obj->width_];
	    for (unsigned idx = 0 ;  idx < obj->width_ ;  idx += 1) {
		  const Nexus*nex = net->pin(idx).nexus();
		  assert(nex->t_cookie());
		  obj->n.pins_[idx] = (ivl_nexus_t) nex->t_cookie();
		  nexus_con_add(obj->n.pins_[idx], obj, idx);
	    }
      }

      return true;
}

void dll_target::net_probe(const NetEvProbe*net)
{
}

void dll_target::scope(const NetScope*net)
{
      ivl_scope_t scope;

      if (net->parent() == 0) {
	    assert(strcmp(des_.root_->name_, net->name().c_str()) == 0);
	    scope = des_.root_;

      } else {
	    scope = new struct ivl_scope_s;
	    scope->name_ = strdup(net->name().c_str());
	    scope->child_ = 0;
	    scope->sibling_ = 0;
	    scope->nsigs_ = 0;
	    scope->sigs_ = 0;
	    scope->nlog_ = 0;
	    scope->log_ = 0;
	    scope->nlpm_ = 0;
	    scope->lpm_ = 0;

	    switch (net->type()) {
		case NetScope::MODULE:
		  scope->type_ = IVL_SCT_MODULE;
		  scope->tname_ = net->module_name();
		  break;
		case NetScope::TASK:
		  scope->type_ = IVL_SCT_TASK;
		  scope->tname_ = strdup(net->task_def()->name().c_str());
		  break;
		case NetScope::FUNC:
		  scope->type_ = IVL_SCT_FUNCTION;
		  scope->tname_ = strdup(net->func_def()->name().c_str());
		  break;
		case NetScope::BEGIN_END:
		  scope->type_ = IVL_SCT_BEGIN;
		  scope->tname_ = scope->name_;
		  break;
		case NetScope::FORK_JOIN:
		  scope->type_ = IVL_SCT_FORK;
		  scope->tname_ = scope->name_;
		  break;
	    }

	    ivl_scope_t parent = find_scope(des_.root_, net->parent());
	    assert(parent != 0);

	    scope->sibling_= parent->child_;
	    parent->child_ = scope;
      }
}

void dll_target::signal(const NetNet*net)
{
      ivl_signal_t obj = new struct ivl_signal_s;

      obj->name_ = strdup(net->name());

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
      }

      obj->scope_->sigs_[obj->scope_->nsigs_-1] = obj;

#ifndef NDEBUG
      { size_t name_len = strlen(obj->scope_->name_);
	assert(0 == strncmp(obj->scope_->name_, obj->name_, name_len));
      }
#endif

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

      obj->nattr_ = net->nattr();
      obj->akey_  = new char*[obj->nattr_];
      obj->aval_  = new char*[obj->nattr_];
      for (unsigned idx = 0 ;  idx < obj->nattr_ ;  idx += 1) {
	    obj->akey_[idx] = strdup(net->attr_key(idx));
	    obj->aval_[idx] = strdup(net->attr_value(idx));
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
		  nexus_sig_add(obj->n.pin_, obj, 0);

	    } else {
		  ivl_nexus_t tmp = nexus_sig_make(obj, 0);
		  tmp->name_ = strdup(nex->name());
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
			nexus_sig_add(obj->n.pins_[idx], obj, idx);

		  } else {
			ivl_nexus_t tmp = nexus_sig_make(obj, idx);
			tmp->name_ = strdup(nex->name());
			nex->t_cookie(tmp);
			obj->n.pins_[idx] = tmp;
		  }
	    }
      }
}

extern const struct target tgt_dll = { "dll", &dll_target_obj };


/*
 * $Log: t-dll.cc,v $
 * Revision 1.30  2001/03/28 06:07:39  steve
 *  Add the ivl_event_t to ivl_target, and use that to generate
 *  .event statements in vvp way ahead of the thread that uses it.
 *
 * Revision 1.29  2001/03/27 03:31:06  steve
 *  Support error code from target_t::end_design method.
 *
 * Revision 1.28  2001/03/20 01:44:14  steve
 *  Put processes in the proper scope.
 *
 * Revision 1.27  2001/01/15 22:08:32  steve
 *  Add missing NetLogic gate types to ::logic method.
 *
 * Revision 1.26  2001/01/15 00:47:02  steve
 *  Pass scope type information to the target module.
 *
 * Revision 1.25  2001/01/06 06:31:59  steve
 *  declaration initialization for time variables.
 *
 * Revision 1.24  2001/01/06 02:29:36  steve
 *  Support arrays of integers.
 *
 * Revision 1.23  2000/12/15 18:06:47  steve
 *  A dlerror implementatin that HP/UX might like.
 *
 * Revision 1.22  2000/12/15 05:45:25  steve
 *  Autoconfigure the dlopen functions.
 *
 * Revision 1.21  2000/12/14 23:23:07  steve
 *  Support more logic gate types.
 *
 * Revision 1.20  2000/12/05 06:29:33  steve
 *  Make signal attributes available to ivl_target API.
 *
 * Revision 1.19  2000/11/11 00:03:36  steve
 *  Add support for the t-dll backend grabing flip-flops.
 *
 * Revision 1.18  2000/11/09 22:19:34  steve
 *  Initialize scope when creating it.
 *
 * Revision 1.17  2000/10/31 17:49:02  steve
 *  Support time variables.
 *
 * Revision 1.16  2000/10/21 16:49:45  steve
 *  Reduce the target entry points to the target_design.
 *
 * Revision 1.15  2000/10/15 04:46:23  steve
 *  Scopes and processes are accessible randomly from
 *  the design, and signals and logic are accessible
 *  from scopes. Remove the target calls that are no
 *  longer needed.
 *
 *  Add the ivl_nexus_ptr_t and the means to get at
 *  them from nexus objects.
 *
 *  Give names to methods that manipulate the ivl_design_t
 *  type more consistent names.
 *
 * Revision 1.14  2000/10/13 03:39:27  steve
 *  Include constants in nexus targets.
 *
 * Revision 1.13  2000/10/08 04:01:55  steve
 *  Back pointers in the nexus objects into the devices
 *  that point to it.
 *
 *  Collect threads into a list in the design.
 *
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
 */

