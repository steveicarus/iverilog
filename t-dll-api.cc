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
#ident "$Id: t-dll-api.cc,v 1.4 2000/09/23 05:15:07 steve Exp $"
#endif

# include  "t-dll.h"

/* THE FOLLOWING ARE FUNCTIONS THAT ARE CALLED FROM THE TARGET. */

extern "C" ivl_expr_type_t ivl_expr_type(ivl_expr_t net)
{
      return net->type_;
}

extern "C" const char*ivl_get_flag(ivl_design_t des, const char*key)
{
      return ((const Design*)des)->get_flag(key).c_str();
}

extern "C" const char*ivl_get_root_name(ivl_design_t des)
{
      return ((const Design*)des)->find_root_scope()->name().c_str();
}

extern "C" const char* ivl_expr_string(ivl_expr_t net)
{
      assert(net->type_ == IVL_EX_STRING);
      return net->u_.string_.value_;
}

extern "C" ivl_logic_t ivl_get_logic_type(ivl_net_logic_t net)
{
      switch (net->dev_->type()) {
	  case NetLogic::AND:
	    return IVL_LO_AND;
	  case NetLogic::OR:
	    return IVL_LO_OR;
      }
      assert(0);
      return IVL_LO_NONE;
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

extern "C" ivl_statement_type_t ivl_statement_type(ivl_statement_t net)
{
      return net->type_;
}

extern "C" unsigned ivl_stmt_block_count(ivl_statement_t net)
{
      assert(net->type_ == IVL_ST_BLOCK);
      return net->u_.block_.nstmt_;
}

extern "C" ivl_statement_t ivl_stmt_block_stmt(ivl_statement_t net,
					       unsigned i)
{
      assert(net->type_ == IVL_ST_BLOCK);
      assert(i < net->u_.block_.nstmt_);
      return net->u_.block_.stmt_ + i;
}

extern "C" ivl_statement_t ivl_stmt_cond_false(ivl_statement_t net)
{
      assert(net->type_ == IVL_ST_CONDIT);
      if (net->u_.condit_.stmt_[1].type_ == IVL_ST_NONE)
	    return 0;
      else
	    return net->u_.condit_.stmt_ + 1;
}

extern "C" ivl_statement_t ivl_stmt_cond_true(ivl_statement_t net)
{
      assert(net->type_ == IVL_ST_CONDIT);
      if (net->u_.condit_.stmt_[0].type_ == IVL_ST_NONE)
	    return 0;
      else
	    return net->u_.condit_.stmt_ + 0;
}

extern "C" unsigned long ivl_stmt_delay_val(ivl_statement_t net)
{
      assert(net->type_ == IVL_ST_DELAY);
      return net->u_.delay_.delay_;
}

extern "C" const char* ivl_stmt_name(ivl_statement_t net)
{
      switch (net->type_) {
	  case IVL_ST_STASK:
	    return net->u_.stask_.name_;
	  default:
	    assert(0);
      }

      return 0;
}

extern "C" ivl_expr_t ivl_stmt_parm(ivl_statement_t net, unsigned idx)
{
      switch (net->type_) {
	  case IVL_ST_STASK:
	    assert(idx < net->u_.stask_.nparm_);
	    return net->u_.stask_.parms_[idx];

	  default:
	    assert(0);
      }
      return 0;
}

extern "C" unsigned ivl_stmt_parm_count(ivl_statement_t net)
{
      switch (net->type_) {
	  case IVL_ST_STASK:
	    return net->u_.stask_.nparm_;
	  default:
	    assert(0);
      }
      return 0;
}

extern "C" ivl_statement_t ivl_stmt_sub_stmt(ivl_statement_t net)
{
      switch (net->type_) {
	  case IVL_ST_DELAY:
	    return net->u_.delay_.stmt_;
	  case IVL_ST_WAIT:
	    return net->u_.wait_.stmt_;
	  case IVL_ST_WHILE:
	    return net->u_.while_.stmt_;
	  default:
	    assert(0);
      }

      return 0;
}

/*
 * $Log: t-dll-api.cc,v $
 * Revision 1.4  2000/09/23 05:15:07  steve
 *  Add enough tgt-verilog code to support hello world.
 *
 * Revision 1.3  2000/09/22 03:58:30  steve
 *  Access to the name of a system task call.
 *
 * Revision 1.2  2000/09/19 04:15:27  steve
 *  Introduce the means to get statement types.
 *
 * Revision 1.1  2000/09/18 01:24:32  steve
 *  Get the structure for ivl_statement_t worked out.
 *
 */

