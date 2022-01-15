module top;
  real in;
  wire signed [31:0] tmp;

  assign tmp = $rtoi(in*2.0);

  initial begin
    for (in=-1.0; in <= 1.0; in=in+1.0) begin
      #1 $display(tmp, " %.5f", in);
    end
    //$display("PASSED");
  end
endmodule
