// Check that invalid indexed part-select l-value bases report an error.

module test;
  reg [31:0] a;

  initial begin
    a[does_not_exist -: 2] = 2'b00;
  end
endmodule
