// Check that it is an error to specify a default value for inout port
// declarations.

module M (
  inout [31:0] x, y = 1 // inout ports do not support default values
);
  initial begin
    $display("FAILED");
  end

endmodule

module test;

  wire [31:0] x, y;

  M i_m (
    .x(x),
    .y(y)
  );

endmodule
