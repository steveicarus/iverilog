module top;
  parameter string Target = "A";
  string str;

  // Case equality.
  if (Target ==? "AA") initial $display("FAILED: Target ==? \"AA\"");
  if (Target !=? "A")  initial $display("FAILED: Target !=? \"A\"");

  initial begin
    str = "AA";
    if (str ==? Target) $display("FAILED: str ==? Target");
    str = "A";
    if (str !=? Target) $display("FAILED: str ==? Target");
    $display("FAILED: EXpected compile error.");
  end

endmodule
