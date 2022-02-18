// Check that trying to override a parameter that does not exist results in an
// error

module a #(
  parameter A = 1
);

  initial begin
    $display("FAILED");
  end
endmodule

module test;

  a #(
    .A(10)
  ) i_a();

  defparam i_a.Z = 10; // Error

endmodule
