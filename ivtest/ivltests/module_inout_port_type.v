// Check Verilog types on a module inout port. In Verilog this is an error, but
// in SystemVerilog it is supported

module test (
  inout reg a,
  inout time b,
  inout integer c
);

  initial begin
    $display("PASSED");
  end

endmodule
