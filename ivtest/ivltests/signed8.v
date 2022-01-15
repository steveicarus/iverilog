/*
 * Copyright (c) 2003 The ASIC Group (www.asicgroup.com)
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

module mult58s(input [4:0] a, input signed [7:0] b, output signed [15:0] p);

wire signed [12:0] pt;

wire signed [5:0] ta;

assign ta = a;
assign pt =  b * ta;

assign p=pt;

endmodule


module test_mult;

integer a,b, prod;

wire [15:0] p;
mult58s dut(a[4:0], b[7:0], p);

initial begin
   for(a = 0; a < (1<<5); a=a+1 )
     for(b=-127; b<128; b=b+1)
       begin
         prod = a * b;
         #1 if(p !== prod[15:0]) begin
            $display("Error Miscompare with a=%h, b=%h expect = %0d (%h) acutal = %h",
					a[4:0], b[7:0], prod, prod[15:0], p);
            $finish;
         end

       end

   $display("PASSED");
end

endmodule
