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
#ident "$Id: t-dll-api.cc,v 1.83 2002/07/05 21:26:17 steve Exp $"
#endif

# include "config.h"
# include  "t-dll.h"

/* THE FOLLOWING ARE FUNCTIONS THAT ARE CALLED FROM THE TARGET. */

extern "C" const char*ivl_design_flag(ivl_design_t des, const char*key)
{
      return des->self->get_flag(key).c_str();
}

extern "C" int ivl_design_process(ivl_design_t des,
				  ivl_process_f func,
				  void*cd)
{
      for (ivl_process_t idx = des->threads_;  idx;  idx = idx->next_) {
	    int rc = (func)(idx, cd);
	    if (rc != 0)
		  return rc;
      }

      return 0;
}

extern "C" ivl_scope_t ivl_design_root(ivl_design_t des)
{
      assert (des->nroots_);
      return des->roots_[0];
}

extern "C" void ivl_design_roots(ivl_design_t des, ivl_scope_t **scopes,
				 unsigned int *nscopes)
{
      assert (nscopes && scopes);
      *scopes = &des->roots_[0];
      *nscopes = des->nroots_;
}

extern "C" int ivl_design_time_precision(ivl_design_t des)
{
      return des->time_precision;
}

extern "C" unsigned ivl_design_consts(ivl_design_t des)
{
      return des->nconsts;
}

extern "C" ivl_net_const_t ivl_design_const(ivl_design_t des, unsigned idx)
{
      assert(idx < des->nconsts);
      return des->consts[idx];
}

extern "C" ivl_expr_type_t ivl_expr_type(ivl_expr_t net)
{
      if (net == 0)
	    return IVL_EX_NONE;
      return net->type_;
}


inline static const char *basename(ivl_scope_t scope, const char *inst)
{
      inst += strlen(ivl_scope_name(scope));
      assert(*inst == '.');
      return inst+1;
}

extern "C" const char*ivl_memory_name(ivl_memory_t net)
{
      return net->name_;
}

extern "C" const char* ivl_memory_basename(ivl_memory_t net)
{
      return basename(net->scope_, net->name_);
}

extern "C" int ivl_memory_root(ivl_memory_t net)
{
      return net->root_;
}

extern "C" unsigned ivl_memory_size(ivl_memory_t net)
{
      return net->size_;
}

extern "C" unsigned ivl_memory_width(ivl_memory_t net)
{
      return net->width_;
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
      return net->signed_;
}

extern "C" const char* ivl_event_name(ivl_event_t net)
{
      return net->name;
}

extern "C" const char* ivl_event_basename(ivl_event_t net)
{
      return basename(net->scope, net->name);
}


extern "C" unsigned ivl_event_nany(ivl_event_t net)
{
      assert(net);
      return net->nany;
}

extern "C" ivl_nexus_t ivl_event_any(ivl_event_t net, unsigned idx)
{
      assert(net);
      assert(idx < net->nany);
      return net->pins[idx];
}

extern "C" unsigned ivl_event_nneg(ivl_event_t net)
{
      assert(net);
      return net->nneg;
}

extern "C" ivl_nexus_t ivl_event_neg(ivl_event_t net, unsigned idx)
{
      assert(net);
      assert(idx < net->nneg);
      return net->pins[net->nany + idx];
}

extern "C" unsigned ivl_event_npos(ivl_event_t net)
{
      assert(net);
      return net->npos;
}

extern "C" ivl_nexus_t ivl_event_pos(ivl_event_t net, unsigned idx)
{
      assert(net);
      assert(idx < net->npos);
      return net->pins[net->nany + net->nneg + idx];
}

extern "C" const char* ivl_expr_bits(ivl_expr_t net)
{
      assert(net && (net->type_ == IVL_EX_NUMBER));
      return net->u_.number_.bits_;
}

extern "C" ivl_scope_t ivl_expr_def(ivl_expr_t net)
{
      assert(net);

      switch (net->type_) {

	  case IVL_EX_UFUNC:
	    return net->u_.ufunc_.def;

	  default:
	    assert(0);
      }

      return 0;
}

extern "C" unsigned ivl_expr_lsi(ivl_expr_t net)
{
      switch (net->type_) {

	  case IVL_EX_SIGNAL:
	    return net->u_.signal_.lsi;

	  default:
	    assert(0);

      }
      return 0;
}

extern "C" const char* ivl_expr_name(ivl_expr_t net)
{
      switch (net->type_) {

	  case IVL_EX_SFUNC:
	    return net->u_.sfunc_.name_;

	  case IVL_EX_SIGNAL:
	    return net->u_.signal_.sig->name_;

	  case IVL_EX_MEMORY:
	    return net->u_.memory_.mem_->name_;

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

	  case IVL_EX_UNARY:
	    return net->u_.unary_.op_;

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
	  case IVL_EX_SELECT:
	    return net->u_.binary_.lef_;

	  case IVL_EX_BITSEL:
	    return net->u_.bitsel_.bit;

	  case IVL_EX_UNARY:
	    return net->u_.unary_.sub_;

	  case IVL_EX_MEMORY:
	    return net->u_.memory_.idx_;

	  case IVL_EX_TERNARY:
	    return net->u_.ternary_.cond;

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
	  case IVL_EX_SELECT:
	    return net->u_.binary_.rig_;

	  case IVL_EX_TERNARY:
	    return net->u_.ternary_.true_e;

	  default:
	    assert(0);
      }

      return 0;
}

extern "C" ivl_expr_t ivl_expr_oper3(ivl_expr_t net)
{
      assert(net);
      switch (net->type_) {

	  case IVL_EX_TERNARY:
	    return net->u_.ternary_.false_e;

	  default:
	    assert(0);
      }
      return 0;
}

extern "C" ivl_expr_t ivl_expr_parm(ivl_expr_t net, unsigned idx)
{
      assert(net);
      switch (net->type_) {

	  case IVL_EX_CONCAT:
	    assert(idx < net->u_.concat_.parms);
	    return net->u_.concat_.parm[idx];

	  case IVL_EX_SFUNC:
	    assert(idx < net->u_.sfunc_.parms);
	    return net->u_.sfunc_.parm[idx];

	  case IVL_EX_UFUNC:
	    assert(idx < net->u_.ufunc_.parms);
	    return net->u_.ufunc_.parm[idx];

	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" unsigned ivl_expr_parms(ivl_expr_t net)
{
      assert(net);
      switch (net->type_) {

	  case IVL_EX_CONCAT:
	    return net->u_.concat_.parms;

	  case IVL_EX_SFUNC:
	    return net->u_.sfunc_.parms;

	  case IVL_EX_UFUNC:
	    return net->u_.ufunc_.parms;

	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" unsigned ivl_expr_repeat(ivl_expr_t net)
{
      assert(net);
      assert(net->type_ == IVL_EX_CONCAT);
      return net->u_.concat_.rept;
}

extern "C" ivl_scope_t ivl_expr_scope(ivl_expr_t net)
{
      assert(net);
      assert(net->type_ == IVL_EX_SCOPE);
      return net->u_.scope_.scope;
}

extern "C" ivl_signal_t ivl_expr_signal(ivl_expr_t net)
{
      assert(net);
      switch (net->type_) {
	  case IVL_EX_BITSEL:
	    return net->u_.bitsel_.sig;

	  case IVL_EX_SIGNAL:
	    return net->u_.signal_.sig;

	  default:
	    assert(0);
	    return 0;
      }
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

extern "C" unsigned long ivl_expr_uvalue(ivl_expr_t net)
{
      assert(net->type_ == IVL_EX_ULONG);
      return net->u_.ulong_.value;
}

extern "C" unsigned ivl_expr_width(ivl_expr_t net)
{
      assert(net);
      return net->width_;
}

extern "C" ivl_memory_t ivl_expr_memory(ivl_expr_t net)
{
      assert(net->type_ == IVL_EX_MEMORY);
      return net->u_.memory_.mem_;
}

extern "C" const char* ivl_logic_attr(ivl_net_logic_t net, const char*key)
{
      assert(net);
      unsigned idx;

      for (idx = 0 ;  idx < net->nattr ;  idx += 1) {

	    if (strcmp(net->attr[idx].key, key) == 0)
		  return net->attr[idx].type == IVL_ATT_STR
			? net->attr[idx].val.str
			: 0;
      }

      return 0;
}

extern "C" unsigned ivl_logic_attr_cnt(ivl_net_logic_t net)
{
      return net->nattr;
}

extern "C" ivl_attribute_t ivl_logic_attr_val(ivl_net_logic_t net,
					      unsigned idx)
{
      assert(idx < net->nattr);
      return net->attr + idx;
}

extern "C" const char* ivl_logic_name(ivl_net_logic_t net)
{
      assert(net);
      return net->name_;
}

extern "C" const char* ivl_logic_basename(ivl_net_logic_t net)
{
      return basename(net->scope_, net->name_);
}

extern "C" ivl_scope_t ivl_logic_scope(ivl_net_logic_t net)
{
      assert(net);
      return net->scope_;
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

extern "C" ivl_udp_t ivl_logic_udp(ivl_net_logic_t net)
{
      assert(net->type_ == IVL_LO_UDP);
      assert(net->udp);
      return net->udp;
}

extern "C" unsigned  ivl_logic_delay(ivl_net_logic_t net, unsigned transition)
{
      assert(transition < 3);
      return net->delay[transition];
}


extern "C" unsigned    ivl_udp_sequ(ivl_udp_t net)
{
      return net->sequ;
}

extern "C" unsigned    ivl_udp_nin(ivl_udp_t net)
{
      return net->nin;
}

extern "C" unsigned    ivl_udp_init(ivl_udp_t net)
{
      return net->init;
}

extern "C" const char* ivl_udp_row(ivl_udp_t net, unsigned idx)
{
      assert(idx < net->nrows);
      assert(net->table);
      assert(net->table[idx]);
      return net->table[idx];
}

extern "C" unsigned    ivl_udp_rows(ivl_udp_t net)
{
      return net->nrows;
}

extern "C" const char* ivl_udp_name(ivl_udp_t net)
{
      assert(net->name);
      return net->name;
}

extern "C" const char* ivl_lpm_basename(ivl_lpm_t net)
{
      return basename(net->scope, net->name);
}


extern "C" ivl_nexus_t ivl_lpm_clk(ivl_lpm_t net)
{
      assert(net);
      switch (net->type) {
	  case IVL_LPM_FF:
	  case IVL_LPM_RAM:
	    return net->u_.ff.clk;
	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" ivl_scope_t ivl_lpm_define(ivl_lpm_t net)
{
      assert(net);
      switch (net->type) {
	  case IVL_LPM_UFUNC:
	    return net->u_.ufunc.def;
	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" ivl_nexus_t ivl_lpm_enable(ivl_lpm_t net)
{
      assert(net);
      switch (net->type) {
	  case IVL_LPM_RAM:
	  case IVL_LPM_FF:
	    return net->u_.ff.we;
	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" ivl_nexus_t ivl_lpm_data(ivl_lpm_t net, unsigned idx)
{
      assert(net);
      switch (net->type) {
	  case IVL_LPM_ADD:
	  case IVL_LPM_CMP_EQ:
	  case IVL_LPM_CMP_GE:
	  case IVL_LPM_CMP_GT:
	  case IVL_LPM_CMP_NE:
	  case IVL_LPM_DIVIDE:
	  case IVL_LPM_MOD:
	  case IVL_LPM_MULT:
	  case IVL_LPM_SUB:
	    assert(idx < net->u_.arith.width);
	    return net->u_.arith.a[idx];

	  case IVL_LPM_SHIFTL:
	  case IVL_LPM_SHIFTR:
	    assert(idx < net->u_.shift.width);
	    return net->u_.shift.d[idx];

	  case IVL_LPM_FF:
	  case IVL_LPM_RAM:
	    assert(idx < net->u_.ff.width);
	    if (net->u_.ff.width == 1)
		  return net->u_.ff.d.pin;
	    else
		  return net->u_.ff.d.pins[idx];

	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" ivl_nexus_t ivl_lpm_datab(ivl_lpm_t net, unsigned idx)
{
      assert(net);
      switch (net->type) {

	  case IVL_LPM_ADD:
	  case IVL_LPM_CMP_EQ:
	  case IVL_LPM_CMP_GE:
	  case IVL_LPM_CMP_GT:
	  case IVL_LPM_CMP_NE:
	  case IVL_LPM_DIVIDE:
	  case IVL_LPM_MOD:
	  case IVL_LPM_MULT:
	  case IVL_LPM_SUB:
	    assert(idx < net->u_.arith.width);
	    return net->u_.arith.b[idx];

	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" ivl_nexus_t ivl_lpm_data2(ivl_lpm_t net, unsigned sdx, unsigned idx)
{
      assert(net);
      switch (net->type) {
	  case IVL_LPM_MUX:
	    assert(sdx < net->u_.mux.size);
	    assert(idx < net->u_.mux.width);
	    return net->u_.mux.d[sdx*net->u_.mux.width + idx];

	  case IVL_LPM_UFUNC: {
		sdx += 1; /* skip the output port. */
		assert(sdx < net->u_.ufunc.ports);
		assert(idx < net->u_.ufunc.port_wid[sdx]);
		unsigned base = 0;
		for (unsigned i = 0 ;  i < sdx ;  i += 1)
		      base += net->u_.ufunc.port_wid[i];
		return net->u_.ufunc.pins[base+idx];
	  }

	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" unsigned ivl_lpm_data2_width(ivl_lpm_t net, unsigned sdx)
{
      assert(net);
      switch (net->type) {
	  case IVL_LPM_UFUNC:
	    sdx += 1; /* skip the output port. */
	    assert(sdx < net->u_.ufunc.ports);
	    return net->u_.ufunc.port_wid[sdx];
	  default:
	    assert(0);
	    return 0;
      }
}
extern "C" const char* ivl_lpm_name(ivl_lpm_t net)
{
      return net->name;
}

extern "C" ivl_nexus_t ivl_lpm_q(ivl_lpm_t net, unsigned idx)
{
      assert(net);

      switch (net->type) {
	  case IVL_LPM_ADD:
	  case IVL_LPM_DIVIDE:
	  case IVL_LPM_MOD:
	  case IVL_LPM_MULT:
	  case IVL_LPM_SUB:
	    assert(idx < net->u_.arith.width);
	    return net->u_.arith.q[idx];

	  case IVL_LPM_CMP_GE:
	  case IVL_LPM_CMP_GT:
	  case IVL_LPM_CMP_EQ:
	  case IVL_LPM_CMP_NE:
	    assert(idx == 0);
	    return net->u_.arith.q[0];

	  case IVL_LPM_FF:
	  case IVL_LPM_RAM:
	    assert(idx < net->u_.ff.width);
	    if (net->u_.ff.width == 1)
		  return net->u_.ff.q.pin;
	    else
		  return net->u_.ff.q.pins[idx];

	  case IVL_LPM_MUX:
	    assert(idx < net->u_.mux.width);
	    if (net->u_.mux.width == 1)
		  return net->u_.mux.q.pin;
	    else
		  return net->u_.mux.q.pins[idx];

	  case IVL_LPM_SHIFTL:
	  case IVL_LPM_SHIFTR:
	    assert(idx < net->u_.shift.width);
	    return net->u_.shift.q[idx];

	  case IVL_LPM_UFUNC:
	    assert(idx < net->u_.ufunc.port_wid[0]);
	    return net->u_.ufunc.pins[idx];

	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" ivl_scope_t ivl_lpm_scope(ivl_lpm_t net)
{
      assert(net);
      return net->scope;
}

extern "C" ivl_nexus_t ivl_lpm_select(ivl_lpm_t net, unsigned idx)
{
      switch (net->type) {
	  case IVL_LPM_RAM:
	    assert(idx < net->u_.ff.swid);
	    if (net->u_.ff.swid == 1)
		  return net->u_.ff.s.pin;
	    else
		  return net->u_.ff.s.pins[idx];

	  case IVL_LPM_MUX:
	    assert(idx < net->u_.mux.swid);
	    if (net->u_.mux.swid == 1)
		  return net->u_.mux.s.pin;
	    else
		  return net->u_.mux.s.pins[idx];

	  case IVL_LPM_SHIFTL:
	  case IVL_LPM_SHIFTR:
	    assert(idx < net->u_.shift.select);
	    return net->u_.shift.s[idx];

	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" unsigned ivl_lpm_selects(ivl_lpm_t net)
{
      switch (net->type) {
	  case IVL_LPM_RAM:
	    return net->u_.ff.swid;
	  case IVL_LPM_MUX:
	    return net->u_.mux.swid;
	  case IVL_LPM_SHIFTL:
	  case IVL_LPM_SHIFTR:
	    return net->u_.shift.select;
	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" unsigned ivl_lpm_size(ivl_lpm_t net)
{
      switch (net->type) {
	  case IVL_LPM_MUX:
	    return net->u_.mux.size;
	  case IVL_LPM_UFUNC:
	    return net->u_.ufunc.ports - 1;
	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" ivl_lpm_type_t ivl_lpm_type(ivl_lpm_t net)
{
      return net->type;
}

extern "C" unsigned ivl_lpm_width(ivl_lpm_t net)
{
      assert(net);
      switch (net->type) {
	  case IVL_LPM_FF:
	  case IVL_LPM_RAM:
	    return net->u_.ff.width;
	  case IVL_LPM_MUX:
	    return net->u_.mux.width;
	  case IVL_LPM_ADD:
	  case IVL_LPM_CMP_EQ:
	  case IVL_LPM_CMP_GE:
	  case IVL_LPM_CMP_GT:
	  case IVL_LPM_CMP_NE:
	  case IVL_LPM_DIVIDE:
	  case IVL_LPM_MOD:
	  case IVL_LPM_MULT:
	  case IVL_LPM_SUB:
	    return net->u_.arith.width;
	  case IVL_LPM_SHIFTL:
	  case IVL_LPM_SHIFTR:
	    return net->u_.shift.width;
	  case IVL_LPM_UFUNC:
	    return net->u_.ufunc.port_wid[0];
	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" ivl_memory_t ivl_lpm_memory(ivl_lpm_t net)
{
      assert(net);
      switch (net->type) {
	  case IVL_LPM_RAM:
	    return net->u_.ff.mem;
	  default:
	    assert(0);
	    return 0;
      }      
}

extern "C" ivl_expr_t ivl_lval_mux(ivl_lval_t net)
{
      assert(net);
      if (net->type_ == IVL_LVAL_MUX)
	    return net->idx;
      return 0x0;
}

extern "C" ivl_expr_t ivl_lval_idx(ivl_lval_t net)
{
      assert(net);
      if (net->type_ == IVL_LVAL_MEM)
	    return net->idx;
      return 0x0;
}

extern "C" ivl_memory_t ivl_lval_mem(ivl_lval_t net)
{
      assert(net);
      if (net->type_ == IVL_LVAL_MEM)
	    return net->n.mem;
      return 0x0;
}

extern "C" unsigned ivl_lval_part_off(ivl_lval_t net)
{
      assert(net);
      return net->loff_;
}

extern "C" unsigned ivl_lval_pins(ivl_lval_t net)
{
      assert(net);
      return net->width_;
}

extern "C" ivl_nexus_t ivl_lval_pin(ivl_lval_t net, unsigned idx)
{
      assert(net);
      assert(idx < net->width_);
      assert(net->type_ != IVL_LVAL_MEM);
      return ivl_signal_pin(net->n.sig, idx+net->loff_);
}

extern "C" ivl_signal_t ivl_lval_sig(ivl_lval_t net)
{
      assert(net);
      return net->n.sig;
}

extern "C" const char* ivl_nexus_name(ivl_nexus_t net)
{
      assert(net);
      return net->name_;
}

extern "C" void* ivl_nexus_get_private(ivl_nexus_t net)
{
      assert(net);
      return net->private_data;
}

extern "C" void ivl_nexus_set_private(ivl_nexus_t net, void*data)
{
      assert(net);
      net->private_data = data;
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

extern "C" ivl_drive_t ivl_nexus_ptr_drive0(ivl_nexus_ptr_t net)
{
      assert(net);
      return (ivl_drive_t)(net->drive0);
}

extern "C" ivl_drive_t ivl_nexus_ptr_drive1(ivl_nexus_ptr_t net)
{
      assert(net);
      return (ivl_drive_t)(net->drive1);
}

extern "C" unsigned ivl_nexus_ptr_pin(ivl_nexus_ptr_t net)
{
      assert(net);
      return net->pin_;
}

extern "C" ivl_net_const_t ivl_nexus_ptr_con(ivl_nexus_ptr_t net)
{
      if (net == 0)
	    return 0;
      if (net->type_ != __NEXUS_PTR_CON)
	    return 0;
      return net->l.con;
}

extern "C" ivl_net_logic_t ivl_nexus_ptr_log(ivl_nexus_ptr_t net)
{
      if (net == 0)
	    return 0;
      if (net->type_ != __NEXUS_PTR_LOG)
	    return 0;
      return net->l.log;
}

extern "C" ivl_lpm_t ivl_nexus_ptr_lpm(ivl_nexus_ptr_t net)
{
      if (net == 0)
	    return 0;
      if (net->type_ != __NEXUS_PTR_LPM)
	    return 0;
      return net->l.lpm;
}

extern "C" ivl_signal_t ivl_nexus_ptr_sig(ivl_nexus_ptr_t net)
{
      if (net == 0)
	    return 0;
      if (net->type_ != __NEXUS_PTR_SIG)
	    return 0;
      return net->l.sig;
}

extern "C" ivl_process_type_t ivl_process_type(ivl_process_t net)
{
      return net->type_;
}

extern "C" ivl_scope_t ivl_process_scope(ivl_process_t net)
{
      return net->scope_;
}

extern "C" ivl_statement_t ivl_process_stmt(ivl_process_t net)
{
      return net->stmt_;
}

extern "C" unsigned ivl_process_attr_cnt(ivl_process_t net)
{
      return net->nattr;
}

extern "C" ivl_attribute_t ivl_process_attr_val(ivl_process_t net,
						unsigned idx)
{
      assert(idx < net->nattr);
      return net->attr + idx;
}

extern "C" const char* ivl_scope_basename(ivl_scope_t net)
{
      assert(net);

      if (net->parent == 0)
	    return net->name_;

      return basename(net->parent, net->name_);
}

extern "C" int ivl_scope_children(ivl_scope_t net,
				  ivl_scope_f func,
				  void*cd)
{
      for (ivl_scope_t cur = net->child_; cur;  cur = cur->sibling_) {
	    int rc = func(cur, cd);
	    if (rc != 0)
		  return rc;
      }

      return 0;
}

extern "C" ivl_statement_t ivl_scope_def(ivl_scope_t net)
{
      assert(net);
      return net->def;
}

extern "C" unsigned ivl_scope_events(ivl_scope_t net)
{
      assert(net);
      return net->nevent_;
}

extern "C" ivl_event_t ivl_scope_event(ivl_scope_t net, unsigned idx)
{
      assert(net);
      assert(idx < net->nevent_);
      return net->event_[idx];
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

extern "C" unsigned ivl_scope_lpms(ivl_scope_t net)
{
      assert(net);
      return net->nlpm_;
}

extern "C" ivl_lpm_t ivl_scope_lpm(ivl_scope_t net, unsigned idx)
{
      assert(net);
      assert(idx < net->nlpm_);
      return net->lpm_[idx];
}

extern "C" unsigned ivl_scope_mems(ivl_scope_t net)
{
      assert(net);
      return net->nmem_;
}

extern "C" ivl_memory_t ivl_scope_mem(ivl_scope_t net, unsigned idx)
{
      assert(net);
      assert(idx < net->nmem_);
      return net->mem_[idx];
}

extern "C" const char* ivl_scope_name(ivl_scope_t net)
{
      return net->name_;
}

extern "C" ivl_scope_t ivl_scope_parent(ivl_scope_t net)
{
      assert(net);
      return net->parent;
}

extern "C" unsigned ivl_scope_ports(ivl_scope_t net)
{
      assert(net);
      assert(net->type_ == IVL_SCT_FUNCTION);
      return net->ports;
}

extern "C" ivl_signal_t ivl_scope_port(ivl_scope_t net, unsigned idx)
{
      assert(net);
      assert(net->type_ == IVL_SCT_FUNCTION);
      assert(idx < net->ports);
      return net->port[idx];
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

extern "C" ivl_scope_type_t ivl_scope_type(ivl_scope_t net)
{
      assert(net);
      return net->type_;
}

extern "C" const char* ivl_scope_tname(ivl_scope_t net)
{
      assert(net);
      return net->tname_;
}

extern "C" const char* ivl_signal_attr(ivl_signal_t net, const char*key)
{
      if (net->nattr == 0)
	    return 0;

      for (unsigned idx = 0 ;  idx < net->nattr ;  idx += 1)

	    if (strcmp(key, net->attr[idx].key) == 0)
		  return net->attr[idx].type == IVL_ATT_STR
			? net->attr[idx].val.str
			: 0;

      return 0;
}

extern "C" unsigned ivl_signal_attr_cnt(ivl_signal_t net)
{
      return net->nattr;
}

extern "C" ivl_attribute_t ivl_signal_attr_val(ivl_signal_t net, unsigned idx)
{
      assert(idx < net->nattr);
      return net->attr + idx;
}

extern "C" const char* ivl_signal_basename(ivl_signal_t net)
{
      return basename(net->scope_, net->name_);
}

extern "C" const char* ivl_signal_name(ivl_signal_t net)
{
      return net->name_;
}

extern "C" ivl_nexus_t ivl_signal_pin(ivl_signal_t net, unsigned idx)
{
      assert(net);
      assert(idx < net->width_);
      if (net->width_ == 1) {
	    return net->n.pin_;

      } else {
	    return net->n.pins_[idx];
      }
}

extern "C" unsigned ivl_signal_pins(ivl_signal_t net)
{
      return net->width_;
}

extern "C" ivl_signal_port_t ivl_signal_port(ivl_signal_t net)
{
      return net->port_;
}

extern "C" int ivl_signal_local(ivl_signal_t net)
{
      return net->local_;
}

extern "C" int ivl_signal_signed(ivl_signal_t net)
{
      return net->signed_;
}

extern "C" int ivl_signal_integer(ivl_signal_t net)
{
      return net->isint_;
}

extern "C" ivl_signal_type_t ivl_signal_type(ivl_signal_t net)
{
      return net->type_;
}

extern "C" ivl_statement_type_t ivl_statement_type(ivl_statement_t net)
{
      return net->type_;
}

extern "C" ivl_scope_t ivl_stmt_block_scope(ivl_statement_t net)
{
      switch (net->type_) {
	  case IVL_ST_BLOCK:
	  case IVL_ST_FORK:
	    return net->u_.block_.scope;
	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" unsigned ivl_stmt_block_count(ivl_statement_t net)
{
      switch (net->type_) {
	  case IVL_ST_BLOCK:
	  case IVL_ST_FORK:
	    return net->u_.block_.nstmt_;
	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" ivl_statement_t ivl_stmt_block_stmt(ivl_statement_t net,
					       unsigned i)
{
      switch (net->type_) {
	  case IVL_ST_BLOCK:
	  case IVL_ST_FORK:
	    return net->u_.block_.stmt_ + i;
	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" ivl_scope_t ivl_stmt_call(ivl_statement_t net)
{
      switch (net->type_) {
	  case IVL_ST_DISABLE:
	    return net->u_.disable_.scope;

	  case IVL_ST_UTASK:
	    return net->u_.utask_.def;
	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" unsigned ivl_stmt_case_count(ivl_statement_t net)
{
      switch (net->type_) {
	  case IVL_ST_CASE:
	  case IVL_ST_CASEX:
	  case IVL_ST_CASEZ:
	    return net->u_.case_.ncase;
	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" ivl_expr_t ivl_stmt_case_expr(ivl_statement_t net, unsigned idx)
{
      switch (net->type_) {
	  case IVL_ST_CASE:
	  case IVL_ST_CASEX:
	  case IVL_ST_CASEZ:
	    assert(idx < net->u_.case_.ncase);
	    return net->u_.case_.case_ex[idx];

	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" ivl_statement_t ivl_stmt_case_stmt(ivl_statement_t net, unsigned idx)
{
      switch (net->type_) {
	  case IVL_ST_CASE:
	  case IVL_ST_CASEX:
	  case IVL_ST_CASEZ:
	    assert(idx < net->u_.case_.ncase);
	    return net->u_.case_.case_st + idx;

	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" ivl_expr_t ivl_stmt_cond_expr(ivl_statement_t net)
{
      switch (net->type_) {
	  case IVL_ST_CONDIT:
	    return net->u_.condit_.cond_;

	  case IVL_ST_CASE:
	  case IVL_ST_CASEX:
	  case IVL_ST_CASEZ:
	    return net->u_.case_.cond;

	  case IVL_ST_REPEAT:
	  case IVL_ST_WHILE:
	    return net->u_.while_.cond_;

	  default:
	    assert(0);
	    return 0;
      }
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

extern "C" ivl_expr_t ivl_stmt_delay_expr(ivl_statement_t net)
{
      switch (net->type_) {
	  case IVL_ST_ASSIGN:
	  case IVL_ST_ASSIGN_NB:
	    return net->u_.assign_.delay;

	  case IVL_ST_DELAYX:
	    return net->u_.delayx_.expr;

	  default:
	    assert(0);
	    return 0;
      }
}

extern "C" unsigned long ivl_stmt_delay_val(ivl_statement_t net)
{
      assert(net->type_ == IVL_ST_DELAY);
      return net->u_.delay_.delay_;
}

extern "C" ivl_event_t ivl_stmt_event(ivl_statement_t net)
{
      switch (net->type_) {
	  case IVL_ST_WAIT:
	    return net->u_.wait_.event_;
	  case IVL_ST_TRIGGER:
	    return net->u_.trig_.event_;
	  default:
	    assert(0);
      }
      return 0;
}

extern "C" ivl_lval_t ivl_stmt_lval(ivl_statement_t net, unsigned idx)
{
      switch (net->type_) {
	  case IVL_ST_ASSIGN:
	  case IVL_ST_ASSIGN_NB:
	    assert(idx < net->u_.assign_.lvals_);
	    return net->u_.assign_.lval_ + idx;

	  case IVL_ST_CASSIGN:
	  case IVL_ST_DEASSIGN:
	  case IVL_ST_FORCE:
	  case IVL_ST_RELEASE:
	    assert(idx < net->u_.cassign_.lvals);
	    return net->u_.cassign_.lval + idx;

	  default:
	    assert(0);
      }
      return 0;
}

extern "C" unsigned ivl_stmt_lvals(ivl_statement_t net)
{
      switch (net->type_) {
	  case IVL_ST_ASSIGN:
	  case IVL_ST_ASSIGN_NB:
	    return net->u_.assign_.lvals_;

	  case IVL_ST_CASSIGN:
	  case IVL_ST_DEASSIGN:
	  case IVL_ST_FORCE:
	  case IVL_ST_RELEASE:
	    return net->u_.cassign_.lvals;

	  default:
	    assert(0);
      }
      return 0;
}

extern "C" unsigned ivl_stmt_lwidth(ivl_statement_t net)
{
      assert((net->type_ == IVL_ST_ASSIGN)
	     || (net->type_ == IVL_ST_ASSIGN_NB));

      unsigned sum = 0;
      for (unsigned idx = 0 ;  idx < net->u_.assign_.lvals_ ;  idx += 1) {
	    ivl_lval_t cur = net->u_.assign_.lval_ + idx;
	    switch(cur->type_) {
		case IVL_LVAL_MUX:
		  sum += 1;
		  break;
		case IVL_LVAL_REG:
		  sum += ivl_lval_pins(cur);
		  break;
		case IVL_LVAL_MEM:
		  sum += ivl_memory_width(ivl_lval_mem(cur));
		  break;
		default:
		  assert(0);
	    }
      }

      return sum;
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

extern "C" ivl_nexus_t ivl_stmt_nexus(ivl_statement_t net, unsigned idx)
{
      switch (net->type_) {
	  case IVL_ST_CASSIGN:
	  case IVL_ST_FORCE:
	    assert(idx < net->u_.cassign_.npins);
	    return net->u_.cassign_.pins[idx];
	  default:
	    assert(0);
      }

      return 0;
}

extern "C" unsigned ivl_stmt_nexus_count(ivl_statement_t net)
{
      switch (net->type_) {
	  case IVL_ST_CASSIGN:
	  case IVL_ST_FORCE:
	    return net->u_.cassign_.npins;
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
	  case IVL_ST_ASSIGN_NB:
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
	  case IVL_ST_DELAYX:
	    return net->u_.delayx_.stmt_;
	  case IVL_ST_FOREVER:
	    return net->u_.forever_.stmt_;
	  case IVL_ST_WAIT:
	    return net->u_.wait_.stmt_;
	  case IVL_ST_REPEAT:
	  case IVL_ST_WHILE:
	    return net->u_.while_.stmt_;
	  default:
	    assert(0);
      }

      return 0;
}

/*
 * $Log: t-dll-api.cc,v $
 * Revision 1.83  2002/07/05 21:26:17  steve
 *  Avoid emitting to vvp local net symbols.
 *
 * Revision 1.82  2002/06/21 04:59:35  steve
 *  Carry integerness throughout the compilation.
 *
 * Revision 1.81  2002/05/27 00:08:45  steve
 *  Support carrying the scope of named begin-end
 *  blocks down to the code generator, and have
 *  the vvp code generator use that to support disable.
 *
 * Revision 1.80  2002/05/26 01:39:03  steve
 *  Carry Verilog 2001 attributes with processes,
 *  all the way through to the ivl_target API.
 *
 *  Divide signal reference counts between rval
 *  and lval references.
 *
 * Revision 1.79  2002/05/24 04:36:23  steve
 *  Verilog 2001 attriubtes on nets/wires.
 *
 * Revision 1.78  2002/05/23 03:08:51  steve
 *  Add language support for Verilog-2001 attribute
 *  syntax. Hook this support into existing $attribute
 *  handling, and add number and void value types.
 *
 *  Add to the ivl_target API new functions for access
 *  of complex attributes attached to gates.
 *
 * Revision 1.77  2002/03/17 19:30:47  steve
 *  Add API to support user defined function.
 *
 * Revision 1.76  2002/03/09 02:10:22  steve
 *  Add the NetUserFunc netlist node.
 *
 * Revision 1.75  2002/01/28 00:52:41  steve
 *  Add support for bit select of parameters.
 *  This leads to a NetESelect node and the
 *  vvp code generator to support that.
 *
 * Revision 1.74  2002/01/03 04:19:01  steve
 *  Add structural modulus support down to vvp.
 *
 * Revision 1.73  2001/12/06 03:11:00  steve
 *  Add ivl_logic_delay function to ivl_target.
 *
 * Revision 1.72  2001/11/14 03:28:49  steve
 *  DLL target support for force and release.
 *
 * Revision 1.71  2001/11/01 04:25:31  steve
 *  ivl_target support for cassign.
 *
 * Revision 1.70  2001/10/31 05:24:52  steve
 *  ivl_target support for assign/deassign.
 *
 * Revision 1.69  2001/10/19 21:53:24  steve
 *  Support multiple root modules (Philip Blundell)
 *
 * Revision 1.68  2001/10/16 02:19:27  steve
 *  Support IVL_LPM_DIVIDE for structural divide.
 *
 * Revision 1.67  2001/09/16 22:19:42  steve
 *  Support attributes to logic gates.
 *
 * Revision 1.66  2001/09/01 01:57:31  steve
 *  Make constants available through the design root
 *
 * Revision 1.65  2001/08/31 22:58:40  steve
 *  Support DFF CE inputs.
 *
 * Revision 1.64  2001/08/28 04:07:18  steve
 *  Add some ivl_target convenience functions.
 *
 * Revision 1.63  2001/08/25 23:50:03  steve
 *  Change the NetAssign_ class to refer to the signal
 *  instead of link into the netlist. This is faster
 *  and uses less space. Make the NetAssignNB carry
 *  the delays instead of the NetAssign_ lval objects.
 *
 *  Change the vvp code generator to support multiple
 *  l-values, i.e. concatenations of part selects.
 *
 * Revision 1.62  2001/08/10 00:40:45  steve
 *  tgt-vvp generates code that skips nets as inputs.
 *
 * Revision 1.61  2001/07/28 01:17:40  steve
 *  Support getting the signal from IVL_EX_SIGNAL expressions.
 *
 * Revision 1.60  2001/07/27 04:51:44  steve
 *  Handle part select expressions as variants of
 *  NetESignal/IVL_EX_SIGNAL objects, instead of
 *  creating new and useless temporary signals.
 *
 * Revision 1.59  2001/07/27 02:41:55  steve
 *  Fix binding of dangling function ports. do not elide them.
 *
 * Revision 1.58  2001/07/25 03:10:49  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.57  2001/07/22 00:17:49  steve
 *  Support the NetESubSignal expressions in vvp.tgt.
 *
 * Revision 1.56  2001/07/19 04:55:06  steve
 *  Support calculated delays in vvp.tgt.
 *
 * Revision 1.55  2001/07/07 20:20:10  steve
 *  Pass parameters to system functions.
 *
 * Revision 1.54  2001/07/07 03:01:37  steve
 *  Detect and make available to t-dll the right shift.
 *
 * Revision 1.53  2001/07/04 22:59:25  steve
 *  handle left shifter in dll output.
 *
 * Revision 1.52  2001/06/30 23:03:16  steve
 *  support fast programming by only writing the bits
 *  that are listed in the input file.
 *
 * Revision 1.51  2001/06/16 23:45:05  steve
 *  Add support for structural multiply in t-dll.
 *  Add code generators and vvp support for both
 *  structural and behavioral multiply.
 *
 * Revision 1.50  2001/06/16 02:41:41  steve
 *  Generate code to support memory access in continuous
 *  assignment statements. (Stephan Boettcher)
 *
 * Revision 1.49  2001/06/15 04:14:19  steve
 *  Generate vvp code for GT and GE comparisons.
 *
 * Revision 1.48  2001/06/07 03:09:37  steve
 *  support subtraction in tgt-vvp.
 *
 * Revision 1.47  2001/06/07 02:12:43  steve
 *  Support structural addition.
 *
 * Revision 1.46  2001/05/20 01:06:16  steve
 *  stub ivl_expr_parms for sfunctions.
 *
 * Revision 1.45  2001/05/17 04:37:02  steve
 *  Behavioral ternary operators for vvp.
 *
 * Revision 1.44  2001/05/08 23:59:33  steve
 *  Add ivl and vvp.tgt support for memories in
 *  expressions and l-values. (Stephan Boettcher)
 *
 * Revision 1.43  2001/05/06 17:48:20  steve
 *  Support memory objects. (Stephan Boettcher)
 *
 * Revision 1.42  2001/04/29 23:17:38  steve
 *  Carry drive strengths in the ivl_nexus_ptr_t, and
 *  handle constant devices in targets.'
 *
 * Revision 1.41  2001/04/26 05:12:02  steve
 *  Implement simple MUXZ for ?: operators.
 *
 * Revision 1.40  2001/04/22 23:09:46  steve
 *  More UDP consolidation from Stephan Boettcher.
 *
 * Revision 1.39  2001/04/21 00:55:46  steve
 *  Generate code for disable.
 *
 * Revision 1.38  2001/04/15 02:58:11  steve
 *  vvp support for <= with internal delay.
 *
 * Revision 1.37  2001/04/06 02:28:02  steve
 *  Generate vvp code for functions with ports.
 *
 * Revision 1.36  2001/04/05 03:20:57  steve
 *  Generate vvp code for the repeat statement.
 *
 * Revision 1.35  2001/04/05 01:12:28  steve
 *  Get signed compares working correctly in vvp.
 *
 * Revision 1.34  2001/04/04 04:50:35  steve
 *  Support forever loops in the tgt-vvp target.
 *
 * Revision 1.33  2001/04/03 04:50:37  steve
 *  Support non-blocking assignments.
 *
 * Revision 1.32  2001/04/02 02:28:12  steve
 *  Generate code for task calls.
 *
 * Revision 1.31  2001/04/02 00:28:35  steve
 *  Support the scope expression node.
 *
 * Revision 1.30  2001/04/01 06:52:27  steve
 *  support the NetWhile statement.
 *
 * Revision 1.29  2001/04/01 01:48:21  steve
 *  Redesign event information to support arbitrary edge combining.
 *
 * Revision 1.28  2001/03/31 17:36:38  steve
 *  Generate vvp code for case statements.
 *
 * Revision 1.27  2001/03/30 05:49:52  steve
 *  Generate code for fork/join statements.
 *
 * Revision 1.26  2001/03/29 03:47:38  steve
 *  Behavioral trigger statements.
 *
 * Revision 1.25  2001/03/29 02:52:39  steve
 *  Add unary ~ operator to tgt-vvp.
 *
 * Revision 1.24  2001/03/28 06:07:39  steve
 *  Add the ivl_event_t to ivl_target, and use that to generate
 *  .event statements in vvp way ahead of the thread that uses it.
 *
 * Revision 1.23  2001/03/27 06:27:40  steve
 *  Generate code for simple @ statements.
 *
 * Revision 1.22  2001/03/20 01:44:13  steve
 *  Put processes in the proper scope.
 *
 * Revision 1.21  2001/01/15 00:47:02  steve
 *  Pass scope type information to the target module.
 *
 * Revision 1.20  2001/01/15 00:05:39  steve
 *  Add client data pointer for scope and process scanners.
 *
 * Revision 1.19  2000/12/05 06:29:33  steve
 *  Make signal attributes available to ivl_target API.
 *
 * Revision 1.18  2000/11/12 17:47:29  steve
 *  flip-flop pins for ivl_target API.
 *
 * Revision 1.17  2000/11/11 00:03:36  steve
 *  Add support for the t-dll backend grabing flip-flops.
 *
 * Revision 1.16  2000/10/28 22:32:34  steve
 *  API for concatenation expressions.
 *
 * Revision 1.15  2000/10/25 05:41:24  steve
 *  Get target signal from nexus_ptr.
 *
 * Revision 1.14  2000/10/18 20:04:39  steve
 *  Add ivl_lval_t and support for assignment l-values.
 *
 * Revision 1.13  2000/10/16 22:44:54  steve
 *  Stubs so that cygwin port will link ivl.
 *
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

