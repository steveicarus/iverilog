/*
 * Copyright (c) 2008 Gyorgy Jeney (nog@sdf.lonestar.org)
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

module fpu_li_s1();

parameter eh = 11;
parameter [eh - 1:0] alap = 1023;

parameter ih2 = 6;

parameter fh = 7;
parameter [fh - 1:0] falap = 63;

localparam ih = 1 << ih2;
localparam nh = (eh > fh ? eh : fh) + 1;

wire [nh - 1:0] exp_norm;

/* IVL compiles this fine but when trying to run it through vvp, it throws
 * this error:
 *
 * internal error: port 0 expects wid=11, got wid=12
 * vvp: concat.cc:56: virtual void vvp_fun_concat::recv_vec4(vvp_net_ptr_t, const
 * vvp_vector4_t&): Assertion `0' failed.
 * Aborted
 *
 * This is a regression caused by this commit:
 * commit a914eda5eff8b088837432a6516584b6a075fcd6
 * Author: Stephen Williams <steve@icarus.com>
 * Date:   Tue Apr 8 20:50:36 2008 -0700
 *
 *     Get part select from vectored parameters correct.
 *
 *     Parameters with vector descriptions that are not zero based and
 *     are used in net contexts should generate the properly ranged
 *     temporary signals. This causes subsequent part selects to work
 *     out properly.
 */
assign exp_norm = alap - falap + ih;

initial
begin
	#1 $display("PASSED");
end

endmodule
