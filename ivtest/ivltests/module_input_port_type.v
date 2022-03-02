// Check Verilog types on a module input port. In Verilog this is an error, but
// in SystemVerilog it is supported

module test (
  input reg a,
  input time b,
  input integer c
);

  initial begin
    $display("PASSED");
  end

endmodule
