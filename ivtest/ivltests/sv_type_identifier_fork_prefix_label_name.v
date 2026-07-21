// Check that fork prefix labels can shadow type identifiers.

typedef int T;

module test;

  reg failed;
  reg value;

  initial begin
    failed = 1'b0;
    value = 1'b0;

    T: fork
      value = 1'b1;
    join : T

    if (value !== 1'b1) begin
      $display("FAILED(%0d). Fork prefix label did not hide typedef",
               `__LINE__);
      failed = 1'b1;
    end

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
