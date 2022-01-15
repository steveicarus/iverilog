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
// Just a little demo of some FSM techniques, including One-Hot and
// using 'default' settings and the case statements to selectively
// update registers (sort of like J-K flip-flops).
//
// tom coonan, 12/98.
//
// SDW - modified test to check final X and Y value... and print out
//       PASSED if it's okay.
//
module onehot (clk, resetb, a, b, x, y);

input clk;
input resetb;
input [7:0] a;
input [7:0] b;
output [7:0] x;
output [7:0] y;

// Use One-Hot encoding.  There will be 16 states.
//
reg [15:0] state, next_state;

// These are working registers.  Declare the register itself (e.g. 'x') and then
// the input bus used to load in a new value (e.g. 'x_in').  The 'x_in' bus will
// physically be a wire bus and 'x' will be the flip-flop register ('x_in' must
// be declared 'reg' because it's used in an always block.
//
reg [7:0] x, x_in;
reg [7:0] y, y_in;

// Update state.  'state' is the actual flip-flop register and next_state is the combinatorial
// bus used to update 'state''s value.  Check for the ZERO state which means an unexpected
// next state was computed.  If this occurs, jump to our initialization state; state[0].
//
// It is considered good practice by many designers to seperate the combinatorial
// and sequential aspects of state registers, and often registers in general.
//
always @(posedge clk or negedge resetb) begin
   if (~resetb) state = 0;
   else begin
      if (next_state == 0) begin
         state = 16'h0001;
      end
      else begin
         state = next_state;
      end
   end
end

// Implement the X flip-flop register.  Always load the input bus into the register.
// Reset to zero.
//
always @(posedge clk or negedge resetb) begin
   if (~resetb) x = 0;
   else         x = x_in;
end

// Implement the Y flip-flop register.  Always load the input bus into the register.
// Reset to zero.
//
always @(posedge clk or negedge resetb) begin
   if (~resetb) y = 0;
   else         y = y_in;
end

// Generate the next_state function.  Also, based on the current state, generate
// any new values for X and Y.
//
always @(state or a or b or x or y) begin
   // *** Establish defaults.

   // Working registers by default retain their current value.  If any particular
   // state does NOT need to change a register, then it doesn't have to reference
   // the register at all.  In these cases, the default below takes affect.  This
   // turns out to be a pretty succinct way to control stuff from the FSM.
   //
   x_in = x;
   y_in = y;

   // State by default will be cleared.  If we somehow ever got into an unknown
   // state, then the default would throw state machine back to zero.  Look
   // at the sequential 'always' block for state to see how this is handled.
   //
   next_state = 0;

   // One-Hot State Machine Encoding.
   //
   // *** Using a 1'b1 in the case statement is the trick to doing One-Hot...
   //     DON'T include a 'default' clause within the case because we want to
   //     establish the defaults above. ***
   //
   case (1'b1) // synopsys parallel_case

      // Initialization state.  Set X and Y register to some interesting starting values.
      //
      state[0]:
         begin
            x_in = 8'd20;
            y_in = 8'd100;
            next_state[1] = 1'b1;
         end

      // Just for fun.. Jump through states..
      state[1]: next_state[2] = 1'b1;
      state[2]: next_state[3] = 1'b1;
      state[3]: next_state[4] = 1'b1;
      state[4]: next_state[5] = 1'b1;
      state[5]: next_state[6] = 1'b1;
      state[6]: next_state[7] = 1'b1;

      // Conditionally decrement Y register.
      state[7]:
         begin
            if (a == 1) begin
               y_in = y - 1;
               next_state[1] = 1'b1;
            end
            else begin
               next_state[8] = 1'b1;
            end
         end

      // Just for fun.. Jump through states..
      state[8]: next_state[9]   = 1'b1;
      state[9]: next_state[10]  = 1'b1;
      state[10]: next_state[11] = 1'b1;

      // Conditionally increment X register.
      state[11]:
         begin
            if (b == 1) begin
               x_in = x + 1;
               next_state[1] = 1'b1;
            end
            else begin
               next_state[12] = 1'b1;
            end
         end

      // Just for fun.. Jump through states..
      state[12]: next_state[13] = 1'b1;
      state[13]: next_state[14] = 1'b1;
      state[14]: next_state[15] = 1'b1;
      state[15]: next_state[1]  = 1'b1; // Don't go back to our
                                        // initialization state, but state
                                        // following that one.
    endcase
end
endmodule

// synopsys translate_off
module test_onehot;
reg clk, resetb;
reg [7:0] a;
reg [7:0] b;
wire [7:0] x;
wire [7:0] y;
reg error;

// Instantiate module.
//
onehot onehot (
   .clk(clk),
   .resetb(resetb),
   .a(a),
   .b(b),
   .x(x),
   .y(y)
);

// Generate clock.
//
initial
begin
   clk = 0;
   forever begin
      #10 clk = ~clk;
   end
end

// Reset..
//
initial begin
   resetb = 0;
   #33 resetb = 1;
end

// Here's the test.
//
// Should see X and Y get initially loaded with their starting values.
// As long as a and b are zero, nothing should change.
// When a is asserted, Y should slowly decrement.  When b is asserted, X should
// slowly increment.  That's it.
//
initial begin
`ifdef DEBUG
   $dumpfile("test.vcd");
   $dumpvars(0,test_onehot);
`endif // DEBUG
   error = 0;
   a = 0;
   b = 0;
   repeat (64) @(posedge clk);
   #1

   // Y should be decremented..
   a = 1;
   b = 0;
   repeat (256) @(posedge clk);
   #1

   // X should be incremented..
   a = 0;
   b = 1;
   repeat (256) @(posedge clk);

   if (x !== 8'd43)
      begin
        error = 1;
        $display("FAILED - X Expected value 43, is %d",x);
      end

   if (y !== 8'd64)
      begin
        error = 1;
        $display("FAILED - Y Expected value 63, is %d",y);
      end

   if(error == 0)
      $display("PASSED");

   $finish;
end

// Monitor the module.
//

endmodule
