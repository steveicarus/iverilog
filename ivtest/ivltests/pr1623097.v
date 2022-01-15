/* pr1623097 */
`timescale 1ns/1ns


module top;

  reg [3:0] state;
  reg [3:0] data;
  reg [3:0] clear;
  reg clk;

  genvar i;

  initial begin
    #0; // avoid time-0 race
    clk = 0; data = 4'b1111; clear = 4'b1111;
    $monitor($time,,"clk=%b, data=%b, clear=%b, state=%b",
	    clk, data, clear, state);

    #10 clear = 4'b0000;
    #10 clk = 1;
    #10 clk = 0; clear = 4'b0010;
    #10 clear = 4'b0000; data = 4'b1010;
    #10 clk = 1;
    #10 clk = 0;
  end


  // This fails!
  generate for (i=0; i<4; i=i+1) begin:sm
    always @(posedge clk or posedge clear[i]) begin
      if (clear[i]) state[i] <= 1'b0;       // Async. clear the flip bit.
      else begin
        state[i] <= #1 data[i];
      end
    end
  end endgenerate



























endmodule
