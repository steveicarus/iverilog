/*
 * Copyright (c) 2000-2022 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2013 / Stephen Williams (steve@icarus.com)
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

# include  <iostream>

# include  <cstring>
# include  <cstdio> // sprintf()
# include  "compiler.h"
# include  "t-dll.h"
# include  "netclass.h"
# include  "netqueue.h"
# include  "netmisc.h"
# include  "discipline.h"
# include  <cstdlib>
# include  "ivl_assert.h"
# include  "ivl_alloc.h"

using namespace std;

struct dll_target dll_target_obj;

#if defined(__WIN32__)

inline ivl_dll_t ivl_dlopen(const char *name)
{
      ivl_dll_t res =  (ivl_dll_t) LoadLibrary(name);
      return res;
}


inline void * ivl_dlsym(ivl_dll_t dll, const char *nm)
{
      return (void*)GetProcAddress((HMODULE)dll, nm);
}

inline void ivl_dlclose(ivl_dll_t dll)
{
      FreeLibrary((HMODULE)dll);
}

const char *dlerror(void)
{
  static char msg[256];
  unsigned long err = GetLastError();
  FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &msg,
		sizeof(msg) - 1,
		NULL
		);
  return msg;
}
#elif defined(HAVE_DLFCN_H)
inline ivl_dll_t ivl_dlopen(const char*name)
{ return dlopen(name,RTLD_LAZY); }

inline void* ivl_dlsym(ivl_dll_t dll, const char*nm)
{
      void*sym = dlsym(dll, nm);
	/* Not found? try without the leading _ */
      if (sym == 0 && nm[0] == '_')
	    sym = dlsym(dll, nm+1);
      return sym;
}

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

ivl_scope_s::ivl_scope_s()
: func_type(IVL_VT_NO_TYPE)
{
      func_signed = false;
      func_width = 0;
}

/*
 * The custom new operator for the ivl_nexus_s type allows us to
 * allocate nexus objects in blocks. There are generally lots of them
 * permanently allocated, and allocating them in blocks reduces the
 * allocation overhead.
 */

template <class TYPE> void* pool_permalloc(size_t s)
{
      static TYPE * pool_ptr = 0;
      static int pool_remaining = 0;
      static const size_t POOL_SIZE = 4096;

      assert(s == sizeof(TYPE));
      if (pool_remaining <= 0) {
	    pool_ptr = new TYPE[POOL_SIZE];
	    pool_remaining = POOL_SIZE;
      }

      TYPE*tmp = pool_ptr;
      pool_ptr += 1;
      pool_remaining -= 1;

      return tmp;
}

void* ivl_nexus_s::operator new(size_t s)
{
      return pool_permalloc<struct ivl_nexus_s>(s);
}

void ivl_nexus_s::operator delete(void*, size_t)
{
      assert(0);
}

void* ivl_net_const_s::operator new(size_t s)
{
      return pool_permalloc<struct ivl_net_const_s>(s);
}

void ivl_net_const_s::operator delete(void*, size_t)
{
      assert(0);
}

static StringHeapLex net_const_strings;

static perm_string make_scope_name(const hname_t&name)
{
      if (! name.has_numbers())
	    return name.peek_name();

      char buf[1024];
      snprintf(buf, sizeof buf, "%s", name.peek_name().str());

      char*cp = buf + strlen(buf);
      size_t ncp = sizeof buf - (cp-buf);

      for (size_t idx = 0 ; idx < name.has_numbers() ; idx += 1) {
	    int len = snprintf(cp, ncp, "[%d]", name.peek_number(idx));
	    cp += len;
	    ncp -= len;
      }

      return lex_strings.make(buf);
}

static void drive_from_link(const Link&lnk, ivl_drive_t&drv0, ivl_drive_t&drv1)
{
      drv0 = lnk.drive0();
      drv1 = lnk.drive1();
}

ivl_attribute_s* dll_target::fill_in_attributes(const Attrib*net)
{
      ivl_attribute_s*attr;
      unsigned nattr = net->attr_cnt();

      if (nattr == 0)
	    return 0;

      attr = new struct ivl_attribute_s[nattr];

      for (unsigned idx = 0 ;  idx < nattr ;  idx += 1) {
	    verinum tmp = net->attr_value(idx);
	    attr[idx].key = net->attr_key(idx);
	    if (tmp.is_string()) {
		  attr[idx].type = IVL_ATT_STR;
		  attr[idx].val.str = strings_.add(tmp.as_string().c_str());

	    } else if (tmp == verinum()) {
		  attr[idx].type = IVL_ATT_VOID;

	    } else {
		  attr[idx].type = IVL_ATT_NUM;
		  attr[idx].val.num = tmp.as_long();
	    }
      }

      return attr;
}

/*
 * This function locates an ivl_scope_t object that matches the
 * NetScope object. The search works by looking for the parent scope,
 * then scanning the parent scope for the NetScope object.
 */
static ivl_scope_t find_scope_from_root(ivl_scope_t root, const NetScope*cur)
{
      if (const NetScope*par = cur->parent()) {
	    ivl_scope_t parent = find_scope_from_root(root, par);
	    if (parent == 0) {
		  return 0;
	    }

	    map<hname_t,ivl_scope_t>::iterator idx = parent->children.find(cur->fullname());
	    if (idx == parent->children.end())
		  return 0;
	    else
		  return idx->second;

      } else {
	    perm_string cur_name = make_scope_name(cur->fullname());
	    if (strcmp(root->name_, cur_name) == 0)
		  return root;
      }

      return 0;
}

ivl_scope_t dll_target::find_scope(ivl_design_s &des, const NetScope*cur)
{
      assert(cur);

	// If the scope is a PACKAGE, then it is a special kind of
	// root scope and it in the packages array instead.
      if (cur->type() == NetScope::PACKAGE) {
	    perm_string cur_name = cur->module_name();
	    for (size_t idx = 0 ; idx < des.packages.size() ; idx += 1) {
		  if (des.packages[idx]->name_ == cur_name)
			return des.packages[idx];
	    }
	    return 0;
      }

      for (unsigned idx = 0; idx < des.roots.size(); idx += 1) {
	    assert(des.roots[idx]);
	    ivl_scope_t scop = find_scope_from_root(des.roots[idx], cur);
	    if (scop)
		  return scop;
      }

      for (size_t idx = 0; idx < des.packages.size(); idx += 1) {
	    assert(des.packages[idx]);
	    ivl_scope_t scop = find_scope_from_root(des.packages[idx], cur);
	    if (scop)
		  return scop;
      }

      return 0;
}

ivl_scope_t dll_target::lookup_scope_(const NetScope*cur)
{
      return find_scope(des_, cur);
}

/*
 * This is a convenience function to locate an ivl_signal_t object
 * given the NetESignal that has the signal name.
 */
ivl_signal_t dll_target::find_signal(ivl_design_s &des, const NetNet*net)
{
      ivl_scope_t scop = find_scope(des, net->scope());
      assert(scop);

      perm_string nname = net->name();

      for (unsigned idx = 0 ;  idx < scop->sigs_.size() ;  idx += 1) {
	    if (strcmp(scop->sigs_[idx]->name_, nname) == 0)
		  return scop->sigs_[idx];
      }

      assert(0);
      return 0;
}

static ivl_nexus_t nexus_sig_make(ivl_signal_t net, unsigned pin)
{
      ivl_nexus_t tmp = new struct ivl_nexus_s;
      tmp->ptrs_.resize(1);
      tmp->ptrs_[0].pin_   = pin;
      tmp->ptrs_[0].type_  = __NEXUS_PTR_SIG;
      tmp->ptrs_[0].l.sig  = net;

      ivl_drive_t drive = IVL_DR_HiZ;
      switch (ivl_signal_type(net)) {
	  case IVL_SIT_REG:
	    drive = IVL_DR_STRONG;
	    break;
	  default:
	    break;
      }
      tmp->ptrs_[0].drive0 = drive;
      tmp->ptrs_[0].drive1 = drive;

      return tmp;
}

static void nexus_sig_add(ivl_nexus_t nex, ivl_signal_t net, unsigned pin)
{
      unsigned top = nex->ptrs_.size();
      nex->ptrs_.resize(top+1);
      ivl_drive_t drive = IVL_DR_HiZ;
      switch (ivl_signal_type(net)) {
	  case IVL_SIT_REG:
	    drive = IVL_DR_STRONG;
	    break;
	  default:
	    break;
      }

      nex->ptrs_[top].type_= __NEXUS_PTR_SIG;
      nex->ptrs_[top].drive0 = drive;
      nex->ptrs_[top].drive1 = drive;
      nex->ptrs_[top].pin_ = pin;
      nex->ptrs_[top].l.sig= net;
}

static void nexus_bra_add(ivl_nexus_t nex, ivl_branch_t net, unsigned pin)
{
      unsigned top = nex->ptrs_.size();
      nex->ptrs_.resize(top+1);
      nex->ptrs_[top].type_= __NEXUS_PTR_BRA;
      nex->ptrs_[top].drive0 = 0;
      nex->ptrs_[top].drive1 = 0;
      nex->ptrs_[top].pin_ = pin;
      nex->ptrs_[top].l.bra= net;
}

/*
 * Add the pin of the logic object to the nexus, and return the nexus
 * pointer used for the pin.
 *
 * NOTE: This pointer is only valid until another pin is added to the
 * nexus.
 */
static ivl_nexus_ptr_t nexus_log_add(ivl_nexus_t nex,
				     ivl_net_logic_t net,
				     unsigned pin)
{
      unsigned top = nex->ptrs_.size();
      nex->ptrs_.resize(top+1);

      nex->ptrs_[top].type_= __NEXUS_PTR_LOG;
      nex->ptrs_[top].drive0 = (pin == 0)? IVL_DR_STRONG : IVL_DR_HiZ;
      nex->ptrs_[top].drive1 = (pin == 0)? IVL_DR_STRONG : IVL_DR_HiZ;
      nex->ptrs_[top].pin_ = pin;
      nex->ptrs_[top].l.log= net;

      return & (nex->ptrs_[top]);
}

static void nexus_con_add(ivl_nexus_t nex, ivl_net_const_t net, unsigned pin,
			  ivl_drive_t drive0, ivl_drive_t drive1)
{
      unsigned top = nex->ptrs_.size();
      nex->ptrs_.resize(top+1);

      nex->ptrs_[top].type_= __NEXUS_PTR_CON;
      nex->ptrs_[top].drive0 = drive0;
      nex->ptrs_[top].drive1 = drive1;
      nex->ptrs_[top].pin_ = pin;
      nex->ptrs_[top].l.con= net;
}

static void nexus_lpm_add(ivl_nexus_t nex, ivl_lpm_t net, unsigned pin,
			  ivl_drive_t drive0, ivl_drive_t drive1)
{
      unsigned top = nex->ptrs_.size();
      nex->ptrs_.resize(top+1);

      nex->ptrs_[top].type_= __NEXUS_PTR_LPM;
      nex->ptrs_[top].drive0 = drive0;
      nex->ptrs_[top].drive1 = drive1;
      nex->ptrs_[top].pin_ = pin;
      nex->ptrs_[top].l.lpm= net;
}

static void nexus_switch_add(ivl_nexus_t nex, ivl_switch_t net, unsigned pin)
{
      unsigned top = nex->ptrs_.size();
      nex->ptrs_.resize(top+1);

      nex->ptrs_[top].type_= __NEXUS_PTR_SWI;
      nex->ptrs_[top].drive0 = IVL_DR_HiZ;
      nex->ptrs_[top].drive1 = IVL_DR_HiZ;
      nex->ptrs_[top].pin_ = pin;
      nex->ptrs_[top].l.swi= net;
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

static void scope_add_switch(ivl_scope_t scope, ivl_switch_t net)
{
      scope->switches.push_back(net);
}

ivl_parameter_t dll_target::scope_find_param(ivl_scope_t scope,
					     const char*name)
{
      unsigned idx = 0;
      while (idx < scope->param.size()) {
	    if (strcmp(name, scope->param[idx].basename) == 0)
		  return &scope->param[idx];

	    idx += 1;
      }

      return 0;
}

/*
 * This method scans the parameters of the scope, and makes
 * ivl_parameter_t objects. This involves saving the name and scanning
 * the expression value.
 */
void dll_target::make_scope_parameters(ivl_scope_t scop, const NetScope*net)
{
      if (net->parameters.empty()) {
	    scop->param.clear();
	    return;
      }

      scop->param.resize(net->parameters.size());

      unsigned idx = 0;
      typedef map<perm_string,NetScope::param_expr_t>::const_iterator pit_t;

      for (pit_t cur_pit = net->parameters.begin()
		 ; cur_pit != net->parameters.end() ; ++ cur_pit ) {

	    assert(idx < scop->param.size());
	    ivl_parameter_t cur_par = &scop->param[idx];
	    cur_par->basename = cur_pit->first;
	    cur_par->local = cur_pit->second.local_flag ||
			     !cur_pit->second.overridable;
	    cur_par->is_type = cur_pit->second.type_flag;

	    if (cur_pit->second.ivl_type == 0) {
		  cerr << "?:?: internal error: "
		       << "No type for parameter " << cur_pit->first
		       << " in scope " << net->fullname() << "?" << endl;
	    }
	    assert(cur_pit->second.ivl_type);

	    cur_par->signed_flag = cur_pit->second.ivl_type->get_signed();
	    cur_par->scope = scop;
	    FILE_NAME(cur_par, &(cur_pit->second));

	      // Type parameters don't have a range or expression
	    if (!cur_pit->second.type_flag) {
		  calculate_param_range(cur_pit->second,
					cur_pit->second.ivl_type,
					cur_par->msb, cur_par->lsb,
					cur_pit->second.val->expr_width());

		  NetExpr*etmp = cur_pit->second.val;
		  if (etmp == 0) {
			cerr << "?:?: internal error: What is the parameter "
			     << "expression for " << cur_pit->first
			     << " in " << net->fullname() << "?" << endl;
		  }
		  assert(etmp);
		  make_scope_param_expr(cur_par, etmp);
	    }

	    idx += 1;
      }
}

void dll_target::make_scope_param_expr(ivl_parameter_t cur_par, NetExpr*etmp)
{
      if (const NetEConst*e = dynamic_cast<const NetEConst*>(etmp)) {

	    expr_const(e);
	    assert(expr_);

	    switch (expr_->type_) {
		case IVL_EX_STRING:
		  expr_->u_.string_.parameter = cur_par;
		  break;
		case IVL_EX_NUMBER:
		  expr_->u_.number_.parameter = cur_par;
		  break;
		default:
		  assert(0);
	    }

      } else if (const NetECReal*er = dynamic_cast<const NetECReal*>(etmp)) {

	    expr_creal(er);
	    assert(expr_);
	    assert(expr_->type_ == IVL_EX_REALNUM);
	    expr_->u_.real_.parameter = cur_par;

      }

      if (expr_ == 0) {
	    cerr << etmp->get_fileline() << ": internal error: "
		 << "Parameter expression not reduced to constant? "
		 << *etmp << endl;
      }
      ivl_assert(*etmp, expr_);

      cur_par->value = expr_;
      expr_ = 0;
}

static void fill_in_scope_function(ivl_scope_t scope, const NetScope*net)
{
      scope->type_ = IVL_SCT_FUNCTION;
      const NetFuncDef*def = net->func_def();
      assert(def);

      if (def->is_void()) {
	      // Special case: If there is no return signal, this is
	      // apparently a VOID function.
	    scope->func_type = IVL_VT_VOID;
	    scope->func_signed = 0;
	    scope->func_width = 0;
      } else {
	    const NetNet*return_sig = def->return_sig();
	    scope->func_type = return_sig->data_type();
	    scope->func_signed = return_sig->get_signed();
	    scope->func_width = return_sig->vector_width();
      }

      scope->tname_ = def->scope()->basename();
}

void dll_target::add_root(const NetScope *s)
{
      ivl_scope_t root_ = new struct ivl_scope_s;
      perm_string name = s->basename();
      root_->name_ = name;
      FILE_NAME(root_, s);
      root_->parent = 0;
      root_->nlog_ = 0;
      root_->log_ = 0;
      root_->nevent_ = 0;
      root_->event_ = 0;
      root_->nlpm_ = 0;
      root_->lpm_ = 0;
      root_->def = 0;
      make_scope_parameters(root_, s);
      root_->tname_ = root_->name_;
      root_->time_precision = s->time_precision();
      root_->time_units = s->time_unit();
      root_->nattr = s->attr_cnt();
      root_->attr  = fill_in_attributes(s);
      root_->is_auto = 0;
      root_->is_cell = s->is_cell();
      switch (s->type()) {
	  case NetScope::PACKAGE:
	    root_->type_ = IVL_SCT_PACKAGE;
	    break;
	  case NetScope::MODULE:
	    root_->type_ = IVL_SCT_MODULE;
	    break;
	  case NetScope::CLASS:
	    root_->type_ = IVL_SCT_CLASS;
	    break;
	  default:
	    assert(0);
      }

      switch (s->type()) {
	  case NetScope::MODULE:
	    root_->ports = s->module_port_nets();
	    if (root_->ports > 0) {
		  root_->u_.net = new NetNet*[root_->ports];
		  for (unsigned idx = 0; idx < root_->ports; idx += 1) {
			root_->u_.net[idx] = s->module_port_net(idx);
		  }
	    }
	    root_->module_ports_info = s->module_port_info();

	    des_.roots.push_back(root_);
	    break;

	  case NetScope::PACKAGE:
	    root_->ports = 0;
	    des_.packages.push_back(root_);
	    break;

	  default:
	    assert(0);
	    break;
      }
}

bool dll_target::start_design(const Design*des)
{
      const char*dll_path_ = des->get_flag("DLL");

      dll_ = ivl_dlopen(dll_path_);

      if ((dll_ == 0) && (dll_path_[0] != '/')) {
	    size_t len = strlen(basedir) + 1 + strlen(dll_path_) + 1;
	    char*tmp = new char[len];
	    snprintf(tmp, len, "%s/%s", basedir, dll_path_);
	    dll_ = ivl_dlopen(tmp);
	    delete[]tmp;
      }

      if (dll_ == 0) {
	    cerr << "error: " << dll_path_ << " failed to load." << endl;
	    cerr << dll_path_ << ": " << dlerror() << endl;
	    return false;
      }

      stmt_cur_ = 0;

	// Initialize the design object.
      des_.self = des;
      des_.time_precision = des->get_precision();

      des_.disciplines.resize(disciplines.size());
      unsigned idx = 0;
      for (map<perm_string,ivl_discipline_t>::const_iterator cur = disciplines.begin()
		 ; cur != disciplines.end() ; ++ cur ) {
	    des_.disciplines[idx] = cur->second;
	    idx += 1;
      }
      assert(idx == des_.disciplines.size());

      list<NetScope *> scope_list;

      scope_list = des->find_package_scopes();
      for (list<NetScope*>::const_iterator cur = scope_list.begin()
		 ; cur != scope_list.end(); ++ cur ) {
	    add_root(*cur);
      }

      scope_list = des->find_root_scopes();
      for (list<NetScope*>::const_iterator cur = scope_list.begin()
		 ; cur != scope_list.end(); ++ cur ) {
	    add_root(*cur);
      }

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
      int rc;
      if (errors == 0) {
	    if (verbose_flag) {
		  cout << " ... invoking target_design" << endl;
	    }

	    rc = (target_)(&des_);
      } else {
	    if (verbose_flag) {
		  cout << " ... skipping target_design due to errors." << endl;
	    }
	    rc = errors;
      }

      ivl_dlclose(dll_);
      return rc;
}

void dll_target::switch_attributes(struct ivl_switch_s *obj,
				   const NetNode*net)
{
      obj->nattr = net->attr_cnt();
      obj->attr  = fill_in_attributes(net);
}

void dll_target::logic_attributes(struct ivl_net_logic_s *obj,
				  const NetNode*net)
{
      obj->nattr = net->attr_cnt();
      obj->attr  = fill_in_attributes(net);
}

void dll_target::make_delays_(ivl_expr_t*delay, const NetObj*net)
{
      delay[0] = 0;
      delay[1] = 0;
      delay[2] = 0;

	/* Translate delay expressions to ivl_target form. Try to
	   preserve pointer equality, not as a rule but to save on
	   expression trees. */
      if (net->rise_time()) {
	    expr_ = 0;
	    net->rise_time()->expr_scan(this);
	    delay[0] = expr_;
	    expr_ = 0;
      }
      if (net->fall_time()) {
	    if (net->fall_time() == net->rise_time()) {
		  delay[1] = delay[0];
	    } else {
		  expr_ = 0;
		  net->fall_time()->expr_scan(this);
		  delay[1] = expr_;
		  expr_ = 0;
	    }
      }
      if (net->decay_time()) {
	    if (net->decay_time() == net->rise_time()) {
		  delay[2] = delay[0];
	    } else {
		  expr_ = 0;
		  net->decay_time()->expr_scan(this);
		  delay[2] = expr_;
		  expr_ = 0;
	    }
      }
}

void dll_target::make_logic_delays_(struct ivl_net_logic_s*obj,
                                    const NetObj*net)
{
      make_delays_(obj->delay, net);
}

void dll_target::make_switch_delays_(struct ivl_switch_s*obj,
                                    const NetObj*net)
{
      make_delays_(obj->delay, net);
}

void dll_target::make_lpm_delays_(struct ivl_lpm_s*obj,
				  const NetObj*net)
{
      make_delays_(obj->delay, net);
}

void dll_target::make_const_delays_(struct ivl_net_const_s*obj,
				    const NetObj*net)
{
      make_delays_(obj->delay, net);
}

bool dll_target::branch(const NetBranch*net)
{
      struct ivl_branch_s*obj = net->target_obj();
      ivl_assert(*net, net->pin_count() == 2);

      assert(net->pin(0).nexus()->t_cookie());
      obj->pins[0] = net->pin(0).nexus()->t_cookie();
      nexus_bra_add(obj->pins[0], obj, 0);

      assert(net->pin(1).nexus()->t_cookie());
      obj->pins[1] = net->pin(1).nexus()->t_cookie();
      nexus_bra_add(obj->pins[1], obj, 1);

      obj->island = net->get_island();

      return true;
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

      obj->type_ = net->transparent()? IVL_LO_BUFT : IVL_LO_BUFZ;
      obj->width_= net->width();
      obj->is_cassign = 0;
      obj->is_port_buffer = net->port_info_index() >= 0;
      obj->npins_= 2;
      obj->pins_ = new ivl_nexus_t[2];
      FILE_NAME(obj, net);

	/* Get the ivl_nexus_t objects connected to the two pins.

	   (We know a priori that the ivl_nexus_t objects have been
	   allocated, because the signals have been scanned before
	   me. This saves me the trouble of allocating them.) */

      assert(net->pin(0).nexus()->t_cookie());
      obj->pins_[0] = net->pin(0).nexus()->t_cookie();
      ivl_nexus_ptr_t out_ptr = nexus_log_add(obj->pins_[0], obj, 0);

      out_ptr->drive0 = net->pin(0).drive0();
      out_ptr->drive1 = net->pin(0).drive1();

      assert(net->pin(1).nexus()->t_cookie());
      obj->pins_[1] = net->pin(1).nexus()->t_cookie();
      nexus_log_add(obj->pins_[1], obj, 1);

	/* Attach the logic device to the scope that contains it. */

      assert(net->scope());
      ivl_scope_t scop = find_scope(des_, net->scope());
      assert(scop);

      obj->scope_ = scop;

      obj->name_ = net->name();
      logic_attributes(obj, net);

      make_logic_delays_(obj, net);

      scope_add_logic(scop, obj);

	// Add bufz to the corresponding port_info entry,
	// if it is an input / output buffer
	// This is needed for the SDF interconnect feature
	// to access the buffers directly from the port_info
      if (obj->is_port_buffer) {
	    scop->module_ports_info[net->port_info_index()].buffer = obj;
      }

      return true;
}

bool dll_target::class_type(const NetScope*in_scope, netclass_t*net)
{
      ivl_scope_t use_scope = find_scope(des_, in_scope);
      use_scope->classes.push_back(net);
      return true;
}

bool dll_target::enumeration(const NetScope*in_scope, netenum_t*net)
{
      ivl_scope_t use_scope = find_scope(des_, in_scope);
      use_scope->enumerations_.push_back(net);
      return true;
}

void dll_target::event(const NetEvent*net)
{
      struct ivl_event_s *obj = new struct ivl_event_s;

      FILE_NAME(obj, net);

      ivl_scope_t scop = find_scope(des_, net->scope());
      obj->name = net->name();
      obj->scope = scop;
      scope_add_event(scop, obj);

      obj->nany = 0;
      obj->nneg = 0;
      obj->npos = 0;
      obj->nedg = 0;

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
		      case NetEvProbe::EDGE:
			obj->nedg += pr->pin_count();
			break;
		  }
	    }

	    unsigned npins = obj->nany + obj->nneg + obj->npos + obj->nedg;
	    obj->pins = (ivl_nexus_t*)calloc(npins, sizeof(ivl_nexus_t));

      } else {
	    obj->pins  = 0;
      }

}

void dll_target::logic(const NetLogic*net)
{
      struct ivl_net_logic_s *obj = new struct ivl_net_logic_s;

      obj->width_ = net->width();
      obj->is_port_buffer = 0;

      FILE_NAME(obj, net);

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
	  case NetLogic::CMOS:
	    obj->type_ = IVL_LO_CMOS;
	    break;
	  case NetLogic::EQUIV:
	    obj->type_ = IVL_LO_EQUIV;
	    break;
	  case NetLogic::IMPL:
	    obj->type_ = IVL_LO_IMPL;
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
	  case NetLogic::RCMOS:
	    obj->type_ = IVL_LO_RCMOS;
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
	/* Some of the logical gates are used to represent operators in a
	 * continuous assignment, so set a flag if that is the case. */
      obj->is_cassign = net->is_cassign();

	/* Connect all the ivl_nexus_t objects to the pins of the
	   device. */

      obj->npins_ = net->pin_count();
      obj->pins_ = new ivl_nexus_t[obj->npins_];

      for (unsigned idx = 0 ;  idx < obj->npins_ ;  idx += 1) {
	    const Nexus*nex = net->pin(idx).nexus();
	    assert(nex->t_cookie());
	    obj->pins_[idx] = nex->t_cookie();
	    ivl_nexus_ptr_t tmp = nexus_log_add(obj->pins_[idx], obj, idx);
	    if (idx == 0) {
		  tmp->drive0 = net->pin(0).drive0();
		  tmp->drive1 = net->pin(0).drive1();
	    }
      }

      assert(net->scope());
      ivl_scope_t scop = find_scope(des_, net->scope());
      assert(scop);

      obj->scope_= scop;
      obj->name_ = net->name();

      logic_attributes(obj, net);

      make_logic_delays_(obj, net);

      scope_add_logic(scop, obj);
}

bool dll_target::tran(const NetTran*net)
{
      struct ivl_switch_s*obj = new struct ivl_switch_s;
      obj->type = net->type();
      obj->width = net->vector_width();
      obj->part = 0;
      obj->offset = 0;
      obj->name = net->name();
      obj->scope = find_scope(des_, net->scope());
      obj->island = net->get_island();
      assert(obj->scope);
      assert(obj->island);
      FILE_NAME(obj, net);

      const Nexus*nex;

      nex = net->pin(0).nexus();
      assert(nex->t_cookie());
      obj->pins[0] = nex->t_cookie();

      nex = net->pin(1).nexus();
      assert(nex->t_cookie());
      obj->pins[1] = nex->t_cookie();

      nexus_switch_add(obj->pins[0], obj, 0);
      nexus_switch_add(obj->pins[1], obj, 1);

      if (net->pin_count() > 2) {
	    nex = net->pin(2).nexus();
	    assert(nex->t_cookie());
	    obj->pins[2] = nex->t_cookie();
	    nexus_switch_add(obj->pins[2], obj, 2);
      } else {
	    obj->pins[2] = 0;
      }

      if (obj->type == IVL_SW_TRAN_VP) {
	    obj->part  = net->part_width();
	    obj->offset= net->part_offset();
      }

      switch_attributes(obj, net);
      make_switch_delays_(obj, net);
      scope_add_switch(obj->scope, obj);

      return true;
}

bool dll_target::substitute(const NetSubstitute*net)
{
      ivl_lpm_t obj = new struct ivl_lpm_s;
      obj->type = IVL_LPM_SUBSTITUTE;
      obj->name = net->name();
      assert(net->scope());
      obj->scope = find_scope(des_, net->scope());
      assert(obj->scope);
      FILE_NAME(obj, net);

      obj->width = net->width();
      obj->u_.substitute.base = net->base();

      obj->u_.substitute.q = net->pin(0).nexus()->t_cookie();
      obj->u_.substitute.a = net->pin(1).nexus()->t_cookie();
      obj->u_.substitute.s = net->pin(2).nexus()->t_cookie();
      nexus_lpm_add(obj->u_.substitute.q, obj, 0, IVL_DR_STRONG, IVL_DR_STRONG);
      nexus_lpm_add(obj->u_.substitute.a, obj, 0, IVL_DR_HiZ,    IVL_DR_HiZ);
      nexus_lpm_add(obj->u_.substitute.s, obj, 0, IVL_DR_HiZ,    IVL_DR_HiZ);

      make_lpm_delays_(obj, net);
      scope_add_lpm(obj->scope, obj);

      return true;
}

bool dll_target::sign_extend(const NetSignExtend*net)
{
      struct ivl_lpm_s*obj = new struct ivl_lpm_s;
      obj->type = IVL_LPM_SIGN_EXT;
      obj->width = net->width();
      obj->name = net->name();
      obj->scope = find_scope(des_, net->scope());
      assert(obj->scope);
      FILE_NAME(obj, net);

      const Nexus*nex;

      nex = net->pin(0).nexus();
      assert(nex->t_cookie());

      obj->u_.reduce.q = nex->t_cookie();
      nexus_lpm_add(obj->u_.reduce.q, obj, 0, IVL_DR_STRONG, IVL_DR_STRONG);

      nex = net->pin(1).nexus();
      assert(nex->t_cookie());

      obj->u_.reduce.a = nex->t_cookie();
      nexus_lpm_add(obj->u_.reduce.a, obj, 1, IVL_DR_HiZ, IVL_DR_HiZ);

      make_lpm_delays_(obj, net);

      scope_add_lpm(obj->scope, obj);

      return true;
}

bool dll_target::ureduce(const NetUReduce*net)
{
      struct ivl_lpm_s*obj = new struct ivl_lpm_s;
      switch (net->type()) {
	  case NetUReduce::NONE:
	    assert(0);
	    delete obj;
	    return false;
	  case NetUReduce::AND:
	    obj->type = IVL_LPM_RE_AND;
	    break;
	  case NetUReduce::OR:
	    obj->type = IVL_LPM_RE_OR;
	    break;
	  case NetUReduce::XOR:
	    obj->type = IVL_LPM_RE_XOR;
	    break;
	  case NetUReduce::NAND:
	    obj->type = IVL_LPM_RE_NAND;
	    break;
	  case NetUReduce::NOR:
	    obj->type = IVL_LPM_RE_NOR;
	    break;
	  case NetUReduce::XNOR:
	    obj->type = IVL_LPM_RE_XNOR;
	    break;
      }

      obj->name = net->name();
      obj->scope = find_scope(des_, net->scope());
      assert(obj->scope);
      FILE_NAME(obj, net);

      obj->width = net->width();

      const Nexus*nex;

      nex = net->pin(0).nexus();
      assert(nex->t_cookie());

      obj->u_.reduce.q = nex->t_cookie();
      nexus_lpm_add(obj->u_.reduce.q, obj, 0, IVL_DR_STRONG, IVL_DR_STRONG);

      nex = net->pin(1).nexus();
      assert(nex->t_cookie());

      obj->u_.reduce.a = nex->t_cookie();
      nexus_lpm_add(obj->u_.reduce.a, obj, 1, IVL_DR_HiZ, IVL_DR_HiZ);

      make_lpm_delays_(obj, net);

      scope_add_lpm(obj->scope, obj);

      return true;
}

void dll_target::net_case_cmp(const NetCaseCmp*net)
{
      struct ivl_lpm_s*obj = new struct ivl_lpm_s;
      switch (net->kind()) {
	  case NetCaseCmp::EEQ:
	    obj->type = IVL_LPM_CMP_EEQ;
	    break;
	  case NetCaseCmp::NEQ:
	    obj->type = IVL_LPM_CMP_NEE;
	    break;
	  case NetCaseCmp::WEQ:
	    obj->type = IVL_LPM_CMP_WEQ;
	    break;
	  case NetCaseCmp::WNE:
	    obj->type = IVL_LPM_CMP_WNE;
	    break;
	  case NetCaseCmp::XEQ:
	      obj->type = IVL_LPM_CMP_EQX;
	    break;
	  case NetCaseCmp::ZEQ:
	    obj->type = IVL_LPM_CMP_EQZ;
	    break;
      }
      obj->name  = net->name();
      obj->scope = find_scope(des_, net->scope());
      assert(obj->scope);
      FILE_NAME(obj, net);

      obj->width = net->width();
      obj->u_.arith.signed_flag = 0;

      const Nexus*nex;

      nex = net->pin(1).nexus();
      assert(nex->t_cookie());

      obj->u_.arith.a = nex->t_cookie();
      nexus_lpm_add(obj->u_.arith.a, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);

      nex = net->pin(2).nexus();
      assert(nex->t_cookie());

      obj->u_.arith.b = nex->t_cookie();
      nexus_lpm_add(obj->u_.arith.b, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);

      nex = net->pin(0).nexus();
      assert(nex->t_cookie());

      obj->u_.arith.q = nex->t_cookie();
      nexus_lpm_add(obj->u_.arith.q, obj, 0, IVL_DR_STRONG, IVL_DR_STRONG);

      make_lpm_delays_(obj, net);

      scope_add_lpm(obj->scope, obj);
}

ivl_event_t dll_target::make_lpm_trigger(const NetEvWait*net)
{
      ivl_event_t trigger = 0;
      if (net) {
            const NetEvent*ev = net->event(0);

              /* Locate the event by name. */
            ivl_scope_t ev_scope = lookup_scope_(ev->scope());

            assert(ev_scope);
            assert(ev_scope->nevent_ > 0);
            for (unsigned idx = 0;  idx < ev_scope->nevent_; idx += 1) {
                  const char*ename =
                        ivl_event_basename(ev_scope->event_[idx]);
                  if (strcmp(ev->name(), ename) == 0) {
                        trigger = ev_scope->event_[idx];
                        break;
                  }
            }

              /* Connect up the probe pins. This wasn't done during the
                 ::event method because the signals weren't scanned yet. */
            assert(ev->nprobe() == 1);
            const NetEvProbe*pr = ev->probe(0);
            for (unsigned bit = 0; bit < pr->pin_count(); bit += 1) {
                  ivl_nexus_t nex = (ivl_nexus_t)
                        pr->pin(bit).nexus()->t_cookie();
                  assert(nex);
                  trigger->pins[bit] = nex;
            }
      }
      return trigger;
}

bool dll_target::net_sysfunction(const NetSysFunc*net)
{
      unsigned idx;
      const Nexus*nex;

      struct ivl_lpm_s*obj = new struct ivl_lpm_s;
      obj->type = IVL_LPM_SFUNC;
      obj->name  = net->name();
      obj->scope = find_scope(des_, net->scope());
      assert(obj->scope);
      FILE_NAME(obj, net);

      obj->u_.sfunc.ports = net->pin_count();

      assert(net->pin_count() >= 1);
      obj->width = net->vector_width();

      obj->u_.sfunc.fun_name = net->func_name();

      obj->u_.sfunc.pins = new ivl_nexus_t[net->pin_count()];

      nex = net->pin(0).nexus();
      assert(nex->t_cookie());

      obj->u_.sfunc.pins[0] = nex->t_cookie();
      nexus_lpm_add(obj->u_.sfunc.pins[0], obj, 0,
		    IVL_DR_STRONG, IVL_DR_STRONG);

      for (idx = 1 ;  idx < net->pin_count() ;  idx += 1) {
	    nex = net->pin(idx).nexus();
	    assert(nex->t_cookie());

	    obj->u_.sfunc.pins[idx] = nex->t_cookie();
	    nexus_lpm_add(obj->u_.sfunc.pins[idx], obj, 0,
			  IVL_DR_HiZ, IVL_DR_HiZ);
      }

	/* Save information about the trigger event if it exists. */
      obj->u_.sfunc.trigger = make_lpm_trigger(net->trigger());

      make_lpm_delays_(obj, net);

      scope_add_lpm(obj->scope, obj);
      return true;
}

/*
 * An IVL_LPM_UFUNC represents a node in a combinational expression
 * that calls a user defined function. I create an LPM object that has
 * the right connections, and refers to the ivl_scope_t of the
 * definition.
 */
bool dll_target::net_function(const NetUserFunc*net)
{
      struct ivl_lpm_s*obj = new struct ivl_lpm_s;
      obj->type = IVL_LPM_UFUNC;
      obj->name  = net->name();
      obj->scope = find_scope(des_, net->scope());
      assert(obj->scope);
      FILE_NAME(obj, net);

	/* Get the definition of the function and save it. */
      const NetScope*def = net->def();
      assert(def);

      obj->u_.ufunc.def = lookup_scope_(def);

	/* Save information about the ports in the ivl_lpm_s
	   structure. Note that port 0 is the return value. */
      obj->u_.ufunc.ports = net->pin_count();

      assert(net->pin_count() >= 1);
      obj->width = net->port_width(0);

	/* Now collect all the pins and connect them to the nexa of
	   the net. The output pins have strong drive, and the
	   remaining input pins are HiZ. */

      obj->u_.ufunc.pins = new ivl_nexus_t[net->pin_count()];

      for (unsigned idx = 0 ;  idx < net->pin_count() ;  idx += 1) {
	    const Nexus*nex = net->pin(idx).nexus();
	    assert(nex->t_cookie());
	    ivl_nexus_t nn = nex->t_cookie();
	    assert(nn);

	    obj->u_.ufunc.pins[idx] = nn;
	    ivl_drive_t drive = idx == 0 ? IVL_DR_STRONG : IVL_DR_HiZ;
	    nexus_lpm_add(obj->u_.ufunc.pins[idx], obj, idx, drive, drive);
      }

	/* Save information about the trigger event if it exists. */
      obj->u_.ufunc.trigger = make_lpm_trigger(net->trigger());

      make_lpm_delays_(obj, net);

	/* All done. Add this LPM to the scope. */
      scope_add_lpm(obj->scope, obj);

      return true;
}

void dll_target::udp(const NetUDP*net)
{
      struct ivl_net_logic_s *obj = new struct ivl_net_logic_s;

      obj->type_ = IVL_LO_UDP;
      obj->is_port_buffer = 0;
      FILE_NAME(obj, net);

	/* The NetUDP class hasn't learned about width yet, so we
	   assume a width of 1. */
      obj->width_ = 1;
      obj->is_cassign = 0;

      static map<perm_string,ivl_udp_t> udps;
      ivl_udp_t u;

      if (udps.find(net->udp_name()) != udps.end()) {
	    u = udps[net->udp_name()];
      } else {
	    u = new struct ivl_udp_s;
	    u->nrows = net->rows();
	    u->table = (ivl_udp_s::ccharp_t*)malloc((u->nrows+1)*sizeof(char*));
	    u->table[u->nrows] = 0x0;
	    u->nin = net->nin();
	    u->sequ = net->is_sequential();
	    u->file = net->udp_file();
	    u->lineno = net->udp_lineno();
	    if (u->sequ) u->init = net->get_initial();
	    else u->init = 'x';
	    u->name = net->udp_name();
	    string inp;
	    char out;
	    unsigned int i = 0;
	    if (net->first(inp, out)) do {
		  string tt = inp+out;
		  u->table[i++] = strings_.add(tt.c_str());
	    } while (net->next(inp, out));
	    assert(i==u->nrows);
	    assert((u->nin + 1) == net->port_count());
	    u->ports = new string [u->nin + 1];
	    for(unsigned idx = 0; idx <= u->nin; idx += 1) {
		  u->ports[idx] = net->port_name(idx);
	    }

	    udps[net->udp_name()] = u;
      }

      obj->udp = u;

      // Some duplication of code here, see: dll_target::logic()

        /* Connect all the ivl_nexus_t objects to the pins of the
	   device. */

      obj->npins_ = net->pin_count();
      obj->pins_ = new ivl_nexus_t[obj->npins_];
      for (unsigned idx = 0 ;  idx < obj->npins_ ;  idx += 1) {
	      /* Skip unconnected input pins. These will take on HiZ
		 values by the code generators. */
	    if (! net->pin(idx).is_linked()) {
		  obj->pins_[idx] = 0;
		  continue;
	    }

	    const Nexus*nex = net->pin(idx).nexus();
	    ivl_assert(*net, nex && nex->t_cookie());
	    obj->pins_[idx] = nex->t_cookie();
	    nexus_log_add(obj->pins_[idx], obj, idx);
      }

      assert(net->scope());
      ivl_scope_t scop = find_scope(des_, net->scope());
      assert(scop);

      obj->scope_= scop;
      obj->name_ = net->name();
      FILE_NAME(obj, net);

      make_logic_delays_(obj, net);

      obj->nattr = 0;
      obj->attr = 0;

      scope_add_logic(scop, obj);
}

void dll_target::lpm_abs(const NetAbs*net)
{
      ivl_lpm_t obj = new struct ivl_lpm_s;
      obj->type = IVL_LPM_ABS;
      obj->name = net->name(); // NetAddSub names are permallocated.
      assert(net->scope());
      obj->scope = find_scope(des_, net->scope());
      assert(obj->scope);
      FILE_NAME(obj, net);

      obj->u_.arith.signed_flag = 0;
      obj->width = net->width();

      const Nexus*nex;
	/* the output is pin(0) */
      nex = net->pin(0).nexus();
      assert(nex->t_cookie());

      obj->u_.arith.q = nex->t_cookie();
      nexus_lpm_add(obj->u_.arith.q, obj, 0, IVL_DR_STRONG, IVL_DR_STRONG);

      nex = net->pin(1).nexus();
      assert(nex->t_cookie());

	/* pin(1) is the input data. */
      obj->u_.arith.a = nex->t_cookie();
      nexus_lpm_add(obj->u_.arith.a, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);

      make_lpm_delays_(obj, net);

      scope_add_lpm(obj->scope, obj);
}

void dll_target::lpm_add_sub(const NetAddSub*net)
{
      ivl_lpm_t obj = new struct ivl_lpm_s;
      if (net->attribute(perm_string::literal("LPM_Direction")) == verinum("SUB"))
	    obj->type = IVL_LPM_SUB;
      else
	    obj->type = IVL_LPM_ADD;
      obj->name = net->name(); // NetAddSub names are permallocated.
      assert(net->scope());
      obj->scope = find_scope(des_, net->scope());
      assert(obj->scope);
      FILE_NAME(obj, net);

      obj->u_.arith.signed_flag = 0;

	/* Choose the width of the adder. If the carry bit is
	   connected, then widen the adder by one and plan on leaving
	   the fake inputs unconnected. */
      obj->width = net->width();
      if (net->pin_Cout().is_linked()) {
	    obj->width += 1;
      }


      const Nexus*nex;

      nex = net->pin_Result().nexus();
      assert(nex->t_cookie());

      obj->u_.arith.q = nex->t_cookie();
      nexus_lpm_add(obj->u_.arith.q, obj, 0, IVL_DR_STRONG, IVL_DR_STRONG);

      nex = net->pin_DataA().nexus();
      assert(nex->t_cookie());

      obj->u_.arith.a = nex->t_cookie();
      nexus_lpm_add(obj->u_.arith.a, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);

      nex = net->pin_DataB().nexus();
      assert(nex->t_cookie());

      obj->u_.arith.b = nex->t_cookie();
      nexus_lpm_add(obj->u_.arith.b, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);

	/* If the carry output is connected, then connect the extra Q
	   pin to the carry nexus and zero the a and b inputs. */
      if (net->pin_Cout().is_linked()) {
	    cerr << "XXXX: t-dll.cc: Forgot how to connect cout." << endl;
      }

      make_lpm_delays_(obj, net);

      scope_add_lpm(obj->scope, obj);
}

bool dll_target::lpm_array_dq(const NetArrayDq*net)
{
      ivl_lpm_t obj = new struct ivl_lpm_s;
      obj->type = IVL_LPM_ARRAY;
      obj->name = net->name();
      obj->u_.array.sig = find_signal(des_, net->mem());
      assert(obj->u_.array.sig);
      obj->scope = find_scope(des_, net->scope());
      assert(obj->scope);
      FILE_NAME(obj, net);
      obj->width = net->width();
      obj->u_.array.swid = net->awidth();

      make_lpm_delays_(obj, net);

      scope_add_lpm(obj->scope, obj);

      const Nexus*nex;

      nex = net->pin_Address().nexus();
      assert(nex->t_cookie());
      obj->u_.array.a = nex->t_cookie();
      nexus_lpm_add(obj->u_.array.a, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);

      nex = net->pin_Result().nexus();
      assert(nex->t_cookie());
      obj->u_.array.q = nex->t_cookie();
      nexus_lpm_add(obj->u_.array.q, obj, 0, IVL_DR_STRONG, IVL_DR_STRONG);

      return true;
}

/*
 * The lpm_clshift device represents both left and right shifts,
 * depending on what is connected to the Direction pin. We convert
 * this device into SHIFTL or SHIFTR devices.
 */
void dll_target::lpm_clshift(const NetCLShift*net)
{
      ivl_lpm_t obj = new struct ivl_lpm_s;
      obj->type = IVL_LPM_SHIFTL;
      obj->name = net->name();
      assert(net->scope());
      obj->scope = find_scope(des_, net->scope());
      assert(obj->scope);
      FILE_NAME(obj, net);

	/* Look at the direction input of the device, and select the
	   shift direction accordingly. */
      if (net->right_flag())
	    obj->type = IVL_LPM_SHIFTR;
      if (net->signed_flag())
	    obj->u_.shift.signed_flag = 1;
      else
	    obj->u_.shift.signed_flag = 0;

      obj->width = net->width();
      obj->u_.shift.select = net->width_dist();

      const Nexus*nex;

      nex = net->pin_Result().nexus();
      assert(nex->t_cookie());

      obj->u_.shift.q = nex->t_cookie();
      nexus_lpm_add(obj->u_.shift.q, obj, 0, IVL_DR_STRONG, IVL_DR_STRONG);

      nex = net->pin_Data().nexus();
      assert(nex->t_cookie());

      obj->u_.shift.d = nex->t_cookie();
      nexus_lpm_add(obj->u_.shift.d, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);

      nex = net->pin_Distance().nexus();
      assert(nex->t_cookie());

      obj->u_.shift.s = nex->t_cookie();
      nexus_lpm_add(obj->u_.shift.s, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);

      make_lpm_delays_(obj, net);

      scope_add_lpm(obj->scope, obj);
}

bool dll_target::lpm_arith1_(ivl_lpm_type_t lpm_type, unsigned width, bool signed_flag, const NetNode*net)
{
      ivl_lpm_t obj = new struct ivl_lpm_s;
      obj->type = lpm_type;
      obj->name = net->name(); // NetCastInt2 names are permallocated
      assert(net->scope());
      obj->scope = find_scope(des_, net->scope());
      assert(obj->scope);
      FILE_NAME(obj, net);

      obj->width = width;
      obj->u_.arith.signed_flag = signed_flag? 1 : 0;

      const Nexus*nex;

      nex = net->pin(0).nexus();
      assert(nex->t_cookie());

      obj->u_.arith.q = nex->t_cookie();

      nex = net->pin(1).nexus();
      assert(nex->t_cookie());
      obj->u_.arith.a = nex->t_cookie();

      nexus_lpm_add(obj->u_.arith.q, obj, 0, IVL_DR_STRONG, IVL_DR_STRONG);
      nexus_lpm_add(obj->u_.arith.a, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);

      make_lpm_delays_(obj, net);

      scope_add_lpm(obj->scope, obj);

      return true;
}

bool dll_target::lpm_cast_int2(const NetCastInt2*net)
{
      return lpm_arith1_(IVL_LPM_CAST_INT2, net->width(), true, net);
}

bool dll_target::lpm_cast_int4(const NetCastInt4*net)
{
      return lpm_arith1_(IVL_LPM_CAST_INT, net->width(), true, net);
}

bool dll_target::lpm_cast_real(const NetCastReal*net)
{
      return lpm_arith1_(IVL_LPM_CAST_REAL, 0, net->signed_flag(), net);
}

/*
 * Make out of the NetCompare object an ivl_lpm_s object. The
 * comparators in ivl_target do not support < or <=, but they can be
 * trivially converted to > and >= by swapping the operands.
 */
void dll_target::lpm_compare(const NetCompare*net)
{
      ivl_lpm_t obj = new struct ivl_lpm_s;
      obj->name = net->name(); // NetCompare names are permallocated
      assert(net->scope());
      obj->scope = find_scope(des_, net->scope());
      assert(obj->scope);
      FILE_NAME(obj, net);

      bool swap_operands = false;

      obj->width = net->width();
      obj->u_.arith.signed_flag = net->get_signed()? 1 : 0;

      const Nexus*nex;

      nex = net->pin_DataA().nexus();
      assert(nex->t_cookie());

      obj->u_.arith.a = nex->t_cookie();

      nex = net->pin_DataB().nexus();
      assert(nex->t_cookie());

      obj->u_.arith.b = nex->t_cookie();


      if (net->pin_AGEB().is_linked()) {
	    nex = net->pin_AGEB().nexus();
	    obj->type = IVL_LPM_CMP_GE;

	    assert(nex->t_cookie());
	    obj->u_.arith.q = nex->t_cookie();
	    nexus_lpm_add(obj->u_.arith.q, obj, 0,
			  IVL_DR_STRONG, IVL_DR_STRONG);

      } else if (net->pin_AGB().is_linked()) {
	    nex = net->pin_AGB().nexus();
	    obj->type = IVL_LPM_CMP_GT;

	    assert(nex->t_cookie());
	    obj->u_.arith.q = nex->t_cookie();
	    nexus_lpm_add(obj->u_.arith.q, obj, 0,
			  IVL_DR_STRONG, IVL_DR_STRONG);

      } else if (net->pin_ALEB().is_linked()) {
	    nex = net->pin_ALEB().nexus();
	    obj->type = IVL_LPM_CMP_GE;

	    assert(nex->t_cookie());
	    obj->u_.arith.q = nex->t_cookie();
	    nexus_lpm_add(obj->u_.arith.q, obj, 0,
			  IVL_DR_STRONG, IVL_DR_STRONG);

	    swap_operands = true;

      } else if (net->pin_ALB().is_linked()) {
	    nex = net->pin_ALB().nexus();
	    obj->type = IVL_LPM_CMP_GT;

	    assert(nex->t_cookie());
	    obj->u_.arith.q = nex->t_cookie();
	    nexus_lpm_add(obj->u_.arith.q, obj, 0,
			  IVL_DR_STRONG, IVL_DR_STRONG);

	    swap_operands = true;

      } else if (net->pin_AEB().is_linked()) {
	    nex = net->pin_AEB().nexus();
	    obj->type = IVL_LPM_CMP_EQ;

	    assert(nex->t_cookie());
	    obj->u_.arith.q = nex->t_cookie();
	    nexus_lpm_add(obj->u_.arith.q, obj, 0,
			  IVL_DR_STRONG, IVL_DR_STRONG);

      } else if (net->pin_ANEB().is_linked()) {
	    nex = net->pin_ANEB().nexus();
	    obj->type = IVL_LPM_CMP_NE;

	    assert(nex->t_cookie());
	    obj->u_.arith.q = nex->t_cookie();
	    nexus_lpm_add(obj->u_.arith.q, obj, 0,
			  IVL_DR_STRONG, IVL_DR_STRONG);

      } else {
	    assert(0);
      }

      if (swap_operands) {
	    ivl_nexus_t tmp = obj->u_.arith.a;
	    obj->u_.arith.a = obj->u_.arith.b;
	    obj->u_.arith.b = tmp;
      }

      nexus_lpm_add(obj->u_.arith.a, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);
      nexus_lpm_add(obj->u_.arith.b, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);

      make_lpm_delays_(obj, net);

      scope_add_lpm(obj->scope, obj);
}

void dll_target::lpm_divide(const NetDivide*net)
{
      ivl_lpm_t obj = new struct ivl_lpm_s;
      obj->type  = IVL_LPM_DIVIDE;
      obj->name  = net->name();
      assert(net->scope());
      obj->scope = find_scope(des_, net->scope());
      assert(obj->scope);
      FILE_NAME(obj, net);

      unsigned wid = net->width_r();

      obj->width = wid;
      obj->u_.arith.signed_flag = net->get_signed()? 1 : 0;

      const Nexus*nex;

      nex = net->pin_Result().nexus();
      assert(nex->t_cookie());

      obj->u_.arith.q = nex->t_cookie();
      nexus_lpm_add(obj->u_.arith.q, obj, 0, IVL_DR_STRONG, IVL_DR_STRONG);

      nex = net->pin_DataA().nexus();
      assert(nex->t_cookie());

      obj->u_.arith.a = nex->t_cookie();
      nexus_lpm_add(obj->u_.arith.a, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);

      nex = net->pin_DataB().nexus();
      assert(nex->t_cookie());

      obj->u_.arith.b = nex->t_cookie();
      nexus_lpm_add(obj->u_.arith.b, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);

      make_lpm_delays_(obj, net);

      scope_add_lpm(obj->scope, obj);
}

void dll_target::lpm_modulo(const NetModulo*net)
{
      ivl_lpm_t obj = new struct ivl_lpm_s;
      obj->type  = IVL_LPM_MOD;
      obj->name  = net->name();
      assert(net->scope());
      obj->scope = find_scope(des_, net->scope());
      assert(obj->scope);
      FILE_NAME(obj, net);

      unsigned wid = net->width_r();

      obj->width = wid;
      obj->u_.arith.signed_flag = net->get_signed()? 1 : 0;

      const Nexus*nex;

      nex = net->pin_Result().nexus();
      assert(nex->t_cookie());

      obj->u_.arith.q = nex->t_cookie();
      nexus_lpm_add(obj->u_.arith.q, obj, 0, IVL_DR_STRONG, IVL_DR_STRONG);

      nex = net->pin_DataA().nexus();
      assert(nex->t_cookie());

      obj->u_.arith.a = nex->t_cookie();
      nexus_lpm_add(obj->u_.arith.a, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);

      nex = net->pin_DataB().nexus();
      assert(nex->t_cookie());

      obj->u_.arith.b = nex->t_cookie();
      nexus_lpm_add(obj->u_.arith.b, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);

      make_lpm_delays_(obj, net);

      scope_add_lpm(obj->scope, obj);
}

void dll_target::lpm_ff(const NetFF*net)
{
      ivl_lpm_t obj = new struct ivl_lpm_s;
      obj->type  = IVL_LPM_FF;
      obj->name  = net->name();
      obj->scope = find_scope(des_, net->scope());
      assert(obj->scope);
      FILE_NAME(obj, net);

      obj->width = net->width();

      scope_add_lpm(obj->scope, obj);

      const Nexus*nex;

	/* Set the clock polarity. */
      obj->u_.ff.negedge_flag = net->is_negedge();

	/* Set the clk signal to point to the nexus, and the nexus to
	   point back to this device. */
      nex = net->pin_Clock().nexus();
      assert(nex->t_cookie());
      obj->u_.ff.clk = nex->t_cookie();
      assert(obj->u_.ff.clk);
      nexus_lpm_add(obj->u_.ff.clk, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);

	/* If there is a clock enable, then connect it up to the FF
	   device. */
      if (net->pin_Enable().is_linked()) {
	    nex = net->pin_Enable().nexus();
	    assert(nex->t_cookie());
	    obj->u_.ff.we = nex->t_cookie();
	    assert(obj->u_.ff.we);
	    nexus_lpm_add(obj->u_.ff.we, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);
      } else {
	    obj->u_.ff.we = 0;
      }

      if (net->pin_Aclr().is_linked()) {
	    nex = net->pin_Aclr().nexus();
	    assert(nex->t_cookie());
	    obj->u_.ff.aclr = nex->t_cookie();
	    assert(obj->u_.ff.aclr);
	    nexus_lpm_add(obj->u_.ff.aclr, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);
      } else {
	    obj->u_.ff.aclr = 0;
      }

      if (net->pin_Aset().is_linked()) {
	    nex = net->pin_Aset().nexus();
	    assert(nex->t_cookie());
	    obj->u_.ff.aset = nex->t_cookie();
	    assert(obj->u_.ff.aset);
	    nexus_lpm_add(obj->u_.ff.aset, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);

	    verinum tmp = net->aset_value();
	    if (tmp.len() > 0)
		  obj->u_.ff.aset_value = expr_from_value_(tmp);
	    else
		  obj->u_.ff.aset_value = 0;

      } else {
	    obj->u_.ff.aset = 0;
	    obj->u_.ff.aset_value = 0;
      }

      if (net->pin_Sclr().is_linked()) {
	    nex = net->pin_Sclr().nexus();
	    assert(nex->t_cookie());
	    obj->u_.ff.sclr = nex->t_cookie();
	    assert(obj->u_.ff.sclr);
	    nexus_lpm_add(obj->u_.ff.sclr, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);
      } else {
	    obj->u_.ff.sclr = 0;
      }

      if (net->pin_Sset().is_linked()) {
	    nex = net->pin_Sset().nexus();
	    assert(nex->t_cookie());
	    obj->u_.ff.sset = nex->t_cookie();
	    assert(obj->u_.ff.sset);
	    nexus_lpm_add(obj->u_.ff.sset, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);

	    verinum tmp = net->sset_value();
	    if (tmp.len() > 0)
		  obj->u_.ff.sset_value = expr_from_value_(tmp);
	    else
		  obj->u_.ff.sset_value = 0;

      } else {
	    obj->u_.ff.sset = 0;
	    obj->u_.ff.sset_value = 0;
      }

      nex = net->pin_Q().nexus();
      assert(nex->t_cookie());
      obj->u_.ff.q.pin = nex->t_cookie();
      nexus_lpm_add(obj->u_.ff.q.pin, obj, 0,
		    IVL_DR_STRONG, IVL_DR_STRONG);

      nex = net->pin_Data().nexus();
      assert(nex->t_cookie());
      obj->u_.ff.d.pin = nex->t_cookie();
      nexus_lpm_add(obj->u_.ff.d.pin, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);
}

void dll_target::lpm_latch(const NetLatch*net)
{
      ivl_lpm_t obj = new struct ivl_lpm_s;
      obj->type  = IVL_LPM_LATCH;
      obj->name  = net->name();
      obj->scope = find_scope(des_, net->scope());
      assert(obj->scope);
      FILE_NAME(obj, net);

      obj->width = net->width();

      scope_add_lpm(obj->scope, obj);

      const Nexus*nex;

      nex = net->pin_Enable().nexus();
      assert(nex->t_cookie());
      obj->u_.latch.e = nex->t_cookie();
      assert(obj->u_.latch.e);
      nexus_lpm_add(obj->u_.latch.e, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);

      nex = net->pin_Q().nexus();
      assert(nex->t_cookie());
      obj->u_.latch.q.pin = nex->t_cookie();
      nexus_lpm_add(obj->u_.latch.q.pin, obj, 0,
		    IVL_DR_STRONG, IVL_DR_STRONG);

      nex = net->pin_Data().nexus();
      assert(nex->t_cookie());
      obj->u_.latch.d.pin = nex->t_cookie();
      nexus_lpm_add(obj->u_.latch.d.pin, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);
}

/*
 * Make the NetMult object into an IVL_LPM_MULT node.
 */
void dll_target::lpm_mult(const NetMult*net)
{
      ivl_lpm_t obj = new struct ivl_lpm_s;
      obj->type  = IVL_LPM_MULT;
      obj->name  = net->name();
      assert(net->scope());
      obj->scope = find_scope(des_, net->scope());
      assert(obj->scope);
      FILE_NAME(obj, net);

      unsigned wid = net->width_r();

      obj->width = wid;
      obj->u_.arith.signed_flag = 0;

      const Nexus*nex;

      nex = net->pin_Result().nexus();
      assert(nex->t_cookie());

      obj->u_.arith.q = nex->t_cookie();
      nexus_lpm_add(obj->u_.arith.q, obj, 0, IVL_DR_STRONG, IVL_DR_STRONG);

      nex = net->pin_DataA().nexus();
      assert(nex->t_cookie());

      obj->u_.arith.a = nex->t_cookie();
      nexus_lpm_add(obj->u_.arith.a, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);

      nex = net->pin_DataB().nexus();
      assert(nex->t_cookie());

      obj->u_.arith.b = nex->t_cookie();
      nexus_lpm_add(obj->u_.arith.b, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);

      make_lpm_delays_(obj, net);

      scope_add_lpm(obj->scope, obj);
}

/*
 * Hook up the mux devices so that the select expression selects the
 * correct sub-expression with the ivl_lpm_data2 function.
 */
void dll_target::lpm_mux(const NetMux*net)
{
      ivl_lpm_t obj = new struct ivl_lpm_s;
      obj->type  = IVL_LPM_MUX;
      obj->name  = net->name(); // The NetMux permallocates its name.
      obj->scope = find_scope(des_, net->scope());
      assert(obj->scope);
      FILE_NAME(obj, net);

      obj->width = net->width();
      obj->u_.mux.size  = net->size();
      obj->u_.mux.swid  = net->sel_width();

      make_lpm_delays_(obj, net);

      scope_add_lpm(obj->scope, obj);

      const Nexus*nex;

	/* Connect the output bits. */
      nex = net->pin_Result().nexus();
      assert(nex->t_cookie());
      obj->u_.mux.q = nex->t_cookie();
      nexus_lpm_add(obj->u_.mux.q, obj, 0,
		    net->pin_Result().drive0(),
		    net->pin_Result().drive1());

	/* Connect the select bits. */
      nex = net->pin_Sel().nexus();
      assert(nex->t_cookie());
      obj->u_.mux.s = nex->t_cookie();
      nexus_lpm_add(obj->u_.mux.s, obj, 0,
		    IVL_DR_HiZ, IVL_DR_HiZ);

      unsigned selects = obj->u_.mux.size;

      obj->u_.mux.d = new ivl_nexus_t [selects];

      for (unsigned sdx = 0 ;  sdx < selects ;  sdx += 1) {
	    nex = net->pin_Data(sdx).nexus();
	    ivl_nexus_t tmp = nex->t_cookie();
	    obj->u_.mux.d[sdx] = tmp;
	    if (tmp == 0) {
		  cerr << net->get_fileline() << ": internal error: "
		       << "dll_target::lpm_mux: "
		       << "Missing data port " << sdx
		       << " of mux " << obj->name << "." << endl;
	    }
	    ivl_assert(*net, tmp);
	    nexus_lpm_add(tmp, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);
      }

}

/*
 * Make the NetPow object into an IVL_LPM_POW node.
 */
void dll_target::lpm_pow(const NetPow*net)
{
      ivl_lpm_t obj = new struct ivl_lpm_s;
      obj->type  = IVL_LPM_POW;
      FILE_NAME(obj, net);
      obj->name  = net->name();
      assert(net->scope());
      obj->scope = find_scope(des_, net->scope());
      assert(obj->scope);
      FILE_NAME(obj, net);

      unsigned wid = net->width_r();
      obj->u_.arith.signed_flag = net->get_signed()? 1 : 0;

      obj->width = wid;

      const Nexus*nex;

      nex = net->pin_Result().nexus();
      assert(nex->t_cookie());

      obj->u_.arith.q = nex->t_cookie();
      nexus_lpm_add(obj->u_.arith.q, obj, 0, IVL_DR_STRONG, IVL_DR_STRONG);

      nex = net->pin_DataA().nexus();
      assert(nex->t_cookie());

      obj->u_.arith.a = nex->t_cookie();
      nexus_lpm_add(obj->u_.arith.a, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);

      nex = net->pin_DataB().nexus();
      assert(nex->t_cookie());

      obj->u_.arith.b = nex->t_cookie();
      nexus_lpm_add(obj->u_.arith.b, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);

      make_lpm_delays_(obj, net);

      scope_add_lpm(obj->scope, obj);
}

bool dll_target::concat(const NetConcat*net)
{
      ivl_lpm_t obj = new struct ivl_lpm_s;
      obj->type = net->transparent()? IVL_LPM_CONCATZ : IVL_LPM_CONCAT;
      obj->name = net->name(); // NetConcat names are permallocated
      assert(net->scope());
      obj->scope = find_scope(des_, net->scope());
      assert(obj->scope);
      FILE_NAME(obj, net);

      obj->width = net->width();

      obj->u_.concat.inputs = net->pin_count() - 1;
      obj->u_.concat.pins = new ivl_nexus_t[obj->u_.concat.inputs+1];

      for (unsigned idx = 0 ;  idx < obj->u_.concat.inputs+1 ; idx += 1) {
	    ivl_drive_t dr = idx == 0? IVL_DR_STRONG : IVL_DR_HiZ;
	    const Nexus*nex = net->pin(idx).nexus();
	    assert(nex->t_cookie());

	    obj->u_.concat.pins[idx] = nex->t_cookie();
	    nexus_lpm_add(obj->u_.concat.pins[idx], obj, 0, dr, dr);
      }

      make_lpm_delays_(obj, net);

      scope_add_lpm(obj->scope, obj);

      return true;
}

bool dll_target::part_select(const NetPartSelect*net)
{
      ivl_lpm_t obj = new struct ivl_lpm_s;
      switch (net->dir()) {
	  case NetPartSelect::VP:
	    obj->type = IVL_LPM_PART_VP;
	    break;
	  case NetPartSelect::PV:
	    obj->type = IVL_LPM_PART_PV;
	    break;
      }
      obj->name = net->name(); // NetPartSelect names are permallocated.
      assert(net->scope());
      obj->scope = find_scope(des_, net->scope());
      assert(obj->scope);
      FILE_NAME(obj, net);

	/* Part selects are always unsigned, so we use this to indicate
	 * if the part select base signal is signed or not. */
      if (net->signed_flag())
	    obj->u_.part.signed_flag = 1;
      else
	    obj->u_.part.signed_flag = 0;

	/* Choose the width of the part select. */
      obj->width = net->width();
      obj->u_.part.base  = net->base();
      obj->u_.part.s = 0;

      const Nexus*nex;

      switch (obj->type) {
	  case IVL_LPM_PART_VP:
	      /* NetPartSelect:pin(0) is the output pin. */
	    nex = net->pin(0).nexus();
	    assert(nex->t_cookie());

	    obj->u_.part.q = nex->t_cookie();

	      /* NetPartSelect:pin(1) is the input pin. */
	    nex = net->pin(1).nexus();
	    assert(nex->t_cookie());

	    obj->u_.part.a = nex->t_cookie();

	      /* If the part select has an additional pin, that pin is
		 a variable select base. */
	    if (net->pin_count() >= 3) {
		  nex = net->pin(2).nexus();
		  assert(nex->t_cookie());
		  obj->u_.part.s = nex->t_cookie();
	    }
	    break;

	  case IVL_LPM_PART_PV:
	      /* NetPartSelect:pin(1) is the output pin. */
	    nex = net->pin(1).nexus();
	    assert(nex->t_cookie());

	    obj->u_.part.q = nex->t_cookie();

	      /* NetPartSelect:pin(0) is the input pin. */
	    nex = net->pin(0).nexus();
	    assert(nex->t_cookie());

	    obj->u_.part.a = nex->t_cookie();
	    break;

	  default:
	    assert(0);
      }

      nexus_lpm_add(obj->u_.part.q, obj, 0, IVL_DR_STRONG, IVL_DR_STRONG);
      nexus_lpm_add(obj->u_.part.a, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);

	/* The select input is optional. */
      if (obj->u_.part.s)
	  nexus_lpm_add(obj->u_.part.s, obj, 0, IVL_DR_HiZ, IVL_DR_HiZ);

      make_lpm_delays_(obj, net);

      scope_add_lpm(obj->scope, obj);

      return true;
}

bool dll_target::replicate(const NetReplicate*net)
{
      ivl_lpm_t obj = new struct ivl_lpm_s;
      obj->type = IVL_LPM_REPEAT;
      obj->name = net->name();
      assert(net->scope());
      obj->scope = find_scope(des_, net->scope());
      assert(obj->scope);
      FILE_NAME(obj, net);

      obj->width = net->width();
      obj->u_.repeat.count = net->repeat();

      ivl_drive_t dr = IVL_DR_STRONG;
      const Nexus*nex = net->pin(0).nexus();
      assert(nex->t_cookie());

      obj->u_.repeat.q = nex->t_cookie();
      nexus_lpm_add(obj->u_.repeat.q, obj, 0, dr, dr);

      dr = IVL_DR_HiZ;
      nex = net->pin(1).nexus();
      assert(nex->t_cookie());

      obj->u_.repeat.a = nex->t_cookie();
      nexus_lpm_add(obj->u_.repeat.a, obj, 0, dr, dr);

      make_lpm_delays_(obj, net);

      scope_add_lpm(obj->scope, obj);

      return true;
}

/*
 * The assignment l-values are captured by the assignment statements
 * themselves in the process handling.
 */
void dll_target::net_assign(const NetAssign_*) const
{
}

bool dll_target::net_const(const NetConst*net)
{
      unsigned idx;
      char*bits;
      static char*bits_tmp = 0;
      static unsigned bits_cnt = 0;

      struct ivl_net_const_s *obj = new struct ivl_net_const_s;

      if (net->is_string()) {
	    obj->type = IVL_VT_STRING;
	    assert((net->width() % 8) == 0);
      } else obj->type = IVL_VT_BOOL;
      assert(net->scope());
      obj->scope = find_scope(des_, net->scope());
      FILE_NAME(obj, net);

	/* constants have a single vector output. */
      assert(net->pin_count() == 1);

      obj->width_ = net->width();
      obj->signed_ = net->value().has_sign();
      if (obj->width_ <= sizeof(obj->b.bit_)) {
	    bits = obj->b.bit_;

      } else {
	    if (obj->width_ >= bits_cnt) {
		  bits_tmp = (char*)realloc(bits_tmp, obj->width_+1);
		  bits_cnt = obj->width_+1;
	    }
	    bits = bits_tmp;
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
		  if (obj->type == IVL_VT_BOOL)
			obj->type = IVL_VT_LOGIC;
		  bits[idx] = 'x';
		  assert(! net->is_string());
		  break;
		case verinum::Vz:
		  if (obj->type == IVL_VT_BOOL)
			obj->type = IVL_VT_LOGIC;
		  bits[idx] = 'z';
		  assert(! net->is_string());
		  break;
	    }

      if (obj->width_ > sizeof(obj->b.bit_)) {
	    bits[obj->width_] = 0;
	    obj->b.bits_ = net_const_strings.make(bits);
      }

	/* Connect to all the nexus objects. Note that the one-bit
	   case can be handled more efficiently without allocating
	   array space. */

      ivl_drive_t drv0, drv1;
      drive_from_link(net->pin(0), drv0, drv1);
      const Nexus*nex = net->pin(0).nexus();
      assert(nex->t_cookie());
      obj->pin_ = nex->t_cookie();
      nexus_con_add(obj->pin_, obj, 0, drv0, drv1);

      des_.consts.push_back(obj);

      make_const_delays_(obj, net);

      return true;
}

bool dll_target::net_literal(const NetLiteral*net)
{

      struct ivl_net_const_s *obj = new struct ivl_net_const_s;

      obj->type = IVL_VT_REAL;
      assert(net->scope());
      obj->scope = find_scope(des_, net->scope());
      FILE_NAME(obj, net);
      obj->width_  = 1;
      obj->signed_ = 1;
      obj->b.real_value = net->value_real().as_double();

	/* Connect to all the nexus objects. Note that the one-bit
	   case can be handled more efficiently without allocating
	   array space. */

      ivl_drive_t drv0, drv1;
      drive_from_link(net->pin(0), drv0, drv1);
      const Nexus*nex = net->pin(0).nexus();
      assert(nex->t_cookie());
      obj->pin_ = nex->t_cookie();
      nexus_con_add(obj->pin_, obj, 0, drv0, drv1);

      des_.consts.push_back(obj);

      make_const_delays_(obj, net);

      return true;
}

void dll_target::net_probe(const NetEvProbe*)
{
}

void dll_target::scope(const NetScope*net)
{
      if (net->parent() == 0) {

	      // Root scopes are already created...

      } else {
	    perm_string sname = make_scope_name(net->fullname());
	    ivl_scope_t scop = new struct ivl_scope_s;
	    scop->name_ = sname;
	    FILE_NAME(scop, net);
	    scop->parent = find_scope(des_, net->parent());
	    assert(scop->parent);
	    scop->parent->children[net->fullname()] = scop;
	    scop->parent->child .push_back(scop);
	    scop->nlog_ = 0;
	    scop->log_ = 0;
	    scop->nevent_ = 0;
	    scop->event_ = 0;
	    scop->nlpm_ = 0;
	    scop->lpm_ = 0;
	    scop->def = 0;
	    make_scope_parameters(scop, net);
	    scop->time_precision = net->time_precision();
	    scop->time_units = net->time_unit();
	    scop->nattr = net->attr_cnt();
	    scop->attr = fill_in_attributes(net);
	    scop->is_auto = net->is_auto();
	    scop->is_cell = net->is_cell();

	    switch (net->type()) {
		case NetScope::PACKAGE:
		  cerr << "?:?" << ": internal error: "
		       << "Package scopes should not have parents." << endl;
		  // fallthrough
		case NetScope::MODULE:
		  scop->type_ = IVL_SCT_MODULE;
		  scop->tname_ = net->module_name();
		  scop->ports = net->module_port_nets();
		  if (scop->ports > 0) {
			scop->u_.net = new NetNet*[scop->ports];
			for (unsigned idx = 0; idx < scop->ports; idx += 1) {
			      scop->u_.net[idx] = net->module_port_net(idx);
			}
		  }
		  scop->module_ports_info = net->module_port_info();
		  break;

		case NetScope::TASK: {
		      const NetTaskDef*def = net->task_def();
		      if (def == 0) {
			    cerr <<  "?:?" << ": internal error: "
				 << "task " << scop->name_
				 << " has no definition." << endl;
		      }
		      assert(def);
		      scop->type_ = IVL_SCT_TASK;
		      scop->tname_ = def->scope()->basename();
		      break;
		}
		case NetScope::FUNC:
		  fill_in_scope_function(scop, net);
		  break;
		case NetScope::BEGIN_END:
		  scop->type_ = IVL_SCT_BEGIN;
		  scop->tname_ = scop->name_;
		  break;
		case NetScope::FORK_JOIN:
		  scop->type_ = IVL_SCT_FORK;
		  scop->tname_ = scop->name_;
		  break;
		case NetScope::GENBLOCK:
		  scop->type_ = IVL_SCT_GENERATE;
		  scop->tname_ = scop->name_;
		  break;
		case NetScope::CLASS:
		  scop->type_ = IVL_SCT_CLASS;
		  scop->tname_ = scop->name_;
		  break;
	    }
      }
}

void dll_target::convert_module_ports(const NetScope*net)
{
      ivl_scope_t scop = find_scope(des_, net);
      if (scop->ports > 0) {
	    NetNet**nets = scop->u_.net;
	    scop->u_.nex = new ivl_nexus_t[scop->ports];
	    for (unsigned idx = 0; idx < scop->ports; idx += 1) {
		  ivl_signal_t sig = find_signal(des_, nets[idx]);
		  scop->u_.nex[idx] = nexus_sig_make(sig, 0);
	    }
	    delete [] nets;
      }
}

void dll_target::signal(const NetNet*net)
{
      ivl_signal_t obj = new struct ivl_signal_s;

      obj->name_ = net->name();

	/* Attach the signal to the ivl_scope_t object that contains
	   it. This involves growing the sigs_ array in the scope
	   object, or creating the sigs_ array if this is the first
	   signal. */
      obj->scope_ = find_scope(des_, net->scope());
      assert(obj->scope_);
      FILE_NAME(obj, net);

      obj->scope_->sigs_.push_back(obj);


	/* Save the primitive properties of the signal in the
	   ivl_signal_t object. */

      { size_t idx = 0;
	netranges_t::const_iterator cur;
	obj->packed_dims.resize(net->packed_dims().size());
	for (cur = net->packed_dims().begin(), idx = 0
		   ; cur != net->packed_dims().end() ; ++cur, idx += 1) {
	    obj->packed_dims[idx] = *cur;
	}
      }

      obj->net_type = net->net_type();
      obj->local_ = net->local_flag()? 1 : 0;
      obj->forced_net_ = (net->type() != NetNet::REG) &&
                         (net->peek_lref() > 0) ? 1 : 0;
      obj->discipline = net->get_discipline();

      obj->array_dimensions_ = net->unpacked_dimensions();

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

      obj->module_port_index_ = net->get_module_port_index();

      switch (net->type()) {

	  case NetNet::REG:
	    obj->type_ = IVL_SIT_REG;
	    break;

	      /* The SUPPLY0/1 net types are replaced with pulldown/up
		 by elaborate. They should not make it here. */
	  case NetNet::SUPPLY0:
	    assert(0);
	    break;
	  case NetNet::SUPPLY1:
	    assert(0);
	    break;

	      /* We will convert this to a TRI after we check that there
		 is only one driver. */
	  case NetNet::UNRESOLVED_WIRE:
	    obj->type_ = IVL_SIT_UWIRE;
	    break;

	  case NetNet::TRI:
	  case NetNet::WIRE:
	  case NetNet::IMPLICIT:
	    obj->type_ = IVL_SIT_TRI;
	    break;

	  case NetNet::TRI0:
	    obj->type_ = IVL_SIT_TRI0;
	    break;

	  case NetNet::TRI1:
	    obj->type_ = IVL_SIT_TRI1;
	    break;

	  case NetNet::TRIAND:
	  case NetNet::WAND:
	    obj->type_ = IVL_SIT_TRIAND;
	    break;

	  case NetNet::TRIOR:
	  case NetNet::WOR:
	    obj->type_ = IVL_SIT_TRIOR;
	    break;

	  default:
	    obj->type_ = IVL_SIT_NONE;
	    break;
      }

	/* Initialize the path fields to be filled in later. */
      obj->npath = 0;
      obj->path = 0;

      obj->nattr = net->attr_cnt();
      obj->attr = fill_in_attributes(net);

      // Special case: IVL_VT_QUEUE objects don't normally show up in the
      // network,  but can in certain special cases. In these cases, it is the
      // object itself and not the array elements that is in the network. of
      // course, only do this if there is at least one link to this signal.
      if (obj->net_type->base_type()==IVL_VT_QUEUE && net->is_linked()) {
	    const Nexus*nex = net->pin(0).nexus();
	    ivl_nexus_t tmp = nexus_sig_make(obj, 0);
	    tmp->nexus_ = nex;
	    tmp->name_ = 0;
	    nex->t_cookie(tmp);
      }

	/* Get the nexus objects for all the pins of the signal. If
	   the signal has only one pin, then write the single
	   ivl_nexus_t object into n.pin_. Otherwise, make an array of
	   ivl_nexus_t cookies.

	   When I create an ivl_nexus_t object, store it in the
	   t_cookie of the Nexus object so that I find it again when I
	   next encounter the nexus. */

      if (obj->array_dimensions_ == 1) {
	    const netranges_t& dims = net->unpacked_dims();
	    if (dims[0].get_msb() < dims[0].get_lsb()) {
		  obj->array_base = dims[0].get_msb();
		  obj->array_addr_swapped = false;
	    } else {
		  obj->array_base = dims[0].get_lsb();
		  obj->array_addr_swapped = true;
	    }
	    obj->array_words = net->unpacked_count();
      } else {
	      // The back-end API doesn't yet support multi-dimension
	      // unpacked arrays, so just report the canonical dimensions.
	    obj->array_base = 0;
	      // For a queue we pass the maximum queue size as the array words.
	    if (obj->net_type->base_type() == IVL_VT_QUEUE) {
		  long max_size = net->queue_type()->max_idx()+1;
		  ivl_assert(*net, max_size >= 0);
		  obj->array_words = max_size;
	    } else
		  obj->array_words = net->unpacked_count();
	    obj->array_addr_swapped = 0;
      }

      ivl_assert(*net, (obj->array_words == net->pin_count()) ||
                       (obj->net_type->base_type() == IVL_VT_QUEUE));
      if (debug_optimizer && obj->array_words > 1000) cerr << "debug: "
	    "t-dll creating nexus array " << obj->array_words << " long" << endl;
      if (obj->array_words > 1 && net->pins_are_virtual()) {
	    obj->pins = NULL;
	    if (debug_optimizer && obj->array_words > 1000) cerr << "debug: "
		"t-dll used NULL for big nexus array" << endl;
	    return;
      }
      if (obj->array_words > 1)
	    obj->pins = new ivl_nexus_t[obj->array_words];

      for (unsigned idx = 0 ;  idx < obj->array_words ;  idx += 1) {

	    const Nexus*nex = net->pins_are_virtual() ? NULL : net->pin(idx).nexus();
	    if (nex == 0) {
		    // Special case: This pin is connected to
		    // nothing. This can happen, for example, if the
		    // variable is only used in behavioral
		    // code. Create a stub nexus.
		  ivl_nexus_t tmp = nexus_sig_make(obj, idx);
		  tmp->nexus_ = nex;
		  tmp->name_ = 0;
		  if (obj->array_words > 1)
			obj->pins[idx] = tmp;
		  else
			obj->pin = tmp;
	    } else if (nex->t_cookie()) {
		  if (obj->array_words > 1) {
			obj->pins[idx] = nex->t_cookie();
			nexus_sig_add(obj->pins[idx], obj, idx);
		  } else {
			obj->pin = nex->t_cookie();
			nexus_sig_add(obj->pin, obj, idx);
		  }
	    } else {
		  ivl_nexus_t tmp = nexus_sig_make(obj, idx);
		  tmp->nexus_ = nex;
		  tmp->name_ = 0;
		  nex->t_cookie(tmp);
		  if (obj->array_words > 1)
			obj->pins[idx] = tmp;
		  else
			obj->pin = tmp;
	    }
      }
      if (debug_optimizer && obj->array_words > 1000) cerr << "debug: t-dll done with big nexus array" << endl;
}

bool dll_target::signal_paths(const NetNet*net)
{
	/* Nothing to do if there are no paths for this signal. */
      if (net->delay_paths() == 0)
	    return true;

      ivl_signal_t obj = find_signal(des_, net);
      assert(obj);

	/* We cannot have already set up the paths for this signal. */
      assert(obj->npath == 0);
      assert(obj->path == 0);

         /* Figure out how many paths there really are. */
      for (unsigned idx = 0 ;  idx < net->delay_paths() ;  idx += 1) {
	    const NetDelaySrc*src = net->delay_path(idx);
	    obj->npath += src->src_count();
      }

      obj->path = new struct ivl_delaypath_s[obj->npath];

      unsigned ptr = 0;
      for (unsigned idx = 0 ;  idx < net->delay_paths() ;  idx += 1) {
	    const NetDelaySrc*src = net->delay_path(idx);

	      /* If this path has a condition, then hook it up. */
	    ivl_nexus_t path_condit = 0;
	    if (src->has_condit()) {
		  const Nexus*nt = src->condit_pin().nexus();
		  path_condit = nt->t_cookie();
	    }

	    for (unsigned pin = 0; pin < src->src_count(); pin += 1) {
		  const Nexus*nex = src->src_pin(pin).nexus();
		  if (! nex->t_cookie()) {
			cerr << src->get_fileline() << ": internal error: "
			     << "No signal connected to pin " << pin
			     << " of delay path to " << net->name()
			     << "." << endl;
		  }
		  assert(nex->t_cookie());
		  obj->path[ptr].scope = lookup_scope_(src->scope());
		  obj->path[ptr].src = nex->t_cookie();
		  obj->path[ptr].condit = path_condit;
		  obj->path[ptr].conditional = src->is_condit();
		  obj->path[ptr].parallel = src->is_parallel();
		  obj->path[ptr].posedge = src->is_posedge();
		  obj->path[ptr].negedge = src->is_negedge();
		  for (unsigned pe = 0 ;  pe < 12 ;  pe += 1) {
			obj->path[ptr].delay[pe] = src->get_delay(pe);
		  }

		  ptr += 1;
	    }

      }

      return true;
}


void dll_target::test_version(const char*target_name)
{
      dll_ = ivl_dlopen(target_name);

      if ((dll_ == 0) && (target_name[0] != '/')) {
	    size_t len = strlen(basedir) + 1 + strlen(target_name) + 1;
	    char*tmp = new char[len];
	    snprintf(tmp, len, "%s/%s", basedir, target_name);
	    dll_ = ivl_dlopen(tmp);
	    delete[]tmp;
      }

      if (dll_ == 0) {
	    cout << "\n\nUnable to load " << target_name
		 << " for version details." << endl;
	    return;
      }

      target_query_f targ_query = (target_query_f)ivl_dlsym(dll_, LU "target_query" TU);
      if (targ_query == 0) {
	    cerr << "Target " << target_name
		 << " has no version hooks." << endl;
	    return;
      }

      const char*version_string = (*targ_query) ("version");
      if (version_string == 0) {
	    cerr << "Target " << target_name
		 << " has no version string" << endl;
	    return;
      }

      cout << target_name << ": " << version_string << endl;
}
