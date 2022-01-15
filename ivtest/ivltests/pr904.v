/*
 * Based on PR#904.
 * This test is part of elist, and *should* generate an
 * error.
 */
module err ();

reg clk, reset_b;
initial begin
      clk = 1'b1;
      #3 forever #10 clk=~clk;
end
initial begin
  reset_b = 0;
  repeat (10) @(posedge clk);
  #3 reset_b = 1;
end


reg [31:0] kuku;
always @(posedge clk or negedge reset_b)
  if (~reset_b) begin
    kuku <= 0;
  end
  else begin
    kuku <= {3'd5,3'd5,,3'd5,3'd5,3'd5};
  end


endmodule
