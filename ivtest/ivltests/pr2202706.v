module top;
  reg passed;
  reg [63:0] wide;
  reg [31:0] norm;

  initial begin
    passed = 1'b1;
    if (! $value$plusargs("option=%h", wide)) begin
      $display("FAILED: Unable to read wide option");
      passed = 1'b0;
    end
    if (wide !== 64'h0123456789abcdef) begin
      $display("FAILED: wide expected 64'h0123456789abcdef, got %h", wide);
      passed = 1'b0;
    end

    if (! $value$plusargs("option=%h", norm)) begin
      $display("FAILED: Unable to read normal option");
      passed = 1'b0;
    end
    if (norm !== 32'h89abcdef) begin
      $display("FAILED: normal expected 32'h89abcdef, got %h", norm);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
