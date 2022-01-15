module top;
  reg passed = 1'b1;
  reg [7:0] in;

  lwr dut(in);

  initial begin
    #1 in = 8'd1;
    #1 in = 8'd2;

    #1 if (passed) $display("PASSED");
  end
endmodule

module lwr(input [7:0] xin);
  wire [7:0] x1 = {xin,{0{1'b0}}};

  always @(x1) if (x1 != $time) begin
    $display("Failed at time %2d, expected %2d, got %2d", $time, $time, x1);
    top.passed = 1'b0;
  end
endmodule
