// SystemVerilog allows parameters without default values in the parameter port
// list. Check that this is supported. The test should fail in Verilog mode.

module a #(
  parameter A
);

initial begin
  if (A == 1) begin
    $display("PASSED");
  end else begin
    $display("FAILED");
  end
end

endmodule

module test;
  a #(.A(1)) i_a();
endmodule
