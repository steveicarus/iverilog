/*
Steve,
      I have small 8bit CPU working in Iverilog, it works if I
      change a line similar to the one below in the test case to

      assign result = (data[0] | data[1])  ? 1:0;

      using the test case below I get,

      elab_net.cc:1368: failed assertion `expr_sig->pin_count() == 1'

      when compiling using the standard "verilog bug.v"  (verilog-20000519)

      This works fine in XL.

   Regards

      Gerard.

 PS thanks for fixing the $monitor function. It works as XL,
    as long as I pipe the output through uniq (./stimexe | uniq)


 */


module stim;
   wire [1:0] data;
   wire       result;


  assign result = data ? 1:0;

  initial
       $display("PASSED");


endmodule // stim


/*
 * Copyright (c) 2000 Gerard A. Allan (gaa@ee.ed.ac.uk)
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
