module top;
  reg res;
  reg [1:0] in;

  initial begin
    in = 2'b00;

    res = ~ |in;
    res = ~ &in;
    res = ~ ^in;

    $display("FAILED: These expressions should be a syntax error.");
  end
endmodule
