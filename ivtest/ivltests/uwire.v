module top;
  reg pass;
  uwire zero, one;

  assign one = 1'b1;

  initial begin
    pass = 1'b1;
    #1;
    if (zero !== 1'bz) begin
      $display("Failed: undriven uwire gave %b", zero);
      pass = 1'b0;
    end
    if (one !== 1'b1) begin
      $display("Failed: driven uwire gave %b", one);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
