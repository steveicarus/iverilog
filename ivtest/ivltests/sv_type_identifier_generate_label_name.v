// Check that generate block labels can shadow visible type identifiers.

typedef int G;
module test;

  reg failed;

  if (1) begin : G
    localparam int VALUE = 1;
  end : G

  initial begin
    failed = 1'b0;

    if (G.VALUE != 1) begin
      $display("FAILED(%0d). generate labels did not hide typedefs", `__LINE__);
      failed = 1'b1;
    end

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
