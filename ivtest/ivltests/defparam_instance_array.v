// Check that defparam can select an instance in an array.

module M;
  parameter [3:0] P = 4'b1100;
endmodule

module test;

  M i_m[1:0]();
  reg failed;

  defparam i_m[1].P = 1;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    `check(i_m[0].P, 4'b1100);
    `check(i_m[1].P, 4'b0001);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
