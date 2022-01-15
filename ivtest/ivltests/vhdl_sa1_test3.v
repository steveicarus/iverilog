module test;
  reg clk, reset;
  wire [24:0] count;

  initial begin
    clk = 1'b0;
    forever #25 clk = ~clk;
  end

  initial begin
    reset = 1'b0;
    @(negedge clk);
    reset = 1'b1;
    repeat(6) @(negedge clk);
    reset = 1'b0;
  end

  initial begin
    #200000;
    #500;
    if (count != 2000) begin
      $display ("Counting FAILED");
      $finish;
    end
    else begin
      $display ("PASSED");
      #20;
      $finish;
  end
end

  bigcount duv (.clk(clk), .reset(reset), .count(count) );

endmodule
