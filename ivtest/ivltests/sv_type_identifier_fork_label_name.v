// Check that fork block labels can shadow visible type identifiers.

typedef int T;

module test;

  reg failed;
  reg value;

  initial begin
    failed = 1'b0;
    value = 1'b0;

    fork : T
      value = 1'b1;
    join : T

    if (value !== 1'b1) begin
      $display("FAILED(%0d). fork label did not hide typedef", `__LINE__);
      failed = 1'b1;
    end

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
