/*
 * land2 - a verilog test for logical and operator && in boolean context
 *
 *
 * Copyright (C) 1999 Stephen G. Tell
 * Portions inspired by qmark.v by Steven Wilson (stevew@home.com)
 * Modified by SDW to self test.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307 USA
 */

module land2;

   reg Clk;
   reg a;
   reg b;
   reg c;
   reg error;

   wire q;
   wire q_calc;

   tand tand_m(q, q_calc, a, b, c);

   initial Clk = 0;
   always #10 Clk = ~Clk;

   always @(posedge Clk)
      begin
          #1;
           if(q != q_calc)
              begin
                  $display("FAILED - expr && using %b%b%b is %b s/b %b",
                            a, b, c, q,q_calc);
                  error = 1;
              end
      end

   reg [3:0] bvec;
   integer   xa;
   integer xb;
   integer xc;
   initial begin
      bvec = 4'bzx10 ;
      error = 0;
      for(xa = 0; xa <= 3; xa = xa + 1)
	    for(xb = 0; xb <= 3; xb = xb + 1)
	       for(xc = 0; xc <= 3; xc = xc + 1)
	          begin
		        @(posedge Clk)
		           a = bvec[xa];
		           b = bvec[xb];
		           c = bvec[xc];
	           end // for (var3 = 0; var3 <= 3; var3 = var3 + 1)
      if(error == 0 ) $display("PASSED");
      $finish;
   end

endmodule

module tand(q, q_calc, a, b, c);
   output q;
   output q_calc;
   input  a;
   input  b;
   input  c;

   wire   q = ( (a===b) && (b===c) );
   reg q_calc;

   always @(a or b or c)
     begin
       if(a === b)
         begin
           if( b === c)
             q_calc = 1'b1;
           else
             q_calc = 1'b0;
         end
       else
         q_calc = 1'b0;
     end

endmodule // foo
