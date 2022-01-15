module top;
  reg [1:0] in;
  subm sm [1:0](in);

  initial begin
  // This should trigger instance 0.
  in[0] = 0;
  #1 in[0] = 1;
  // This should trigger instance 1.
  in[1] = 0;
  #1 in[1] = 1;
  end
endmodule

module subm(input wire in);
  always @(posedge in) $display("In %m at %0t", $time);
endmodule
