module main;
  parameter [15:0] a = 16'h8421;
  reg [3:0] b, c;
  reg pass;

  always @* begin
    b = a[c+:4];
//    $display($time, " c: %d, b: %h", c, b);
  end

  initial begin
    pass = 1'b1;
    c = 0;
    #1 if (b !== 4'd1) begin
      $display("FAILED: c = 0, expected 1, got %0d", b);
      pass = 1'b0;
    end

    #9 c = 4;
    #1 if (b !== 4'd2) begin
      $display("FAILED: c = 4, expected 2, got %0d", b);
      pass = 1'b0;
    end

    #9 c = 8;
    #1 if (b !== 4'd4) begin
      $display("FAILED: c = 8, expected 4, got %0d", b);
      pass = 1'b0;
    end

    #9 c = 12;
    #1 if (b !== 4'd8) begin
      $display("FAILED: c = 12, expected 8, got %0d", b);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
