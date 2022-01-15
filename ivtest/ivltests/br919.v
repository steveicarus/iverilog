module top;
  reg pass;
  reg we;
  reg [7:0] c;
  reg signed [7:0] res;

  initial begin
    pass = 1'b1;
    c = 8'd3;
    we = 1'b1;

//    $display ("res(we) = %d", (we ? (-$signed(c)) / 8'sd2 : 8'd1));
//    $display ("res(1)  = %d", (1'b1 ? (-$signed(c)) / 8'sd2 : 8'd1));
    res = we ? (-$signed(c)) / 8'sd2 : 8'd1;
    if (res !== 126) begin
      $display("Failed: variable ternary, expected 126, got %d", res);
      pass = 1'b0;
    end

    res = 1'b1 ? (-$signed(c)) / 8'sd2 : 8'd1;
    if (res !== 126) begin
      $display("Failed: constant true ternary, expected 126, got %d", res);
      pass = 1'b0;
    end

    res = 1'b0 ? 8'd1 : (-$signed(c)) / 8'sd2;
    if (res !== 126) begin
      $display("Failed: constant false ternary, expected 126, got %d", res);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end

endmodule
