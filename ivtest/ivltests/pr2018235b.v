module top;
  reg pass = 1'b1;
  reg in [1:1];
  wire out = in[1];

  initial begin
    in[1] = 1'b0;
    #1 if(out != 1'b0) begin
      $display("FAILED: CA from array, expected 1'b0, got %b", out);
      pass = 1'b0;
    end
    if (pass) $display("PASSED");
  end
endmodule
