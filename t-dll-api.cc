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
#ident "$Id: t-dll-api.cc,v 1.12 2000/10/15 04:46:23 steve Exp $"
#endif

# include  "t-dll.h"

/* THE FOLLOWING ARE FUNCTIONS THAT ARE CALLED FROM THE TARGET. */

extern "C" const char*ivl_design_flag(ivl_design_t des, const char*key)
{
      return des->self->get_flag(key).c_str();
}

extern "C" int ivl_design_process(ivl_design_t des, ivl_process_f func)
{
      for (ivl_process_t idx = des->threads_;  idx;  idx = idx->next_) {
	    int rc = (func)(idx);
	    if (rc != 0)
		  return rc;
      }

      return 0;
}

extern "C" ivl_scope_t ivl_design_root(ivl_design_t des)
{
      return des->root_;
}

extern "C" ivl_expr_type_t ivl_expr_type(ivl_expr_t net)
{
      if (net == 0)
	    return IVL_EX_NONE;
      return net->type_;
}

extern "C" const char*ivl_const_bits(ivl_net_const_t net)
{
      assert(net);
      if (net->width_ <= sizeof(char*))
	    return net->b.bit_;
      else
	    return net->b.bits_;
}

extern "C" ivl_nexus_t ivl_const_pin(ivl_net_const_t net, unsigned idx)
{
      assert(net);
      assert(idx < net->width_);
      if (net->width_ == 1)
	    return net->n.pin_;
      else
	    return net->n.pins_[idx];
}

extern "C" unsigned ivl_const_pins(ivl_net_const_t net)
{
      assert(net);
      return net->width_;
}

extern "C" int ivl_const_signed(ivl_net_const_t net)
{
      assert(net);
      return net->signed_ == 1;
}

extern "C" const char* ivl_expr_bits(ivl_expr_t net)
{
      assert(net && (net->type_ == IVL_EX_NUMBER));
      return net->u_.number_.bits_;
}

extern "C" const char* ivl_expr_name(ivl_expr_t net)
{
      switch (net->type_) {

	  case IVL_EX_SFUNC:
	    return net->u_.sfunc_.name_;

	  case IVL_EX_SIGNAL:
	    return net->u_.subsig_.name_;

	  default:
	    assert(0);
      }
      return 0;
}

extern "C" char ivl_expr_opcode(ivl_expr_t net)
{
      assert(net);
      switch (net->type_) {
	  case IVL_EX_BINARY:
	    return net->u_.binary_.op_;

	  default:
	    assert(0);
      }
      return 0;
}

extern "C" ivl_expr_t ivl_expr_oper1(ivl_expr_t net)
{
      assert(net);
      switch (net->type_) {
	  case IVL_EX_BINARY:
	    return net->u_.binary_.lef_;

	  default:
	    assert(0);
      }

      return 0;
}

extern "C" ivl_expr_t ivl_expr_oper2(ivl_expr_t net)
{
      assert(net);
      switch (net->type_) {
	  case IVL_EX_BINARY:
	    return net->u_.binary_.rig_;

	  default:
	    assert(0);
      }

      return 0;
}

extern "C" int ivl_expr_signed(ivl_expr_t net)
{
      assert(net);
      return net->signed_;
}

extern "C" const char* ivl_expr_string(ivl_expr_t net)
{
      assert(net->type_ == IVL_EX_STRING);
      return net->u_.string_.value_;
}

extern "C" unsigned ivl_expr_width(ivl_expr_t net)
{
      assert(net);
      return net->width_;
}

extern "C" const char* ivl_logic_name(ivl_net_logic_t net)
{
      assert(net);
      return net->name_;
}

extern "C" const char* ivl_logic_basename(ivl_net_logic_t net)
{
      const char*nam = net->name_;
      nam += strlen(ivl_scope_name(net->scope_));
      assert(*nam == '.');
      nam += 1;
      return nam;
}

extern "C" ivl_logic_t ivl_logic_type(ivl_net_logic_t net)
{
      return net->type_;
}

extern "C" unsigned ivl_logic_pins(ivl_net_logic_t net)
{
      return net->npins_;
}

extern "C" ivl_nexus_t ivl_logic_pin(ivl_net_logic_t net, unsigned pin)
{
      assert(pin < net->npins_);
      return net->pins_[pin];
}

extern "C" const char* ivl_nexus_name(ivl_nexus_t net)
{
      assert(net);
      return net->name_;
}

extern "C" unsigned ivl_nexus_ptrs(ivl_nexus_t net)
{
      assert(net);
      return net->nptr_;
}

extern "C" ivl_nexus_ptr_t ivl_nexus_ptr(ivl_nexus_t net, unsigned idx)
{
      assert(net);
      assert(idx < net->nptr_);
      return net->ptrs_ + idx;
}

extern "C" ivl_process_type_t ivl_process_type(ivl_process_t net)
{
      return net->type_;
}

extern "C" ivl_statement_t ivl_process_stmt(ivl_process_t net)
{
      return net->stmt_;
}

extern "C" int ivl_scope_children(ivl_scope_t net, ivl_scope_f func)
{
      for (ivl_scope_t cur = net->child_; cur;  cur = cur->sibling_) {
	    int rc = func(cur);
	    if (rc != 0)
		  return rc;
      }

      return 0;
}

extern "C" unsigned ivl_scope_logs(ivl_scope_t net)
{
      assert(net);
      return net->nlog_;
}

extern "C" ivl_net_logic_t ivl_scope_log(ivl_scope_t net, unsigned idx)
{
      assert(net);
      assert(idx < net->nlog_);
      return net->log_[idx];
}

extern "C" const char* ivl_scope_name(ivl_scope_t net)
{
      return net->name_;
}

extern "C" unsigned ivl_scope_sigs(ivl_scope_t net)
{
      assert(net);
      return net->nsigs_;
}

extern "C" ivl_signal_t ivl_scope_sig(ivl_scope_t net, unsigned idx)
{
      assert(net);
      assert(idx < net->nsigs_);
      return net->sigs_[idx];
}

extern "C" const char* ivl_signal_basename(ivl_signal_t net)
{
      const char*nam = net->name_;
      nam += strlen(ivl_scope_name(net->scope_));
      assert(*nam == '.');
      nam += 1;
      return nam;
}

extern "C" const char* ivl_signal_name(ivl_signal_t net)
{
      return net->name_;
}

extern "C" unsigned ivl_signal_pins(ivl_signal_t net)
{
      return net->width_;
}

extern "C" ivl_signal_port_t ivl_signal_port(ivl_signal_t net)
{
      return net->port_;
}

extern "C" ivl_signal_type_t ivl_signal_type(ivl_signal_t net)
{
      return net->type_;
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

extern "C" ivl_expr_t ivl_stmt_cond_expr(ivl_statement_t net)
{
      assert(net && (net->type_ == IVL_ST_CONDIT));
      return net->u_.condit_.cond_;
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

extern "C" unsigned ivl_stmt_lwidth(ivl_statement_t net)
{
      assert(net->type_ == IVL_ST_ASSIGN);
      return net->u_.assign_.lwidth_;
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

extern "C" ivl_expr_t ivl_stmt_rval(ivl_statement_t net)
{
      switch (net->type_) {
	  case IVL_ST_ASSIGN:
	    return net->u_.assign_.rval_;
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
 * Revision 1.12  2000/10/15 04:46:23  steve
 *  Scopes and processes are accessible randomly from
 *  the design, and signals and logic are accessible
 *  from scopes. Remove the target calls that are no
 *  longer needed.
 *
 *  Add the ivl_nexus_ptr_t and the means to get at
 *  them from nexus objects.
 *
 *  Give names to methods that manipulate the ivl_design_t
 *  type more consistent names.
 *
 * Revision 1.11  2000/10/08 04:01:54  steve
 *  Back pointers in the nexus objects into the devices
 *  that point to it.
 *
 *  Collect threads into a list in the design.
 *
 * Revision 1.10  2000/10/06 23:46:50  steve
 *  ivl_target updates, including more complete
 *  handling of ivl_nexus_t objects. Much reduced
 *  dependencies on pointers to netlist objects.
 *
 * Revision 1.9  2000/10/05 05:03:01  steve
 *  xor and constant devices.
 *
 * Revision 1.8  2000/09/30 02:18:15  steve
 *  ivl_expr_t support for binary operators,
 *  Create a proper ivl_scope_t object.
 *
 * Revision 1.7  2000/09/26 00:30:07  steve
 *  Add EX_NUMBER and ST_TRIGGER to dll-api.
 *
 * Revision 1.6  2000/09/24 15:46:00  steve
 *  API access to signal type and port type.
 *
 * Revision 1.5  2000/09/24 02:21:53  steve
 *  Add support for signal expressions.
 *
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

