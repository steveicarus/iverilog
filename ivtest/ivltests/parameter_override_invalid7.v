// Check that localparam declared in parameter port list can not be overridden

module a #(
  localparam B = 2
);

  initial begin
    $display("FAILED");
  end
endmodule

module test;

  a #(
    .A(10) // Error
  ) i_a();

endmodule
