/*
 * Copyright (c) 2003-2022 Stephen Williams (steve@icarus.com)
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
# include  "array.h"
# include  "vvp_net_sig.h"
# include  "logic.h"
# include  "schedule.h"
#ifdef CHECK_WITH_VALGRIND
# include  "vvp_cleanup.h"
#endif
# include  <cstdio>
# include  <cstdlib>
# include  <cstring>
# include  <iostream>
# include  <cassert>

using namespace std;

static void __compile_var_real(char*label, char*name,
			       vvp_array_t array, unsigned long array_addr)
{
      vvp_net_t*net = new vvp_net_t;

      if (vpip_peek_current_scope()->is_automatic()) {
	    vvp_fun_signal_real_aa*tmp = new vvp_fun_signal_real_aa;
	    net->fil = tmp;
	    net->fun = tmp;
      } else {
	    net->fil = new vvp_wire_real;
	    net->fun = new vvp_fun_signal_real_sa;
      }

      define_functor_symbol(label, net);

      vpiHandle obj = vpip_make_real_var(name, net);
      compile_vpi_symbol(label, obj);

      if (name) {
	    assert(!array);
	    vpip_attach_to_current_scope(obj);
            if (!vpip_peek_current_scope()->is_automatic())
                  schedule_init_vector(vvp_net_ptr_t(net,0), 0.0);
      }
      if (array) {
	    assert(!name);
	    array->attach_word(array_addr, obj);
      }
      free(label);
      delete[] name;
}

void compile_var_real(char*label, char*name)
{
      __compile_var_real(label, name, 0, 0);
}

void compile_var_string(char*label, char*name)
{
      vvp_net_t*net = new vvp_net_t;

      if (vpip_peek_current_scope()->is_automatic()) {
	    vvp_fun_signal_string_aa*tmp = new vvp_fun_signal_string_aa;
	    net->fil = tmp;
	    net->fun = tmp;
      } else {
	    net->fil = 0;
	    net->fun = new vvp_fun_signal_string_sa;
      }

      define_functor_symbol(label, net);

      vpiHandle obj = vpip_make_string_var(name, net);
      compile_vpi_symbol(label, obj);

      vpip_attach_to_current_scope(obj);
      free(label);
      delete[] name;
}

void compile_var_darray(char*label, char*name, unsigned size)
{
      vvp_net_t*net = new vvp_net_t;

      if (vpip_peek_current_scope()->is_automatic()) {
	    vvp_fun_signal_object_aa*tmp = new vvp_fun_signal_object_aa(size);
	    net->fil = tmp;
	    net->fun = tmp;
      } else {
	    net->fil = 0;
	    net->fun = new vvp_fun_signal_object_sa(size);
      }

      define_functor_symbol(label, net);

      vpiHandle obj = vpip_make_darray_var(name, net);
      compile_vpi_symbol(label, obj);

      vpip_attach_to_current_scope(obj);
      free(label);
      delete[] name;
}

void compile_var_queue(char*label, char*name, unsigned size)
{
      vvp_net_t*net = new vvp_net_t;

      if (vpip_peek_current_scope()->is_automatic()) {
	    vvp_fun_signal_object_aa*tmp = new vvp_fun_signal_object_aa(size);
	    net->fil = tmp;
	    net->fun = tmp;
      } else {
	    net->fil = 0;
	    net->fun = new vvp_fun_signal_object_sa(size);
      }

      define_functor_symbol(label, net);

      vpiHandle obj = vpip_make_queue_var(name, net);
      compile_vpi_symbol(label, obj);

      vpip_attach_to_current_scope(obj);
      free(label);
      delete[] name;
}

void compile_var_cobject(char*label, char*name)
{
      vvp_net_t*net = new vvp_net_t;

      if (vpip_peek_current_scope()->is_automatic()) {
	    vvp_fun_signal_object_aa*tmp = new vvp_fun_signal_object_aa(1);
	    net->fil = tmp;
	    net->fun = tmp;
      } else {
	    net->fil = 0;
	    net->fun = new vvp_fun_signal_object_sa(1);
      }

      define_functor_symbol(label, net);

      vpiHandle obj = vpip_make_cobject_var(name, net);
      compile_vpi_symbol(label, obj);

      vpip_attach_to_current_scope(obj);
      free(label);
      delete[] name;
}

/*
 * A variable is a special functor, so we allocate that functor and
 * write the label into the symbol table.
 */
void compile_variable(char*label, char*name,
		      int msb, int lsb, int vpi_type_code,
		      bool signed_flag, bool local_flag)
{
      unsigned wid = ((msb > lsb)? msb-lsb : lsb-msb) + 1;

      vvp_net_t*net = new vvp_net_t;

      vvp_bit4_t init;
      if (vpi_type_code == vpiIntVar)
	    init = BIT4_0;
      else
	    init = BIT4_X;

      if (vpip_peek_current_scope()->is_automatic()) {
	    vvp_fun_signal4_aa*tmp = new vvp_fun_signal4_aa(wid, init);
	    net->fil = tmp;
            net->fun = tmp;
      } else {
	    net->fil = new vvp_wire_vec4(wid, init);
            net->fun = new vvp_fun_signal4_sa(wid);
      }
      vvp_signal_value*vfil = dynamic_cast<vvp_signal_value*>(net->fil);

      define_functor_symbol(label, net);

      vpiHandle obj = 0;
      if (! local_flag) {
	      /* Make the vpiHandle for the reg. */
	    switch (vpi_type_code) {
		case vpiLogicVar:
		  obj = vpip_make_var4(name, msb, lsb, signed_flag, net);
		  break;
		case vpiIntegerVar:
		  obj = vpip_make_int4(name, msb, lsb, net);
		  break;
		case vpiIntVar: // This handles all the atom2 int types
		  obj = vpip_make_int2(name, msb, lsb, signed_flag, net);
		  break;
		default:
		  fprintf(stderr, "internal error: %s: vpi_type_code=%d\n", name, vpi_type_code);
		  break;
	    }
	    assert(obj);
	    compile_vpi_symbol(label, obj);
      }
	// If the signal has a name, then it goes into the current
	// scope as a signal.
      if (name) {
	    if (obj) vpip_attach_to_current_scope(obj);
            if (!vpip_peek_current_scope()->is_automatic()) {
		  vvp_vector4_t tmp;
		  vfil->vec4_value(tmp);
	          schedule_init_vector(vvp_net_ptr_t(net,0), tmp);
	    }
      }

      free(label);
      delete[] name;
}


vvp_net_t* create_constant_node(const char*val_str)
{
      if (c4string_test(val_str)) {
	    vvp_net_t*net = new vvp_net_t;
	    net->fun = new vvp_fun_bufz;
	    schedule_init_vector(vvp_net_ptr_t(net,0), c4string_to_vector4(val_str));
	    return net;
      }

      if (c8string_test(val_str)) {
	    vvp_net_t*net = new vvp_net_t;
	    net->fun = new vvp_fun_bufz;
	    schedule_init_vector(vvp_net_ptr_t(net,0), c8string_to_vector8(val_str));
	    return net;
      }

      if (crstring_test(val_str)) {
	    vvp_net_t*net = new vvp_net_t;
	    net->fun = new vvp_fun_bufz;
	    schedule_init_vector(vvp_net_ptr_t(net,0), crstring_to_double(val_str));
	    return net;
      }

      return 0;
}

class base_net_resolv : public resolv_list_s {
    public:
      explicit base_net_resolv(char*ref_label, vvp_array_t array,
			       __vpiScope*scope,
			       char*my_label, char*name,
			       unsigned array_addr, bool local_flag)
      : resolv_list_s(ref_label)
      { my_label_ = my_label;
	array_ = array;
	name_ = name;
	scope_ = scope;
	array_addr_ = array_addr;
	local_flag_ = local_flag;
      }

    protected:
      char*my_label_;
      vvp_array_t array_;
      char*name_;
      __vpiScope*scope_;
      unsigned array_addr_;
      bool local_flag_;
};

class __compile_net_resolv : public base_net_resolv {

    public:
      explicit __compile_net_resolv(char*ref_label, vvp_array_t array,
				    __vpiScope*scope,
				    char*my_label, char*name,
				    int msb, int lsb, unsigned array_addr,
				    int vpi_type_code, bool signed_flag, bool local_flag)
      : base_net_resolv(ref_label, array, scope, my_label, name, array_addr, local_flag)
      { msb_ = msb;
	lsb_ = lsb;
	vpi_type_code_ = vpi_type_code;
	signed_flag_ = signed_flag;
      }

      ~__compile_net_resolv() { }

      bool resolve(bool message_flag);

    private:
      int msb_, lsb_;
      int vpi_type_code_;
      bool signed_flag_;
};

/*
 * Here we handle .net records from the vvp source:
 *
 *    .net   <name>, <msb>, <lsb>, <input> ;
 *    .net/s <name>, <msb>, <lsb>, <input> ;
 *    .net8   <name>, <msb>, <lsb>, <input> ;
 *    .net8/s <name>, <msb>, <lsb>, <input> ;
 *
 * Create a VPI handle to represent it, and fill that handle in with
 * references into the net.
 */

static void do_compile_net(vvp_net_t*node, vvp_array_t array,
			   __vpiScope*scope,
			   char*my_label, char*name,
			   int msb, int lsb, unsigned array_addr,
			   int vpi_type_code, bool signed_flag, bool local_flag)
{
      unsigned wid = ((msb > lsb)? msb-lsb : lsb-msb) + 1;
      assert(node);

      vvp_wire_base*vsig = dynamic_cast<vvp_wire_base*>(node->fil);

      if (vsig == 0) {
	    switch (vpi_type_code) {
		case vpiIntVar:
		  vsig = new vvp_wire_vec4(wid,BIT4_0);
		  break;
		case vpiLogicVar:
		  vsig = new vvp_wire_vec4(wid,BIT4_Z);
		  break;
		case -vpiLogicVar:
		  vsig = new vvp_wire_vec8(wid);
		  break;
	    }
	    assert(vsig);
	    node->fil = vsig;
      }

      vpiHandle obj = 0;
      if (! local_flag) {
	      /* Make the vpiHandle for the reg. */
	    obj = vpip_make_net4(scope, name, msb, lsb, signed_flag, node);
	      /* This attaches the label to the vpiHandle */
	    compile_vpi_symbol(my_label, obj);
      }
#ifdef CHECK_WITH_VALGRIND
      else pool_local_net(node);
#endif

	// REMOVE ME! Giving the net a label is a legacy of the times
	// when the .net was a functor of its own. In the long run, we
	// must fix the code generator to not rely on the label of the
	// .net, then we will remove that label.
      define_functor_symbol(my_label, node);

      if (array)
	    array->attach_word(array_addr, obj);
      else if (obj)
	    vpip_attach_to_scope(scope,obj);

      free(my_label);
      delete[] name;
}

static void __compile_net(char*label,
			  char*name, char*array_label, unsigned long array_addr,
			  int msb, int lsb,
			  int vpi_type_code, bool signed_flag, bool local_flag,
			  unsigned argc, struct symb_s*argv)
{
      vvp_array_t array = array_label? array_find(array_label) : NULL;
      assert(array_label ? array!=0 : true);

      free(array_label);

      assert(argc == 1);
      vvp_net_t*node = vvp_net_lookup(argv[0].text);
#if 1
      if (node == 0) {
	      /* No existing net, but the string value may be a
		 constant. In that case, we will wind up generating a
		 bufz node that can carry the constant value.

	         NOTE: This is a hack! The code generator should be
	         fixed so that this is no longer needed. */
	    node = create_constant_node(argv[0].text);
      }
#endif
      if (node == 0) {
	    __vpiScope*scope = vpip_peek_current_scope();
	    __compile_net_resolv*res
		  = new __compile_net_resolv(argv[0].text,
					     array, scope, label, name,
					     msb, lsb, array_addr,
					     vpi_type_code, signed_flag, local_flag);
	    resolv_submit(res);
	    free(argv);
	    return;
      }
      assert(node);

      __vpiScope*scope = vpip_peek_current_scope();
      do_compile_net(node, array, scope, label, name, msb, lsb, array_addr,
		     vpi_type_code, signed_flag, local_flag);

      free(argv[0].text);
      free(argv);
}

bool __compile_net_resolv::resolve(bool msg_flag)
{
      vvp_net_t*node = vvp_net_lookup(label());
      if (node == 0) {
	    if (msg_flag)
		  cerr << "Unable to resolve label " << label() << endl;
	    return false;
      }

      do_compile_net(node, array_, scope_, my_label_, name_, msb_, lsb_, array_addr_, vpi_type_code_, signed_flag_, local_flag_);
      return true;
}

void compile_net(char*label, char*name, int msb, int lsb,
		 int vpi_type_code, bool signed_flag, bool local_flag,
		 unsigned argc, struct symb_s*argv)
{
      __compile_net(label, name, 0, 0, msb, lsb,
		    vpi_type_code, signed_flag, local_flag,
		    argc, argv);
}

void compile_netw(char*label, char*array_label, unsigned long array_addr,
		  int msb, int lsb, int vpi_type_code, bool signed_flag,
		  unsigned argc, struct symb_s*argv)
{
      __compile_net(label, 0, array_label, array_addr,
		    msb, lsb, vpi_type_code, signed_flag, false,
		    argc, argv);
}

class __compile_real_net_resolv : public base_net_resolv {

    public:
      explicit __compile_real_net_resolv(char*ref_label, vvp_array_t array,
					 __vpiScope*scope,
					 char*my_label, char*name,
					 unsigned array_addr, bool local_flag)
      : base_net_resolv(ref_label, array, scope, my_label, name, array_addr, local_flag)
      {
      }

      ~__compile_real_net_resolv() { }

      bool resolve(bool message_flag);

    private:
};

static void __compile_real_net2(vvp_net_t*node, vvp_array_t array,
				__vpiScope*scope,
				char*my_label, char*name,
				unsigned array_addr, bool local_flag)
{
      vvp_wire_base*fil = dynamic_cast<vvp_wire_base*> (node->fil);

      if (fil == 0) {
	    fil = new vvp_wire_real;
	    node->fil = fil;
      }

      vpiHandle obj = 0;
      if (!local_flag) {
	    obj = vpip_make_real_net(scope, name, node);
	    compile_vpi_symbol(my_label, obj);
      }
#ifdef CHECK_WITH_VALGRIND
      else pool_local_net(node);
#endif

	// REMOVE ME! Giving the net a label is a legacy of the times
	// when the .net was a functor of its own. In the long run, we
	// must fix the code generator to not rely on the label of the
	// .net, then we will remove that label.
      define_functor_symbol(my_label, node);

     if (array)
	    array->attach_word(array_addr, obj);
      else if (obj)
	    vpip_attach_to_scope(scope, obj);

      free(my_label);
      delete[] name;
}

static void __compile_real(char*label, char*name,
                           char*array_label, unsigned long array_addr,
                           int msb, int lsb, bool local_flag,
                           unsigned argc, struct symb_s*argv)
{
      assert(msb == 0 && lsb == 0);
      vvp_array_t array = array_label ? array_find(array_label) : NULL;
      assert(array_label ? array!=0 : true);

      free(array_label);

      assert(argc == 1);
      vvp_net_t*node = vvp_net_lookup(argv[0].text);
      if (node == 0) {
	      /* No existing net, but the string value may be a
		 constant. In that case, we will wind up generating a
		 bufz node that can carry the constant value. */
	    node = create_constant_node(argv[0].text);
      }
      __vpiScope*scope = vpip_peek_current_scope();
      if (node == 0) {
	    __compile_real_net_resolv*res
		  = new __compile_real_net_resolv(argv[0].text, array,
						  scope, label, name,
						  array_addr, local_flag);
	    resolv_submit(res);
	    free(argv);
	    return;
      }

      assert(node);
      __compile_real_net2(node, array, scope, label, name, array_addr,
                          local_flag);
      free(argv[0].text);
      free(argv);
}

bool __compile_real_net_resolv::resolve(bool msg_flag)
{
      vvp_net_t*node = vvp_net_lookup(label());
      if (node == 0) {
	    if (msg_flag)
		  cerr << "Unable to resolve label " << label() << endl;
	    return false;
      }

      __compile_real_net2(node, array_, scope_, my_label_, name_, array_addr_, local_flag_);
      return true;
}

void compile_net_real(char*label, char*name, int msb, int lsb, bool local_flag,
		      unsigned argc, struct symb_s*argv)
{
      __compile_real(label, name, 0, 0,
                     msb, lsb, local_flag, argc, argv);
}

void compile_netw_real(char*label, char*array_label, unsigned long array_addr,
                       int msb, int lsb,
                       unsigned argc, struct symb_s*argv)
{
      __compile_real(label, 0, array_label, array_addr,
                     msb, lsb, false, argc, argv);
}
