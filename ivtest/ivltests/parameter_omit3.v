// Tests that it possible to omit the `parameter` keyword in a parameter port
// list before changing the parameter type in SystemVerilog. In Verilog this is
// not allowed and should result in an error.

module a #(parameter real A = 1.0, integer B = 2);
  initial begin
    if (A == 10.1 && B == 20) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end
endmodule

module test;
  a #(.A(10.1), .B(20)) i_a();
endmodule
