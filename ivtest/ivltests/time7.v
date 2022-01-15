/*
 * Copyright (c) 2000 Nadim Shaikli <nadim_shaikli@hotmail.com>
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

/* This is made up from PR#63 */
module main;

   reg one, clk;
   reg [1:0] a, b, c, passed;
   reg [7:0] count;

   always
     #1  one = ~one;

   // generate a clock
   always
     #10 clk = ~clk;

   initial
     begin
        $display ("\n<< BEGIN >>");
        one             = 1'b1;
        clk             = 1'b0;
        passed          = 2'b00;
        count           = 0;
        #15 a[1:0]      = 2'b01;
        #10 a[1:0]      = 2'b10;
        #20 $display ("\n<< END  >>");
	if (passed == 2)
          $display ("PASSED");
        else
          $display ("FAILED");
        $finish;
     end

   always @(clk)
     begin
        // Problematic lines below -- comment them out to see timing skew
        b[1:0] <= #2.5 a[1:0];
        c[1:0] <= #7.8 a[1:0];
     end

   always @(one)
     count[7:0] <= count + 1;

   always @(count)
     begin
        case ( count )
          'd25:
            if (b[1:0] == 2'b01)
              begin
                 $display ("@ %0t - Got ONE", $time);
                 passed = passed + 1;
              end
            else
              $display ("@ %0t - failure", $time);
          'd29:
            if (b[1:0] == 2'b01)
              begin
                 $display ("@ %0t - Got ONE", $time);
                 passed = passed + 1;
              end
            else
              $display ("@ %0t - failure", $time);
          default:
            $display ("@ %0t - no count", $time);
        endcase
     end

   // Waves definition
//   initial
//     begin
//        $recordvars("primitives", "drivers");
//        $dumpfile("out.dump");
//        $dumpvars(5, main);
// Line below ought to work
//       $dumpvars;
//     end

endmodule // main
