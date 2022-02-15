// Check that parameter declared in the module body can not be overridden by a
// defparam if the module has a parameter port list.

module a #(parameter A = 1);
  // This behaves like a localparam
  parameter B = 2;

  initial begin
    $display("FAILED");
  end
endmodule

module test;

  a i_a();

  defparam i_a.A = 10;
  defparam i_a.B = 20; // Error

endmodule
