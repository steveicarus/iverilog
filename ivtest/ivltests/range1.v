/*
 * Copyright (c) 2000 Nadim Shaikli
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

/***************************************************
*
* Problem: Core dump on 'out of range' error
*          search for 'thing[9]'
*
***************************************************/

module main;

   reg       clk;
   reg [3:0] sig;
   reg [7:0] thing;

   // generate a clock
   always
     #10 clk = ~clk;

   initial
     begin
        $display ("\n<< BEGIN >>");

        case ( sig[3:0] )
          4'b0000: thing[0]     = 1'b1;
          4'b0010: thing[2]     = 1'b1;
          4'b0011: thing[9]     = 1'b1;
        endcase // case( sig[3:0] )

        $display ("<< END  >>\n");
        $finish;
     end

   // Waves definition
//   initial
//     begin
//        $dumpfile("out.dump");
//        $dumpvars(0, main);
//     end

endmodule // main
