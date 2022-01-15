/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
 * This program checks that conbinational UDPs with x outputs are
 * properly executed.
 */

module main;

   wire   Y;
   reg	  A, B, S;

   muxx1 MUX2 ( Y, S, A, B ) ;

   initial begin
      S = 0;
      A = 0;
      B = 0;
      #1 if (Y !== 1'b0) begin
	 $display("FAILED -- Y is %b", Y);
	 $finish;
      end

      B = 1;
      #1 if (Y !== 1'b0) begin
	 $display("FAILED -- Y is %b", Y);
	 $finish;
      end

      S = 1;
      #1 if (Y !== 1'b1) begin
	 $display("FAILED -- Y is %b", Y);
	 $finish;
      end

      B = 1'bx;
      #1 if (Y !== 1'bx) begin
	 $display("FAILED -- Y is %b", Y);
	 $finish;
      end

      B = 1;
      S = 1'bx;
      #1 if (Y !== 1'bx) begin
	 $display("FAILED -- Y is %b", Y);
	 $finish;
      end

      B = 0;
      #1 if (Y !== 1'b0) begin
	 $display("FAILED -- Y is %b", Y);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule

primitive muxx1(Q, S, A, B);
   output Q;
   input  S, A, B;

   table
   //   S  A  B    Q
        0  0  ?  : 0 ;
        0  1  ?  : 1 ;
        0  x  ?  : x ;  // problem line
        1  ?  0  : 0 ;
        1  ?  1  : 1 ;
        1  ?  x  : x ;  // problem line
        x  0  0  : 0 ;
        x  1  1  : 1 ;

   endtable
endprimitive
