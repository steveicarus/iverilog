module test;
  parameter j=0;

  reg [3:0] in [7:0];
  wire [3:0] out [7:0];

  assign out[(j+1)*4 - 1 : j*4] = in[j];
//  assign out[j] = in[j]; // This is what was probably wanted.

  initial $display("out[0]: %b", out[0]);

endmodule
