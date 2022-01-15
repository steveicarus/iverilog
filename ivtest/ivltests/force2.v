//
// Copyright (c) 2002 Stephen Williams (steve at icarus.com)
//
//    This source code is free software; you can redistribute it
//    and/or modify it in source code form under the terms of the GNU
//    General Public License as published by the Free Software
//    Foundation; either version 2 of the License, or (at your option)
//    any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA

module main;

   reg [3:0]  val_drv = 4'b0101;
   wire [3:0] val = val_drv;

   initial begin
      #50 if (val !== val_drv) begin
	 $display("FAILED -- initial val %b !== %b", val, val_drv);
	 $finish;
      end

      force val = 4'b1010;
      #1 if (val !== 4'b1010) begin
	 $display("FAILED -- force 1010 failed, val=%b", val);
	 $finish;
      end

      // Use force to "lift" the driver.
      force val = 4'bzzzz;

      if (val !== 4'bzzzz) begin
	 $display("FAILED -- force z failed, val=%b", val);
	 $finish;
      end

      release val;
      #1 if (val !== 4'b0101) begin
	 $display("FAILED -- unforced val = %b", val);
	 $finish;
      end

      val_drv = 4'b1010;
      #1 if (val !== 4'b1010) begin
	 $display("FAILED -- val_drv=%b, val=%b", val_drv, val);
	 $finish;
      end

      $display("PASSED");
   end
endmodule
