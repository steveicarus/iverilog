/*
 * Copyright (c) 2002-2021 Stephen Williams (steve@icarus.com)
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

# include  "config.h"
# include  "netlist.h"
# include  "compiler.h"
# include  "PExpr.h"
# include  <iostream>

using namespace std;

/*
 * To make a NetUserFunc device, make as many pins as there are ports
 * in the function. Get the port count from the function definition,
 * which accounts for all the inputs, plus one for the phantom output
 * that is the result.
 */
NetUserFunc::NetUserFunc(NetScope*s, perm_string n, NetScope*d,
                         NetEvWait*trigger__)
: NetNode(s, n, d->func_def()->port_count()+1),
  def_(d), trigger_(trigger__)
{
      pin(0).set_dir(Link::OUTPUT);

      for (unsigned idx = 1 ;  idx < pin_count() ;  idx += 1) {

	    pin(idx).set_dir(Link::INPUT);
	    pin(idx).drive0(IVL_DR_HiZ);
	    pin(idx).drive1(IVL_DR_HiZ);
      }
}

NetUserFunc::~NetUserFunc()
{
}

unsigned NetUserFunc::port_width(unsigned port) const
{
      NetFuncDef*fdef = def_->func_def();

	/* Port 0 is the return port. */
      if (port == 0) {
	    const NetNet*sig = fdef->return_sig();
	    assert(sig);
	    return sig->vector_width();
      }

      port -= 1;
      assert(port < fdef->port_count());
      const NetNet*port_sig = fdef->port(port);

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

      if (dscope->type() != NetScope::FUNC) {
	    cerr << get_fileline() << ": error: Attempt to call scope "
		 << scope_path(dscope) << " as a function." << endl;
	    des->errors += 1;
	    return false;
      }

      return true;
}


NetSysFunc::NetSysFunc(NetScope*s, perm_string n,
		       const struct sfunc_return_type*def,
		       unsigned ports, NetEvWait*trigger__)
: NetNode(s, n, ports), def_(def), trigger_(trigger__)
{
      pin(0).set_dir(Link::OUTPUT); // Q

      for (unsigned idx = 1 ;  idx < pin_count() ;  idx += 1) {

	    pin(idx).set_dir(Link::INPUT);
	    pin(idx).drive0(IVL_DR_HiZ);
	    pin(idx).drive1(IVL_DR_HiZ);
      }
}

NetSysFunc::~NetSysFunc()
{
}

const char*NetSysFunc::func_name() const
{
      return def_->name;
}

ivl_variable_type_t NetSysFunc::data_type() const
{
      return def_->type;
}

unsigned NetSysFunc::vector_width() const
{
      return def_->wid;
}
