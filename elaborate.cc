/*
 * Copyright (c) 1998-2021 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2013 / Stephen Williams (steve@icarus.com)
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

# include "config.h"

/*
 * Elaboration takes as input a complete parse tree and the name of a
 * root module, and generates as output the elaborated design. This
 * elaborated design is presented as a Module, which does not
 * reference any other modules. It is entirely self contained.
 */

# include  <typeinfo>
# include  <cstdlib>
# include  <cstring>
# include  <iostream>
# include  <sstream>
# include  <list>
# include  "pform.h"
# include  "PClass.h"
# include  "PEvent.h"
# include  "PGenerate.h"
# include  "PPackage.h"
# include  "PScope.h"
# include  "PSpec.h"
# include  "netlist.h"
# include  "netenum.h"
# include  "netvector.h"
# include  "netdarray.h"
# include  "netparray.h"
# include  "netclass.h"
# include  "netmisc.h"
# include  "util.h"
# include  "parse_api.h"
# include  "compiler.h"
# include  "ivl_assert.h"


// Implemented in elab_scope.cc
extern void set_scope_timescale(Design*des, NetScope*scope, PScope*pscope);

void PGate::elaborate(Design*, NetScope*) const
{
      cerr << "internal error: what kind of gate? " <<
	    typeid(*this).name() << endl;
}

/*
 * Elaborate the continuous assign. (This is *not* the procedural
 * assign.) Elaborate the lvalue and rvalue, and do the assignment.
 */
void PGAssign::elaborate(Design*des, NetScope*scope) const
{
      assert(scope);

      NetExpr* rise_time, *fall_time, *decay_time;
      eval_delays(des, scope, rise_time, fall_time, decay_time, true);

      ivl_drive_t drive0 = strength0();
      ivl_drive_t drive1 = strength1();

      assert(pin(0));
      assert(pin(1));

	/* Elaborate the l-value. */
      NetNet*lval = pin(0)->elaborate_lnet(des, scope);
      if (lval == 0) {
	    return;
      }

	// If this turns out to be an assignment to an unpacked array,
	// then handle that special case elsewhere.
      if (lval->pin_count() > 1) {
	    elaborate_unpacked_array_(des, scope, lval);
	    return;
      }

      ivl_assert(*this, lval->pin_count() == 1);

      if (debug_elaborate) {
	    cerr << get_fileline() << ": PGAssign::elaborate: elaborated l-value"
		 << " width=" << lval->vector_width()
		 << ", pin_count=" << lval->pin_count() << endl;
      }

      NetExpr*rval_expr = elaborate_rval_expr(des, scope, lval->net_type(),
					      lval->data_type(),
					      lval->vector_width(), pin(1));

      if (rval_expr == 0) {
	    cerr << get_fileline() << ": error: Unable to elaborate r-value: "
		 << *pin(1) << endl;
	    des->errors += 1;
	    return;
      }

      if (lval->enumeration()) {
	    if (! rval_expr->enumeration()) {
		  cerr << get_fileline() << ": error: "
		          "This assignment requires an explicit cast." << endl;
		  des->errors += 1;
	    } else if (! lval->enumeration()->matches(rval_expr->enumeration())) {
		  cerr << get_fileline() << ": error: "
		          "Enumeration type mismatch in assignment." << endl;
		  des->errors += 1;
	    }
      }

      NetNet*rval = rval_expr->synthesize(des, scope, rval_expr);

      if (rval == 0) {
	    cerr << get_fileline() << ": internal error: "
		 << "Failed to synthesize expression: " << *rval_expr << endl;
	    des->errors += 1;
	    return;
      }

      if (debug_elaborate) {
	    cerr << get_fileline() << ": debug: PGAssign: elaborated r-value"
		 << " width="<< rval->vector_width()
		 << ", type="<< rval->data_type()
		 << ", expr=" << *rval_expr << endl;
      }

      ivl_assert(*this, lval && rval);
      ivl_assert(*this, rval->pin_count() == 1);

	// Detect the case that the rvalue-expression is a simple
	// expression. In this case, we will need to create a driver
	// (later) to carry strengths.
      bool need_driver_flag = false;
      if (dynamic_cast<NetESignal*>(rval_expr))
	    need_driver_flag = true;

	// expression elaboration should have caused the rval width to
	// match the l-value by now.
      if (rval->vector_width() < lval->vector_width()) {
	    cerr << get_fileline() << ": internal error: "
		 << "lval-rval width mismatch: "
		 << "rval->vector_width()==" << rval->vector_width()
		 << ", lval->vector_width()==" << lval->vector_width() << endl;
      }
      ivl_assert(*this, rval->vector_width() >= lval->vector_width());

	/* If the r-value insists on being larger than the l-value,
	   use a part select to chop it down down to size. */
      if (lval->vector_width() < rval->vector_width()) {
	    NetPartSelect*tmp = new NetPartSelect(rval, 0,lval->vector_width(),
						  NetPartSelect::VP);
	    des->add_node(tmp);
	    tmp->set_line(*this);
	    netvector_t*osig_vec = new netvector_t(rval->data_type(),
						   lval->vector_width()-1,0);
	    NetNet*osig = new NetNet(scope, scope->local_symbol(),
				     NetNet::TRI, osig_vec);
	    osig->set_line(*this);
	    osig->local_flag(true);
	    connect(osig->pin(0), tmp->pin(0));
	    rval = osig;
	    need_driver_flag = false;
      }

	/* When we are given a non-default strength value and if the drive
	 * source is a bit, part, indexed select or a concatenation we need
	 * to add a driver (BUFZ) to convey the strength information. */
      if ((drive0 != IVL_DR_STRONG || drive1 != IVL_DR_STRONG) &&
          ((dynamic_cast<NetESelect*>(rval_expr)) ||
	   (dynamic_cast<NetEConcat*>(rval_expr)))) {
	    need_driver_flag = true;
      }

      if (need_driver_flag) {
	    NetBUFZ*driver = new NetBUFZ(scope, scope->local_symbol(),
					 rval->vector_width(), false);
	    driver->set_line(*this);
	    des->add_node(driver);

	    connect(rval->pin(0), driver->pin(1));

	    netvector_t*tmp_vec = new netvector_t(rval->data_type(),
						  rval->vector_width()-1,0);
	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    NetNet::WIRE, tmp_vec);
	    tmp->set_line(*this);
	    tmp->local_flag(true);

	    connect(driver->pin(0), tmp->pin(0));

	    rval = tmp;
      }

	/* Set the drive and delays for the r-val. */

      if (drive0 != IVL_DR_STRONG || drive1 != IVL_DR_STRONG)
	    rval->pin(0).drivers_drive(drive0, drive1);

      if (rise_time || fall_time || decay_time)
	    rval->pin(0).drivers_delays(rise_time, fall_time, decay_time);

      connect(lval->pin(0), rval->pin(0));

      if (lval->local_flag())
	    delete lval;

}

void PGAssign::elaborate_unpacked_array_(Design*des, NetScope*scope, NetNet*lval) const
{
      PEIdent*rval_pident = dynamic_cast<PEIdent*> (pin(1));
      ivl_assert(*this, rval_pident);

      NetNet*rval_net = rval_pident->elaborate_unpacked_net(des, scope);

      ivl_assert(*this, rval_net->pin_count() == lval->pin_count());

      assign_unpacked_with_bufz(des, scope, this, lval, rval_net);
}

unsigned PGBuiltin::calculate_array_count_(Design*des, NetScope*scope,
					   long&high, long&low) const
{
      unsigned count = 1;
      high = 0;
      low = 0;

	/* If the Verilog source has a range specification for the
	   gates, then I am expected to make more than one
	   gate. Figure out how many are desired. */
      if (msb_) {
	    NetExpr*msb_exp = elab_and_eval(des, scope, msb_, -1, true);
	    NetExpr*lsb_exp = elab_and_eval(des, scope, lsb_, -1, true);

	    NetEConst*msb_con = dynamic_cast<NetEConst*>(msb_exp);
	    NetEConst*lsb_con = dynamic_cast<NetEConst*>(lsb_exp);

	    if (msb_con == 0) {
		  cerr << get_fileline() << ": error: Unable to evaluate "
			"expression " << *msb_ << endl;
		  des->errors += 1;
		  return 0;
	    }

	    if (lsb_con == 0) {
		  cerr << get_fileline() << ": error: Unable to evaluate "
			"expression " << *lsb_ << endl;
		  des->errors += 1;
		  return 0;
	    }

	    verinum msb = msb_con->value();
	    verinum lsb = lsb_con->value();

	    delete msb_exp;
	    delete lsb_exp;

	    if (msb.as_long() > lsb.as_long())
		  count = msb.as_long() - lsb.as_long() + 1;
	    else
		  count = lsb.as_long() - msb.as_long() + 1;

	    low = lsb.as_long();
	    high = msb.as_long();

	    if (debug_elaborate) {
		  cerr << get_fileline() << ": debug: PGBuiltin: Make array "
		       << "[" << high << ":" << low << "]" << " of "
		       << count << " gates for " << get_name() << endl;
	    }
      }

      return count;
}

void PGBuiltin::calculate_gate_and_lval_count_(unsigned&gate_count,
                                               unsigned&lval_count) const
{
      switch (type()) {
	  case BUF:
	  case NOT:
	    if (pin_count() > 2) gate_count = pin_count() - 1;
	    else gate_count = 1;
            lval_count = gate_count;
	    break;
	  case PULLDOWN:
	  case PULLUP:
	    gate_count = pin_count();
            lval_count = gate_count;
	    break;
	  case TRAN:
	  case RTRAN:
	  case TRANIF0:
	  case TRANIF1:
	  case RTRANIF0:
	  case RTRANIF1:
	    gate_count = 1;
            lval_count = 2;
	    break;
	  default:
	    gate_count = 1;
            lval_count = 1;
	    break;
      }
}

NetNode* PGBuiltin::create_gate_for_output_(Design*des, NetScope*scope,
					    perm_string inst_name,
					    unsigned instance_width) const
{
      NetNode*gate = 0;

      switch (type()) {

	  case AND:
	    if (pin_count() < 2) {
		  cerr << get_fileline() << ": error: the AND "
			"primitive must have an input." << endl;
		  des->errors += 1;
	    } else {
		  gate = new NetLogic(scope, inst_name, pin_count(),
				      NetLogic::AND, instance_width);
	    }
	    break;

	  case BUF:
	    if (pin_count() < 2) {
		  cerr << get_fileline() << ": error: the BUF "
			"primitive must have an input." << endl;
		  des->errors += 1;
	    } else {
		  gate = new NetLogic(scope, inst_name, 2,
		                      NetLogic::BUF, instance_width);
	    }
	    break;

	  case BUFIF0:
	    if (pin_count() != 3) {
		  cerr << get_fileline() << ": error: the BUFIF0 "
			"primitive must have three arguments." << endl;
		  des->errors += 1;
	    } else {
		  gate = new NetLogic(scope, inst_name, pin_count(),
					  NetLogic::BUFIF0, instance_width);
	    }
	    break;

	  case BUFIF1:
	    if (pin_count() != 3) {
		  cerr << get_fileline() << ": error: the BUFIF1 "
			"primitive must have three arguments." << endl;
		  des->errors += 1;
	    } else {
		  gate = new NetLogic(scope, inst_name, pin_count(),
					  NetLogic::BUFIF1, instance_width);
	    }
	    break;

	  case CMOS:
	    if (pin_count() != 4) {
		  cerr << get_fileline() << ": error: the CMOS "
			"primitive must have four arguments." << endl;
		  des->errors += 1;
	    } else {
		  gate = new NetLogic(scope, inst_name, pin_count(),
				      NetLogic::CMOS, instance_width);
	    }
	    break;

	  case NAND:
	    if (pin_count() < 2) {
		  cerr << get_fileline() << ": error: the NAND "
			"primitive must have an input." << endl;
		  des->errors += 1;
	    } else {
		  gate = new NetLogic(scope, inst_name, pin_count(),
				      NetLogic::NAND, instance_width);
	    }
	    break;

	  case NMOS:
	    if (pin_count() != 3) {
		  cerr << get_fileline() << ": error: the NMOS "
			"primitive must have three arguments." << endl;
		  des->errors += 1;
	    } else {
		  gate = new NetLogic(scope, inst_name, pin_count(),
				      NetLogic::NMOS, instance_width);
	    }
	    break;

	  case NOR:
	    if (pin_count() < 2) {
		  cerr << get_fileline() << ": error: the NOR "
			"primitive must have an input." << endl;
		  des->errors += 1;
	    } else {
		  gate = new NetLogic(scope, inst_name, pin_count(),
				      NetLogic::NOR, instance_width);
	    }
	    break;

	  case NOT:
	    if (pin_count() < 2) {
		  cerr << get_fileline() << ": error: the NOT "
			"primitive must have an input." << endl;
		  des->errors += 1;
	    } else {
		  gate = new NetLogic(scope, inst_name, 2,
		                      NetLogic::NOT, instance_width);
	    }
	    break;

	  case NOTIF0:
	    if (pin_count() != 3) {
		  cerr << get_fileline() << ": error: the NOTIF0 "
			"primitive must have three arguments." << endl;
		  des->errors += 1;
	    } else {
		  gate = new NetLogic(scope, inst_name, pin_count(),
				      NetLogic::NOTIF0, instance_width);
	    }
	    break;

	  case NOTIF1:
	    if (pin_count() != 3) {
		  cerr << get_fileline() << ": error: the NOTIF1 "
			"primitive must have three arguments." << endl;
		  des->errors += 1;
	    } else {
		  gate = new NetLogic(scope, inst_name, pin_count(),
				      NetLogic::NOTIF1, instance_width);
	    }
	    break;

	  case OR:
	    if (pin_count() < 2) {
		  cerr << get_fileline() << ": error: the OR "
			"primitive must have an input." << endl;
		  des->errors += 1;
	    } else {
		  gate = new NetLogic(scope, inst_name, pin_count(),
				      NetLogic::OR, instance_width);
	    }
	    break;

	  case RCMOS:
	    if (pin_count() != 4) {
		  cerr << get_fileline() << ": error: the RCMOS "
			"primitive must have four arguments." << endl;
		  des->errors += 1;
	    } else {
		  gate = new NetLogic(scope, inst_name, pin_count(),
				      NetLogic::RCMOS, instance_width);
	    }
	    break;

	  case RNMOS:
	    if (pin_count() != 3) {
		  cerr << get_fileline() << ": error: the RNMOS "
			"primitive must have three arguments." << endl;
		  des->errors += 1;
	    } else {
		  gate = new NetLogic(scope, inst_name, pin_count(),
				      NetLogic::RNMOS, instance_width);
	    }
	    break;

	  case RPMOS:
	    if (pin_count() != 3) {
		  cerr << get_fileline() << ": error: the RPMOS "
			"primitive must have three arguments." << endl;
		  des->errors += 1;
	    } else {
		  gate = new NetLogic(scope, inst_name, pin_count(),
				      NetLogic::RPMOS, instance_width);
	    }
	    break;

	  case PMOS:
	    if (pin_count() != 3) {
		  cerr << get_fileline() << ": error: the PMOS "
			"primitive must have three arguments." << endl;
		  des->errors += 1;
	    } else {
		  gate = new NetLogic(scope, inst_name, pin_count(),
				      NetLogic::PMOS, instance_width);
	    }
	    break;

	  case PULLDOWN:
	    gate = new NetLogic(scope, inst_name, 1,
				NetLogic::PULLDOWN, instance_width);
	    break;

	  case PULLUP:
	    gate = new NetLogic(scope, inst_name, 1,
				NetLogic::PULLUP, instance_width);
	    break;

	  case XNOR:
	    if (pin_count() < 2) {
		  cerr << get_fileline() << ": error: the XNOR "
			"primitive must have an input." << endl;
		  des->errors += 1;
	    } else {
		  gate = new NetLogic(scope, inst_name, pin_count(),
				      NetLogic::XNOR, instance_width);
	    }
	    break;

	  case XOR:
	    if (pin_count() < 2) {
		  cerr << get_fileline() << ": error: the XOR "
			"primitive must have an input." << endl;
		  des->errors += 1;
	    } else {
		  gate = new NetLogic(scope, inst_name, pin_count(),
				      NetLogic::XOR, instance_width);
	    }
	    break;

	  case TRAN:
	    if (pin_count() != 2) {
		  cerr << get_fileline() << ": error: Pin count for "
		       << "tran device." << endl;
		  des->errors += 1;
	    } else {
		  gate = new NetTran(scope, inst_name, IVL_SW_TRAN,
		                     instance_width);
	    }
	    break;

	  case RTRAN:
	    if (pin_count() != 2) {
		  cerr << get_fileline() << ": error: Pin count for "
		       << "rtran device." << endl;
		  des->errors += 1;
	    } else {
		  gate = new NetTran(scope, inst_name, IVL_SW_RTRAN,
		                     instance_width);
	    }
	    break;

	  case TRANIF0:
	    if (pin_count() != 3) {
		  cerr << get_fileline() << ": error: Pin count for "
		       << "tranif0 device." << endl;
		  des->errors += 1;
	    } else {
		  gate = new NetTran(scope, inst_name, IVL_SW_TRANIF0,
		                     instance_width);
	    }
	    break;

	  case RTRANIF0:
	    if (pin_count() != 3) {
		  cerr << get_fileline() << ": error: Pin count for "
		       << "rtranif0 device." << endl;
		  des->errors += 1;
	    } else {
		  gate = new NetTran(scope, inst_name, IVL_SW_RTRANIF0,
		                     instance_width);
	    }
	    break;

	  case TRANIF1:
	    if (pin_count() != 3) {
		  cerr << get_fileline() << ": error: Pin count for "
		       << "tranif1 device." << endl;
		  des->errors += 1;
	    } else {
		  gate = new NetTran(scope, inst_name, IVL_SW_TRANIF1,
		                     instance_width);
	    }
	    break;

	  case RTRANIF1:
	    if (pin_count() != 3) {
		  cerr << get_fileline() << ": error: Pin count for "
		       << "rtranif1 device." << endl;
		  des->errors += 1;
	    } else {
		  gate = new NetTran(scope, inst_name, IVL_SW_RTRANIF1,
		                     instance_width);
	    }
	    break;

	  default:
	    cerr << get_fileline() << ": internal error: unhandled "
		  "gate type." << endl;
	    des->errors += 1;
	    break;
      }

      return gate;
}

bool PGBuiltin::check_delay_count(Design*des) const
{
      switch (type()) {
	  case AND:
	  case NAND:
	  case OR:
	  case NOR:
	  case XOR:
	  case XNOR:
	  case BUF:
	  case NOT:
	    if (delay_count() > 2) {
		  cerr << get_fileline() << ": error: More than two delays "
		       << "given to a " << gate_name() << " gate." << endl;
		  des->errors += 1;
		  return true;
	    }
	    break;

	  case BUFIF0:
	  case NOTIF0:
	  case BUFIF1:
	  case NOTIF1:
	    if (delay_count() > 3) {
		  cerr << get_fileline() << ": error: More than three delays "
		       << "given to a " << gate_name() << " gate." << endl;
		  des->errors += 1;
		  return true;
	    }
	    break;

	  case NMOS:
	  case RNMOS:
	  case PMOS:
	  case RPMOS:
	  case CMOS:
	  case RCMOS:
	    if (delay_count() > 3) {
		  cerr << get_fileline() << ": error: More than three delays "
		       << "given to a " << gate_name() << " switch." << endl;
		  des->errors += 1;
		  return true;
	    }
	    break;

	  case TRAN:
	  case RTRAN:
	    if (delay_count() != 0) {
		  cerr << get_fileline() << ": error: A " << gate_name()
		       << " switch does not take any delays." << endl;
		  des->errors += 1;
		  return true;
	    }
	    break;

	  case TRANIF0:
	  case TRANIF1:
	    if (delay_count() > 2) {
		  cerr << get_fileline() << ": error: More than two delays "
		       << "given to a " << gate_name() << " switch." << endl;
		  des->errors += 1;
		  return true;
	    }
	    break;

	  case RTRANIF0:
	  case RTRANIF1:
	    if (delay_count() > 2) {
		  cerr << get_fileline() << ": error: More than two delays "
		       << "given to an " << gate_name() << " switch." << endl;
		  des->errors += 1;
		  return true;
	    }
	    break;

	  case PULLUP:
	  case PULLDOWN:
	    if (delay_count() != 0) {
		  cerr << get_fileline() << ": error: A " << gate_name()
		       << " source does not take any delays." << endl;
		  des->errors += 1;
		  return true;
	    }
	    break;

	  default:
	    cerr << get_fileline() << ": internal error: unhandled "
		  "gate type." << endl;
	    des->errors += 1;
	    return true;
	    break;
      }

      return false;
}

/*
 * Elaborate a Builtin gate. These normally get translated into
 * NetLogic nodes that reflect the particular logic function.
 */
void PGBuiltin::elaborate(Design*des, NetScope*scope) const
{
      unsigned instance_width = 1;
      perm_string name = get_name();

      if (name == "") name = scope->local_symbol();

	/* Calculate the array bounds and instance count for the gate,
	   as described in the Verilog source. If there is none, then
	   the count is 1, and high==low==0. */

      long low=0, high=0;
      unsigned array_count = calculate_array_count_(des, scope, high, low);
      if (array_count == 0) return;

      unsigned gate_count = 0, lval_count = 0;
      calculate_gate_and_lval_count_(gate_count, lval_count);

	/* Now we have a gate count. Elaborate the lval (output or
           bi-directional) expressions only. We do it early so that
           we can see if we can make wide gates instead of an array
           of gates. */

      vector<NetNet*>lval_sigs (lval_count);

      for (unsigned idx = 0 ; idx < lval_count ; idx += 1) {
	    if (pin(idx) == 0) {
		  cerr << get_fileline() << ": error: Logic gate port "
			"expressions are not optional." << endl;
		  des->errors += 1;
		  return;
	    }
            if (lval_count > gate_count)
	          lval_sigs[idx] = pin(idx)->elaborate_bi_net(des, scope);
            else
	          lval_sigs[idx] = pin(idx)->elaborate_lnet(des, scope);

	      // The only way this should return zero is if an error
	      // happened, so for that case just return.
	    if (lval_sigs[idx] == 0) return;

	      // For now, assume all the outputs are the same width.
	    ivl_assert(*this, idx == 0 || lval_sigs[idx]->vector_width() == lval_sigs[0]->vector_width());
      }

	/* Detect the special case that the l-value width exactly
	   matches the gate count. In this case, we will make a single
	   gate that has the desired vector width.

	   NOTE: This assumes that all the outputs have the same
	   width. For gates with 1 output, this is trivially true. */
      if (lval_sigs[0]->vector_width() == array_count) {
	    instance_width = array_count;
	    array_count = 1;

	    if (debug_elaborate && instance_width != 1)
		  cerr << get_fileline() << ": debug: PGBuiltin: "
			"Collapsed gate array into single wide "
			"(" << instance_width << ") instance." << endl;
      }

	/* Calculate the gate delays from the delay expressions
	   given in the source. For logic gates, the decay time
	   is meaningless because it can never go to high
	   impedance. However, the bufif devices can generate
	   'bz output, so we will pretend that anything can.

	   If only one delay value expression is given (i.e., #5
	   nand(foo,...)) then rise, fall and decay times are
	   all the same value. If two values are given, rise and
	   fall times are use, and the decay time is the minimum
	   of the rise and fall times. Finally, if all three
	   values are given, they are taken as specified. */

      if (check_delay_count(des)) return;
      NetExpr* rise_time, *fall_time, *decay_time;
      eval_delays(des, scope, rise_time, fall_time, decay_time, true);

      struct attrib_list_t*attrib_list;
      unsigned attrib_list_n = 0;
      attrib_list = evaluate_attributes(attributes, attrib_list_n,
					des, scope);

	/* Allocate all the netlist nodes for the gates. */
      vector<NetNode*>cur (array_count*gate_count);

	/* Now make as many gates as the bit count dictates. Give each
	   a unique name, and set the delay times. */

      for (unsigned idx = 0 ;  idx < array_count*gate_count ;  idx += 1) {
	    unsigned array_idx = idx/gate_count;
	    unsigned gate_idx = idx%gate_count;

	    ostringstream tmp;
	    unsigned index = (low < high)? (low+array_idx) : (low-array_idx);

	    tmp << name << "<" << index << "." << gate_idx << ">";
	    perm_string inm = lex_strings.make(tmp.str());

	    cur[idx] = create_gate_for_output_(des, scope, inm, instance_width);
	    if (cur[idx] == 0)
		  return;

	    for (unsigned adx = 0 ;  adx < attrib_list_n ;  adx += 1)
		  cur[idx]->attribute(attrib_list[adx].key,
				      attrib_list[adx].val);

	      /* Set the delays and drive strength for all built in gates. */
	    cur[idx]->rise_time(rise_time);
	    cur[idx]->fall_time(fall_time);
	    cur[idx]->decay_time(decay_time);

	    cur[idx]->pin(0).drive0(strength0());
	    cur[idx]->pin(0).drive1(strength1());

	    cur[idx]->set_line(*this);
	    des->add_node(cur[idx]);
      }


      delete[]attrib_list;

	/* The gates have all been allocated, this loop runs through
	   the parameters and attaches the ports of the objects. */

      for (unsigned idx = 0 ;  idx < pin_count() ;  idx += 1) {

	    PExpr*ex = pin(idx);
	    if (ex == 0) {
		  cerr << get_fileline() << ": error: Logic gate port "
		          "expressions are not optional." << endl;
		  des->errors += 1;
		  return;
	    }
	    NetNet*sig = 0;
	    if (idx < lval_count) {
		  sig = lval_sigs[idx];

	    } else {
                    // If this is an array, the port expression is required
                    // to be the exact width required (this will be checked
                    // later). But if this is a single instance, consensus
                    // is that we just take the LSB of the port expression.
		  NetExpr*tmp = elab_and_eval(des, scope, ex, msb_ ? -1 : 1);
                  if (tmp == 0)
                        continue;
                  if (msb_ == 0 && tmp->expr_width() != 1)
                        tmp = new NetESelect(tmp, make_const_0(1), 1,
                                             IVL_SEL_IDX_UP);
		  sig = tmp->synthesize(des, scope, tmp);
		  delete tmp;
	    }

	    if (sig == 0)
		  continue;

	    ivl_assert(*this, sig);

	    if (array_count == 1) {
		    /* Handle the case where there is one gate that
		       carries the whole vector width. */

		  if (1 == sig->vector_width() && instance_width != 1) {

			assert(sig->vector_width() == 1);
			NetReplicate*rep
			      = new NetReplicate(scope,
						 scope->local_symbol(),
						 instance_width,
						 instance_width);
			rep->set_line(*this);
			des->add_node(rep);
			connect(rep->pin(1), sig->pin(0));

			netvector_t*osig_vec = new netvector_t(IVL_VT_LOGIC,
							       instance_width-1,0);
			sig = new NetNet(scope, scope->local_symbol(),
					 NetNet::WIRE, osig_vec);
			sig->set_line(*this);
			sig->local_flag(true);
			connect(rep->pin(0), sig->pin(0));

		  }

		  if (instance_width != sig->vector_width()) {

			cerr << get_fileline() << ": error: "
			     << "Expression width " << sig->vector_width()
			     << " does not match width " << instance_width
			     << " of logic gate array port " << idx+1
			     << "." << endl;
			des->errors += 1;
		  }

		    // There is only 1 instance, but there may be
		    // multiple outputs to that gate. That would
		    // potentially mean multiple actual gates.
		    // Although in Verilog proper a multiple
		    // output gate has only 1 input, this conditional
		    // handles gates with N outputs and M inputs.
		  if (idx < gate_count) {
			connect(cur[idx]->pin(0), sig->pin(0));
		  } else {
			for (unsigned dev = 0 ; dev < gate_count; dev += 1)
			      connect(cur[dev]->pin(idx-gate_count+1), sig->pin(0));
		  }

	    } else if (sig->vector_width() == 1) {

		    /* Handle the case where a single bit is connected
		       repetitively to all the instances. If idx is an
		       output port, connect it to all array_count
		       devices that have outputs at this
		       position. Otherwise, idx is an input to all
		       array_count*gate_count devices. */

		  if (idx < gate_count) {
			for (unsigned gdx = 0 ; gdx < array_count ; gdx += 1) {
			      unsigned dev = gdx*gate_count;
			      connect(cur[dev+idx]->pin(0), sig->pin(0));
			}
		  } else {
			unsigned use_idx = idx - gate_count + 1;
			for (unsigned gdx = 0 ;  gdx < cur.size() ;  gdx += 1)
			      connect(cur[gdx]->pin(use_idx), sig->pin(0));
		  }

	    } else if (sig->vector_width() == array_count) {

                    /* Bi-directional switches should get collapsed into
                       a single wide instance, so should never reach this
                       point. Check this is so, as the following code
                       doesn't handle bi-directional connections. */
                  ivl_assert(*this, lval_count == gate_count);

		    /* Handle the general case that each bit of the
		       value is connected to a different instance. In
		       this case, the output is handled slightly
		       different from the inputs. */
		  if (idx < gate_count) {
			NetConcat*cc = new NetConcat(scope,
						     scope->local_symbol(),
						     sig->vector_width(),
						     array_count);
			cc->set_line(*this);
			des->add_node(cc);

			  /* Connect the concat to the signal. */
			connect(cc->pin(0), sig->pin(0));

			  /* Connect the outputs of the gates to the concat. */
			for (unsigned gdx = 0 ;  gdx < array_count;  gdx += 1) {
			      unsigned dev = gdx*gate_count;
			      connect(cur[dev+idx]->pin(0), cc->pin(gdx+1));

			      netvector_t*tmp2_vec = new netvector_t(IVL_VT_LOGIC);
			      NetNet*tmp2 = new NetNet(scope,
						       scope->local_symbol(),
						       NetNet::WIRE, tmp2_vec);
			      tmp2->set_line(*this);
			      tmp2->local_flag(true);
			      connect(cc->pin(gdx+1), tmp2->pin(0));
			}

		  } else for (unsigned gdx = 0 ;  gdx < array_count ;  gdx += 1) {
			  /* Use part selects to get the bits
			     connected to the inputs of out gate. */
			NetPartSelect*tmp1 = new NetPartSelect(sig, gdx, 1,
							   NetPartSelect::VP);
			tmp1->set_line(*this);
			des->add_node(tmp1);
			connect(tmp1->pin(1), sig->pin(0));
			netvector_t*tmp2_vec = new netvector_t(sig->data_type());
			NetNet*tmp2 = new NetNet(scope, scope->local_symbol(),
						 NetNet::WIRE, tmp2_vec);
			tmp2->set_line(*this);
			tmp2->local_flag(true);
			connect(tmp1->pin(0), tmp2->pin(0));
			unsigned use_idx = idx - gate_count + 1;
			unsigned dev = gdx*gate_count;
			for (unsigned gdx2 = 0 ; gdx2 < gate_count ; gdx2 += 1)
			      connect(cur[dev+gdx2]->pin(use_idx), tmp1->pin(0));
		  }

	    } else {
		  cerr << get_fileline() << ": error: Gate count of " <<
			array_count << " does not match net width of " <<
			sig->vector_width() << " at pin " << idx << "."
		       << endl;
		  des->errors += 1;
	    }

      }

}

NetNet*PGModule::resize_net_to_port_(Design*des, NetScope*scope,
				     NetNet*sig, unsigned port_wid,
				     NetNet::PortType dir, bool as_signed) const
{
      ivl_assert(*this, dir != NetNet::NOT_A_PORT);
      ivl_assert(*this, dir != NetNet::PIMPLICIT);

      netvector_t*tmp_type = new netvector_t(IVL_VT_LOGIC, port_wid-1, 0);
      NetNet*tmp = new NetNet(scope, scope->local_symbol(),
			      NetNet::WIRE, tmp_type);
      tmp->local_flag(true);
      tmp->set_line(*this);

	// Handle the special case of a bi-directional part
	// select. Create a NetTran(VP) instead of a uni-directional
	// NetPartSelect node.
      if (dir == NetNet::PINOUT) {
	    unsigned wida = sig->vector_width();
	    unsigned widb = tmp->vector_width();
	    bool part_b = widb < wida;
	      // This needs to pad the value!
	      // Also delete the inout specific warning when this is fixed.
	      // It is located just before this routine is called.
	    NetTran*node = new NetTran(scope, scope->local_symbol(),
				       part_b? wida : widb,
				       part_b? widb : wida,
				       0);
	    if (part_b) {
		  connect(node->pin(0), sig->pin(0));
		  connect(node->pin(1), tmp->pin(0));
	    } else {
		  connect(node->pin(0), tmp->pin(0));
		  connect(node->pin(1), sig->pin(0));
	    }

	    node->set_line(*this);
	    des->add_node(node);

	    return tmp;
      }

      unsigned pwidth = tmp->vector_width();
      unsigned swidth = sig->vector_width();
      switch (dir) {
	  case NetNet::POUTPUT:
	    if (pwidth > swidth) {
		  NetPartSelect*node = new NetPartSelect(tmp, 0, swidth,
					   NetPartSelect::VP);
		  connect(node->pin(0), sig->pin(0));
		  des->add_node(node);
	    } else {
		  NetNet*osig;
		  if (as_signed) {
			osig = pad_to_width_signed(des, tmp, swidth, *this);
		  } else {
			osig = pad_to_width(des, tmp, swidth, *this);
		  }
		  connect(osig->pin(0), sig->pin(0));
	    }
	    break;

	  case NetNet::PINPUT:
	    if (pwidth > swidth) {
		  delete tmp;
		  if (as_signed) {
			tmp = pad_to_width_signed(des, sig, pwidth, *this);
		  } else {
			tmp = pad_to_width(des, sig, pwidth, *this);
		  }
	    } else {
		  NetPartSelect*node = new NetPartSelect(sig, 0, pwidth,
					   NetPartSelect::VP);
		  connect(node->pin(0), tmp->pin(0));
		  des->add_node(node);
	    }
	    break;

	  case NetNet::PINOUT:
	    ivl_assert(*this, 0);
	    break;

	  case NetNet::PREF:
	    ivl_assert(*this, 0);
	    break;

	  default:
	    ivl_assert(*this, 0);
      }

      return tmp;
}

static bool need_bufz_for_input_port(const vector<NetNet*>&prts)
{
      if (prts[0]->port_type() != NetNet::PINPUT)
	    return false;

      if (prts[0]->pin(0).nexus()->drivers_present())
	    return true;

      return false;
}

/*
 * Convert a wire or tri to a tri0 or tri1 as needed to make
 * an unconnected drive pull for floating inputs.
 */
static void convert_net(Design*des, const LineInfo *line,
                        NetNet *net, NetNet::Type type)
{
	// If the types already match just return.
      if (net->type() == type) return;

	// We can only covert a wire or tri to have a default pull.
      if (net->type() == NetNet::WIRE || net->type() == NetNet::TRI) {
	    net->type(type);
	    return;
      }

	// We may have to support this at some point in time!
      cerr << line->get_fileline() << ": sorry: Can not pull floating "
              "input type '" << net->type() << "'." << endl;
      des->errors += 1;
}

static void isolate_and_connect(Design*des, NetScope*scope, const PGModule*mod,
				NetNet*port, NetNet*sig, NetNet::PortType ptype)
{
      switch (ptype) {
	  case NetNet::POUTPUT:
	    {
		  NetBUFZ*tmp = new NetBUFZ(scope, scope->local_symbol(),
					    sig->vector_width(), true);
		  tmp->set_line(*mod);
		  des->add_node(tmp);
		  connect(tmp->pin(1), port->pin(0));
		  connect(tmp->pin(0), sig->pin(0));
	    }
	    break;
	  case NetNet::PINOUT:
	    {
		  NetTran*tmp = new NetTran(scope, scope->local_symbol(),
					    sig->vector_width(),
					    sig->vector_width(), 0);
		  tmp->set_line(*mod);
		  des->add_node(tmp);
		  connect(tmp->pin(1), port->pin(0));
		  connect(tmp->pin(0), sig->pin(0));
	    }
	    break;
	  default:
	    ivl_assert(*mod, 0);
	    break;
      }
}

/*
 * Instantiate a module by recursively elaborating it. Set the path of
 * the recursive elaboration so that signal names get properly
 * set. Connect the ports of the instantiated module to the signals of
 * the parameters. This is done with BUFZ gates so that they look just
 * like continuous assignment connections.
 */
void PGModule::elaborate_mod_(Design*des, Module*rmod, NetScope*scope) const
{

      assert(scope);

      if (debug_elaborate) {
	    cerr << get_fileline() << ": debug: Instantiate module "
		 << rmod->mod_name() << " with instance name "
		 << get_name() << " in scope " << scope_path(scope) << endl;
      }

	// This is the array of pin expressions, shuffled to match the
	// order of the declaration. If the source instantiation uses
	// bind by order, this is the same as the source list. Otherwise,
	// the source list is rearranged by name binding into this list.
      vector<PExpr*>pins (rmod->port_count());
      vector<bool>pins_fromwc (rmod->port_count(), false);
      vector<bool>pins_is_explicitly_not_connected (rmod->port_count(), false);

	// If the instance has a pins_ member, then we know we are
	// binding by name. Therefore, make up a pins array that
	// reflects the positions of the named ports.
      if (pins_) {
	    unsigned nexp = rmod->port_count();

	      // Scan the bindings, matching them with port names.
	    for (unsigned idx = 0 ;  idx < npins_ ;  idx += 1) {

		    // Handle wildcard named port
		  if (pins_[idx].name[0] == '*') {
			for (unsigned j = 0 ; j < nexp ; j += 1) {
			      if ((!pins[j]) && (!pins_is_explicitly_not_connected[j])) {
				    pins_fromwc[j] = true;
				    NetNet*       net = 0;
				    const NetExpr*par = 0;
				    NetEvent*     eve = 0;
				    pform_name_t path_;
				    path_.push_back(name_component_t(rmod->ports[j]->name));
				    symbol_search(this, des, scope,
						  path_, net, par, eve);
				    if (net != 0) {
					  pins[j] = new PEIdent(rmod->ports[j]->name, true);
					  pins[j]->set_lineno(get_lineno());
					  pins[j]->set_file(get_file());
				    }
			      }
			}
			continue;
		  }

		    // Given a binding, look at the module port names
		    // for the position that matches the binding name.
		  unsigned pidx = rmod->find_port(pins_[idx].name);

		    // If the port name doesn't exist, the find_port
		    // method will return the port count. Detect that
		    // as an error.
		  if (pidx == nexp) {
			cerr << get_fileline() << ": error: port ``" <<
			      pins_[idx].name << "'' is not a port of "
			     << get_name() << "." << endl;
			des->errors += 1;
			continue;
		  }

		    // If I am overriding a wildcard port, delete and
		    // override it
		  if (pins_fromwc[pidx]) {
			delete pins[pidx];
			pins_fromwc[pidx] = false;

		    // If I already explicitly bound something to
		    // this port, then the pins array will already
		    // have a pointer value where I want to place this
		    // expression.
		  } else if (pins[pidx]) {
			cerr << get_fileline() << ": error: port ``" <<
			      pins_[idx].name << "'' already bound." <<
			      endl;
			des->errors += 1;
			continue;
		  }

		    // OK, do the binding by placing the expression in
		    // the right place.
		  pins[pidx] = pins_[idx].parm;
		  if (!pins[pidx])
			pins_is_explicitly_not_connected[pidx] = true;
	    }


      } else if (pin_count() == 0) {

	      /* Handle the special case that no ports are
		 connected. It is possible that this is an empty
		 connect-by-name list, so we'll allow it and assume
		 that is the case. */

	    for (unsigned idx = 0 ;  idx < rmod->port_count() ;  idx += 1)
		  pins[idx] = 0;

      } else {

	      /* Otherwise, this is a positional list of port
		 connections. In this case, the port count must be
		 right. Check that is is, the get the pin list. */

	    if (pin_count() != rmod->port_count()) {
		  cerr << get_fileline() << ": error: Wrong number "
			"of ports. Expecting " << rmod->port_count() <<
			", got " << pin_count() << "."
		       << endl;
		  des->errors += 1;
		  return;
	    }

	      // No named bindings, just use the positional list I
	      // already have.
	    assert(pin_count() == rmod->port_count());
	    pins = get_pins();
      }

	// Elaborate these instances of the module. The recursive
	// elaboration causes the module to generate a netlist with
	// the ports represented by NetNet objects. I will find them
	// later.

      NetScope::scope_vec_t&instance = scope->instance_arrays[get_name()];
      if (debug_elaborate) cerr << get_fileline() << ": debug: start "
	    "recursive elaboration of " << instance.size() << " instance(s) of " <<
	    get_name() << "..." << endl;
      for (unsigned inst = 0 ;  inst < instance.size() ;  inst += 1) {
	    rmod->elaborate(des, instance[inst]);
	    instance[inst]->set_num_ports( rmod->port_count() );
      }
      if (debug_elaborate) cerr << get_fileline() << ": debug: ...done." << endl;


	// Now connect the ports of the newly elaborated designs to
	// the expressions that are the instantiation parameters. Scan
	// the pins, elaborate the expressions attached to them, and
	// bind them to the port of the elaborated module.

	// This can get rather complicated because the port can be
	// unconnected (meaning an empty parameter is passed) connected
	// to a concatenation, or connected to an internally
	// unconnected port.

      for (unsigned idx = 0 ;  idx < pins.size() ;  idx += 1) {
	    bool unconnected_port = false;
	    bool using_default = false;

	    perm_string port_name = rmod->get_port_name(idx);

	      // If the port is unconnected, substitute the default
	      // value. The parser ensures that a default value only
	      // exists for input ports.
	    if (pins[idx] == 0) {
		  PExpr*default_value = rmod->get_port_default_value(idx);
		  if (default_value) {
			pins[idx] = default_value;
			using_default = true;
		  }
	    }

	      // Skip unconnected module ports. This happens when a
	      // null parameter is passed in and there is no default
	      // value.
	    if (pins[idx] == 0) {

		  if (pins_fromwc[idx]) {
			cerr << get_fileline() << ": error: Wildcard named "
			        "port connection (.*) did not find a matching "
			        "identifier for port " << (idx+1) << " ("
			     << port_name << ")." << endl;
			des->errors += 1;
			return;
		  }

		    // We need this information to support the
		    // unconnected_drive directive and for a
		    // unconnected input warning when asked for.
		  vector<PEIdent*> mport = rmod->get_port(idx);
		  if (mport.empty()) continue;

		  perm_string pname = peek_tail_name(mport[0]->path());

		  NetNet*tmp = instance[0]->find_signal(pname);

		    // Handle the error case where there is no internal
		    // signal connected to the port.
		  if (!tmp) continue;
		  assert(tmp);

		  if (tmp->port_type() == NetNet::PINPUT) {
			  // If we have an unconnected input convert it
			  // as needed if an unconnected_drive directive
			  // was given. This only works for tri or wire!
			switch (rmod->uc_drive) {
			    case Module::UCD_PULL0:
			      convert_net(des, this, tmp, NetNet::TRI0);
			      break;
			    case Module::UCD_PULL1:
			      convert_net(des, this, tmp, NetNet::TRI1);
			      break;
			    case Module::UCD_NONE:
			      break;
			}

			  // Print a warning for an unconnected input.
			if (warn_portbinding) {
			      cerr << get_fileline() << ": warning: "
				   << "Instantiating module "
				   << rmod->mod_name()
				   << " with dangling input port "
				   << (idx+1) << " (" << port_name;
			      switch (rmod->uc_drive) {
				  case Module::UCD_PULL0:
				    cerr << ") pulled low." << endl;
				    break;
				  case Module::UCD_PULL1:
				    cerr << ") pulled high." << endl;
				    break;
				  case Module::UCD_NONE:
				    cerr << ") floating." << endl;
				    break;
			      }
			}
		  }
		  unconnected_port = true;
	    }

	      // Inside the module, the port connects zero or more signals
	      // that were already elaborated. List all those signals
	      // and the NetNet equivalents, for all the instances.
	    vector<PEIdent*> mport = rmod->get_port(idx);
	    vector<NetNet*>  prts (mport.size() * instance.size());

	    if (debug_elaborate) {
		  cerr << get_fileline() << ": debug: " << get_name()
		       << ": Port " << (idx+1) << " (" << port_name
		       << ") has " << prts.size() << " sub-ports." << endl;
	    }

	      // Count the internal vector bits of the port.
	    unsigned prts_vector_width = 0;

	    for (unsigned inst = 0 ;  inst < instance.size() ;  inst += 1) {
		    // Scan the instances from MSB to LSB. The port
		    // will be assembled in that order as well.
		  NetScope*inst_scope = instance[instance.size()-inst-1];

		  unsigned int prt_vector_width = 0;
		  PortType::Enum ptype = PortType::PIMPLICIT;
		    // Scan the module sub-ports for this instance...
		    // (Sub-ports are concatenated ports that form the
		    // single port for the instance. This is not a
		    // commonly used feature.)
		  for (unsigned ldx = 0 ;  ldx < mport.size() ;  ldx += 1) {
			unsigned lbase = inst * mport.size();
			PEIdent*pport = mport[ldx];
			ivl_assert(*this, pport);
			NetNet *netnet = pport->elaborate_subport(des, inst_scope);
			prts[lbase + ldx] = netnet;
			if (netnet == 0)
			      continue;

			ivl_assert(*this, netnet);
			unsigned port_width = netnet->vector_width() * netnet->pin_count();
			prts_vector_width += port_width;
			prt_vector_width += port_width;
			ptype = PortType::merged(netnet->port_type(), ptype);
		  }
		  inst_scope->add_module_port_info(idx, port_name, ptype, prt_vector_width );
	    }

	      // If I find that the port is unconnected inside the
	      // module, then there is nothing to connect. Skip the
	      // argument.
	    if ((prts_vector_width == 0) || unconnected_port) {
		  continue;
	    }

	      // We know by design that each instance has the same
	      // width port. Therefore, the prts_pin_count must be an
	      // even multiple of the instance count.
	    assert(prts_vector_width % instance.size() == 0);

	    if (!prts.empty() && (prts[0]->port_type() == NetNet::PINPUT)
	        && prts[0]->pin(0).nexus()->drivers_present()
	        && pins[idx]->is_collapsible_net(des, scope,
	                                         prts[0]->port_type())) {
                  prts[0]->port_type(NetNet::PINOUT);

		  cerr << pins[idx]->get_fileline() << ": warning: input port "
		       << prts[0]->name() << " is coerced to inout." << endl;
	    }

	    if (!prts.empty() && (prts[0]->port_type() == NetNet::POUTPUT)
	        && (prts[0]->type() != NetNet::REG)
	        && prts[0]->pin(0).nexus()->has_floating_input()
	        && pins[idx]->is_collapsible_net(des, scope,
	                                         prts[0]->port_type())) {
                  prts[0]->port_type(NetNet::PINOUT);

		  cerr << pins[idx]->get_fileline() << ": warning: output port "
		       << prts[0]->name() << " is coerced to inout." << endl;
	    }

	      // Elaborate the expression that connects to the
	      // module[s] port. sig is the thing outside the module
	      // that connects to the port.

	    NetNet*sig = 0;
	    NetNet::PortType ptype;
	    if (prts.empty())
		   ptype = NetNet::NOT_A_PORT;
	    else
		   ptype = prts[0]->port_type();
	    if (prts.empty() || (ptype == NetNet::PINPUT)) {

		    // Special case: If the input port is an unpacked
		    // array, then there should be no sub-ports and
		    // the r-value expression is processed
		    // differently.
		  if (prts.size() >= 1 && prts[0]->pin_count()>1) {
			ivl_assert(*this, prts.size()==1);

			PEIdent*rval_pident = dynamic_cast<PEIdent*> (pins[idx]);
			ivl_assert(*this, rval_pident);

			NetNet*rval_net = rval_pident->elaborate_unpacked_net(des, scope);
			ivl_assert(*this, rval_net->pin_count() == prts[0]->pin_count());
			assign_unpacked_with_bufz(des, scope, this, prts[0], rval_net);
			continue;
		  }

		    /* Input to module. elaborate the expression to
		       the desired width. If this in an instance
		       array, then let the net determine its own
		       width. We use that, then, to decide how to hook
		       it up.

		       NOTE that this also handles the case that the
		       port is actually empty on the inside. We assume
		       in that case that the port is input. */

		  NetExpr*tmp_expr = elab_and_eval(des, scope, pins[idx], -1, using_default);
		  if (tmp_expr == 0) {
			cerr << pins[idx]->get_fileline()
			     << ": error: Failed to elaborate port "
			     << (using_default ? "default value." : "expression.")
			     << endl;
			des->errors += 1;
			continue;
		  }

		  if (debug_elaborate) {
			cerr << get_fileline() << ": debug: "
			     << "Elaborating INPUT port expression: " << *tmp_expr << endl;
		  }

		  sig = tmp_expr->synthesize(des, scope, tmp_expr);
		  if (sig == 0) {
			cerr << pins[idx]->get_fileline()
			     << ": internal error: Port expression "
			     << "too complicated for elaboration." << endl;
			continue;
		  }

		  delete tmp_expr;
		  if (!sig->get_lineno()) sig->set_line(*this);

		  if (need_bufz_for_input_port(prts)) {
			NetBUFZ*tmp = new NetBUFZ(scope, scope->local_symbol(),
						  sig->vector_width(), true);
			tmp->set_line(*this);
			des->add_node(tmp);
			connect(tmp->pin(1), sig->pin(0));

			netvector_t*tmp2_vec = new netvector_t(sig->data_type(),
							       sig->vector_width()-1,0);
			NetNet*tmp2 = new NetNet(scope, scope->local_symbol(),
						 NetNet::WIRE, tmp2_vec);
			tmp2->local_flag(true);
			tmp2->set_line(*this);
			connect(tmp->pin(0), tmp2->pin(0));
			sig = tmp2;
		  }

		    // If we have a real signal driving a bit/vector port
		    // then we convert the real value using the appropriate
		    // width cast. Since a real is only one bit the whole
		    // thing needs to go to each instance when arrayed.
		  if ((sig->data_type() == IVL_VT_REAL ) &&
		      !prts.empty() && (prts[0]->data_type() != IVL_VT_REAL )) {
			sig = cast_to_int4(des, scope, sig,
			                  prts_vector_width/instance.size());
		  }
		    // If we have a bit/vector signal driving a real port
		    // then we convert the value to a real.
		  if ((sig->data_type() != IVL_VT_REAL ) &&
		      !prts.empty() && (prts[0]->data_type() == IVL_VT_REAL )) {
			sig = cast_to_real(des, scope, sig);
		  }
		    // If we have a 4-state bit/vector signal driving a
		    // 2-state port then we convert the value to 2-state.
		  if ((sig->data_type() == IVL_VT_LOGIC ) &&
		      !prts.empty() && (prts[0]->data_type() == IVL_VT_BOOL )) {
			sig = cast_to_int2(des, scope, sig,
					   sig->vector_width());
		  }

	    } else if (ptype == NetNet::PINOUT) {

		    // For now, do not support unpacked array outputs.
		  ivl_assert(*this, prts[0]->unpacked_dimensions()==0);

		    /* Inout to/from module. This is a more
		       complicated case, where the expression must be
		       an lnet, but also an r-value net.

		       Normally, this winds up being the same as if we
		       just elaborated as an lnet, as passing a simple
		       identifier elaborates to the same NetNet in
		       both cases so the extra elaboration has no
		       effect. But if the expression passed to the
		       inout port is a part select, a special part
		       select must be created that can pass data in
		       both directions.

		       Use the elaborate_bi_net method to handle all
		       the possible cases. */

		  sig = pins[idx]->elaborate_bi_net(des, scope);
		  if (sig == 0) {
			cerr << pins[idx]->get_fileline() << ": error: "
			     << "Inout port expression must support "
			     << "continuous assignment." << endl;
			cerr << pins[idx]->get_fileline() << ":      : Port "
			     << (idx+1) << " (" << port_name << ") of "
			     << rmod->mod_name() << " is connected to "
			     << *pins[idx] << endl;
			des->errors += 1;
			continue;
		  }

		    // We do not support automatic bits to real conversion
		    // for inout ports.
		  if ((sig->data_type() == IVL_VT_REAL ) &&
		      !prts.empty() && (prts[0]->data_type() != IVL_VT_REAL )) {
			cerr << pins[idx]->get_fileline() << ": error: "
			     << "Cannot automatically connect bit based "
			        "inout port " << (idx+1) << " (" << port_name
			     << ") of module " << rmod->mod_name()
			     << " to real signal " << sig->name() << "." << endl;
			des->errors += 1;
			continue;
		  }

		    // We do not support real inout ports at all.
		  if (!prts.empty() && (prts[0]->data_type() == IVL_VT_REAL )) {
			cerr << pins[idx]->get_fileline() << ": error: "
			     << "No support for connecting real inout ports ("
			        "port " << (idx+1) << " (" << port_name
			     << ") of module " << rmod->mod_name() << ")." << endl;
			des->errors += 1;
			continue;
		  }


	    } else {

		    /* Port type must be OUTPUT here. */
		  ivl_assert(*this, ptype == NetNet::POUTPUT);

		    // Special case: If the output port is an unpacked
		    // array, then there should be no sub-ports and
		    // the passed port expression is processed
		    // differently. Note that we are calling it the
		    // "r-value" expression, but since this is an
		    // output port, we assign to it from the internal object.
		  if (prts[0]->pin_count() > 1) {
			ivl_assert(*this, prts.size()==1);

			PEIdent*rval_pident = dynamic_cast<PEIdent*>(pins[idx]);
			ivl_assert(*this, rval_pident);

			NetNet*rval_net = rval_pident->elaborate_unpacked_net(des, scope);
			ivl_assert(*this, rval_net->pin_count() == prts[0]->pin_count());

			assign_unpacked_with_bufz(des, scope, this, rval_net, prts[0]);
			continue;
		  }

		    // At this point, arrays are handled.
		  ivl_assert(*this, prts[0]->unpacked_dimensions()==0);

		    /* Output from module. Elaborate the port
		       expression as the l-value of a continuous
		       assignment, as the port will continuous assign
		       into the port. */

		  sig = pins[idx]->elaborate_lnet(des, scope);
		  if (sig == 0) {
			cerr << pins[idx]->get_fileline() << ": error: "
			     << "Output port expression must support "
			     << "continuous assignment." << endl;
			cerr << pins[idx]->get_fileline() << ":      : Port "
			     << (idx+1) << " (" << port_name << ") of "
			     << rmod->mod_name() << " is connected to "
			     << *pins[idx] << endl;
			des->errors += 1;
			continue;
		  }

		    // If we have a real port driving a bit/vector signal
		    // then we convert the real value using the appropriate
		    // width cast. Since a real is only one bit the whole
		    // thing needs to go to each instance when arrayed.
		  if ((sig->data_type() != IVL_VT_REAL ) &&
		      !prts.empty() && (prts[0]->data_type() == IVL_VT_REAL )) {
			if (sig->vector_width() % instance.size() != 0) {
			      cerr << pins[idx]->get_fileline() << ": error: "
			              "When automatically converting a real "
			              "port of an arrayed instance to a bit "
			              "signal" << endl;
			      cerr << pins[idx]->get_fileline() << ":      : "
			              "the signal width ("
			           << sig->vector_width() << ") must be an "
			              "integer multiple of the instance count ("
			           << instance.size() << ")." << endl;
			      des->errors += 1;
			      continue;
			}
			prts_vector_width = sig->vector_width();
			for (unsigned pidx = 0; pidx < prts.size(); pidx += 1) {
			      prts[pidx] = cast_to_int4(des, scope, prts[pidx],
			                               prts_vector_width /
			                               instance.size());
			}
		  }

		    // If we have a bit/vector port driving a single real
		    // signal then we convert the value to a real.
		  if ((sig->data_type() == IVL_VT_REAL ) &&
		      !prts.empty() && (prts[0]->data_type() != IVL_VT_REAL )) {
			prts_vector_width -= prts[0]->vector_width() - 1;
			prts[0] = cast_to_real(des, scope, prts[0]);
			  // No support for multiple real drivers.
			if (instance.size() != 1) {
			      cerr << pins[idx]->get_fileline() << ": error: "
			           << "Cannot connect an arrayed instance of "
			              "module " << rmod->mod_name() << " to "
			              "real signal " << sig->name() << "."
			           << endl;
			      des->errors += 1;
			      continue;
			}
		  }

		    // If we have a 4-state bit/vector port driving a
		    // 2-state signal then we convert the value to 2-state.
		  if ((sig->data_type() == IVL_VT_BOOL ) &&
		      !prts.empty() && (prts[0]->data_type() == IVL_VT_LOGIC )) {
			for (unsigned pidx = 0; pidx < prts.size(); pidx += 1) {
			      prts[pidx] = cast_to_int2(des, scope, prts[pidx],
			                                prts[pidx]->vector_width());
			}
		  }

		    // A real to real connection is not allowed for arrayed
		    // instances. You cannot have multiple real drivers.
		  if ((sig->data_type() == IVL_VT_REAL ) &&
		      !prts.empty() && (prts[0]->data_type() == IVL_VT_REAL ) &&
		      instance.size() != 1) {
			cerr << pins[idx]->get_fileline() << ": error: "
			     << "An arrayed instance of " << rmod->mod_name()
			     << " cannot have a real port (port " << (idx+1)
			     << " : " << port_name << ") connected to a "
			        "real signal (" << sig->name() << ")." << endl;
			des->errors += 1;
			continue;
		  }

	    }

	    assert(sig);

#ifndef NDEBUG
	    if ((! prts.empty())
		&& (ptype != NetNet::PINPUT)) {
		  assert(sig->type() != NetNet::REG);
	    }
#endif

	      /* If we are working with an instance array, then the
		 signal width must match the port width exactly. */
	    if ((instance.size() != 1)
		&& (sig->vector_width() != prts_vector_width)
		&& (sig->vector_width() != prts_vector_width/instance.size())) {
		  cerr << pins[idx]->get_fileline() << ": error: "
		       << "Port expression width " << sig->vector_width()
		       << " does not match expected width "<< prts_vector_width
		       << " or " << (prts_vector_width/instance.size())
		       << "." << endl;
		  des->errors += 1;
		  continue;
	    }

	    if (debug_elaborate) {
		  cerr << get_fileline() << ": debug: " << get_name()
		       << ": Port " << (idx+1) << " (" << port_name
		       << ") has vector width of " << prts_vector_width
		       << "." << endl;
	    }

	      // Check that the parts have matching pin counts. If
	      // not, they are different widths. Note that idx is 0
	      // based, but users count parameter positions from 1.
	    if ((instance.size() == 1)
		&& (prts_vector_width != sig->vector_width())) {
		  bool as_signed = false;

		  switch (ptype) {
		    case NetNet::POUTPUT:
			as_signed = prts[0]->get_signed();
			break;
		    case NetNet::PINPUT:
			as_signed = sig->get_signed();
			break;
		    case NetNet::PINOUT:
			  /* This may not be correct! */
			as_signed = prts[0]->get_signed() && sig->get_signed();
			break;
		    case NetNet::PREF:
			ivl_assert(*this, 0);
			break;
		    default:
			ivl_assert(*this, 0);
		  }

		  cerr << get_fileline() << ": warning: Port " << (idx+1)
		       << " (" << port_name << ") of "
		       << type_ << " expects " << prts_vector_width <<
			" bits, got " << sig->vector_width() << "." << endl;

		    // Delete this when inout ports pad correctly.
		  if (ptype == NetNet::PINOUT) {
		     if (prts_vector_width > sig->vector_width()) {
			cerr << get_fileline() << ":        : Leaving "
			     << (prts_vector_width-sig->vector_width())
			     << " high bits of the port unconnected."
			     << endl;
		     } else {
			cerr << get_fileline() << ":        : Leaving "
			     << (sig->vector_width()-prts_vector_width)
			     << " high bits of the expression dangling."
			     << endl;
		     }
		    // Keep the if, but delete the "} else" when fixed.
		  } else if (prts_vector_width > sig->vector_width()) {
			cerr << get_fileline() << ":        : Padding ";
			if (as_signed) cerr << "(signed) ";
			cerr << (prts_vector_width-sig->vector_width())
			     << " high bits of the port."
			     << endl;
		  } else {
			if (ptype == NetNet::PINPUT) {
			      cerr << get_fileline() << ":        : Pruning ";
			} else {
			      cerr << get_fileline() << ":        : Padding ";
			}
			if (as_signed) cerr << "(signed) ";
			cerr << (sig->vector_width()-prts_vector_width)
			     << " high bits of the expression."
			     << endl;
		  }

		  sig = resize_net_to_port_(des, scope, sig, prts_vector_width,
					    ptype, as_signed);
	    }

	      // Connect the sig expression that is the context of the
	      // module instance to the ports of the elaborated module.

	      // The prts_pin_count variable is the total width of the
	      // port and is the maximum number of connections to
	      // make. sig is the elaborated expression that connects
	      // to that port. If sig has too few pins, then reduce
	      // the number of connections to make.

	      // Connect this many of the port pins. If the expression
	      // is too small, then reduce the number of connects.
	    unsigned ccount = prts_vector_width;
	    if (instance.size() == 1 && sig->vector_width() < ccount)
		  ccount = sig->vector_width();

	      // Now scan the concatenation that makes up the port,
	      // connecting pins until we run out of port pins or sig
	      // pins. The sig object is the NetNet that is connected
	      // to the port from the outside, and the prts object is
	      // an array of signals to be connected to the sig.

	    NetConcat*ctmp;

	    if (prts.size() == 1) {

		    // The simplest case, there are no
		    // parts/concatenations on the inside of the
		    // module, so the port and sig need simply be
		    // connected directly. But don't collapse ports
		    // that are a delay path destination, to avoid
		    // the delay being applied to other drivers of
		    // the external signal.
		  if (prts[0]->delay_paths() > 0) {
			isolate_and_connect(des, scope, this, prts[0], sig, ptype);
		  } else {
			connect(prts[0]->pin(0), sig->pin(0));
		  }

	    } else if (sig->vector_width()==prts_vector_width/instance.size()
		       && prts.size()/instance.size() == 1) {

		  if (debug_elaborate){
			cerr << get_fileline() << ": debug: " << get_name()
			     << ": Replicating " << prts_vector_width
			     << " bits across all "
			     << prts_vector_width/instance.size()
			     << " sub-ports." << endl;
		  }

		    // The signal width is exactly the width of a
		    // single instance of the port. In this case,
		    // connect the sig to all the ports identically.
		  for (unsigned ldx = 0 ;  ldx < prts.size() ;	ldx += 1) {
			if (prts[ldx]->delay_paths() > 0) {
			      isolate_and_connect(des, scope, this, prts[ldx], sig, ptype);
			} else {
			      connect(prts[ldx]->pin(0), sig->pin(0));
			}
		  }

	    } else switch (ptype) {
		case NetNet::POUTPUT:
		  ctmp = new NetConcat(scope, scope->local_symbol(),
				       prts_vector_width, prts.size());
		  ctmp->set_line(*this);
		  des->add_node(ctmp);
		  connect(ctmp->pin(0), sig->pin(0));
		  for (unsigned ldx = 0 ;  ldx < prts.size() ;  ldx += 1) {
			connect(ctmp->pin(ldx+1),
				prts[prts.size()-ldx-1]->pin(0));
		  }
		  break;

		case NetNet::PINPUT:
		  if (debug_elaborate){
			cerr << get_fileline() << ": debug: " << get_name()
			     << ": Dividing " << prts_vector_width
			     << " bits across all "
			     << prts_vector_width/instance.size()
			     << " input sub-ports of port "
			     << (idx+1) << "." << endl;
		  }

		  for (unsigned ldx = 0, spin = 0 ;
		       ldx < prts.size() ;  ldx += 1) {
			NetNet*sp = prts[prts.size()-ldx-1];
			NetPartSelect*ptmp = new NetPartSelect(sig, spin,
							   sp->vector_width(),
							   NetPartSelect::VP);
			ptmp->set_line(*this);
			des->add_node(ptmp);
			connect(ptmp->pin(0), sp->pin(0));
			spin += sp->vector_width();
		  }
		  break;

		case NetNet::PINOUT:
		  for (unsigned ldx = 0, spin = 0 ;
		       ldx < prts.size() ;  ldx += 1) {
			NetNet*sp = prts[prts.size()-ldx-1];
			NetTran*ttmp = new NetTran(scope,
			                           scope->local_symbol(),
			                           sig->vector_width(),
			                           sp->vector_width(),
			                           spin);
			ttmp->set_line(*this);
			des->add_node(ttmp);
			connect(ttmp->pin(0), sig->pin(0));
			connect(ttmp->pin(1), sp->pin(0));
			spin += sp->vector_width();
		  }
		  break;

		case NetNet::PREF:
		  cerr << get_fileline() << ": sorry: "
		       << "Reference ports not supported yet." << endl;
		  des->errors += 1;
		  break;

		case NetNet::PIMPLICIT:
		  cerr << get_fileline() << ": internal error: "
		       << "Unexpected IMPLICIT port" << endl;
		  des->errors += 1;
		  break;
		case NetNet::NOT_A_PORT:
		  cerr << get_fileline() << ": internal error: "
		       << "Unexpected NOT_A_PORT port." << endl;
		  des->errors += 1;
		  break;
	    }

      }

}

unsigned PGModule::calculate_instance_count_(Design*des, NetScope*scope,
                                             long&high, long&low,
                                             perm_string name) const
{
      unsigned count = 1;
      high = 0;
      low = 0;

	/* If the Verilog source has a range specification for the UDP, then
	 * I am expected to make more than one gate. Figure out how many are
	 * desired. */
      if (msb_) {
	    NetExpr*msb_exp = elab_and_eval(des, scope, msb_, -1, true);
	    NetExpr*lsb_exp = elab_and_eval(des, scope, lsb_, -1, true);

	    NetEConst*msb_con = dynamic_cast<NetEConst*>(msb_exp);
	    NetEConst*lsb_con = dynamic_cast<NetEConst*>(lsb_exp);

	    if (msb_con == 0) {
		  cerr << get_fileline() << ": error: Unable to evaluate "
			"expression " << *msb_ << endl;
		  des->errors += 1;
		  return 0;
	    }

	    if (lsb_con == 0) {
		  cerr << get_fileline() << ": error: Unable to evaluate "
			"expression " << *lsb_ << endl;
		  des->errors += 1;
		  return 0;
	    }

	    verinum msb = msb_con->value();
	    verinum lsb = lsb_con->value();

	    delete msb_exp;
	    delete lsb_exp;

	    if (msb.as_long() > lsb.as_long())
		  count = msb.as_long() - lsb.as_long() + 1;
	    else
		  count = lsb.as_long() - msb.as_long() + 1;

	    low = lsb.as_long();
	    high = msb.as_long();

	    if (debug_elaborate) {
		  cerr << get_fileline() << ": debug: PGModule: Make range "
		       << "[" << high << ":" << low << "]" << " of "
		       << count << " UDPs for " << name << endl;
	    }
      }

      return count;
}

/*
 * From a UDP definition in the source, make a NetUDP
 * object. Elaborate the pin expressions as netlists, then connect
 * those networks to the pins.
 */

void PGModule::elaborate_udp_(Design*des, PUdp*udp, NetScope*scope) const
{
      NetExpr*rise_expr =0, *fall_expr =0, *decay_expr =0;

      perm_string my_name = get_name();
      if (my_name == 0)
	    my_name = scope->local_symbol();

	/* When the parser notices delay expressions in front of a
	   module or primitive, it interprets them as parameter
	   overrides. Correct that misconception here. */
      if (overrides_) {
	    if (overrides_->size() > 2) {
		  cerr << get_fileline() << ": error: UDPs take at most two "
		          "delay arguments." << endl;
		  des->errors += 1;
	    } else {
		  PDelays tmp_del;
		  tmp_del.set_delays(overrides_, false);
		  tmp_del.eval_delays(des, scope, rise_expr, fall_expr,
		                      decay_expr, true);
	    }
      }

      long low = 0, high = 0;
      unsigned inst_count = calculate_instance_count_(des, scope, high, low,
                                                      my_name);
      if (inst_count == 0) return;

      if (inst_count != 1) {
	    cerr << get_fileline() << ": sorry: UDPs with a range ("
	         << my_name << " [" << high << ":" << low << "]) are "
	         << "not supported." << endl;
	    des->errors += 1;
	    return;
      }

      assert(udp);
      NetUDP*net = new NetUDP(scope, my_name, udp->ports.count(), udp);
      net->set_line(*this);
      net->rise_time(rise_expr);
      net->fall_time(fall_expr);
      net->decay_time(decay_expr);

      struct attrib_list_t*attrib_list;
      unsigned attrib_list_n = 0;
      attrib_list = evaluate_attributes(attributes, attrib_list_n,
					des, scope);

      for (unsigned adx = 0 ;  adx < attrib_list_n ;  adx += 1)
	    net->attribute(attrib_list[adx].key, attrib_list[adx].val);

      delete[]attrib_list;


	// This is the array of pin expressions, shuffled to match the
	// order of the declaration. If the source instantiation uses
	// bind by order, this is the same as the source
	// list. Otherwise, the source list is rearranged by name
	// binding into this list.
      vector<PExpr*>pins;

	// Detect binding by name. If I am binding by name, then make
	// up a pins array that reflects the positions of the named
	// ports. If this is simply positional binding in the first
	// place, then get the binding from the base class.
      if (pins_) {
	    unsigned nexp = udp->ports.count();
	    pins = vector<PExpr*>(nexp);

	      // Scan the bindings, matching them with port names.
	    for (unsigned idx = 0 ;  idx < npins_ ;  idx += 1) {

		    // Given a binding, look at the module port names
		    // for the position that matches the binding name.
		  unsigned pidx = udp->find_port(pins_[idx].name);

		    // If the port name doesn't exist, the find_port
		    // method will return the port count. Detect that
		    // as an error.
		  if (pidx == nexp) {
			cerr << get_fileline() << ": error: port ``" <<
			      pins_[idx].name << "'' is not a port of "
			     << get_name() << "." << endl;
			des->errors += 1;
			continue;
		  }

		    // If I already bound something to this port, then
		    // the (*exp) array will already have a pointer
		    // value where I want to place this expression.
		  if (pins[pidx]) {
			cerr << get_fileline() << ": error: port ``" <<
			      pins_[idx].name << "'' already bound." <<
			      endl;
			des->errors += 1;
			continue;
		  }

		    // OK, do the binding by placing the expression in
		    // the right place.
		  pins[pidx] = pins_[idx].parm;
	    }

      } else {

	      /* Otherwise, this is a positional list of port
		 connections. In this case, the port count must be
		 right. Check that is is, the get the pin list. */

	    if (pin_count() != udp->ports.count()) {
		  cerr << get_fileline() << ": error: Wrong number "
			"of ports. Expecting " << udp->ports.count() <<
			", got " << pin_count() << "."
		       << endl;
		  des->errors += 1;
		  return;
	    }

	      // No named bindings, just use the positional list I
	      // already have.
	    assert(pin_count() == udp->ports.count());
	    pins = get_pins();
      }


	/* Handle the output port of the primitive special. It is an
	   output port (the only output port) so must be passed an
	   l-value net. */
      if (pins[0] == 0) {
	    cerr << get_fileline() << ": warning: output port unconnected."
		 << endl;

      } else {
	    NetNet*sig = pins[0]->elaborate_lnet(des, scope);
	    if (sig == 0) {
		  cerr << get_fileline() << ": error: "
		       << "Output port expression is not valid." << endl;
		  cerr << get_fileline() << ":      : Output "
		       << "port of " << udp->name_
		       << " is " << udp->ports[0] << "." << endl;
		  des->errors += 1;
		  return;
	    } else {
		  connect(sig->pin(0), net->pin(0));
	    }
	    if (sig->vector_width() != 1) {
		  cerr << get_fileline() << ": error: "
		       << "Output port expression " << *pins[0]
		       << " is too wide (" << sig->vector_width()
		       << ") expected 1." << endl;
		  des->errors += 1;
	    }
      }

	/* Run through the pins, making netlists for the pin
	   expressions and connecting them to the pin in question. All
	   of this is independent of the nature of the UDP. */
      for (unsigned idx = 1 ;  idx < net->pin_count() ;  idx += 1) {
	    if (pins[idx] == 0)
		  continue;

	    NetExpr*expr_tmp = elab_and_eval(des, scope, pins[idx], 1);
	    if (expr_tmp == 0) {
		  cerr << "internal error: Expression too complicated "
			"for elaboration:" << *pins[idx] << endl;
		  continue;
	    }
	    NetNet*sig = expr_tmp->synthesize(des, scope, expr_tmp);
	    ivl_assert(*this, sig);
	    sig->set_line(*this);

	    delete expr_tmp;

	    connect(sig->pin(0), net->pin(idx));
	    if (sig->vector_width() != 1) {
		  cerr << get_fileline() << ": error: "
		       << "Input port expression " << *pins[idx]
		       << " is too wide (" << sig->vector_width()
		       << ") expected 1." << endl;
		  des->errors += 1;
	    }
      }

	// All done. Add the object to the design.
      des->add_node(net);
}


bool PGModule::elaborate_sig(Design*des, NetScope*scope) const
{
      if (bound_type_) {
	    return elaborate_sig_mod_(des, scope, bound_type_);
      }

	// Look for the module type
      map<perm_string,Module*>::const_iterator mod = pform_modules.find(type_);
      if (mod != pform_modules.end())
	    return elaborate_sig_mod_(des, scope, (*mod).second);

	// elaborate_sig_udp_ currently always returns true so skip all this
	// for now.
#if 0
      map<perm_string,PUdp*>::const_iterator udp = pform_primitives.find(type_);
      if (udp != pform_primitives.end())
	    return elaborate_sig_udp_(des, scope, (*udp).second);
#endif

      return true;
}


void PGModule::elaborate(Design*des, NetScope*scope) const
{
      if (bound_type_) {
	    elaborate_mod_(des, bound_type_, scope);
	    return;
      }

	// Look for the module type
      map<perm_string,Module*>::const_iterator mod = pform_modules.find(type_);
      if (mod != pform_modules.end()) {
	    elaborate_mod_(des, (*mod).second, scope);
	    return;
      }

	// Try a primitive type
      map<perm_string,PUdp*>::const_iterator udp = pform_primitives.find(type_);
      if (udp != pform_primitives.end()) {
	    assert((*udp).second);
	    elaborate_udp_(des, (*udp).second, scope);
	    return;
      }

      if (!ignore_missing_modules) {
        cerr << get_fileline() << ": internal error: Unknown module type: " <<
	      type_ << endl;
      }
}

void PGModule::elaborate_scope(Design*des, NetScope*sc) const
{
	// If the module type is known by design, then go right to it.
      if (bound_type_) {
	    elaborate_scope_mod_(des, bound_type_, sc);
	    return;
      }

	// Look for the module type
      map<perm_string,Module*>::const_iterator mod = pform_modules.find(type_);
      if (mod != pform_modules.end()) {
	    elaborate_scope_mod_(des, mod->second, sc);
	    return;
      }

	// Try a primitive type
      map<perm_string,PUdp*>::const_iterator udp = pform_primitives.find(type_);
      if (udp != pform_primitives.end())
	    return;

	// Not a module or primitive that I know about yet, so try to
	// load a library module file (which parses some new Verilog
	// code) and try again.
      int parser_errors = 0;
      if (load_module(type_, parser_errors)) {

	      // Try again to find the module type
	    mod = pform_modules.find(type_);
	    if (mod != pform_modules.end()) {
		  elaborate_scope_mod_(des, mod->second, sc);
		  return;
	    }

	      // Try again to find a primitive type
	    udp = pform_primitives.find(type_);
	    if (udp != pform_primitives.end())
		  return;
      }

      if (parser_errors) {
            cerr << get_fileline() << ": error: Failed to parse library file." << endl;
            des->errors += parser_errors + 1;
      }

	// Not a module or primitive that I know about or can find by
	// any means, so give up.
      if (!ignore_missing_modules) {
        cerr << get_fileline() << ": error: Unknown module type: " << type_ << endl;
        missing_modules[type_] += 1;
        des->errors += 1;
      }
}


NetProc* Statement::elaborate(Design*des, NetScope*) const
{
      cerr << get_fileline() << ": internal error: elaborate: "
	    "What kind of statement? " << typeid(*this).name() << endl;
      NetProc*cur = new NetProc;
      des->errors += 1;
      return cur;
}


NetAssign_* PAssign_::elaborate_lval(Design*des, NetScope*scope) const
{
	// A function called as a task does not have an L-value.
      if (! lval_) {
	      // The R-value must be a simple function call.
	    assert (dynamic_cast<PECallFunction*>(rval_));
	    PExpr::width_mode_t mode = PExpr::SIZED;
	    rval_->test_width(des, scope, mode);
	      // Create a L-value that matches the function return type.
	    NetNet*tmp;
	    netvector_t*tmp_vec = new netvector_t(rval_->expr_type(),
	                                          rval_->expr_width()-1, 0,
	                                          rval_->has_sign());

	    if(rval_->expr_type() == IVL_VT_DARRAY) {
		netdarray_t*darray = new netdarray_t(tmp_vec);
		tmp = new NetNet(scope, scope->local_symbol(), NetNet::REG, darray);
	    } else {
		tmp = new NetNet(scope, scope->local_symbol(), NetNet::REG, tmp_vec);
	    }

	    tmp->set_file(rval_->get_file());
	    tmp->set_lineno(rval_->get_lineno());
	    NetAssign_*lv = new NetAssign_(tmp);
	    return lv;
      }

      if (debug_elaborate) {
	    cerr << get_fileline() << ": PAssign_::elaborate_lval: "
		 << "lval_ = " << *lval_ << endl;
	    cerr << get_fileline() << ": PAssign_::elaborate_lval: "
		 << "lval_ expr type = " << typeid(*lval_).name() << endl;
      }

      return lval_->elaborate_lval(des, scope, false, false);
}

NetExpr* PAssign_::elaborate_rval_(Design*des, NetScope*scope,
				   ivl_type_t net_type) const
{
      ivl_assert(*this, rval_);

      NetExpr*rv = elab_and_eval(des, scope, rval_, net_type, is_constant_);

      if (!is_constant_ || !rv) return rv;

      cerr << get_fileline() << ": error: "
            "The RHS expression must be constant." << endl;
      cerr << get_fileline() << "       : "
            "This expression violates the rule: " << *rv << endl;
      des->errors += 1;
      delete rv;
      return 0;
}

NetExpr* PAssign_::elaborate_rval_(Design*des, NetScope*scope,
				   ivl_type_t lv_net_type,
				   ivl_variable_type_t lv_type,
				   unsigned lv_width,
				   bool force_unsigned) const
{
      ivl_assert(*this, rval_);

	// Don't have a good value for the lv_net_type argument to
	// elaborate_rval_expr, so punt and pass nil. In the future we
	// should look into fixing calls to this method to pass a
	// net_type instead of the separate lv_width/lv_type values.
      NetExpr*rv = elaborate_rval_expr(des, scope, lv_net_type, lv_type, lv_width,
				       rval(), is_constant_, force_unsigned);

      if (!is_constant_ || !rv) return rv;

      if (dynamic_cast<NetEConst*>(rv)) return rv;
      if (dynamic_cast<NetECReal*>(rv)) return rv;

      cerr << get_fileline() << ": error: "
            "The RHS expression must be constant." << endl;
      cerr << get_fileline() << "       : "
            "This expression violates the rule: " << *rv << endl;
      des->errors += 1;
      delete rv;
      return 0;
}

/*
 * This function elaborates delay expressions. This is a little
 * different from normal elaboration because the result may need to be
 * scaled.
 */
static NetExpr*elaborate_delay_expr(PExpr*expr, Design*des, NetScope*scope)
{
      NetExpr*dex = elab_and_eval(des, scope, expr, -1);

	// If the elab_and_eval returns nil, then the function
	// failed. It should already have printed an error message,
	// but we can add some detail. Lets add the error count, just
	// in case.
      if (dex == 0) {
	    cerr << expr->get_fileline() << ": error: "
		 << "Unable to elaborate (or evaluate) delay expression."
		 << endl;
	    des->errors += 1;
	    return 0;
      }

      check_for_inconsistent_delays(scope);

	/* If the delay expression is a real constant or vector
	   constant, then evaluate it, scale it to the local time
	   units, and return an adjusted NetEConst. */

      if (NetECReal*tmp = dynamic_cast<NetECReal*>(dex)) {
	    uint64_t delay = get_scaled_time_from_real(des, scope, tmp);

	    delete tmp;
	    NetEConst*tmp2 = new NetEConst(verinum(delay, 64));
	    tmp2->set_line(*expr);
	    return tmp2;
      }


      if (NetEConst*tmp = dynamic_cast<NetEConst*>(dex)) {
	    verinum fn = tmp->value();
	    uint64_t delay = des->scale_to_precision(fn.as_ulong64(), scope);

	    delete tmp;
	    NetEConst*tmp2 = new NetEConst(verinum(delay, 64));
	    tmp2->set_line(*expr);
	    return tmp2;
      }


	/* The expression is not constant, so generate an expanded
	   expression that includes the necessary scale shifts, and
	   return that expression. */
      ivl_assert(*expr, dex);
      if (dex->expr_type() == IVL_VT_REAL) {
	      // Scale the real value.
	    int shift = scope->time_unit() - scope->time_precision();
	    assert(shift >= 0);
	    double round = 1;
	    for (int lp = 0; lp < shift; lp += 1) round *= 10.0;

	    NetExpr*scal_val = new NetECReal(verireal(round));
	    scal_val->set_line(*expr);
	    dex = new NetEBMult('*', dex, scal_val, 1, true);
	    dex->set_line(*expr);

	      // Cast this part of the expression to an integer.
	    dex = new NetECast('v', dex, 64, false);
	    dex->set_line(*expr);

	      // Now scale the integer value.
	    shift = scope->time_precision() - des->get_precision();
	    assert(shift >= 0);
	    uint64_t scale = 1;
	    for (int lp = 0; lp < shift; lp += 1) scale *= 10;

	    scal_val = new NetEConst(verinum(scale, 64));
	    scal_val->set_line(*expr);
	    dex = new NetEBMult('*', dex, scal_val, 64, false);
	    dex->set_line(*expr);
      } else {
	    int shift = scope->time_unit() - des->get_precision();
	    assert(shift >= 0);
	    uint64_t scale = 1;
	    for (int lp = 0; lp < shift; lp += 1) scale *= 10;

	    NetExpr*scal_val = new NetEConst(verinum(scale, 64));
	    scal_val->set_line(*expr);
	    dex = new NetEBMult('*', dex, scal_val, 64, false);
	    dex->set_line(*expr);
      }

      return dex;
}

NetProc* PAssign::elaborate_compressed_(Design*des, NetScope*scope) const
{
      ivl_assert(*this, ! delay_);
      ivl_assert(*this, ! count_);
      ivl_assert(*this, ! event_);

      NetAssign_*lv = elaborate_lval(des, scope);
      if (lv == 0) return 0;

	// Compressed assignments should behave identically to the
	// equivalent uncompressed assignments. This means we need
	// to take the type of the LHS into account when determining
	// the type of the RHS expression.
      bool force_unsigned;
      switch (op_) {
	  case 'l':
	  case 'r':
	  case 'R':
	      // The right-hand operand of shift operations is
	      // self-determined.
	    force_unsigned = false;
	    break;
	  default:
	    force_unsigned = !lv->get_signed();
	    break;
      }
      NetExpr*rv = elaborate_rval_(des, scope, 0, lv->expr_type(),
				   count_lval_width(lv), force_unsigned);
      if (rv == 0) return 0;

	// The ivl_target API doesn't support signalling the type
	// of a lval, so convert arithmetic shifts into logical
	// shifts now if the lval is unsigned.
      char op = op_;
      if ((op == 'R') && !lv->get_signed())
	    op = 'r';

      NetAssign*cur = new NetAssign(lv, op, rv);
      cur->set_line(*this);

      return cur;
}

/*
 * Assignments within program blocks are supposed to be run
 * in the Reactive region, but that is currently not supported
 * so find out if we are assigning to something outside a
 * program block and print a warning for that.
 */
static bool lval_not_program_variable(const NetAssign_*lv)
{
      while (lv) {
	    NetScope*sig_scope = lv->scope();
	    if (! sig_scope->program_block()) return true;
	    lv = lv->more;
      }
      return false;
}

NetProc* PAssign::elaborate(Design*des, NetScope*scope) const
{
      assert(scope);

	/* If this is a compressed assignment, then handle the
	   elaboration in a specialized function. */
      if (op_ != 0)
	    return elaborate_compressed_(des, scope);

	/* elaborate the lval. This detects any part selects and mux
	   expressions that might exist. */
      NetAssign_*lv = elaborate_lval(des, scope);
      if (lv == 0) return 0;

      if (scope->program_block() && lval_not_program_variable(lv)) {
	    cerr << get_fileline() << ": warning: Program blocking "
	            "assignments are not currently scheduled in the "
	            "Reactive region."
	         << endl;
      }

	/* If there is an internal delay expression, elaborate it. */
      NetExpr*delay = 0;
      if (delay_ != 0)
	    delay = elaborate_delay_expr(delay_, des, scope);

      NetExpr*rv;
      const ivl_type_s*lv_net_type = lv->net_type();

      if (debug_elaborate) {
	    cerr << get_fileline() << ": PAssign::elaborate: ";
	    if (lv_net_type)
		  cerr << "lv_net_type=" << *lv_net_type << endl;
	    else
		  cerr << "lv_net_type=<nil>" << endl;
      }

	/* If the l-value is a compound type of some sort, then use
	   the newer net_type form of the elaborate_rval_ method to
	   handle the new types. */
      if (dynamic_cast<const netclass_t*> (lv_net_type)) {
	    ivl_assert(*this, lv->more==0);
	    rv = elaborate_rval_(des, scope, lv_net_type);

      } else if (const netdarray_t*dtype = dynamic_cast<const netdarray_t*> (lv_net_type)) {
	    ivl_assert(*this, lv->more==0);
	    if (debug_elaborate) {
		  if (lv->word())
			cerr << get_fileline() << ": PAssign::elaborate: "
			     << "lv->word() = " << *lv->word() << endl;
		  else
			cerr << get_fileline() << ": PAssign::elaborate: "
			     << "lv->word() = <nil>" << endl;
	    }
	    ivl_type_t use_lv_type = lv_net_type;
	    if (lv->word())
		  use_lv_type = dtype->element_type();

	    rv = elaborate_rval_(des, scope, use_lv_type);

      } else if (const netuarray_t*utype = dynamic_cast<const netuarray_t*>(lv_net_type)) {
	    ivl_assert(*this, lv->more==0);
	    if (debug_elaborate) {
		  if (lv->word())
			cerr << get_fileline() << ": PAssign::elaborate: "
			     << "lv->word() = " << *lv->word() << endl;
		  else
			cerr << get_fileline() << ": PAssign::elaborate: "
			     << "lv->word() = <nil>" << endl;
	    }
	    ivl_assert(*this, lv->word());
	    ivl_type_t use_lv_type = utype->element_type();

	    ivl_assert(*this, use_lv_type);
	    rv = elaborate_rval_(des, scope, use_lv_type);

      } else {
	      /* Elaborate the r-value expression, then try to evaluate it. */
	    rv = elaborate_rval_(des, scope, lv_net_type, lv->expr_type(), count_lval_width(lv));
      }

      if (rv == 0) {
	    delete lv;
	    return 0;
      }
      assert(rv);

      if (count_) assert(event_);

	/* Rewrite delayed assignments as assignments that are
	   delayed. For example, a = #<d> b; becomes:

	     begin
	        tmp = b;
		#<d> a = tmp;
	     end

	   If the delay is an event delay, then the transform is
	   similar, with the event delay replacing the time delay. It
	   is an event delay if the event_ member has a value.

	   This rewriting of the expression allows me to not bother to
	   actually and literally represent the delayed assign in the
	   netlist. The compound statement is exactly equivalent. */

      if (delay || event_) {
	    unsigned wid = count_lval_width(lv);

	    netvector_t*tmp2_vec = new netvector_t(rv->expr_type(),wid-1,0);
	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    NetNet::REG, tmp2_vec);
	    tmp->local_flag(true);
	    tmp->set_line(*this);

	    NetESignal*sig = new NetESignal(tmp);

	      /* Generate an assignment of the l-value to the temporary... */
	    NetAssign_*lvt = new NetAssign_(tmp);

	    NetAssign*a1 = new NetAssign(lvt, rv);
	    a1->set_line(*this);

	      /* Generate an assignment of the temporary to the r-value... */
	    NetAssign*a2 = new NetAssign(lv, sig);
	    a2->set_line(*this);

	      /* Generate the delay statement with the final
		 assignment attached to it. If this is an event delay,
		 elaborate the PEventStatement. Otherwise, create the
		 right NetPDelay object. For a repeat event control
		 repeat the event and then do the final assignment.  */
	    NetProc*st;
	    if (event_) {
		  if (count_) {
			NetExpr*count = elab_and_eval(des, scope, count_, -1);
			if (count == 0) {
			      cerr << get_fileline() << ": Unable to "
			              "elaborate repeat expression." << endl;
			      des->errors += 1;
			      return 0;
			}
			st = event_->elaborate(des, scope);
			if (st == 0) {
			      cerr << event_->get_fileline() << ": error: "
			              "unable to elaborate event expression."
			      << endl;
			      des->errors += 1;
			      return 0;
			}
			st->set_line(*this);

			  // If the expression is a constant, handle
			  // certain special iteration counts.
			if (NetEConst*ce = dynamic_cast<NetEConst*>(count)) {
			      long val = ce->value().as_long();
				// We only need the real statement.
			      if (val <= 0) {
				    delete count;
				    delete st;
				    st = 0;

				// We don't need the repeat statement.
			      } else if (val == 1) {
				    delete count;

				// We need a repeat statement.
			      } else {
				    st = new NetRepeat(count, st);
				    st->set_line(*this);
			      }
			} else {
			      st = new NetRepeat(count, st);
			      st->set_line(*this);
			}
		  } else {
			st = event_->elaborate_st(des, scope, a2);
			if (st == 0) {
			      cerr << event_->get_fileline() << ": error: "
			              "unable to elaborate event expression."
			      << endl;
			      des->errors += 1;
			      return 0;
			}
			st->set_line(*this);
		  }
	    } else {
		  NetPDelay*de = new NetPDelay(delay, a2);
		  de->set_line(*this);
		  st = de;
	    }

	      /* And build up the complex statement. */
	    NetBlock*bl = new NetBlock(NetBlock::SEQU, 0);
	    bl->append(a1);
	    if (st) bl->append(st);
	    if (count_) bl->append(a2);
	    bl->set_line(*this);

	    return bl;
      }

      if (lv->enumeration()) {
	    if (! rv->enumeration()) {
		  cerr << get_fileline() << ": error: "
		          "This assignment requires an explicit cast." << endl;
		  des->errors += 1;
	    } else if (! lv->enumeration()->matches(rv->enumeration())) {
		  cerr << get_fileline() << ": error: "
		          "Enumeration type mismatch in assignment." << endl;
		  des->errors += 1;
	    }
      }

      NetAssign*cur = new NetAssign(lv, rv);
      cur->set_line(*this);

      return cur;
}

/*
 * Elaborate non-blocking assignments. The statement is of the general
 * form:
 *
 *    <lval> <= #<delay> <rval> ;
 */
NetProc* PAssignNB::elaborate(Design*des, NetScope*scope) const
{
      assert(scope);

      if (scope->in_func()) {
	    cerr << get_fileline() << ": error: functions cannot have non "
	            "blocking assignment statements." << endl;
	    des->errors += 1;
	    return 0;
      }

      if (scope->in_final()) {
	    cerr << get_fileline() << ": error: final procedures cannot have "
		    "non blocking assignment statements." << endl;
	    des->errors += 1;
	    return 0;
      }

      if (scope->is_auto() && lval()->has_aa_term(des, scope)) {
	    cerr << get_fileline() << ": error: automatically allocated "
                    "variables may not be assigned values using non-blocking "
	            "assignments." << endl;
	    des->errors += 1;
	    return 0;
      }

	/* Elaborate the l-value. */
      NetAssign_*lv = elaborate_lval(des, scope);
      if (lv == 0) return 0;

      if (scope->program_block() && lval_not_program_variable(lv)) {
	    cerr << get_fileline() << ": warning: Program non-blocking "
	            "assignments are not currently scheduled in the "
	            "Reactive-NBA region."
	         << endl;
      }

      NetExpr*rv = elaborate_rval_(des, scope, 0, lv->expr_type(), count_lval_width(lv));
      if (rv == 0) return 0;

      NetExpr*delay = 0;
      if (delay_ != 0) {
	    assert(count_ == 0 && event_ == 0);
	    delay = elaborate_delay_expr(delay_, des, scope);
      }

      NetExpr*count = 0;
      NetEvWait*event = 0;
      if (count_ != 0 || event_ != 0) {
	    if (count_ != 0) {
                  if (scope->is_auto() && count_->has_aa_term(des, scope)) {
                        cerr << get_fileline() << ": error: automatically "
                                "allocated variables may not be referenced "
                                "in intra-assignment event controls of "
                                "non-blocking assignments." << endl;
                        des->errors += 1;
                        return 0;
                  }

		  assert(event_ != 0);
		  count = elab_and_eval(des, scope, count_, -1);
		  if (count == 0) {
			cerr << get_fileline() << ": Unable to elaborate "
			        "repeat expression." << endl;
			des->errors += 1;
			return 0;
		  }
	    }

            if (scope->is_auto() && event_->has_aa_term(des, scope)) {
                  cerr << get_fileline() << ": error: automatically "
                          "allocated variables may not be referenced "
                          "in intra-assignment event controls of "
                          "non-blocking assignments." << endl;
                  des->errors += 1;
                  return 0;
            }

	    NetProc*st = event_->elaborate(des, scope);
	    if (st == 0) {
		  cerr << get_fileline() << ": unable to elaborate "
		          "event expression." << endl;
		  des->errors += 1;
		  return 0;
	    }
	    event = dynamic_cast<NetEvWait*>(st) ;
	    assert(event);

	      // Some constant values are special.
	    if (NetEConst*ce = dynamic_cast<NetEConst*>(count)) {
		  long val = ce->value().as_long();
		    // We only need the assignment statement.
		  if (val <= 0) {
			delete count;
			delete event;
			count = 0;
			event = 0;
		    // We only need the event.
		  } else if (val == 1) {
			delete count;
			count = 0;
		  }
	    }
      }

	/* All done with this node. Mark its line number and check it in. */
      NetAssignNB*cur = new NetAssignNB(lv, rv, event, count);
      cur->set_delay(delay);
      cur->set_line(*this);
      return cur;
}


/*
 * This is the elaboration method for a begin-end block. Try to
 * elaborate the entire block, even if it fails somewhere. This way I
 * get all the error messages out of it. Then, if I detected a failure
 * then pass the failure up.
 */
NetProc* PBlock::elaborate(Design*des, NetScope*scope) const
{
      assert(scope);

      NetBlock::Type type;
      switch (bl_type_) {
	  case PBlock::BL_SEQ:
	    type = NetBlock::SEQU;
	    break;
	  case PBlock::BL_PAR:
	    type = NetBlock::PARA;
	    break;
	  case PBlock::BL_JOIN_NONE:
	    type = NetBlock::PARA_JOIN_NONE;
	    break;
	  case PBlock::BL_JOIN_ANY:
	    type = NetBlock::PARA_JOIN_ANY;
	    break;
	    // Added to remove a "type" uninitialized compiler warning.
	    // This should never be reached since all the PBlock enumeration
	    // cases are handled above.
	  default:
	    type = NetBlock::SEQU;
	    assert(0);
      }

      NetScope*nscope = 0;
      if (pscope_name() != 0) {
	    nscope = scope->child(hname_t(pscope_name()));
	    if (nscope == 0) {
		  cerr << get_fileline() << ": internal error: "
			"unable to find block scope " << scope_path(scope)
		       << "." << pscope_name() << endl;
		  des->errors += 1;
		  return 0;
	    }
	    assert(nscope);
      }

      NetBlock*cur = new NetBlock(type, nscope);

      if (nscope) {
	      // Handle any variable initialization statements in this scope.
	      // For automatic scopes these statements need to be executed
	      // each time the block is entered, so add them to the main
	      // block. For static scopes, put them in a separate process
	      // that will be executed at the start of simulation.
	    if (nscope->is_auto()) {
		  for (unsigned idx = 0; idx < var_inits.size(); idx += 1) {
			NetProc*tmp = var_inits[idx]->elaborate(des, nscope);
			if (tmp) cur->append(tmp);
		  }
	    } else {
		  elaborate_var_inits_(des, nscope);
	    }
      }

      if (nscope == 0)
	    nscope = scope;

	// Handle the special case that the sequential block contains
	// only one statement. There is no need to keep the block node.
	// Also, don't elide named blocks, because they might be
	// referenced elsewhere.
      if ((type == NetBlock::SEQU) && (list_.size() == 1) &&
          (pscope_name() == 0)) {
	    assert(list_[0]);
	    NetProc*tmp = list_[0]->elaborate(des, nscope);
	    return tmp;
      }

      for (unsigned idx = 0 ;  idx < list_.size() ;  idx += 1) {
	    assert(list_[idx]);

	      // Detect the error that a super.new() statement is in the
	      // midst of a block. Report the error. Continue on with the
	      // elaboration so that other errors might be found.
	    if (PChainConstructor*supernew = dynamic_cast<PChainConstructor*> (list_[idx])) {
	          if (debug_elaborate) {
		        cerr << get_fileline() << ": PBlock::elaborate: "
			     << "Found super.new statement, idx=" << idx << ", "
			     << " at " << supernew->get_fileline() << "."
			     << endl;
		  }
		  if (idx > 0) {
		        des->errors += 1;
			cerr << supernew->get_fileline() << ": error: "
			     << "super.new(...) must be the first statement in a block."
			     << endl;
		  }
	    }

	    NetProc*tmp = list_[idx]->elaborate(des, nscope);
	      // If the statement fails to elaborate, then simply
	      // ignore it. Presumably, the elaborate for the
	      // statement already generated an error message and
	      // marked the error count in the design so no need to
	      // do any of that here.
	    if (tmp == 0) {
		  continue;
	    }

	      // If the result turns out to be a noop, then skip it.
	    if (NetBlock*tbl = dynamic_cast<NetBlock*>(tmp))
		  if (tbl->proc_first() == 0) {
			delete tbl;
			continue;
		  }

	    cur->append(tmp);
      }

	// Update flags in parent scope.
      if (!nscope->is_const_func())
	    scope->is_const_func(false);
      if (nscope->calls_sys_task())
	    scope->calls_sys_task(true);

      cur->set_line(*this);
      return cur;
}

static int test_case_width(Design*des, NetScope*scope, PExpr*pe,
			   PExpr::width_mode_t&mode)
{
      unsigned expr_width = pe->test_width(des, scope, mode);
      if (debug_elaborate) {
	    cerr << pe->get_fileline() << ": debug: test_width "
		 << "of case expression " << *pe
		 << endl;
	    cerr << pe->get_fileline() << ":        "
		 << "returns type=" << pe->expr_type()
		 << ", width="      << expr_width
		 << ", signed="     << pe->has_sign()
		 << ", mode="       << PExpr::width_mode_name(mode)
		 << endl;
      }
      return expr_width;
}

static NetExpr*elab_and_eval_case(Design*des, NetScope*scope, PExpr*pe,
				  bool context_is_real, bool context_unsigned,
				  unsigned context_width)
{
      if (context_unsigned)
	    pe->cast_signed(false);

      unsigned width = context_is_real ? pe->expr_width() : context_width;
      NetExpr*expr = pe->elaborate_expr(des, scope, width, PExpr::NO_FLAGS);
      if (expr == 0) return 0;

      if (context_is_real)
	    expr = cast_to_real(expr);

      eval_expr(expr, context_width);

      return expr;
}

/*
 * Elaborate a case statement.
 */
NetProc* PCase::elaborate(Design*des, NetScope*scope) const
{
      ivl_assert(*this, scope);

	/* The type of the case expression and case item expressions is
	   determined according to the following rules:

	    - if any of the expressions is real, all the expressions are
	      evaluated as real (non-real expressions will be treated as
	      self-determined, then converted to real)

	    - otherwise if any of the expressions is unsigned, all the
	      expressions are evaluated as unsigned

	    - otherwise all the expressions are evaluated as signed

	   If the type is not real, the bit width is determined by the
	   largest self-determined width of any of the expressions. */

      PExpr::width_mode_t context_mode = PExpr::SIZED;
      unsigned context_width = test_case_width(des, scope, expr_, context_mode);
      bool context_is_real = (expr_->expr_type() == IVL_VT_REAL);
      bool context_unsigned = !expr_->has_sign();

      for (unsigned idx = 0; idx < items_->count(); idx += 1) {

	    PCase::Item*cur = (*items_)[idx];

	    for (list<PExpr*>::iterator idx_expr = cur->expr.begin()
		 ; idx_expr != cur->expr.end() ; ++idx_expr) {

		  PExpr*cur_expr = *idx_expr;
		  ivl_assert(*this, cur_expr);

		  PExpr::width_mode_t cur_mode = PExpr::SIZED;
		  unsigned cur_width = test_case_width(des, scope, cur_expr,
						       cur_mode);
		  if (cur_mode > context_mode)
			context_mode = cur_mode;
		  if (cur_width > context_width)
			context_width = cur_width;
		  if (cur_expr->expr_type() == IVL_VT_REAL)
			context_is_real = true;
		  if (!cur_expr->has_sign())
			context_unsigned = true;
	    }
      }

      if (context_is_real) {
	    context_width = 1;
	    context_unsigned = false;

      } else if (context_mode >= PExpr::LOSSLESS) {

	      /* Expressions may choose a different size if they are
		 in a lossless context, so we need to run through the
		 process again to get the final expression width. */

	    context_width = test_case_width(des, scope, expr_, context_mode);

	    for (unsigned idx = 0; idx < items_->count(); idx += 1) {

		  PCase::Item*cur = (*items_)[idx];

		  for (list<PExpr*>::iterator idx_expr = cur->expr.begin()
		       ; idx_expr != cur->expr.end() ; ++idx_expr) {

			PExpr*cur_expr = *idx_expr;
			ivl_assert(*this, cur_expr);

			unsigned cur_width = test_case_width(des, scope, cur_expr,
							     context_mode);
			if (cur_width > context_width)
			      context_width = cur_width;
		  }
	    }

	    if (context_width < integer_width)
		  context_width += 1;
      }

      if (debug_elaborate) {
	    cerr << get_fileline() << ": debug: case context is ";
	    if (context_is_real) {
		  cerr << "real" << endl;
	    } else {
		  cerr << (context_unsigned ? "unsigned" : "signed")
		       << " vector, width=" << context_width << endl;
	    }
      }
      NetExpr*expr = elab_and_eval_case(des, scope, expr_,
					context_is_real,
					context_unsigned,
					context_width);
      if (expr == 0) {
	    cerr << get_fileline() << ": error: Unable to elaborate this case"
		  " expression." << endl;
	    return 0;
      }

	/* Count the items in the case statement. Note that there may
	   be some cases that have multiple guards. Count each as a
	   separate item. */
      unsigned icount = 0;
      for (unsigned idx = 0 ;  idx < items_->count() ;  idx += 1) {
	    PCase::Item*cur = (*items_)[idx];

	    if (cur->expr.empty())
		  icount += 1;
	    else
		  icount += cur->expr.size();
      }

      NetCase*res = new NetCase(quality_, type_, expr, icount);
      res->set_line(*this);

	/* Iterate over all the case items (guard/statement pairs)
	   elaborating them. If the guard has no expression, then this
	   is a "default" case. Otherwise, the guard has one or more
	   expressions, and each guard is a case. */
      unsigned inum = 0;
      for (unsigned idx = 0 ;  idx < items_->count() ;  idx += 1) {

	    ivl_assert(*this, inum < icount);
	    PCase::Item*cur = (*items_)[idx];

	    if (cur->expr.empty()) {
		    /* If there are no expressions, then this is the
		       default case. */
		  NetProc*st = 0;
		  if (cur->stat)
			st = cur->stat->elaborate(des, scope);

		  res->set_case(inum, 0, st);
		  inum += 1;

	    } else for (list<PExpr*>::iterator idx_expr = cur->expr.begin()
			      ; idx_expr != cur->expr.end() ; ++idx_expr) {

		    /* If there are one or more expressions, then
		       iterate over the guard expressions, elaborating
		       a separate case for each. (Yes, the statement
		       will be elaborated again for each.) */
		  PExpr*cur_expr = *idx_expr;
		  ivl_assert(*this, cur_expr);
		  NetExpr*gu = elab_and_eval_case(des, scope, cur_expr,
						  context_is_real,
						  context_unsigned,
						  context_width);

		  NetProc*st = 0;
		  if (cur->stat)
			st = cur->stat->elaborate(des, scope);

		  res->set_case(inum, gu, st);
		  inum += 1;
	    }
      }

      res->prune();

      return res;
}

NetProc* PChainConstructor::elaborate(Design*des, NetScope*scope) const
{
      assert(scope);

      if (debug_elaborate) {
	    cerr << get_fileline() << ": PChainConstructor::elaborate: "
		 << "Elaborate constructor chain in scope=" << scope_path(scope) << endl;
      }

	// The scope is the <class>.new function, so scope->parent()
	// is the class. Use that to get the class type that we are
	// constructing.
      NetScope*scope_class = scope->parent();
      const netclass_t*class_this = scope_class->class_def();
      ivl_assert(*this, class_this);

	// We also need the super-class.
      const netclass_t*class_super = class_this->get_super();
      if (class_super == 0) {
	    cerr << get_fileline() << ": error: "
		 << "Class " << class_this->get_name()
		 << " has no parent class for super.new constructor chaining." << endl;
	    des->errors += 1;
	    NetBlock*tmp = new NetBlock(NetBlock::SEQU, 0);
	    tmp->set_line(*this);
	    return tmp;
      }

	// Need the "this" variable for the current constructor. We're
	// going to pass this to the chained constructor.
      NetNet*var_this = scope->find_signal(perm_string::literal(THIS_TOKEN));

	// If super.new is an implicit constructor, then there are no
	// arguments (other than "this" to worry about, so make a
	// NetEUFunc and there we go.
      if (NetScope*new_scope = class_super->method_from_name(perm_string::literal("new@"))) {
	    NetESignal*eres = new NetESignal(var_this);
	    vector<NetExpr*> parms(1);
	    parms[0] = eres;
	    NetEUFunc*tmp = new NetEUFunc(scope, new_scope, eres, parms, true);
	    tmp->set_line(*this);

	    NetAssign_*lval_this = new NetAssign_(var_this);
	    NetAssign*stmt = new NetAssign(lval_this, tmp);
	    stmt->set_line(*this);
	    return stmt;
      }

	// If super.new(...) is a user defined constructor, then call
	// it. This is a bit more complicated because there may be arguments.
      if (NetScope*new_scope = class_super->method_from_name(perm_string::literal("new"))) {

	    int missing_parms = 0;
	    NetFuncDef*def = new_scope->func_def();
	    ivl_assert(*this, def);

	    NetESignal*eres = new NetESignal(var_this);
	    vector<NetExpr*> parms (def->port_count());
	    parms[0] = eres;

	    for (size_t idx = 1 ; idx < parms.size() ; idx += 1) {
		  if (idx <= parms_.size() && parms_[idx-1]) {
			PExpr*tmp = parms_[idx-1];
			parms[idx] = elaborate_rval_expr(des, scope,
							 def->port(idx)->net_type(),
							 def->port(idx)->data_type(),
							 def->port(idx)->vector_width(),
							 tmp, false);
			continue;
		  }

		  if (NetExpr*tmp = def->port_defe(idx)) {
			parms[idx] = tmp;
			continue;
		  }

		  missing_parms += 1;
		  parms[idx] = 0;
	    }

	    if (missing_parms) {
		  cerr << get_fileline() << ": error: "
		       << "Missing " << missing_parms
		       << " arguments to constructor " << scope_path(new_scope) << "." << endl;
		  des->errors += 1;
	    }

	    NetEUFunc*tmp = new NetEUFunc(scope, new_scope, eres, parms, true);
	    tmp->set_line(*this);

	    NetAssign_*lval_this = new NetAssign_(var_this);
	    NetAssign*stmt = new NetAssign(lval_this, tmp);
	    stmt->set_line(*this);
	    return stmt;
      }

	// There is no constructor at all in the parent, so skip it.
      NetBlock*tmp = new NetBlock(NetBlock::SEQU, 0);
      tmp->set_line(*this);
      return tmp;
}

NetProc* PCondit::elaborate(Design*des, NetScope*scope) const
{
      assert(scope);

      if (debug_elaborate)
	    cerr << get_fileline() << ":  PCondit::elaborate: "
		 << "Elaborate condition statement"
		 << " with conditional: " << *expr_ << endl;

	// Elaborate and try to evaluate the conditional expression.
      NetExpr*expr = elab_and_eval(des, scope, expr_, -1);
      if (expr == 0) {
	    cerr << get_fileline() << ": error: Unable to elaborate"
		  " condition expression." << endl;
	    des->errors += 1;
	    return 0;
      }

	// If the condition of the conditional statement is constant,
	// then look at the value and elaborate either the if statement
	// or the else statement. I don't need both. If there is no
	// else_ statement, then use an empty block as a noop.
      if (NetEConst*ce = dynamic_cast<NetEConst*>(expr)) {
	    verinum val = ce->value();
	    if (debug_elaborate) {
		  cerr << get_fileline() << ": debug: Condition expression "
		       << "is a constant " << val << "." << endl;
	    }

	    verinum::V reduced = verinum::V0;
	    for (unsigned idx = 0 ;  idx < val.len() ;  idx += 1)
		  reduced = reduced | val[idx];

	    delete expr;
	    if (reduced == verinum::V1)
		  if (if_) {
			return if_->elaborate(des, scope);
		  } else {
			NetBlock*tmp = new NetBlock(NetBlock::SEQU, 0);
			tmp->set_line(*this);
			return tmp;
		  }
	    else if (else_)
		  return else_->elaborate(des, scope);
	    else
		  return new NetBlock(NetBlock::SEQU, 0);
      }

	// If the condition expression is more than 1 bits, then
	// generate a comparison operator to get the result down to
	// one bit. Turn <e> into <e> != 0;

      if (expr->expr_width() < 1) {
	    cerr << get_fileline() << ": internal error: "
		  "incomprehensible expression width (0)." << endl;
	    return 0;
      }

	// Make sure the condition expression evaluates to a condition.
      expr = condition_reduce(expr);

	// Well, I actually need to generate code to handle the
	// conditional, so elaborate.
      NetProc*i = if_? if_->elaborate(des, scope) : 0;
      NetProc*e = else_? else_->elaborate(des, scope) : 0;

	// Detect the special cases that the if or else statements are
	// empty blocks. If this is the case, remove the blocks as
	// null statements.
      if (NetBlock*tmp = dynamic_cast<NetBlock*>(i)) {
	    if (tmp->proc_first() == 0) {
		  delete i;
		  i = 0;
	    }
      }

      if (NetBlock*tmp = dynamic_cast<NetBlock*>(e)) {
	    if (tmp->proc_first() == 0) {
		  delete e;
		  e = 0;
	    }
      }

      NetCondit*res = new NetCondit(expr, i, e);
      res->set_line(*this);
      return res;
}

NetProc* PCallTask::elaborate(Design*des, NetScope*scope) const
{
      if (peek_tail_name(path_)[0] == '$')
	    return elaborate_sys(des, scope);
      else
	    return elaborate_usr(des, scope);
}

/*
 * A call to a system task involves elaborating all the parameters,
 * then passing the list to the NetSTask object.
 *XXXX
 * There is a single special case in the call to a system
 * task. Normally, an expression cannot take an unindexed
 * memory. However, it is possible to take a system task parameter a
 * memory if the expression is trivial.
 */
NetProc* PCallTask::elaborate_sys(Design*des, NetScope*scope) const
{
      assert(scope);

      if (path_.size() > 1) {
	    cerr << get_fileline() << ": error: Hierarchical system task names"
		 << " make no sense: " << path_ << endl;
	    des->errors += 1;
      }

      unsigned parm_count = parms_.size();

	/* Catch the special case that the system task has no
	   parameters. The "()" string will be parsed as a single
	   empty parameter, when we really mean no parameters at all. */
      if ((parm_count== 1) && (parms_[0] == 0))
	    parm_count = 0;

      vector<NetExpr*>eparms (parm_count);

      perm_string name = peek_tail_name(path_);

      for (unsigned idx = 0 ;  idx < parm_count ;  idx += 1) {
	    PExpr*ex = parms_[idx];
	    if (ex != 0) {
		  eparms[idx] = elab_sys_task_arg(des, scope, name, idx, ex);
	    } else {
		  eparms[idx] = 0;
	    }
      }

	// Special case: Specify blocks are turned off, and this is an
	// $sdf_annotate system task. There will be nothing for $sdf
	// to annotate, and the user is intending to turn the behavior
	// off anyhow, so replace the system task invocation with a no-op.
      if (gn_specify_blocks_flag == false && name == "$sdf_annotate") {

	    cerr << get_fileline() << ": warning: Omitting $sdf_annotate() "
	         << "since specify blocks are being omitted." << endl;
	    NetBlock*noop = new NetBlock(NetBlock::SEQU, scope);
	    noop->set_line(*this);
	    return noop;
      }

      scope->calls_sys_task(true);

      NetSTask*cur = new NetSTask(name, def_sfunc_as_task, eparms);
      cur->set_line(*this);
      return cur;
}

/*
 * A call to a user defined task is different from a call to a system
 * task because a user task in a netlist has no parameters: the
 * assignments are done by the calling thread. For example:
 *
 *  task foo;
 *    input a;
 *    output b;
 *    [...]
 *  endtask;
 *
 *  [...] foo(x, y);
 *
 * is really:
 *
 *  task foo;
 *    reg a;
 *    reg b;
 *    [...]
 *  endtask;
 *
 *  [...]
 *  begin
 *    a = x;
 *    foo;
 *    y = b;
 *  end
 */
NetProc* PCallTask::elaborate_usr(Design*des, NetScope*scope) const
{
      assert(scope);

      NetScope*pscope = scope;
      if (package_) {
	    pscope = des->find_package(package_->pscope_name());
	    ivl_assert(*this, pscope);
      }

      NetScope*task = des->find_task(pscope, path_);
      if (task == 0) {
	      // For SystemVerilog this may be a few other things.
	    if (gn_system_verilog()) {
		  NetProc *tmp;
		    // This could be a method attached to a signal
		    // or defined in this object?
		  bool try_implicit_this = scope->get_class_scope() && path_.size() == 1;
		  tmp = elaborate_method_(des, scope, try_implicit_this);
		  if (tmp) return tmp;
		    // Or it could be a function call ignoring the return?
		  tmp = elaborate_function_(des, scope);
		  if (tmp) return tmp;
	    }

	    cerr << get_fileline() << ": error: Enable of unknown task "
		 << "``" << path_ << "''." << endl;
	    des->errors += 1;
	    return 0;
      }

      assert(task);
      assert(task->type() == NetScope::TASK);
      NetTaskDef*def = task->task_def();
      if (def == 0) {
	    cerr << get_fileline() << ": internal error: task " << path_
		 << " doesn't have a definition in " << scope_path(scope)
		 << "." << endl;
	    des->errors += 1;
	    return 0;
      }
      assert(def);

	/* In SystemVerilog a method calling another method in the
	 * current class needs to be elaborated as a method with an
	 * implicit this added.  */
      if (gn_system_verilog() && (path_.size() == 1)) {
	    const NetScope *c_scope = scope->get_class_scope();
	    if (c_scope && (c_scope == task->get_class_scope())) {
		  NetProc *tmp = elaborate_method_(des, scope, true);
		  assert(tmp);
		  return tmp;
	    }
      }

      unsigned parm_count = def->port_count();

	/* Handle non-automatic tasks with no parameters specially. There is
           no need to make a sequential block to hold the generated code. */
      if ((parm_count == 0) && !task->is_auto()) {
	      // Check if a task call is allowed in this context.
	    test_task_calls_ok_(des, scope);

	    NetUTask*cur = new NetUTask(task);
	    cur->set_line(*this);
	    return cur;
      }

      return elaborate_build_call_(des, scope, task, 0);
}

/*
 * This private method is called to elaborate built-in methods. The
 * method_name is the detected name of the built-in method, and the
 * sys_task_name is the internal system-task name to use.
 */
NetProc* PCallTask::elaborate_sys_task_method_(Design*des, NetScope*scope,
					       NetNet*net,
					       perm_string method_name,
					       const char*sys_task_name) const
{
      NetESignal*sig = new NetESignal(net);
      sig->set_line(*this);

	/* If there is a single NULL argument then ignore it since it is
	 * left over from the parser and is not needed by the method. */
      unsigned nparms = parms_.size();
      if ((nparms == 1) && (parms_[0] == 0)) nparms = 0;

      vector<NetExpr*>argv (1 + nparms);
      argv[0] = sig;

      if (method_name == "delete") {
	      // The queue delete method takes an optional element.
	    if (net->queue_type()) {
		  if (nparms > 1)  {
			cerr << get_fileline() << ": error: queue delete() "
			     << "method takes zero or one argument." << endl;
			des->errors += 1;
		  }
	    } else if (nparms > 0) {
		  cerr << get_fileline() << ": error: darray delete() "
		       << "method takes no arguments." << endl;
		  des->errors += 1;
	    }
      }

      for (unsigned idx = 0 ; idx < nparms ; idx += 1) {
	    PExpr*ex = parms_[idx];
	    if (ex != 0) {
		  argv[idx+1] = elab_sys_task_arg(des, scope,
						  method_name,
						  idx, ex);
	    } else {
		  argv[idx+1] = 0;
	    }
      }

      NetSTask*sys = new NetSTask(sys_task_name, IVL_SFUNC_AS_TASK_IGNORE, argv);
      sys->set_line(*this);
      return sys;
}

/*
 * This private method is called to elaborate queue push methods. The
 * sys_task_name is the internal system-task name to use.
 */
NetProc* PCallTask::elaborate_queue_method_(Design*des, NetScope*scope,
					    NetNet*net,
					    perm_string method_name,
					    const char*sys_task_name) const
{
      NetESignal*sig = new NetESignal(net);
      sig->set_line(*this);

      unsigned nparms = parms_.size();
	// insert() requires two arguments.
      if ((method_name == "insert") && (nparms != 2)) {
	    cerr << get_fileline() << ": error: " << method_name
		 << "() method requires two arguments." << endl;
	    des->errors += 1;
      }
	// push_front() and push_back() require one argument.
      if ((method_name != "insert") && (nparms != 1)) {
	    cerr << get_fileline() << ": error: " << method_name
		 << "() method requires a single argument." << endl;
	    des->errors += 1;
      }

	// Get the context width if this is a logic type.
      ivl_variable_type_t base_type = net->darray_type()->element_base_type();
      int context_width = -1;
      switch (base_type) {
	  case IVL_VT_BOOL:
	  case IVL_VT_LOGIC:
	    context_width = net->darray_type()->element_width();
	    break;
	  default:
	    break;
      }

      vector<NetExpr*>argv (nparms+1);
      argv[0] = sig;
      if (method_name != "insert") {
	    if ((nparms == 0) || (parms_[0] == 0)) {
		  argv[1] = 0;
		  cerr << get_fileline() << ": error: " << method_name
		       << "() methods first argument is missing." << endl;
		  des->errors += 1;
	    } else
		  argv[1] = elab_and_eval(des, scope, parms_[0], context_width,
		                          false, false, base_type);
      } else {
	    if ((nparms == 0) || (parms_[0] == 0)) {
		  argv[1] = 0;
		  cerr << get_fileline() << ": error: " << method_name
		       << "() methods first argument is missing." << endl;
		  des->errors += 1;
	    } else
		  argv[1] = elab_and_eval(des, scope, parms_[0], 32,
		                          false, false, IVL_VT_LOGIC);

	    if ((nparms < 2) || (parms_[1] == 0)) {
		  argv[2] = 0;
		  cerr << get_fileline() << ": error: " << method_name
		       << "() methods second argument is missing." << endl;
		  des->errors += 1;
	    } else
		  argv[2] = elab_and_eval(des, scope, parms_[1], context_width,
		                          false, false, base_type);
      }

      NetSTask*sys = new NetSTask(sys_task_name, IVL_SFUNC_AS_TASK_IGNORE, argv);
      sys->set_line(*this);
      return sys;
}

/*
 * This is used for array/queue function methods called as tasks.
 */
NetProc* PCallTask::elaborate_method_func_(NetScope*scope,
                                           NetNet*net,
                                           ivl_variable_type_t type,
                                           unsigned width,
                                           bool signed_flag,
					   perm_string method_name,
                                           const char*sys_task_name) const
{
      cerr << get_fileline() << ": warning: method function '"
           << method_name << "' is being called as a task." << endl;

	// Generate the function.
      NetESFunc*sys_expr = new NetESFunc(sys_task_name, type, width, 1);
      sys_expr->set_line(*this);
      NetESignal*arg = new NetESignal(net);
      arg->set_line(*net);
      sys_expr->parm(0, arg);
	// Create a L-value that matches the function return type.
      NetNet*tmp;
      netvector_t*tmp_vec = new netvector_t(type, width-1, 0, signed_flag);
      tmp = new NetNet(scope, scope->local_symbol(), NetNet::REG, tmp_vec);
      tmp->set_line(*this);
      NetAssign_*lv = new NetAssign_(tmp);
	// Generate an assign to the fake L-value.
      NetAssign*cur = new NetAssign(lv, sys_expr);
      cur->set_line(*this);
      return cur;
}

NetProc* PCallTask::elaborate_method_(Design*des, NetScope*scope,
                                      bool add_this_flag) const
{
      pform_name_t use_path = path_;
      perm_string method_name = peek_tail_name(use_path);
      use_path.pop_back();

      NetNet *net;
      ivl_type_t cls_val = 0;
      const NetExpr *par;
      ivl_type_t par_type = 0;
      NetEvent *eve;

	/* Add the implicit this reference when requested. */
      if (add_this_flag) {
	    assert(use_path.empty());
	    use_path.push_front(name_component_t(perm_string::literal(THIS_TOKEN)));
      }

	// There is no signal to search for so this cannot be a method.
      if (use_path.empty()) return 0;

	// Search for an object using the use_path. This should
	// resolve to a class object. Note that the "this" symbol
	// (internally represented as "@") is handled by there being a
	// "this" object in the instance scope.
      symbol_search(this, des, scope, use_path, net, par, eve, par_type, cls_val);

      if (net == 0)
	    return 0;

	// Is this a delete method for dynamic arrays or queues?
      if (net->darray_type()) {
	    if (method_name=="delete")
		  return elaborate_sys_task_method_(des, scope, net, method_name,
		                                    "$ivl_darray_method$delete");
	    else if (method_name=="size")
		    // This returns an int. It could be removed, but keep for now.
		  return elaborate_method_func_(scope, net,
		                                IVL_VT_BOOL, 32,
		                                true, method_name,
		                                "$size");
	    else if (method_name=="reverse") {
		  cerr << get_fileline() << ": sorry: 'reverse()' "
		          "array sorting method is not currently supported."
		       << endl;
		  des->errors += 1;
		  return 0;
	    } else if (method_name=="sort") {
		  cerr << get_fileline() << ": sorry: 'sort()' "
		          "array sorting method is not currently supported."
		       << endl;
		  des->errors += 1;
		  return 0;
	    } else if (method_name=="rsort") {
		  cerr << get_fileline() << ": sorry: 'rsort()' "
		          "array sorting method is not currently supported."
		       << endl;
		  des->errors += 1;
		  return 0;
	    } else if (method_name=="shuffle") {
		  cerr << get_fileline() << ": sorry: 'shuffle()' "
		          "array sorting method is not currently supported."
		       << endl;
		  des->errors += 1;
		  return 0;
	    }
      }

      if (net->queue_type()) {
	    const netdarray_t*use_darray = net->darray_type();
	    if (method_name == "push_back")
		  return elaborate_queue_method_(des, scope, net, method_name,
						 "$ivl_queue_method$push_back");
	    else if (method_name == "push_front")
		  return elaborate_queue_method_(des, scope, net, method_name,
						 "$ivl_queue_method$push_front");
	    else if (method_name == "insert")
		  return elaborate_queue_method_(des, scope, net, method_name,
						 "$ivl_queue_method$insert");
	    else if (method_name == "pop_front")
		  return elaborate_method_func_(scope, net,
		                                use_darray->element_base_type(),
		                                use_darray->element_width(),
		                                false, method_name,
		                                "$ivl_queue_method$pop_front");
	    else if (method_name == "pop_back")
		  return elaborate_method_func_(scope, net,
		                                use_darray->element_base_type(),
		                                use_darray->element_width(),
		                                false, method_name,
		                                "$ivl_queue_method$pop_back");
      }

      if (const netclass_t*class_type = net->class_type()) {
	    NetScope*task = class_type->method_from_name(method_name);
	    if (task == 0) {
		  cerr << get_fileline() << ": error: "
		       << "Can't find task " << method_name
		       << " in class " << class_type->get_name() << endl;
		  des->errors += 1;
		  return 0;
	    }

	    if (debug_elaborate) {
		  cerr << get_fileline() << ": PCallTask::elaborate_method_: "
		       << "Elaborate " << class_type->get_name()
		       << " method " << task->basename() << endl;
	    }

	    NetESignal*use_this = new NetESignal(net);
	    use_this->set_line(*this);

	    return elaborate_build_call_(des, scope, task, use_this);
      }

      return 0;
}

/*
 * If during elaboration we determine (for sure) that we are calling a
 * task (and not just a void function) then this method tests if that
 * task call is allowed in the current context. If so, return true. If
 * not, print an error message and return false;
 */
bool PCallTask::test_task_calls_ok_(Design*des, NetScope*scope) const
{
      if (scope->in_func()) {
	    cerr << get_fileline() << ": error: Functions cannot enable/call "
	            "tasks." << endl;
	    des->errors += 1;
	    return false;
      }

      if (scope->in_final()) {
	    cerr << get_fileline() << ": error: final procedures cannot "
	            "enable/call tasks." << endl;
	    des->errors += 1;
	    return false;
      }

      return true;
}

NetProc* PCallTask::elaborate_function_(Design*des, NetScope*scope) const
{
      NetFuncDef*func = des->find_function(scope, path_);
	// This is not a function, so this task call cannot be a function
	// call with a missing return assignment.
      if (! func) return 0;

      if (gn_system_verilog() && func->is_void())
	    return elaborate_void_function_(des, scope, func);

	// Generate a function call version of this task call.
      PExpr*rval = new PECallFunction(package_, path_, parms_);
      rval->set_file(get_file());
      rval->set_lineno(get_lineno());
	// Generate an assign to nothing.
      PAssign*tmp = new PAssign(0, rval);
      tmp->set_file(get_file());
      tmp->set_lineno(get_lineno());
      cerr << get_fileline() << ": warning: User function '"
           << peek_tail_name(path_) << "' is being called as a task." << endl;
	// Elaborate the assignment to a dummy variable.
      return tmp->elaborate(des, scope);
}

NetProc* PCallTask::elaborate_void_function_(Design*des, NetScope*scope,
					     NetFuncDef*def) const
{
      NetScope*dscope = def->scope();

      if (debug_elaborate) {
	    cerr << get_fileline() << ": PCallTask::elaborate_void_function_: "
		 << "function void " << scope_path(dscope)
		 << endl;
      }

	// If we haven't already elaborated the function, do so now.
	// This allows elaborate_build_call_ to elide the function call
	// if the function body is empty.
      if (dscope->elab_stage() < 3) {
	    const PFunction*pfunc = dscope->func_pform();
	    ivl_assert(*this, pfunc);
	    pfunc->elaborate(des, dscope);
      }
      return elaborate_build_call_(des, scope, dscope, 0);
}

NetProc* PCallTask::elaborate_build_call_(Design*des, NetScope*scope,
					  NetScope*task, NetExpr*use_this) const
{
      NetBaseDef*def = 0;
      if (task->type() == NetScope::TASK) {
	    def = task->task_def();

	      // OK, this is certainly a TASK that I'm calling. Make
	      // sure that is OK where I am. Even if this test fails,
	      // continue with the elaboration as if it were OK so
	      // that we can catch more errors.
	    test_task_calls_ok_(des, scope);

      } else if (task->type() == NetScope::FUNC) {
	    NetFuncDef*tmp = task->func_def();
	    if (!tmp->is_void()) {
		  cerr << get_fileline() << ": error: "
		       << "Calling a non-void function as a task." << endl;
		  des->errors += 1;
	    }
	    def = tmp;
      }

	/* The caller has checked the parms_ size to make sure it
	   matches the task definition, so we can just use the task
	   definition to get the parm_count. */

      unsigned parm_count = def->port_count();

      if (parms_.size() > parm_count) {
	    cerr << get_fileline() << ": error: "
		 << "Too many arguments (" << parms_.size()
		 << ", expecting " << parm_count << ")"
		 << " in call to task." << endl;
	    des->errors += 1;
      }

      NetBlock*block = new NetBlock(NetBlock::SEQU, 0);
      block->set_line(*this);

	/* Detect the case where the definition of the task is known
	   empty. In this case, we need not bother with calls to the
	   task, all the assignments, etc. Just return a no-op. */

      if (const NetBlock*tp = dynamic_cast<const NetBlock*>(def->proc())) {
	    if (tp->proc_first() == 0) {
		  if (debug_elaborate) {
			cerr << get_fileline() << ": PCallTask::elaborate_build_call_: "
			     << "Eliding call to empty task " << task->basename() << endl;
		  }
		  return block;
	    }
      }

        /* If this is an automatic task, generate a statement to
           allocate the local storage. */

      if (task->is_auto()) {
	    NetAlloc*ap = new NetAlloc(task);
	    ap->set_line(*this);
	    block->append(ap);
      }

	/* If this is a method call, then the use_this pointer will
	   have an expression for the "this" argument. The "this"
	   argument is the first argument of any method, so emit it
	   here. */

      if (use_this) {
	    ivl_assert(*this, def->port_count() >= 1);
	    NetNet*port = def->port(0);
	    ivl_assert(*this, port->port_type()==NetNet::PINPUT);

	    NetAssign_*lv = new NetAssign_(port);
	    NetAssign*pr = new NetAssign(lv, use_this);
	    pr->set_line(*this);
	    block->append(pr);
      }

	/* Generate assignment statement statements for the input and
	   INOUT ports of the task. These are managed by writing
	   assignments with the task port the l-value and the passed
	   expression the r-value. We know by definition that the port
	   is a reg type, so this elaboration is pretty obvious. */

      for (unsigned idx = use_this?1:0 ;  idx < parm_count ;  idx += 1) {

	    size_t parms_idx = use_this? idx-1 : idx;

	    NetNet*port = def->port(idx);
	    assert(port->port_type() != NetNet::NOT_A_PORT);
	    if (port->port_type() == NetNet::POUTPUT)
		  continue;

	    NetAssign_*lv = new NetAssign_(port);
	    unsigned wid = count_lval_width(lv);
	    ivl_variable_type_t lv_type = lv->expr_type();

	    NetExpr*rv = 0;

	    if (parms_idx < parms_.size() && parms_[parms_idx]) {
		  rv = elaborate_rval_expr(des, scope, port->net_type(),
					   lv_type, wid, parms_ [parms_idx]);
		  if (NetEEvent*evt = dynamic_cast<NetEEvent*> (rv)) {
			cerr << evt->get_fileline() << ": error: An event '"
			     << evt->event()->name() << "' can not be a user "
			      "task argument." << endl;
			des->errors += 1;
			continue;
		  }

	    } else if (def->port_defe(idx)) {
		  if (! gn_system_verilog()) {
			cerr << get_fileline() << ": internal error: "
			     << "Found (and using) default task expression "
			        "requires SystemVerilog." << endl;
			des->errors += 1;
		  }
		  rv = def->port_defe(idx);
		  if (lv_type==IVL_VT_BOOL||lv_type==IVL_VT_LOGIC)
			rv = pad_to_width(rv->dup_expr(), wid, *this);

	    } else {
		  cerr << get_fileline() << ": error: "
		       << "Missing argument " << (idx+1)
		       << " of call to task." << endl;
		  des->errors += 1;
		  continue;
	    }

	    NetAssign*pr = new NetAssign(lv, rv);
	    pr->set_line(*this);
	    block->append(pr);
      }

	/* Generate the task call proper... */
      NetUTask*cur = new NetUTask(task);
      cur->set_line(*this);
      block->append(cur);

	/* Generate assignment statements for the output and INOUT
	   ports of the task. The l-value in this case is the
	   expression passed as a parameter, and the r-value is the
	   port to be copied out.

	   We know by definition that the r-value of this copy-out is
	   the port, which is a reg. The l-value, however, may be any
	   expression that can be a target to a procedural
	   assignment, including a memory word. */

      for (unsigned idx = use_this?1:0 ;  idx < parm_count ;  idx += 1) {

	    size_t parms_idx = use_this? idx-1 : idx;

	    NetNet*port = def->port(idx);

	      /* Skip input ports. */
	    assert(port->port_type() != NetNet::NOT_A_PORT);
	    if (port->port_type() == NetNet::PINPUT)
		  continue;


	      /* Elaborate an l-value version of the port expression
		 for output and inout ports. If the expression does
		 not exist or is not a valid l-value print an error
		 message. Note that the elaborate_lval method already
		 printed a detailed message for the latter case. */
	    NetAssign_*lv = 0;
	    if (parms_idx < parms_.size() && parms_[parms_idx]) {
		  lv = parms_[parms_idx]->elaborate_lval(des, scope, false, false);
		  if (lv == 0) {
			cerr << parms_[parms_idx]->get_fileline() << ": error: "
			     << "I give up on task port " << (idx+1)
			     << " expression: " << *parms_[parms_idx] << endl;
		  }
	    } else if (port->port_type() == NetNet::POUTPUT) {
		    // Output ports were skipped earlier, so
		    // report the error now.
		  cerr << get_fileline() << ": error: "
		       << "Missing argument " << (idx+1)
		       << " of call to task." << endl;
		  des->errors += 1;
	    }

	    if (lv == 0)
		  continue;

	    NetExpr*rv = new NetESignal(port);

	      /* Handle any implicit cast. */
	    unsigned lv_width = count_lval_width(lv);
	    if (lv->expr_type() != rv->expr_type()) {
		  switch (lv->expr_type()) {
		      case IVL_VT_REAL:
			rv = cast_to_real(rv);
			break;
		      case IVL_VT_BOOL:
			rv = cast_to_int2(rv, lv_width);
			break;
		      case IVL_VT_LOGIC:
			rv = cast_to_int4(rv, lv_width);
			break;
		      default:
			  /* Don't yet know how to handle this. */
			ivl_assert(*this, 0);
			break;
		  }
	    }
	    rv = pad_to_width(rv, lv_width, *this);

	      /* Generate the assignment statement. */
	    NetAssign*ass = new NetAssign(lv, rv);
	    ass->set_line(*this);

	    block->append(ass);
      }

        /* If this is an automatic task, generate a statement to free
           the local storage. */
      if (task->is_auto()) {
	    NetFree*fp = new NetFree(task);
	    fp->set_line(*this);
	    block->append(fp);
      }

      return block;
}

static bool check_parm_is_const(NetExpr*param)
{
// FIXME: Are these actually needed and are others needed?
//      if (dynamic_cast<NetEConstEnum*>(param)) { cerr << "Enum" << endl; return; }
//      if (dynamic_cast<NetECString*>(param)) { cerr << "String" << endl; return; }
      if (dynamic_cast<NetEConstParam*>(param)) return true;
      if (dynamic_cast<NetECReal*>(param)) return true;
      if (dynamic_cast<NetECRealParam*>(param)) return true;
      if (dynamic_cast<NetEConst*>(param)) return true;

      return false;
}

static bool get_value_as_long(const NetExpr*expr, long&val)
{
      switch(expr->expr_type()) {
	  case IVL_VT_REAL: {
	    const NetECReal*c = dynamic_cast<const NetECReal*> (expr);
	    assert(c);
	    verireal tmp = c->value();
	    val = tmp.as_long();
	    break;
	  }

	  case IVL_VT_BOOL:
	  case IVL_VT_LOGIC: {
	    const NetEConst*c = dynamic_cast<const NetEConst*>(expr);
	    assert(c);
	    verinum tmp = c->value();
	    if (tmp.is_string()) return false;
	    val = tmp.as_long();
	    break;
	  }

	  default:
	    return false;
      }

      return true;
}

static bool get_value_as_string(const NetExpr*expr, string&str)
{
      switch(expr->expr_type()) {
	  case IVL_VT_REAL:
	    return false;
	    break;

	  case IVL_VT_BOOL:
	  case IVL_VT_LOGIC: {
	    const NetEConst*c = dynamic_cast<const NetEConst*>(expr);
	    assert(c);
	    verinum tmp = c->value();
	    if (!tmp.is_string()) return false;
	    str = tmp.as_string();
	    break;
	  }

	  default:
	    return false;
      }

      return true;
}

/* Elaborate an elaboration task. */
bool PCallTask::elaborate_elab(Design*des, NetScope*scope) const
{
      assert(scope);
      assert(path_.size() == 1);

      unsigned parm_count = parms_.size();

        /* Catch the special case that the elaboration task has no
           parameters. The "()" string will be parsed as a single
           empty parameter, when we really mean no parameters at all. */
      if ((parm_count== 1) && (parms_[0] == 0))
            parm_count = 0;

      perm_string name = peek_tail_name(path_);

      if (!gn_system_verilog()) {
	    cerr << get_fileline() << ": error: Elaboration task '"
	         << name << "' requires SystemVerilog." << endl;
	    des->errors += 1;
	    return false;
      }

      if (name != "$fatal" &&
          name != "$error" &&
          name != "$warning" &&
          name != "$info") {
	    cerr << get_fileline() << ": error: '" << name
	         << "' is not a valid elaboration task." << endl;
	    des->errors += 1;
	    return true;
      }

      vector<NetExpr*>eparms (parm_count);

      bool const_parms = true;
      for (unsigned idx = 0 ;  idx < parm_count ;  idx += 1) {
            PExpr*ex = parms_[idx];
            if (ex != 0) {
                  eparms[idx] = elab_sys_task_arg(des, scope, name, idx, ex);
		  if (!check_parm_is_const(eparms[idx])) {
			cerr << get_fileline() << ": error: Elaboration task "
			     << name << "() parameter [" << idx+1 << "] '"
			     << *eparms[idx] << "' is not constant." << endl;
			des->errors += 1;
			const_parms = false;
		  }
            } else {
                  eparms[idx] = 0;
            }
      }
      if (!const_parms) return true;

	// Drop the $ and convert to upper case for the severity string
      string sstr = name.str()+1;
      transform(sstr.begin(), sstr.end(), sstr.begin(), ::toupper);

      cerr << sstr << ": " << get_fileline() << ":";
      if (parm_count != 0) {
	    unsigned param_start = 0;
	      // Drop the finish number, but check it is actually valid!
	    if (name == "$fatal") {
		  long finish_num = 1;
		  if (!get_value_as_long(eparms[0],finish_num)) {
			cerr << endl;
			cerr << get_fileline() << ": error: Elaboration task "
			        "$fatal() finish number argument must be "
			        "numeric." << endl;;
			des->errors += 1;
			cerr << string(sstr.size()+1, ' ');
		  } else if ((finish_num < 0) || (finish_num > 2)) {
			cerr << endl;
			cerr << get_fileline() << ": error: Elaboration task "
			        "$fatal() finish number argument must be in "
			        "the range [0-2], given '" << finish_num
			     << "'." << endl;;
			des->errors += 1;
			cerr << string(sstr.size()+1, ' ');
		  }
		  param_start += 1;
	    }

	      // FIXME: For now just handle a single string value.
	    string str;
	    if ((parm_count == param_start + 1) && (get_value_as_string(eparms[param_start], str))) {
		  cerr << " " << str;
	    } else if (param_start < parm_count) {
		  cerr << endl;
		  cerr << get_fileline() << ": sorry: Elaboration tasks "
		          "currently only support a single string argument.";
		  des->errors += 1;
	    }
/*
	    cerr << *eparms[0];
	    for (unsigned idx = 1; idx < parm_count; idx += 1) {
	    cerr << ", " << *eparms[idx];
	    }
*/
      }
      cerr << endl;

      cerr << string(sstr.size(), ' ') << "  During elaboration  Scope: "
           << scope->fullname() << endl;

	// For a fatal mark as an error and fail elaboration.
      if (name == "$fatal") {
	    des->errors += 1;
	    return false;
      }

	// For an error just set it as an error.
      if (name == "$error") des->errors += 1;

      return true;
}

/*
 * Elaborate a procedural continuous assign. This really looks very
 * much like other procedural assignments, at this point, but there
 * is no delay to worry about. The code generator will take care of
 * the differences between continuous assign and normal assignments.
 */
NetCAssign* PCAssign::elaborate(Design*des, NetScope*scope) const
{
      NetCAssign*dev = 0;
      assert(scope);

      if (scope->is_auto() && lval_->has_aa_term(des, scope)) {
	    cerr << get_fileline() << ": error: automatically allocated "
                    "variables may not be assigned values using procedural "
	            "continuous assignments." << endl;
	    des->errors += 1;
	    return 0;
      }

      if (scope->is_auto() && expr_->has_aa_term(des, scope)) {
	    cerr << get_fileline() << ": error: automatically allocated "
                    "variables may not be referenced in procedural "
	            "continuous assignments." << endl;
	    des->errors += 1;
	    return 0;
      }

      NetAssign_*lval = lval_->elaborate_lval(des, scope, true, false);
      if (lval == 0)
	    return 0;

      unsigned lwid = count_lval_width(lval);
      ivl_variable_type_t ltype = lval->expr_type();

	// Need to figure out a better thing to do about the
	// lv_net_type argument to elaborate_rval_expr here. This
	// would entail getting the NetAssign_ to give us an
	// ivl_type_t as needed.
      NetExpr*rexp = elaborate_rval_expr(des, scope, 0, ltype, lwid, expr_);
      if (rexp == 0)
	    return 0;

      dev = new NetCAssign(lval, rexp);

      if (debug_elaborate) {
	    cerr << get_fileline() << ": debug: Elaborate cassign,"
		 << " lval width=" << lwid
		 << " rval width=" << rexp->expr_width()
		 << " rval=" << *rexp
		 << endl;
      }

      dev->set_line(*this);
      return dev;
}

NetDeassign* PDeassign::elaborate(Design*des, NetScope*scope) const
{
      assert(scope);

      if (scope->is_auto() && lval_->has_aa_term(des, scope)) {
	    cerr << get_fileline() << ": error: automatically allocated "
                    "variables may not be assigned values using procedural "
	            "continuous assignments." << endl;
	    des->errors += 1;
	    return 0;
      }

      NetAssign_*lval = lval_->elaborate_lval(des, scope, true, false);
      if (lval == 0)
	    return 0;

      NetDeassign*dev = new NetDeassign(lval);
      dev->set_line( *this );
      return dev;
}

/*
 * Elaborate the delay statement (of the form #<expr> <statement>) as a
 * NetPDelay object. If the expression is constant, evaluate it now
 * and make a constant delay. If not, then pass an elaborated
 * expression to the constructor of NetPDelay so that the code
 * generator knows to evaluate the expression at run time.
 */
NetProc* PDelayStatement::elaborate(Design*des, NetScope*scope) const
{
      assert(scope);

      if (scope->in_func()) {
	    cerr << get_fileline() << ": error: functions cannot have "
	            "delay statements." << endl;
	    des->errors += 1;
	    return 0;
      }

      if (scope->in_final()) {
	    cerr << get_fileline() << ": error: final procedures cannot "
	            "have delay statements." << endl;
	    des->errors += 1;
	    return 0;
      }

	/* This call evaluates the delay expression to a NetEConst, if
	   possible. This includes transforming NetECReal values to
	   integers, and applying the proper scaling. */
      NetExpr*dex = elaborate_delay_expr(delay_, des, scope);

      NetPDelay *obj;
      if (NetEConst*tmp = dynamic_cast<NetEConst*>(dex)) {
	    if (statement_)
		  obj = new NetPDelay(tmp->value().as_ulong64(),
		                      statement_->elaborate(des, scope));
	    else
		  obj = new NetPDelay(tmp->value().as_ulong64(), 0);

	    delete dex;

      } else {
	    if (statement_)
		  obj = new NetPDelay(dex, statement_->elaborate(des, scope));
	    else
		  obj = new NetPDelay(dex, 0);
      }
      obj->set_line(*this);
      return obj;
}

/*
 * The disable statement is not yet supported.
 */
NetProc* PDisable::elaborate(Design*des, NetScope*scope) const
{
      assert(scope);

	/* If the disable scope_ is empty then this is a SystemVerilog
	 * disable fork statement. */
      if (scope_.empty()) {
	    if (gn_system_verilog()) {
		  NetDisable*obj = new NetDisable(0);
		  obj->set_line(*this);
		  return obj;
	    } else {
		  cerr << get_fileline()
		       << ": error: 'disable fork' requires SystemVerilog."
		       << endl;
		  des->errors += 1;
		  return 0;
	    }
      }

      list<hname_t> spath = eval_scope_path(des, scope, scope_);

      NetScope*target = des->find_scope(scope, spath);
      if (target == 0) {
	    cerr << get_fileline() << ": error: Cannot find scope "
		 << scope_ << " in " << scope_path(scope) << endl;
	    des->errors += 1;
	    return 0;
      }

      switch (target->type()) {
	  case NetScope::FUNC:
	    cerr << get_fileline() << ": error: Cannot disable functions." << endl;
	    des->errors += 1;
	    return 0;

	  case NetScope::MODULE:
	    cerr << get_fileline() << ": error: Cannot disable modules." << endl;
	    des->errors += 1;
	    return 0;

	  default:
	    break;
      }

      NetDisable*obj = new NetDisable(target);
      obj->set_line(*this);
      return obj;
}

/*
 * The do/while loop is fairly directly represented in the netlist.
 */
NetProc* PDoWhile::elaborate(Design*des, NetScope*scope) const
{
      NetExpr*ce = elab_and_eval(des, scope, cond_, -1);
      NetProc*sub;
      if (statement_)
	    sub = statement_->elaborate(des, scope);
      else
	    sub = new NetBlock(NetBlock::SEQU, 0);
      if (ce == 0 || sub == 0) {
	    delete ce;
	    delete sub;
	    return 0;
      }
      NetDoWhile*loop = new NetDoWhile(ce, sub);
      loop->set_line(*this);
      return loop;
}

/*
 * An event statement is an event delay of some sort, attached to a
 * statement. Some Verilog examples are:
 *
 *      @(posedge CLK) $display("clock rise");
 *      @event_1 $display("event triggered.");
 *      @(data or negedge clk) $display("data or clock fall.");
 *
 * The elaborated netlist uses the NetEvent, NetEvWait and NetEvProbe
 * classes. The NetEvWait class represents the part of the netlist
 * that is executed by behavioral code. The process starts waiting on
 * the NetEvent when it executes the NetEvWait step. Net NetEvProbe
 * and NetEvTrig are structural and behavioral equivalents that
 * trigger the event, and awakens any processes blocking in the
 * associated wait.
 *
 * The basic data structure is:
 *
 *       NetEvWait ---/--->  NetEvent  <----\---- NetEvProbe
 *        ...         |                     |         ...
 *       NetEvWait ---+                     +---- NetEvProbe
 *                                          |         ...
 *                                          +---- NetEvTrig
 *
 * That is, many NetEvWait statements may wait on a single NetEvent
 * object, and Many NetEvProbe objects may trigger the NetEvent
 * object. The many NetEvWait objects pointing to the NetEvent object
 * reflects the possibility of different places in the code blocking
 * on the same named event, like so:
 *
 *         event foo;
 *           [...]
 *         always begin @foo <statement1>; @foo <statement2> end
 *
 * This tends to not happen with signal edges. The multiple probes
 * pointing to the same event reflect the possibility of many
 * expressions in the same blocking statement, like so:
 *
 *         wire reset, clk;
 *           [...]
 *         always @(reset or posedge clk) <stmt>;
 *
 * Conjunctions like this cause a NetEvent object be created to
 * represent the overall conjunction, and NetEvProbe objects for each
 * event expression.
 *
 * If the NetEvent object represents a named event from the source,
 * then there are NetEvTrig objects that represent the trigger
 * statements instead of the NetEvProbe objects representing signals.
 * For example:
 *
 *         event foo;
 *         always @foo <stmt>;
 *         initial begin
 *                [...]
 *            -> foo;
 *                [...]
 *            -> foo;
 *                [...]
 *         end
 *
 * Each trigger statement in the source generates a separate NetEvTrig
 * object in the netlist. Those trigger objects are elaborated
 * elsewhere.
 *
 * Additional complications arise when named events show up in
 * conjunctions. An example of such a case is:
 *
 *         event foo;
 *         wire bar;
 *         always @(foo or posedge bar) <stmt>;
 *
 * Since there is by definition a NetEvent object for the foo object,
 * this is handled by allowing the NetEvWait object to point to
 * multiple NetEvent objects. All the NetEvProbe based objects are
 * collected and pointed as the synthetic NetEvent object, and all the
 * named events are added into the list of NetEvent object that the
 * NetEvWait object can refer to.
 */

NetProc* PEventStatement::elaborate_st(Design*des, NetScope*scope,
				       NetProc*enet) const
{
      assert(scope);

      if (scope->in_func()) {
	    cerr << get_fileline() << ": error: functions cannot have "
	            "event statements." << endl;
	    des->errors += 1;
	    return 0;
      }

      if (scope->in_final()) {
	    cerr << get_fileline() << ": error: final procedures cannot "
	            "have event statements." << endl;
	    des->errors += 1;
	    return 0;
      }

	/* Create a single NetEvent and NetEvWait. Then, create a
	   NetEvProbe for each conjunctive event in the event
	   list. The NetEvProbe objects all refer back to the NetEvent
	   object. */

      NetEvent*ev = new NetEvent(scope->local_symbol());
      ev->set_line(*this);
      ev->local_flag(true);
      unsigned expr_count = 0;

      NetEvWait*wa = new NetEvWait(enet);
      wa->set_line(*this);

	/* If there are no expressions, this is a signal that it is an
	   @* statement. Generate an expression to use. */

      if (expr_.count() == 0) {
	    assert(enet);
	     /* For synthesis or always_comb/latch we want just the inputs,
	      * but for the rest we want inputs and outputs that may cause
	      * a value to change. */
	    extern bool synthesis; /* Synthesis flag from main.cc */
	    bool rem_out = false;
	    if (synthesis || always_sens_) {
		  rem_out = true;
	    }
	      // If this is an always_comb/latch then we need an implicit T0
	      // trigger of the event expression.
	    if (always_sens_) wa->set_t0_trigger();
	    NexusSet*nset = enet->nex_input(rem_out, always_sens_);
	    if (nset == 0) {
		  cerr << get_fileline() << ": error: Unable to elaborate:"
		       << endl;
		  enet->dump(cerr, 6);
		  des->errors += 1;
		  return enet;
	    }

	    if (nset->size() == 0) {
                  if (always_sens_) return wa;

		  cerr << get_fileline() << ": warning: @* found no "
		          "sensitivities so it will never trigger."
		       << endl;
		    /* Add the currently unreferenced event to the scope. */
		  scope->add_event(ev);
		    /* Delete the current wait, create a new one with no
		     * statement and add the event to it. This creates a
		     * perpetual wait since nothing will ever trigger the
		     * unreferenced event. */
		  delete wa;
		  wa = new NetEvWait(0);
		  wa->set_line(*this);
		  wa->add_event(ev);
		  return wa;
	    }

	    NetEvProbe*pr = new NetEvProbe(scope, scope->local_symbol(),
					   ev, NetEvProbe::ANYEDGE,
					   nset->size());
	    for (unsigned idx = 0 ;  idx < nset->size() ;  idx += 1) {
		  unsigned wid = nset->at(idx).wid;
		  unsigned vwid = nset->at(idx).lnk.nexus()->vector_width();
		    // Is this a part select?
		  if (always_sens_ && (wid != vwid)) {
			cerr << get_fileline() << ": sorry: constant "
			        "selects in always_* processes are not "
			        "currently supported (all bits will be "
			        "included)." << endl;
# if 0
			unsigned base = nset->at(idx).base;
cerr << get_fileline() << ": base = " << base << endl;
// FIXME: make this work with selects that go before the base.
			assert(base < vwid);
			if (base + wid > vwid) wid = vwid - base;
cerr << get_fileline() << ": base = " << base << ", width = " << wid
     << ", expr width = " << vwid << endl;
nset->at(idx).lnk.dump_link(cerr, 4);
cerr << endl;
// FIXME: Convert the link to the appropriate NetNet
			netvector_t*tmp_vec = new netvector_t(IVL_VT_BOOL, vwid, 0);
			NetNet*sig = new NetNet(scope, scope->local_symbol(), NetNet::IMPLICIT, tmp_vec);
			NetPartSelect*tmp = new NetPartSelect(sig, base, wid, NetPartSelect::VP);
			des->add_node(tmp);
			tmp->set_line(*this);
// FIXME: create a part select to get the correct bits to connect.
			connect(tmp->pin(1), nset->at(idx).lnk);
			connect(tmp->pin(0), pr->pin(idx));
# endif
			connect(nset->at(idx).lnk, pr->pin(idx));
		  } else {
			connect(nset->at(idx).lnk, pr->pin(idx));
		  }
	    }

	    delete nset;
	    des->add_node(pr);

	    expr_count = 1;

      } else for (unsigned idx = 0 ;  idx < expr_.count() ;  idx += 1) {

	    assert(expr_[idx]->expr());

	      /* If the expression is an identifier that matches a
		 named event, then handle this case all at once and
		 skip the rest of the expression handling. */

	    if (PEIdent*id = dynamic_cast<PEIdent*>(expr_[idx]->expr())) {
		  NetNet*       sig = 0;
		  const NetExpr*par = 0;
		  NetEvent*     eve = 0;

		  NetScope*use_scope = scope;
		  if (id->package()) {
			use_scope = des->find_package(id->package()->pscope_name());
			ivl_assert(*this, use_scope);
		  }

		  NetScope*found_in = symbol_search(this, des, use_scope,
                                                    id->path(),
						    sig, par, eve);

		  if (found_in && eve) {
			wa->add_event(eve);
			  /* You can not look for the posedge or negedge of
			   * an event. */
			if (expr_[idx]->type() != PEEvent::ANYEDGE) {
                              cerr << get_fileline() << ": error: ";
                              switch (expr_[idx]->type()) {
				  case PEEvent::POSEDGE:
				    cerr << "posedge";
				    break;
				  case PEEvent::NEGEDGE:
				    cerr << "negedge";
				    break;
				  default:
				    cerr << "unknown edge type!";
				    assert(0);
			      }
			      cerr << " can not be used with a named event ("
			           << eve->name() << ")." << endl;
                              des->errors += 1;
			}
			continue;
		  }
	    }


	      /* So now we have a normal event expression. Elaborate
		 the sub-expression as a net and decide how to handle
		 the edge. */

            if (scope->is_auto()) {
                  if (! dynamic_cast<PEIdent*>(expr_[idx]->expr())) {
                        cerr << get_fileline() << ": sorry, complex event "
                                "expressions are not yet supported in "
                                "automatic tasks." << endl;
                        des->errors += 1;
                        return 0;
                  }
            }

	    NetExpr*tmp = elab_and_eval(des, scope, expr_[idx]->expr(), -1);
	    if (tmp == 0) {
		  cerr << get_fileline() << ": error: "
			  "Failed to evaluate event expression '"
		       << *expr_[idx] << "'." << endl;
		  des->errors += 1;
		  continue;
	    }

	    NetNet*expr = tmp->synthesize(des, scope, tmp);
	    if (expr == 0) {
		  expr_[idx]->dump(cerr);
		  cerr << endl;
		  des->errors += 1;
		  continue;
	    }
	    assert(expr);

	    delete tmp;

	    unsigned pins = (expr_[idx]->type() == PEEvent::ANYEDGE)
		  ? expr->pin_count() : 1;

	    NetEvProbe*pr;
	    switch (expr_[idx]->type()) {
		case PEEvent::POSEDGE:
		  pr = new NetEvProbe(scope, scope->local_symbol(), ev,
				      NetEvProbe::POSEDGE, pins);
		  break;

		case PEEvent::NEGEDGE:
		  pr = new NetEvProbe(scope, scope->local_symbol(), ev,
				      NetEvProbe::NEGEDGE, pins);
		  break;

		case PEEvent::EDGE:
		  pr = new NetEvProbe(scope, scope->local_symbol(), ev,
				      NetEvProbe::EDGE, pins);
		  break;

		case PEEvent::ANYEDGE:
		  pr = new NetEvProbe(scope, scope->local_symbol(), ev,
				      NetEvProbe::ANYEDGE, pins);
		  break;

		default:
		  pr = NULL;
		  assert(0);
	    }

	    for (unsigned p = 0 ;  p < pr->pin_count() ; p += 1)
		  connect(pr->pin(p), expr->pin(p));

	    des->add_node(pr);
	    expr_count += 1;
      }

	/* If there was at least one conjunction that was an
	   expression (and not a named event) then add this
	   event. Otherwise, we didn't use it so delete it. */
      if (expr_count > 0) {
	    scope->add_event(ev);
	    wa->add_event(ev);
	      /* NOTE: This event that I am adding to the wait may be
		 a duplicate of another event somewhere else. However,
		 I don't know that until all the modules are hooked
		 up, so it is best to leave find_similar_event to
		 after elaboration. */
      } else {
	    delete ev;
      }

      return wa;
}

/*
 * This is the special case of the event statement, the wait
 * statement. This is elaborated into a slightly more complicated
 * statement that uses non-wait statements:
 *
 *     wait (<expr>)  <statement>
 *
 * becomes
 *
 *     begin
 *         while (1 !== <expr>)
 *           @(<expr inputs>) <noop>;
 *         <statement>;
 *     end
 */
NetProc* PEventStatement::elaborate_wait(Design*des, NetScope*scope,
					 NetProc*enet) const
{
      assert(scope);
      assert(expr_.count() == 1);

      if (scope->in_func()) {
	    cerr << get_fileline() << ": error: functions cannot have "
	            "wait statements." << endl;
	    des->errors += 1;
	    return 0;
      }

      if (scope->in_final()) {
	    cerr << get_fileline() << ": error: final procedures cannot "
	            "have wait statements." << endl;
	    des->errors += 1;
	    return 0;
      }

      PExpr *pe = expr_[0]->expr();

	/* Elaborate wait expression. Don't eval yet, we will do that
	   shortly, after we apply a reduction or. */

      PExpr::width_mode_t mode = PExpr::SIZED;
      pe->test_width(des, scope, mode);
      NetExpr*expr = pe->elaborate_expr(des, scope, pe->expr_width(),
                                        PExpr::NO_FLAGS);
      if (expr == 0) {
	    cerr << get_fileline() << ": error: Unable to elaborate"
		  " wait condition expression." << endl;
	    des->errors += 1;
	    return 0;
      }

	// If the condition expression is more than 1 bits, then
	// generate a reduction operator to get the result down to
	// one bit. In other words, Turn <e> into |<e>;

      if (expr->expr_width() < 1) {
	    cerr << get_fileline() << ": internal error: "
		  "incomprehensible wait expression width (0)." << endl;
	    return 0;
      }

      if (expr->expr_width() > 1) {
	    assert(expr->expr_width() > 1);
	    NetEUReduce*cmp = new NetEUReduce('|', expr);
	    cmp->set_line(*pe);
	    expr = cmp;
      }

	/* precalculate as much as possible of the wait expression. */
      eval_expr(expr);

	/* Detect the unusual case that the wait expression is
	   constant. Constant true is OK (it becomes transparent) but
	   constant false is almost certainly not what is intended. */
      assert(expr->expr_width() == 1);
      if (NetEConst*ce = dynamic_cast<NetEConst*>(expr)) {
	    verinum val = ce->value();
	    assert(val.len() == 1);

	      /* Constant true -- wait(1) <s1> reduces to <s1>. */
	    if (val[0] == verinum::V1) {
		  delete expr;
		  assert(enet);
		  return enet;
	    }

	      /* Otherwise, false. wait(0) blocks permanently. */

	    cerr << get_fileline() << ": warning: wait expression is "
		 << "constant false." << endl;
	    cerr << get_fileline() << ":        : The statement will "
		 << "block permanently." << endl;

	      /* Create an event wait and an otherwise unreferenced
		 event variable to force a perpetual wait. */
	    NetEvent*wait_event = new NetEvent(scope->local_symbol());
	    wait_event->set_line(*this);
	    wait_event->local_flag(true);
	    scope->add_event(wait_event);

	    NetEvWait*wait = new NetEvWait(0);
	    wait->add_event(wait_event);
	    wait->set_line(*this);

	    delete expr;
	    delete enet;
	    return wait;
      }

	/* Invert the sense of the test with an exclusive NOR. In
	   other words, if this adjusted expression returns TRUE, then
	   wait. */
      assert(expr->expr_width() == 1);
      expr = new NetEBComp('N', expr, new NetEConst(verinum(verinum::V1)));
      expr->set_line(*pe);
      eval_expr(expr);

      NetEvent*wait_event = new NetEvent(scope->local_symbol());
      wait_event->set_line(*this);
      wait_event->local_flag(true);
      scope->add_event(wait_event);

      NetEvWait*wait = new NetEvWait(0 /* noop */);
      wait->add_event(wait_event);
      wait->set_line(*this);

      NexusSet*wait_set = expr->nex_input();
      if (wait_set == 0) {
	    cerr << get_fileline() << ": internal error: No NexusSet"
		 << " from wait expression." << endl;
	    des->errors += 1;
	    return 0;
      }

      if (wait_set->size() == 0) {
	    cerr << get_fileline() << ": internal error: Empty NexusSet"
		 << " from wait expression." << endl;
	    des->errors += 1;
	    return 0;
      }

      NetEvProbe*wait_pr = new NetEvProbe(scope, scope->local_symbol(),
					  wait_event, NetEvProbe::ANYEDGE,
					  wait_set->size());
      for (unsigned idx = 0; idx < wait_set->size() ;  idx += 1)
	    connect(wait_set->at(idx).lnk, wait_pr->pin(idx));

      delete wait_set;
      des->add_node(wait_pr);

      NetWhile*loop = new NetWhile(expr, wait);
      loop->set_line(*this);

	/* If there is no real substatement (i.e., "wait (foo) ;") then
	   we are done. */
      if (enet == 0)
	    return loop;

	/* Create a sequential block to combine the wait loop and the
	   delayed statement. */
      NetBlock*block = new NetBlock(NetBlock::SEQU, 0);
      block->append(loop);
      block->append(enet);
      block->set_line(*this);

      return block;
}

/*
 * This is a special case of the event statement, the wait fork
 * statement. This is elaborated into a simplified statement.
 *
 *     wait fork;
 *
 * becomes
 *
 *     @(0) <noop>;
 */
NetProc* PEventStatement::elaborate_wait_fork(Design*des, NetScope*scope) const
{
      assert(scope);
      assert(expr_.count() == 1);
      assert(expr_[0] == 0);
      assert(! statement_);

      if (scope->in_func()) {
	    cerr << get_fileline() << ": error: functions cannot have "
	            "wait fork statements." << endl;
	    des->errors += 1;
	    return 0;
      }

      if (scope->in_final()) {
	    cerr << get_fileline() << ": error: final procedures cannot "
	            "have wait fork statements." << endl;
	    des->errors += 1;
	    return 0;
      }

      if (gn_system_verilog()) {
	    NetEvWait*wait = new NetEvWait(0 /* noop */);
	    wait->add_event(0);
	    wait->set_line(*this);
	    return wait;
      } else {
	    cerr << get_fileline()
	         << ": error: 'wait fork' requires SystemVerilog." << endl;
	    des->errors += 1;
	    return 0;
      }

}

NetProc* PEventStatement::elaborate(Design*des, NetScope*scope) const
{
	/* Check to see if this is a wait fork statement. */
      if ((expr_.count() == 1) && (expr_[0] == 0))
		  return elaborate_wait_fork(des, scope);

      NetProc*enet = 0;
      if (statement_) {
	    enet = statement_->elaborate(des, scope);
	    if (enet == 0)
		  return 0;

      } else {
	    enet = new NetBlock(NetBlock::SEQU, 0);
	    enet->set_line(*this);
      }

      if ((expr_.count() == 1) && (expr_[0]->type() == PEEvent::POSITIVE))
	    return elaborate_wait(des, scope, enet);

      return elaborate_st(des, scope, enet);
}

/*
 * Forever statements are represented directly in the netlist. It is
 * theoretically possible to use a while structure with a constant
 * expression to represent the loop, but why complicate the code
 * generators so?
 */
NetProc* PForever::elaborate(Design*des, NetScope*scope) const
{
      NetProc*stat;
      if (statement_)
	    stat = statement_->elaborate(des, scope);
      else
	    stat = new NetBlock(NetBlock::SEQU, 0);
      if (stat == 0) return 0;

      NetForever*proc = new NetForever(stat);
      proc->set_line(*this);
      return proc;
}

/*
 * Force is like a procedural assignment, most notably procedural
 * continuous assignment:
 *
 *    force <lval> = <rval>
 *
 * The <lval> can be anything that a normal behavioral assignment can
 * take, plus net signals. This is a little bit more lax than the
 * other procedural assignments.
 */
NetForce* PForce::elaborate(Design*des, NetScope*scope) const
{
      NetForce*dev = 0;
      assert(scope);

      if (scope->is_auto() && lval_->has_aa_term(des, scope)) {
	    cerr << get_fileline() << ": error: automatically allocated "
                    "variables may not be assigned values using procedural "
	            "force statements." << endl;
	    des->errors += 1;
	    return 0;
      }

      if (scope->is_auto() && expr_->has_aa_term(des, scope)) {
	    cerr << get_fileline() << ": error: automatically allocated "
                    "variables may not be referenced in procedural force "
	            "statements." << endl;
	    des->errors += 1;
	    return 0;
      }

      NetAssign_*lval = lval_->elaborate_lval(des, scope, false, true);
      if (lval == 0)
	    return 0;

      unsigned lwid = count_lval_width(lval);
      ivl_variable_type_t ltype = lval->expr_type();

	// Like a variety of other assigns, we need to figure out a
	// better way to get a reasonable lv_net_type value, and that
	// probably will involve NetAssign_ having a method for
	// synthesizing one as needed.
      NetExpr*rexp = elaborate_rval_expr(des, scope, 0, ltype, lwid, expr_);
      if (rexp == 0)
	    return 0;

      dev = new NetForce(lval, rexp);

      if (debug_elaborate) {
	    cerr << get_fileline() << ": debug: Elaborate force,"
		 << " lval width=" << lval->lwidth()
		 << " rval width=" << rexp->expr_width()
		 << " rval=" << *rexp
		 << endl;
      }

      dev->set_line(*this);
      return dev;
}

static void find_property_in_class(const LineInfo&loc, const NetScope*scope, perm_string name, const netclass_t*&found_in, int&property)
{
      found_in = find_class_containing_scope(loc, scope);
      property = -1;

      if (found_in==0) return;

      property = found_in->property_idx_from_name(name);
      if (property < 0) {
	    found_in = 0;
	    return;
      }
}

/*
 * The foreach statement can be written as a for statement like so:
 *
 *     for (<idx> = $low(<array>) ; <idx> <= $high(<array>) ; <idx> += 1)
 *          <statement_>
 *
 * The <idx> variable is already known to be in the containing named
 * block scope, which was created by the parser.
 */
NetProc* PForeach::elaborate(Design*des, NetScope*scope) const
{
	// Locate the signal for the array variable
      pform_name_t array_name;
      array_name.push_back(name_component_t(array_var_));
      NetNet*array_sig = des->find_signal(scope, array_name);

	// And if necessary, look for the class property that is
	// referenced.
      const netclass_t*class_scope = 0;
      int class_property = -1;
      if (array_sig == 0)
	    find_property_in_class(*this, scope, array_var_, class_scope, class_property);

      if (debug_elaborate && array_sig) {
	    cerr << get_fileline() << ": PForeach::elaborate: "
		 << "Found array_sig in " << scope_path(array_sig->scope()) << "." << endl;
      }

      if (debug_elaborate && class_scope) {
	    cerr << get_fileline() << ": PForeach::elaborate: "
		 << "Found array_sig property (" << class_property
		 << ") in class " << class_scope->get_name()
		 << " as " << *class_scope->get_prop_type(class_property) << "." << endl;
      }

      if (class_scope!=0 && class_property >= 0) {
	    ivl_type_t ptype = class_scope->get_prop_type(class_property);
	    const netsarray_t*atype = dynamic_cast<const netsarray_t*> (ptype);
	    if (atype == 0) {
		  cerr << get_fileline() << ": error: "
		       << "I can't handle the type of " << array_var_
		       << " as a foreach loop." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    const std::vector<netrange_t>&dims = atype->static_dimensions();
	    if (dims.size() < index_vars_.size()) {
		  cerr << get_fileline() << ": error: "
		       << "class " << class_scope->get_name()
		       << " property " << array_var_
		       << " has too few dimensions for foreach dimension list." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    return elaborate_static_array_(des, scope, dims);
      }

      if (array_sig == 0) {
	    cerr << get_fileline() << ": error:"
		 << " Unable to find foreach array " << array_name
		 << " in scope " << scope_path(scope)
		 << "." << endl;
	    des->errors += 1;
	    return 0;
      }

      ivl_assert(*this, array_sig);

      if (debug_elaborate) {
	    cerr << get_fileline() << ": PForeach::elaborate: "
		 << "Scan array " << array_sig->name()
		 << " of " << array_sig->data_type()
		 << " with " << array_sig->unpacked_dimensions() << " unpacked"
		 << " and " << array_sig->packed_dimensions()
		 << " packed dimensions." << endl;
      }

      std::vector<netrange_t>dims = array_sig->unpacked_dims();
      if (array_sig->packed_dimensions() > 0) {
            dims.insert(dims.end(), array_sig->packed_dims().begin(), array_sig->packed_dims().end());
      }

	// Classic arrays are processed this way.
      if (array_sig->data_type()==IVL_VT_BOOL)
	    return elaborate_static_array_(des, scope, dims);
      if (array_sig->data_type()==IVL_VT_LOGIC)
	    return elaborate_static_array_(des, scope, dims);
      if (array_sig->unpacked_dimensions() >= index_vars_.size())
	    return elaborate_static_array_(des, scope, dims);

	// At this point, we know that the array is dynamic so we
	// handle that slightly differently, using run-time tests.

      if (index_vars_.size() != 1) {
	    cerr << get_fileline() << ": sorry: "
		 << "Multi-index foreach loops not supported." << endl;
	    des->errors += 1;
      }

	// Get the signal for the index variable.
      pform_name_t index_name;
      index_name.push_back(name_component_t(index_vars_[0]));
      NetNet*idx_sig = des->find_signal(scope, index_name);
      ivl_assert(*this, idx_sig);

      NetESignal*array_exp = new NetESignal(array_sig);
      array_exp->set_line(*this);

      NetESignal*idx_exp = new NetESignal(idx_sig);
      idx_exp->set_line(*this);

	// Make an initialization expression for the index.
      NetESFunc*init_expr = new NetESFunc("$low", IVL_VT_BOOL, 32, 1);
      init_expr->set_line(*this);
      init_expr->parm(0, array_exp);

	// Make a condition expression: idx <= $high(array)
      NetESFunc*high_exp = new NetESFunc("$high", IVL_VT_BOOL, 32, 1);
      high_exp->set_line(*this);
      high_exp->parm(0, array_exp);

      NetEBComp*cond_expr = new NetEBComp('L', idx_exp, high_exp);
      cond_expr->set_line(*this);

	/* Elaborate the statement that is contained in the foreach
	   loop. */
      NetProc*sub;
      if (statement_)
	    sub = statement_->elaborate(des, scope);
      else
	    sub = new NetBlock(NetBlock::SEQU, 0);

	/* Make a step statement: idx += 1 */
      NetAssign_*idx_lv = new NetAssign_(idx_sig);
      NetEConst*step_val = make_const_val(1);
      NetAssign*step = new NetAssign(idx_lv, '+', step_val);
      step->set_line(*this);

      NetForLoop*stmt = new NetForLoop(idx_sig, init_expr, cond_expr, sub, step);
      stmt->set_line(*this);
      stmt->wrap_up();

      return stmt;
}

/*
 * This is a variant of the PForeach::elaborate() method that handles
 * the case that the array has static dimensions. We can use constants
 * and possibly do some optimizations.
 */
NetProc* PForeach::elaborate_static_array_(Design*des, NetScope*scope,
					   const vector<netrange_t>&dims) const
{
      if (debug_elaborate) {
	    cerr << get_fileline() << ": PForeach::elaborate_static_array_: "
		 << "Handle as array with static dimensions." << endl;
      }

      ivl_assert(*this, index_vars_.size() > 0);
      ivl_assert(*this, dims.size() >= index_vars_.size());

      NetProc*sub;
      if (statement_)
	    sub = statement_->elaborate(des, scope);
      else
	    sub = new NetBlock(NetBlock::SEQU, 0);
      NetForLoop*stmt = 0;

      for (int idx_idx = index_vars_.size()-1 ; idx_idx >= 0 ; idx_idx -= 1) {
	    const netrange_t&idx_range = dims[idx_idx];

	      // Get the $high and $low constant values for this slice
	      // of the array.
	    NetEConst*hig_expr = make_const_val_s(idx_range.get_msb());
	    NetEConst*low_expr = make_const_val_s(idx_range.get_lsb());
	    if (idx_range.get_msb() < idx_range.get_lsb()) {
		  NetEConst*tmp = hig_expr;
		  hig_expr = low_expr;
		  low_expr = tmp;
	    }

	    hig_expr->set_line(*this);
	    low_expr->set_line(*this);

	    pform_name_t idx_name;
	    idx_name.push_back(name_component_t(index_vars_[idx_idx]));
	    NetNet*idx_sig = des->find_signal(scope, idx_name);
	    ivl_assert(*this, idx_sig);

	      // Make the condition expression <idx> <= $high(slice)
	    NetESignal*idx_expr = new NetESignal(idx_sig);
	    idx_expr->set_line(*this);

	    NetEBComp*cond_expr = new NetEBComp('L', idx_expr, hig_expr);
	    cond_expr->set_line(*this);

	      // Make the step statement: <idx> += 1
	    NetAssign_*idx_lv = new NetAssign_(idx_sig);
	    NetEConst*step_val = make_const_val_s(1);
	    NetAssign*step = new NetAssign(idx_lv, '+', step_val);
	    step->set_line(*this);

	    stmt = new NetForLoop(idx_sig, low_expr, cond_expr, sub, step);
	    stmt->set_line(*this);
	    stmt->wrap_up();

	    sub = stmt;
      }

      return stmt? stmt : sub;
}

/*
 * elaborate the for loop as the equivalent while loop. This eases the
 * task for the target code generator. The structure is:
 *
 *     begin : top
 *       name1_ = expr1_;
 *       while (cond_) begin : body
 *          statement_;
 *          name2_ = expr2_;
 *       end
 *     end
 */
NetProc* PForStatement::elaborate(Design*des, NetScope*scope) const
{
      NetExpr*initial_expr;
      assert(scope);

      const PEIdent*id1 = dynamic_cast<const PEIdent*>(name1_);
      assert(id1);

	/* make the expression, and later the initial assignment to
	   the condition variable. The statement in the for loop is
	   very specifically an assignment. */
      NetNet*sig = des->find_signal(scope, id1->path());
      if (sig == 0) {
	    cerr << id1->get_fileline() << ": register ``" << id1->path()
		 << "'' unknown in " << scope_path(scope) << "." << endl;
	    des->errors += 1;
	    return 0;
      }
      assert(sig);

	/* Make the r-value of the initial assignment, and size it
	   properly. Then use it to build the assignment statement. */
      initial_expr = elaborate_rval_expr(des, scope, sig->net_type(),
					 sig->data_type(), sig->vector_width(),
					 expr1_);

      if (debug_elaborate && initial_expr) {
	    cerr << get_fileline() << ": debug: FOR initial assign: "
		 << sig->name() << " = " << *initial_expr << endl;
      }

	/* Elaborate the statement that is contained in the for
	   loop. If there is an error, this will return 0 and I should
	   skip the append. No need to worry, the error has been
	   reported so it's OK that the netlist is bogus. */
      NetProc*sub;
      if (statement_)
	    sub = statement_->elaborate(des, scope);
      else
	    sub = new NetBlock(NetBlock::SEQU, 0);

	/* Now elaborate the for_step statement. I really should do
	   some error checking here to make sure the step statement
	   really does step the variable. */
      NetProc*step = step_->elaborate(des, scope);

	/* Elaborate the condition expression. Try to evaluate it too,
	   in case it is a constant. This is an interesting case
	   worthy of a warning. */
      NetExpr*ce = elab_and_eval(des, scope, cond_, -1);
      if (dynamic_cast<NetEConst*>(ce)) {
	    cerr << get_fileline() << ": warning: condition expression "
		  "of for-loop is constant." << endl;
      }

	/* Error recovery - if we failed to elaborate any of the loop
	   expressions, give up now. */
      if (initial_expr == 0 || ce == 0 || step == 0 || sub == 0) {
	    delete initial_expr;
	    delete ce;
	    delete step;
	    delete sub;
	    return 0;
      }
	/* All done, build up the loop. */

      NetForLoop*loop = new NetForLoop(sig, initial_expr, ce, sub, step);
      loop->set_line(*this);
      loop->wrap_up();
      return loop;
}

/*
 * (See the PTask::elaborate methods for basic common stuff.)
 *
 * The return value of a function is represented as a reg variable
 * within the scope of the function that has the name of the
 * function. So for example with the function:
 *
 *    function [7:0] incr;
 *      input [7:0] in1;
 *      incr = in1 + 1;
 *    endfunction
 *
 * The scope of the function is <parent>.incr and there is a reg
 * variable <parent>.incr.incr. The elaborate_1 method is called with
 * the scope of the function, so the return reg is easily located.
 *
 * The function parameters are all inputs, except for the synthetic
 * output parameter that is the return value. The return value goes
 * into port 0, and the parameters are all the remaining ports.
 */

void PFunction::elaborate(Design*des, NetScope*scope) const
{
      if (scope->elab_stage() > 2)
            return;

      scope->set_elab_stage(3);

      NetFuncDef*def = scope->func_def();
      if (def == 0) {
	    cerr << get_fileline() << ": internal error: "
		 << "No function definition for function "
		 << scope_path(scope) << endl;
	    des->errors += 1;
	    return;
      }
      assert(def);

      NetProc*st;
      if (statement_ == 0) {
	    st = new NetBlock(NetBlock::SEQU, 0);
      } else {
	    st = statement_->elaborate(des, scope);
	    if (st == 0) {
		  cerr << statement_->get_fileline() << ": error: Unable to elaborate "
			  "statement in function " << scope->basename() << "." << endl;
		  scope->is_const_func(true); // error recovery
		  des->errors += 1;
		  return;
	    }
      }

	// Handle any variable initialization statements in this scope.
	// For automatic functions, these statements need to be executed
	// each time the function is called, so insert them at the start
	// of the elaborated definition. For static functions, put them
	// in a separate process that will be executed before the start
	// of simulation.
      if (is_auto_) {
	      // Get the NetBlock of the statement. If it is not a
	      // NetBlock then create one to wrap the initialization
	      // statements and the original statement.
	    NetBlock*blk = dynamic_cast<NetBlock*> (st);
	    if ((blk == 0) && (var_inits.size() > 0)) {
		  blk = new NetBlock(NetBlock::SEQU, scope);
		  blk->set_line(*this);
		  blk->append(st);
		  st = blk;
	    }
	    for (unsigned idx = var_inits.size(); idx > 0; idx -= 1) {
		  NetProc*tmp = var_inits[idx-1]->elaborate(des, scope);
		  if (tmp) blk->prepend(tmp);
	    }
      } else {
	    elaborate_var_inits_(des, scope);
      }

      def->set_proc(st);
}

NetProc* PRelease::elaborate(Design*des, NetScope*scope) const
{
      assert(scope);

      if (scope->is_auto() && lval_->has_aa_term(des, scope)) {
	    cerr << get_fileline() << ": error: automatically allocated "
                    "variables may not be assigned values using procedural "
	            "force statements." << endl;
	    des->errors += 1;
	    return 0;
      }

      NetAssign_*lval = lval_->elaborate_lval(des, scope, false, true);
      if (lval == 0)
	    return 0;

      NetRelease*dev = new NetRelease(lval);
      dev->set_line( *this );
      return dev;
}

NetProc* PRepeat::elaborate(Design*des, NetScope*scope) const
{
      assert(scope);

      NetExpr*expr = elab_and_eval(des, scope, expr_, -1);
      if (expr == 0) {
	    cerr << get_fileline() << ": Unable to elaborate"
		  " repeat expression." << endl;
	    des->errors += 1;
	    return 0;
      }
	// If the expression is real, convert to an integer. 64 bits
	// should be more enough for any real use case.
      if (expr->expr_type() == IVL_VT_REAL)
	    expr = cast_to_int4(expr, 64);

      NetProc*stat;
      if (statement_)
	    stat = statement_->elaborate(des, scope);
      else
	    stat = new NetBlock(NetBlock::SEQU, 0);
      if (stat == 0) return 0;

	// If the expression is a constant, handle certain special
	// iteration counts.
      if (NetEConst*ce = dynamic_cast<NetEConst*>(expr)) {
	    long val = ce->value().as_long();
	    if (val <= 0) {
		  delete expr;
		  delete stat;
		  return new NetBlock(NetBlock::SEQU, 0);
	    } else if (val == 1) {
		  delete expr;
		  return stat;
	    }
      }

      NetRepeat*proc = new NetRepeat(expr, stat);
      proc->set_line( *this );
      return proc;
}

NetProc* PReturn::elaborate(Design*des, NetScope*scope) const
{
      NetScope*target = scope;
      for (;;) {
	    if (target == 0) {
		  cerr << get_fileline() << ": error: "
		       << "Return statement is not in a function." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    if (target->type() == NetScope::FUNC)
		  break;

	    if (target->type() == NetScope::TASK) {
		  cerr << get_fileline() << ": error: "
		       << "Cannot \"return\" from tasks." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    if (target->type()==NetScope::BEGIN_END) {
		  target = target->parent();
		  continue;
	    }

	    cerr << get_fileline() << ": error: "
		 << "Cannot \"return\" from this scope: " << scope_path(target) << endl;
	    des->errors += 1;
	    return 0;
      }
      ivl_assert(*this, target->type() == NetScope::FUNC);

      if (target->func_def()->is_void()) {
	    if (expr_) {
		  cerr << get_fileline() << ": error: "
		       << "A value can't be returned from a void function." << endl;
		  des->errors += 1;
		  return 0;
	    }
	    NetDisable*disa = new NetDisable(target);
	    disa->set_line( *this );
	    return disa;
      }

      if (expr_ == 0) {
	    cerr << get_fileline() << ": error: "
		 << "Return from " << scope_path(target)
		 << " requires a return value expression." << endl;
	    des->errors += 1;
	    return 0;
      }

      NetNet*res = target->find_signal(target->basename());
      ivl_assert(*this, res);
      ivl_variable_type_t lv_type = res->data_type();
      unsigned long wid = res->vector_width();
      NetAssign_*lv = new NetAssign_(res);

      NetExpr*val = elaborate_rval_expr(des, scope, res->net_type(),
					lv_type, wid, expr_);

      NetBlock*proc = new NetBlock(NetBlock::SEQU, 0);
      proc->set_line( *this );

      NetAssign*assn = new NetAssign(lv, val);
      assn->set_line( *this );
      proc->append(assn);

      NetDisable*disa = new NetDisable(target);
      disa->set_line( *this );
      proc->append( disa );

      return proc;
}

/*
 * A task definition is elaborated by elaborating the statement that
 * it contains, and connecting its ports to NetNet objects. The
 * netlist doesn't really need the array of parameters once elaboration
 * is complete, but this is the best place to store them.
 *
 * The first elaboration pass finds the reg objects that match the
 * port names, and creates the NetTaskDef object. The port names are
 * in the form task.port.
 *
 *      task foo;
 *        output blah;
 *        begin <body> end
 *      endtask
 *
 * So in the foo example, the PWire objects that represent the ports
 * of the task will include a foo.blah for the blah port. This port is
 * bound to a NetNet object by looking up the name. All of this is
 * handled by the PTask::elaborate_sig method and the results stashed
 * in the created NetTaskDef attached to the scope.
 *
 * Elaboration pass 2 for the task definition causes the statement of
 * the task to be elaborated and attached to the NetTaskDef object
 * created in pass 1.
 *
 * NOTE: I am not sure why I bothered to prepend the task name to the
 * port name when making the port list. It is not really useful, but
 * that is what I did in pform_make_task_ports, so there it is.
 */

void PTask::elaborate(Design*des, NetScope*task) const
{
      NetTaskDef*def = task->task_def();
      assert(def);

      NetProc*st;
      if (statement_ == 0) {
	    st = new NetBlock(NetBlock::SEQU, 0);

      } else {

	    st = statement_->elaborate(des, task);
	    if (st == 0) {
		  cerr << statement_->get_fileline() << ": Unable to elaborate "
			"statement in task " << scope_path(task)
		       << " at " << get_fileline() << "." << endl;
		  return;
	    }
      }

	// Handle any variable initialization statements in this scope.
	// For automatic tasks , these statements need to be executed
	// each time the task is called, so insert them at the start
	// of the elaborated definition. For static tasks, put them
	// in a separate process that will be executed before the start
	// of simulation.
      if (is_auto_) {
	      // Get the NetBlock of the statement. If it is not a
	      // NetBlock then create one to wrap the initialization
	      // statements and the original statement.
	    NetBlock*blk = dynamic_cast<NetBlock*> (st);
	    if ((blk == 0) && (var_inits.size() > 0)) {
		  blk = new NetBlock(NetBlock::SEQU, task);
		  blk->set_line(*this);
		  blk->append(st);
		  st = blk;
	    }
	    for (unsigned idx = var_inits.size(); idx > 0; idx -= 1) {
		  NetProc*tmp = var_inits[idx-1]->elaborate(des, task);
		  if (tmp) blk->prepend(tmp);
	    }
      } else {
	    elaborate_var_inits_(des, task);
      }

      def->set_proc(st);
}

NetProc* PTrigger::elaborate(Design*des, NetScope*scope) const
{
      assert(scope);

      NetScope*use_scope = scope;
      if (package_) {
	    use_scope = des->find_package(package_->pscope_name());
	    ivl_assert(*this, use_scope);
      }

      NetNet*       sig = 0;
      const NetExpr*par = 0;
      NetEvent*     eve = 0;

      NetScope*found_in = symbol_search(this, des, use_scope, event_,
					sig, par, eve);

      if (found_in == 0) {
	    cerr << get_fileline() << ": error: event <" << event_ << ">"
		 << " not found." << endl;
	    des->errors += 1;
	    return 0;
      }

      if (eve == 0) {
	    cerr << get_fileline() << ": error:  <" << event_ << ">"
		 << " is not a named event." << endl;
	    des->errors += 1;
	    return 0;
      }

      NetEvTrig*trig = new NetEvTrig(eve);
      trig->set_line(*this);
      return trig;
}

NetProc* PNBTrigger::elaborate(Design*des, NetScope*scope) const
{
      assert(scope);

      NetNet*       sig = 0;
      const NetExpr*par = 0;
      NetEvent*     eve = 0;

      NetScope*found_in = symbol_search(this, des, scope, event_,
					sig, par, eve);

      if (found_in == 0) {
	    cerr << get_fileline() << ": error: event <" << event_ << ">"
		 << " not found." << endl;
	    des->errors += 1;
	    return 0;
      }

      if (eve == 0) {
	    cerr << get_fileline() << ": error:  <" << event_ << ">"
		 << " is not a named event." << endl;
	    des->errors += 1;
	    return 0;
      }

      NetExpr*dly = 0;
      if (dly_) dly = elab_and_eval(des, scope, dly_, -1);
      NetEvNBTrig*trig = new NetEvNBTrig(eve, dly);
      trig->set_line(*this);
      return trig;
}

/*
 * The while loop is fairly directly represented in the netlist.
 */
NetProc* PWhile::elaborate(Design*des, NetScope*scope) const
{
      NetExpr*ce = elab_and_eval(des, scope, cond_, -1);
      NetProc*sub;
      if (statement_)
	    sub = statement_->elaborate(des, scope);
      else
	    sub = new NetBlock(NetBlock::SEQU, 0);
      if (ce == 0 || sub == 0) {
	    delete ce;
	    delete sub;
	    return 0;
      }
      NetWhile*loop = new NetWhile(ce, sub);
      loop->set_line(*this);
      return loop;
}

bool PProcess::elaborate(Design*des, NetScope*scope) const
{
      scope->in_final(type() == IVL_PR_FINAL);
      NetProc*cur = statement_->elaborate(des, scope);
      scope->in_final(false);
      if (cur == 0) {
	    return false;
      }

      NetProcTop*top=new NetProcTop(scope, type(), cur);
      ivl_assert(*this, top);

	// Evaluate the attributes for this process, if there
	// are any. These attributes are to be attached to the
	// NetProcTop object.
      struct attrib_list_t*attrib_list;
      unsigned attrib_list_n = 0;
      attrib_list = evaluate_attributes(attributes, attrib_list_n, des, scope);

      for (unsigned adx = 0 ;  adx < attrib_list_n ;  adx += 1)
	    top->attribute(attrib_list[adx].key,
			   attrib_list[adx].val);

      delete[]attrib_list;

      top->set_line(*this);
      des->add_process(top);

	/* Detect the special case that this is a combinational
	always block. We want to attach an _ivl_schedule_push
	attribute to this process so that it starts up and
	gets into its wait statement before non-combinational
	code is executed. */
      do {
	    if ((top->type() != IVL_PR_ALWAYS) &&
	        (top->type() != IVL_PR_ALWAYS_COMB) &&
	        (top->type() != IVL_PR_ALWAYS_FF) &&
	        (top->type() != IVL_PR_ALWAYS_LATCH))
		  break;

	    NetEvWait*st = dynamic_cast<NetEvWait*>(top->statement());
	    if (st == 0)
		  break;

	    if (st->nevents() != 1)
		  break;

	    NetEvent*ev = st->event(0);

	    if (ev->nprobe() == 0)
		  break;

	    bool anyedge_test = true;
	    for (unsigned idx = 0 ;  anyedge_test && (idx<ev->nprobe())
		       ; idx += 1) {
		  const NetEvProbe*pr = ev->probe(idx);
		  if (pr->edge() != NetEvProbe::ANYEDGE)
			anyedge_test = false;
	    }

	    if (! anyedge_test)
		  break;

	    top->attribute(perm_string::literal("_ivl_schedule_push"),
			   verinum(1));
      } while (0);

      return true;
}

void PSpecPath::elaborate(Design*des, NetScope*scope) const
{
      uint64_t delay_value[12];
      unsigned ndelays = 0;

	/* Do not elaborate specify delay paths if this feature is
	   turned off. */
      if (!gn_specify_blocks_flag) return;

      ivl_assert(*this, conditional || (condition==0));

      ndelays = delays.size();
      if (ndelays > 12) ndelays = 12;

      check_for_inconsistent_delays(scope);

	/* Elaborate the delay values themselves. Remember to scale
	   them for the timescale/precision of the scope. */
      for (unsigned idx = 0 ;  idx < ndelays ;  idx += 1) {
	    PExpr*exp = delays[idx];
	    NetExpr*cur = elab_and_eval(des, scope, exp, -1);

	    if (NetEConst*con = dynamic_cast<NetEConst*> (cur)) {
		  verinum fn = con->value();
		  delay_value[idx] = des->scale_to_precision(fn.as_ulong64(),
		                                             scope);

	    } else if (NetECReal*rcon = dynamic_cast<NetECReal*>(cur)) {
		  delay_value[idx] = get_scaled_time_from_real(des, scope,
		                                               rcon);

	    } else {
		  cerr << get_fileline() << ": error: Path delay value "
		       << "must be constant (" << *cur << ")." << endl;
		  delay_value[idx] = 0;
		  des->errors += 1;
	    }
	    delete cur;
      }

      switch (delays.size()) {
	  case 1:
	  case 2:
	  case 3:
	  case 6:
	  case 12:
	    break;
	  default:
	    cerr << get_fileline() << ": error: Incorrect delay configuration."
		 << " Given " << delays.size() << " delay expressions." << endl;
	    ndelays = 1;
	    des->errors += 1;
	    break;
      }

      NetNet*condit_sig = 0;
      if (conditional && condition) {

	    NetExpr*tmp = elab_and_eval(des, scope, condition, -1);
	    ivl_assert(*condition, tmp);

	      // FIXME: Look for constant expressions here?

	      // Get a net form.
	    condit_sig = tmp->synthesize(des, scope, tmp);
	    ivl_assert(*condition, condit_sig);
      }

	/* A parallel connection does not support more than a one to one
	   connection (source/destination). */
      if (! full_flag_ && ((src.size() != 1) || (dst.size() != 1))) {
	    /* To be compatible with NC-Verilog we allow a parallel connection
	     * with multiple sources/destinations if all the paths are only a
	     * single bit wide (a scalar or a one bit vector). */
	    bool all_single = true;
	    typedef std::vector<perm_string>::const_iterator str_vec_iter;
	    for (str_vec_iter cur = src.begin();
		 ( cur != src.end() && all_single); ++ cur) {
		  NetNet *psig = scope->find_signal(*cur);
		    /* We will report a missing signal as invalid later. For
		     * now assume it's a single bit. */
		  if (psig == 0) continue;
		  if (psig->vector_width() != 1) all_single = false;
	    }
	    for (str_vec_iter cur = dst.begin();
		 ( cur != dst.end() && all_single); ++ cur) {
		  NetNet *psig = scope->find_signal(*cur);
		    /* The same as above for source paths. */
		  if (psig == 0) continue;
		  if (psig->vector_width() != 1) all_single = false;
	    }

	    if (! all_single) {
		  cerr << get_fileline() << ": error: Parallel connections "
		          "only support one source/destination path found ("
		       << src.size() << "/" << dst.size() << ")." << endl;
		  des->errors += 1;
	    }
      }

	/* Create all the various paths from the path specifier. */
      typedef std::vector<perm_string>::const_iterator str_vector_iter;
      for (str_vector_iter cur = dst.begin()
		 ; cur != dst.end() ; ++ cur ) {

	    if (debug_elaborate) {
		  cerr << get_fileline() << ": debug: Path to " << (*cur);
		  if (condit_sig)
			cerr << " if " << condit_sig->name();
		  else if (conditional)
			cerr << " ifnone";
		  cerr << " from ";
	    }

	    NetNet*dst_sig = scope->find_signal(*cur);
	    if (dst_sig == 0) {
		  cerr << get_fileline() << ": error: No wire '"
		       << *cur << "' in this module." << endl;
		  des->errors += 1;
		  continue;
	    }

	    unsigned long dst_wid = dst_sig->vector_width();

	    if (dst_sig->port_type() != NetNet::POUTPUT
		&& dst_sig->port_type() != NetNet::PINOUT) {

		  cerr << get_fileline() << ": error: Path destination "
		       << *cur << " must be an output or inout port." << endl;
		  des->errors += 1;
	    }

	    NetDelaySrc*path = new NetDelaySrc(scope, scope->local_symbol(),
					       src.size(), condit_sig,
					       conditional, !full_flag_);
	    path->set_line(*this);

	      // The presence of the data_source_expression indicates
	      // that this is an edge sensitive path. If so, then set
	      // the edges. Note that edge==0 is BOTH edges.
	    if (data_source_expression) {
		  if (edge >= 0) path->set_posedge();
		  if (edge <= 0) path->set_negedge();
	    }

	    switch (ndelays) {
		case 12:
		  path->set_delays(delay_value[0],  delay_value[1],
				   delay_value[2],  delay_value[3],
				   delay_value[4],  delay_value[5],
				   delay_value[6],  delay_value[7],
				   delay_value[8],  delay_value[9],
				   delay_value[10], delay_value[11]);
		  break;
		case 6:
		  path->set_delays(delay_value[0], delay_value[1],
				   delay_value[2], delay_value[3],
				   delay_value[4], delay_value[5]);
		  break;
		case 3:
		  path->set_delays(delay_value[0], delay_value[1],
				   delay_value[2]);
		  break;
		case 2:
		  path->set_delays(delay_value[0], delay_value[1]);
		  break;
		case 1:
		  path->set_delays(delay_value[0]);
		  break;
	    }

	    unsigned idx = 0;
	    for (str_vector_iter cur_src = src.begin()
		       ; cur_src != src.end() ; ++ cur_src ) {
		  NetNet*src_sig = scope->find_signal(*cur_src);
		  if (src_sig == 0) {
			cerr << get_fileline() << ": error: No wire '"
			     << *cur_src << "' in this module." << endl;
			des->errors += 1;
			continue;
		  }

		  if (debug_elaborate) {
			if (cur_src != src.begin()) cerr << " and ";
			cerr << src_sig->name();
		  }

		  if ( (src_sig->port_type() != NetNet::PINPUT)
		    && (src_sig->port_type() != NetNet::PINOUT) ) {

			cerr << get_fileline() << ": error: Path source "
			     << *cur_src << " must be an input or inout port."
			     << endl;
			des->errors += 1;
		  }

		    // For a parallel connection the source and destination
		    // must be the same width.
		  if (! full_flag_) {
			unsigned long src_wid = src_sig->vector_width();
			if (src_wid != dst_wid) {
			      cerr << get_fileline() << ": error: For a "
			              "parallel connection the "
			              "source/destination width must match "
			              "found (" << src_wid << "/" << dst_wid
			           << ")." << endl;
			      des->errors += 1;
			}
		  }

		  connect(src_sig->pin(0), path->pin(idx));
		  idx += 1;
	    }
	    if (debug_elaborate) {
		  cerr << endl;
	    }

	    if (condit_sig)
		  connect(condit_sig->pin(0), path->pin(idx));

	    dst_sig->add_delay_path(path);
      }
}

static void elaborate_functions(Design*des, NetScope*scope,
				const map<perm_string,PFunction*>&funcs)
{
      typedef map<perm_string,PFunction*>::const_iterator mfunc_it_t;
      for (mfunc_it_t cur = funcs.begin()
		 ; cur != funcs.end() ; ++ cur ) {

	    hname_t use_name ( (*cur).first );
	    NetScope*fscope = scope->child(use_name);
	    assert(fscope);
	    (*cur).second->elaborate(des, fscope);
      }
}

static void elaborate_tasks(Design*des, NetScope*scope,
			    const map<perm_string,PTask*>&tasks)
{
      typedef map<perm_string,PTask*>::const_iterator mtask_it_t;
      for (mtask_it_t cur = tasks.begin()
		 ; cur != tasks.end() ; ++ cur ) {

	    hname_t use_name ( (*cur).first );
	    NetScope*tscope = scope->child(use_name);
	    assert(tscope);
	    (*cur).second->elaborate(des, tscope);
      }
}

static void elaborate_classes(Design*des, NetScope*scope,
			      const map<perm_string,PClass*>&classes)
{
      for (map<perm_string,PClass*>::const_iterator cur = classes.begin()
		 ; cur != classes.end() ; ++ cur) {
	    netclass_t*use_class = scope->find_class(des, cur->second->pscope_name());
	    use_class->elaborate(des, cur->second);

	    if (use_class->test_for_missing_initializers()) {
		  cerr << cur->second->get_fileline() << ": error: "
		       << "Const properties of class " << use_class->get_name()
		       << " are missing initialization." << endl;
		  des->errors += 1;
	    }
      }
}

bool PPackage::elaborate(Design*des, NetScope*scope) const
{
      bool result_flag = true;

	// Elaborate function methods, and...
      elaborate_functions(des, scope, funcs);

	// Elaborate task methods.
      elaborate_tasks(des, scope, tasks);

	// Elaborate class definitions.
      elaborate_classes(des, scope, classes);

	// Elaborate the variable initialization statements, making a
	// single initial process out of them.
      result_flag &= elaborate_var_inits_(des, scope);

      return result_flag;
}

/*
 * When a module is instantiated, it creates the scope then uses this
 * method to elaborate the contents of the module.
 */

bool Module::elaborate(Design*des, NetScope*scope) const
{
      bool result_flag = true;

	// Elaborate within the generate blocks.
      typedef list<PGenerate*>::const_iterator generate_it_t;
      for (generate_it_t cur = generate_schemes.begin()
		 ; cur != generate_schemes.end() ; ++ cur ) {
	    (*cur)->elaborate(des, scope);
      }

	// Elaborate functions.
      elaborate_functions(des, scope, funcs);

	// Elaborate the task definitions. This is done before the
	// behaviors so that task calls may reference these, and after
	// the signals so that the tasks can reference them.
      elaborate_tasks(des, scope, tasks);

	// Elaborate class definitions.
      elaborate_classes(des, scope, classes);

	// Get all the gates of the module and elaborate them by
	// connecting them to the signals. The gate may be simple or
	// complex.
      const list<PGate*>&gl = get_gates();

      for (list<PGate*>::const_iterator gt = gl.begin()
		 ; gt != gl.end() ; ++ gt ) {

	    (*gt)->elaborate(des, scope);
      }

	// Elaborate the variable initialization statements, making a
	// single initial process out of them.
      result_flag &= elaborate_var_inits_(des, scope);

	// Elaborate the behaviors, making processes out of them. This
	// involves scanning the PProcess* list, creating a NetProcTop
	// for each process.
      result_flag &= elaborate_behaviors_(des, scope);

	// Elaborate the specify paths of the module.

      for (list<PSpecPath*>::const_iterator sp = specify_paths.begin()
		 ; sp != specify_paths.end() ; ++ sp ) {

	    (*sp)->elaborate(des, scope);
      }

	// Elaborate the elaboration tasks.
      for (list<PCallTask*>::const_iterator et = elab_tasks.begin()
		 ; et != elab_tasks.end() ; ++ et ) {
	    result_flag &= (*et)->elaborate_elab(des, scope);
      }

      return result_flag;
}

/*
 * Elaborating a netclass_t means elaborating the PFunction and PTask
 * objects that it contains. The scopes and signals have already been
 * elaborated in the class of the netclass_t scope, so we can get the
 * child scope for each definition and use that for the context of the
 * function.
 */
void netclass_t::elaborate(Design*des, PClass*pclass)
{
      if (! pclass->type->initialize_static.empty()) {
	    std::vector<Statement*>&stmt_list = pclass->type->initialize_static;
	    NetBlock*stmt = new NetBlock(NetBlock::SEQU, 0);
	    for (size_t idx = 0 ; idx < stmt_list.size() ; idx += 1) {
		  NetProc*tmp = stmt_list[idx]->elaborate(des, class_scope_);
		  if (tmp == 0) continue;
		  stmt->append(tmp);
	    }
	    NetProcTop*top = new NetProcTop(class_scope_, IVL_PR_INITIAL, stmt);
	    top->set_line(*pclass);
	    des->add_process(top);
      }

      for (map<perm_string,PFunction*>::iterator cur = pclass->funcs.begin()
		 ; cur != pclass->funcs.end() ; ++ cur) {
	    if (debug_elaborate) {
		  cerr << cur->second->get_fileline() << ": netclass_t::elaborate: "
		       << "Elaborate class " << scope_path(class_scope_)
		       << " function method " << cur->first << endl;
	    }

	    NetScope*scope = class_scope_->child( hname_t(cur->first) );
	    ivl_assert(*cur->second, scope);
	    cur->second->elaborate(des, scope);
      }

      for (map<perm_string,PTask*>::iterator cur = pclass->tasks.begin()
		 ; cur != pclass->tasks.end() ; ++ cur) {
	    if (debug_elaborate) {
		  cerr << cur->second->get_fileline() << ": netclass_t::elaborate: "
		       << "Elaborate class " << scope_path(class_scope_)
		       << " task method " << cur->first << endl;
	    }

	    NetScope*scope = class_scope_->child( hname_t(cur->first) );
	    ivl_assert(*cur->second, scope);
	    cur->second->elaborate(des, scope);
      }
}

bool PGenerate::elaborate(Design*des, NetScope*container) const
{
      if (direct_nested_)
	    return elaborate_direct_(des, container);

      bool flag = true;

      if (debug_elaborate) {
	    cerr << get_fileline() << ": PGenerate::elaborate: "
		  "generate " << scheme_type
		 << " elaborating in scope " << scope_path(container)
		 << "." << endl;
	    cerr << get_fileline() << ": PGenerate::elaborate: "
		  "generate scope_name=" << scope_name
		 << ", id_number=" << id_number << endl;
      }

	// Handle the special case that this is a CASE scheme. In this
	// case the PGenerate itself does not have the generated
	// item. Look instead for the case ITEM that has a scope
	// generated for it.
      if (scheme_type == PGenerate::GS_CASE) {

	    typedef list<PGenerate*>::const_iterator generate_it_t;
	    for (generate_it_t cur = generate_schemes.begin()
		       ; cur != generate_schemes.end() ; ++ cur ) {
		  PGenerate*item = *cur;
		  if (item->direct_nested_ || !item->scope_list_.empty()) {
			flag &= item->elaborate(des, container);
		  }
	    }
	    return flag;
      }

      typedef list<NetScope*>::const_iterator scope_list_it_t;
      for (scope_list_it_t cur = scope_list_.begin()
		 ; cur != scope_list_.end() ; ++ cur ) {

	    NetScope*scope = *cur;
	      // Check that this scope is one that is contained in the
	      // container that the caller passed in.
	    if (scope->parent() != container)
		  continue;

	      // If this was an unnamed generate block, replace its
	      // temporary name with a name generated using the naming
	      // scheme defined in the Verilog-2005 standard.
	    const char*name = scope_name.str();
	    if (name[0] == '$') {

		  if (!scope->auto_name("genblk", '0', name + 4)) {
			cerr << get_fileline() << ": warning: Couldn't build"
			     << " unique name for unnamed generate block"
			     << " - using internal name " << name << endl;
		  }
	    }
	    if (debug_elaborate)
		  cerr << get_fileline() << ": debug: Elaborate in "
		       << "scope " << scope_path(scope) << endl;

	    flag = elaborate_(des, scope) & flag;
      }

      return flag;
}

bool PGenerate::elaborate_direct_(Design*des, NetScope*container) const
{
      bool flag = true;

      if (debug_elaborate) {
	    cerr << get_fileline() << ": debug: "
		 << "Direct nesting elaborate in scope "
		 << scope_path(container)
		 << ", scheme_type=" << scheme_type << endl;
      }

	// Elaborate for a direct nested generated scheme knows
	// that there are only sub_schemes to be elaborated.  There
	// should be exactly 1 active generate scheme, search for it
	// using this loop.
      typedef list<PGenerate*>::const_iterator generate_it_t;
      for (generate_it_t cur = generate_schemes.begin()
		 ; cur != generate_schemes.end() ; ++ cur ) {
	    PGenerate*item = *cur;
	    if (debug_elaborate) {
		  cerr << get_fileline() << ": PGenerate::elaborate_direct_: "
		       << "item->scope_name=" << item->scope_name
		       << ", item->scheme_type=" << item->scheme_type
		       << ", item->direct_nested_=" << item->direct_nested_
		       << ", item->scope_list_.size()=" << item->scope_list_.size()
		       << "." << endl;
	    }

	      // Special case: If this is a case generate scheme, then
	      // the PGenerate object (item) does not actually
	      // contain anything. Instead scan the case items, which
	      // are listed as sub-schemes of the item.
	    if (item->scheme_type == PGenerate::GS_CASE) {
		  for (generate_it_t icur = item->generate_schemes.begin()
			     ; icur != item->generate_schemes.end() ; ++ icur ) {
			PGenerate*case_item = *icur;
			if (case_item->direct_nested_ || !case_item->scope_list_.empty()) {
			      flag &= case_item->elaborate(des, container);
			}
		  }
	    } else {
		  if (item->direct_nested_ || !item->scope_list_.empty()) {
			  // Found the item, and it is direct nested.
			flag &= item->elaborate(des, container);
		  }
	    }
      }
      return flag;
}

bool PGenerate::elaborate_(Design*des, NetScope*scope) const
{
      elaborate_functions(des, scope, funcs);
      elaborate_tasks(des, scope, tasks);

      typedef list<PGate*>::const_iterator gates_it_t;
      for (gates_it_t cur = gates.begin() ; cur != gates.end() ; ++ cur )
	    (*cur)->elaborate(des, scope);

      elaborate_var_inits_(des, scope);

      typedef list<PProcess*>::const_iterator proc_it_t;
      for (proc_it_t cur = behaviors.begin(); cur != behaviors.end(); ++ cur )
	    (*cur)->elaborate(des, scope);

      typedef list<PGenerate*>::const_iterator generate_it_t;
      for (generate_it_t cur = generate_schemes.begin()
		 ; cur != generate_schemes.end() ; ++ cur ) {
	    (*cur)->elaborate(des, scope);
      }

      return true;
}

bool PScope::elaborate_behaviors_(Design*des, NetScope*scope) const
{
      bool result_flag = true;

	// Elaborate the behaviors, making processes out of them. This
	// involves scanning the PProcess* list, creating a NetProcTop
	// for each process.
      for (list<PProcess*>::const_iterator st = behaviors.begin()
		 ; st != behaviors.end() ; ++ st ) {

	    result_flag &= (*st)->elaborate(des, scope);
      }

      for (list<AProcess*>::const_iterator st = analog_behaviors.begin()
		 ; st != analog_behaviors.end() ; ++ st ) {

	    result_flag &= (*st)->elaborate(des, scope);
      }

      return result_flag;
}

bool LexicalScope::elaborate_var_inits_(Design*des, NetScope*scope) const
{
      if (var_inits.size() == 0)
	    return true;

      NetProc*proc = 0;
      if (var_inits.size() == 1) {
	    proc = var_inits[0]->elaborate(des, scope);
      } else {
	    NetBlock*blk = new NetBlock(NetBlock::SEQU, 0);
	    bool flag = true;
	    for (unsigned idx = 0; idx < var_inits.size(); idx += 1) {
		  NetProc*tmp = var_inits[idx]->elaborate(des, scope);
		  if (tmp)
			blk->append(tmp);
		  else
			flag = false;
	    }
	    if (flag) proc = blk;
      }
      if (proc == 0)
	    return false;

      NetProcTop*top = new NetProcTop(scope, IVL_PR_INITIAL, proc);
      if (const LineInfo*li = dynamic_cast<const LineInfo*>(this)) {
	    top->set_line(*li);
      }
      if (gn_system_verilog()) {
	    top->attribute(perm_string::literal("_ivl_schedule_init"),
			   verinum(1));
      }
      des->add_process(top);

      scope->set_var_init(proc);

      return true;
}

class elaborate_package_t : public elaborator_work_item_t {
    public:
      elaborate_package_t(Design*d, NetScope*scope, PPackage*p)
      : elaborator_work_item_t(d), scope_(scope), package_(p)
      { }

      ~elaborate_package_t() { }

      virtual void elaborate_runrun()
      {
	    if (! package_->elaborate_scope(des, scope_))
		  des->errors += 1;
      }

    private:
      NetScope*scope_;
      PPackage*package_;
};

class elaborate_root_scope_t : public elaborator_work_item_t {
    public:
      elaborate_root_scope_t(Design*des__, NetScope*scope, Module*rmod)
      : elaborator_work_item_t(des__), scope_(scope), rmod_(rmod)
      { }

      ~elaborate_root_scope_t() { }

      virtual void elaborate_runrun()
      {
	    Module::replace_t root_repl;
	    for (list<Module::named_expr_t>::iterator cur = Module::user_defparms.begin()
		       ; cur != Module::user_defparms.end() ; ++ cur ) {

		  pform_name_t tmp_name = cur->first;
		  if (peek_head_name(tmp_name) != scope_->basename())
			continue;

		  tmp_name.pop_front();
		  if (tmp_name.size() != 1)
			continue;

		  root_repl[peek_head_name(tmp_name)] = cur->second;
	    }

	    if (! rmod_->elaborate_scope(des, scope_, root_repl))
		  des->errors += 1;
      }

    private:
      NetScope*scope_;
      Module*rmod_;
};

class top_defparams : public elaborator_work_item_t {
    public:
      explicit top_defparams(Design*des__)
      : elaborator_work_item_t(des__)
      { }

      ~top_defparams() { }

      virtual void elaborate_runrun()
      {
	    if (debug_scopes) {
		  cerr << "debug: top_defparams::elaborate_runrun()" << endl;
	    }
	      // This method recurses through the scopes, looking for
	      // defparam assignments to apply to the parameters in the
	      // various scopes. This needs to be done after all the scopes
	      // and basic parameters are taken care of because the defparam
	      // can assign to a parameter declared *after* it.
	    des->run_defparams();

	      // At this point, all parameter overrides are done. Scan the
	      // scopes and evaluate the parameters all the way down to
	      // constants.
	    des->evaluate_parameters();

	    if (debug_scopes) {
		  cerr << "debug: top_defparams::elaborate_runrun() done" << endl;
	    }
      }
};

class later_defparams : public elaborator_work_item_t {
    public:
      explicit later_defparams(Design*des__)
      : elaborator_work_item_t(des__)
      { }

      ~later_defparams() { }

      virtual void elaborate_runrun()
      {
	    if (debug_scopes) {
		  cerr << "debug: later_defparams::elaborate_runrun()" << endl;
	    }

	    list<NetScope*>tmp_list;
	    for (set<NetScope*>::iterator cur = des->defparams_later.begin()
		       ; cur != des->defparams_later.end() ; ++ cur )
		  tmp_list.push_back(*cur);

	    des->defparams_later.clear();

	    while (! tmp_list.empty()) {
		  NetScope*cur = tmp_list.front();
		  tmp_list.pop_front();
		  cur->run_defparams_later(des);
	    }

	      // The overridden parameters will be evaluated later in
	      // a top_defparams work item.

	    if (debug_scopes) {
		  cerr << "debuf: later_defparams::elaborate_runrun() done" << endl;
	    }
      }
};

static ostream& operator<< (ostream&o, ivl_process_type_t t)
{
      switch (t) {
	case IVL_PR_ALWAYS:
	    o << "always";
	    break;
	case IVL_PR_ALWAYS_COMB:
	    o << "always_comb";
	    break;
	case IVL_PR_ALWAYS_FF:
	    o << "always_ff";
	    break;
	case IVL_PR_ALWAYS_LATCH:
	    o << "always_latch";
	    break;
	case IVL_PR_INITIAL:
	    o << "initial";
	    break;
	case IVL_PR_FINAL:
	    o << "final";
	    break;
      }
      return o;
}

bool Design::check_proc_delay() const
{
      bool result = false;

      for (const NetProcTop*pr = procs_ ;  pr ;  pr = pr->next_) {
	      /* If this is an always process and we have no or zero delay then
	       * a runtime infinite loop will happen. If we possibly have some
	       * delay then print a warning that an infinite loop is possible.
	       */
	    if (pr->type() == IVL_PR_ALWAYS) {
		  DelayType dly_type = pr->statement()->delay_type();

		  if (dly_type == NO_DELAY || dly_type == ZERO_DELAY) {
			cerr << pr->get_fileline() << ": error: always "
			        "process does not have any delay." << endl;
			cerr << pr->get_fileline() << ":      : A runtime "
			        "infinite loop will occur." << endl;
			result = true;

		  } else if (dly_type == POSSIBLE_DELAY && warn_inf_loop) {
			cerr << pr->get_fileline() << ": warning: always "
			        "process may not have any delay." << endl;
			cerr << pr->get_fileline() << ":        : A runtime "
			     << "infinite loop may be possible." << endl;
		  }
	    }

	      // The always_comb/ff/latch processes have special delay rules
	      // that need to be checked.
	    if ((pr->type() == IVL_PR_ALWAYS_COMB) ||
	        (pr->type() == IVL_PR_ALWAYS_FF) ||
	        (pr->type() == IVL_PR_ALWAYS_LATCH)) {
		  const NetEvWait *wait = dynamic_cast<const NetEvWait*> (pr->statement());
		  if (! wait) {
			  // The always_comb/latch processes have an event
			  // control added automatically by the compiler.
			assert(pr->type() == IVL_PR_ALWAYS_FF);
			cerr << pr->get_fileline() << ": error: the first "
			        "statement of an always_ff process must be "
			        "an event control statement." << endl;
			result = true;
		  } else if (wait->statement()->delay_type(true) != NO_DELAY) {
			cerr << pr->get_fileline() << ": error: there must ";

			if (pr->type() == IVL_PR_ALWAYS_FF) {
			      cerr << "only be a single event control and "
			              "no blocking delays in an always_ff "
			              "process.";
			} else {
			      cerr << "be no event controls or blocking "
			              "delays in an " << pr->type()
			           << " process.";
			}
			cerr << endl;
			result = true;
		  }

		  if ((pr->type() != IVL_PR_ALWAYS_FF) &&
		      (wait->nevents() == 0)) {
			if (pr->type() == IVL_PR_ALWAYS_LATCH) {
			      cerr << pr->get_fileline() << ": error: "
			              "always_latch process has no event "
			              "control." << endl;
			      result = true;
			} else {
			      assert(pr->type() == IVL_PR_ALWAYS_COMB);
			      cerr << pr->get_fileline() << ": warning: "
			              "always_comb process has no "
			              "sensitivities." << endl;
			}
		  }
	    }

	        /* If this is a final block it must not have a delay,
		   but this should have been caught by the statement
		   elaboration, so maybe this should be an internal
		   error? */
	    if (pr->type() == IVL_PR_FINAL) {
		  DelayType dly_type = pr->statement()->delay_type();

		  if (dly_type != NO_DELAY) {
			cerr << pr->get_fileline() << ": error: final"
			     << " statement contains a delay." << endl;
			result = true;
		  }
	    }
      }

      return result;
}

static void print_nexus_name(const Nexus*nex)
{
      for (const Link*cur = nex->first_nlink(); cur; cur = cur->next_nlink()) {
	    if (cur->get_dir() != Link::OUTPUT) continue;
	    const NetPins*obj = cur->get_obj();
	      // For a NetNet (signal) just use the name.
	    if (const NetNet*net = dynamic_cast<const NetNet*>(obj)) {
		  cerr << net->name();
		  return;
	      // For a NetPartSelect calculate the name.
	    } else if (const NetPartSelect*ps = dynamic_cast<const NetPartSelect*>(obj)) {
		  assert(ps->pin_count() >= 2);
		  assert(ps->pin(1).get_dir() == Link::INPUT);
		  assert(ps->pin(1).is_linked());
		  print_nexus_name(ps->pin(1).nexus());
		  cerr << "[]";
		  return;
	      // For a NetUReduce calculate the name.
	    } else if (const NetUReduce*reduce = dynamic_cast<const NetUReduce*>(obj)) {
		  assert(reduce->pin_count() == 2);
		  assert(reduce->pin(1).get_dir() == Link::INPUT);
		  assert(reduce->pin(1).is_linked());
		  switch (reduce->type()) {
		    case NetUReduce::AND:
			cerr << "&";
			break;
		    case NetUReduce::OR:
			cerr << "|";
			break;
		    case NetUReduce::XOR:
			cerr << "^";
			break;
		    case NetUReduce::NAND:
			cerr << "~&";
			break;
		    case NetUReduce::NOR:
			cerr << "~|";
			break;
		    case NetUReduce::XNOR:
			cerr << "~^";
			break;
		    case NetUReduce::NONE:
			assert(0);
		  }
		  print_nexus_name(reduce->pin(1).nexus());
		  return;
	    } else if (const NetLogic*logic = dynamic_cast<const NetLogic*>(obj)) {
		  assert(logic->pin_count() >= 2);
		  assert(logic->pin(1).get_dir() == Link::INPUT);
		  assert(logic->pin(1).is_linked());
		  switch (logic->type()) {
		    case NetLogic::NOT:
			cerr << "~";
			break;
		    default:
			  // The other operators should never be used here,
			  // so just return the nexus name.
			cerr << nex->name();
			return;
		  }
		  print_nexus_name(logic->pin(1).nexus());
		  return;
	    }
	// Use the following to find the type of anything that may be missing:
	//    cerr << "(" << typeid(*obj).name() << ") ";
      }
	// Otherwise just use the nexus name so somthing is printed.
      cerr << nex->name();
}

static void print_event_probe_name(const NetEvProbe *prb)
{
      assert(prb->pin_count() == 1);
      assert(prb->pin(0).get_dir() == Link::INPUT);
      assert(prb->pin(0).is_linked());
      print_nexus_name(prb->pin(0).nexus());
}

static void check_event_probe_width(const LineInfo *info, const NetEvProbe *prb)
{
      assert(prb->pin_count() == 1);
      assert(prb->pin(0).get_dir() == Link::INPUT);
      assert(prb->pin(0).is_linked());
      if (prb->edge() == NetEvProbe::ANYEDGE) return;
      if (prb->pin(0).nexus()->vector_width() > 1) {
	    cerr << info->get_fileline() << " warning: Synthesis wants "
                    "the sensitivity list expressions for '";
	    switch (prb->edge()) {
	      case NetEvProbe::POSEDGE:
		  cerr << "posedge ";
		  break;
	      case NetEvProbe::NEGEDGE:
		  cerr << "negedge ";
		  break;
	      default:
		  break;
	    }
	    print_nexus_name(prb->pin(0).nexus());
	    cerr << "' to be a single bit." << endl;
      }
}

static void check_ff_sensitivity(const NetProc* statement)
{
      const NetEvWait *evwt = dynamic_cast<const NetEvWait*> (statement);
	// We have already checked for and reported if the first statmemnt is
	// not a wait.
      if (! evwt) return;

      for (unsigned cevt = 0; cevt < evwt->nevents(); cevt += 1) {
	    const NetEvent *evt = evwt->event(cevt);
	    for (unsigned cprb = 0; cprb < evt->nprobe(); cprb += 1) {
		  const NetEvProbe *prb = evt->probe(cprb);
		  check_event_probe_width(evwt, prb);
		  if (prb->edge() == NetEvProbe::ANYEDGE) {
			cerr << evwt->get_fileline() << " warning: Synthesis "
			        "requires the sensitivity list of an "
			        "always_ff process to only be edge "
			        "sensitive. ";
			print_event_probe_name(prb);
			cerr  << " is missing a pos/negedge." << endl;
		  }
	    }
      }
}

/*
 * Check to see if the always_* processes only contain synthesizable
 * constructs.
 */
bool Design::check_proc_synth() const
{
      bool result = false;
      for (const NetProcTop*pr = procs_ ;  pr ;  pr = pr->next_) {
	    if ((pr->type() == IVL_PR_ALWAYS_COMB) ||
	        (pr->type() == IVL_PR_ALWAYS_FF) ||
	        (pr->type() == IVL_PR_ALWAYS_LATCH)) {
		  result |= pr->statement()->check_synth(pr->type(),
		                                         pr->scope());
		  if (pr->type() == IVL_PR_ALWAYS_FF) {
			check_ff_sensitivity(pr->statement());
		  }
	    }
      }
      return result;
}

/*
 * Check whether all design elements have an explicit timescale or all
 * design elements use the default timescale. If a mixture of explicit
 * and default timescales is found, a warning message is output. Note
 * that we only need to check the top level design elements - nested
 * design elements will always inherit the timescale from their parent
 * if they don't have any local timescale declarations.
 *
 * NOTE: Strictly speaking, this should be an error for SystemVerilog
 * (1800-2012 section 3.14.2).
 */
static void check_timescales(bool&some_explicit, bool&some_implicit,
			     const PScope*scope)
{
      if (scope->time_unit_is_default)
	    some_implicit = true;
      else
	    some_explicit = true;
      if (scope->time_prec_is_default)
	    some_implicit = true;
      else
	    some_explicit = true;
}

static void check_timescales()
{
      bool some_explicit = false;
      bool some_implicit = false;
      map<perm_string,Module*>::iterator mod;
      for (mod = pform_modules.begin(); mod != pform_modules.end(); ++mod) {
	    const Module*mp = (*mod).second;
	    check_timescales(some_explicit, some_implicit, mp);
	    if (some_explicit && some_implicit)
		  break;
      }
      vector<PPackage*>::iterator pkg;
      if (gn_system_verilog() && !(some_explicit && some_implicit)) {
	    for (pkg = pform_packages.begin(); pkg != pform_packages.end(); ++pkg) {
		  const PPackage*pp = *pkg;
		  check_timescales(some_explicit, some_implicit, pp);
		  if (some_explicit && some_implicit)
			break;
	    }
      }
      if (gn_system_verilog() && !(some_explicit && some_implicit)) {
	    for (pkg = pform_units.begin(); pkg != pform_units.end(); ++pkg) {
		  const PPackage*pp = *pkg;
		    // We don't need a timescale if the compilation unit
		    // contains no items outside a design element.
		  if (pp->parameters.empty() &&
		      pp->localparams.empty() &&
		      pp->wires.empty() &&
		      pp->tasks.empty() &&
		      pp->funcs.empty() &&
		      pp->classes.empty())
			continue;

		  check_timescales(some_explicit, some_implicit, pp);
		  if (some_explicit && some_implicit)
			break;
	    }
      }

      if (!(some_explicit && some_implicit))
	    return;

      if (gn_system_verilog()) {
	    cerr << "warning: "
		 << "Some design elements have no explicit time unit and/or"
		 << endl;
	    cerr << "       : "
		 << "time precision. This may cause confusing timing results."
		 << endl;
	    cerr << "       : "
		 << "Affected design elements are:"
		 << endl;
      } else {
	    cerr << "warning: "
		 << "Some modules have no timescale. This may cause"
		 << endl;
	    cerr << "       : "
		 << "confusing timing results.	Affected modules are:"
		 << endl;
      }

      for (mod = pform_modules.begin(); mod != pform_modules.end(); ++mod) {
	    Module*mp = (*mod).second;
	    if (mp->has_explicit_timescale())
		  continue;
	    cerr << "       :   -- module " << (*mod).first
		 << " declared here: " << mp->get_fileline() << endl;
      }

      if (!gn_system_verilog())
	    return;

      for (pkg = pform_packages.begin(); pkg != pform_packages.end(); ++pkg) {
	    PPackage*pp = *pkg;
	    if (pp->has_explicit_timescale())
		  continue;
	    cerr << "       :   -- package " << pp->pscope_name()
		 << " declared here: " << pp->get_fileline() << endl;
      }

      for (pkg = pform_units.begin(); pkg != pform_units.end(); ++pkg) {
	    PPackage*pp = *pkg;
	    if (pp->has_explicit_timescale())
		  continue;

	    if (pp->parameters.empty() &&
		pp->localparams.empty() &&
		pp->wires.empty() &&
		pp->tasks.empty() &&
		pp->funcs.empty() &&
		pp->classes.empty())
		  continue;

	    cerr << "       :   -- compilation unit";
	    if (pform_units.size() > 1) {
		  cerr << " from: " << pp->get_file();
	    }
	    cerr << endl;
      }
}

/*
 * This function is the root of all elaboration. The input is the list
 * of root module names. The function locates the Module definitions
 * for each root, does the whole elaboration sequence, and fills in
 * the resulting Design.
 */

struct pack_elem {
      PPackage*pack;
      NetScope*scope;
};

struct root_elem {
      Module *mod;
      NetScope *scope;
};

Design* elaborate(list<perm_string>roots)
{
      unsigned npackages = pform_packages.size();
      if (gn_system_verilog())
	    npackages += pform_units.size();

      vector<struct root_elem> root_elems(roots.size());
      vector<struct pack_elem> pack_elems(npackages);
      map<LexicalScope*,NetScope*> unit_scopes;
      bool rc = true;
      unsigned i = 0;

	// This is the output design. I fill it in as I scan the root
	// module and elaborate what I find.
      Design*des = new Design;

	// Elaborate the compilation unit scopes. From here on, these are
	// treated as an additional set of packages.
      if (gn_system_verilog()) {
	    for (vector<PPackage*>::iterator pkg = pform_units.begin()
		       ; pkg != pform_units.end() ; ++pkg) {
		  PPackage*unit = *pkg;
		  NetScope*scope = des->make_package_scope(unit->pscope_name(), 0, true);
		  scope->set_line(unit);
		  scope->add_imports(&unit->explicit_imports);
		  set_scope_timescale(des, scope, unit);

		  elaborator_work_item_t*es = new elaborate_package_t(des, scope, unit);
		  des->elaboration_work_list.push_back(es);

		  pack_elems[i].pack = unit;
		  pack_elems[i].scope = scope;
		  i += 1;

		  unit_scopes[unit] = scope;
	    }
      }

	// Elaborate the packages. Package elaboration is simpler
	// because there are fewer sub-scopes involved. Note that
	// in SystemVerilog, packages are not allowed to refer to
	// the compilation unit scope, but the VHDL preprocessor
	// assumes they can.
      for (vector<PPackage*>::iterator pkg = pform_packages.begin()
		 ; pkg != pform_packages.end() ; ++pkg) {
	    PPackage*pack = *pkg;
	    NetScope*unit_scope = unit_scopes[pack->parent_scope()];
	    NetScope*scope = des->make_package_scope(pack->pscope_name(), unit_scope, false);
	    scope->set_line(pack);
	    scope->add_imports(&pack->explicit_imports);
	    set_scope_timescale(des, scope, pack);

	    elaborator_work_item_t*es = new elaborate_package_t(des, scope, pack);
	    des->elaboration_work_list.push_back(es);

	    pack_elems[i].pack = pack;
	    pack_elems[i].scope = scope;
	    i += 1;
      }

	// Scan the root modules by name, and elaborate their scopes.
      i = 0;
      for (list<perm_string>::const_iterator root = roots.begin()
		 ; root != roots.end() ; ++ root ) {

	      // Look for the root module in the list.
	    map<perm_string,Module*>::const_iterator mod = pform_modules.find(*root);
	    if (mod == pform_modules.end()) {
		  cerr << "error: Unable to find the root module \""
		       << (*root) << "\" in the Verilog source." << endl;
		  cerr << "     : Perhaps ``-s " << (*root)
		       << "'' is incorrect?" << endl;
		  des->errors++;
		  continue;
	    }

	      // Get the module definition for this root instance.
	    Module *rmod = (*mod).second;

	      // Get the compilation unit scope for this module.
	    NetScope*unit_scope = unit_scopes[rmod->parent_scope()];

	      // Make the root scope. This makes a NetScope object and
	      // pushes it into the list of root scopes in the Design.
	    NetScope*scope = des->make_root_scope(*root, unit_scope,
						  rmod->program_block,
						  rmod->is_interface);

	      // Collect some basic properties of this scope from the
	      // Module definition.
	    scope->set_line(rmod);
	    scope->add_imports(&rmod->explicit_imports);
	    set_scope_timescale(des, scope, rmod);

	      // Save this scope, along with its definition, in the
	      // "root_elems" list for later passes.
	    root_elems[i].mod = rmod;
	    root_elems[i].scope = scope;
	    i += 1;

	      // Arrange for these scopes to be elaborated as root
	      // scopes. Create an "elaborate_root_scope" object to
	      // contain the work item, and append it to the scope
	      // elaborations work list.
	    elaborator_work_item_t*es = new elaborate_root_scope_t(des, scope, rmod);
	    des->elaboration_work_list.push_back(es);
      }

	// Run the work list of scope elaborations until the list is
	// empty. This list is initially populated above where the
	// initial root scopes are primed.
      while (! des->elaboration_work_list.empty()) {
	      // Push a work item to process the defparams of any scopes
	      // that are elaborated during this pass. For the first pass
	      // this will be all the root scopes. For subsequent passes
	      // it will be any scopes created during the previous pass
	      // by a generate construct or instance array.
	    des->elaboration_work_list.push_back(new top_defparams(des));

	      // Transfer the queue to a temporary queue.
	    list<elaborator_work_item_t*> cur_queue;
	    while (! des->elaboration_work_list.empty()) {
		  cur_queue.push_back(des->elaboration_work_list.front());
		  des->elaboration_work_list.pop_front();
	    }

	      // Run from the temporary queue. If the temporary queue
	      // items create new work queue items, they will show up
	      // in the elaboration_work_list and then we get to run
	      // through them in the next pass.
	    while (! cur_queue.empty()) {
		  elaborator_work_item_t*tmp = cur_queue.front();
		  cur_queue.pop_front();
		  tmp->elaborate_runrun();
		  delete tmp;
	    }

	    if (! des->elaboration_work_list.empty()) {
		  des->elaboration_work_list.push_back(new later_defparams(des));
	    }
      }

      if (debug_elaborate) {
	    cerr << "<toplevel>: elaborate: "
		 << "elaboration work list done. Start processing residual defparams." << endl;
      }

	// Look for residual defparams (that point to a non-existent
	// scope) and clean them out.
      des->residual_defparams();

	// Errors already? Probably missing root modules. Just give up
	// now and return nothing.
      if (des->errors > 0)
	    return des;

	// Now we have the full design, check for timescale inconsistencies.
      if (warn_timescale)
	    check_timescales();

      if (debug_elaborate) {
	    cerr << "<toplevel>: elaborate: "
		 << "Start calling Package elaborate_sig methods." << endl;
      }

	// With the parameters evaluated down to constants, we have
	// what we need to elaborate signals and memories. This pass
	// creates all the NetNet and NetMemory objects for declared
	// objects.
      for (i = 0; i < pack_elems.size(); i += 1) {
	    PPackage*pack = pack_elems[i].pack;
	    NetScope*scope= pack_elems[i].scope;

	    if (! pack->elaborate_sig(des, scope)) {
		  if (debug_elaborate) {
			cerr << "<toplevel>" << ": debug: " << pack->pscope_name()
			     << ": elaborate_sig failed!!!" << endl;
		  }
		  delete des;
		  return 0;
	    }
      }

      if (debug_elaborate) {
	    cerr << "<toplevel>: elaborate: "
		 << "Start calling $root elaborate_sig methods." << endl;
      }

      if (debug_elaborate) {
	    cerr << "<toplevel>: elaborate: "
		 << "Start calling root module elaborate_sig methods." << endl;
      }

      for (i = 0; i < root_elems.size(); i++) {
	    Module *rmod = root_elems[i].mod;
	    NetScope *scope = root_elems[i].scope;
	    scope->set_num_ports( rmod->port_count() );

	    if (debug_elaborate) {
		  cerr << "<toplevel>" << ": debug: " << rmod->mod_name()
		       << ": port elaboration root "
		       << rmod->port_count() << " ports" << endl;
	    }

	    if (! rmod->elaborate_sig(des, scope)) {
		  if (debug_elaborate) {
			cerr << "<toplevel>" << ": debug: " << rmod->mod_name()
			     << ": elaborate_sig failed!!!" << endl;
		  }
		  delete des;
		  return 0;
	    }

	      // Some of the generators need to have the ports correctly
	      // defined for the root modules. This code does that.
	    for (unsigned idx = 0; idx < rmod->port_count(); idx += 1) {
		  vector<PEIdent*> mport = rmod->get_port(idx);
                  unsigned int prt_vector_width = 0;
                  PortType::Enum ptype = PortType::PIMPLICIT;
		  for (unsigned pin = 0; pin < mport.size(); pin += 1) {
			  // This really does more than we need and adds extra
			  // stuff to the design that should be cleaned later.
			NetNet *netnet = mport[pin]->elaborate_subport(des, scope);
			if (netnet != 0) {
			  // Elaboration may actually fail with
			  // erroneous input source
                          prt_vector_width += netnet->vector_width() * netnet->pin_count();
                          ptype = PortType::merged(netnet->port_type(), ptype);
			}
		  }
                  if (debug_elaborate) {
                           cerr << "<toplevel>" << ": debug: " << rmod->mod_name()
                                << ": adding module port "
                                << rmod->get_port_name(idx) << endl;
                     }
		  scope->add_module_port_info(idx, rmod->get_port_name(idx), ptype, prt_vector_width );
	    }
      }

	// Now that the structure and parameters are taken care of,
	// run through the pform again and generate the full netlist.

      for (i = 0; i < pack_elems.size(); i += 1) {
	    PPackage*pkg = pack_elems[i].pack;
	    NetScope*scope = pack_elems[i].scope;
	    rc &= pkg->elaborate(des, scope);
      }

      for (i = 0; i < root_elems.size(); i++) {
	    Module *rmod = root_elems[i].mod;
	    NetScope *scope = root_elems[i].scope;
	    rc &= rmod->elaborate(des, scope);
      }

      if (rc == false) {
	    delete des;
	    return 0;
      }

	// Now that everything is fully elaborated verify that we do
	// not have an always block with no delay (an infinite loop),
        // or a final block with a delay.
      bool has_failure = des->check_proc_delay();

	// Check to see if the always_comb/ff/latch processes only have
	// synthesizable constructs
      has_failure |= des->check_proc_synth();

      if (debug_elaborate) {
               cerr << "<toplevel>" << ": debug: "
                    << " finishing with "
                    <<  des->find_root_scopes().size() << " root scopes " << endl;
         }

      if (has_failure) {
	    delete des;
	    des = 0;
      }

      return des;
}
