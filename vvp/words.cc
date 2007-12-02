/*
 * Copyright (c) 2003-2007 Stephen Williams (steve@icarus.com)
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: words.cc,v 1.9 2007/04/10 01:26:16 steve Exp $"
#endif

# include  "compile.h"
# include  "vpi_priv.h"
# include  "array.h"
# include  "schedule.h"
# include  <stdio.h>
# include  <stdlib.h>
# include  <string.h>
# include  <iostream>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <assert.h>

static void __compile_var_real(char*label, char*name,
			       vvp_array_t array, unsigned long array_addr,
			       int msb, int lsb)
{
      vvp_fun_signal_real*fun = new vvp_fun_signal_real;
      vvp_net_t*net = new vvp_net_t;
      net->fun = fun;
      define_functor_symbol(label, net);

      vpiHandle obj = vpip_make_real_var(name, net);

      compile_vpi_symbol(label, obj);

      if (!array && name) {
	    vpip_attach_to_current_scope(obj);
	    schedule_init_vector(vvp_net_ptr_t(net,0), fun->real_value());
      }
      if (array) {
	    array_attach_word(array, array_addr, obj);
      }
      free(label);
      if (name) free(name);
}

void compile_var_real(char*label, char*name, int msb, int lsb)
{
      __compile_var_real(label, name, 0, 0, msb, lsb);
}

void compile_varw_real(char*label, char*name, vvp_array_t array,
		       unsigned long addr,
		       int msb, int lsb)
{
      __compile_var_real(label, name, array, addr, msb, lsb);
}

/*
 * A variable is a special functor, so we allocate that functor and
 * write the label into the symbol table.
 */
static void __compile_var(char*label, char*name,
			  vvp_array_t array, unsigned long array_addr,
			  int msb, int lsb, char signed_flag)
{
      unsigned wid = ((msb > lsb)? msb-lsb : lsb-msb) + 1;

      vvp_fun_signal*vsig = new vvp_fun_signal(wid);
      vvp_net_t*node = new vvp_net_t;

      node->fun = vsig;
      define_functor_symbol(label, node);

	/* Make the vpiHandle for the reg. */
      vpiHandle obj = (signed_flag > 1) ?
			vpip_make_int(name, msb, lsb, node) :
			vpip_make_reg(name, msb, lsb, signed_flag!=0, node);
      compile_vpi_symbol(label, obj);
	// If the signal has a name, then it goes into the current
	// scope as a signal.
      if (!array && name) {
	    vpip_attach_to_current_scope(obj);
	    schedule_init_vector(vvp_net_ptr_t(node,0), vsig->vec4_value());
      }
	// If this is an array word, then it does not have a name, and
	// it is attached to the addressed array.
      if (array) {
	    array_attach_word(array, array_addr, obj);
      }
      free(label);
      if (name) free(name);
}

void compile_variable(char*label, char*name,
		      int msb, int lsb, char signed_flag)
{
      __compile_var(label, name, 0, 0, msb, lsb, signed_flag);
}

/*
* In this case, the variable it intended to be attached to the array
* as a word. The array_addr is the *canonical* address of the word in
* the array.
*
* This function is actually used by the compile_array function,
* instead of directly by the parser.
*/
void compile_variablew(char*label, char*name, vvp_array_t array,
		       unsigned long array_addr,
		       int msb, int lsb, char signed_flag)
{
      __compile_var(label, name, array, array_addr, msb, lsb, signed_flag);
}

/*
 * Here we handle .net records from the vvp source:
 *
 *    <label> .net   <name>, <msb>, <lsb>, <input> ;
 *    <label> .net/s <name>, <msb>, <lsb>, <input> ;
 *    <label> .net8   <name>, <msb>, <lsb>, <input> ;
 *    <label> .net8/s <name>, <msb>, <lsb>, <input> ;
 *
 * Create a VPI handle to represent it, and fill that handle in with
 * references into the net.
 */
static void __compile_net(char*label, char*name,
			  char*array_label, unsigned long array_addr,
			  int msb, int lsb,
			  bool signed_flag, bool net8_flag,
			  unsigned argc, struct symb_s*argv)
{
      unsigned wid = ((msb > lsb)? msb-lsb : lsb-msb) + 1;

      vvp_net_t*node = new vvp_net_t;

      vvp_array_t array = array_label? array_find(array_label) : 0;
      assert(array_label? array!=0 : true);

      vvp_fun_signal_base*vsig = net8_flag
	    ? dynamic_cast<vvp_fun_signal_base*>(new vvp_fun_signal8(wid))
	    : dynamic_cast<vvp_fun_signal_base*>(new vvp_fun_signal(wid,BIT4_Z));
      node->fun = vsig;

	/* Add the label into the functor symbol table. */
      define_functor_symbol(label, node);

      assert(argc == 1);

	/* Connect the source to my input. */
      inputs_connect(node, 1, argv);

	/* Make the vpiHandle for the reg. */
      vpiHandle obj = vpip_make_net(name, msb, lsb, signed_flag, node);
	/* This attaches the label to the vpiHandle */
      compile_vpi_symbol(label, obj);
        /* If this is an array word, then attach it to the
	   array. Otherwise, attach it to the current scope. */
      if (array)
	    array_attach_word(array, array_addr, obj);
      else
	    vpip_attach_to_current_scope(obj);

      free(label);
      if (name) free(name);
      if (array_label) free(array_label);
      free(argv);
}

void compile_net(char*label, char*name,
		 int msb, int lsb,
		 bool signed_flag, bool net8_flag,
		 unsigned argc, struct symb_s*argv)
{
      __compile_net(label, name, 0, 0,
		    msb, lsb, signed_flag, net8_flag,
		    argc, argv);
}

void compile_netw(char*label, char*array_label, unsigned long array_addr,
		 int msb, int lsb,
		 bool signed_flag, bool net8_flag,
		 unsigned argc, struct symb_s*argv)
{
        /* Get the array name and build the word name. */
      const char *name = get_array_name(array_label);
      unsigned len = strlen(name) + 32;
      char *nbuf = (char *)malloc(len);
      snprintf(nbuf, len, "%s[%lu]", name, array_addr +
               get_array_base(array_label));
      __compile_net(label, strdup(nbuf), array_label, array_addr,
		    msb, lsb, signed_flag, net8_flag,
		    argc, argv);
      free(nbuf);
}

void compile_net_real(char*label, char*name, int msb, int lsb,
		      unsigned argc, struct symb_s*argv)
{
      vvp_net_t*net = new vvp_net_t;

      vvp_fun_signal_real*fun = new vvp_fun_signal_real;
      net->fun = fun;

	/* Add the label into the functor symbol table. */
      define_functor_symbol(label, net);

      assert(argc == 1);

	/* Connect the source to my input. */
      inputs_connect(net, 1, argv);

	/* Make the vpiHandle for the reg. */
      vpiHandle obj = vpip_make_real_var(name, net);
      compile_vpi_symbol(label, obj);
      vpip_attach_to_current_scope(obj);

      free(label);
      free(name);
      free(argv);
}

void compile_alias(char*label, char*name, int msb, int lsb, bool signed_flag,
		 unsigned argc, struct symb_s*argv)
{
      assert(argc == 1);

      vvp_net_t*node = vvp_net_lookup(argv[0].text);

	/* Add the label into the functor symbol table. */
      define_functor_symbol(label, node);


	/* Make the vpiHandle for the reg. */
      vpiHandle obj = vpip_make_net(name, msb, lsb, signed_flag, node);
      compile_vpi_symbol(label, obj);
      vpip_attach_to_current_scope(obj);

      free(label);
      free(name);
      free(argv[0].text);
      free(argv);
}

void compile_alias_real(char*label, char*name, int msb, int lsb,
		      unsigned argc, struct symb_s*argv)
{
      assert(argc == 1);

      vvp_net_t*node = vvp_net_lookup(argv[0].text);

	/* Add the label into the functor symbol table. */
      define_functor_symbol(label, node);


	/* Make the vpiHandle for the reg. */
      vpiHandle obj = vpip_make_real_var(name, node);
      compile_vpi_symbol(label, obj);
      vpip_attach_to_current_scope(obj);

      free(label);
      free(name);
      free(argv[0].text);
      free(argv);
}

