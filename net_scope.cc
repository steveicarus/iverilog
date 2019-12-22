/*
 * Copyright (c) 2000-2019 Stephen Williams (steve@icarus.com)
 * Copyright (c) 2016 CERN Michele Castellana (michele.castellana@cern.ch)
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
# include "compiler.h"

# include  "netlist.h"
# include  "netclass.h"
# include  "netenum.h"
# include  "netvector.h"
# include  "PPackage.h"
# include  <cstring>
# include  <cstdlib>
# include  <sstream>
# include  "ivl_assert.h"

class PExpr;

Definitions::Definitions()
{
}

Definitions::~Definitions()
{
}

void Definitions::add_enumeration_set(const enum_type_t*key, netenum_t*enum_set)
{
      netenum_t*&tmp = enum_sets_[key];
      assert(tmp == 0);
      tmp = enum_set;
}

bool Definitions::add_enumeration_name(netenum_t*enum_set, perm_string name)
{
      netenum_t::iterator enum_val = enum_set->find_name(name);
      assert(enum_val != enum_set->end_name());

      NetEConstEnum*val = new NetEConstEnum(this, name, enum_set, enum_val->second);

      pair<map<perm_string,NetEConstEnum*>::iterator, bool> cur;
      cur = enum_names_.insert(make_pair(name,val));

	// Return TRUE if the name is added (i.e. is NOT a duplicate.)
      return cur.second;
}

netenum_t* Definitions::enumeration_for_key(const enum_type_t*key) const
{
      map<const enum_type_t*,netenum_t*>::const_iterator cur;

      cur = enum_sets_.find(key);
      if (cur != enum_sets_.end())
	    return cur->second;
      else
	    return 0;
}

/*
 * This locates the enumeration TYPE for the given enumeration literal.
 */
const netenum_t*Definitions::enumeration_for_name(perm_string name)
{
      NetEConstEnum*tmp = enum_names_[name];
      assert(tmp != 0);

      return tmp->enumeration();
}

/*
 * This locates the VALUE for the given enumeration literal.
 */
const NetExpr* Definitions::enumeration_expr(perm_string key)
{
      map<perm_string,NetEConstEnum*>::const_iterator eidx;

      eidx = enum_names_.find(key);
      if (eidx != enum_names_.end()) {
	    return eidx->second;
      } else {
	    return 0;
      }
}

void Definitions::add_class(netclass_t*net_class)
{
      classes_[net_class->get_name()] = net_class;
}

/*
 * The NetScope class keeps a scope tree organized. Each node of the
 * scope tree points to its parent, its right sibling and its leftmost
 * child. The root node has no parent or siblings. The node stores the
 * name of the scope. The complete hierarchical name of the scope is
 * formed by appending the path of scopes from the root to the scope
 * in question.
 */

NetScope::NetScope(NetScope*up, const hname_t&n, NetScope::TYPE t, NetScope*in_unit,
		   bool nest, bool program, bool interface, bool compilation_unit)
: type_(t), name_(n), nested_module_(nest), program_block_(program),
  is_interface_(interface), is_unit_(compilation_unit), unit_(in_unit), up_(up)
{
      imports_ = 0;
      typedefs_ = 0;
      events_ = 0;
      lcounter_ = 0;
      is_auto_ = false;
      is_cell_ = false;
      calls_stask_ = false;
      in_final_ = false;

      if (compilation_unit)
	    unit_ = this;

      if (up) {
	    need_const_func_ = up->need_const_func_;
	    is_const_func_ = up->is_const_func_;
	    time_unit_ = up->time_unit();
	    time_prec_ = up->time_precision();
	    time_from_timescale_ = up->time_from_timescale();
	      // Need to check for duplicate names?
	    up_->children_[name_] = this;
	    if (unit_ == 0)
		  unit_ = up_->unit_;
      } else {
	    need_const_func_ = false;
	    is_const_func_ = false;
	    time_unit_ = 0;
	    time_prec_ = 0;
	    time_from_timescale_ = false;
      }

      var_init_ = 0;
      switch (t) {
	  case NetScope::TASK:
	    task_ = 0;
	    break;
	  case NetScope::FUNC:
	    func_ = 0;
	    break;
	  case NetScope::MODULE:
	  case NetScope::PACKAGE:
	    module_name_ = perm_string();
	    break;
	  case NetScope::CLASS:
	    class_def_ = 0;
	    break;
	  default:  /* BEGIN_END and FORK_JOIN, do nothing */
	    break;
      }
      func_pform_ = 0;
      elab_stage_ = 1;
      lineno_ = 0;
      def_lineno_ = 0;
      genvar_tmp_val = 0;
      tie_hi_ = 0;
      tie_lo_ = 0;
}

NetScope::~NetScope()
{
      lcounter_ = 0;

	/* name_ and module_name_ are perm-allocated. */
}

void NetScope::set_line(const LineInfo*info)
{
      file_ = info->get_file();
      def_file_ = file_;
      lineno_ = info->get_lineno();
      def_lineno_ = lineno_;
}

void NetScope::set_line(perm_string file, unsigned lineno)
{
      file_ = file;
      def_file_ = file;
      lineno_ = lineno;
      def_lineno_ = lineno;
}

void NetScope::set_line(perm_string file, perm_string def_file,
                        unsigned lineno, unsigned def_lineno)
{
      file_ = file;
      def_file_ = def_file;
      lineno_ = lineno;
      def_lineno_ = def_lineno;
}

void NetScope::add_imports(const map<perm_string,PPackage*>*imports)
{
      if (!imports->empty())
	    imports_ = imports;
}

NetScope*NetScope::find_import(const Design*des, perm_string name)
{
      if (imports_ == 0)
	    return 0;

      map<perm_string,PPackage*>::const_iterator cur = imports_->find(name);
      if (cur != imports_->end()) {
            return des->find_package(cur->second->pscope_name());
      } else
            return 0;
}

void NetScope::add_typedefs(const map<perm_string,data_type_t*>*typedefs)
{
      if (!typedefs->empty())
	    typedefs_ = typedefs;
}

NetScope*NetScope::find_typedef_scope(const Design*des, data_type_t*type)
{
      assert(type);

      NetScope *cur_scope = this;
      while (cur_scope) {
	    if (cur_scope->typedefs_ && cur_scope->typedefs_->find(type->name) != cur_scope->typedefs_->end())
		  return cur_scope;
	    NetScope*import_scope = cur_scope->find_import(des, type->name);
	    if (import_scope)
		  cur_scope = import_scope;
	    else if (cur_scope == unit_)
		  return 0;
	    else
		  cur_scope = cur_scope->parent();

	    if (cur_scope == 0)
		  cur_scope = unit_;
      }

      return 0;
}

/*
 * Look for the enumeration in the current scope and any parent scopes.
 */
const netenum_t*NetScope::find_enumeration_for_name(const Design*des, perm_string name)
{
      NetScope *cur_scope = this;
      while (cur_scope) {
	    NetEConstEnum*tmp = cur_scope->enum_names_[name];
	    if (tmp) break;
	    NetScope*import_scope = cur_scope->find_import(des, name);
	    if (import_scope)
		  cur_scope = import_scope;
	    else if (cur_scope == unit_)
		  return 0;
	    else
		  cur_scope = cur_scope->parent();

	    if (cur_scope == 0)
		  cur_scope = unit_;
      }

      assert(cur_scope);
      return cur_scope->enum_names_[name]->enumeration();
}

void NetScope::set_parameter(perm_string key, bool is_annotatable,
			     PExpr*val, ivl_variable_type_t type__,
			     PExpr*msb, PExpr*lsb, bool signed_flag,
			     bool local_flag,
			     NetScope::range_t*range_list,
			     const LineInfo&file_line)
{
      param_expr_t&ref = parameters[key];
      ref.is_annotatable = is_annotatable;
      ref.msb_expr = msb;
      ref.lsb_expr = lsb;
      ref.val_expr = val;
      ref.val_scope = this;
      ref.type = type__;
      ref.msb = 0;
      ref.lsb = 0;
      ref.signed_flag = signed_flag;
      ref.local_flag = local_flag;
      ivl_assert(file_line, ref.range == 0);
      ref.range = range_list;
      ref.val = 0;
      ref.set_line(file_line);
}

/*
 * This is a simplified version of set_parameter, for use when the
 * parameter value is already known. It is currently only used to
 * add a genvar to the parameter list.
 */
void NetScope::set_parameter(perm_string key, NetExpr*val,
			     const LineInfo&file_line)
{
      param_expr_t&ref = parameters[key];
      ref.is_annotatable = false;
      ref.msb_expr = 0;
      ref.lsb_expr = 0;
      ref.val_expr = 0;
      ref.val_scope = this;
      ref.type = IVL_VT_BOOL;
      ref.msb = 0;
      ref.lsb = 0;
      ref.signed_flag = false;
      ref.val = val;
      ref.set_line(file_line);
}

bool NetScope::auto_name(const char*prefix, char pad, const char* suffix)
{
	// Find the current reference to myself in the parent scope.
      map<hname_t,NetScope*>::iterator self = up_->children_.find(name_);
      assert(self != up_->children_.end());
      assert(self->second == this);

	// This is to keep the pad attempts from being stuck in some
	// sort of infinite loop. This should not be a practical
	// limit, but an extreme one.
      const size_t max_pad_attempts = 32 + strlen(prefix);

      string use_prefix = prefix;

	// Try a variety of potential new names. Make sure the new
	// name is not in the parent scope. Keep looking until we are
	// sure we have a unique name, or we run out of names to try.
      while (use_prefix.size() <= max_pad_attempts) {
	      // Try this name...
	    string tmp = use_prefix + suffix;
	    hname_t new_name(lex_strings.make(tmp.c_str()), name_.peek_numbers());
	    if (!up_->child(new_name)) {
		    // Ah, this name is unique. Rename myself, and
		    // change my name in the parent scope.
		  name_ = new_name;
		  up_->children_.erase(self);
		  up_->children_[name_] = this;
		  return true;
	    }

	      // Name collides, so try a different name.
	    use_prefix = use_prefix + pad;
      }
      return false;
}

/*
 * Return false if the parameter does not already exist.
 * A parameter is not automatically created.
 */
bool NetScope::replace_parameter(perm_string key, PExpr*val, NetScope*scope)
{
      if (parameters.find(key) == parameters.end())
	    return false;

      param_expr_t&ref = parameters[key];
      if (ref.local_flag)
	    return false;

      ref.val_expr = val;
      ref.val_scope = scope;
      return true;
}

bool NetScope::make_parameter_unannotatable(perm_string key)
{
      bool flag = false;

      if (parameters.find(key) != parameters.end()) {
	    param_expr_t&ref = parameters[key];
	    flag = ref.is_annotatable;
	    ref.is_annotatable = false;
      }

      return flag;
}

/*
 * NOTE: This method takes a const char* as a key to lookup a
 * parameter, because we don't save that pointer. However, due to the
 * way the map<> template works, we need to *cheat* and use the
 * perm_string::literal method to fake the compiler into doing the
 * compare without actually creating a perm_string.
 */
const NetExpr* NetScope::get_parameter(Design*des,
				       const char* key,
				       const NetExpr*&msb,
				       const NetExpr*&lsb)
{
      return get_parameter(des, perm_string::literal(key), msb, lsb);
}

const NetExpr* NetScope::get_parameter(Design*des,
				       perm_string key,
				       const NetExpr*&msb,
				       const NetExpr*&lsb)
{
      map<perm_string,param_expr_t>::iterator idx;

      idx = parameters.find(key);
      if (idx != parameters.end()) {
            if (idx->second.val_expr)
                  evaluate_parameter_(des, idx);

	    msb = idx->second.msb;
	    lsb = idx->second.lsb;
	    return idx->second.val;
      }

      msb = 0;
      lsb = 0;
      const NetExpr*tmp = enumeration_expr(key);
      return tmp;
}

NetScope::param_ref_t NetScope::find_parameter(perm_string key)
{
      map<perm_string,param_expr_t>::iterator idx;

      idx = parameters.find(key);
      if (idx != parameters.end()) return idx;

	// To get here the parameter must already exist, so we should
	// never get here.
      assert(0);
	// But return something to avoid a compiler warning.
      return idx;
}

void NetScope::print_type(ostream&stream) const
{
      switch (type_) {
	case BEGIN_END:
	    stream << "sequential block";
	    break;
	case FORK_JOIN:
	    stream << "parallel block";
	    break;
	case FUNC:
	    stream << "function";
	    break;
	case MODULE:
	    stream << "module <" << module_name_ << "> instance";
	    break;
	case TASK:
	    stream << "task";
	    break;
	case GENBLOCK:
	    stream << "generate block";
	    break;
	case PACKAGE:
	    stream << "package " << module_name_;
	    break;
	case CLASS:
	    stream << "class";
	    break;
      }
}

void NetScope::set_task_def(NetTaskDef*def)
{
      assert( type_ == TASK );
      assert( task_ == 0 );
      task_ = def;
}

NetTaskDef* NetScope::task_def()
{
      assert( type_ == TASK );
      return task_;
}

const NetTaskDef* NetScope::task_def() const
{
      assert( type_ == TASK );
      return task_;
}

void NetScope::set_func_def(NetFuncDef*def)
{
      assert( type_ == FUNC );
      assert( func_ == 0 );
      func_ = def;
}

NetFuncDef* NetScope::func_def()
{
      assert( type_ == FUNC );
      return func_;
}

bool NetScope::in_func() const
{
      return (type_ == FUNC) ? true : false;
}

const NetFuncDef* NetScope::func_def() const
{
      assert( type_ == FUNC );
      return func_;
}

void NetScope::set_class_def(netclass_t*def)
{
      assert( type_ == CLASS );
      assert( class_def_==0  );
      class_def_ = def;
}

const netclass_t* NetScope::class_def(void) const
{
      if (type_==CLASS)
	    return class_def_;
      else
	    return 0;
}

void NetScope::set_module_name(perm_string n)
{
      assert(type_==MODULE || type_==PACKAGE);
      module_name_ = n;
}

perm_string NetScope::module_name() const
{
      assert(type_==MODULE || type_==PACKAGE);
      return module_name_;
}

void NetScope::set_num_ports(unsigned int num_ports)
{
    assert(type_ == MODULE);
    assert(ports_.empty());
    ports_.resize( num_ports );
}

void NetScope::add_module_port_net(NetNet*subport)
{
      assert(type_ == MODULE);
      port_nets.push_back(subport);
}


void NetScope::add_module_port_info( unsigned idx, perm_string name, PortType::Enum ptype,
                                unsigned long width )
{
      assert(type_ == MODULE);
      assert(ports_.size() > idx);
      PortInfo &info = ports_[idx];
      info.name = name;
      info.type = ptype;
      info.width = width;
}


unsigned NetScope::module_port_nets() const
{
      assert(type_ == MODULE);
      return port_nets.size();
}


const std::vector<PortInfo> & NetScope::module_port_info() const
{
      assert(type_ == MODULE);
      return ports_;
}



NetNet* NetScope::module_port_net(unsigned idx) const
{
      assert(type_ == MODULE);
      assert(idx < port_nets.size());
      return port_nets[idx];
}

void NetScope::time_unit(int val)
{
      time_unit_ = val;
}

void NetScope::time_precision(int val)
{
      time_prec_ = val;
}

void NetScope::time_from_timescale(bool val)
{
      time_from_timescale_ = val;
}

int NetScope::time_unit() const
{
      return time_unit_;
}

int NetScope::time_precision() const
{
      return time_prec_;
}

bool NetScope::time_from_timescale() const
{
      return time_from_timescale_;
}

perm_string NetScope::basename() const
{
      return name_.peek_name();
}

void NetScope::add_event(NetEvent*ev)
{
      assert(ev->scope_ == 0);
      ev->scope_ = this;
      ev->snext_ = events_;
      events_ = ev;
}

void NetScope::rem_event(NetEvent*ev)
{
      assert(ev->scope_ == this);
      ev->scope_ = 0;
      if (events_ == ev) {
	    events_ = ev->snext_;

      } else {
	    NetEvent*cur = events_;
	    while (cur->snext_ != ev) {
		  assert(cur->snext_);
		  cur = cur->snext_;
	    }
	    cur->snext_ = ev->snext_;
      }

      ev->snext_ = 0;
}


NetEvent* NetScope::find_event(perm_string name)
{
      for (NetEvent*cur = events_;  cur ;  cur = cur->snext_)
	    if (cur->name() == name)
		  return cur;

      return 0;
}

void NetScope::add_genvar(perm_string name, LineInfo *li)
{
      assert((type_ == MODULE) || (type_ == GENBLOCK));
      genvars_[name] = li;
}

LineInfo* NetScope::find_genvar(perm_string name)
{
      if (genvars_.find(name) != genvars_.end())
	    return genvars_[name];
      else
            return 0;
}

void NetScope::add_signal(NetNet*net)
{
      signals_map_[net->name()]=net;
}

void NetScope::rem_signal(NetNet*net)
{
      assert(net->scope() == this);
      signals_map_.erase(net->name());
}

/*
 * This method looks for a signal within the current scope. The name
 * is assumed to be the base name of the signal, so no sub-scopes are
 * searched.
 */
NetNet* NetScope::find_signal(perm_string key)
{
      if (signals_map_.find(key)!=signals_map_.end())
	    return signals_map_[key];
      else
	    return 0;
}

netclass_t*NetScope::find_class(perm_string name)
{
	// Special case: The scope itself is the class that we are
	// looking for. This may happen for example when elaborating
	// methods within the class.
      if (type_==CLASS && name_==hname_t(name))
	    return class_def_;

	// Look for the class directly within this scope.
      map<perm_string,netclass_t*>::const_iterator cur = classes_.find(name);
      if (cur != classes_.end())
	    return cur->second;

      if (up_==0 && type_==CLASS) {
	    assert(class_def_);

	    NetScope*def_parent = class_def_->definition_scope();
	    return def_parent->find_class(name);
      }

	// Try looking up for the class.
      if (up_!=0 && type_!=MODULE)
	    return up_->find_class(name);

	// Try the compilation unit.
      if (unit_ != 0)
	    return unit_->find_class(name);

	// Nowhere left to try...
      return 0;
}

/*
 * This method locates a child scope by name. The name is the simple
 * name of the child, no hierarchy is searched.
 */
NetScope* NetScope::child(const hname_t&name)
{
      map<hname_t,NetScope*>::iterator cur = children_.find(name);
      if (cur == children_.end())
	    return 0;
      else
	    return cur->second;
}

const NetScope* NetScope::child(const hname_t&name) const
{
      map<hname_t,NetScope*>::const_iterator cur = children_.find(name);
      if (cur == children_.end())
	    return 0;
      else
	    return cur->second;
}

/* Helper function to see if the given scope is defined in a class and if
 * so return the class scope. */
const NetScope* NetScope::get_class_scope() const
{
      const NetScope*scope = this;
      while (scope) {
	    switch(scope->type()) {
		case NetScope::CLASS:
		  return scope;
		case NetScope::TASK:
		case NetScope::FUNC:
		case NetScope::BEGIN_END:
		case NetScope::FORK_JOIN:
		  break;
		case NetScope::MODULE:
		case NetScope::GENBLOCK:
		case NetScope::PACKAGE:
		  return 0;
		default:
		  assert(0);
	    }
	    scope = scope->parent();
      }
      return scope;
}

const NetScope* NetScope::child_byname(perm_string name) const
{
      hname_t hname (name);
      map<hname_t,NetScope*>::const_iterator cur = children_.lower_bound(hname);

      if (cur == children_.end())
	    return 0;

      if (cur->first.peek_name() == name)
	    return cur->second;

      return 0;
}


perm_string NetScope::local_symbol()
{
      perm_string sym;
      do {
	    ostringstream res;
	    res << "_ivl_" << (lcounter_++);
	    perm_string sym_tmp = lex_strings.make(res.str());

	      // If the name already exists as a signal, try again.
	    if (signals_map_.find(sym_tmp) != signals_map_.end())
		  continue;
	      // If the name already exists as a parameter, try again.
	    if (parameters.find(sym_tmp) != parameters.end())
		  continue;
	    if (genvars_.find(sym_tmp) != genvars_.end())
		  continue;
	      // If the name already exists as a class, try again.
	    if (classes_.find(sym_tmp) != classes_.end())
		  continue;

	      // No collisions, this is the one.
	    sym = sym_tmp;
      } while (sym.nil());
      return sym;
}

void NetScope::add_tie_hi(Design*des)
{
      if (tie_hi_ == 0) {
	    NetNet*sig = new NetNet(this, lex_strings.make("_LOGIC1"),
				    NetNet::WIRE, &netvector_t::scalar_logic);
	    sig->local_flag(true);

	    tie_hi_ = new NetLogic(this, local_symbol(),
				   1, NetLogic::PULLUP, 1);
	    des->add_node(tie_hi_);

	    connect(sig->pin(0), tie_hi_->pin(0));
      }
}

void NetScope::add_tie_lo(Design*des)
{
      if (tie_lo_ == 0) {
	    NetNet*sig = new NetNet(this, lex_strings.make("_LOGIC0"),
				    NetNet::WIRE, &netvector_t::scalar_logic);
	    sig->local_flag(true);

	    tie_lo_ = new NetLogic(this, local_symbol(),
				   1, NetLogic::PULLDOWN, 1);
	    des->add_node(tie_lo_);

	    connect(sig->pin(0), tie_lo_->pin(0));
      }
}
