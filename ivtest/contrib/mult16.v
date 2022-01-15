//
// Copyright (c) 1999 Thomas Coonan (tcoonan@mindspring.com)
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
//
// **** Here's a simple, sequential multiplier.  Very simple, unsigned..
// Not very well tested, play with testbench, use at your own risk, blah blah blah..
//

//
// Unsigned 16-bit multiply (multiply two 16-bit inputs to get a 32-bit output)
//
// Present data and assert start synchronous with clk.
// Assert start for ONLY one cycle.
// Wait N cycles for answer (at most).  Answer will remain stable until next start.
// You may use DONE signal as handshake.
//
// Written by tom coonan
//
module mult16 (clk, resetb, start, done, ain, bin, yout);
parameter N = 16;
input			clk;
input			resetb;
input			start; // Register the ain and bin inputs (they can change afterwards)
//input [N-1:0]		ain;
//input [N-1:0]		bin;
//output [2*N-1:0]	yout;
input [15:0]		ain;
input [15:0]		bin;
output [31:0]	yout;

output			done;

//reg [2*N-1:0]		a;
//reg [N-1:0]		b;
//reg [2*N-1:0]		yout;
reg [31:0]		a;
reg [15:0]		b;
reg [31:0]		yout;

reg		done;

always @(posedge clk or negedge resetb) begin
   if (~resetb) begin
      a <= 0;
      b <= 0;
      yout <= 0;
      done <= 1'b1;
   end
   else begin
      // Load will register the input and clear the counter.
      if (start) begin
         a    <= ain;
         b    <= bin;
         yout <= 0;
         done <= 0;
      end
      else begin
         // Go until b is zero
         if (~done) begin
            if (b != 0) begin
               // If '1' then add a to sum
               if (b[0]) begin
                  yout <= yout + a;
               end
               b <= b >> 1;
               a <= a << 1;
               $display ("a = %h, b = %h, yout = %h", a,b,yout);
            end
            else begin
               done <= 1'b1;
            end
         end
      end
   end
end
endmodule


module mul16;
reg clk, resetb, start;
reg [15:0] a;
reg [15:0] b;
wire [31:0] y;
wire done;

mult16 mult16inst (clk, resetb, start, done, a, b, y);

initial begin
   clk = 0;
   forever begin
      #10 clk = ~clk;
   end
end

initial begin
   resetb = 0;
   #30 resetb = 1;
end

integer num_errors;
parameter MAX_TRIALS = 10;

initial begin
//   $dumpfile ("multdiv.vcd");
//   $dumpvars (0,a);
//   $dumpvars (0,b);
//   $dumpvars (0,y);
//   $dumpvars (0,resetb);
//   $dumpvars (0,done);
   num_errors = 0;

   #100;

   // Do a bunch of random multiplies
   repeat (MAX_TRIALS) begin
      test_multiply ($random, $random);
   end

   // Special cases
   test_multiply ($random, 1);
   test_multiply (1, $random);
   test_multiply ($random, 0);
   test_multiply (0, $random);

   $display ("Done.  %0d Errors", num_errors);
   if(num_errors == 0)
      $display("PASSED");
   #800;
   $finish;
end

task test_multiply;
   input [15:0] aarg;
   input [15:0] barg;

   integer expected_answer;

   begin
      if (~done) begin
         $display ("Multiplier is Busy!!");
      end
      else begin
         @(negedge clk);
         start = 1;
         a = aarg;
         b = barg;
         @(negedge clk) start = 0;
         @(posedge done);
         expected_answer = a*b;
         $display ("%0d * %0d = %0h, Reality = %0h", a, b, y, expected_answer);
         if (y !== expected_answer) begin
            $display ("   FAILED!");
            num_errors = num_errors + 1;
         end
      end
   end
endtask

endmodule
