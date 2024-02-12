/*
 * Copyright (c) 2000-2024 Stephen Williams (steve@icarus.com)
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
# include  "PExpr.h"
# include  "PPackage.h"
# include  "PWire.h"
# include  <cstring>
# include  <cstdlib>
# include  <sstream>
# include  "ivl_assert.h"

using namespace std;

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

      NetEConstEnum*val = new NetEConstEnum(name, enum_set, enum_val->second);

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

string NetScope::get_fileline() const
{
      ostringstream buf;
      buf << (file_? file_ : "") << ":" << lineno_;
      string res = buf.str();
      return res;
}

string NetScope::get_def_fileline() const
{
      ostringstream buf;
      buf << (def_file_? def_file_ : "") << ":" << def_lineno_;
      string res = buf.str();
      return res;
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

void NetScope::add_typedefs(const map<perm_string,typedef_t*>*typedefs)
{
      if (!typedefs->empty())
	    typedefs_ = *typedefs;
}

NetScope*NetScope::find_typedef_scope(const Design*des, const typedef_t*type)
{
      ivl_assert(*this, type);

      NetScope *cur_scope = this;
      while (cur_scope) {
	    auto it = cur_scope->typedefs_.find(type->name);
	    if (it != cur_scope->typedefs_.end() && it->second == type)
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
 * Attach to the a parameter name in the scope a value and a type. The value
 * (val_expr) is the PExpr form that is not yet elaborated. Later, when
 * elaboration happens, the val_expr is elaborated and written to the val
 * member.
 */
void NetScope::set_parameter(perm_string key, bool is_annotatable,
			     const LexicalScope::param_expr_t &param,
			     NetScope::range_t *range_list)
{
      param_expr_t&ref = parameters[key];
      ref.is_annotatable = is_annotatable;
      ref.val_expr = param.expr;
      ref.val_type = param.data_type;
      ref.val_scope = this;
      ref.local_flag = param.local_flag;
      ref.overridable = param.overridable;
      ref.type_flag = param.type_flag;
      ref.lexical_pos = param.lexical_pos;
      ivl_assert(param, !ref.range);
      ref.range = range_list;
      ref.val = 0;
      ref.ivl_type = 0;
      ref.set_line(param);
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
      ref.val_expr = 0;
      ref.val_type = 0;
      ref.val_scope = this;
      ref.ivl_type = netvector_t::integer_type();
      ivl_assert(file_line, ref.ivl_type);
      ref.val = val;
      ref.set_line(file_line);
}

bool NetScope::auto_name(const char*prefix, char pad, const char* suffix)
{
	// Find the current reference to myself in the parent scope.
      map<hname_t,NetScope*>::iterator self = up_->children_.find(name_);
      ivl_assert(*this, self != up_->children_.end());
      ivl_assert(*this, self->second == this);

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
	    perm_string base_name = lex_strings.make(tmp.c_str());
	    hname_t new_name(base_name, name_.peek_numbers());
	    if (!up_->child(new_name) && !up_->symbol_exists(base_name)) {
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
void NetScope::replace_parameter(Design *des, perm_string key, PExpr*val,
			         NetScope*scope, bool defparam)
{
      if (parameters.find(key) == parameters.end()) {
	    cerr << val->get_fileline() << ": error: parameter `"
	         << key << "` not found in `"
	         << scope_path(this) << "`." << endl;
	    des->errors++;
	    return;
      }

      param_expr_t&ref = parameters[key];
      if (ref.local_flag) {
	    cerr << val->get_fileline() << ": error: "
	         << "Cannot override localparam `" << key << "` in `"
	         << scope_path(this) << "`." << endl;
	    des->errors++;
	    return;
      }
      if (!ref.overridable) {
	    cerr << val->get_fileline() << ": error: "
		 << "Cannot override parameter `" << key << "` in `"
		 << scope_path(this) << "`. Parameter cannot be overridden "
		 << "in the scope it has been declared in."
		 << endl;
	    des->errors++;
	    return;
      }

      if (ref.type_flag && defparam) {
	    cerr << val->get_fileline() << ": error: "
		 << "Cannot override type parameter `" << key << "` in `"
		 << scope_path(this) << "`. It is not allowed to override type"
		 << " parameters using a defparam statement."
		 << endl;
	    des->errors++;
	    return;
      }

      ref.val_expr = val;
      ref.val_scope = scope;
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
const NetExpr* NetScope::get_parameter(Design*des, const char* key,
				       ivl_type_t&ivl_type)
{
      return get_parameter(des, perm_string::literal(key), ivl_type);
}

const NetExpr* NetScope::get_parameter(Design*des, perm_string key,
				       ivl_type_t&ivl_type)
{
      map<perm_string,param_expr_t>::iterator idx;

      idx = parameters.find(key);
      if (idx != parameters.end()) {
            if (idx->second.val_expr)
                  evaluate_parameter_(des, idx);

	    ivl_type = idx->second.ivl_type;
	    return idx->second.val;
      }

      ivl_type = 0;
      const NetExpr*tmp = enumeration_expr(key);
      return tmp;
}

LineInfo NetScope::get_parameter_line_info(perm_string key) const
{
      map<perm_string,param_expr_t>::const_iterator idx;

      idx = parameters.find(key);
      if (idx != parameters.end()) return idx->second;

	// To get here the parameter must already exist, so we should
	// never get here.
      assert(0);
	// But return something to avoid a compiler warning.
      return LineInfo();
}

unsigned NetScope::get_parameter_lexical_pos(perm_string key) const
{
      map<perm_string,param_expr_t>::const_iterator idx;

      idx = parameters.find(key);
      if (idx != parameters.end()) return idx->second.lexical_pos;

	// If we get here, assume an enumeration value.
      return 0;
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
      ivl_assert(*this, type_ == TASK);
      ivl_assert(*this, task_ == nullptr);
      task_ = def;
}

NetTaskDef* NetScope::task_def()
{
      ivl_assert(*this, type_ == TASK);
      return task_;
}

const NetTaskDef* NetScope::task_def() const
{
      ivl_assert(*this, type_ == TASK);
      return task_;
}

void NetScope::set_func_def(NetFuncDef*def)
{
      ivl_assert(*this, type_ == FUNC);
      ivl_assert(*this, func_ == nullptr);
      func_ = def;
}

NetFuncDef* NetScope::func_def()
{
      ivl_assert(*this, type_ == FUNC);
      return func_;
}

bool NetScope::in_func() const
{
      return (type_ == FUNC) ? true : false;
}

const NetFuncDef* NetScope::func_def() const
{
      ivl_assert(*this, type_ == FUNC);
      return func_;
}

void NetScope::set_class_def(netclass_t*def)
{
      ivl_assert(*this, type_ == CLASS);
      ivl_assert(*this, class_def_ == nullptr);
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
      ivl_assert(*this, type_==MODULE || type_==PACKAGE);
      module_name_ = n;
}

perm_string NetScope::module_name() const
{
      ivl_assert(*this, type_==MODULE || type_==PACKAGE);
      return module_name_;
}

void NetScope::set_num_ports(unsigned int num_ports)
{
    ivl_assert(*this, type_ == MODULE);
    ivl_assert(*this, ports_.empty());
    ports_.resize( num_ports );
}

void NetScope::add_module_port_net(NetNet*subport)
{
      ivl_assert(*this, type_ == MODULE);
      port_nets.push_back(subport);
}


void NetScope::add_module_port_info( unsigned idx, perm_string name, PortType::Enum ptype,
                                unsigned long width )
{
      ivl_assert(*this, type_ == MODULE);
      ivl_assert(*this, ports_.size() > idx);
      PortInfo &info = ports_[idx];
      info.name = name;
      info.type = ptype;
      info.width = width;
      info.buffer = nullptr;
}

PortInfo* NetScope::get_module_port_info( unsigned idx )
{
      ivl_assert(*this, type_ == MODULE);
      ivl_assert(*this, ports_.size() > idx);
      return &ports_[idx];
}

unsigned NetScope::module_port_nets() const
{
      ivl_assert(*this, type_ == MODULE);
      return port_nets.size();
}


const std::vector<PortInfo> & NetScope::module_port_info() const
{
      ivl_assert(*this, type_ == MODULE);
      return ports_;
}



NetNet* NetScope::module_port_net(unsigned idx) const
{
      ivl_assert(*this, type_ == MODULE);
      ivl_assert(*this, idx < port_nets.size());
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
      ivl_assert(*this, ev->scope_ == nullptr);
      ev->scope_ = this;
      ev->snext_ = events_;
      events_ = ev;
}

void NetScope::rem_event(NetEvent*ev)
{
      ivl_assert(*this, ev->scope_ == this);
      ev->scope_ = 0;
      if (events_ == ev) {
	    events_ = ev->snext_;

      } else {
	    NetEvent*cur = events_;
	    while (cur->snext_ != ev) {
		  ivl_assert(*this, cur->snext_);
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
      ivl_assert(*li, (type_ == MODULE) || (type_ == GENBLOCK));
      genvars_[name] = li;
}

LineInfo* NetScope::find_genvar(perm_string name)
{
      if (genvars_.find(name) != genvars_.end())
	    return genvars_[name];
      else
            return 0;
}

void NetScope::add_signal_placeholder(PWire*wire)
{
      signal_placeholders_[wire->basename()] = wire;
}

void NetScope::rem_signal_placeholder(PWire*wire)
{
      signal_placeholders_.erase(wire->basename());
}

PWire* NetScope::find_signal_placeholder(perm_string name)
{
      if (signal_placeholders_.find(name) != signal_placeholders_.end())
	    return signal_placeholders_[name];
      else
	    return 0;
}

void NetScope::add_signal(NetNet*net)
{
      signals_map_[net->name()]=net;
}

void NetScope::rem_signal(NetNet*net)
{
      ivl_assert(*this, net->scope() == this);
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

netclass_t*NetScope::find_class(const Design*des, perm_string name)
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

        // Try the imports.
      NetScope*import_scope = find_import(des, name);
      if (import_scope)
            return import_scope->find_class(des, name);

      if (up_==0 && type_==CLASS) {
	    ivl_assert(*this, class_def_);

	    NetScope*def_parent = class_def_->definition_scope();
	    return def_parent->find_class(des, name);
      }

	// Try looking up for the class.
      if (up_!=0 && type_!=MODULE)
	    return up_->find_class(des, name);

	// Try the compilation unit.
      if (unit_ != 0 && this != unit_)
	    return unit_->find_class(des, name);

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
		  ivl_assert(*this, 0);
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


bool NetScope::symbol_exists(perm_string sym)
{
      if (signals_map_.find(sym) != signals_map_.end())
            return true;
      if (parameters.find(sym) != parameters.end())
            return true;
      if (genvars_.find(sym) != genvars_.end())
            return true;
      if (classes_.find(sym) != classes_.end())
          return true;
      if (typedefs_.find(sym) != typedefs_.end())
          return true;
      if (find_event(sym))
          return true;

      return false;
}

perm_string NetScope::local_symbol()
{
      perm_string sym;
      do {
	    ostringstream res;
	    res << "_ivl_" << (lcounter_++);
	    perm_string sym_tmp = lex_strings.make(res.str());

	      // If the name already exists, try again.
	    if (symbol_exists(sym_tmp))
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
