// Verify that enums can reference items from enums declared before them

module test;

enum logic {
  A = 1
} a;

enum logic {
  B = A
} b;

enum logic {
  C = B
} c;

initial begin
  if (A == B && A == C) begin
    $display("PASSED");
  end else begin
    $display("FAILED");
  end
end

endmodule
