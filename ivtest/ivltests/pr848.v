/* Based on PR#848 */

module err ();

reg clk;
initial begin
      clk = 1'b1;
      #3 forever #10 clk=~clk;
end

reg [31:0] mem [10:0];

wire [32:0] kuku = {1'b0,mem[3]};

always @(posedge clk) begin
   if (kuku !== 33'h0_xx_xx_xx_xx) begin
      $display("FAILED -- kuku has wrong value %h", kuku);
      $finish;
   end

   $display("PASSED");
   $finish;
end

endmodule
