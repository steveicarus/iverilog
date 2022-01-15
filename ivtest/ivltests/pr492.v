/*
 * Copyright (c) 2002 Richard M. Myers
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

 `timescale 10 ns/ 10 ns

module top ;

  reg clk ;
  reg [11:0] x_os_integ, y_os_integ;
  reg [5:0] x_os, y_os;

  initial
	  begin
    //$dumpfile("show_math.vcd");
    //$dumpvars(1, top);
	  clk = 1'h0 ;
    x_os = 6'h01;
    y_os = 6'h3f;
    x_os_integ = 12'h000;
    y_os_integ = 12'h000;
	  end

  initial
	  begin
	#60;
	  forever #3 clk = ~clk ; // 16Mhz
	end

  always @( posedge clk )
    begin
    // Integration period set above depending on configured modem speed.
    x_os_integ <= x_os_integ + {{6{x_os[5]}}, {x_os[5:0]}};
    y_os_integ <= y_os_integ + {{6{y_os[5]}}, {y_os[5:0]}};

    $display ("%x %x", x_os_integ, y_os_integ);
    end

  initial
    begin
    #200 $finish(0);
    end

endmodule
