module top;
  integer i = 0;

  generate
    for(i=0; i<4; i=i+1) begin:U
      reg [1:0] a = i;
    end
  endgenerate

  initial $display("FAILED");
endmodule
