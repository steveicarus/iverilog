// Check that localparam can not be overridden by defparam

module a;
  localparam A = 1;

  initial begin
    $display("FAILED");
  end
endmodule

module test;

  a i_a();

  defparam i_a.A = 10; // Error

endmodule
