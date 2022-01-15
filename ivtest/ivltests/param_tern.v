/*
 * Copyright (c) 1998-2000 Stephen Williams (steve@icarus.com)
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

module main;

   parameter PARM08 = 8;
   parameter PARM04 = PARM08 >> 1;
   parameter PARM16 = PARM08 << 1;

   parameter PARM10 = ((PARM08 <=2) ? 1:
                      ((PARM08 <=4) ? 2:
                      ((PARM08 <=8) ? 3:4)));

   // this parameterized input compiles ok
   wire [PARM04 : 0] in04;
   wire [PARM16 : 0] in05;

   reg [PARM08 : 0]  out00;
   reg [PARM04 : 0]  out04;
   reg [PARM16 : 0]  out05;

   // this parameterized doesn't compile, stack dump
   wire [PARM10:0]   in99;

   initial begin
      if (PARM08 !== 8) begin
	 $display("FAILED -- PARM08 == %b", PARM08);
	 $finish;
      end

      if (PARM04 !== 4) begin
	 $display("FAILED -- PARM04 == %b", PARM04);
	 $finish;
      end

      if (PARM16 !== 16) begin
	 $display("FAILED -- PARM16 == %b", PARM16);
	 $finish;
      end


      if (PARM10 !== 3) begin
	 $display("FAILED -- PARM10 == %b", PARM10);
	 $finish;
      end

      $display("PASSED");
   end

endmodule
