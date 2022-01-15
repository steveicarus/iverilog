/*
 * Copyright (c) 2002 Stephen Williams (steve@icarus.com)
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
 * This tests that parameters can be used in concatenations if
 * the expression it represents has definite width. This test
 * is based on PR#282.
 */

module t;

parameter
  SET  = 1'b1,
  CLR  = 1'b0,
  S1   = 2'd1,
  HINC = 3'd4;

//bit signif  12:11, 10 , 9 , 8 , 7, 6 , 5 , 4 , 3 ,2:0
parameter
            x = {S1,CLR,CLR,CLR,CLR,SET,SET,CLR,CLR,HINC };

initial begin
   $display("x=%b, $sizeof(x)=%d", x, $sizeof(x));
   if (x !== 13'b0100001100100) begin
      $display("FAILED -- x is %b", x);
      $finish;
   end

   if ($sizeof(x) != 13) begin
      $display("FAILED -- x is %d'b%b", $sizeof(x), x);
      $finish;
   end

   $display("PASSED");

end

endmodule
