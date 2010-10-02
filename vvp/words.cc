/*
 * Copyright (c) 2003-2010 Stephen Williams (steve@icarus.com)
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

static void __compile_var_real(char*label, char*name,
			       vvp_array_t array, unsigned long array_addr,
			       int msb, int lsb)
{
      vvp_net_t*net = new vvp_net_t;

      if (vpip_peek_current_scope()->is_automatic) {
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
            if (!vpip_peek_current_scope()->is_automatic)
                  schedule_init_vector(vvp_net_ptr_t(net,0), 0.0);
      }
      if (array) {
	    assert(!name);
	    array_attach_word(array, array_addr, obj);
      }
      free(label);
      delete[] name;
}

void compile_var_real(char*label, char*name, int msb, int lsb)
{
      __compile_var_real(label, name, 0, 0, msb, lsb);
}

void compile_varw_real(char*label, vvp_array_t array,
		       unsigned long addr,
		       int msb, int lsb)
{
      __compile_var_real(label, 0, array, addr, msb, lsb);
}

/*
 * A variable is a special functor, so we allocate that functor and
 * write the label into the symbol table.
 */
static void __compile_var(char*label, char*name,
			  vvp_array_t array, unsigned long array_addr,
			  int msb, int lsb, char signed_flag, bool local_flag)
{
      unsigned wid = ((msb > lsb)? msb-lsb : lsb-msb) + 1;

      vvp_net_t*net = new vvp_net_t;

      if (vpip_peek_current_scope()->is_automatic) {
	    vvp_fun_signal4_aa*tmp = new vvp_fun_signal4_aa(wid);
	    net->fil = tmp;
            net->fun = tmp;
      } else {
	    net->fil = new vvp_wire_vec4(wid, BIT4_X);
            net->fun = new vvp_fun_signal4_sa(wid);
      }
      vvp_signal_value*vfil = dynamic_cast<vvp_signal_value*>(net->fil);

      define_functor_symbol(label, net);

      vpiHandle obj = 0;
      if (! local_flag && !array) {
	      /* Make the vpiHandle for the reg. */
	    obj = (signed_flag > 1) ?
		  vpip_make_int(name, msb, lsb, net) :
		  vpip_make_reg(name, msb, lsb, signed_flag!=0, net);
	    compile_vpi_symbol(label, obj);
      }
	// If the signal has a name, then it goes into the current
	// scope as a signal.
      if (name) {
	    assert(!array);
	    if (obj) vpip_attach_to_current_scope(obj);
            if (!vpip_peek_current_scope()->is_automatic) {
		  vvp_vector4_t tmp;
		  vfil->vec4_value(tmp);
	          schedule_init_vector(vvp_net_ptr_t(net,0), tmp);
	    }
      }
	// If this is an array word, then it does not have a name, and
	// it is attached to the addressed array.
      if (array) {
	    assert(!name);
	    if (obj) array_attach_word(array, array_addr, obj);
      }
      free(label);
      delete[] name;
}

void compile_variable(char*label, char*name,
		      int msb, int lsb, char signed_flag, bool local_flag)
{
      __compile_var(label, name, 0, 0, msb, lsb, signed_flag, local_flag);
}

/*
* In this case, the variable it intended to be attached to the array
* as a word. The array_addr is the *canonical* address of the word in
* the array.
*
* This function is actually used by the compile_array function,
* instead of directly by the parser.
*/
void compile_variablew(char*label, vvp_array_t array,
		       unsigned long array_addr,
		       int msb, int lsb, char signed_flag)
{
      __compile_var(label, 0, array, array_addr, msb, lsb, signed_flag, false);
}

vvp_net_t* create_constant_node(const char*label, const char*val_str)
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
			       struct __vpiScope*scope,
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
      struct __vpiScope*scope_;
      unsigned array_addr_;
      bool local_flag_;
};

class __compile_net_resolv : public base_net_resolv {

    public:
      explicit __compile_net_resolv(char*ref_label, vvp_array_t array,
				    struct __vpiScope*scope,
				    char*my_label, char*name,
				    int msb, int lsb, unsigned array_addr,
				    bool signed_flag, bool net8_flag, bool local_flag)
      : base_net_resolv(ref_label, array, scope, my_label, name, array_addr, local_flag)
      { msb_ = msb;
	lsb_ = lsb;
	signed_flag_ = signed_flag;
	net8_flag_ = net8_flag;
      }

      ~__compile_net_resolv() { }

      bool resolve(bool message_flag);

    private:
      int msb_, lsb_;
      bool signed_flag_, net8_flag_;
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

static void __compile_net2(vvp_net_t*node, vvp_array_t array,
			   struct __vpiScope*scope,
			   char*my_label, char*name,
			   int msb, int lsb, unsigned array_addr,
			   bool signed_flag, bool net8_flag, bool local_flag)
{
      unsigned wid = ((msb > lsb)? msb-lsb : lsb-msb) + 1;
      assert(node);

      vvp_wire_base*vsig = dynamic_cast<vvp_wire_base*>(node->fil);

      if (vsig == 0) {
	    vsig = net8_flag
		  ? dynamic_cast<vvp_wire_base*>(new vvp_wire_vec8(wid))
		  : dynamic_cast<vvp_wire_base*>(new vvp_wire_vec4(wid,BIT4_Z));

	    node->fil = vsig;
      }

      vpiHandle obj = 0;
      if (! local_flag) {
	      /* Make the vpiHandle for the reg. */
	    obj = vpip_make_net(name, msb, lsb, signed_flag, node);
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
	    array_attach_word(array, array_addr, obj);
      else if (obj)
	    vpip_attach_to_scope(scope,obj);

      free(my_label);
      delete[] name;
}

static void __compile_net(char*label,
			  char*name, char*array_label, unsigned long array_addr,
			  int msb, int lsb,
			  bool signed_flag, bool net8_flag, bool local_flag,
			  unsigned argc, struct symb_s*argv)
{
      vvp_array_t array = array_label? array_find(array_label) : 0;
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
	    node = create_constant_node(label, argv[0].text);
      }
#endif
      if (node == 0) {
	    struct __vpiScope*scope = vpip_peek_current_scope();
	    __compile_net_resolv*res
		  = new __compile_net_resolv(argv[0].text,
					     array, scope, label, name,
					     msb, lsb, array_addr,
					     signed_flag, net8_flag, local_flag);
	    resolv_submit(res);
	    free(argv);
	    return;
      }
      assert(node);

      struct __vpiScope*scope = vpip_peek_current_scope();
      __compile_net2(node, array, scope, label, name, msb, lsb, array_addr,
		     signed_flag, net8_flag, local_flag);

      free(argv[0].text);
      free(argv);
}

bool __compile_net_resolv::resolve(bool msg_flag)
{
      vvp_net_t*node = vvp_net_lookup(label());
      if (node == 0) {
	    return false;
      }

      __compile_net2(node, array_, scope_, my_label_, name_, msb_, lsb_, array_addr_, signed_flag_, net8_flag_, local_flag_);
      return true;
}

void compile_net(char*label, char*name, int msb, int lsb,
		 bool signed_flag, bool net8_flag, bool local_flag,
		 unsigned argc, struct symb_s*argv)
{
      __compile_net(label, name, 0, 0,
		    msb, lsb, signed_flag, net8_flag, local_flag,
		    argc, argv);
}

void compile_netw(char*label, char*array_label, unsigned long array_addr,
		 int msb, int lsb,
		 bool signed_flag, bool net8_flag,
		 unsigned argc, struct symb_s*argv)
{
      __compile_net(label, 0, array_label, array_addr,
		    msb, lsb, signed_flag, net8_flag, false,
		    argc, argv);
}


class __compile_real_net_resolv : public base_net_resolv {

    public:
      explicit __compile_real_net_resolv(char*ref_label, vvp_array_t array,
					 struct __vpiScope*scope,
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
				struct __vpiScope*scope,
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
	    obj = vpip_make_real_var(name, node);
	    compile_vpi_symbol(my_label, obj);
      }

	// REMOVE ME! Giving the net a label is a legacy of the times
	// when the .net was a functor of its own. In the long run, we
	// must fix the code generator to not rely on the label of the
	// .net, then we will remove that label.
      define_functor_symbol(my_label, node);

     if (array)
	    array_attach_word(array, array_addr, obj);
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
      vvp_array_t array = array_label ? array_find(array_label) : 0;
      assert(array_label ? array!=0 : true);

      free(array_label);

      assert(argc == 1);
      vvp_net_t*node = vvp_net_lookup(argv[0].text);
      if (node == 0) {
	      /* No existing net, but the string value may be a
		 constant. In that case, we will wind up generating a
		 bufz node that can carry the constant value. */
	    node = create_constant_node(label, argv[0].text);
      }
      struct __vpiScope*scope = vpip_peek_current_scope();
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

void compile_aliasw(char*label, char*array_label, unsigned long array_addr,
                    int msb, int lsb, unsigned argc, struct symb_s*argv)
{
      vvp_array_t array = array_find(array_label);
      assert(array);

      assert(argc == 1);
      vvp_net_t*node = vvp_net_lookup(argv[0].text);

	/* Add the label into the functor symbol table. */
      assert(node);
      define_functor_symbol(label, node);

      vpiHandle obj = vvp_lookup_handle(argv[0].text);
      assert(obj);
      array_alias_word(array, array_addr, obj);

      free(label);
      free(array_label);
      free(argv[0].text);
      free(argv);
}
