module top;
  reg clk;
  reg pass = 1'b1;

  generate
    genvar n;
    for (n=0; n<4; n=n+1) begin : loop
      reg [n:0] r;
      always @(clk) r = n;
    end
  endgenerate

  initial begin
    clk = 0;
    #1;

    if (loop[0].r !== 0) begin
      $display("Failed generate instance 0");
      pass = 1'b0;
    end

    if (loop[1].r !== 1) begin
      $display("Failed generate instance 1");
      pass = 1'b0;
    end

    if (loop[2].r !== 2) begin
      $display("Failed generate instance 2");
      pass = 1'b0;
    end

    if (loop[3].r !== 3) begin
      $display("Failed generate instance 3");
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
