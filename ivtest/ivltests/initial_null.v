// Check that a null initial statement requires SystemVerilog.

module test;

  initial ; // Pass in SystemVerilog, error in Verilog.

  initial begin
    $display("PASSED");
  end

endmodule
