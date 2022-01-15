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
//
// Integer Multicycle Divide circuit (divide a 16-bit number by a 16-bit number in 16 cycles).
//
// a / b = q with remainder r
//
// Where a is 16-bits,
// Where b is 16 bits
//
// Module is actually parameterized if you want other widths.
//
// *** Test the ranges of values for which you'll use this.  For example, you
//     can't divide FFFF by FF without underflow (overflow?).  Mess with
//     the testbench.  You may need to widen some thing.  ***
//
// The answer is 16-bits and the remainder is also 16-bits.
// After the start pulse, the module requires 16 cycles to complete.
// The q/r outputs stay the same until next start pulse.
// Start pulse should be a single cycle.
// Division by zero results in a quotient equal to FFFF and remainder equal to 'a'.
//
//
// Written by tom coonan.
//
// Notes:
//    - This ain't fancy.  I wanted something straight-forward quickly.  Go study
//      more elaborate algorithms if you want to optimize area or speed.  If you
//      have an isolated divide and can spare N cycles for N bits; this may meet your needs.
//    - You might want to think more about the sizes of things.  I wanted a basic estimate
//      of gates plus I specifically needed to divide 16-bits (not even full range)
//      by 8-bits.
//    - Handle divide by zero at higher level..
//    - I needed a remainder so I could easily to truncate and rounding stuff,
//      but remove this to save gates if you don't need a remainder.
//    - This is about 800 asic gates (0.25um, Standard Cell, 27Mhz).  27Mhz
//      is my system clock and NOT the maximum it can go..
//    - I tried to keep everything parameterized by N, but I only worked through
//      the N=16 case because that's what I needed...
//
module div16 (clk, resetb, start, a, b, q, r, done);

parameter N = 16;	// a/b = q remainder r, where all operands are N wide.

input		clk;
input		resetb;	// Asynchronous, active low reset.
input		start;	// Pulse this to start the division.
input [N-1:0]	a;	// This is the number we are dividing (the dividend)
input [N-1:0]	b;	// This is the 'divisor'
output [N-1:0]	q;	// This is the 'quotient'
output [N-1:0]	r;	// Here is the remainder.
output		done;	// Will be asserted when q and r are available.

// Registered q
reg [N-1:0]	q;
reg		done;

// Power is the current 2^n bit we are considering.  Power is a shifting
// '1' that starts at the highest power of 2 and goes all the way down
// to ...00001  Shift this until it is zero at which point we stop.
//
reg [N-1:0]	power;

// This is the accumulator.  We are start with the accumulator set to 'a' (the dividend).
// For each (divisor*2^N) term, we see if we can subtract (divisor*2^N) from the accumulator.
// We subtract these terms as long as adding in the term doesn't cause the accumulator
// to exceed a.  When we are done, whatever is left in the accumulator is the remainder.
//
reg [N-1:0]	accum;

// This is the divisor*2^N term.  Essentually, we are taking the divisor ('b'), initially
// shifting it all the way to the left, and shifting it 1 bit at a time to the right.
//
reg [(2*N-1):0]	bpower;

// Remainder will be whatever is left in the accumulator.
assign r = accum;

// Do this addition here for resource sharing.
// ** Note that 'accum' is N bits wide, but bpower is 2*N-1 bits wide **
//
wire [2*N-1:0] accum_minus_bpower = accum - bpower;

always @(posedge clk or negedge resetb) begin
   if (~resetb) begin
      q <= 0;
      accum <= 0;
      power <= 0;
      bpower <= 0;
      done <= 0;
   end
   else begin
      if (start) begin
         // Reinitialize the divide circuit.
         q      <= 0;
         accum  <= a; // Accumulator initially gets the dividend.
         power[N-1] <= 1'b1; // We start with highest power of 2 (which is a '1' in MSB)
         bpower <= b << N-1; // Start with highest bpower, which is (divisor * 2^(N-1))
         done <= 0;
      end
      else begin
         // Go until power is zero.
         //
         if (power != 0) begin
            //
            // Can we add this divisor*2^(power) to the accumulator without going negative?
            // Just test the MSB of the subtraction.  If it is '1', then it must be negative.
            //
            if ( ~accum_minus_bpower[2*N-1]) begin
               // Yes!  Set this power of 2 in the quotieny and
               // then actually comitt to the subtraction from our accumulator.
               //
               q     <= q | power;
               accum <= accum_minus_bpower;
            end
            // Regardless, always go to next lower power of 2.
            //
            power  <= power >> 1;
            bpower <= bpower >> 1;
         end
         else begin
            // We're done.  Set done flag.
            done <= 1;
         end
      end
   end
end
endmodule

// synopsys translate_off
module test_div16;
reg		clk;
reg		resetb;
reg		start;
reg [15:0]	a;
reg [15:0]	b;
wire [15:0]	q;
wire [15:0]	r;
wire		done;

integer		num_errors;

div16 div16 (
   .clk(clk),
   .resetb(resetb),
   .start(start),
   .a(a),
   .b(b),
   .q(q),
   .r(r),
   .done(done)
);

initial begin
   num_errors = 0;

   start = 0;

   // Wait till reset is completely over.
   #200;

   // Do some divisions where divisor is constrained to 8-bits and dividend is 16-bits
   $display ("16-bit Dividend, 8-bit divisor");
   repeat (25) begin
      do_divide ($random, $random & 255);
   end

   // Do some divisions where divisor is constrained to 12-bits and dividend is 16-bits
   $display ("\n16-bit Dividend, 12-bit divisor");
   repeat (25) begin
      do_divide ($random, $random & 4095);
   end

   // Do some divisions where both divisor and dividend is 16-bits
   $display ("\n16-bit Dividend, 16-bit divisor");
   repeat (25) begin
      do_divide ($random, $random);
   end

   // Special cases
   $display ("\nSpecial Cases:");
   do_divide (16'hFFFF,  16'hFFFF); // largest possible quotient
   do_divide (312,  1); // divide by 1
   do_divide (  0, 42); // divide 0 by something else
   do_divide (312,  0); // divide by zero

   // That's all.  Summarize the test.
   if (num_errors === 0) begin
      $display ("\n\nPASSED");
   end
   else begin
      $display ("\n\nFAILED - There were %0d Errors.", num_errors);
   end

   $finish;
end

task do_divide;
   input [15:0] arga;
   input [15:0] argb;

   begin
      a = arga;
      b = argb;
      @(posedge clk);
      #1 start = 1;
      @(posedge clk);
      #1 start = 0;
      while (~done) @(posedge clk);
      #1;

      $display ("Circuit: %0d / %0d = %0d, rem = %0d\t\t......... Reality: %0d, rem = %0d", arga, argb, q, r, a/b, a%b);
      if (b !== 0) begin
         if (q !== a/b) begin
            $display ("   Error!  Unexpected Quotient\n\n");
            num_errors = num_errors + 1;
         end
         if (r !== a % b) begin
            $display ("   Error!  Unexpected Remainder\n\n");
            num_errors = num_errors + 1;
         end
      end
   end
endtask

initial begin
   clk = 0;
   forever begin
      #10 clk = 1;
      #10 clk = 0;
   end
end

initial begin
   resetb = 0;
   #133 resetb = 1;
end

//initial begin
//   $dumpfile ("test_div16.vcd");
//   $dumpvars (0,test_div16);
//end

endmodule
