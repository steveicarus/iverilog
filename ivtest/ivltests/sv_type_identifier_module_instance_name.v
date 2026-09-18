// Check that a module instance can shadow an outer typedef.

typedef int i_m;

module M(input wire in, output wire out);
  assign out = in;
endmodule

module test;
  wire out;

  M i_m(.in(1'b1), .out(out)); // Instantiates module and shadows the outer typedef.

  initial begin
    #1;
    if (out !== 1'b1) begin
      $display("FAILED");
    end else begin
      $display("PASSED");
    end
  end
endmodule
