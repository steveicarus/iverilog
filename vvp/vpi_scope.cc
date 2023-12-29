/*
 * Copyright (c) 2001-2023 Stephen Williams (steve@icarus.com)
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

# include  "compile.h"
# include  "vpi_priv.h"
# include  "symbols.h"
# include  "statistics.h"
# include  "config.h"
#ifdef CHECK_WITH_VALGRIND
# include  "vvp_cleanup.h"
#endif
# include  <vector>
# include  <cstring>
# include  <cstdlib>
# include  <cassert>
# include  "ivl_alloc.h"

using namespace std;

static vector<vpiHandle> vpip_root_table;

static vpiHandle make_subset_iterator_(int type_code, vector<vpiHandle>&table);

vpiHandle vpip_make_root_iterator(int type_code)
{
      return make_subset_iterator_(type_code, vpip_root_table);
}

void vpip_make_root_iterator(__vpiHandle**&table, unsigned&ntable)
{
      table = &vpip_root_table[0];
      ntable = vpip_root_table.size();
}

#ifdef CHECK_WITH_VALGRIND
void port_delete(__vpiHandle*handle);
void port_bit_delete(__vpiHandle*handle);

/* Class definitions need to be cleaned up at the end. */
static class_type **class_list = 0;
static unsigned class_list_count = 0;

static void delete_sub_scopes(__vpiScope *scope)
{
      for (unsigned idx = 0; idx < scope->intern.size(); idx += 1) {
	    vpiHandle item = (scope->intern)[idx];
	    __vpiScope*lscope = static_cast<__vpiScope*>(item);
	    switch(item->get_type_code()) {
		case vpiFunction:
		case vpiTask:
		  contexts_delete(lscope);
		  // fallthrough
		case vpiModule:
		case vpiGenScope:
		case vpiNamedBegin:
		case vpiNamedFork:
		case vpiClassTypespec:
		  delete_sub_scopes(lscope);
		  vthreads_delete(lscope);
		  delete item;
		  break;
		case vpiMemory:
		case vpiNetArray:
		  memory_delete(item);
		  break;
		case vpiModPath:
		    /* The destination ModPath is cleaned up later. */
		  delete item;
		  break;
		case vpiInterModPath:
		  delete item;
		  break;
		case vpiNamedEvent:
		  named_event_delete(item);
		  break;
		case vpiNet:
		case vpiReg:
		case vpiIntegerVar:
		case vpiLongIntVar:
		case vpiShortIntVar:
		case vpiIntVar:
		case vpiByteVar:
		case vpiBitVar:
		  signal_delete(item);
		  break;
		case vpiParameter:
		  parameter_delete(item);
		  break;
		case vpiRealVar:
		  real_delete(item);
		  break;
		case vpiEnumTypespec:
		  enum_delete(item);
		  break;
		case vpiPort:
		  port_delete(item);
		  break;
		case vpiPortBit:
		  port_bit_delete(item);
		  break;
		case vpiStringVar:
		  string_delete(item);
		  break;
		case vpiClassVar:
		  class_delete(item);
		  break;
		case vpiArrayVar:
		  switch(item->vpi_get(vpiArrayType)) {
		      case vpiQueueArray:
			queue_delete(item);
			break;
		      case vpiDynamicArray:
			darray_delete(item);
			break;
		      default:
			fprintf(stderr, "Need support for array type: %d\n",
			                item->vpi_get(vpiArrayType));
			assert(0);
			break;
		  }
		  break;
		default:
		  fprintf(stderr, "Need support for type: %d\n",
		          item->get_type_code());
		  assert(0);
		  break;
	    }
      }
      scope->intern.clear();

	/* Save any class definitions to clean up later. */
      map<std::string, class_type*>::iterator citer;
      for (citer = scope->classes.begin();
           citer != scope->classes.end(); ++ citer ) {
	    class_list_count += 1;
	    class_list = (class_type **) realloc(class_list,
	                 class_list_count*sizeof(class_type **));
	    class_list[class_list_count-1] = citer->second;
      }
}

void root_table_delete(void)
{
      for (unsigned idx = 0; idx < vpip_root_table.size(); idx += 1) {
	    __vpiScope *scope = static_cast<__vpiScope *>
	          (vpip_root_table[idx]);
	    vthreads_delete(scope);
	    delete_sub_scopes(scope);
	    delete scope;
      }
      vpip_root_table.clear();

	/* Clean up all the class definitions. */
      for (unsigned idx = 0; idx < class_list_count; idx += 1) {
            class_def_delete(class_list[idx]);
      }
      free(class_list);
      class_list = 0;
      class_list_count = 0;
}
#endif

static void construct_scope_fullname(__vpiScope*ref, char*buf)
{
      if (ref->scope) {
	    construct_scope_fullname(ref->scope, buf);
	      // Add a "." separator, unless this is for a package
	    if (ref->scope->get_type_code() != vpiPackage) {
		  strcat(buf, ".");
	    }
      }

      strcat(buf, ref->scope_name());
	// For a package, add a "::" to the end
      if (ref->get_type_code() == vpiPackage) strcat(buf, "::");
}

static const char* scope_get_type(int code)
{
      switch (code) {
          case vpiModule:
            return "vpiModule";
          case vpiGenScope:
            return "vpiGenScope";
          case vpiFunction:
            return "vpiFunction";
          case vpiTask:
            return "vpiTask";
          case vpiNamedBegin:
            return "vpiNamedBegin";
          case vpiNamedFork:
            return "vpiNamedFork";
          default:
            fprintf(stderr, "VPI error: invalid scope type code %d.\n", code);
            return NULL;
      }
}

static char* scope_get_str(int code, vpiHandle obj)
{
      __vpiScope*ref = dynamic_cast<__vpiScope*>(obj);
      assert(ref);

      char buf[4096];  // XXX is a fixed buffer size really reliable?
      const char *p=0;
      switch (code) {
	  case vpiDefFile:
	    p = file_names[ref->def_file_idx];
	    break;

	  case vpiFile:
	    p = file_names[ref->file_idx];
	    break;

	  case vpiFullName:
	    buf[0] = 0;
	    construct_scope_fullname(ref, buf);
	    p = buf;
	    break;

	  case vpiName:
	    p = ref->scope_name();
	    break;

	  case vpiDefName:
	    p = ref->scope_def_name();
	    break;

	  case vpiType:
	    p = scope_get_type(code);
	    break;

	  default:
	    fprintf(stderr, "VPI error: invalid scope string code %d.\n", code);
	    return NULL;
      }
      return simple_set_rbuf_str(p);
}

static vpiHandle scope_get_handle(int code, vpiHandle obj)
{
      __vpiScope*rfp = dynamic_cast<__vpiScope*>(obj);
      assert(rfp);

      switch (code) {

	  case vpiScope:
	    return rfp->scope;

	  case vpiModule:
	    return rfp->scope;
      }

      return 0;
}

/* compares vpiType's considering object classes */
static int compare_types(int code, int type)
{
	/* NOTE: The Verilog VPI does not for any object support
	   vpiScope as an iterator parameter, so it is used here as a
	   means to scan everything in the *current* scope. */
      if (code == vpiScope)
	    return 1;

      if (code == type)
	    return 1;

      if ( code == vpiInternalScope &&
	     (type == vpiModule ||
	     type == vpiGenScope ||
	     type == vpiFunction ||
	     type == vpiTask ||
	     type == vpiNamedBegin ||
	     type == vpiNamedFork) )
	    return 1;

      if ( code == vpiInstance &&
	    (type == vpiModule ||
	     type == vpiProgram ||
	     type == vpiInterface ||
	     type == vpiPackage) )
	    return 1;

      if ( code == vpiVariables &&
	     (type == vpiIntegerVar  ||
	      type == vpiBitVar      ||
	      type == vpiByteVar     ||
	      type == vpiShortIntVar ||
	      type == vpiIntVar      ||
	      type == vpiLongIntVar  ||
	      type == vpiTimeVar     ||
	      type == vpiRealVar))
	    return 1;

      return 0;
}

static vpiHandle make_subset_iterator_(int type_code, vector<vpiHandle>&table)
{
      unsigned mcnt = 0, ncnt = 0;
      vpiHandle*args;

      for (unsigned idx = 0; idx < table.size(); idx += 1)
	    if (compare_types(type_code, table[idx]->get_type_code()))
		  mcnt += 1;

      if (mcnt == 0)
	    return 0;

      args = (vpiHandle*)calloc(mcnt, sizeof(vpiHandle));
      for (unsigned idx = 0; idx < table.size(); idx += 1)
	    if (compare_types(type_code, table[idx]->get_type_code()))
		  args[ncnt++] = table[idx];

      assert(ncnt == mcnt);

      return vpip_make_iterator(mcnt, args, true);
}

/*
 * This function implements the vpi_iterate method for vpiModule and
 * similar objects. The vpi_iterate allows the user to iterate over
 * things that are contained in the scope object, by generating an
 * iterator for the requested set of items.
 */
static vpiHandle module_iter(int code, vpiHandle obj)
{
      __vpiScope*ref = dynamic_cast<__vpiScope*>(obj);
      assert(ref);

      return make_subset_iterator_(code, ref->intern);
}


__vpiScope::__vpiScope(const char*nam, const char*tnam, bool auto_flag)
: is_automatic_(auto_flag)
{
      name_ = vpip_name_string(nam);
      tname_ = vpip_name_string(tnam? tnam : "");
}

int __vpiScope::vpi_get(int code)
{
      switch (code) {
	  case vpiCellInstance:
	    return is_cell? 1 : 0;

	  case vpiDefLineNo:
	    return def_lineno;

	  case vpiLineNo:
	    return lineno;

	  case vpiTimeUnit:
	    return time_units;

	  case vpiTimePrecision:
	    return time_precision;

	  case vpiTopModule:
	    return 0x0 == scope;

          case vpiAutomatic:
	    return is_automatic_? 1 : 0;
      }

      return vpiUndefined;
}


char*__vpiScope::vpi_get_str(int code)
{ return scope_get_str(code, this); }

vpiHandle __vpiScope::vpi_handle(int code)
{ return scope_get_handle(code, this); }

vpiHandle __vpiScope::vpi_iterate(int code)
{ return module_iter(code, this); }


class vpiScopeModule  : public __vpiScope {
    public:
      inline vpiScopeModule(const char*nam, const char*tnam)
      : __vpiScope(nam,tnam,false) { }
      int get_type_code(void) const { return vpiModule; }
};

struct vpiScopePackage  : public __vpiScope {
      inline vpiScopePackage(const char*nam, const char*tnam)
      : __vpiScope(nam,tnam) { }
      int get_type_code(void) const { return vpiPackage; }
};

struct vpiScopeTask  : public __vpiScope {
      inline vpiScopeTask(const char*nam, const char*tnam)
      : __vpiScope(nam,tnam) { }
      int get_type_code(void) const { return vpiTask; }
};

struct vpiScopeTaskAuto  : public __vpiScope {
      inline vpiScopeTaskAuto(const char*nam, const char*tnam)
      : __vpiScope(nam,tnam,true) {  }
      int get_type_code(void) const { return vpiTask; }
};

struct vpiScopeBegin  : public __vpiScope {
      inline vpiScopeBegin(const char*nam, const char*tnam)
      : __vpiScope(nam,tnam,false) { }
      int get_type_code(void) const { return vpiNamedBegin; }
};

class vpiScopeBeginAuto  : public __vpiScope {
    public:
      inline vpiScopeBeginAuto(const char*nam, const char*tnam)
      : __vpiScope(nam,tnam,true) {  }
      int get_type_code(void) const { return vpiNamedBegin; }
};

struct vpiScopeGenerate  : public __vpiScope {
      inline vpiScopeGenerate(const char*nam, const char*tnam)
      : __vpiScope(nam,tnam) { }
      int get_type_code(void) const { return vpiGenScope; }
};

struct vpiScopeFork  : public __vpiScope {
      inline vpiScopeFork(const char*nam, const char*tnam)
      : __vpiScope(nam,tnam,false) { }
      int get_type_code(void) const { return vpiNamedFork; }
};

class vpiScopeForkAuto  : public __vpiScope {
    public:
      inline vpiScopeForkAuto(const char*nam, const char*tnam)
      : __vpiScope(nam,tnam,true) {  }
      int get_type_code(void) const { return vpiNamedFork; }
};

struct vpiScopeClass  : public __vpiScope {
      inline vpiScopeClass(const char*nam, const char*tnam)
      : __vpiScope(nam,tnam) { }
      int get_type_code(void) const { return vpiClassTypespec; }
};

/*
 * The current_scope is a compile time concept. As the vvp source is
 * compiled, items that have scope are placed in the current
 * scope. The ".scope" directives select the scope that is current.
 */
static __vpiScope*current_scope = 0;

void vpip_attach_to_scope(__vpiScope*scope, vpiHandle obj)
{
      assert(scope);
      scope->intern.push_back(obj);
}

/*
 * When the compiler encounters a scope declaration, this function
 * creates and initializes a __vpiScope object with the requested name
 * and within the addressed parent. The label is used as a key in the
 * symbol table and the name is used to construct the actual object.
 */
void
compile_scope_decl(char*label, char*type, char*name, char*tname,
                   char*parent, long file_idx, long lineno,
                   long def_file_idx, long def_lineno, long is_cell)
{
      count_vpi_scopes += 1;
      char vec_type;
      char sign_flag;
      unsigned wid;

      __vpiScope*scope;
      if (strcmp(type,"module") == 0) {
	    scope = new vpiScopeModule(name, tname);
      } else if ( sscanf(type, "function.vec%c.%c%u", &vec_type, &sign_flag, &wid) == 3 ) {
	    int type_code;
	    if (sign_flag=='s') {
		  type_code = vpiSizedSignedFunc;
	    } else if (sign_flag=='u') {
		  type_code = vpiSizedFunc;
	    } else if (sign_flag=='i') {
		  type_code = vpiIntFunc;
	    } else {
		  assert(0);
		  type_code = vpiSizedFunc;
	    }
	    vvp_bit4_t init_val = vec_type == '4' ? BIT4_X : BIT4_0;
	    scope = new vpiScopeFunction(name, tname, false, type_code, wid, init_val);

      } else if ( sscanf(type, "autofunction.vec%c.%c%u", &vec_type, &sign_flag, &wid) == 3 ) {
	    int type_code;
	    switch (sign_flag) {
		case 's':
		  type_code = vpiSizedSignedFunc;
		  break;
		case 'u':
		  type_code = vpiSizedFunc;
		  break;
		default:
		  assert(0);
		  type_code = vpiSizedFunc;
		  break;
	    }
	    vvp_bit4_t init_val = vec_type == '4' ? BIT4_X : BIT4_0;
	    scope = new vpiScopeFunction(name, tname, true, type_code, wid, init_val);

      } else if (strcmp(type,"function.obj") == 0) {
	    scope = new vpiScopeFunction(name, tname, false, vpiSizedFunc, 0, BIT4_0);
      } else if (strcmp(type,"autofunction.obj") == 0) {
	    scope = new vpiScopeFunction(name, tname, true, vpiSizedFunc, 0, BIT4_0);
      } else if (strcmp(type,"function.real") == 0) {
	    scope = new vpiScopeFunction(name, tname, false, vpiRealFunc, 0, BIT4_0);
      } else if (strcmp(type,"autofunction.real") == 0) {
	    scope = new vpiScopeFunction(name, tname, true, vpiRealFunc, 0, BIT4_0);
      } else if (strcmp(type,"function.str") == 0) {
	    scope = new vpiScopeFunction(name, tname, false, vpiOtherFunc, 0, BIT4_0);
      } else if (strcmp(type,"autofunction.str") == 0) {
	    scope = new vpiScopeFunction(name, tname, true, vpiOtherFunc, 0, BIT4_0);
      } else if (strcmp(type,"function.void") == 0) {
	    scope = new vpiScopeFunction(name, tname, false, vpiOtherFunc, 0, BIT4_0);
      } else if (strcmp(type,"autofunction.void") == 0) {
	    scope = new vpiScopeFunction(name, tname, true, vpiOtherFunc, 0, BIT4_0);
      } else if (strcmp(type,"task") == 0) {
	    scope = new vpiScopeTask(name, tname);
      } else if (strcmp(type,"autotask") == 0) {
	    scope = new vpiScopeTaskAuto(name, tname);
      } else if (strcmp(type,"fork") == 0) {
	    scope = new vpiScopeFork(name, tname);
      } else if (strcmp(type,"autofork") == 0) {
	    scope = new vpiScopeForkAuto(name, tname);
      } else if (strcmp(type,"begin") == 0) {
	    scope = new vpiScopeBegin(name, tname);
      } else if (strcmp(type,"autobegin") == 0) {
	    scope = new vpiScopeBeginAuto(name, tname);
      } else if (strcmp(type,"generate") == 0) {
	    scope = new vpiScopeGenerate(name, tname);
      } else if (strcmp(type,"package") == 0) {
	    scope = new vpiScopePackage(name, tname);
      } else if (strcmp(type,"class") == 0) {
	    scope = new vpiScopeClass(name, tname);
      } else {
	    scope = new vpiScopeModule(name, tname);
	    assert(0);
      }

      scope->file_idx = (unsigned) file_idx;
      scope->lineno  = (unsigned) lineno;
      scope->def_file_idx = (unsigned) def_file_idx;
      scope->def_lineno  = (unsigned) def_lineno;
      scope->item = 0;
      scope->nitem = 0;
      scope->live_contexts = 0;
      scope->free_contexts = 0;

      if (is_cell) scope->is_cell = true;
      else scope->is_cell = false;

      current_scope = scope;

      compile_vpi_symbol(label, scope);

      free(label);
      free(type);
      delete[] name;
      delete[] tname;

      if (parent) {
	    static vpiHandle obj;
	    compile_vpi_lookup(&obj, parent);
	    assert(obj);
	    __vpiScope*sp = dynamic_cast<__vpiScope*>(obj);
	    vpip_attach_to_scope(sp, scope);
	    scope->scope = dynamic_cast<__vpiScope*>(obj);

	      /* Inherit time units and precision from the parent scope. */
	    scope->time_units = sp->time_units;
	    scope->time_precision = sp->time_precision;

      } else {
	    scope->scope = 0x0;

	    vpip_root_table.push_back(scope);

	      /* Root scopes inherit time_units and precision from the
	         system precision. */
	    scope->time_units = vpip_get_time_precision();
	    scope->time_precision = vpip_get_time_precision();
      }
}

void compile_scope_recall(char*symbol)
{
	/* A __vpiScope starts with a __vpiHandle structure so this is
	   a safe cast. We need the (void*) to avoid a dereferenced
	   type punned pointer warning from some gcc compilers. */
      compile_vpi_lookup((vpiHandle*)(void*)&current_scope, symbol);
      assert(current_scope);
}

/*
 * This function handles the ".timescale" directive in the vvp
 * source. It sets in the current scope the specified units value.
 */
void compile_timescale(long units, long precision)
{
      assert(current_scope);
      current_scope->time_units = units;
      current_scope->time_precision = precision;
}

__vpiScope* vpip_peek_current_scope(void)
{
      return current_scope;
}

void vpip_attach_to_current_scope(vpiHandle obj)
{
      vpip_attach_to_scope(current_scope, obj);
}

__vpiScope* vpip_peek_context_scope(void)
{
      __vpiScope*scope = current_scope;

        /* A context is allocated for each automatic task or function.
           Storage for nested scopes (named blocks) is allocated in
           the parent context. */
      while (scope->scope && scope->scope->is_automatic())
            scope = scope->scope;

      return scope;
}

unsigned vpip_add_item_to_context(automatic_hooks_s*item,
                                  __vpiScope*scope)
{
      assert(scope);
      assert(scope->is_automatic());

      unsigned idx = scope->nitem++;

      if (scope->item == 0)
	    scope->item = (automatic_hooks_s**)
		  malloc(sizeof(automatic_hooks_s*));
      else
	    scope->item = (automatic_hooks_s**)
		  realloc(scope->item, sizeof(automatic_hooks_s*)*scope->nitem);

      scope->item[idx] = item;

        /* Offset the context index by 2 to leave space for the list links. */
      return 2 + idx;
}


vpiPortInfo::vpiPortInfo( __vpiScope *parent,
              unsigned index,
              int vpi_direction,
              unsigned width,
              const char *name,
              char* buffer) :
      parent_(parent),
      index_(index),
      direction_(vpi_direction),
      width_(width),
      name_(name)
{
      if (buffer != nullptr) functor_ref_lookup(&ref_, buffer);
      else ref_ = nullptr;
}

vpiPortInfo::~vpiPortInfo()
{
      delete[] name_;
}

#ifdef CHECK_WITH_VALGRIND
void port_delete(__vpiHandle *handle)
{
      delete dynamic_cast<vpiPortInfo *>(handle);
}
#endif

int vpiPortInfo::vpi_get(int code)
{
      switch( code ) {

        case vpiDirection :
          return direction_;
        case vpiPortIndex :
          return index_;
        case vpiSize :
          return width_;
        default :
          return vpiUndefined;
      }

}


char *vpiPortInfo::vpi_get_str(int code)
{
      switch( code ) {
        case vpiName :
          return simple_set_rbuf_str(name_);
        default :
          return NULL;
      }

}


vpiHandle vpiPortInfo::vpi_handle(int code)
{

      switch (code) {

          case vpiParent:
          case vpiScope:
          case vpiModule:
            return parent_;
          default :
            break;
      }

      return 0;
}

static vpiHandle portinfo_iterate(int code, vpiHandle ref)
{
      vpiPortInfo*rfp = dynamic_cast<vpiPortInfo*>(ref);
      assert(rfp);
      unsigned width = rfp->get_width();

      switch (code) {
	  case vpiBit: {
	    vpiHandle*args = (vpiHandle*)calloc(width, sizeof(vpiHandle*));

	    for (unsigned i = 0; i<rfp->port_bits_.size(); i++) {
		  args[i] = rfp->port_bits_[i];
	    }

	    return vpip_make_iterator(width, args, true);
	  }
      }
      return 0;
}

vpiHandle vpiPortInfo::vpi_iterate(int code)
{
      return portinfo_iterate(code, this);
}


vpiPortBitInfo::vpiPortBitInfo(vpiPortInfo *parent,
                               unsigned bit) :
      parent_(parent),
      bit_(bit)
{
}

vpiPortBitInfo::~vpiPortBitInfo()
{
}

#ifdef CHECK_WITH_VALGRIND
void port_bit_delete(__vpiHandle *handle)
{
      delete dynamic_cast<vpiPortBitInfo *>(handle);
}
#endif

vpiHandle vpiPortBitInfo::vpi_handle(int code)
{

      switch (code) {
          case vpiParent:
            return parent_;
          default :
            break;
      }

      return 0;
}

int vpiPortBitInfo::vpi_get(int code)
{
      switch( code ) {
        case vpiBit :
          return bit_;
        default :
          break;
      }

      return 0;
}

/* Port info is meta-data to allow vpi queries of the port signature of modules for
 * code-generators etc.  There are no actual nets corresponding to instances of module ports
 * as elaboration directly connects nets connected through module ports.
 */
void compile_port_info( unsigned index, int vpi_direction, unsigned width, const char *name, char* buffer )
{
      vpiPortInfo* obj = new vpiPortInfo( vpip_peek_current_scope(),
                                          index, vpi_direction, width, name, buffer );
      vpip_attach_to_current_scope(obj);

	// Create vpiPortBit objects
      for (unsigned i=0; i<width; i++) {
	    vpiPortBitInfo* obj_bit = new vpiPortBitInfo(obj, i);
	    obj->add_port_bit(obj_bit);
	    vpip_attach_to_current_scope(obj_bit);
      }
}
