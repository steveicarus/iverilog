/*
 * Copyright (c) 2003 Stephen Williams (steve@icarus.com)
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
#ident "$Id: words.cc,v 1.3 2005/07/06 04:29:25 steve Exp $"
#endif

# include  "compile.h"
# include  "vpi_priv.h"
# include  <stdio.h>
# include  <stdlib.h>
# include  <string.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <assert.h>

#if 0
void compile_word(char*label, char*type, char*name)
{
      assert(strcmp(type, "real") == 0);
      free(type);

      vvp_fun_signal_real*fun = new vvp_fun_signal_real;
      vvp_net_t*net = new vvp_net_t;
      net->fun = fun;
      define_functor_symbol(label, net);

      vpiHandle obj = vpip_make_real_var(name, net);
      free(name);

      compile_vpi_symbol(label, obj);
      free(label);

      vpip_attach_to_current_scope(obj);
}
#endif

void compile_var_real(char*label, char*name, int msb, int lsb)
{
      vvp_fun_signal_real*fun = new vvp_fun_signal_real;
      vvp_net_t*net = new vvp_net_t;
      net->fun = fun;
      define_functor_symbol(label, net);

      vpiHandle obj = vpip_make_real_var(name, net);
      free(name);

      compile_vpi_symbol(label, obj);
      free(label);

      vpip_attach_to_current_scope(obj);
}

/*
 * A variable is a special functor, so we allocate that functor and
 * write the label into the symbol table.
 */
void compile_variable(char*label, char*name, int msb, int lsb,
		      char signed_flag)
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
      vpip_attach_to_current_scope(obj);

      free(label);
      free(name);
}

/*
 * Here we handle .net records from the vvp source:
 *
 *    <label> .net   <name>, <msb>, <lsb>, <input> ;
 *    <label> .net/s <name>, <msb>, <lsb>, <input> ;
 *
 * Create a VPI handle to represent it, and fill that handle in with
 * references into the net.
 */
void compile_net(char*label, char*name, int msb, int lsb, bool signed_flag,
		 unsigned argc, struct symb_s*argv)
{
      unsigned wid = ((msb > lsb)? msb-lsb : lsb-msb) + 1;

      vvp_net_t*node = new vvp_net_t;

      vvp_fun_signal*vsig = new vvp_fun_signal(wid);
      node->fun = vsig;

	/* Add the label into the functor symbol table. */
      define_functor_symbol(label, node);

      assert(argc == 1);

	/* Connect the source to my input. */
      inputs_connect(node, 1, argv);

	/* Make the vpiHandle for the reg. */
      vpiHandle obj = vpip_make_net(name, msb, lsb, signed_flag, node);
      compile_vpi_symbol(label, obj);
      vpip_attach_to_current_scope(obj);

      free(label);
      free(name);
      free(argv);
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

/*
 * $Log: words.cc,v $
 * Revision 1.3  2005/07/06 04:29:25  steve
 *  Implement real valued signals and arith nodes.
 *
 * Revision 1.2  2003/02/11 05:20:45  steve
 *  Include vpiRealVar objects in vpiVariables scan.
 *
 * Revision 1.1  2003/01/25 23:48:06  steve
 *  Add thread word array, and add the instructions,
 *  %add/wr, %cmp/wr, %load/wr, %mul/wr and %set/wr.
 *
 */

