// Check that it is not possible to override parameters in generate blocks

module test;

  generate
    genvar i;
    for (i = 0; i < 2; i = i + 1) begin : loop
      parameter A = i;
      reg [A:0] r = A+1;
    end
  endgenerate

  defparam loop[0].A = 10;
  defparam loop[1].A = 20;

endmodule
