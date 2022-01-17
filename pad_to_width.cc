/*
 * Copyright (c) 1999-2016 Stephen Williams (steve@icarus.com)
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

# include  "netenum.h"
# include  "netlist.h"
# include  "netvector.h"
# include  "netmisc.h"


NetExpr*pad_to_width(NetExpr*expr, unsigned wid, bool signed_flag,
		     const LineInfo&info, ivl_type_t use_type)
{
      if (wid <= expr->expr_width() && !use_type) {
	    expr->cast_signed(signed_flag);
	    return expr;
      }

	/* If the expression is a const, then replace it with a wider
	   const. This is a more efficient result. */
      if (NetEConst*tmp = dynamic_cast<NetEConst*>(expr)) {
	    verinum oval = tmp->value();
	    oval.has_sign(signed_flag);
	    oval = pad_to_width(oval, wid);
	    if (const netenum_t *enum_type = dynamic_cast<const netenum_t *>(use_type)) {
		  // The name of the enum is set to <nil> here, but the name is
		  // only used in debugging output, so this is ok
		  tmp = new NetEConstEnum(perm_string(), enum_type, oval);
	    } else {
		  tmp = new NetEConst(oval);
	    }
	    tmp->set_line(info);
	    delete expr;
	    return tmp;
      }

      NetESelect*tmp = new NetESelect(expr, 0, wid, use_type);
      tmp->cast_signed(signed_flag);
      tmp->set_line(info);
      return tmp;
}

NetExpr*cast_to_width(NetExpr*expr, unsigned wid, bool signed_flag,
		      const LineInfo&info)
{
        /* If the expression is a const, then replace it with a new
           const. This is a more efficient result. */
      if (NetEConst*tmp = dynamic_cast<NetEConst*>(expr)) {
            tmp->cast_signed(signed_flag);
            if (wid != tmp->expr_width()) {
                  tmp = new NetEConst(verinum(tmp->value(), wid));
                  tmp->set_line(info);
                  delete expr;
            }
            return tmp;
      }

      NetESelect*tmp = new NetESelect(expr, 0, wid);
      tmp->cast_signed(signed_flag);
      tmp->set_line(info);

      return tmp;
}

/*
 * Pad a NetNet to the desired vector width by concatenating a
 * NetConst of constant zeros. Use a NetConcat node to do the
 * concatenation.
 */
NetNet*pad_to_width(Design*des, NetNet*net, unsigned wid, const LineInfo&info)
{
      NetScope*scope = net->scope();

      if (net->vector_width() >= wid)
	    return net;

	// Make the NetConcat and connect the input net to the lsb input.
      NetConcat*cc = new NetConcat(scope, scope->local_symbol(), wid, 2);
      cc->set_line(info);
      des->add_node(cc);
      connect(cc->pin(1), net->pin(0));

	// Make a NetConst of the desired width and connect in to the
	// lsb input of the NetConcat.
      verinum pad(verinum::V0, wid - net->vector_width());
      NetConst*con = new NetConst(scope, scope->local_symbol(), pad);
      con->set_line(info);
      des->add_node(con);
      connect(cc->pin(2), con->pin(0));

	// Make a NetNet for the NetConst to NetConcat link.
      netvector_t*tmp_vec = new netvector_t(net->data_type(),
					    wid - net->vector_width() - 1, 0);
      NetNet*tmp = new NetNet(scope, scope->local_symbol(),
			      NetNet::WIRE, tmp_vec);
      tmp->set_line(info);
      tmp->local_flag(true);
      connect(cc->pin(2), tmp->pin(0));

	// Create a NetNet of the output width and connect it to the
	// NetConcat node output pin.
      tmp_vec = new netvector_t(net->data_type(), wid-1, 0);
      tmp = new NetNet(scope, scope->local_symbol(),
		       NetNet::WIRE, tmp_vec);
      tmp->set_line(info);
      tmp->local_flag(true);
      connect(cc->pin(0), tmp->pin(0));

      return tmp;
}

NetNet*pad_to_width_signed(Design*des, NetNet*net, unsigned wid,
                           const LineInfo&info)
{
      NetScope*scope = net->scope();

      if (net->vector_width() >= wid)
	    return net;

      NetSignExtend*se
	    = new NetSignExtend(scope, scope->local_symbol(), wid);
      se->set_line(info);
      des->add_node(se);

      netvector_t*tmp_vec = new netvector_t(net->data_type(), wid-1, 0);
      tmp_vec->set_signed(true);
      NetNet*tmp = new NetNet(scope, scope->local_symbol(), NetNet::WIRE, tmp_vec);
      tmp->set_line(info);
      tmp->local_flag(true);

      connect(tmp->pin(0), se->pin(0));
      connect(se->pin(1), net->pin(0));

      return tmp;
}

NetNet*crop_to_width(Design*des, NetNet*net, unsigned wid)
{
      NetScope*scope = net->scope();

      if (net->vector_width() <= wid)
	    return net;

      NetPartSelect*ps = new NetPartSelect(net, 0, wid, NetPartSelect::VP);
      ps->set_line(*net);
      des->add_node(ps);

      netvector_t*tmp_vec = new netvector_t(net->data_type(), wid-1, 0);
      NetNet*tmp = new NetNet(scope, scope->local_symbol(),
			      NetNet::WIRE, tmp_vec);
      tmp->set_line(*net);
      tmp->local_flag(true);
      connect(ps->pin(0), tmp->pin(0));

      return tmp;
}
