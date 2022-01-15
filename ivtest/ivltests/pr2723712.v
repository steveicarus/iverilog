module top;
  reg  [7:0] a;
  reg  [2:0] b;
  wire [7:0] y, z;

  assign y = a >> b;
  assign z = $signed(a) >> $signed(b);

  initial begin
    // Example vector
    a = 8'b10101010;
    b = 3'b101;
    #1;

    // Test for correctness
    if (y === z && y === 8'b00000101)
      $display("PASSED");
    else
      $display("FAILED, expected 8'b00000101, got %b/%b", y, z);
  end
endmodule
