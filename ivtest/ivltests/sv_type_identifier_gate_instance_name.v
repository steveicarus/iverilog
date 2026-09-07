// Check that a gate instance can shadow a previously used outer typedef.

typedef logic [7:0] G;

module test;
  wire out;

  G value; // Declares variable using the outer typedef.
  localparam WIDTH = $bits(G);
  and G(out, 1'b1, 1'b1); // Instantiates gate and shadows the outer typedef.

  initial begin
    value = 8'h2a;
    #1;
    if (out !== 1'b1 || value !== 8'h2a || WIDTH !== 8) begin
      $display("FAILED");
    end else begin
      $display("PASSED");
    end
  end
endmodule
