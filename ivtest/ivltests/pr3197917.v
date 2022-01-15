module top;
  reg [23:0] in1;
  reg [54:0] in2;

  initial begin
    in1 = 24'b111111000000111111000000;
    in2 = 55'b0000011111000001111100000111110000011111000001111100000;
    #1;
    if (dut.arg !== 96'b111111000000111111000000zzzzzzzzzzzzzzzzz0000011111000001111100000111110000011111000001111100000) begin
      $display("FAILED");
    end else $display("PASSED");
  end
  test dut(in1, in2);
endmodule

module test(arg[119:96], arg[78:24]);
  input [119:24] arg;

endmodule
