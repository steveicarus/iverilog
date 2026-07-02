// Regression test: comparing a `string` parameter to a string constant of
// a different length must constant-fold cleanly instead of tripping the
// "lv.len() == rv.len()" assertion in the NetEBComp evaluation functions.
// Covers ==, !=, ===, !==, ==? and !=? with the string operand on either
// side and both shorter and longer than the other operand.

module main #(parameter string Target = "A",
	      parameter string Long = "ABC");

   integer errors = 0;

   // Equality / inequality, parameter shorter than the literal.
   if (Target == "AA") initial begin errors += 1; $display("FAILED: Target == \"AA\""); end
   if (Target != "A")  initial begin errors += 1; $display("FAILED: Target != \"A\""); end

   // Literal on the left.
   if ("AA" == Target) initial begin errors += 1; $display("FAILED: \"AA\" == Target"); end

   // Parameter longer than the literal.
   if (Long == "AB")   initial begin errors += 1; $display("FAILED: Long == \"AB\""); end
   if (Long != "ABC")  initial begin errors += 1; $display("FAILED: Long != \"ABC\""); end

   // Two string parameters of different lengths.
   if (Target == Long) initial begin errors += 1; $display("FAILED: Target == Long"); end

   // Case equality.
   if (Target === "AA") initial begin errors += 1; $display("FAILED: Target === \"AA\""); end
   if (Target !== "A")  initial begin errors += 1; $display("FAILED: Target !== \"A\""); end

   // Wildcard equality.
   if (Target ==? "AA") initial begin errors += 1; $display("FAILED: Target ==? \"AA\""); end
   if (Target !=? "A")  initial begin errors += 1; $display("FAILED: Target !=? \"A\""); end

   initial #1 if (errors == 0) $display("PASSED");

endmodule
