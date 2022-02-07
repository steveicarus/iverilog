// Check whether it is possible to declare a parameter in a generate block
// In Verilog this should fail, in SystemVerilog the parameter is elaborated as
// localparam and the test should pass.

module test;

  generate
    genvar i;
    for (i = 0; i < 2; i = i + 1) begin : loop
      parameter A = i;
      reg [A:0] r = A+1;
    end
  endgenerate

  initial begin
    if (loop[0].r == 1 && loop[1].r == 2) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
