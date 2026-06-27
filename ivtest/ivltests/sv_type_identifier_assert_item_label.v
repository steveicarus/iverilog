// Check that assertion item labels can shadow visible type identifiers.

typedef int CHECK_A;
typedef int CHECK_B;

module test;

  CHECK_A: assert property (1);
  CHECK_B: assert final (1);

  initial begin
    $display("PASSED");
  end

endmodule
