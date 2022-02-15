// Check that parameter declared in the module body can not be overridden if the
// module has a parameter port list.

module a #(
  parameter A = 1
);
  // This behaves like a localparam
  parameter B = 1;

  initial begin
    $display("FAILED");
  end
endmodule

module test;

  a #(
    .A(10),
    .B(20) // Error
  ) i_a();

endmodule
