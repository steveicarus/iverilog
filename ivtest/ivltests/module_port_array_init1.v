// Check that initializers values are supported for module array ports

module M (
  input [31:0] x[0:1] = '{1, 2},
  output reg [31:0] y[0:1] = '{3, 4}
);

  initial begin
    #1
    if (x[0] === 1 && x[1] === 2 && y[0] === 3 && y[1] === 4) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule

module test;

  M i_m ();

endmodule
