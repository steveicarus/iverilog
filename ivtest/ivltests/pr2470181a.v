module main;
  reg a, b, reset, pass;

  always @*
    a = b | reset;

  always @* begin
    b = 1'b0;
    #2;
    b = a;
  end

  initial begin
    pass = 1'b1;
    reset = 1'b1;
    #1 if(b !== 1'b0) begin
      $display("FAILED initial zero for 1'b1, got %b", b);
      pass = 1'b0;
    end
    #2 if(b !== 1'b1) begin
      $display("FAILED initial set to 1'b1, got %b", b);
      pass = 1'b0;
    end

    // Since b is already 1'b1 reset can not change a to zero.
    reset = 1'b0;
    #1 if(b !== 1'b1) begin
      $display("FAILED block of initial zero for 1'b0, got %b", b);
      pass = 1'b0;
    end
    #2 if(b !== 1'b1) begin
      $display("FAILED block of initial set to 1'b0, got %b", b);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end

endmodule
