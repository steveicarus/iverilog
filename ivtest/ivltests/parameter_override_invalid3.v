// Check that localparam can not be overridden

module a;
  localparam A = 1;

  initial begin
    $display("FAILED");
  end
endmodule

module test;

  a #(
    .A(10) // Error
  ) i_a();

endmodule
