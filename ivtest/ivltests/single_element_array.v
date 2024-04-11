module SingleElementArray(
  input wire [48:0] x1,
  output wire [48:0] out
);
  wire [48:0] x17[0:0];
  assign x17[0] = x1;
  assign out = {x17[0]};
endmodule

module testbench;
  reg [48:0] in;
  wire [48:0] out;

  SingleElementArray dut(.x1(in), .out(out));

  initial begin
    in = 49'h0000000000000;
    #1;
    if (out != 49'h0000000000000) begin
      $display("FAILED");
      $finish;
    end
    in = 49'h1555555555555;
    #1;
    if (out != 49'h1555555555555) begin
      $display("FAILED");
      $finish;
    end
    $display("PASSED");
  end
endmodule
