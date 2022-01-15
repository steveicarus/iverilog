module top;
  reg pass = 1'b1;
  reg [2:0] in;
  real rin;
  wire out, rout;

  assign out = ('d4 == in**2'd2);
  assign rout = (4.0 == rin**2);

  initial begin
    in = 'd0; rin = 0.0;
    #1 if (out != 1'b0 && rout != 1'b0) begin
      $display("FAILED 0/0.0 check");
      pass = 1'b0;
    end

    #1 in = 'd1; rin = 1.0;
    #1 if (out != 1'b0 && rout != 1'b0) begin
      $display("FAILED 1/1.0 check");
      pass = 1'b0;
    end

    #1 in = 'd2; rin = 2.0;
    #1 if (out != 1'b1 && rout != 1'b1) begin
      $display("FAILED 2/2.0 check");
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
