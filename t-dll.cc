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
#ident "$Id: t-dll.cc,v 1.51 2001/06/19 03:01:10 steve Exp $"
#endif

# include  "compiler.h"
# include  "t-dll.h"
# include  <malloc.h>

#if defined(__WIN32__)

inline ivl_dll_t ivl_dlopen(const char *name)
{
  return (ivl_dll_t) LoadLibrary(name);
}


inline void * ivl_dlsym(ivl_dll_t dll, const char *nm)
{
  FARPROC sym;
  return GetProcAddress((HMODULE)dll, nm);
}

inline void ivl_dlclose(ivl_dll_t dll)
{
  FreeLibrary((HMODULE)dll);
}

const char *dlerror(void)
{
  static char msg[255];

  FormatMessage( 
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &msg,
		0,
		NULL 
		);
  return msg;
}
#elif defined(HAVE_DLFCN_H)
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

/*
 * This function locates an ivl_memory_t object that matches the
 * NetMemory object. The search works by looking for the parent scope,
 * then scanning the parent scope for the NetMemory object.
 */
static ivl_memory_t find_memory(ivl_scope_t root, const NetMemory*cur)
{
      ivl_scope_t tmp;
      ivl_memory_t mem ;

      if (!root)
	    return 0;

      for (unsigned i = 0; i < ivl_scope_mems(root); i++) {
	    mem = ivl_scope_mem(root, i);
	    if (!strcmp(ivl_memory_name(mem), cur->name().c_str()))
		  return mem;
      }

      mem = find_memory(root->child_, cur);
      if (mem)
	    return mem;
      
      mem = find_memory(root->sibling_, cur);
      if (mem)
	    return mem;
 
      return 0;
}

ivl_memory_t dll_target::lookup_memory_(const NetMemory*cur)
{
      return find_memory(des_.root_, cur);
}

static ivl_nexus_t nexus_sig_make(ivl_signal_t net, unsigned pin)
{
      ivl_nexus_t tmp = new struct ivl_nexus_s;
      tmp->nptr_ = 1;
      tmp->ptrs_ = (struct ivl_nexus_ptr_s*)
	    malloc(sizeof(struct ivl_nexus_ptr_s));
      tmp->ptrs_[0].pin_   = pin;
      tmp->ptrs_[0].type_  = __NEXUS_PTR_SIG;
      tmp->ptrs_[0].l.sig  = net;

      ivl_drive_t drive = IVL_DR_HiZ;
      switch (ivl_signal_type(net)) {
	  case IVL_SIT_REG:
	    drive = IVL_DR_STRONG;
	    break;
	  case IVL_SIT_SUPPLY0:
	  case IVL_SIT_SUPPLY1:
	    drive = IVL_DR_SUPPLY;
	    break;
      }
      tmp->ptrs_[0].drive0 = drive;
      tmp->ptrs_[0].drive1 = drive;

      return tmp;
}

static void nexus_sig_add(ivl_nexus_t nex, ivl_signal_t net, unsigned pin)
{
      unsigned top = nex->nptr_ + 1;
      nex->ptrs_ = (struct ivl_nexus_ptr_s*)
	    realloc(nex->ptrs_, top * sizeof(struct ivl_nexus_ptr_s));
      nex->nptr_ = top;

      ivl_drive_t drive = IVL_DR_HiZ;
      switch (ivl_signal_type(net)) {
	  case IVL_SIT_REG:
	    drive = IVL_DR_STRONG;
	    break;
	  case IVL_SIT_SUPPLY0:
	  case IVL_SIT_SUPPLY1:
	    drive = IVL_DR_SUPPLY;
	    break;
      }

      nex->ptrs_[top-1].type_= __NEXUS_PTR_SIG;
      nex->ptrs_[top-1].drive0 = drive;
      nex->ptrs_[top-1].drive1 = drive;
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
      nex->ptrs_[top-1].drive0 = (pin == 0)? IVL_DR_STRONG : IVL_DR_HiZ;
      nex->ptrs_[top-1].drive1 = (pin == 0)? IVL_DR_STRONG : IVL_DR_HiZ;
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
      nex->ptrs_[top-1].drive0 = IVL_DR_STRONG;
      nex->ptrs_[top-1].drive1 = IVL_DR_STRONG;
      nex->ptrs_[top-1].pin_ = pin;
      nex->ptrs_[top-1].l.con= net;
}

static void nexus_lpm_add(ivl_nexus_t nex, ivl_lpm_t net, unsigned pin,
			  ivl_drive_t drive0, ivl_drive_t drive1)
{
      unsigned top = nex->nptr_ + 1;
      nex->ptrs_ = (struct ivl_nexus_ptr_s*)
	    realloc(nex->ptrs_, top * sizeof(struct ivl_nexus_ptr_s));
      nex->nptr_ = top;

      nex->ptrs_[top-1].type_= __NEXUS_PTR_LPM;
      nex->ptrs_[top-1].drive0 = drive0;
      nex->ptrs_[top-1].drive1 = drive0;
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

static void scope_add_mem(ivl_scope_t scope, ivl_memory_t net)
{
      scope->nmem_ += 1;
      scope->mem_   = (ivl_memory_t*)
	    realloc(scope->mem_, scope->nmem_*sizeof(ivl_memory_t));
      scope->mem_[scope->nmem_-1] = net;
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
      des_.root_->nevent_ = 0;
      des_.root_->event_ = 0;
      des_.root_->nlpm_ = 0;
      des_.root_->lpm_ = 0;
      des_.root_->nmem_ = 0;
      des_.root_->mem_ = 0;
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

      obj->nany = 0;
      obj->nneg = 0;
      obj->npos = 0;

      if (net->nprobe() >= 1) {

	    for (unsigned idx = 0 ;  idx < net->nprobe() ;  idx += 1) {
		  const NetEvProbe*pr = net->probe(idx);
		  switch (pr->edge()) {
		      case NetEvProbe::ANYEDGE:
			obj->nany += pr->pin_count();
			break;
		      case NetEvProbe::NEGEDGE:
			obj->nneg += pr->pin_count();
			break;
		      case NetEvProbe::POSEDGE:
			obj->npos += pr->pin_count();
			break;
		  }
	    }

	    unsigned npins = obj->nany + obj->nneg + obj->npos;
	    obj->pins = (ivl_nexus_t*)calloc(npins, sizeof(ivl_nexus_t));

      } else {
	    obj->pins  = 0;
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
	  case NetLogic::PULLDOWN:
	    obj->type_ = IVL_LO_PULLDOWN;
	    break;
	  case NetLogic::PULLUP:
	    obj->type_ = IVL_LO_PULLUP;
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

void dll_target::net_case_cmp(const NetCaseCmp*net)
{
      struct ivl_net_logic_s *obj = new struct ivl_net_logic_s;

      obj->type_ = IVL_LO_EEQ;

	/* Connect all the ivl_nexus_t objects to the pins of the
	   device. */

      obj->npins_ = 3;
      obj->pins_ = new ivl_nexus_t[obj->npins_];
      for (unsigned idx = 0 ;  idx < obj->npins_ ;  idx += 1) {
	    const Nexus*nex = net->pin(idx).nexus();
	    assert(nex->t_cookie());
	    obj->pins_[idx] = (ivl_nexus_t) nex->t_cookie();
	    nexus_log_add(obj->pins_[idx], obj, idx);
      }

      // assert(net->scope());
      // ivl_scope_t scope = find_scope(des_.root_, net->scope());
      // assert(scope);
      ivl_scope_t scope = des_.root_;

      obj->scope_= scope;
      obj->name_ = strdup(net->name());

      scope_add_logic(scope, obj);
}

void dll_target::udp(const NetUDP*net)
{
      struct ivl_net_logic_s *obj = new struct ivl_net_logic_s;

      obj->type_ = IVL_LO_UDP;

      static map<string,ivl_udp_t> udps;
      ivl_udp_t u;

      if (udps.find(net->udp_name()) != udps.end())
	{
	  u = udps[net->udp_name()];
	}
      else
	{
	  u = new struct ivl_udp_s;
	  u->nrows = net->rows();
	  u->table = (char**)malloc((u->nrows+1)*sizeof(char*));
	  assert(u->table);
	  u->table[u->nrows] = 0x0;
	  u->nin = net->nin();
	  u->sequ = net->is_sequential();
	  if (u->sequ)
	    u->init = net->get_initial();
	  u->name = strdup(net->udp_name().c_str());
	  string inp;
	  char out;
	  int i = 0;
	  if (net->first(inp, out))
	    do
	      {
		string tt = inp+out;
		u->table[i++] = strdup(tt.c_str());
	      } while (net->next(inp, out));
	  assert(i==u->nrows);

	  udps[net->udp_name()] = u;
	}
      
      obj->udp = u;
      
      // Some duplication of code here, see: dll_target::logic()

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

void dll_target::memory(const NetMemory*net)
{
      ivl_memory_t obj = new struct ivl_memory_s;
      obj->name_  = strdup(net->name().c_str());
      obj->scope_ = find_scope(des_.root_, net->scope());
      obj->width_ = net->width();
      obj->signed_ = 0;
      obj->size_ = net->count();
      obj->root_ = -net->index_to_address(0);

      scope_add_mem(obj->scope_, obj);
}

void dll_target::lpm_add_sub(const NetAddSub*net)
{
      ivl_lpm_t obj = new struct ivl_lpm_s;
      if (net->attribute("LPM_Direction") == "SUB")
	    obj->type = IVL_LPM_SUB;
      else
	    obj->type = IVL_LPM_ADD;
      obj->name = strdup(net->name());
      assert(net->scope());
      obj->scope = find_scope(des_.root_, net->scope());
      assert(obj->scope);

	/* Choose the width of the adder. If the carry bit is
	   connected, then widen the adder by one and plan on leaving
	   the fake inputs unconnected. */
      obj->u_.arith.width = net->width();
      if (net->pin_Cout().is_linked()) {
	    obj->u_.arith.width += 1;
      }

      obj->u_.arith.q = new ivl_nexus_t[3 * obj->u_.arith.width];
      obj->u_.arith.a = obj->u_.arith.q + obj->u_.arith.width;
      obj->u_.arith.b = obj->u_.arith.a + obj->u_.arith.width;

      for (unsigned idx = 0 ;  idx < net->width() ;  idx += 1) {
	    const Nexus*nex;

	    nex = net->pin_Result(idx).nexus();
	    assert(nex->t_cookie());

	    obj->u_.arith.q[idx] = (ivl_nexus_t) nex->t_cookie();
	    nexus_lpm_add(obj->u_.arith.q[idx], obj, 0,
			  IVL_DR_STRONG, IVL_DR_STRONG);

	    nex = net->pin_DataA(idx).nexus();
	    assert(nex->t_cookie());

	    obj->u_.arith.a[idx] = (ivl_nexus_t) nex->t_cookie();
	    nexus_lpm_add(obj->u_.arith.a[idx], obj, 0,
			  IVL_DR_HiZ, IVL_DR_HiZ);

	    nex = net->pin_DataB(idx).nexus();
	    assert(nex->t_cookie());

	    obj->u_.arith.b[idx] = (ivl_nexus_t) nex->t_cookie();
	    nexus_lpm_add(obj->u_.arith.b[idx], obj, 0,
			  IVL_DR_HiZ, IVL_DR_HiZ);
      }

	/* If the carry output is connected, then connect the extra Q
	   pin to the carry nexus and zero the a and b inputs. */
      if (net->pin_Cout().is_linked()) {
	    unsigned carry = obj->u_.arith.width - 1;
	    const Nexus*nex = net->pin_Cout().nexus();
	    assert(nex->t_cookie());

	    obj->u_.arith.q[carry] = (ivl_nexus_t) nex->t_cookie();
	    nexus_lpm_add(obj->u_.arith.q[carry], obj, 0,
			  IVL_DR_STRONG, IVL_DR_STRONG);

	    obj->u_.arith.a[carry] = 0;
	    obj->u_.arith.b[carry] = 0;
      }

      scope_add_lpm(obj->scope, obj);
}

/*
 * Make out of the NetCompare object an ivl_lpm_s object. The
 * comparators in ivl_target do not support < or <=, but they can be
 * trivially converted to > and >= by swapping the operands.
 */
void dll_target::lpm_compare(const NetCompare*net)
{
      ivl_lpm_t obj = new struct ivl_lpm_s;
      obj->name = strdup(net->name());
      assert(net->scope());
      obj->scope = find_scope(des_.root_, net->scope());
      assert(obj->scope);

      bool swap_operands = false;

      obj->u_.arith.width = net->width();

      obj->u_.arith.q = new ivl_nexus_t[1 + 2 * obj->u_.arith.width];
      obj->u_.arith.a = obj->u_.arith.q + 1;
      obj->u_.arith.b = obj->u_.arith.a + obj->u_.arith.width;

      if (net->pin_AGEB().is_linked()) {
	    const Nexus*nex = net->pin_AGEB().nexus();
	    obj->type = IVL_LPM_CMP_GE;

	    assert(nex->t_cookie());
	    obj->u_.arith.q[0] = (ivl_nexus_t) nex->t_cookie();
	    nexus_lpm_add(obj->u_.arith.q[0], obj, 0,
			  IVL_DR_STRONG, IVL_DR_STRONG);

      } else if (net->pin_AGB().is_linked()) {
	    const Nexus*nex = net->pin_AGB().nexus();
	    obj->type = IVL_LPM_CMP_GT;

	    assert(nex->t_cookie());
	    obj->u_.arith.q[0] = (ivl_nexus_t) nex->t_cookie();
	    nexus_lpm_add(obj->u_.arith.q[0], obj, 0,
			  IVL_DR_STRONG, IVL_DR_STRONG);

      } else if (net->pin_ALEB().is_linked()) {
	    const Nexus*nex = net->pin_ALEB().nexus();
	    obj->type = IVL_LPM_CMP_GE;

	    assert(nex->t_cookie());
	    obj->u_.arith.q[0] = (ivl_nexus_t) nex->t_cookie();
	    nexus_lpm_add(obj->u_.arith.q[0], obj, 0,
			  IVL_DR_STRONG, IVL_DR_STRONG);

	    swap_operands = true;

      } else if (net->pin_ALB().is_linked()) {
	    const Nexus*nex = net->pin_ALB().nexus();
	    obj->type = IVL_LPM_CMP_GT;

	    assert(nex->t_cookie());
	    obj->u_.arith.q[0] = (ivl_nexus_t) nex->t_cookie();
	    nexus_lpm_add(obj->u_.arith.q[0], obj, 0,
			  IVL_DR_STRONG, IVL_DR_STRONG);

	    swap_operands = true;

      } else {
	    assert(0);
      }

      for (unsigned idx = 0 ;  idx < net->width() ;  idx += 1) {
	    const Nexus*nex;

	    nex = swap_operands
		  ? net->pin_DataB(idx).nexus()
		  : net->pin_DataA(idx).nexus();

	    assert(nex->t_cookie());

	    obj->u_.arith.a[idx] = (ivl_nexus_t) nex->t_cookie();
	    nexus_lpm_add(obj->u_.arith.a[idx], obj, 0,
			  IVL_DR_HiZ, IVL_DR_HiZ);

	    nex = swap_operands
		  ? net->pin_DataA(idx).nexus()
		  : net->pin_DataB(idx).nexus();

	    assert(nex->t_cookie());

	    obj->u_.arith.b[idx] = (ivl_nexus_t) nex->t_cookie();
	    nexus_lpm_add(obj->u_.arith.b[idx], obj, 0,
			  IVL_DR_HiZ, IVL_DR_HiZ);
      }


      scope_add_lpm(obj->scope, obj);
}

void dll_target::lpm_ff(const NetFF*net)
{
      ivl_lpm_t obj = new struct ivl_lpm_s;
      obj->type  = IVL_LPM_FF;
      obj->name  = strdup(net->name());
      obj->scope = find_scope(des_.root_, net->scope());
      assert(obj->scope);

      obj->u_.ff.width = net->width();

      scope_add_lpm(obj->scope, obj);

      const Nexus*nex;

	/* Set the clk signal to point to the nexus, and the nexus to
	   point back to this device. */
      nex = net->pin_Clock().nexus();
      assert(nex->t_cookie());
      obj->u_.ff.clk = (ivl_nexus_t) nex->t_cookie();
      assert(obj->u_.ff.clk);
      nexus_lpm_add(obj->u_.ff.clk, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);

      if (obj->u_.ff.width == 1) {
	    nex = net->pin_Q(0).nexus();
	    assert(nex->t_cookie());
	    obj->u_.ff.q.pin = (ivl_nexus_t) nex->t_cookie();
	    nexus_lpm_add(obj->u_.ff.q.pin, obj, 0,
			  IVL_DR_STRONG, IVL_DR_STRONG);

	    nex = net->pin_Data(0).nexus();
	    assert(nex->t_cookie());
	    obj->u_.ff.d.pin = (ivl_nexus_t) nex->t_cookie();
	    nexus_lpm_add(obj->u_.ff.d.pin, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);

      } else {
	    obj->u_.ff.q.pins = new ivl_nexus_t [obj->u_.ff.width * 2];
	    obj->u_.ff.d.pins = obj->u_.ff.q.pins + obj->u_.ff.width;

	    for (unsigned idx = 0 ;  idx < obj->u_.ff.width ;  idx += 1) {
		  nex = net->pin_Q(idx).nexus();
		  assert(nex->t_cookie());
		  obj->u_.ff.q.pins[idx] = (ivl_nexus_t) nex->t_cookie();
		  nexus_lpm_add(obj->u_.ff.q.pins[idx], obj, 0,
			  IVL_DR_STRONG, IVL_DR_STRONG);

		  nex = net->pin_Data(idx).nexus();
		  assert(nex->t_cookie());
		  obj->u_.ff.d.pins[idx] = (ivl_nexus_t) nex->t_cookie();
		  nexus_lpm_add(obj->u_.ff.d.pins[idx], obj, 0,
				IVL_DR_HiZ, IVL_DR_HiZ);
	    }
      }
}

void dll_target::lpm_ram_dq(const NetRamDq*net)
{
      ivl_lpm_t obj = new struct ivl_lpm_s;
      obj->type  = IVL_LPM_RAM;
      obj->name  = strdup(net->name());
      obj->u_.ff.mem = lookup_memory_(net->mem());
      assert(obj->u_.ff.mem);
      obj->scope = find_scope(des_.root_, net->mem()->scope());
      assert(obj->scope);

      obj->u_.ff.width = net->width();
      obj->u_.ff.swid = net->awidth();
      
      scope_add_lpm(obj->scope, obj);

      const Nexus*nex;

      // A write port is present only if something is connected to 
      // the clock input.

      bool has_write_port = net->pin_InClock().is_linked();

      // Connect the write clock and write enable

      if (has_write_port) {
	    nex = net->pin_InClock().nexus();
	    assert(nex->t_cookie());
	    obj->u_.ff.clk = (ivl_nexus_t) nex->t_cookie();
	    assert(obj->u_.ff.clk);
	    nexus_lpm_add(obj->u_.ff.clk, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);

	    nex = net->pin_WE().nexus();
	    if (nex && nex->t_cookie()) {
		  obj->u_.ff.we = (ivl_nexus_t) nex->t_cookie();
		  assert(obj->u_.ff.we);
		  nexus_lpm_add(obj->u_.ff.we, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);
	    }
	    else
		  obj->u_.ff.we = 0x0;
      }
      else {
	    obj->u_.ff.clk = 0x0;
	    obj->u_.ff.we = 0x0;
      }
	    
      // Connect the address bus

      if (obj->u_.ff.swid == 1) {
	    nex = net->pin_Address(0).nexus();
	    assert(nex->t_cookie());
	    obj->u_.ff.s.pin = (ivl_nexus_t) nex->t_cookie();
	    nexus_lpm_add(obj->u_.ff.s.pin, obj, 0,
			  IVL_DR_HiZ, IVL_DR_HiZ);
      }
      else {
	    obj->u_.ff.s.pins = new ivl_nexus_t [obj->u_.ff.swid];
	    
	    for (unsigned idx = 0 ;  idx < obj->u_.ff.swid ;  idx += 1) {
		  nex = net->pin_Address(idx).nexus();
		  assert(nex->t_cookie());
		  obj->u_.ff.s.pins[idx] = (ivl_nexus_t) nex->t_cookie();
		  nexus_lpm_add(obj->u_.ff.s.pins[idx], obj, 0,
				IVL_DR_HiZ, IVL_DR_HiZ);
	    }
      }
      
      // Connect the data busses

      if (obj->u_.ff.width == 1) {
	    nex = net->pin_Q(0).nexus();
	    assert(nex->t_cookie());
	    obj->u_.ff.q.pin = (ivl_nexus_t) nex->t_cookie();
	    nexus_lpm_add(obj->u_.ff.q.pin, obj, 0,
			  IVL_DR_STRONG, IVL_DR_STRONG);

	    if (has_write_port) {
		  nex = net->pin_Data(0).nexus();
		  assert(nex->t_cookie());
		  obj->u_.ff.d.pin = (ivl_nexus_t) nex->t_cookie();
		  nexus_lpm_add(obj->u_.ff.d.pin, obj,
				0, IVL_DR_HiZ, IVL_DR_HiZ);
	    }
      } 
      else if (has_write_port) {
	    obj->u_.ff.q.pins = new ivl_nexus_t [obj->u_.ff.width * 2];
	    obj->u_.ff.d.pins = obj->u_.ff.q.pins + obj->u_.ff.width;

	    for (unsigned idx = 0 ;  idx < obj->u_.ff.width ;  idx += 1) {
		  nex = net->pin_Q(idx).nexus();
		  assert(nex->t_cookie());
		  obj->u_.ff.q.pins[idx] = (ivl_nexus_t) nex->t_cookie();
		  nexus_lpm_add(obj->u_.ff.q.pins[idx], obj, 0,
				IVL_DR_STRONG, IVL_DR_STRONG);
		  
		  nex = net->pin_Data(idx).nexus();
		  assert(nex->t_cookie());
		  obj->u_.ff.d.pins[idx] = (ivl_nexus_t) nex->t_cookie();
		  nexus_lpm_add(obj->u_.ff.d.pins[idx], obj, 0,
				IVL_DR_HiZ, IVL_DR_HiZ);
	    }
      } 
      else {
	    obj->u_.ff.q.pins = new ivl_nexus_t [obj->u_.ff.width];
	    
	    for (unsigned idx = 0 ;  idx < obj->u_.ff.width ;  idx += 1) {
		  nex = net->pin_Q(idx).nexus();
		  assert(nex->t_cookie());
		  obj->u_.ff.q.pins[idx] = (ivl_nexus_t) nex->t_cookie();
		  nexus_lpm_add(obj->u_.ff.q.pins[idx], obj, 0,
				IVL_DR_STRONG, IVL_DR_STRONG);
	    }
      }
}

void dll_target::lpm_mult(const NetMult*net)
{
      ivl_lpm_t obj = new struct ivl_lpm_s;
      obj->type  = IVL_LPM_MULT;
      obj->name  = strdup(net->name());
      assert(net->scope());
      obj->scope = find_scope(des_.root_, net->scope());
      assert(obj->scope);

      unsigned wid = net->width_r();

      obj->u_.arith.width = wid;

      obj->u_.arith.q = new ivl_nexus_t[3 * obj->u_.arith.width];
      obj->u_.arith.a = obj->u_.arith.q + obj->u_.arith.width;
      obj->u_.arith.b = obj->u_.arith.a + obj->u_.arith.width;

      for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
	    const Nexus*nex;

	    nex = net->pin_Result(idx).nexus();
	    assert(nex->t_cookie());

	    obj->u_.arith.q[idx] = (ivl_nexus_t) nex->t_cookie();
	    nexus_lpm_add(obj->u_.arith.q[idx], obj, 0,
			  IVL_DR_STRONG, IVL_DR_STRONG);

	    if (idx < net->width_a()) {
		  nex = net->pin_DataA(idx).nexus();
		  assert(nex);
		  assert(nex->t_cookie());

		  obj->u_.arith.a[idx] = (ivl_nexus_t) nex->t_cookie();
		  nexus_lpm_add(obj->u_.arith.a[idx], obj, 0,
				IVL_DR_HiZ, IVL_DR_HiZ);

	    } else {
		  obj->u_.arith.a[idx] = 0;
	    }


	    if (idx < net->width_b()) {
		  nex = net->pin_DataB(idx).nexus();
		  assert(nex);
		  assert(nex->t_cookie());

		  obj->u_.arith.b[idx] = (ivl_nexus_t) nex->t_cookie();
		  nexus_lpm_add(obj->u_.arith.b[idx], obj, 0,
				IVL_DR_HiZ, IVL_DR_HiZ);

	    } else {
		  obj->u_.arith.b[idx] = 0;
	    }
      }

      scope_add_lpm(obj->scope, obj);
}

void dll_target::lpm_mux(const NetMux*net)
{
      ivl_lpm_t obj = new struct ivl_lpm_s;
      obj->type  = IVL_LPM_MUX;
      obj->name  = strdup(net->name());
      obj->scope = find_scope(des_.root_, net->scope());
      assert(obj->scope);

      obj->u_.mux.width = net->width();
      obj->u_.mux.size  = net->size();
      obj->u_.mux.swid  = net->sel_width();

      scope_add_lpm(obj->scope, obj);

      const Nexus*nex;

	/* Connect the output bits. */
      if (obj->u_.mux.width == 1) {
	    nex = net->pin_Result(0).nexus();
	    assert(nex->t_cookie());
	    obj->u_.mux.q.pin = (ivl_nexus_t) nex->t_cookie();
	    nexus_lpm_add(obj->u_.mux.q.pin, obj, 0,
			  IVL_DR_STRONG, IVL_DR_STRONG);

      } else {
	    obj->u_.mux.q.pins = new ivl_nexus_t [obj->u_.mux.width];

	    for (unsigned idx = 0 ;  idx < obj->u_.mux.width ;  idx += 1) {
		  nex = net->pin_Result(idx).nexus();
		  assert(nex->t_cookie());
		  obj->u_.mux.q.pins[idx] = (ivl_nexus_t) nex->t_cookie();
		  nexus_lpm_add(obj->u_.mux.q.pins[idx], obj, 0,
				IVL_DR_STRONG, IVL_DR_STRONG);
	    }
      }

	/* Connect the select bits. */
      if (obj->u_.mux.swid == 1) {
	    nex = net->pin_Sel(0).nexus();
	    assert(nex->t_cookie());
	    obj->u_.mux.s.pin = (ivl_nexus_t) nex->t_cookie();
	    nexus_lpm_add(obj->u_.mux.s.pin, obj, 0,
			  IVL_DR_HiZ, IVL_DR_HiZ);

      } else {
	    obj->u_.mux.s.pins = new ivl_nexus_t [obj->u_.mux.swid];

	    for (unsigned idx = 0 ;  idx < obj->u_.mux.swid ;  idx += 1) {
		  nex = net->pin_Sel(idx).nexus();
		  assert(nex->t_cookie());
		  obj->u_.mux.s.pins[idx] = (ivl_nexus_t) nex->t_cookie();
		  nexus_lpm_add(obj->u_.mux.s.pins[idx], obj, 0,
				IVL_DR_HiZ, IVL_DR_HiZ);
	    }
      }

      unsigned width = obj->u_.mux.width;
      unsigned selects = obj->u_.mux.size;

      obj->u_.mux.d = new ivl_nexus_t [width * selects];

      for (unsigned sdx = 0 ;  sdx < selects ;  sdx += 1)
	    for (unsigned ddx = 0 ;  ddx < width ;  ddx += 1) {
		  nex = net->pin_Data(ddx, sdx).nexus();
		  ivl_nexus_t tmp = (ivl_nexus_t) nex->t_cookie();
		  obj->u_.mux.d[sdx*width + ddx] = tmp;
		  nexus_lpm_add(tmp, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);
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
	    scope->nevent_ = 0;
	    scope->event_ = 0;
	    scope->nlpm_ = 0;
	    scope->lpm_ = 0;
	    scope->nmem_ = 0;
	    scope->mem_ = 0;

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
      obj->signed_= net->get_signed()? 1 : 0;

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
 * Revision 1.51  2001/06/19 03:01:10  steve
 *  Add structural EEQ gates (Stephan Boettcher)
 *
 * Revision 1.50  2001/06/18 03:25:20  steve
 *  RAM_DQ pins are inputs, so connect HiZ to the nexus.
 *
 * Revision 1.49  2001/06/16 23:45:05  steve
 *  Add support for structural multiply in t-dll.
 *  Add code generators and vvp support for both
 *  structural and behavioral multiply.
 *
 * Revision 1.48  2001/06/16 02:41:42  steve
 *  Generate code to support memory access in continuous
 *  assignment statements. (Stephan Boettcher)
 *
 * Revision 1.47  2001/06/15 05:01:09  steve
 *  support LE and LT comparators.
 *
 * Revision 1.46  2001/06/15 04:14:19  steve
 *  Generate vvp code for GT and GE comparisons.
 *
 * Revision 1.45  2001/06/07 04:20:10  steve
 *  Account for carry out on add devices.
 *
 * Revision 1.44  2001/06/07 03:09:37  steve
 *  support subtraction in tgt-vvp.
 *
 * Revision 1.43  2001/06/07 02:12:43  steve
 *  Support structural addition.
 *
 * Revision 1.42  2001/05/20 15:09:39  steve
 *  Mingw32 support (Venkat Iyer)
 *
 * Revision 1.41  2001/05/12 03:18:45  steve
 *  Make sure LPM devices have drives on outputs.
 *
 * Revision 1.40  2001/05/08 23:59:33  steve
 *  Add ivl and vvp.tgt support for memories in
 *  expressions and l-values. (Stephan Boettcher)
 *
 * Revision 1.39  2001/05/03 01:52:45  steve
 *  dll build of many probes forgot to index the probe.
 *
 * Revision 1.38  2001/04/29 23:17:38  steve
 *  Carry drive strengths in the ivl_nexus_ptr_t, and
 *  handle constant devices in targets.'
 *
 * Revision 1.37  2001/04/29 20:19:10  steve
 *  Add pullup and pulldown devices.
 *
 * Revision 1.36  2001/04/26 05:12:02  steve
 *  Implement simple MUXZ for ?: operators.
 */

