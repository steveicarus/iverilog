/*
 * Copyright (c) 2002-2004 Stephen Williams (steve@icarus.com)
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
#ident "$Id: net_func.cc,v 1.8 2006/06/18 04:15:50 steve Exp $"
#endif

# include  "config.h"
# include  "netlist.h"
# include  "compiler.h"
# include  "PExpr.h"
# include  <iostream>

/*
 * To make a NetUserFunc device, make as many pins as there are ports
 * in the function. Get the port count from the function definition,
 * which accounts for all the inputs, plus one for the phantom output
 * that is the result.
 */
NetUserFunc::NetUserFunc(NetScope*s, perm_string n, NetScope*d)
: NetNode(s, n, d->func_def()->port_count()+1),
  def_(d)
{
      pin(0).set_dir(Link::OUTPUT);
      pin(0).set_name(def_->basename(), 0);

      for (unsigned idx = 1 ;  idx < pin_count() ;  idx += 1) {

	    pin(idx).set_dir(Link::INPUT);
	    pin(idx).set_name(perm_string::literal("D"), idx-1);
	    pin(idx).drive0(Link::HIGHZ);
	    pin(idx).drive1(Link::HIGHZ);
      }
}

NetUserFunc::~NetUserFunc()
{
}

unsigned NetUserFunc::port_width(unsigned port) const
{
      NetFuncDef*def = def_->func_def();

	/* Port 0 is the return port. */
      if (port == 0) {
	    const NetNet*sig = def->return_sig();
	    assert(sig);
	    return sig->vector_width();
      }

      port -= 1;
      assert(port < def->port_count());
      const NetNet*port_sig = def->port(port);

      return port_sig->vector_width();
}

const NetScope* NetUserFunc::def() const
{
      return def_;
}

/*
 * This method of the PECallFunction class checks that the parameters
 * of the PECallFunction match the function definition. This is used
 * during elaboration to validate the parameters before using them.
 */
bool PECallFunction::check_call_matches_definition_(Design*des, NetScope*dscope) const
{
      assert(dscope);

	/* How many parameters have I got? Normally the size of the
	   list is correct, but there is the special case of a list of
	   1 nil pointer. This is how the parser tells me of no
	   parameter. In other words, ``func()'' is 1 nil parameter. */

      unsigned parms_count = parms_.count();
      if ((parms_count == 1) && (parms_[0] == 0))
	    parms_count = 0;

      if (dscope->type() != NetScope::FUNC) {
	    cerr << get_line() << ": error: Attempt to call scope "
		 << dscope->name() << " as a function." << endl;
	    des->errors += 1;
	    return false;
      }

      if (parms_count != dscope->func_def()->port_count()) {
	    cerr << get_line() << ": error: Function " << dscope->name()
		 << " expects " << (dscope->func_def()->port_count()-1)
		 << " arguments, you passed " << parms_count << "."
		 << endl;
	    des->errors += 1;
	    return false;
      }

      return true;
}


NetSysFunc::NetSysFunc(NetScope*s, perm_string n,
		       const struct sfunc_return_type*def,
		       unsigned ports)
: NetNode(s, n, ports)
{
      def_ = def;

      pin(0).set_dir(Link::OUTPUT);
      pin(0).set_name(perm_string::literal("Q"), 0);

      for (unsigned idx = 1 ;  idx < pin_count() ;  idx += 1) {

	    pin(idx).set_dir(Link::INPUT);
	    pin(idx).set_name(perm_string::literal("D"), idx-1);
	    pin(idx).drive0(Link::HIGHZ);
	    pin(idx).drive1(Link::HIGHZ);
      }
}

NetSysFunc::~NetSysFunc()
{
}

const char*NetSysFunc::func_name() const
{
      return def_->name;
}

unsigned NetSysFunc::vector_width() const
{
      return def_->wid;
}


/*
 * $Log: net_func.cc,v $
 * Revision 1.8  2006/06/18 04:15:50  steve
 *  Add support for system functions in continuous assignments.
 *
 * Revision 1.7  2005/03/18 02:56:03  steve
 *  Add support for LPM_UFUNC user defined functions.
 *
 * Revision 1.6  2004/05/31 23:34:37  steve
 *  Rewire/generalize parsing an elaboration of
 *  function return values to allow for better
 *  speed and more type support.
 *
 * Revision 1.5  2004/02/18 17:11:56  steve
 *  Use perm_strings for named langiage items.
 *
 * Revision 1.4  2003/03/06 00:28:41  steve
 *  All NetObj objects have lex_string base names.
 *
 * Revision 1.3  2002/08/12 01:34:59  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.2  2002/03/31 04:07:40  steve
 *  Update for gcc 3.0
 *
 * Revision 1.1  2002/03/09 02:10:22  steve
 *  Add the NetUserFunc netlist node.
 *
 */

