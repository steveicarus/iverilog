// Tests that it possible to omit the initial `parameter` keyword in a parameter
// port list in SystemVerilog. In Verilog this is not allowed and should result
// in an error.

module a #(A = 1);
  initial begin
    if (A == 10) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end
endmodule

module test;
  a #(.A(10)) i_a();
endmodule
