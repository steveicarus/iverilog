module t;
  wire [3:0] w;
  assign w[4:1] = 4'b1010;
  initial begin
    #1;
    if (w !== 4'b010z) $display("FAILED: expected 4'b010z, got '%b'", w);
    else $display("PASSED");
  end
endmodule
