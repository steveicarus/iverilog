/*
 * Copyright (c) 2001-2020 Stephen Williams (steve@icarus.com)
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

/*
 * This file keeps the table of system/task definitions. This table is
 * built up before the input source file is parsed, and is used by the
 * compiler when %vpi_call statements are encountered.
 */
# include  "vpi_priv.h"
# include  "vthread.h"
# include  "compile.h"
# include  "config.h"
#ifdef CHECK_WITH_VALGRIND
# include  "vvp_cleanup.h"
#endif
# include  <iostream>
# include  <cstdio>
# include  <cstdlib>
# include  <cstring>
# include  <cassert>
# include  "ivl_alloc.h"

using namespace std;

inline __vpiUserSystf::__vpiUserSystf()
{ }

int __vpiUserSystf::get_type_code(void) const
{ return vpiUserSystf; }


static vpiHandle systask_handle(int type, vpiHandle ref)
{
      struct __vpiSysTaskCall*rfp = dynamic_cast<__vpiSysTaskCall*>(ref);

      switch (type) {
	  case vpiScope:
	    return rfp->scope;

	  case vpiUserSystf:
	      /* Assert that vpiUserDefn is true! */
	    assert(rfp->defn->is_user_defn);
	    return rfp->defn;

	  default:
	    return 0;
      };
}

static int systask_get(int type, vpiHandle ref)
{
      struct __vpiSysTaskCall*rfp = dynamic_cast<__vpiSysTaskCall*>(ref);

      switch (type) {
	    /* This is not the correct way to get this information, but
	     * some of the code that implements the acc and tf routines
	     * use this method so we will keep it in for now. */
	  case vpiTimeUnit:
	    return rfp->scope->time_units;
	  case vpiTimePrecision:
	    return rfp->scope->time_precision;

	  case vpiLineNo:
	    return rfp->lineno;

	  case vpiUserDefn:
	    return rfp->defn->is_user_defn;

	  default:
	    return vpiUndefined;
      }
}

// support getting vpiSize for a system function call
static int sysfunc_get(int type, vpiHandle ref)
{
      struct __vpiSysTaskCall*rfp = dynamic_cast<__vpiSysTaskCall*>(ref);

      switch (type) {
	  case vpiSize:
	    assert(0); // This should be handled by derived classes
	    return 0;

	  case vpiLineNo:
	    return rfp->lineno;

	  case vpiUserDefn:
	    return rfp->defn->is_user_defn;

	  default:
	    return vpiUndefined;
      }
}

/*
 * the get_str function only needs to support vpiName
 */

static char *systask_get_str(int type, vpiHandle ref)
{
      struct __vpiSysTaskCall*rfp = dynamic_cast<__vpiSysTaskCall*>(ref);

      switch (type) {
          case vpiFile:
            assert(rfp->file_idx < file_names.size());
            return simple_set_rbuf_str(file_names[rfp->file_idx]);

          case vpiName:
            return simple_set_rbuf_str(rfp->defn->info.tfname);
      }

      return 0;
}

/*
 * the iter function only supports getting an iterator of the
 * arguments. This works equally well for tasks and functions.
 */
static vpiHandle systask_iter(int, vpiHandle ref)
{
      struct __vpiSysTaskCall*rfp = dynamic_cast<__vpiSysTaskCall*>(ref);

      if (rfp->nargs == 0)
	    return 0;

      return vpip_make_iterator(rfp->nargs, rfp->args, false);
}

struct systask_def : public __vpiSysTaskCall {
      virtual ~systask_def() {};
      virtual int get_type_code(void) const { return vpiSysTaskCall; }
      virtual int vpi_get(int code)         { return systask_get(code, this); }
      virtual char*vpi_get_str(int code)    { return systask_get_str(code, this); }
      virtual vpiHandle vpi_handle(int code) { return systask_handle(code, this); }
      virtual vpiHandle vpi_iterate(int code){ return systask_iter(code, this); }
};

struct sysfunc_def : public systask_def {
      virtual ~sysfunc_def() {};
      virtual int get_type_code(void) const { return vpiSysFuncCall; }
      virtual int vpi_get(int code)         { return sysfunc_get(code, this); }
};

struct sysfunc_real : public sysfunc_def {
      vpiHandle vpi_put_value(p_vpi_value val, int flags);
      inline double return_value() const { return return_value_; }

    private:
      double return_value_;
};

vpiHandle sysfunc_real::vpi_put_value(p_vpi_value vp, int)
{
      put_value = true;

      return_value_ = 0.0;
      switch (vp->format) {
	  case vpiRealVal:
	    return_value_ = vp->value.real;
	    break;
	  default:
	    fprintf(stderr, "Unsupported format %d.\n", (int)vp->format);
	    assert(0);
      }

      return 0;
}

struct sysfunc_str : public sysfunc_def {
      int vpi_get(int code);
      vpiHandle vpi_put_value(p_vpi_value val, int flags);
      inline const std::string& return_value() const { return return_value_; };

    private:
      std::string return_value_;
};

int sysfunc_str::vpi_get(int code)
{
      switch (code) {
	  case vpiSize:
	    return return_value_.size();

	  case vpiLineNo:
	    return lineno;

	  case vpiUserDefn:
	    return defn->is_user_defn;

	  default:
	    return vpiUndefined;
      }
}

vpiHandle sysfunc_str::vpi_put_value(p_vpi_value vp, int)
{
      put_value = true;

      switch (vp->format) {
	  case vpiStringVal:
	    return_value_ = std::string(vp->value.str);
	    break;
	  default:
	    fprintf(stderr, "Unsupported format %d.\n", (int)vp->format);
	    assert(0);
      }

      return 0;
}

class sysfunc_vec4 : public sysfunc_def {
    public:
      explicit inline sysfunc_vec4(unsigned wid): return_value_(wid, BIT4_X) { }
      int vpi_get(int code);
      vpiHandle vpi_put_value(p_vpi_value val, int flags);
      inline const vvp_vector4_t& return_value() const { return return_value_; }

    private:
      vpiHandle put_value_scalar_(p_vpi_value vp);
      vpiHandle put_value_int_(p_vpi_value vp);
      vpiHandle put_value_string_(p_vpi_value vp);
      vpiHandle put_value_vector_(p_vpi_value vp);
      vpiHandle put_value_time_(p_vpi_value vp);

      vvp_vector4_t return_value_;
};

int sysfunc_vec4::vpi_get(int code)
{
      switch (code) {
	  case vpiSize:
	    return return_value_.size();

	  case vpiLineNo:
	    return lineno;

	  case vpiUserDefn:
	    return defn->is_user_defn;

	  default:
	    return vpiUndefined;
      }
}

vpiHandle sysfunc_vec4::put_value_scalar_(p_vpi_value vp)
{
      switch (vp->value.scalar) {
	  case vpi0:
	    return_value_.set_bit(0, BIT4_0);
	    break;
	  case vpi1:
	    return_value_.set_bit(0, BIT4_1);
	    break;
	  case vpiX:
	    return_value_.set_bit(0, BIT4_X);
	    break;
	  case vpiZ:
	    return_value_.set_bit(0, BIT4_Z);
	    break;
	  default:
	    fprintf(stderr, "Unsupported value %d.\n", (int)vp->value.scalar);
	    assert(0);
      }
      return 0;
}

vpiHandle sysfunc_vec4::put_value_int_(p_vpi_value vp)
{
      long tmp = vp->value.integer;
      unsigned width = return_value_.size();
      for (unsigned idx = 0 ;  idx < width ;  idx += 1) {
	    return_value_.set_bit(idx, (tmp&1)? BIT4_1 : BIT4_0);
	    tmp >>= 1;
      }

      return 0;
}

vpiHandle sysfunc_vec4::put_value_string_(p_vpi_value vp)
{
      size_t slen = strlen(vp->value.str);
      unsigned wid = return_value_.size();

      for (unsigned idx = 0 ; idx < wid ; idx += 1) {
	    unsigned cidx = idx / 8;
	      // If wider then the string, then pad with zero.
	    if (cidx >= slen) {
		  return_value_.set_bit(idx, BIT4_0);
		  continue;
	    }

	      // Put the end of the string in the LSB of the vector
	    char use_char = vp->value.str[slen - 1 - cidx];

	    if ((use_char >> (idx % 8)) & 1)
		  return_value_.set_bit(idx, BIT4_1);
	    else
		  return_value_.set_bit(idx, BIT4_0);
      }
      return 0;
}

vpiHandle sysfunc_vec4::put_value_vector_(p_vpi_value vp)
{
      unsigned width = return_value_.size();
      for (unsigned idx = 0 ; idx < width ; idx += 1) {
	    unsigned word = idx / 32;
	    unsigned bidx = idx % 32;

	    unsigned long aval = vp->value.vector[word].aval >> bidx;
	    unsigned long bval = vp->value.vector[word].bval >> bidx;

	    int bit = (aval&1) | ((bval<<1)&2);
	    vvp_bit4_t bit4;

	    switch (bit) {
		case 0:
		  bit4 = BIT4_0;
		  break;
		case 1:
		  bit4 = BIT4_1;
		  break;
		case 2:
		  bit4 = BIT4_Z;
		  break;
		case 3:
		  bit4 = BIT4_X;
		  break;
		default:
		  assert(0);
		  bit4 = BIT4_X;
		  break;
	    }
	    return_value_.set_bit(idx, bit4);
      }
      return 0;
}

vpiHandle sysfunc_vec4::put_value_time_(p_vpi_value vp)
{
      unsigned width = return_value_.size();
      long tmp = 0;
      for (unsigned idx = 0 ; idx < width ; idx += 1) {
	    if (idx == 0)
		  tmp = vp->value.time->low;
	    else if (idx == 32)
		  tmp = vp->value.time->high;
	    else if (idx == 64)
		  tmp = 0;

	    return_value_.set_bit(idx, (tmp&1)? BIT4_1 : BIT4_0);
	    tmp >>= 1;
      }
      return 0;
}

vpiHandle sysfunc_vec4::vpi_put_value(p_vpi_value vp, int)
{
      put_value = true;

      switch (vp->format) {
	  case vpiScalarVal:
	    return put_value_scalar_(vp);
	  case vpiIntVal:
	    return put_value_int_(vp);
	  case vpiStringVal:
	    return put_value_string_(vp);
	  case vpiVectorVal:
	    return put_value_vector_(vp);
	  case vpiTimeVal:
	    return put_value_time_(vp);
	  default:
	    fprintf(stderr, "Unsupported format %d setting sysfunc vec4 value.\n", (int)vp->format);
	    assert(0);
      }

      return 0;
}

struct sysfunc_4net : public sysfunc_def {
      explicit inline sysfunc_4net(unsigned wid) : vwid_(wid) { }
      int vpi_get(int code);
      vpiHandle vpi_put_value(p_vpi_value val, int flags);

    private:
      unsigned vwid_;
};

// support getting vpiSize for a system function call
int sysfunc_4net::vpi_get(int code)
{
      switch (code) {
	  case vpiSize:
	    return vwid_;

	  case vpiLineNo:
	    return lineno;

	  case vpiUserDefn:
	    return defn->is_user_defn;

	  default:
	    return vpiUndefined;
      }
}

vpiHandle sysfunc_4net::vpi_put_value(p_vpi_value vp, int)
{
      put_value = true;

      vvp_vector4_t val (vwid_);

      switch (vp->format) {

          case vpiScalarVal: {
	        switch(vp->value.scalar) {
		      case vpi0:
			val.set_bit(0, BIT4_0);
                        break;
		      case vpi1:
                        val.set_bit(0, BIT4_1);
                        break;
		      case vpiX:
                        val.set_bit(0, BIT4_X);
                        break;
		      case vpiZ:
                        val.set_bit(0, BIT4_Z);
                        break;
		      default:
                        fprintf(stderr, "Unsupported bit value %d.\n",
                                (int)vp->value.scalar);
                        assert(0);
                }
		break;
          }

	  case vpiIntVal: {
		long tmp = vp->value.integer;
		for (unsigned idx = 0 ;  idx < vwid_ ;  idx += 1) {
		      val.set_bit(idx, (tmp&1)? BIT4_1 : BIT4_0);
		      tmp >>= 1;
		}
		break;
	  }

          case vpiTimeVal: {
                unsigned long tmp = vp->value.time->low;
                for (unsigned idx = 0 ;  idx < vwid_ ;  idx += 1) {
                      val.set_bit(idx, (tmp&1)? BIT4_1 : BIT4_0);

                      if (idx == 31)
                            tmp = vp->value.time->high;
                      else
                            tmp >>= 1;
                }
                break;
          }

	  case vpiVectorVal:

	    for (unsigned wdx = 0 ;  wdx < vwid_ ;  wdx += 32) {
		  unsigned word = wdx / 32;
		  unsigned long aval = vp->value.vector[word].aval;
		  unsigned long bval = vp->value.vector[word].bval;

		  for (unsigned idx = 0 ;  (wdx+idx) < vwid_ && idx < 32;
		       idx += 1) {
			int bit = (aval&1) | ((bval<<1)&2);
			vvp_bit4_t bit4;

			switch (bit) {
			    case 0:
			      bit4 = BIT4_0;
			      break;
			    case 1:
			      bit4 = BIT4_1;
			      break;
			    case 2:
			      bit4 = BIT4_Z;
			      break;
			    case 3:
			      bit4 = BIT4_X;
			      break;
			    default:
			      bit4 = BIT4_X;
			      fprintf(stderr, "Unsupported bit value %d.\n",
			              bit);
			      assert(0);
			}
			val.set_bit(wdx+idx, bit4);

			aval >>= 1;
			bval >>= 1;
		  }
	    }
	    break;

	  default:
	    fprintf(stderr, "XXXX format=%d, vwid_=%u\n", (int)vp->format, vwid_);
	    assert(0);
      }

      fnet->send_vec4(val, vthread_get_wt_context());
      return 0;
}

struct sysfunc_rnet : public sysfunc_def {
      vpiHandle vpi_put_value(p_vpi_value val, int flags);
};

vpiHandle sysfunc_rnet::vpi_put_value(p_vpi_value vp, int)
{
      put_value = true;

      double value;
      switch (vp->format) {

	  case vpiRealVal:
	    value = vp->value.real;
	    break;

	  default:
	    value = 0.0;
	    fprintf(stderr, "Unsupported format %d.\n", (int)vp->format);
	    assert(0);
      }

      fnet->send_real(value, vthread_get_wt_context());

      return 0;
}

struct sysfunc_no : public sysfunc_def {
      vpiHandle vpi_put_value(p_vpi_value val, int flags);
};

vpiHandle sysfunc_no::vpi_put_value(p_vpi_value, int)
{
      return 0;
}


  /* **** Manipulate the internal data structures. **** */

/*
 * We keep a table of all the __vpiUserSystf objects that are created
 * so that the user can iterate over them. The def_table is an array
 * of pointers to __vpiUserSystf objects. This table can be searched
 * by name using the vpi_find_systf function, and they can be
 * collected into an iterator using the vpip_make_systf_iterator function.
 */
static struct __vpiUserSystf**def_table = 0;
static unsigned def_count = 0;

static struct __vpiUserSystf* allocate_def(void)
{
      if (def_table == 0) {
	    def_table = (struct __vpiUserSystf**)
		  malloc(sizeof (struct __vpiUserSystf*));

	    def_table[0] = new __vpiUserSystf;

	    def_count = 1;
	    return def_table[0];
      }

      def_table = (struct __vpiUserSystf**)
	    realloc(def_table, (def_count+1)*sizeof (struct __vpiUserSystf*));

      def_table[def_count] = new __vpiUserSystf;

      return def_table[def_count++];
}

#ifdef CHECK_WITH_VALGRIND
void def_table_delete(void)
{
      for (unsigned idx = 0; idx < def_count; idx += 1) {
	    free(const_cast<char *>(def_table[idx]->info.tfname));
	    delete def_table[idx];
      }
      free(def_table);
      def_table = 0;
      def_count = 0;
}
#endif

class __vpiSystfIterator : public __vpiHandle {
    public:
      explicit __vpiSystfIterator(unsigned idx);
      int get_type_code(void) const;
      vpiHandle vpi_index(int idx);
      free_object_fun_t free_object_fun(void);

      unsigned next;
};

static vpiHandle systf_iterator_scan(vpiHandle ref, int)
{
      __vpiSystfIterator*obj = dynamic_cast<__vpiSystfIterator*>(ref);

      if (obj->next >= def_count) {
	    vpi_free_object(ref);
	    return 0;
      }

      unsigned use_index = obj->next;
      while (!def_table[use_index]->is_user_defn) {
	    obj->next += 1;
	    use_index = obj->next;
	    if (obj->next >= def_count) {
		  vpi_free_object(ref);
		  return 0;
	    }
      }
      obj->next += 1;
      return def_table[use_index];
}

static int systf_iterator_free_object(vpiHandle ref)
{
      __vpiSystfIterator*obj = dynamic_cast<__vpiSystfIterator*>(ref);
      delete obj;
      return 1;
}

inline __vpiSystfIterator::__vpiSystfIterator(unsigned idx)
{ next = idx; }

int __vpiSystfIterator::get_type_code(void) const
{ return vpiIterator; }

vpiHandle __vpiSystfIterator::vpi_index(int idx)
{ return systf_iterator_scan(this, idx); }

__vpiHandle::free_object_fun_t __vpiSystfIterator::free_object_fun(void)
{ return &systf_iterator_free_object; }

vpiHandle vpip_make_systf_iterator(void)
{
	/* Check to see if there are any user defined functions. */
      bool have_user_defn = false;
      unsigned idx;
      for (idx = 0; idx < def_count; idx += 1) {
	    if (def_table[idx]->is_user_defn) {
		  have_user_defn = true;
		  break;
	    }
      }
      if (!have_user_defn) return 0;

      __vpiSystfIterator*res = new __vpiSystfIterator(idx);
      res->next = idx;
      return res;
}

struct __vpiUserSystf* vpip_find_systf(const char*name)
{
      for (unsigned idx = 0 ;  idx < def_count ;  idx += 1)
	    if (strcmp(def_table[idx]->info.tfname, name) == 0)
		  return def_table[idx];

      return 0;
}

void vpip_make_systf_system_defined(vpiHandle ref)
{
      assert(ref);
      struct __vpiUserSystf*obj = dynamic_cast<__vpiUserSystf*>(ref);
      assert(obj);
      obj->is_user_defn = false;
}

/*
 * To get better error message we need to cache the vpi_call fail
 * information so that we can print the file name.
 */
enum vpi_call_error_type {VPI_CALL_NO_DEF, VPI_CALL_TASK_AS_FUNC,
                          VPI_CALL_FUNC_AS_TASK, VPI_CALL_FUNC_AS_TASK_WARN};
typedef struct vpi_call_error {
      vpi_call_error_type type;
      char *name;
      long file_idx;
      long lineno;
} vpi_call_error_s, *vpi_call_error_p;

static vpi_call_error_p vpi_call_error_lst = NULL;
static unsigned vpi_call_error_num = 0;

static void add_vpi_call_error(vpi_call_error_type type, const char *name,
                               long file_idx, long lineno)
{
      vpi_call_error_lst = (vpi_call_error_p)
                            realloc((void *)vpi_call_error_lst,
                                    (vpi_call_error_num + 1) *
                                    sizeof(vpi_call_error_s));
      vpi_call_error_lst[vpi_call_error_num].type = type;
      vpi_call_error_lst[vpi_call_error_num].name = strdup(name);
      vpi_call_error_lst[vpi_call_error_num].file_idx = file_idx;
      vpi_call_error_lst[vpi_call_error_num].lineno = lineno;
      vpi_call_error_num += 1;
}

void print_vpi_call_errors()
{
      for (unsigned idx = 0; idx < vpi_call_error_num; idx += 1) {
	    switch (vpi_call_error_lst[idx].type) {
		case VPI_CALL_NO_DEF:
		  fprintf(stderr, "%s:%d: Error: System task/function %s() is "
		                  "not defined by any module.\n",
		                  file_names[vpi_call_error_lst[idx].file_idx],
		                  (int)vpi_call_error_lst[idx].lineno,
		                  vpi_call_error_lst[idx].name);
		  break;
		case VPI_CALL_TASK_AS_FUNC:
		  fprintf(stderr, "%s:%d: Error: %s() is a system task, it "
			          "cannot be called as a function.\n",
		                  file_names[vpi_call_error_lst[idx].file_idx],
		                  (int)vpi_call_error_lst[idx].lineno,
		                  vpi_call_error_lst[idx].name);
		  break;
		case VPI_CALL_FUNC_AS_TASK:
		  fprintf(stderr, "%s:%d: Error: %s() is a system function, it "
			          "cannot be called as a task.\n",
		                  file_names[vpi_call_error_lst[idx].file_idx],
		                  (int)vpi_call_error_lst[idx].lineno,
		                  vpi_call_error_lst[idx].name);
		  break;
		case VPI_CALL_FUNC_AS_TASK_WARN:
                 fprintf(stderr, "%s:%d: Warning: Calling system function "
                                 "%s() as a task.\n",
		                  file_names[vpi_call_error_lst[idx].file_idx],
		                  (int)vpi_call_error_lst[idx].lineno,
		                  vpi_call_error_lst[idx].name);
                 fprintf(stderr, "%s:%d:          The functions return "
                                 "value will be ignored.\n",
                                 file_names[vpi_call_error_lst[idx].file_idx],
                                 (int)vpi_call_error_lst[idx].lineno);
		  break;
	    }
	    free(vpi_call_error_lst[idx].name);
      }
      free(vpi_call_error_lst);
      fflush(stderr);
}

#ifdef CHECK_WITH_VALGRIND
static void cleanup_vpi_call_args(unsigned argc, vpiHandle*argv)
{
      if (argc) {
	      /* Since this is just being used to cleanup the arguments a
	       * system task definition can be used. */
	    struct __vpiSysTaskCall*obj = new systask_def;
	    obj->nargs = argc;
	    obj->args  = argv;
	    vpi_call_delete(obj);
      }
}
#endif

/*
 * A vpi_call is actually built up into a vpiSysTaskCall VPI object
 * that refers back to the vpiUserSystf VPI object that is the
 * definition. So this function is called by the compiler when a
 * %vpi_call statement is encountered. Create here a vpiHandle that
 * describes the call, and return it. The %vpi_call instruction will
 * store this handle for when it is executed.
 */
vpiHandle vpip_build_vpi_call(const char*name, int val_code, unsigned return_width,
			      vvp_net_t*fnet,
			      bool func_as_task_err, bool func_as_task_warn,
			      unsigned argc, vpiHandle*argv,
			      unsigned vec4_stack, unsigned real_stack, unsigned string_stack,
			      long file_idx, long lineno)
{
      assert(!(func_as_task_err && func_as_task_warn));

      struct __vpiUserSystf*defn = vpip_find_systf(name);
      if (defn == 0) {
	    add_vpi_call_error(VPI_CALL_NO_DEF, name, file_idx, lineno);
#ifdef CHECK_WITH_VALGRIND
	    cleanup_vpi_call_args(argc, argv);
#endif
	    return 0;
      }

      switch (defn->info.type) {
	  case vpiSysTask:
	    if (val_code != 0 || fnet != 0) {
		  add_vpi_call_error(VPI_CALL_TASK_AS_FUNC, name, file_idx,
		                     lineno);
#ifdef CHECK_WITH_VALGRIND
		  cleanup_vpi_call_args(argc, argv);
#endif
		  return 0;
	    }
	    break;

	  case vpiSysFunc:
	    if (val_code == 0 && fnet == 0) {
		  if (func_as_task_err) {
			add_vpi_call_error(VPI_CALL_FUNC_AS_TASK,
			                   name, file_idx, lineno);
#ifdef CHECK_WITH_VALGRIND
			cleanup_vpi_call_args(argc, argv);
#endif
			return 0;
		  } else if (func_as_task_warn) {
			add_vpi_call_error(VPI_CALL_FUNC_AS_TASK_WARN,
			                   name, file_idx, lineno);
		  }
	    }
	    break;

	  default:
	    fprintf(stderr, "Unsupported vpi_call type %d.\n",
	                    (int)defn->info.type);
	    assert(0);
      }

      struct __vpiSysTaskCall*obj = 0;

      switch (defn->info.type) {
	  case vpiSysTask:
	    obj = new systask_def;
	    break;

	  case vpiSysFunc:
	    if (fnet && val_code == -vpiRealVal) {
		  obj = new sysfunc_rnet;

	    } else if (fnet && val_code == -vpiVectorVal) {
		  obj = new sysfunc_4net(return_width);

	    } else if (val_code == -vpiRealVal) {
		  obj = new sysfunc_real;

	    } else if (val_code == -vpiVectorVal) {
		  obj = new sysfunc_vec4(return_width);

	    } else if (val_code == -vpiStringVal) {
		  obj = new sysfunc_str;

           } else if (val_code == 0 && fnet == 0) {
		  obj = new sysfunc_no;

	    } else {
		  fprintf(stderr, "XXXX fnet=%p, val_code=%d\n", fnet, val_code);
		  assert(0);
	    }
	    break;
      }

      obj->scope = vpip_peek_current_scope();
      obj->defn  = defn;
      obj->nargs = argc;
      obj->args  = argv;
      obj->vec4_stack = vec4_stack;
      obj->real_stack = real_stack;
      obj->string_stack = string_stack;
      obj->fnet  = fnet;
      obj->file_idx  = (unsigned) file_idx;
      obj->lineno   = (unsigned) lineno;
      obj->userdata  = 0;
      obj->put_value = false;

      compile_compiletf(obj);

      return obj;
}

#ifdef CHECK_WITH_VALGRIND
void vpi_call_delete(vpiHandle item)
{
      struct __vpiSysTaskCall*obj = dynamic_cast<__vpiSysTaskCall*>(item);
	/* The object can be NULL if there was an error. */
      if (!obj) return;
      for (unsigned arg = 0; arg < obj->nargs; arg += 1) {
	    switch (obj->args[arg]->get_type_code()) {
		case vpiConstant:
		  switch (vpi_get(_vpiFromThr, obj->args[arg])) {
		      case _vpiNoThr:
			constant_delete(obj->args[arg]);
			break;
		      case _vpiString:
			thread_string_delete(obj->args[arg]);
			break;
		      case _vpiVThr:
			thread_vthr_delete(obj->args[arg]);
			break;
		      case _vpiWord:
			thread_word_delete(obj->args[arg]);
			break;
		      default:
			assert(0);
		  }
		  break;
		case vpiMemoryWord:
		  switch (vpi_get(_vpiFromThr, obj->args[arg])) {
		      case _vpi_at_A:
			A_delete(obj->args[arg]);
			break;
		  }
		  break;
		case vpiPartSelect:
		  switch (vpi_get(_vpiFromThr, obj->args[arg])) {
		      case _vpi_at_PV:
			PV_delete(obj->args[arg]);
			break;
		      case _vpi_at_APV:
			APV_delete(obj->args[arg]);
			break;
		  }
		  break;
	    }
      }
      free(obj->args);
      delete obj;
}
#endif

/*
 * This function is used by the %vpi_call instruction to actually
 * place the call to the system task/function. For now, only support
 * calls to system tasks.
 */

vthread_t vpip_current_vthread;

void vpip_execute_vpi_call(vthread_t thr, vpiHandle ref)
{
      vpip_current_vthread = thr;

      vpip_cur_task = dynamic_cast<__vpiSysTaskCall*>(ref);

      if (vpip_cur_task->defn->info.calltf) {
	    assert(vpi_mode_flag == VPI_MODE_NONE);
	    vpi_mode_flag = VPI_MODE_CALLTF;
	    vpip_cur_task->put_value = false;
	    vpip_cur_task->defn->info.calltf(vpip_cur_task->defn->info.user_data);
	    vpi_mode_flag = VPI_MODE_NONE;
	      /* If the function call did not set a value then put a
	       * default value (0). */
	    if (ref->get_type_code() == vpiSysFuncCall &&
	        !vpip_cur_task->put_value) {
		  s_vpi_value val;
		  val.format = vpiIntVal;
		  val.value.integer = 0;
		  vpi_put_value(ref, &val, 0, vpiNoDelay);
	    }
      }
      if (vpip_cur_task->vec4_stack > 0)
	    vthread_pop_vec4(thr, vpip_cur_task->vec4_stack);
      if (vpip_cur_task->real_stack > 0)
	    vthread_pop_real(thr, vpip_cur_task->real_stack);
      if (vpip_cur_task->string_stack > 0)
	    vthread_pop_str(thr, vpip_cur_task->string_stack);

	/* If the function returns a value, then push the value
	   to the appropriate thread stack. */
      if (sysfunc_real*func_real = dynamic_cast<sysfunc_real*>(ref)) {
	    vthread_push(thr, func_real->return_value());
      }
      else if (sysfunc_str*func_string = dynamic_cast<sysfunc_str*>(ref)) {
	    vthread_push(thr, func_string->return_value());
      }
      else if (sysfunc_vec4*func_vec4 = dynamic_cast<sysfunc_vec4*>(ref)) {
	    vthread_push(thr, func_vec4->return_value());
      }
      vpip_cur_task = 0;
}

/*
 * This is the entry function that a VPI module uses to hook a new
 * task/function into the simulator. The function creates a new
 * __vpi_userSystf to represent the definition for the calls that come
 * to pass later.
 */
vpiHandle vpi_register_systf(const struct t_vpi_systf_data*ss)
{
      struct __vpiUserSystf*cur = allocate_def();
      assert(ss);
      switch (ss->type) {
	  case vpiSysTask:
	  case vpiSysFunc:
	    break;
	  default:
	    fprintf(stderr, "Unsupported type %d.\n", (int)ss->type);
	    assert(0);
      }

      cur->info = *ss;
      cur->info.tfname = strdup(ss->tfname);
      cur->is_user_defn = true;

      return cur;
}

PLI_INT32 vpi_put_userdata(vpiHandle ref, void*data)
{
      struct __vpiSysTaskCall*rfp = dynamic_cast<__vpiSysTaskCall*>(ref);
      if (rfp == 0)
	    return 0;

      rfp->userdata = data;
      return 1;
}

void* vpi_get_userdata(vpiHandle ref)
{
      struct __vpiSysTaskCall*rfp = dynamic_cast<__vpiSysTaskCall*>(ref);
      assert(rfp);

      return rfp->userdata;
}
