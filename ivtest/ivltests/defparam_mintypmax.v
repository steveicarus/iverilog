// Check that defparam accepts a min:typ:max expression without extra parentheses.

module M;
  parameter P = 0;
endmodule

module test;
  M i_m();
  defparam i_m.P = 1:2:3;

  initial begin
    if (i_m.P === `EXPECTED) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end
endmodule
