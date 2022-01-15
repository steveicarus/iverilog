/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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

/*
 * This test was derived from PR617.v
 **/
module main();
  parameter INIT_00 = 32'hffffffff;
  reg [17:0] t;
  reg [8:0] c;

  initial begin
     t = {17'd0,INIT_00[0]}<<1;
     $display("%b",t);
     if (t !== 18'b00_0000_0000_0000_0010) begin
	$display("FAILED");
	$finish;
     end

     c = 0;
     t = {17'd0,INIT_00[c]}<<1;
     $display("%b",t);
     if (t !== 18'b00_0000_0000_0000_0010) begin
	$display("FAILED");
	$finish;
     end

     c = 16;
     t = {17'd0,INIT_00[c]}<<1;
     $display("%b",t);
     if (t !== 18'b00_0000_0000_0000_0010) begin
	$display("FAILED");
	$finish;
     end

     $display("PASSED");
  end
endmodule
