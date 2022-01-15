module top;
  reg pass = 1'b1;
  reg sel = 0;
  wire [2:0] out;
  pullup(out[0]);
  pullup(out[1]);
  assign out = sel ? 3'b000 : 3'bzzz;

  initial begin
    #1 if (out !== 3'bz11) begin
      $display("FAILED: initial value, expected 3'bz11, got %b", out);
      pass = 1'b0;
    end
    #1 sel = 1;
    #1 if (out !== 3'b000) begin
      $display("FAILED: final value, expected 3'b000, got %b", out);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
