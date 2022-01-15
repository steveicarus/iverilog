/* This test checks two thing:
 *
 *   The first is that the AND arguments are padded if one is
 *   smaller than the other. This was causing an assert.
 *
 *   The second is that the reduction operator does not pass
 *   the expression width to its arguments. This will give an
 *   incorrect result (01 vs 00).
 */
module test ();
   reg pass = 1'b1;

   reg [1:0] ra;
   wire [1:0] a;
   wire [3:0] b = 4'b1111;
   wire [3:0] c = 4'b1111;

   assign a = |((c & ~(1'b1<<9'h00)) & b);

   initial begin
      #1;
      if (a !== 2'b01) begin
          $display("FAILED: cont. assign, expected 2'b01, got %b", a);
          pass = 1'b0;
      end

      ra = |((c & ~(1'b1<<9'h00)) & b);
      if (ra !== 2'b01) begin
          $display("FAILED: proc. assign, expected 2'b01, got %b", ra);
          pass = 1'b0;
      end

      if (pass) $display("PASSED");
   end
endmodule
