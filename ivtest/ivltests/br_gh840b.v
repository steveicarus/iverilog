// Testchase for #840 on Github

module test;
  logic [1:0][1:0] a;
  logic [2:0][1:0] b;
  generate
    for(genvar i=0;i<3;i++)
      assign b[i] = a[i];
  endgenerate
endmodule
