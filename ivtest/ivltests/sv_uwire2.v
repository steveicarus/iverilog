
// This simple program tests that a variable can be assigned
// party by continuous assignment, and partly by behavioral
// assignment. As long as the parts don't overlap, this is
// legal (in SystemVerilog)

module main;

   logic [3:0] foo;

   // Part of the vector is assigned by continuous assignment
   logic [1:0] bar;
   assign foo[2:1] = bar;

   initial begin
      // Part of the vector is assigned behaviorally.
      foo[0:0] = 1'b1;
      foo[3:3] = 1'b1;
      bar = 2'b00;

      #1 if (foo !== 4'b1001) begin
	 $display("FAILED -- foo=%b", foo);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end // initial begin

endmodule // main
