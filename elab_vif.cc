/*
 * Copyright (c) 2026 Icarus UVM track
 *
 * Helpers for SystemVerilog virtual-interface elaboration.
 */

# include  "config.h"
# include  "PExpr.h"
# include  "netlist.h"
# include  "netvif.h"
# include  "netclass.h"
# include  "netmisc.h"
# include  "compiler.h"
# include  "ivl_assert.h"

using namespace std;

/*
 * Build `$ivl_vif_new(sig0, sig1, ...)` from a concrete interface scope.
 */
NetExpr* elab_vif_new_from_scope(Design*des, const LineInfo&loc,
				 NetScope*iface_scope, const netvif_t*vif_type)
{
      if (!iface_scope || !iface_scope->is_interface() || !vif_type) {
	    cerr << loc.get_fileline() << ": error: "
		 << "Virtual interface assignment requires an interface instance."
		 << endl;
	    des->errors += 1;
	    return 0;
      }

      if (iface_scope->module_name() != vif_type->interface_name()) {
	    cerr << loc.get_fileline() << ": error: "
		 << "Interface type mismatch: expected `"
		 << vif_type->interface_name() << "', got `"
		 << iface_scope->module_name() << "'." << endl;
	    des->errors += 1;
	    return 0;
      }

      size_t n = vif_type->get_members();
      NetESFunc*fun = new NetESFunc("$ivl_vif_new",
				     static_cast<ivl_type_t>(vif_type),
				     static_cast<unsigned>(n));
      fun->set_line(loc);

      for (size_t idx = 0; idx < n; idx += 1) {
	    perm_string mname = vif_type->get_member_name(idx);
	    NetNet*sig = iface_scope->find_signal(mname);
	    if (!sig) {
		  cerr << loc.get_fileline() << ": error: "
		       << "Interface `" << iface_scope->module_name()
		       << "' has no member `" << mname
		       << "' for virtual interface." << endl;
		  des->errors += 1;
		  delete fun;
		  return 0;
	    }
	    NetESignal*es = new NetESignal(sig);
	    es->set_line(loc);
	    fun->parm(idx, es);
      }

      return fun;
}

/*
 * `$ivl_vif_get(vif_handle, member_idx)` reading a VI member.
 */
NetExpr* elab_vif_member_get(const LineInfo&loc, NetExpr*vif_base,
			     const netvif_t*vif_type, int member_idx)
{
      ivl_assert(loc, vif_base && vif_type && member_idx >= 0);
      ivl_type_t mtype = vif_type->get_member_type(static_cast<size_t>(member_idx));
      NetESFunc*fun = new NetESFunc("$ivl_vif_get", mtype, 2);
      fun->set_line(loc);
      fun->parm(0, vif_base);
      NetEConst*idx = new NetEConst(verinum(static_cast<uint64_t>(member_idx), 32));
      idx->set_line(loc);
      fun->parm(1, idx);
      return fun;
}

NetSTask* elab_vif_wait_task(const LineInfo&loc, NetExpr*vif_base,
			     int edge_code, int member_idx)
{
      vector<NetExpr*> parms(3);
      parms[0] = vif_base;
      parms[1] = new NetEConst(verinum(static_cast<uint64_t>(edge_code), 32));
      parms[1]->set_line(loc);
      parms[2] = new NetEConst(verinum(static_cast<uint64_t>(member_idx), 32));
      parms[2]->set_line(loc);
      NetSTask*task = new NetSTask("$ivl_vif_wait", IVL_SFUNC_AS_TASK_IGNORE, parms);
      task->set_line(loc);
      return task;
}
