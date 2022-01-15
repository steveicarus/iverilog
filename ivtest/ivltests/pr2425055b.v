module top;
  reg pass;
  reg [1:0] in, shift, result;
  reg signed [1:0] ins;

  initial begin
    pass = 1'b1;
    in = 2'b01;
    shift = 2'bx1;

    result = in << shift;
    if (result !== 2'bxx) begin
      $display("Failed <<, expected 2'bxx, got %b", result);
      pass = 1'b0;
    end

    result = in <<< shift;
    if (result !== 2'bxx) begin
      $display("Failed <<<, expected 2'bxx, got %b", result);
      pass = 1'b0;
    end

    result = in >> shift;
    if (result !== 2'bxx) begin
      $display("Failed >>, expected 2'bxx, got %b", result);
      pass = 1'b0;
    end

    result = in >>> shift;
    if (result !== 2'bxx) begin
      $display("Failed >>>, expected 2'bxx, got %b", result);
      pass = 1'b0;
    end

    ins = 2'b10;
    result = ins >>> shift;
    if (result !== 2'bxx) begin
      $display("Failed >>> (signed), expected 2'bxx, got %b", result);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
