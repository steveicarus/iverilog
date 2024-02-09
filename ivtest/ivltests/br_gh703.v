module test;

logic [7:0] dout;
logic [7:0] sel;

for (genvar i = 0; i < 8; i++) begin
  if (i == 0) begin

    assign dout[i] = 1'b0;

  end else if (i == 1) begin

    assign dout[i] = 1'b1;

  end else begin

    // using always block reports error
    always @(*) begin
        if (sel[i]) dout[i] = 1'b1;
        else dout[i] = 1'b0;
    end

  end
end

logic [7:0] expected;

reg failed = 0;

initial begin
  sel = 8'd1;
  repeat (8) begin
    #1 $display("%b %b", sel, dout);
    expected = sel & 8'b11111100 | 8'b00000010;
    if (dout !== expected) failed = 1;
    sel = sel << 1;
  end
  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
