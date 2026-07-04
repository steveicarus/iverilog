// Check that block labels can shadow visible type identifiers.

typedef int T;

module test;

  reg failed;
  reg value;

  initial begin
    failed = 1'b0;
    value = 0;

    begin : T
      value = value + 1;
    end : T

    if (value !== 1) begin
      $display("FAILED(%0d). block labels did not hide typedefs", `__LINE__);
      failed = 1'b1;
    end

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
