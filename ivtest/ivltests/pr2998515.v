module top;
  reg pass;
  reg signed [63:0] error;

  initial begin
    pass = 1'b1;
    error = 0;
    error = error + 64'h40000000;
    error = error + 64'h80000000;
    if (error !== 64'hc0000000) begin
      $display("FAILED immediate add, got %h", error);
      pass = 1'b0;
    end

    error = error + -64'sh40000000;
    error = error + -64'sh80000000;
    if (error !== 64'h00000000) begin
      $display("FAILED immediate add, got %h", error);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
