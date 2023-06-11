// Check that an error is reported when specifying too many ports in a ordered
// list connection.

module M (
    output a,
    input  b
);
    assign a = b;
endmodule

module test;

  wire a, b, c;

  assign b = 1'b0;
  assign c = 1'b1;

  M i_M (a, b, c); // Error, too many ports.

  initial begin
    $display("FAILED");
  end

endmodule
