// Regression test: comparing a `string` parameter to a string constant of
// a different length must constant-fold cleanly instead of tripping the
// "lv.len() == rv.len()" assertion in the NetEBComp evaluation functions.
// Covers ==, !=, === and !== with the string operand on either side and
// both shorter and longer than the other operand.

module main;
   string Target = "A";
   string Long = "ABC";

   integer errors = 0;

   initial begin
      // Equality / inequality, string shorter than the literal.
      if (Target == "AA") begin errors += 1; $display("FAILED: Target == \"AA\""); end
      if (Target != "A")  begin errors += 1; $display("FAILED: Target != \"A\""); end

      // Literal on the left.
      if ("AA" == Target) begin errors += 1; $display("FAILED: \"AA\" == Target"); end

      // String longer than the literal.
      if (Long == "AB")   begin errors += 1; $display("FAILED: Long == \"AB\""); end
      if (Long != "ABC")  begin errors += 1; $display("FAILED: Long != \"ABC\""); end

      // Two strings of different lengths.
      if (Target == Long) begin errors += 1; $display("FAILED: Target == Long"); end

      // Case equality.
      if (Target === "AA") begin errors += 1; $display("FAILED: Target === \"AA\""); end
      if (Target !== "A")  begin errors += 1; $display("FAILED: Target !== \"A\""); end

      if (errors == 0) $display("PASSED");
   end

endmodule
