/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: t-dll-api.cc,v 1.1 2000/09/18 01:24:32 steve Exp $"
#endif

# include  "t-dll.h"

/* THE FOLLOWING ARE FUNCTIONS THAT ARE CALLED FROM THE TARGET. */

extern "C" const char*ivl_get_flag(ivl_design_t des, const char*key)
{
      return ((const Design*)des)->get_flag(key).c_str();
}

extern "C" const char*ivl_get_root_name(ivl_design_t des)
{
      return ((const Design*)des)->find_root_scope()->name().c_str();
}

extern "C" ivl_logic_t ivl_get_logic_type(ivl_net_logic_t net)
{
      switch (net->dev_->type()) {
	  case NetLogic::AND:
	    return IVL_AND;
	  case NetLogic::OR:
	    return IVL_OR;
      }
      assert(0);
      return IVL_AND;
}

extern "C" unsigned ivl_get_logic_pins(ivl_net_logic_t net)
{
      return net->dev_->pin_count();
}

extern "C" ivl_nexus_t ivl_get_logic_pin(ivl_net_logic_t net, unsigned pin)
{
      return (ivl_nexus_t) (net->dev_->pin(pin).nexus());
}

extern "C" const char* ivl_get_nexus_name(ivl_nexus_t net)
{
      const Nexus*nex = (const Nexus*)net;
      return nex->name();
}

extern "C" ivl_process_type_t ivl_get_process_type(ivl_process_t net)
{
      return net->type_;
}

extern "C" ivl_statement_t ivl_get_process_stmt(ivl_process_t net)
{
      return net->stmt_;
}

extern "C" unsigned ivl_get_signal_pins(ivl_net_signal_t net)
{
      const NetNet*sig = (const NetNet*)net;
      return sig->pin_count();
}

/*
 * $Log: t-dll-api.cc,v $
 * Revision 1.1  2000/09/18 01:24:32  steve
 *  Get the structure for ivl_statement_t worked out.
 *
 */

