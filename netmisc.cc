/*
 * Copyright (c) 2001-2003 Stephen Williams (steve@icarus.com)
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
#ident "$Id: netmisc.cc,v 1.8.2.1 2006/05/15 03:55:23 steve Exp $"
#endif

# include "config.h"

# include  "netlist.h"
# include  "netmisc.h"
# include  "PExpr.h"

NetNet* add_to_net(Design*des, NetNet*sig, long val)
{
      if (val == 0)
	    return sig;

      NetScope*scope = sig->scope();
      unsigned long abs_val = (val >= 0)? val : (-val);
      unsigned width = sig->pin_count();

      verinum val_v (abs_val, width);

      NetConst*val_c = new NetConst(scope, scope->local_symbol(), val_v);

      NetNet*val_s = new NetNet(scope, scope->local_symbol(),
			      NetNet::IMPLICIT, width);
      val_s->local_flag(true);

      NetNet*res = new NetNet(scope, scope->local_symbol(),
			      NetNet::IMPLICIT, width);
      res->local_flag(true);

      NetAddSub*add = new NetAddSub(scope, scope->local_symbol(), width);

      for (unsigned idx = 0 ;  idx < width ;  idx += 1)
	    connect(sig->pin(idx), add->pin_DataA(idx));

      for (unsigned idx = 0 ;  idx < width ;  idx += 1)
	    connect(val_c->pin(idx), add->pin_DataB(idx));

      for (unsigned idx = 0 ;  idx < width ;  idx += 1)
	    connect(val_s->pin(idx), add->pin_DataB(idx));

      for (unsigned idx = 0 ;  idx < width ;  idx += 1)
	    connect(res->pin(idx), add->pin_Result(idx));

      if (val < 0)
	    add->attribute(perm_string::literal("LPM_Direction"), verinum("SUB"));
      else
	    add->attribute(perm_string::literal("LPM_Direction"), verinum("ADD"));

      des->add_node(add);
      des->add_node(val_c);

      return res;
}

NetNet* reduction_or(Design*des, NetNet*isig)
{
      NetScope*scope = isig->scope();

      NetLogic*olog = new NetLogic(scope, scope->local_symbol(),
				   isig->pin_count()+1, NetLogic::OR);
      olog->set_line(*isig);
      des->add_node(olog);

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, 1);
      osig->local_flag(true);
      osig->set_line(*isig);

      connect(olog->pin(0), osig->pin(0));
      for (unsigned idx = 0 ;  idx < isig->pin_count() ;  idx += 1)
	    connect(olog->pin(1+idx), isig->pin(idx));

      return osig;
}

NetExpr* elab_and_eval(Design*des, NetScope*scope, const PExpr*pe)
{
      NetExpr*tmp = pe->elaborate_expr(des, scope);
      if (tmp == 0)
	    return 0;

      if (NetExpr*tmp2 = tmp->eval_tree()) {
	    delete tmp;
	    tmp = tmp2;
      }

      return tmp;
}


/*
 * $Log: netmisc.cc,v $
 * Revision 1.8.2.1  2006/05/15 03:55:23  steve
 *  Fix synthesis of expressions with land of vectors.
 *
 * Revision 1.8  2004/02/20 18:53:35  steve
 *  Addtrbute keys are perm_strings.
 *
 * Revision 1.7  2004/02/18 17:11:57  steve
 *  Use perm_strings for named langiage items.
 *
 * Revision 1.6  2003/03/06 00:28:42  steve
 *  All NetObj objects have lex_string base names.
 *
 * Revision 1.5  2003/02/26 01:29:24  steve
 *  LPM objects store only their base names.
 *
 * Revision 1.4  2002/08/31 03:48:50  steve
 *  Fix reverse bit ordered bit select in continuous assignment.
 *
 * Revision 1.3  2002/08/12 01:35:00  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.2  2001/07/25 03:10:49  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.1  2001/02/11 02:15:52  steve
 *  Add the netmisc.cc source file.
 *
 */

