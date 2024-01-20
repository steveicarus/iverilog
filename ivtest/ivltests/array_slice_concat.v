module ArraySliceWithNarrowStart(
  input wire [159:0] a,
  input wire start,
  output wire [95:0] out
);
  wire [31:0] a_unflattened[0:4];
  assign a_unflattened[0] = a[31:0];
  assign a_unflattened[1] = a[63:32];
  assign a_unflattened[2] = a[95:64];
  assign a_unflattened[3] = a[127:96];
  assign a_unflattened[4] = a[159:128];
  wire [31:0] array_slice_6[0:2];
  assign array_slice_6[0] = a_unflattened[{2'h0, start} > 3'h4 ? 3'h4 : {2'h0, start} + 3'h0];
  assign array_slice_6[1] = a_unflattened[{2'h0, start} > 3'h3 ? 3'h4 : {2'h0, start} + 3'h1];
  assign array_slice_6[2] = a_unflattened[{2'h0, start} > 3'h2 ? 3'h4 : {2'h0, start} + 3'h2];
  assign out = {array_slice_6[2], array_slice_6[1], array_slice_6[0]};
endmodule

module top;
  reg [159:0] a;
  reg start;
  wire [95:0] out;

  ArraySliceWithNarrowStart dut(.a(a), .start(start), .out(out));

  initial begin
     a = {32'h44444444, 32'h33333333, 32'h22222222, 32'h11111111};
     start = 1;
     #1;
     if (out !== 96'h444444443333333322222222) begin
	 $display("FAILED");
	 $finish;
     end
     $display("PASSED");
  end
endmodule
